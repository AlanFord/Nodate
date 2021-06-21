// Basic Pushy example for Nodate's STM32 framework.

#include <usart.h>
#include <io.h>
#include <gpio.h>
#include <nodate.h>

#include "printf.h"


void uartCallback(char ch) {
	// Copy character into send buffer.
#if defined  STM32F0 || STM32F1 || defined STM32F4
	USART::sendUart(USART_2, ch);
#elif defined STM32F7
	USART::sendUart(USART_3, ch);
#else
#error failed to identify board in pushy.cpp
#endif
}


int main () {
SystemCoreClockUpdate();
#if   defined STM32F0
	// nucleo-f030r8
	USART::startUart(USART_2, GPIO_PORT_A, 2, 1, GPIO_PORT_A, 3, 1, 9600, uartCallback);
#elif defined STM32F1
	// nucleo-f103rb
	USART::startUart(USART_2, GPIO_PORT_A, 2, 0, GPIO_PORT_A, 3, 0, 9600, uartCallback);
#elif defined STM32F4
	// nucleo-f446re
	USART::startUart(USART_2, GPIO_PORT_A, 2, 7, GPIO_PORT_A, 3, 7, 9600, uartCallback);
#elif defined STM32F7
	// nucleo-f746zg
	USART::startUart(USART_3, GPIO_PORT_D, 8, 7, GPIO_PORT_D, 9, 7, 9600, uartCallback);
#else
#error failed to identify board in pushy.cpp
#endif
	// Set LED & button.
	uint8_t 	led_pin;
	GPIO_ports 	led_port;
	bool 		led_iterate = false;
	uint32_t 	led_idx = 0;
	if (boardLEDs_count > 0) {
		if (boardLEDs_count > 1) { led_iterate = true; led_idx = 1; }
		led_pin = boardLEDs[led_idx].pin.pin;
		led_port = boardLEDs[led_idx].pin.port;
	}
	else {
		//led_pin = 3; // Nucleo-f042k6: Port B, pin 3.
		//led_port = GPIO_PORT_B;
		//led_pin = 13; // STM32F4-Discovery: Port D, pin 13 (orange)
		//led_port = GPIO_PORT_D;
		//led_pin = 7; // Nucleo-F746ZG: Port B, pin 7 (blue)
		//led_port = GPIO_PORT_B;
		//led_pin = 13;	// Blue Pill: Port C, pin 13.
		//led_port = GPIO_PORT_C;
		//led_pin = 13;	// Otter Pill: Port B, pin 13.
		//led_port = GPIO_PORT_B;
		//led_pin = 9;	// STM32F0-Discovery (PC9, green).
		//led_port = GPIO_PORT_C;
		//led_pin = 1;	// Maple Mini
		//led_port = GPIO_PORT_B;
	}
	
	uint8_t 	button_pin;
	GPIO_ports 	button_port;
	if (boardButtons_count > 0) {
		button_pin = boardButtons[0].pin.pin;
		button_port = boardButtons[0].pin.port;
	}
	else {
		//button_pin = 1; // Nucleo-f042k6 (PB1)
		//button_port = GPIO_PORT_B;
		button_pin = 0; // STM32F0-Discovery / STM32F4-Discovery (PA0)
		button_port = GPIO_PORT_A;
		//button_pin = 13; // Nucleo-F746ZG (PC13)
		//button_port = GPIO_PORT_C;
		//button_pin = 10; // Blue Pill / Otter Pill
		//button_port = GPIO_PORT_B;
		//button_pin = 8;	// Maple Mini
		//button_port = GPIO_PORT_B;
	}
	
	// Set the pin mode on the LED pin.
	GPIO::set_output(led_port, led_pin, GPIO_PULL_UP);
	GPIO::write(led_port, led_pin, GPIO_LEVEL_LOW);
	
	// Set input mode on button pin.
	GPIO::set_input(button_port, button_pin, GPIO_FLOATING);
	
	// Set up stdout.
#if defined  STM32F0 || STM32F1 || defined STM32F4
	// nucleo-f030r8
	// nucleo-f103rb
	// nucleo-f446re
	IO::setStdOutTarget(USART_2);
#elif defined STM32F7
	// nucleo-f746zg
	IO::setStdOutTarget(USART_3);
#else
#error failed to identify board in pushy.cpp
#endif

	printf("Starting pushy example...\n");
	// If the button pulls down to ground (high to low), 'button_down' is low when pushed.
	// If the button is pulled up to Vdd (low to high), 'button_down' is high when pushed.
	uint8_t button_down;
	while (1) {
		button_down = GPIO::read(button_port, button_pin);
		if (button_down == 1) {
			GPIO::write(led_port, led_pin, GPIO_LEVEL_HIGH);
		}
		else {
			GPIO::write(led_port, led_pin, GPIO_LEVEL_LOW);
		}
	}
	
	return 0;
}
