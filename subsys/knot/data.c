/**
 * Put your licence here
 */

#include <zephyr/types.h>
#include <misc/slist.h>
#include <kernel.h>
#include <knot_types.h>
#include <knot_protocol.h>
#include "main.h"

#define NODE_RX(_node) CONTAINER_OF(_node, struct _data_items, node)

static sys_slist_t data_items_list;
static struct _data_items *last_item;

static struct _data_items *find_data_item(u8_t id)
{
	struct _data_items *item, *aux;

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&data_items_list, item, aux, node)
	{
		if (item->id == id)
			return item;
	}

	return NULL;
}

static int data_function_is_valid(knot_functions *func)
{
	if (func == NULL)
		return -1;

	if (func->data_f.read == NULL && func->data_f.write == NULL)
		return -1;

	return 0;
}

void reset_data_items(void)
{
	struct _data_items *item;
	sys_snode_t *snode;

	//FIXME: Use SYS_SLIST_FOR_EACH_CONTAINER_SAFE macro
	do {
		snode = sys_slist_get(&data_items_list);
		if (snode != NULL){
			item = NODE_RX(snode);
			k_free(item);
		}
	} while (snode != NULL);

	sys_slist_init(&data_items_list);
}

s8_t register_data_item(u8_t id, const char *name,
			u16_t type_id, u8_t value_type,
			u8_t unit, knot_functions *func)
{
	struct _data_items *item = NULL;

	if ((!item) || (knot_schema_is_valid(type_id, value_type, unit) != 0) ||
		name == NULL || (data_function_is_valid(func) != 0))
		return -1;

	item = k_malloc(sizeof(struct _data_items));
	if (item == NULL)
		return -1;

	last_item = item;
	sys_slist_append(&data_items_list, &item->node);

	item->id					= id;
	item->name					= name;
	item->type_id					= type_id;
	item->unit					= unit;
	item->value_type				= value_type;

	/* Set default config */
	item->config.event_flags			= KNOT_EVT_FLAG_TIME;
	item->config.time_sec 				= 30;
	/* As "last_data" is a union, we need just to set the "biggest" member */
	item->last_data.val_f.value_int			= 0;
	item->last_data.val_f.value_dec			= 0;
	/* As "lower_limit" is a union, we need just to set the "biggest" member */
	item->config.lower_limit.val_f.value_int	= 0;
	item->config.lower_limit.val_f.value_dec	= 0;
	/* As "upper_limit" is a union, we need just to set the "biggest" member */
	item->config.upper_limit.val_f.value_int	= 0;
	item->config.upper_limit.val_f.value_dec	= 0;
	/* As "functions" is a union, we need just to set only one of its members */
	item->functions.data_f.read			= func->data_f.read;
	item->functions.data_f.write			= func->data_f.write;
	/* Starting last_timeout with the current time */
	item->last_timeout 				= k_uptime_get();

	return 0;
}

s8_t config_data_item(u8_t id, u8_t evflags, u16_t time_sec,
			knot_value_type *lower, knot_value_type *upper)
{
	struct _data_items *item;

	/*Check if config is valid*/
	if (knot_config_is_valid(evflags, time_sec, lower, upper)
								!= KNOT_SUCCESS)
		return -1;

	item = find_data_item(id);
	if (!item)
		return -1;

	item->config.event_flags = evflags;
	item->config.time_sec = time_sec;

	/*
	 * "lower/upper limit" is a union, we need
	 * just to set the "biggest" member.
	 */

	if (lower)
		memcpy(&(item->config.lower_limit), lower, sizeof(*lower));

	if (upper)
		memcpy(&(item->config.upper_limit), upper, sizeof(*upper));

	return 0;
}
