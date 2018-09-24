/*
 * Copyright (c) 2018, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "data_app.h"
#include "knot_app.h"

int knot_set_int(u8_t id, int value);

int knot_get_int(u8_t id, int *value);

int knot_set_float(u8_t id, s32_t value_int, u32_t value_dec);

int knot_get_float(u8_t id, s32_t *value_int, u32_t *value_dec);

int knot_set_bool(u8_t id, bool value);

int knot_get_bool(u8_t id, bool *value);

int knot_register_data(u8_t sensor_id, const char *name, u16_t type_id,
			u8_t unit, u8_t value_type,
			knot_callback read, knot_callback write);

int knot_config_data(u8_t sensor_id, u8_t event_flags, u16_t time_sec,
			s32_t upper_int, u32_t upper_dec,
			s32_t lower_int, u32_t lower_dec);