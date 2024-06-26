/**
 * Apollo board definitions for Cynthion r0.3 and above
 *
 * Copyright (c) 2020-2023 Great Scott Gadgets <info@greatscottgadgets.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APOLLO_BOARD_H__
#define __APOLLO_BOARD_H__

#include <sam.h>
#include <hal/include/hal_gpio.h>
#include <stdbool.h>

#if (((_BOARD_REVISION_MAJOR_ == 0) && (_BOARD_REVISION_MINOR_ >= 6)) || (_BOARD_REVISION_MAJOR_ == 1))
#define BOARD_HAS_USB_SWITCH
/*
 * Hardware revisions r0.3 through r0.5 have a button, but its GPIO pin is
 * shared with LED_A.  We never needed to use the button on those revisions
 * because they effectively had a dedicated USB port for Apollo.  Sharing the
 * pin is tricky, but we'll probably never need to implement that.  Unless and
 * until we implement it, pretend that the button does not exist.
 */
#define BOARD_HAS_PROGRAM_BUTTON
#endif

/**
 * GPIO pins for each of the microcontroller LEDs.
 */
typedef enum {
	LED_A = PIN_PA16, // Blue
	LED_B = PIN_PA17, // Pink
	LED_C = PIN_PA22, // White
	LED_D = PIN_PA23, // Pink
	LED_E = PIN_PA27, // Blue

	LED_COUNT = 5
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
	FPGA_PROGRAM   = PIN_PA08,
#if ((_BOARD_REVISION_MAJOR_ == 0) && (_BOARD_REVISION_MINOR_ < 6))
	PROGRAM_BUTTON = PIN_PA16,
	PHY_RESET      = PIN_PA09,
#else
	PROGRAM_BUTTON = PIN_PA02,
	USB_SWITCH     = PIN_PA06,
	FPGA_ADV       = PIN_PA09,
#endif
#if ((_BOARD_REVISION_MAJOR_ == 1) && (_BOARD_REVISION_MINOR_ > 2))
	FPGA_INITN     = PIN_PA03,
	FPGA_DONE      = PIN_PA04,
#endif
};


#endif
