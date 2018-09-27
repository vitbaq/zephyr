/*
 * Copyright (c) 2018, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>

#include <string.h>

#include "knot_protocol.h"
#include "knot_types.h"
#include "msg.h"
#include "kaio.h"
#include "knot.h"

#define MIN(a, b)         (((a) < (b)) ? (a) : (b))

static struct aio {
	/* KNoT identifier */
	u8_t			id;

	/* Schema values */
	knot_schema		schema;

	/* Data values */
	bool			refresh;
	knot_value_type		value;
	u8_t			raw_length;

	/* Config values */
	knot_config		config;

	/* Time values */
	u32_t			last_timeout;

	knot_callback_t		read_cb;
	knot_callback_t		write_cb;
} aio[KNOT_THING_DATA_MAX];

static u8_t last_id = 0xff;

void knot_start(void)
{
	int index;

	memset(aio, 0, sizeof(aio));

	for (index = 0; (index < sizeof(aio) / sizeof(struct aio)); index++)
		aio[index].id = 0xff;
}

void knot_stop(void)
{

}

s8_t knot_register(u8_t id, const char *name,
		   u16_t type_id, u8_t value_type, u8_t unit,
		   knot_callback_t read_cb, knot_callback_t write_cb)
{
	struct aio *io;

	/* Out of index? */
	if (id >= KNOT_THING_DATA_MAX)
		return -EINVAL;

	/* Assigned already? */
	if (aio[id].id != 0xff)
		return -EACCES;

	/* Basic field validation */
	if (knot_schema_is_valid(type_id, value_type, unit) != 0 || !name)
		return -EINVAL;

	io = &aio[id];

	io->id = id;
	io->schema.type_id = type_id;
	io->schema.unit = unit;
	io->schema.value_type = value_type;
	io->refresh = true;

	strncpy(io->schema.name, name,
		MIN(KNOT_PROTOCOL_DATA_NAME_LEN, strlen(name)));

	/* Set default config */
	io->config.event_flags = KNOT_EVT_FLAG_TIME;
	io->config.time_sec = 30;

	io->read_cb = read_cb;
	io->write_cb = write_cb;

	if (id > last_id || last_id == 0xff)
		last_id = id;

	return 0;
}

const knot_schema *kaio_get_schema(u8_t id)
{
	if (aio[id].id == 0xff)
		return NULL;

	return &(aio[id].schema);
}

u8_t kaio_get_last_id(void)
{
	return last_id;
}

s8_t kaio_read(u8_t id, knot_value_type *value)
{
	struct aio *io;

	if (aio[id].id == 0xff)
		return -EINVAL;

	io = &aio[id];

	if (io->read_cb != NULL)
		if (io->read_cb(id) < 0)
			return -1;

	if (io->refresh == true){
		switch (io->schema.value_type) {
		case KNOT_VALUE_TYPE_INT:
			value->val_i.value = io->value.val_i.value;
			break;
		case KNOT_VALUE_TYPE_FLOAT:
			value->val_f.value_int = io->value.val_f.value_int;
			value->val_f.value_dec = io->value.val_f.value_dec;
			break;
		case KNOT_VALUE_TYPE_BOOL:
			value->val_b = io->value.val_b;
			break;
		case KNOT_VALUE_TYPE_RAW:
			break;
		default:
			return -1;
		}
	} else {
		value = NULL;
	}

	return 0;
}

s8_t kaio_write(u8_t id, knot_value_type *value)
{
	struct aio *io;

	if (aio[id].id == 0xff)
		return -EINVAL;

	io = &aio[id];

	if (io->write_cb == NULL)
		return -1;

	switch (io->schema.value_type) {
	case KNOT_VALUE_TYPE_INT:
		io->value.val_i.value = value->val_i.value;
		io->write_cb(id);
	case KNOT_VALUE_TYPE_FLOAT:
		io->value.val_f.value_int = value->val_f.value_int;
		io->value.val_f.value_dec = value->val_f.value_dec;
		io->write_cb(id);
		break;
	case KNOT_VALUE_TYPE_BOOL:
		io->value.val_b = value->val_b;
		io->write_cb(id);
		break;
	case KNOT_VALUE_TYPE_RAW:
		break;
	default:
		return -1;
	}

	return 0;
}

s8_t kaio_refresh_value(u8_t id)
{
	struct aio *io;

	if (aio[id].id == 0xff)
		return -EINVAL;

	io = &aio[id];

	io->refresh = true;

	return 0;
}
