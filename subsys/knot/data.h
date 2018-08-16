/**
 * Put your licence here
 */

#include <zephyr/types.h>
#include <knot_types.h>
#include <knot_protocol.h>

typedef int (*data_function)		(void *value);

/* Return ammount read or written */
typedef int (*raw_read_function)	(u8_t *buffer, u8_t len);
typedef int (*raw_write_function)	(const u8_t *buffer, u8_t len);

typedef struct __attribute__ ((packed)) {
	data_function 		read;
	data_function 		write;
} data_functions;

typedef struct __attribute__ ((packed)) {
	raw_read_function 	read;
	raw_write_function 	write;
} raw_functions;

typedef union __attribute__ ((packed)) {
	data_functions		data_f;
	raw_functions		raw_f;
} knot_functions;

struct _data_items {
	/* KNoT Identifier */
	u8_t			id;

	/* Node Identifier */
	sys_snode_t		node;

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

	/* Read and Write app functions */
	knot_functions		functions;
};