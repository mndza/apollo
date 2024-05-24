
/**
 * Interface code for communicating with the FPGA over the Debug SPI connection.
 *
 * This file is part of LUNA.
 *
 * Copyright (c) 2024 Great Scott Gadgets <info@greatscottgadgets.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <tusb.h>
#include <apollo_board.h>


/**
 * Request that sends a block of data over our debug SPI.
 */
bool handle_flash_spi_send(uint8_t rhport, tusb_control_request_t const* request)
{
	return false;
}


bool handle_flash_spi_send_complete(uint8_t rhport, tusb_control_request_t const* request)
{
	return false;
}
