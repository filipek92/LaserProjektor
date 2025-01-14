#include "main.h"
#include "led.h"
#include "init.h"

#define IMG_BUFFER 960*1000/8
#include "img.h"

#define Y_SIZEB (y_size/8)
#define DEFUALT_PREAMBLE 	40 // PreImage data
#define PREAMBLE_MAX 		200

#define DMA_PREAMBLE	0
#define DMA_DATA		1

volatile int dma_state = DMA_PREAMBLE;

void sendLine();
void inc_line();

//Commands
int arglist(int argc, char *argv[]);
int setline(int argc, char *argv[]);
int rotation(int argc, char *argv[]);
int linefreq(int argc, char *argv[]);
int steptoline(int argc, char *argv[]);
int motor(int argc, char *argv[]);
int resolution(int argc, char *argv[]);
int prescaler(int argc, char *argv[]);
int transfer(int argc, char *argv[]);
int dump(int argc, char *argv[]);
int laser(int argc, char *argv[]);
int crc(int argc, char *argv[]);
int buffer(int argc, char *argv[]);
int echo(int argc, char *argv[]);
int id(int argc, char *argv[]);
int preamble(int argc, char *argv[]);
int off(int argc, char *argv[]);

DMA_HandleTypeDef	dmauartrx;
DMA_HandleTypeDef	dmaspitx;
SPI_HandleTypeDef	print_spi;
TIM_HandleTypeDef	tim;
TIM_HandleTypeDef	tim_motor;
UART_HandleTypeDef 	pc_uart;
Terminal_t			term;
CRC_HandleTypeDef	hcrc;

volatile uint16_t scan_line = 0;
volatile int steptoline_cnt = 8;

HAL_StatusTypeDef status;

float line_frequency;
float bit_frequency;

volatile uint16_t diff;
volatile uint8_t flag=0;

volatile uint16_t x_size = X_SIZE;
volatile uint16_t y_size = Y_SIZE;

volatile uint8_t laser_on = 1;

uint8_t preamble_data[PREAMBLE_MAX];
uint8_t preamble_len=DEFUALT_PREAMBLE;

void main(){
	init_peripherals();
	TERM_AddCommand(&term, "args", arglist);
	TERM_AddCommand(&term, "line", setline);
	TERM_AddCommand(&term, "rotation", rotation);
	TERM_AddCommand(&term, "linef", linefreq);
	TERM_AddCommand(&term, "liner", steptoline);
	TERM_AddCommand(&term, "motor", motor);
	TERM_AddCommand(&term, "resolution", resolution);
	TERM_AddCommand(&term, "prescaler", prescaler);
	TERM_AddCommand(&term, "transfer", transfer);
	TERM_AddCommand(&term, "dump", dump);
	TERM_AddCommand(&term, "bindump", dump);
	TERM_AddCommand(&term, "laser", laser);
	TERM_AddCommand(&term, "crc", crc);
	TERM_AddCommand(&term, "buffer", buffer);
	TERM_AddCommand(&term, "echo", echo);
	TERM_AddCommand(&term, "id", id);
	TERM_AddCommand(&term, "preamble", preamble);
	TERM_AddCommand(&term, "off", off);

	memset(preamble_data, 0x00, DEFUALT_PREAMBLE);

	printf("Projector Boot\r\n");
	printf("SystemCoreClock: %lu Hz\r\n", SystemCoreClock);

	LED_On(LED_G);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);

	printf("Ready\r\n");
	TERM_Prompt(&term);
	sendLine();
	scan_line = 3;
	while (1){
		LED_Toggle(LED_G);
		HAL_Delay(50);
	}
}

void sendLine(){
	uint8_t *data = (uint8_t *) img+(scan_line*Y_SIZEB);
	HAL_SPI_Transmit_DMA(&print_spi, data, Y_SIZEB);
}

void inc_line(){
	scan_line = (scan_line+1)%x_size;
}

