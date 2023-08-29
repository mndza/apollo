/**
 * switch control for USB port shared by Apollo and FPGA
 *
 * This file is part of Apollo.
 *
 * Copyright (c) 2023 Great Scott Gadgets <info@greatscottgadgets.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_switch.h"
#include "apollo_board.h"
#include "led.h"
#include <hal/include/hal_gpio.h>

#include <tusb.h>
#include <bsp/board.h>
#include <hpl/pm/hpl_pm_base.h>
#include <hpl/gclk/hpl_gclk_base.h>
#include <peripheral_clk_config.h>


#if (((_BOARD_REVISION_MAJOR_ == 0) && (_BOARD_REVISION_MINOR_ >= 6)) || (_BOARD_REVISION_MAJOR_ == 1))
#define WITH_USB_SWITCH
#endif

#ifdef WITH_USB_SWITCH

static bool control_to_fpga = false;

// Store the timestamp of the last physical port advertisement
#define TIMEOUT 100UL
static uint32_t last_phy_adv = 0;

// Create a reference to our SERCOM object.
typedef Sercom sercom_t;
static sercom_t *sercom = SERCOM1;

static void fpga_adv_init(void);
static void fpga_adv_byte_received_cb(uint8_t byte, int parity_error);

#endif

/**
 * Initialize USB switch control
 */
void usb_switch_init(void)
{
#ifndef WITH_USB_SWITCH
	gpio_set_pin_pull_mode(PROGRAM_BUTTON, GPIO_PULL_UP);
	gpio_set_pin_direction(PROGRAM_BUTTON, GPIO_DIRECTION_IN);
#else
	gpio_set_pin_pull_mode(PROGRAM_BUTTON, GPIO_PULL_OFF);
	gpio_set_pin_direction(PROGRAM_BUTTON, GPIO_DIRECTION_IN);

	gpio_set_pin_direction(FPGA_INT, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(FPGA_INT, GPIO_PULL_UP);

	gpio_set_pin_direction(USB_SWITCH, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(USB_SWITCH, false);
	control_to_fpga = true;

	fpga_adv_init();
#endif
}

/**
 * Hand off shared USB port to FPGA.
 */
void hand_off_usb(void)
{
#ifndef WITH_USB_SWITCH
	led_on(LED_D);
#else
	if (control_to_fpga == true) return;
	// Disable internal pull-up resistor on D+/D- pins for a moment to force a disconnection
	tud_disconnect();
	board_delay(100);
	gpio_set_pin_level(USB_SWITCH, false);
	led_off(LED_D);
	control_to_fpga = true;
#endif
}

/**
 * Take control of USB port from FPGA.
 */
void take_over_usb(void)
{
#ifndef WITH_USB_SWITCH
	led_on(LED_D);
#else
	if (control_to_fpga == false) return;
	gpio_set_pin_level(USB_SWITCH, true);
	// Disable internal pull-up resistor on D+/D- pins for a moment to force a disconnection
	tud_disconnect();
	board_delay(100);
	tud_connect();
	control_to_fpga = false;
#endif
}

/**
 * Handle switch control user request.
 */
void switch_control_task(void)
{
	if ((gpio_get_pin_level(PROGRAM_BUTTON) == false)) {
		take_over_usb();
	}

#ifdef WITH_USB_SWITCH
	// Take over USB after timeout
	if (board_millis() - last_phy_adv < TIMEOUT) return;
	take_over_usb();
#endif
}

/**
 * Honor requests from FPGA_ADV again
 */
void honor_fpga_adv(void)
{
#ifdef WITH_USB_SWITCH
	if (board_millis() - last_phy_adv < TIMEOUT) {
		hand_off_usb();
	}
#endif
}

#ifdef WITH_USB_SWITCH
/**
 * Initialize FPGA_ADV receive-only serial port
 */
static void fpga_adv_init(void)
{
	// Disable the SERCOM before configuring it, to 1) ensure we're not transacting
	// during configuration; and 2) as many of the registers are R/O when the SERCOM is enabled.
	while(sercom->USART.SYNCBUSY.bit.ENABLE);
	sercom->USART.CTRLA.bit.ENABLE = 0;

	// Software reset the SERCOM to restore initial values.
	while(sercom->USART.SYNCBUSY.bit.SWRST);
	sercom->USART.CTRLA.bit.SWRST = 1;

	// The SWRST bit becomes accessible again once the software reset is
	// complete -- we'll use this to wait for the reset to be finshed.
	while(sercom->USART.SYNCBUSY.bit.SWRST);

	// Ensure we can work with the full SERCOM.
	while(sercom->USART.SYNCBUSY.bit.SWRST || sercom->USART.SYNCBUSY.bit.ENABLE);

	// Pinmux the relevant pins to be used for the SERCOM.
	gpio_set_pin_function(PIN_PA09, MUX_PA09C_SERCOM1_PAD3);

	// Set up clocking for the SERCOM peripheral.
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM1);
	_gclk_enable_channel(SERCOM1_GCLK_ID_CORE, GCLK_CLKCTRL_GEN_GCLK0_Val);

	// Configure the SERCOM for UART mode.
	sercom->USART.CTRLA.reg =
		SERCOM_USART_CTRLA_DORD            |  // LSB first
		SERCOM_USART_CTRLA_RXPO(3)         |  // RX on PA09 (PAD[3])
		SERCOM_USART_CTRLA_SAMPR(0)        |  // use 16x oversampling
		SERCOM_USART_CTRLA_FORM(1)		   |  // enable parity
		SERCOM_USART_CTRLA_RUNSTDBY        |  // don't autosuspend the clock
		SERCOM_USART_CTRLA_MODE_USART_INT_CLK; // use internal clock

	// Configure our baud divisor.
	const uint32_t baudrate = 9600;
	const uint32_t baud = (((uint64_t)CONF_CPU_FREQUENCY << 16) - ((uint64_t)baudrate << 20)) / CONF_CPU_FREQUENCY;
	sercom->USART.BAUD.reg = baud;

	// Configure TX/RX and framing.
	sercom->USART.CTRLB.reg =
			SERCOM_USART_CTRLB_CHSIZE(0) | //8-bit words
			SERCOM_USART_CTRLB_RXEN;       //  Enable RX.

	// Wait for our changes to apply.
	while (sercom->USART.SYNCBUSY.bit.CTRLB);

	// Enable our receive interrupt, as we want to asynchronously dump data into
	// the UART console.
	sercom->USART.INTENSET.reg = SERCOM_USART_INTENSET_RXC;

	// Enable the UART IRQ.
	NVIC_EnableIRQ(SERCOM1_IRQn);

	// Finally, enable the SERCOM.
	sercom->USART.CTRLA.bit.ENABLE = 1;
	while(sercom->USART.SYNCBUSY.bit.ENABLE);

	// Update timestamp
	last_phy_adv = board_millis();
}

/**
 * FPGA_ADV interrupt handler.
 */
void SERCOM1_Handler(void)
{
	// If we've just received a character, handle it.
	if (sercom->USART.INTFLAG.bit.RXC)
	{
		// Read the relevant character, which marks this interrupt as serviced.
		uint16_t byte = sercom->USART.DATA.reg;
		fpga_adv_byte_received_cb(byte, sercom->USART.STATUS.bit.PERR);
	}
}

static void fpga_adv_byte_received_cb(uint8_t byte, int parity_error) {
	if (parity_error) {
		return;
	}

	if (byte == 'A') {
		last_phy_adv = board_millis();
	}
}
#endif
