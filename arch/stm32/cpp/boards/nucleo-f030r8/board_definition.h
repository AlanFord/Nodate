/*
	board_definition.h - board definition file for the ST Nucleo-F030R8 board.
	
	MCU: STM32F030R8
	
	2021/03/28, Alan Ford
*/

#ifndef BOARD_DEFINITION
#define BOARD_DEFINITION

#include <rcc.h>
#include <usart.h>

#include "../board_types.h"

// --- CLOCKS ---

// >> Max SysClock Profile <<
// Maximum System Clock speed configuration.
// HSE in bypass mode (using 8 MHz clock from ST-Link side).
// PLLM: 8 / 2 => 4 * 12 = 48 MHz SysClock.
// Configures APB & AHB clocks to match SysClock.
extern RccSysClockConfig maxSysClockCfg;


// --- UARTS ---

// Define the number and features of the U(S)ARTs on the board.
extern uint8_t boardUSART_count;
extern USART_def boardUSARTs[2];

// --- LEDS ---

// Define the number and features of the user-addressable LEDs on the board.
extern uint8_t boardLEDs_count;
extern BoardLED boardLEDs[1];


// --- BUTTONS ---

// Define the user-defined buttons on the board.
extern uint8_t boardButtons_count;
extern BoardButton boardButtons[1];

#endif
