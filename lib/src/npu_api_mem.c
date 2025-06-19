/*
 * Copyright Telechips Inc.
 *
 * TCC Version 1.0
 *
 * This source code contains confidential information of Telechips.
 *
 * Any unauthorized use without a written permission of Telechips including not
 * limited to re-distribution in source or binary form is strictly prohibited.
 *
 * This source code is provided "AS IS" and nothing contained in this source code
 * shall constitute any express or implied warranty of any kind, including without
 * limitation, any warranty of merchantability, fitness for a particular purpose
 * or non-infringement of any patent, copyright or other third party intellectual
 * property right.
 * No warranty is made, express or implied, regarding the information's accuracy,
 * completeness, or performance.
 *
 * In no event shall Telechips be liable for any claim, damages or other
 * liability arising from, out of or in connection with this source code or
 * the use in the source code.
 *
 * This source code is provided subject to the terms of a Mutual Non-Disclosure
 * Agreement between Telechips and Company.
 */

#include "npu_api.h"

char* npu_api_malloc(size_t size)
{
    void* ptr = malloc(size);
    return (char*)ptr;
}

void npu_api_free(void *addr)
{
    free(addr);
}

npu_buf_t* npu_buf_malloc(void)
{
    void* ptr = malloc(sizeof(npu_buf_t));
    return (npu_buf_t*)ptr;
}

void npu_buf_free(void* addr)
{
    free(addr);
}

npu_net_t* npu_net_malloc(void)
{
    void* ptr = malloc(sizeof(npu_net_t));
    return (npu_net_t*)ptr;
}

void npu_net_free(void *addr)
{
    free(addr);
}

struct enlight_net* enlight_net_malloc(void)
{
    void* ptr = malloc(sizeof(struct enlight_net));
    return (struct enlight_net *)ptr;
}

void enlight_net_free(void *addr)
{
    free(addr);
}
