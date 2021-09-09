/*
 * Copyright (C) 2016 BlueKitchen GmbH
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

#define BTSTACK_FILE__ "btstack_uart_rtthread.c"

/*
 *  btstack_uart_rtthread.c
 *
 *  Common code to access serial port via asynchronous block read/write commands
 *
 */

#include "btstack_uart.h"
#include "btstack_run_loop.h"
#include "btstack_debug.h"
#include "btstack_run_loop_rtthread.h"

#include <rtthread.h>
#include <rtdevice.h>

enum {
    CONFIG_PARITY = 0,
    CONFIG_BAUDRATE,
    CONFIG_FLOWCONTROL
};

// btstack uart config
static const btstack_uart_config_t * uart_config;

// data source for integration with BTstack Runloop
static btstack_data_source_t write_data_source;
static btstack_data_source_t read_data_source;

// rtthread uart config
static struct serial_configure rt_uart_config = RT_SERIAL_CONFIG_DEFAULT;

// block write
static int        btstack_uart_block_write_bytes_len;
static const uint8_t * btstack_uart_block_write_bytes_data;
static int             btstack_uart_block_writing;

// block read
static uint16_t        btstack_uart_block_read_bytes_len;
static uint8_t * btstack_uart_block_read_bytes_data;
static int             btstack_uart_block_reading;

// callbacks
static void (*block_sent)(void);
static void (*block_received)(void);

static void uart_write_poll_process(btstack_data_source_t *ds, btstack_data_source_callback_type_t callback_type);
static void uart_read_poll_process(btstack_data_source_t *ds, btstack_data_source_callback_type_t callback_type);

static int btstack_uart_rtthread_init(const btstack_uart_config_t * config) {
    uart_config = config;
    return 0;
}

static void btstack_uart_rtthread_set_block_received( void (*block_handler)(void)){
    block_received = block_handler;
}

static void btstack_uart_rtthread_set_block_sent( void (*block_handler)(void)){
    block_sent = block_handler;
}

static void btstack_uart_rtthread_set_parity_option(const int parity) {
    switch (parity) {
        case BTSTACK_UART_PARITY_OFF:
            rt_uart_config.parity = PARITY_NONE;
            break;
        case BTSTACK_UART_PARITY_EVEN:
            rt_uart_config.parity = PARITY_EVEN;
            break;
        case BTSTACK_UART_PARITY_ODD:
            rt_uart_config.parity = PARITY_ODD;
            break;
        default:
            break;
    }
}

static int btstack_uart_rtthread_set_baudrate_option(const uint32_t baudrate) {
    switch (baudrate) {
        case   9600:  rt_uart_config.baud_rate = BAUD_RATE_9600;    break;
        case  19200:  rt_uart_config.baud_rate = BAUD_RATE_19200;   break;
        case  38400:  rt_uart_config.baud_rate = BAUD_RATE_38400;   break;
        case  57600:  rt_uart_config.baud_rate = BAUD_RATE_57600;   break;
        case 115200:  rt_uart_config.baud_rate = BAUD_RATE_115200;  break;
        case 230400:  rt_uart_config.baud_rate = BAUD_RATE_230400;  break;
        case 460800:  rt_uart_config.baud_rate = BAUD_RATE_460800;  break;
        case 921600:  rt_uart_config.baud_rate = BAUD_RATE_921600;  break;
        default:
            log_error("Baudrate %d is not support now", (int)baudrate);
            return -1;
    }

    return 0;
}

static void btstack_uart_rtthread_set_flowcontrol_option(const int flowcontrol) {
    // Flowcontrol unsupported now
    UNUSED(flowcontrol);
}

