/**
 * Put your licence here
 */

#include <zephyr/types.h>
#include <knot_types.h>
#include <knot_protocol.h>

struct _data_items {
	/* KNoT Identifier */
	u8_t			id;

	/* Schema Values */
	u8_t			value_type;	// KNOT_VALUE_TYPE_* (int, float, bool, raw)
	u8_t			unit;		// KNOT_UNIT_*
	u16_t			type_id;	// KNOT_TYPE_ID_*
	const char		*name;		// App defined data item name

	/* Control the upper lower message flow */
	u8_t lower_flag;
	u8_t upper_flag;

	/* Data Values */
	knot_value_type		last_data;

	/* Config Values */
	knot_config		config;	// Flags indicating when data will be sent

	/* Time Values */
	s64_t			last_timeout;	// Stores the last time the data was sent
};