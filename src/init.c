#include "init.h"

void initLineRate();
void initUsart();
void irq_init();
void spi_init();
void initSPIClock();

void init_peripherals(){
	irq_init();
	spi_init();
	initUsart();
	LED_Init();
	initLineRate();
	initSPIClock();
}

inline void initLineRate(){
	__GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef gpio;
	gpio.Alternate = GPIO_AF1_TIM2;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pin = GPIO_PIN_11;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &gpio);

	__TIM2_CLK_ENABLE();
	tim.Instance = TIM2;
	tim.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
	tim.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim.Init.Period = 0xFFFF;
	tim.Init.Prescaler = 83;
	tim.Init.RepetitionCounter = 1;


	TIM_IC_InitTypeDef ic;
	ic.ICFilter = 0;
	ic.ICPolarity = TIM_ICPOLARITY_FALLING;
	ic.ICPrescaler = TIM_ICPSC_DIV1;
	ic.ICSelection = TIM_ICSELECTION_DIRECTTI;

	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_SetPriority(TIM2_IRQn, 0);

	HAL_TIM_IC_Init(&tim);
	HAL_TIM_IC_ConfigChannel(&tim, &ic, TIM_CHANNEL_4);
	HAL_TIM_IC_Start_IT(&tim, TIM_CHANNEL_4);
}

void initSPIClock(){
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef gpio;
	gpio.Alternate = GPIO_AF2_TIM3;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pin = GPIO_PIN_1;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &gpio);

	__TIM3_CLK_ENABLE();
	tim_spi_clock.Instance = TIM3;
	tim_spi_clock.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
	tim_spi_clock.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim_spi_clock.Init.Period = 99;
	tim_spi_clock.Init.Prescaler = 0;
	tim_spi_clock.Init.RepetitionCounter = 1;

	TIM_OC_InitTypeDef oc;
	oc.OCMode = TIM_OCMODE_TOGGLE;
	oc.OCPolarity = TIM_OCPOLARITY_HIGH;
	HAL_TIM_OC_Init(&tim_spi_clock);
	HAL_TIM_OC_ConfigChannel(&tim_spi_clock, &oc, TIM_CHANNEL_4);
	HAL_TIM_OC_Start(&tim_spi_clock, TIM_CHANNEL_4);
}

void initUsart(){
	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitTypeDef gpio;
	gpio.Alternate = GPIO_AF8_UART4;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FAST;
	gpio.Pin = GPIO_PIN_10; // TX
	gpio.Mode = GPIO_MODE_AF_PP;
	HAL_GPIO_Init(GPIOC, &gpio);

	gpio.Alternate = GPIO_AF8_UART4;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FAST;
	gpio.Pin = GPIO_PIN_11; // RX
	gpio.Mode = GPIO_MODE_AF_PP;
	HAL_GPIO_Init(GPIOC, &gpio);

	__HAL_RCC_UART4_CLK_ENABLE();
	pc_uart.Instance = UART4;
	pc_uart.Init.BaudRate = 115200;
	pc_uart.Init.Mode = UART_MODE_TX | UART_MODE_RX;
	pc_uart.Init.Parity = UART_PARITY_NONE;
	pc_uart.Init.StopBits = UART_STOPBITS_1;
	pc_uart.Init.WordLength = UART_WORDLENGTH_8B;
	pc_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	pc_uart.Init.OverSampling = UART_OVERSAMPLING_8;
	HAL_UART_Init(&pc_uart);

	__HAL_UART_ENABLE_IT(&pc_uart, UART_IT_RXNE);

	NVIC_EnableIRQ(UART4_IRQn);
	NVIC_SetPriority(UART4_IRQn, 1);
}


void irq_init(){
	GPIO_InitTypeDef gpio;
	gpio.Mode = GPIO_MODE_IT_FALLING;
	gpio.Speed = GPIO_SPEED_FAST;

	__HAL_RCC_GPIOC_CLK_ENABLE();
	gpio.Pin = GPIO_PIN_7;
	gpio.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOC, &gpio);
	NVIC_SetPriority(EXTI9_5_IRQn, 2);
	NVIC_EnableIRQ(EXTI9_5_IRQn);

	__HAL_RCC_GPIOA_CLK_ENABLE();
	gpio.Pin = GPIO_PIN_0;
	gpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio);

	NVIC_SetPriority(EXTI0_IRQn, 2);
	NVIC_EnableIRQ(EXTI0_IRQn);
}

void spi_init(){
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef gpio;
	gpio.Alternate = GPIO_AF5_SPI1;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pin = GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOA, &gpio);

	__HAL_RCC_SPI1_CLK_ENABLE();
	print_spi.Instance = SPI1;
	print_spi.Init.Mode = SPI_MODE_MASTER;
	print_spi.Init.Direction = SPI_DIRECTION_2LINES;
	print_spi.Init.DataSize = SPI_DATASIZE_8BIT;
	print_spi.Init.CLKPolarity = SPI_POLARITY_HIGH;
	print_spi.Init.CLKPhase = SPI_PHASE_1EDGE;
	print_spi.Init.NSS = SPI_NSS_SOFT;
	print_spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
	print_spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
	print_spi.Init.TIMode = SPI_TIMODE_DISABLED;
	print_spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
	HAL_SPI_Init(&print_spi);

	__HAL_RCC_DMA2_CLK_ENABLE();
	dmaspitx.Init.Channel = DMA_CHANNEL_3;
	dmaspitx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	dmaspitx.Init.PeriphInc = DMA_PINC_DISABLE;
	dmaspitx.Init.MemInc = DMA_MINC_ENABLE;
	dmaspitx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	dmaspitx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	dmaspitx.Init.Mode = DMA_NORMAL;
	dmaspitx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
	dmaspitx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	dmaspitx.Instance = DMA2_Stream3;
	HAL_DMA_Init(&dmaspitx);
	__HAL_LINKDMA(&print_spi,hdmatx,dmaspitx);

	NVIC_EnableIRQ(DMA2_Stream3_IRQn);
	NVIC_SetPriority(DMA2_Stream3_IRQn, 3);
}