static int btstack_uart_rtthread_set(int config, void *data)
{
    int err = 0;

    switch (config) {
        case CONFIG_PARITY:
            btstack_uart_rtthread_set_parity_option(*(int *)data);
            break;
        case CONFIG_BAUDRATE:
            err = btstack_uart_rtthread_set_baudrate_option(*(uint32_t *)data);
            if (err)
                return -1;
            break;
        case CONFIG_FLOWCONTROL:
            btstack_uart_rtthread_set_flowcontrol_option(*(int *)data);
            break;
        default:
            return -1;
    }

    rt_device_t uart_device = write_data_source.source.handle;
    RT_ASSERT(uart_device != NULL);

    err = rt_device_control(uart_device, RT_DEVICE_CTRL_CONFIG, &rt_uart_config);
    if (err != RT_EOK) {
        log_error("Unable to configure device %s", uart_device->parent.name);
        return -1;
    }

    return 0;
}

static int btstack_uart_rtthread_open(void) {

    const char * device_name = uart_config->device_name;
    const uint32_t baudrate  = uart_config->baudrate;
    const int flowcontrol    = uart_config->flowcontrol;
    const int parity         = uart_config->parity;

    rt_device_t uart_device = rt_device_find(device_name);
    if (uart_device == RT_NULL) {
        log_error("Unable to find device %s", device_name);
        return -1;
    }

    rt_err_t err = rt_device_open(uart_device, RT_DEVICE_FLAG_INT_RX);
    if (err != RT_EOK) {
        log_error("Unable to open device %s", device_name);
        return -1;
    }

    btstack_uart_rtthread_set_parity_option(parity);
    btstack_uart_rtthread_set_flowcontrol_option(flowcontrol);

    if (btstack_uart_rtthread_set_baudrate_option(baudrate) < 0) {
        return -1;
    }

    err = rt_device_control(uart_device, RT_DEVICE_CTRL_CONFIG, &rt_uart_config);
    if (err != RT_EOK) {
        log_error("Unable to configure device %s", device_name);
        return -1;
    }

    log_info("Open device %s success", device_name);

    // set up data_source
    btstack_run_loop_set_data_source_handle(&write_data_source, uart_device);
    btstack_run_loop_set_data_source_handler(&write_data_source, &uart_write_poll_process);
    btstack_run_loop_add_data_source(&write_data_source);

    btstack_run_loop_set_data_source_handle(&read_data_source, uart_device);
    btstack_run_loop_set_data_source_handler(&read_data_source, &uart_read_poll_process);
    btstack_run_loop_add_data_source(&read_data_source);

    // wait a bit - at least cheap FTDI232 clones might send the first byte out incorrectly
    rt_thread_mdelay(100);

    return 0;
}

static int btstack_uart_rtthread_close_new(void) {
    // first remove run loop handler
    btstack_run_loop_remove_data_source(&write_data_source);
    btstack_run_loop_remove_data_source(&read_data_source);
 
    // then close device
    rt_device_close((rt_device_t)write_data_source.source.handle);
    write_data_source.source.handle = NULL;
    read_data_source.source.handle = NULL;
    return 0;
}

static int btstack_uart_rtthread_set_baudrate(uint32_t baudrate) {
    return btstack_uart_rtthread_set(CONFIG_BAUDRATE, &baudrate);
}

static int btstack_uart_rtthread_set_parity(int parity) {
    return btstack_uart_rtthread_set(CONFIG_PARITY, &parity);
}

static int btstack_uart_rtthread_set_flowcontrol(int flowcontrol) {
    return btstack_uart_rtthread_set(CONFIG_FLOWCONTROL, &flowcontrol);
}

static void btstack_uart_rtthread_send_block(const uint8_t *data, uint16_t size){
    // setup async write
    btstack_uart_block_write_bytes_data = data;
    btstack_uart_block_write_bytes_len  = size;
    btstack_uart_block_writing = 1;
    btstack_run_loop_enable_data_source_callbacks(&write_data_source, DATA_SOURCE_CALLBACK_POLL);
    btstack_run_loop_rtthread_trigger();
}

static void btstack_uart_rtthread_receive_block(uint8_t *buffer, uint16_t len){
    btstack_uart_block_read_bytes_data = buffer;
    btstack_uart_block_read_bytes_len = len;
    btstack_uart_block_reading = 1;
    btstack_run_loop_enable_data_source_callbacks(&read_data_source, DATA_SOURCE_CALLBACK_POLL);
    btstack_run_loop_rtthread_trigger();
}

