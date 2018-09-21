/*
 * Copyright (c) 2018, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

void data_items_reset(void);

s8_t data_create_schema(u8_t id, knot_msg_schema *msg);

s8_t data_write_callback(u8_t id, knot_msg_data *data);

s8_t data_read_callback(u8_t id);