void HAL_GPIO_EXTI_Callback(uint16_t pin){
	if(pin==GPIO_PIN_0){
		laser_on = !laser_on;
		laser(0, NULL);
	}
	if(pin==GPIO_PIN_7) sendLine();
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
	static uint16_t last_value;

	if (htim->Instance==TIM2){
		uint16_t now = __HAL_TIM_GET_COMPARE(&tim, TIM_CHANNEL_4);	//read TIM2 channel 1 capture value
		uint16_t scan_time = (now-last_value); //in us;
		line_frequency = 1000000.0/scan_time;
		last_value = now;

		if(laser_on){
			dma_state = DMA_PREAMBLE;
			HAL_SPI_Transmit_DMA(&print_spi, preamble_data, preamble_len);
		}
	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	UNUSED(hspi);
	if(dma_state==DMA_PREAMBLE){
		dma_state = DMA_DATA;
		uint8_t *data = (uint8_t *) img+(scan_line*Y_SIZEB);
		HAL_SPI_Transmit_DMA(&print_spi, data, Y_SIZEB);
	}else{
		uint8_t after = /*laser_on?*/0xFF/*:0x00*/;
		HAL_SPI_Transmit(&print_spi, &after, 1, 1); //Send Dummy 0xFF, to force output to 1 for synchro impuls
	}
}

int arglist(int argc, char *argv[]){
	printf("ArgList:\r\n");
	for(int i = 0; i < argc; i++){
		printf(" %d. \"%s\"\r\n", i, argv[i]);
	}
	return 0;
}

int setline(int argc, char *argv[]){
	if(argc==2) scan_line = atoi(argv[1]);
	printf("Scanline %d\r\n", scan_line);
	return 0;
}

int rotation(int argc, char *argv[]){
	int retcode = 0;
	if(argc==2){
		int arg = atoi(argv[1]);
		if(strcasecmp(argv[1], "stop")==0 || arg==0){
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
			printf("Rotation stopped\r\n");
			return 0;
		}
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);

		uint32_t arr_reg = (42000000.0/(tim_motor.Instance->PSC+1))/arg - 1;
		if(arr_reg < 65535){
			tim_motor.Instance->ARR = arr_reg;
		}
		else{
			float f = (42000000.0/(tim_motor.Instance->PSC+1))/(65535 + 1)+1;
			printf("Error: Too slow frequency for this prescaler\r\n");
			printf("Minimum is %d Hz\r\n", (int) f);
			retcode = 1;
		}
	}else{
		float minfreq = ((42000000.0/(tim_motor.Instance->PSC+1))/65536)+1;
		float maxfreq = (42000000.0/(tim_motor.Instance->PSC+1))/1;
		printf("Min frequency %lu Hz, Max frequency %lu Hz\r\n", (uint32_t) minfreq, (uint32_t) maxfreq);
	}

	if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0)==GPIO_PIN_SET){
		printf("Rotation stopped\r\n");
		return 0;
	}
	float freq = (42000000.0/(tim_motor.Instance->PSC+1))/(tim_motor.Instance->ARR + 1);
	printf("Motor ARR=%lu\r\n", tim_motor.Instance->ARR);
	printf("Motor frequency %lu Hz\r\n", (uint32_t) freq);
	return retcode;
}

int prescaler(int argc, char *argv[]){
	int retcode = 0;
	if(argc==2){
		tim_motor.Instance->PSC = atoi(argv[1]) - 1;
	}
	float freq = (42000000.0/(tim_motor.Instance->PSC+1))/(tim_motor.Instance->ARR + 1);
	float minfreq = ((42000000.0/(tim_motor.Instance->PSC+1))/65536)+1;
	float maxfreq = (42000000.0/(tim_motor.Instance->PSC+1))/1;
	printf("Prescaler = %lu\r\n", tim_motor.Instance->PSC+1);
	printf("Motor frequency %lu Hz\r\n", (uint32_t) freq);
	printf("Min frequency %lu Hz, Max frequency %lu Hz\r\n", (uint32_t) minfreq, (uint32_t) maxfreq);
	return retcode;
}

int id(int argc, char *argv[]){
	UNUSED(argc); UNUSED(argv);
	printf("Unique ID is %X", UNIQUE_ID[0]);
	for(int i=1; i<12; i++){
		printf("-%X", UNIQUE_ID[i]);
	}
	printf("\r\n");
	return 0;
}

int linefreq(int argc, char *argv[]){
	UNUSED(argc); UNUSED(argv);
	printf("Scan frequency %lu Hz\r\n", (uint32_t) line_frequency);
	return 0;
}

int steptoline(int argc, char *argv[]){
	if(argc==2) steptoline_cnt = atoi(argv[1]);
	printf("Step to line ratio %d steps/line\r\n", steptoline_cnt);
	return 0;
}

int motor(int argc, char *argv[]){
	int state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);
	if(argc == 2){
		state = strcasecmp("ON",argv[1])==0;
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, state);
	}
	printf("Motor is %s\r\n", state?"ON":"OFF");
	return 0;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	UNUSED(huart);
	printf("\r\nTransfer done\r\n");
	TERM_Prompt(&term);
	TERM_PrintBuffer(&term);
}

int transfer(int argc, char *argv[]){
	int start = 0;
	int len = 1;

	if(argc > 1) len = atoi(argv[1]);
	if(argc > 2) start = atoi(argv[2]);

	if(start >= IMG_BUFFER) return -1;
	if((len+start) > IMG_BUFFER) return -2;

	HAL_StatusTypeDef status = HAL_UART_Receive_DMA(&pc_uart, img+start, len);
	if(status==HAL_OK){
		printf("Ready for transfer %d bytes\r\n", len);
	}else{
		printf("Error, status %d\r\n", status);
	}
	return status;
}