static void uart_write_poll_process(btstack_data_source_t *ds, btstack_data_source_callback_type_t callback_type)
{
    if (callback_type != DATA_SOURCE_CALLBACK_POLL)
        return ;
    
    rt_device_t uart_device = (rt_device_t) ds->source.handle;

    size_t bytes_written = rt_device_write(uart_device, 0, btstack_uart_block_write_bytes_data, btstack_uart_block_write_bytes_len);
    if (bytes_written == 0){
        log_error("wrote zero bytes\n");
    }

    btstack_uart_block_write_bytes_data += bytes_written;
    btstack_uart_block_write_bytes_len  -= bytes_written;

    if (btstack_uart_block_write_bytes_len) {
        btstack_run_loop_rtthread_trigger();
        return ;
    }

    btstack_run_loop_disable_data_source_callbacks(ds, DATA_SOURCE_CALLBACK_POLL);

    // notify done
    if (block_sent) {
        block_sent();
    }
}

static void uart_read_poll_process(btstack_data_source_t *ds, btstack_data_source_callback_type_t callback_type)
{
    if (callback_type != DATA_SOURCE_CALLBACK_POLL)
        return ;

    if (btstack_uart_block_read_bytes_len == 0) {
        log_info("called but no read pending");
        btstack_run_loop_disable_data_source_callbacks(ds, DATA_SOURCE_CALLBACK_POLL);
    }
    
    rt_device_t uart_device = (rt_device_t) ds->source.handle;

    size_t bytes_read = rt_device_read(uart_device, 0, btstack_uart_block_read_bytes_data, btstack_uart_block_read_bytes_len);
    if (bytes_read == 0){
        // log_error("read zero bytes\n");
    }

    btstack_uart_block_read_bytes_len   -= bytes_read;
    btstack_uart_block_read_bytes_data  += bytes_read;
    if (btstack_uart_block_read_bytes_len > 0) {
        btstack_run_loop_rtthread_trigger();
        return;
    }

    btstack_run_loop_disable_data_source_callbacks(ds, DATA_SOURCE_CALLBACK_POLL);

    if (block_received) {
        block_received();
    }
}

static const btstack_uart_t btstack_uart_rtthread = {
    /* int  (*init)(btstack_uart_config_t * config); */                    &btstack_uart_rtthread_init,
    /* int  (*open)(void); */                                              &btstack_uart_rtthread_open,
    /* int  (*close)(void); */                                             &btstack_uart_rtthread_close_new,
    /* void (*set_block_received)(void (*handler)(void)); */               &btstack_uart_rtthread_set_block_received,
    /* void (*set_block_sent)(void (*handler)(void)); */                   &btstack_uart_rtthread_set_block_sent,
    /* int  (*set_baudrate)(uint32_t baudrate); */                         &btstack_uart_rtthread_set_baudrate,
    /* int  (*set_parity)(int parity); */                                  &btstack_uart_rtthread_set_parity,
    /* int  (*set_flowcontrol)(int flowcontrol); */                        &btstack_uart_rtthread_set_flowcontrol,
    /* void (*receive_block)(uint8_t *buffer, uint16_t len); */            &btstack_uart_rtthread_receive_block,
    /* void (*send_block)(const uint8_t *buffer, uint16_t length); */      &btstack_uart_rtthread_send_block,

    /* int  (*get_supported_sleep_modes); */                               NULL,
    /* void (*set_sleep)(btstack_uart_sleep_mode_t sleep_mode); */         NULL,
    /* void (*set_wakeup_handler)(void (*handler)(void)); */               NULL,
    /* void (*set_frame_received)(void (*handler)(uint16_t frame_size); */ NULL,
    /* void (*set_fraae_sent)(void (*handler)(void)); */                   NULL,
    /* void (*receive_frame)(uint8_t *buffer, uint16_t len); */            NULL,
    /* void (*send_frame)(const uint8_t *buffer, uint16_t length); */      NULL,

};

const btstack_uart_t * btstack_uart_rtthread_instance(void){
	return &btstack_uart_rtthread;
}