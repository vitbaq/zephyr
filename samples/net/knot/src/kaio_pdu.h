/*
 * Copyright (c) 2018, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

s8_t kaio_pdu_create_schema(u8_t id, knot_msg_schema *msg);

s8_t kaio_update_value(u8_t id, knot_value_type *value);

s8_t kaio_refresh_value(u8_t id);
