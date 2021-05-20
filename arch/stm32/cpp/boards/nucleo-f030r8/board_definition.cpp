/*
	board_definition.cpp - board definition file for the ST Nucleo-F030R8 board.
	
	MCU: STM32F030R8
	
	2021/03/28, Alan Ford
*/




#include "../board_types.h"
#include <rcc.h>
#include <usart.h>


RccSysClockConfig maxSysClockCfg;
BoardLED boardLEDs[1];
BoardButton boardButtons[1];
USART_def boardUSARTs[2];

uint8_t boardLEDs_count = 1;
uint8_t boardButtons_count = 1;

uint8_t boardUSART_count = 2;
static GpioPinDef usartTxDef[2];
static GpioPinDef usartRxDef[2];

bool init() {
	maxSysClockCfg.source 		= RCC_SYSCLOCK_SRC_PLL;
	maxSysClockCfg.HSE_bypass 	= false;
	maxSysClockCfg.HSI_enabled	= false;
	maxSysClockCfg.PLL_enabled = true;
	maxSysClockCfg.PLL_source	= RCC_PLLCLOCK_SRC_HSI;
	maxSysClockCfg.PLLM		= 12;
	maxSysClockCfg.PLLN		= 1;
	maxSysClockCfg.PLLP		= 1;
	maxSysClockCfg.PLLQ		= 1;
	maxSysClockCfg.AHB_prescale	= 1;
	maxSysClockCfg.APB1_prescale	= 1;
	maxSysClockCfg.APB2_prescale	= 1;
	maxSysClockCfg.FLASH_latency	= 1;

	BoardLED bl;
	bl.pin.port = GPIO_PORT_A;
	bl.pin.pin = 5;
	bl.pin.pupd = GPIO_PULL_UP;
	bl.pin.type = GPIO_PUSH_PULL;
	bl.pin.speed = GPIO_LOW;
	bl.rgb = { 0, 0xff, 0 };
    boardLEDs[0] = bl;


    BoardButton bb;
    bb.pin = { GPIO_PORT_C, 13, GPIO_PULL_UP };
    boardButtons[0] = bb;

    USART_def usart;
    usart.usart = USART_1;
    usart.configs = 2;
    usartTxDef[0] = { .port = GPIO_PORT_A, .pin = 9, .af = 1 }; //check
    usartTxDef[1] = { .port = GPIO_PORT_B, .pin = 6, .af = 0 };
    usart.tx = usartTxDef;
    usartRxDef[0] = { .port = GPIO_PORT_A, .pin = 10, .af = 1 }; //check
    usartRxDef[1] = { .port = GPIO_PORT_B, .pin = 7, .af = 0 };
    usart.rx = usartRxDef;
    boardUSARTs[0] = usart;
    
    usart.usart = USART_2;
    usart.configs = 2;
    usartTxDef[0] = { .port = GPIO_PORT_A, .pin = 2, .af = 1 }; //check
    usartTxDef[1] = { .port = GPIO_PORT_B, .pin = 14, .af = 1 };
    usart.tx = usartTxDef;
    usartRxDef[0] = { .port = GPIO_PORT_A, .pin = 3, .af = 1 }; //check
    usartRxDef[1] = { .port = GPIO_PORT_A, .pin = 3, .af = 1 };
    usart.rx = usartRxDef;
    boardUSARTs[1] = usart;

	return true;
}

bool initialized = init();
