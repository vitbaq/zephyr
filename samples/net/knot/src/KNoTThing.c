/*
 * Copyright (c) 2018, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <zephyr/types.h>

#include "knot_types.h"

#include "KNoTThing.h"

int knot_register_data(u8_t sensor_id, const char *name, u16_t type_id,
			u8_t unit, u8_t value_type,
			knot_callback read, knot_callback write)
{
	return data_item_register(sensor_id, name, type_id, unit, value_type,
				read, write);
}

int knot_config_data(u8_t sensor_id, u8_t event_flags, u16_t time_sec,
			s32_t upper_int, u32_t upper_dec,
			s32_t lower_int, u32_t lower_dec)
{

	knot_value_type lower;
	knot_value_type upper;

	lower.val_f.value_int = lower_int;
	lower.val_f.value_dec = lower_dec;
	upper.val_f.value_int = upper_int;
	upper.val_f.value_dec = upper_dec;

	return data_item_config(sensor_id, event_flags, time_sec,
				&lower, &upper);
}