int dump(int argc, char *argv[]){
	unsigned int start = 0;
	unsigned int len = IMG_BUFFER;
	int bin = argv[0][0]=='b';
	if(argc > 1) len = atoi(argv[1]);
	if(argc > 2) start = atoi(argv[2]);

	if(start >= IMG_BUFFER) return -1;
	if((len+start) > IMG_BUFFER) return -2;

	if(bin){
		return HAL_UART_Transmit(&pc_uart, img+start, len, 1000);
	}
	printf("Dumping image memory from %d (physical address 0x%x)\r\n", start, (unsigned int) (start+img));
	for(unsigned int i=0; i<len; i++){
		printf("0x%08x (0x%08x) = 0x%02x\r\n", i+start,(unsigned int) (img+i+start),(unsigned int) img[i+start]);
	}
	return 0;
}

uint32_t crc32(uint8_t *addr, int len, uint32_t crc) {
   int i, j;
   uint32_t byte, mask;

   for(i = 0; i<len; i++){
      byte = addr[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
   }
   return ~crc;
}

int crc(int argc, char *argv[]){
	unsigned int start = 0;
	unsigned int len = IMG_BUFFER;
	if(argc > 1) len = atoi(argv[1]);
	if(argc > 2) start = atoi(argv[2]);

	if(start >= IMG_BUFFER) return -1;
	if((len+start) > IMG_BUFFER) return -2;

	//uint32_t crc = HAL_CRC_Calculate(&hcrc, img+start, len);
	uint32_t crc = crc32(img+start, len, 0xFFFFFFFF);

	printf("CRC is 0x%08X\r\n", (unsigned int) crc);
	return crc;
}

int resolution(int argc, char *argv[]){
	if(argc == 3){
		uint16_t tmp_x_size = atoi(argv[1]);
		uint16_t tmp_y_size = atoi(argv[2]);

		if(((tmp_x_size*tmp_y_size)/8)>IMG_BUFFER){
			printf("Maximum size overflow (%d px)\r\n", tmp_x_size*tmp_y_size);
			return -1;
		}

		if((tmp_y_size%8)!=0){
			printf("Y size must be divisible by 8!");
			return -2;
		}

		x_size = tmp_x_size;
		y_size = tmp_y_size;
	}
	printf("Image resolution is %ux%d px\r\n", x_size, y_size);
	return 0;
}

int laser(int argc, char *argv[]){
	if(argc > 2){
		printf("Wrong arguments");
		return -1;
	}
	if(argc == 2) laser_on = strcasecmp("on", argv[1])==0;

	GPIO_InitTypeDef gpio;
	gpio.Alternate = laser_on?GPIO_AF5_SPI1:0;
	gpio.Mode = laser_on?GPIO_MODE_AF_PP:GPIO_MODE_OUTPUT_PP;
	gpio.Pin = GPIO_PIN_7;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOA, &gpio);

	laser_on?LED_On(LED_R):LED_Off(LED_R);

	printf("Laser is %s\r\n", laser_on?"ON":"OFF");
	return 0;
}

int off(int argc, char *argv[]){
	UNUSED(argc);
	UNUSED(argv);
	char *a[] = {NULL, "off"};
	laser(2, a);
	motor(2, a);
	rotation(2, a);
	return 0;
}

int buffer(int argc, char *argv[]){
	UNUSED(argc); UNUSED(argv);
	printf("Buffer size is %d bytes (%d pixels)\r\n", IMG_BUFFER, IMG_BUFFER*8);
	return 0;
}

int echo(int argc, char *argv[]){
	if(argc == 2){
		if(strcasecmp("on", argv[1]) == 0){
			term.echo = 1;
			term.prompt = "Projektor #";
		}else if(strcasecmp("off", argv[1]) == 0){
			term.echo = 0;
			term.prompt = 0;
		}else{
			return -1;
		}
	}
	printf("Echo is %s.\r\n", term.echo?"ENABLED":"DISABLED");
	return 0;
}

int preamble(int argc, char *argv[]){
	if(argc > 3) return -1;
	if(argc > 1){
		uint8_t tmp = atoi(argv[1]);
		preamble_len = tmp<PREAMBLE_MAX?tmp:PREAMBLE_MAX;
	}
	if(argc > 2) memset(preamble_data, atoi(argv[2]), preamble_len);
	printf("Preamble len will be %d\r\n", preamble_len);
	return 0;
}

//RTC_HandleTypeDef rtc;
//int date(int argc, char *argv[]){
//	rtc.Init.HourFormat = RTC_HOURFORMAT_24;
//	HAL_RTC_Init(&rtc);
//	RTC_DateTypeDef d;
//	RTC_TimeTypeDef t;
//	HAL_RTC_GetDate(&rtc, &d, RTC_FORMAT_BIN);
//	HAL_RTC_GetTime(&rtc, &t, RTC_FORMAT_BIN);
//
//	printf("%d:%02d:%02d.%04d %02.%02.%02d", t.Hours, t.Minutes, t.Seconds, d.Date, d.Month, d.Year);
//	return 0;
//}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){
	static int cnt=0;
	if(htim == &tim_motor){
		if(cnt++ % steptoline_cnt == 0){
			LED_Toggle(LED_B);
			inc_line();
		}
	}
}
