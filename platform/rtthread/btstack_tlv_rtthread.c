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
 *
 * THIS SOFTWARE IS PROVIDED BY MATTHIAS RINGWALD AND CONTRIBUTORS
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
 */

#define BTSTACK_FILE__ "btstack_tlv_rtthread.c"

#include "btstack_tlv.h"
#include "btstack_tlv_rtthread.h"
#include "btstack_debug.h"
#include "btstack_util.h"
#include "btstack_debug.h"

#include <rtthread.h>
#include "easyflash.h"

#define EASYFLASH_KEY_SIZE		15

void tag2key(uint32_t tag, char *key)
{
	rt_snprintf(key, EASYFLASH_KEY_SIZE, "0x%08x", tag);
}

/**
 * Get Value for Tag
 * @param context
 * @param tag
 * @param buffer
 * @param buffer_size
 * @returns size of value
 */
static int btstack_tlv_rtthread_get_tag(void * context, uint32_t tag, uint8_t * buffer, uint32_t buffer_size)
{
	UNUSED(context);
	
	// tag: 0x12345678 - key: "0x12345678"
	char key[EASYFLASH_KEY_SIZE];
	tag2key(tag, key);

	uint32_t bytes_to_read = 0;
	ef_get_env_blob(key, NULL, 0, &bytes_to_read);
	if (bytes_to_read == 0) {
		log_error("get tag %s fail", key);
		return 0;
	}

	// return len if buffer = NULL
	if (!buffer)
		return bytes_to_read;
	
	// otherwise copy data into buffer
	bytes_to_read = btstack_min(bytes_to_read, buffer_size);
	ef_get_env_blob(key, buffer, bytes_to_read, NULL);

	log_info("get tag %s success, value: ", key);
	log_info_hexdump(buffer, bytes_to_read);

	return bytes_to_read;
}

/**
 * Store Tag 
 * @param context
 * @param tag
 * @param data
 * @param data_size
 * @returns 0 on success
 */
static int btstack_tlv_rtthread_store_tag(void * context, uint32_t tag, const uint8_t * data, uint32_t data_size)
{
	UNUSED(context);

	// tag: 0x12345678 - key: "0x12345678"
	char key[EASYFLASH_KEY_SIZE];
	tag2key(tag, key);

	EfErrCode err = ef_set_env_blob(key, data, data_size);
	if (err != EF_NO_ERR) {
		log_error("store tag %s fail, err (%d)", key, err);
		return -1;
	}

	log_info("store tag %s success, value: ", key);
	log_info_hexdump(data, data_size);

	return 0;
}

/**
 * Delete Tag
 *  @note it is not expected that delete operation fails, please use at least log_error in case of errors
 * @param context
 * @param tag
 */
static void btstack_tlv_rtthread_delete_tag(void * context,  uint32_t tag)
{
	UNUSED(context);

	// tag: 0x12345678 - key: "0x12345678"
	char key[EASYFLASH_KEY_SIZE];
	tag2key(tag, key);

	EfErrCode err = ef_del_env(key);
	if (err != EF_NO_ERR) {
		log_error("delete tag %s fail, err (%d)", key, err);
	}

	log_info("delete tag %s success", key);
}

static const btstack_tlv_t btstack_tlv_rtthread = {
	/* int  (*get_tag)(..);     */ &btstack_tlv_rtthread_get_tag,
	/* int (*store_tag)(..);    */ &btstack_tlv_rtthread_store_tag,
	/* void (*delete_tag)(v..); */ &btstack_tlv_rtthread_delete_tag,
};

/**
 * Init Tag Length Value Store
 */
const btstack_tlv_t * btstack_tlv_rtthread_init_instance(btstack_tlv_rtthread_t * self){
	UNUSED(self);

	return &btstack_tlv_rtthread;
}
