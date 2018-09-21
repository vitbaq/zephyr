/*
 * Copyright (c) 2018, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <string.h>

#include "knot_types.h"

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

s8_t data_item_register(u8_t id, const char *name,
			u16_t type_id, u8_t value_type, u8_t unit,
			knot_callback read, knot_callback write)
{
	if ((data_items[id].id == -1) ||
		(knot_schema_is_valid(type_id, value_type, unit) != 0) ||
		name == NULL)
		return -1;

	data_items[id].id = id;
	data_items[id].name = name;
	data_items[id].type_id = type_id;
	data_items[id].unit = unit;
	data_items[id].value_type = value_type;

	data_items[id].new_value = false;

	/* Set default config */
	data_items[id].config.event_flags = KNOT_EVT_FLAG_TIME;
	data_items[id].config.time_sec = 30;
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

	/* Starting last_timeout with the current time */
	data_items[id].last_timeout = k_uptime_get();

	data_items[id].read = read;
	data_items[id].write = write;

	return 0;
}

s8_t data_item_config(u8_t id, u8_t evflags, u16_t time_sec,
			knot_value_type *lower, knot_value_type *upper)
{
	/*Check if config is valid*/
	if (knot_config_is_valid(evflags, time_sec, lower, upper)
								!= KNOT_SUCCESS)
		return -1;

	if (data_items[id].id == -1)
		return -1;

	data_items[id].config.event_flags = evflags;
	data_items[id].config.time_sec = time_sec;

	/*
	 * "lower/upper limit" is a union, we need
	 * just to set the "biggest" member.
	 */

	if (lower)
		memcpy(&(data_items[id].config.lower_limit), 
							lower, sizeof(*lower));

	if (upper)
		memcpy(&(data_items[id].config.upper_limit), 
							upper, sizeof(*upper));

	return 0;
}

s8_t data_create_schema(u8_t id, knot_msg_schema *msg)
{
	if (id == KNOT_THING_DATA_MAX-1)
		msg->hdr.type = KNOT_MSG_SCHEMA_END;
	else
		msg->hdr.type = KNOT_MSG_SCHEMA;

	msg->sensor_id = id;
	msg->values.value_type = data_items[id].value_type;
	msg->values.unit = data_items[id].unit;
	msg->values.type_id = data_items[id].type_id;
	strncpy(msg->values.name, data_items[id].name, 
						sizeof(msg->values.name));

	msg->hdr.payload_len = sizeof(msg->values) + sizeof(msg->sensor_id);

	return 0;
}

s8_t data_write_callback(u8_t id, knot_msg_data *data)
{
	int ret = -1;

	if (data_items[id].write == NULL)
		return ret;

	switch (data_items[id].value_type) {
	case KNOT_VALUE_TYPE_INT:
	case KNOT_VALUE_TYPE_FLOAT:
	case KNOT_VALUE_TYPE_BOOL:
		data_items[id].last_value = data->payload;
		data_items[id].write(id);
		break;
	case KNOT_VALUE_TYPE_RAW:
		break;
	default:
		return -1;
	}

	return 0;
}

s8_t data_read_callback(u8_t id)
{
	if (data_items[id].id == -1)
		return -1;

	if (data_items[id].read == NULL)
		return -1;

	if (data_items[id].read(id) < 0)
		return -1;

	return 0;
}

s8_t data_verify_value(knot_msg_data *data)
{
	static u8_t id = 0;

	data_read_callback(id);
	if (data_items[id].new_value == true){

		data->hdr.type = KNOT_MSG_DATA;
		data->sensor_id = id;
		data->hdr.payload_len = sizeof(data->sensor_id);

		switch (data_items[id].value_type) {
		case KNOT_VALUE_TYPE_INT:
			data->payload.val_i.value =
				data_items[id].last_value.val_i.value;
			data->hdr.payload_len += sizeof(knot_value_type_int);
			break;
		case KNOT_VALUE_TYPE_FLOAT:
			data->payload.val_f.value_int =
				data_items[id].last_value.val_f.value_int;
			data->payload.val_f.value_dec =
				data_items[id].last_value.val_f.value_dec;
			data->hdr.payload_len += sizeof(knot_value_type_float);
			break;
		case KNOT_VALUE_TYPE_BOOL:
			data->payload.val_b = data_items[id].last_value.val_b;
			data->hdr.payload_len += sizeof(knot_value_type_bool);
			break;
		case KNOT_VALUE_TYPE_RAW:
			break;
		default:
			return -1;
		}

		data_items[id].new_value = false;
	}

	id++;
	if (id > KNOT_THING_DATA_MAX)
		id = 0;

	return 0;
}

s8_t data_set_value(u8_t id, u8_t value_type, knot_value_type value)
{
	u32_t current_time;

	switch (value_type) {
	case KNOT_VALUE_TYPE_INT:
		if (KNOT_EVT_FLAG_UPPER_THRESHOLD &
			data_items[id].config.event_flags
			&& value.val_i.value >
			data_items[id].last_value.val_i.value)
			data_items[id].new_value = true;

		if (KNOT_EVT_FLAG_LOWER_THRESHOLD &
		data_items[id].config.event_flags
		&& value.val_i.value <
		data_items[id].last_value.val_i.value)
			data_items[id].new_value = true;

		if (value.val_i.value != data_items[id].last_value.val_i.value){
			data_items[id].last_value.val_i.value =
							value.val_i.value;
			if (KNOT_EVT_FLAG_CHANGE &
				data_items[id].config.event_flags)
				data_items[id].new_value = true;
		}
		break;
	case KNOT_VALUE_TYPE_FLOAT:
		if (KNOT_EVT_FLAG_UPPER_THRESHOLD &
		data_items[id].config.event_flags
		&& value.val_f.value_int >
		data_items[id].last_value.val_f.value_int)
			data_items[id].new_value = true;

		if (KNOT_EVT_FLAG_LOWER_THRESHOLD &
		data_items[id].config.event_flags
		&& value.val_f.value_int <
		data_items[id].last_value.val_f.value_int)
			data_items[id].new_value = true;

		if (value.val_f.value_int !=
			data_items[id].last_value.val_f.value_int){
			data_items[id].last_value.val_f.value_int =
							value.val_f.value_int;
			data_items[id].last_value.val_f.value_dec =
							value.val_f.value_dec;
			if (KNOT_EVT_FLAG_CHANGE &
			data_items[id].config.event_flags)
				data_items[id].new_value = true;
		}
		break;
	case KNOT_VALUE_TYPE_BOOL:
		if (value.val_b != data_items[id].last_value.val_b){
			data_items[id].last_value.val_b = value.val_b;
			if (KNOT_EVT_FLAG_CHANGE &
			data_items[id].config.event_flags)
				data_items[id].new_value = true;
		}
		break;
	case KNOT_VALUE_TYPE_RAW:
		break;
	default:
		return -1;
	}

	if (KNOT_EVT_FLAG_TIME & data_items[id].config.event_flags){
		current_time = k_uptime_get();
		current_time -= data_items[id].last_timeout;
		if (current_time >= (data_items[id].config.time_sec * 1000))
			data_items[id].new_value = true;
	}

	return 0;
}