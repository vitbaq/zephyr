/*
 * Copyright (c) 2018, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "knot_protocol.h"

typedef int (*knot_callback) (u8_t id);

s8_t data_item_register(u8_t id, const char *name,
			u16_t type_id, u8_t value_type, u8_t unit,
			knot_callback read, knot_callback write);

s8_t data_item_config(u8_t id, u8_t evflags, u16_t time_sec,
			knot_value_type *lower, knot_value_type *upper);