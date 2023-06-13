/**
 * Code for reading FPGA status after triggering reconfiguration.
 *
 * This file is part of LUNA.
 *
 * Copyright (c) 2023 Great Scott Gadgets <info@greatscottgadgets.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <tusb.h>
#include <bsp/board.h>
#include <apollo_board.h>

#include "fpga_status.h"
#include "jtag.h"

// Opcode for retrieving STATUS register
#define LSC_READ_STATUS    (0x3C)

// STATUS register fields
enum {
	FLAG_DONE            = (1 << 8),
	FLAG_BUSY            = (1 << 12),
	FLAG_INVALID_COMMAND = (1 << 28),
	FLAG_EXECUTION_FAIL  = (1 << 26),
	MASK_BSE_ERROR_CODE  = (7 << 23),  // 0b000 = No error
};
#define ERROR_MASK \
	(MASK_BSE_ERROR_CODE | FLAG_EXECUTION_FAIL | FLAG_INVALID_COMMAND)

static inline void delay(int cycles)
{
	while (cycles-- != 0)
		__NOP();
}

/**
 * Returns the STATUS register from the FPGA.
 */
uint32_t read_fpga_status(void)
{
	uint8_t out_buffer[4] = {0};
	uint32_t status;

	jtag_go_to_state(STATE_SHIFT_IR);
	out_buffer[0] = LSC_READ_STATUS;
	jtag_tap_shift(out_buffer, (uint8_t*)&status, 8, 1);
	jtag_go_to_state(STATE_PAUSE_IR);

	jtag_go_to_state(STATE_SHIFT_DR);
	out_buffer[0] = 0;
	jtag_tap_shift(out_buffer, (uint8_t*)&status, 32, 1);
	jtag_go_to_state(STATE_PAUSE_DR);

	return status;
}

/**
 * Returns whether the FPGA was configured correctly or not.
 */
bool fpga_configured_ok(void)
{
	uint32_t status;

	jtag_init();
	// Wait until the FPGA finishes configuration
	while ((status = read_fpga_status()) & FLAG_BUSY) {
		delay(100000);
	}
	jtag_deinit();

	// Decide configuration result from the STATUS flags
	if ((status & FLAG_DONE) && (!(status & ERROR_MASK))) {
		return true;
	} else {
		return false;
	}
}