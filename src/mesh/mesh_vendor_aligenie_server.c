/*
 * Copyright (C) 2019 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

#define BTSTACK_FILE__ "mesh_vendor_aligenie_server.c"

#include <stdio.h>

#include "btstack_debug.h"
#include "btstack_util.h"

#include "mesh/mesh_vendor_aligenie_server.h"
#include "mesh/mesh_vendor_aligenie_model.h"
#include "mesh/mesh_upper_transport.h"
#include "mesh/mesh_foundation.h"
#include "mesh/mesh_access.h"

static void mesh_access_emit_state_update_aligenie(mesh_model_t * model, model_state_update_reason_t reason)
{
    btstack_assert(model != NULL);

    if (model->model_packet_handler == NULL) 
        return ;

    mesh_vendor_aligenie_state_t * state = (mesh_vendor_aligenie_state_t *)model->model_data;
    uint8_t event[12] = {HCI_EVENT_MESH_META, 10, MESH_SUBEVENT_VENDOR_ALIGENIE};
    uint32_t bytes_available = btstack_ring_buffer_bytes_available(&state->attrs_visited);
    uint32_t bytes_read = 0;
    uint32_t dummy_read;
    mesh_vendor_aligenie_attr_t * attr;

    while (bytes_read < bytes_available) {
        // Get attr from ringbuffer, and then put it in again.
        btstack_ring_buffer_read(&state->attrs_visited, &attr, sizeof(attr), &dummy_read);
        bytes_read += dummy_read;
        btstack_ring_buffer_write(&state->attrs_visited, &attr, sizeof(attr));

        uint8_t pos = 3;
        event[pos++] = mesh_access_get_element_index(model);
        little_endian_store_32(event, pos, model->model_identifier);
        pos += 4;
        event[pos++] = (uint8_t)reason;
        little_endian_store_32(event, pos, attr);
        pos += 4;
        (*model->model_packet_handler)(HCI_EVENT_PACKET, 0, event, sizeof(event));
    }
}

static void mesh_vendor_aligenie_server_transition_handler(mesh_transition_t * base_transition, model_state_update_reason_t event)
{
    mesh_vendor_aligenie_state_t * state = (mesh_vendor_aligenie_state_t*) base_transition;
    switch (event) {
        case MODEL_STATE_UPDATE_REASON_SET:
            mesh_access_state_changed(state->base_transition.mesh_model);
            break;
        default:
            btstack_assert(false);
            break;
    }

    // notify app
    mesh_model_t * model = state->base_transition.mesh_model;
    mesh_access_emit_state_update_aligenie(model, event);
}

static void vendor_aligenie_server_send_message(uint16_t src, uint16_t dest, uint16_t netkey_index, uint16_t appkey_index, mesh_pdu_t *pdu)
{
    uint8_t  ttl  = mesh_foundation_default_ttl_get();
    mesh_upper_transport_setup_access_pdu_header(pdu, netkey_index, appkey_index, ttl, src, dest, 0);
    mesh_access_send_unacknowledged_pdu(pdu);
}

static mesh_vendor_aligenie_attr_t * vendor_aligenie_find_attr_by_type(const mesh_vendor_aligenie_state_t * vendor_aligenie_state, const uint16_t attr_type)
{
    // find attr item in table
    mesh_vendor_aligenie_attr_t * attr = vendor_aligenie_state->attrs;
    btstack_assert(attr != NULL);

    for (; attr->attr_value != NULL; attr++) {
        if (attr->attr_type == attr_type)
            break;
    }

    // If attribute type is not support, return end of entry {0, NULL, 0, ""}.
    return attr;
}

static mesh_transition_t * vendor_aligenie_server_get_base_transition(mesh_model_t * mesh_model)
{
    mesh_vendor_aligenie_state_t * vendor_aligenie_server_state = (mesh_vendor_aligenie_state_t *)mesh_model->model_data;
    return &vendor_aligenie_server_state->base_transition;
}

static void mesh_access_parser_aligenie_attr_value(mesh_access_parser_state_t * parser, mesh_vendor_aligenie_attr_t * attr)
{
    const char * format = attr->format;
    uint8_t * attr_value = (uint8_t *)attr->attr_value;
    uint16_t position = 0;

    uint8_t  byte;
    uint16_t word;
    uint32_t longword;
    while (*format) {
        switch (*format) {
            case '1':
                byte = mesh_access_parser_get_uint8(parser);
                attr_value[position] = byte;
                position += 1;
                break;
            case '2':
                word = mesh_access_parser_get_uint16(parser);
                little_endian_store_16(attr_value, position, word);
                position += 2;
                break;
            case '3':
                longword = mesh_access_parser_get_uint24(parser);
                little_endian_store_24(attr_value, position, longword);
                position += 3;
                break;
            case '4':
                longword = mesh_access_parser_get_uint32(parser);
                little_endian_store_32(attr_value, position, longword);
                position += 4;
                break;
            default:
                btstack_assert(false);
                break;
        }
        format++;
    }
}

static void vendor_aligenie_handle_get_or_set_message(mesh_model_t *mesh_model, mesh_pdu_t *pdu, bool set)
{
    btstack_assert(mesh_model != NULL);
    btstack_assert(mesh_model->model_data != NULL);

    mesh_access_parser_state_t parser;
    mesh_access_parser_init(&parser, (mesh_pdu_t*) pdu);
    
    // The TID field is a transaction identifier indicating whether the message is 
    // a new message or a retransmission of a previously sent message
    uint8_t tid = mesh_access_parser_get_uint8(&parser);
    mesh_vendor_aligenie_state_t * vendor_aligenie_server_state = (mesh_vendor_aligenie_state_t *)mesh_model->model_data;

    mesh_transition_t * base_transition = vendor_aligenie_server_get_base_transition(mesh_model);
    switch (mesh_access_transitions_transaction_status(base_transition, tid, mesh_pdu_src(pdu), mesh_pdu_dst(pdu))) {
        case MESH_TRANSACTION_STATUS_RETRANSMISSION:
            // ignore on retransmission
            break;
        default:
            while (mesh_access_parser_available(&parser) >= 2) {
                uint16_t attr_type = mesh_access_parser_get_uint16(&parser);
                mesh_vendor_aligenie_attr_t * attr = (mesh_vendor_aligenie_attr_t *)vendor_aligenie_find_attr_by_type(vendor_aligenie_server_state->attrs, attr_type);

                if (attr->attr_value == NULL) {
                    printf("Attribute type 0x%04x not supported\n", attr_type);
                    mesh_vendor_aligenie_attr_report_error(vendor_aligenie_server_state, attr_type, VENDOR_ALIGENIE_ATTR_NOT_SUPPORT);

                    break;
                }

                // Handle set message.
                if (set) {
                    if (mesh_access_parser_available(&parser) < attr->attr_value_length) {
                        printf("Attribute type 0x%04x value length is error\n", attr_type);
                        mesh_vendor_aligenie_attr_report_error(vendor_aligenie_server_state, attr_type, VENDOR_ALIGENIE_PARAMETER_ERROR);

                        break;
                    }

                    mesh_access_parser_aligenie_attr_value(&parser, attr);
                }

                // Write attribute visited to ringbuffer, wait to send status message.
                btstack_ring_buffer_write(&vendor_aligenie_server_state->attrs_visited, &attr, sizeof(attr));
            }

            mesh_access_transitions_init_transaction(base_transition, tid, mesh_pdu_src(pdu), mesh_pdu_dst(pdu));

            if (set) {
                mesh_access_transition_setup(mesh_model, base_transition, 0, 0, &mesh_vendor_aligenie_server_transition_handler);
                mesh_access_state_changed(mesh_model);
            }

            break;
    }
}

static void mesh_access_message_add_aligenie_attr_value(mesh_upper_transport_builder_t * builder, mesh_vendor_aligenie_attr_t * attr)
{
    const char * format = attr->format;
    uint8_t * attr_value = (uint8_t *)attr->attr_value;
    uint16_t position = 0;

    uint8_t  byte;
    uint16_t word;
    uint32_t longword;
    while (*format) {
        switch(*format) {
            case '1':
                byte = attr_value[position];
                mesh_access_message_add_uint8(builder, byte);
                position += 1;
                break;
            case '2':
                word = little_endian_read_16(attr_value, position);
                mesh_access_message_add_uint16(builder, word);
                position += 2;
                break;
            case '3':
                longword = little_endian_read_24(attr_value, position);
                mesh_access_message_add_uint24(builder, longword);
                position += 3;
                break;
            case '4':
                longword = little_endian_read_32(attr_value, position);
                mesh_access_message_add_uint32(builder, longword);
                position += 4;
                break;
        }
        format++;
    }
}

static mesh_pdu_t * mesh_vendor_aligenie_status_message(mesh_model_t *model)
{
    static uint8_t status_tid = 0;
    uint32_t dummy_read;

    mesh_upper_transport_builder_t builder;
    mesh_access_message_init(&builder, MESH_VENDOR_ALIGENIE_ATTR_STATUS);

    mesh_access_message_add_uint8(&builder, status_tid++);

    mesh_vendor_aligenie_state_t * state = (mesh_vendor_aligenie_state_t *)model->model_data;
    mesh_vendor_aligenie_attr_t * attrs = state->attrs;
    btstack_assert(attrs != NULL);
    
    mesh_vendor_aligenie_attr_t * attr;
    while (!btstack_ring_buffer_empty(&state->attrs_visited)) {
        btstack_ring_buffer_read(&state->attrs_visited, &attr, sizeof(attr), &dummy_read);

        mesh_access_message_add_uint16(&builder, attr->attr_type);
        mesh_access_message_add_aligenie_attr_value(&builder, attr);
    }

    aligenie_attr_visited_error_t error;
    while (!btstack_ring_buffer_empty(&state->state_errors)) {
        btstack_ring_buffer_read(&state->state_errors, &error, sizeof(error), &dummy_read);

        mesh_access_message_add_uint16(&builder, ALIGENIE_ERROR_CODE);
        mesh_access_message_add_uint16(&builder, error.attr_type);
        mesh_access_message_add_uint8(&builder, error.error_code);
    }

    return (mesh_pdu_t *) mesh_access_message_finalize(&builder);
}

// Attr Type: 2 byte,
// Different Attr Type, The Attr Value length ?

// check received message is right, then make reply message, finally send message.
static void vendor_aligenie_attr_get_handler(struct mesh_model * vendor_aligenie_server_model, mesh_pdu_t * pdu)
{
    // 1 TID，N Attr Type (2 bytes every)
    vendor_aligenie_handle_get_or_set_message(vendor_aligenie_server_model, pdu, false);
    
    mesh_upper_transport_pdu_t * transport_pdu = (mesh_upper_transport_pdu_t *)mesh_vendor_aligenie_status_message(vendor_aligenie_server_model);
    if (transport_pdu) {
        vendor_aligenie_server_send_message(mesh_access_get_element_address(vendor_aligenie_server_model), mesh_pdu_src(pdu), mesh_pdu_netkey_index(pdu), mesh_pdu_appkey_index(pdu), (mesh_pdu_t *) transport_pdu);
    }

    mesh_access_message_processed(pdu);
}

static void vendor_aligenie_attr_set_handler(struct mesh_model * vendor_aligenie_server_model, mesh_pdu_t * pdu)
{
    vendor_aligenie_handle_get_or_set_message(vendor_aligenie_server_model, pdu, true);

    mesh_upper_transport_pdu_t * transport_pdu = (mesh_upper_transport_pdu_t *)mesh_vendor_aligenie_status_message(vendor_aligenie_server_model);
    if (transport_pdu) {
        vendor_aligenie_server_send_message(mesh_access_get_element_address(vendor_aligenie_server_model), mesh_pdu_src(pdu), mesh_pdu_netkey_index(pdu), mesh_pdu_appkey_index(pdu), (mesh_pdu_t *) transport_pdu);
    }

    mesh_access_message_processed(pdu);
}

static void vendor_aligenie_attr_set_unacknowledged_handler(struct mesh_model * vendor_aligenie_server_model, mesh_pdu_t * pdu)
{
    vendor_aligenie_handle_get_or_set_message(vendor_aligenie_server_model, pdu, true);
     mesh_access_message_processed(pdu);
}

static void vendor_aligenie_attr_confirmation_handler(struct mesh_model * vendor_aligenie_server_model, mesh_pdu_t * pdu)
{
    
}

// Vendor AliGenie Message
const static mesh_operation_t mesh_vendor_aligenie_model_operations[] = {
    // Message Opcode, Message Parameters minimal length, Operation handle
    { MESH_VENDOR_ALIGENIE_ATTR_GET,                 3,  vendor_aligenie_attr_get_handler},
    { MESH_VENDOR_ALIGENIE_ATTR_SET,                 4,  vendor_aligenie_attr_set_handler},
    { MESH_VENDOR_ALIGENIE_ATTR_SET_UNACKNOWLEDGED,  4,  vendor_aligenie_attr_set_unacknowledged_handler},
    { MESH_VENDOR_ALIGENIE_ATTR_CONFIRMATION,        1,  vendor_aligenie_attr_confirmation_handler},
    { 0, 0, NULL }
};

const mesh_operation_t * mesh_vendor_aligenie_server_get_operations(void)
{
    return mesh_vendor_aligenie_model_operations;
}
