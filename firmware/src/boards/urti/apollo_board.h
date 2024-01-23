/**
 * Apollo board definitions for URTI r0.1 and above
 *
 * Copyright (c) 2024 Great Scott Gadgets <info@greatscottgadgets.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APOLLO_BOARD_H__
#define __APOLLO_BOARD_H__

#include <sam.h>
#include <hal/include/hal_gpio.h>
#include <stdbool.h>

#define BOARD_HAS_PROGRAM_BUTTON
#define BOARD_HAS_USB_SWITCH

/**
 * GPIO pins for each of the microcontroller LEDs.
 */
typedef enum {
	LED_A = PIN_PA04,
	LED_B = PIN_PA03,
	LED_C = PIN_PA02,

	LED_COUNT = 3
} led_t;


/**
 * GPIO pins for FPGA JTAG
 */
enum {
	// Each of the JTAG pins.
	TCK_GPIO = PIN_PA15,
	TDO_GPIO = PIN_PA10,
	TDI_GPIO = PIN_PA14,
	TMS_GPIO = PIN_PA11,
};


/**
 * Other GPIO pins
 */
enum {
	PROGRAM_BUTTON = PIN_PA23,
	USB_SWITCH     = PIN_PA22,
	FPGA_PROGRAM   = PIN_PA07,
	FPGA_ADV       = PIN_PA09,
	FPGA_ENABLEN   = PIN_PA27,
	FPGA_INITN     = PIN_PA16,
	FPGA_DONE      = PIN_PA08,
};


#endif
