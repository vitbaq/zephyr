/*
 * Copyright (c) 2018, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>

#include "knot_types.h"
#include "knot_protocol.h"

#include "data_app.h"
#include "data.h"

#define KNOT_THING_DATA_MAX	3

static struct _data_items {
	/* KNoT identifier */
	u8_t			id;

	/* Schema values */
	u8_t			value_type;
	u8_t			unit;
	uint16_t		type_id;
	const char		*name;

	/* Data values */
	bool			new_value;
	knot_value_type		last_value;
	u8_t			*last_value_raw;
	u8_t			raw_length;

	/* Config values */
	knot_config		config;

	/* Time values */
	u32_t			last_timeout;

	/* KNoT callback functions */
	knot_callback		read;
	knot_callback		write;
} data_items[KNOT_THING_DATA_MAX];

void data_items_reset(void)
{
	u8_t id;

	for (id = 0; id < KNOT_THING_DATA_MAX; ++id) {
		data_items[id].id = -1;
		data_items[id].name = NULL;
		data_items[id].type_id = KNOT_TYPE_ID_INVALID;
		data_items[id].unit = KNOT_UNIT_NOT_APPLICABLE;
		data_items[id].value_type = KNOT_VALUE_TYPE_INVALID;
		data_items[id].config.event_flags = KNOT_EVT_FLAG_UNREGISTERED;
		
		data_items[id].new_value = false;

		/* As "last_value" is a union,
		we need just to set the "biggest" member */
		data_items[id].last_value.val_f.value_int = 0;
		data_items[id].last_value.val_f.value_dec = 0;

		/* As "lower_limit" is a union,
		we need just to set the "biggest" member */
		data_items[id].config.lower_limit.val_f.value_int = 0;
		data_items[id].config.lower_limit.val_f.value_dec = 0;

		/* As "upper_limit" is a union,
		we need just to set the "biggest" member */
		data_items[id].config.upper_limit.val_f.value_int = 0;
		data_items[id].config.upper_limit.val_f.value_dec = 0;
		data_items[id].last_value_raw = NULL;
		data_items[id].raw_length = 0;

		/* Last timeout reset */
		data_items[id].last_timeout = 0;

		data_items[id].read = NULL;
		data_items[id].write = NULL;
	}
}