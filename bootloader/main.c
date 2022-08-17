/*
* STM32 HID Bootloader - USB HID bootloader for STM32F10X
* Copyright (c) 2018 Bruno Freitas - bruno@brunofreitas.com
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stm32f10x.h>

#include "usb.h"
#include "hid.h"
#include "bitwise.h"

// HID Bootloader takes 4K
#define USER_PROGRAM 0x08001000

int main() {
	uint32_t usrSp = *(volatile uint32_t *)USER_PROGRAM;
	uint32_t usrMain = *(volatile uint32_t *)(USER_PROGRAM + 0x04); /* reset ptr in vector table */

	// Turn GPIOB clock on
	bit_set(RCC->APB2ENR, RCC_APB2ENR_IOPBEN);

	// Set B14 (col 0) as Output Mode High
	bit_set(GPIOB->CRH, GPIO_CRH_MODE14);
	bit_clear(GPIOB->CRH, GPIO_CRH_CNF14);
	bit_set(GPIOB->BSRR, GPIO_BSRR_BS14);

	// Set B8 (row 3) as Input Mode Pull Down
	bit_clear(GPIOB->CRH, GPIO_CRH_MODE8);
	bit_clear(GPIOB->CRH, GPIO_CRH_CNF8_0);
	bit_set(GPIOB->CRH, GPIO_CRH_CNF8_1);
	bit_clear(GPIOB->ODR, GPIO_ODR_ODR8);

	// If B8 (key 0,3) is HIGH then go into HID bootloader...
	if(GPIOB->IDR & GPIO_IDR_IDR8) {
		USB_Init(HIDUSB_EPHandler, HIDUSB_Reset);
	} else {
		// Turn GPIOB clock off
		bit_clear(RCC->APB2ENR, RCC_APB2ENR_IOPBEN);

		SCB->VTOR = USER_PROGRAM;

		__asm__ volatile(
			"msr msp, %0\n"
			"bx %1\n"
			:: "r" (usrSp), "r" (usrMain));
	}

	for(;;);
}
