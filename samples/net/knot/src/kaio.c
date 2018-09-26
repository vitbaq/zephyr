/*
 * Copyright (c) 2018, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <string.h>

#include "knot_protocol.h"
#include "knot_types.h"
#include "kaio_pdu.h"
#include "kaio.h"

#define KNOT_THING_DATA_MAX    3
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

	kaio_callback_t		read_cb;
	kaio_callback_t		write_cb;
} aio[KNOT_THING_DATA_MAX];

s8_t kaio_register(u8_t id, const char *name,
		   u16_t type_id, u8_t value_type, u8_t unit,
		   kaio_callback_t read_cb, kaio_callback_t write_cb)
{
	struct aio *io;

	/* Assigned already? */
	if (aio[id].id != -1)
		return -EACCES;

	/* Basic field validation */
	if (knot_schema_is_valid(type_id, value_type, unit) != 0 || !name)
		return -EINVAL;

	memset(&aio[id], 0, sizeof(aio[id]));
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

	return 0;
}

s8_t kaio_pdu_create_schema(u8_t id, knot_msg_schema *msg)
{
	struct aio *io;

	if (aio[id].id == -1)
		return -EINVAL;

	io = &aio[id];

	if (id == KNOT_THING_DATA_MAX-1)
		msg->hdr.type = KNOT_MSG_SCHEMA;
	else
		msg->hdr.type = KNOT_MSG_SCHEMA_END;

	msg->sensor_id = id;

	msg->values.value_type = io->schema.value_type;
	msg->values.unit = io->schema.unit;
	/* TODO: missing endianess */
	msg->values.type_id = io->schema.type_id;
	strncpy(msg->values.name, io->schema.name, KNOT_PROTOCOL_DATA_NAME_LEN);

	msg->hdr.payload_len = sizeof(msg->values) + sizeof(msg->sensor_id);

	return 0;
}

static s8_t kaio_read_callback(u8_t id)
{
	if (aio[id].id == -1)
		return -EINVAL;

	if (aio[id].read_cb == NULL)
		return -1;

	if (aio[id].read_cb(id) < 0)
		return -1;

	return 0;
}

s8_t kaio_update_value(u8_t id, knot_value_type *value)
{
	struct aio *io;

	if (aio[id].id == -1)
		return -EINVAL;

	io = &aio[id];

	if (io->write_cb == NULL)
		return -1;

	switch (io->schema.value_type) {
	case KNOT_VALUE_TYPE_INT:
		io->value.val_i.value = value->val_i.value;
	case KNOT_VALUE_TYPE_FLOAT:
		io->value.val_f.value_int = value->val_f.value_int;
		io->value.val_f.value_dec = value->val_f.value_dec;
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

	if (aio[id].id == -1)
		return -EINVAL;

	io = &aio[id];

	io->refresh = true;

	return 0;
}

s8_t kaio_set_value(u8_t id, u8_t value_type, knot_value_type value)
{
	u32_t current_time;
	struct aio *io;
	
	if (aio[id].id == -1)
		return -EINVAL;

	io = &aio[id];

	switch (value_type) {
	case KNOT_VALUE_TYPE_INT:
		if (KNOT_EVT_FLAG_CHANGE & io->config.event_flags
				&& value.val_i.value != io->value.val_i.value)
			io->refresh = true;
		
		if (KNOT_EVT_FLAG_UPPER_THRESHOLD & io->config.event_flags
				&& value.val_i.value > io->value.val_i.value)
			io->refresh = true;
		
		if (KNOT_EVT_FLAG_LOWER_THRESHOLD & io->config.event_flags
				&& value.val_i.value < io->value.val_i.value)
			io->refresh = true;

		if (io->refresh == true)
			io->value.val_i.value = value.val_i.value;

		break;
	case KNOT_VALUE_TYPE_FLOAT:
		if (KNOT_EVT_FLAG_CHANGE & io->config.event_flags
				&& value.val_f.value_int 
				!= io->value.val_f.value_int)
			io->refresh = true;
		
		if (KNOT_EVT_FLAG_UPPER_THRESHOLD & io->config.event_flags
				&& value.val_f.value_int 
				> io->value.val_f.value_int)
			io->refresh = true;
		
		if (KNOT_EVT_FLAG_LOWER_THRESHOLD & io->config.event_flags
				&& value.val_f.value_int 
				< io->value.val_f.value_int)
			io->refresh = true;

		if (io->refresh == true){
			io->value.val_f.value_int = value.val_f.value_int;
			io->value.val_f.value_dec = value.val_f.value_dec;
		}
		break;
	case KNOT_VALUE_TYPE_BOOL:
		if (KNOT_EVT_FLAG_CHANGE & io->config.event_flags 
				&& value.val_b != io->value.val_b) {
			io->refresh = true;
			io->value.val_b = value.val_b;
		}
		break;
	case KNOT_VALUE_TYPE_RAW:
		break;
	default:
		return -1;
	}

	if (KNOT_EVT_FLAG_TIME & io->config.event_flags){
		current_time = k_uptime_get();
		current_time -= io->last_timeout;
		if (current_time >= (io->config.time_sec * 1000))
			io->refresh = true;
	}

	return 0;
}

s8_t data_get_value(u8_t id, u8_t value_type, knot_value_type *value)
{
	struct aio *io;
	
	if (aio[id].id == -1)
		return -EINVAL;

	io = &aio[id];

	switch (value_type) {
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
	return 0;
}