/*
 * Copyright (c) 2018, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <zephyr/types.h>

#include "knot_types.h"

#include "KNoTThing.h"

int knot_set_int(u8_t id, int value)
{
	knot_value_type int_v;

	int_v.val_i.value = value;

	return data_set_value(id, KNOT_VALUE_TYPE_INT, int_v);
}

int knot_get_int(u8_t id, int *value)
{
	int ret;
	knot_value_type int_v;

	ret = data_get_value(id, KNOT_VALUE_TYPE_INT, &int_v);

	*value = int_v.val_i.value;

	return ret;
}

int knot_set_float(u8_t id, s32_t value_int, u32_t value_dec)
{
	knot_value_type float_v;

	float_v.val_f.value_int = value_int;
	float_v.val_f.value_dec = value_dec;

	return data_set_value(id, KNOT_VALUE_TYPE_FLOAT, float_v);
}

int knot_get_float(u8_t id, s32_t *value_int, u32_t *value_dec)
{
	int ret;
	knot_value_type float_v;

	ret = data_get_value(id, KNOT_VALUE_TYPE_FLOAT, &float_v);

	*value_int = float_v.val_f.value_int;
	*value_dec = float_v.val_f.value_dec;

	return ret;
}

int knot_set_bool(u8_t id, bool value)
{
	knot_value_type bool_v;

	bool_v.val_b = value;

	return data_set_value(id, KNOT_VALUE_TYPE_BOOL, bool_v);
}

int knot_get_bool(u8_t id, bool *value)
{
	int ret;
	knot_value_type bool_v;

	ret = data_get_value(id, KNOT_VALUE_TYPE_BOOL, &bool_v);

	*value = bool_v.val_b;

	return ret;
}

int knot_set_raw(u8_t id, u8_t buffer, s32_t len)
{
	return -1;
}

int knot_get_raw(u8_t id)
{
	return -1;
}

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