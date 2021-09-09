/*
 * Copyright (C) 2017 BlueKitchen GmbH
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

#define BTSTACK_FILE__ "btstack_run_loop_rtthread.c"

#include "btstack_run_loop_rtthread.h"

#include "btstack_linked_list.h"
#include "btstack_debug.h"
#include "btstack_util.h"

#include <stddef.h> // NULL

#include <rtthread.h>

#define RUN_LOOP_QUEUE_LENGTH       5
#define RUN_LOOP_QUEUE_ITEM_SIZE    sizeof(function_call_t)

// bit 0 event group reserved to wakeup run loop
#define EVENT_GROUP_FLAG_RUN_LOOP (1 << 0)

typedef struct function_call {
    void (*fn)(void * arg);
    void * arg;
} function_call_t;

static rt_mq_t      btstack_run_loop_queue;
static rt_event_t   btstack_run_loop_event_group;
static rt_thread_t  btstack_run_loop_task;

static uint32_t btstack_run_loop_rtthread_get_time_ms(void) {
    btstack_assert(RT_TICK_PER_SECOND >= 1000);

    rt_tick_t ticks = rt_tick_get();
    uint32_t ms = ticks / (RT_TICK_PER_SECOND / 1000);
    return ms;
}

// set timer
static void btstack_run_loop_rtthread_set_timer(btstack_timer_source_t *ts, uint32_t timeout_in_ms){
    ts->timeout = btstack_run_loop_rtthread_get_time_ms() + timeout_in_ms + 1;
}

static void btstack_run_loop_rtthread_init(void) {
    btstack_run_loop_base_init();

    btstack_run_loop_queue = rt_mq_create("btstack.run.loop.queue", RUN_LOOP_QUEUE_ITEM_SIZE, RUN_LOOP_QUEUE_LENGTH, RT_IPC_FLAG_PRIO);
    btstack_assert(btstack_run_loop_queue != NULL);

    btstack_run_loop_event_group = rt_event_create("btstack.run.loop.event", RT_IPC_FLAG_PRIO);
    btstack_assert(btstack_run_loop_event_group != NULL);

    btstack_run_loop_task = rt_thread_self();
    log_info("run loop init, task %p, queue item size %u", btstack_run_loop_task, (int) sizeof(function_call_t));
}

/**
 * Execute run_loop
 */
static void btstack_run_loop_rtthread_execute(void) {
    log_debug("RL: execute");

    while (1) {

        // process data sources
        btstack_run_loop_base_poll_data_sources();

        // process registered function calls on run loop thread
        while (1) {
            function_call_t message = { NULL, NULL };
            rt_err_t err = rt_mq_recv(btstack_run_loop_queue, &message, sizeof(message), RT_WAITING_NO);
            if (err != RT_EOK)  break;
            if (message.fn) {
                message.fn(message.arg);
            }
        }

        // process timers
        uint32_t now = btstack_run_loop_rtthread_get_time_ms();
        btstack_run_loop_base_process_timers(now);

        // wait for timeout or event group/task notification
        int32_t timeout_next_timer_ms = btstack_run_loop_base_get_time_until_timeout(now);

        int timeout_ms = RT_WAITING_FOREVER;
        if (timeout_next_timer_ms >= 0) {
            timeout_ms = timeout_next_timer_ms;
        }
        
        log_debug("RL: wait with timeout %u", (int) timeout_ms);

        rt_event_recv(btstack_run_loop_event_group, EVENT_GROUP_FLAG_RUN_LOOP, RT_EVENT_FLAG_CLEAR | RT_EVENT_FLAG_OR, timeout_ms, NULL);
    }
}

// schedules execution from regular thread
void btstack_run_loop_rtthread_trigger(void) {
    rt_event_send(btstack_run_loop_event_group, EVENT_GROUP_FLAG_RUN_LOOP);
}

void btstack_run_loop_rtthread_execute_code_on_main_thread(void (*fn)(void *arg), void * arg) {
    
    // directly call function if already on btstack task
    if (rt_thread_self() == btstack_run_loop_task) {
        (*fn)(arg);
        return ;
    }

    function_call_t message;
    message.fn  = fn;
    message.arg = arg;
    rt_err_t err = rt_mq_send(btstack_run_loop_queue, &message, sizeof(message));
    if (err != RT_EOK) {
        log_error("Failed to post fn %p", fn);
    }
    
    btstack_run_loop_rtthread_trigger();
}


static const btstack_run_loop_t btstack_run_loop_rtthread = {
    &btstack_run_loop_rtthread_init,
    &btstack_run_loop_base_add_data_source,
    &btstack_run_loop_base_remove_data_source,
    &btstack_run_loop_base_enable_data_source_callbacks,
    &btstack_run_loop_base_disable_data_source_callbacks,
    &btstack_run_loop_rtthread_set_timer,
    &btstack_run_loop_base_add_timer,
    &btstack_run_loop_base_remove_timer,
    &btstack_run_loop_rtthread_execute,
    &btstack_run_loop_base_dump_timer,
    &btstack_run_loop_rtthread_get_time_ms,
};

const btstack_run_loop_t * btstack_run_loop_rtthread_get_instance(void){
    return &btstack_run_loop_rtthread;
}
