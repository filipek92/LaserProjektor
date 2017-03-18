#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "diag/Trace.h"
#include "stm32f4xx.h"

extern SPI_HandleTypeDef print_spi;
extern TIM_HandleTypeDef tim;
extern USART_HandleTypeDef pc_usart;
extern DMA_HandleTypeDef dmaspitx;

#endif