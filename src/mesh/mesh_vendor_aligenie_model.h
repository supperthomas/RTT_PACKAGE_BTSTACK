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

#ifndef __MESH_VENDOR_ALIGENIE_NODEL_H
#define __MESH_VENDOR_ALIGENIE_NODEL_H

#include "btstack_linked_list.h"
#include "btstack_ring_buffer.h"
#include "mesh/mesh_access.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define MESH_VENDOR_ALIGENIE_ATTR_GET                   0xD001A8u
#define MESH_VENDOR_ALIGENIE_ATTR_SET                   0xD101A8u
#define MESH_VENDOR_ALIGENIE_ATTR_SET_UNACKNOWLEDGED    0xD201A8u
#define MESH_VENDOR_ALIGENIE_ATTR_STATUS                0xD301A8u
#define MESH_VENDOR_ALIGENIE_ATTR_INDICATION            0xD401A8u
#define MESH_VENDOR_ALIGENIE_ATTR_CONFIRMATION          0xD501A8u
#define MESH_VENDOR_ALIGENIE_TRANSPARENT_MSG            0xCF01A8u



/* Attr type */
#define ALIGENIE_ERROR_CODE                     0x0000u
#define ALIGENIE_ATTR_COLOR                     0x0123u
#define ALIGENIE_ATTR_ONOFF                     0x0100u
#define ALIGENIE_ATTR_LIGHTNESS_LEVEL           0x0121u
#define ALIGENIE_ATTR_COLOR_TEMPERATURE         0x0122u
#define ALIGENIE_ATTR_MAIN_LIGHT_ONOFF          0x0534u
#define ALIGENIE_ATTR_BACKGROUND_LIGHT_ONOFF    0x0533u
#define ALIGENIE_ATTR_MODE_NUMBER               0xF004u
#define ALIGENIE_ATTR_EVENT                     0xF009u

/**
 * @format 11414
 * @param subevent_code
 * @param element_index
 * @param model_identifier
 * @param reason
 * @param attr_point
 */
#define MESH_SUBEVENT_VENDOR_ALIGENIE       0xFF

/*
static uint8_t  vendor_onoff;
static uint8_t  vendor_color[3];
static uint16_t vendor_lightness_level;

static mesh_model_t vendor_aligenie_server_model;

mesh_vendor_aligenie_attr_t attr_array[] =
{
    {ALIGENIE_ATTR_ONOFF,           &vendor_onoff,           1,     "1"     },
    {ALIGENIE_ATTR_COLOR,           &vendor_color[0],        6,     "222"   },
    {ALIGENIE_ATTR_LIGHTNESS_LEVEL, &vendor_lightness_level, 2,     "2"     },
    {0, NULL, 0, ""}
};

mesh_vendor_aligenie_server_register_attrs(&attr_array[0]);

*/

enum {
    VENDOR_ALIGENIE_DEVICE_NOT_READY   = 0x80,
    VENDOR_ALIGENIE_ATTR_NOT_SUPPORT   = 0x81,
    VENDOR_ALIGENIE_OPCODE_NOT_SUPPORT = 0x82,
    VENDOR_ALIGENIE_PARAMETER_ERROR    = 0x83,
    VENDOR_ALIGENIE_DEVICE_STATE_ERROR = 0x84,
    VENDOR_ALIGENIE_NOT_FIND_INDEX     = 0x85,
    VENDOR_ALIGENIE_STORAGE_FULL       = 0x86,
    VENDOR_ALIGENIE_FORMAT_ERROR       = 0x87,
};

typedef struct {
    const uint16_t  attr_type;
    void  *         attr_value;
    const uint8_t   attr_value_length;
    const char *    format;
} mesh_vendor_aligenie_attr_t;

typedef struct aligenie_attr_visited_error {
    uint8_t     error_code;
    uint16_t    attr_type;
} aligenie_attr_visited_error_t;

typedef struct {
    mesh_transition_t base_transition;

    btstack_ring_buffer_t attrs_visited;
    btstack_ring_buffer_t state_errors;
    mesh_vendor_aligenie_attr_t *attrs;
} mesh_vendor_aligenie_state_t;

static inline mesh_vendor_aligenie_attr_report_error(mesh_vendor_aligenie_state_t * state, uint16_t attr_type, uint8_t error_code)
{
    aligenie_attr_visited_error_t error = {
        .attr_type = attr_type,
        .error_code = error_code
    };
    btstack_ring_buffer_write(&state->state_errors, &error, sizeof(error));
}

static inline mesh_subevent_vendor_aligenie_get_element_index(const uint8_t * event)
{
    return event[3];
}

static inline mesh_subevent_vendor_aligenie_get_model_identifier(const uint8_t * event)
{
    return little_endian_read_32(event, 4);
}

static inline mesh_subevent_vendor_aligenie_get_reason(const uint8_t * event)
{
    return event[8];
}

static inline mesh_subevent_vendor_aligenie_get_attr_pointer(const uint8_t * event)
{
    return little_endian_read_32(event, 9);
}

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* __MESH_VENDOR_ALIGENIE_NODEL_H */
