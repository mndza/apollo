/**
 * Code for reading FPGA status after triggering reconfiguration.
 *
 * This file is part of LUNA.
 *
 * Copyright (c) 2023 Great Scott Gadgets <info@greatscottgadgets.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FPGA_STATUS_H__
#define __FPGA_STATUS_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * Returns the STATUS register from the FPGA.
 */
uint32_t read_fpga_status(void);

/**
 * Returns whether the FPGA was configured correctly or not.
 */
bool fpga_configured_ok(void);

#endif
