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

#ifndef __NPU_TEST_H__
#define __NPU_TEST_H__

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include "npu_api.h"

// #define APP_DEBUG_PRINT
#define APP_INFO_PRINT
#define APP_ERROR_PRINT

#define NPU_CORE_CLOCK	1000000000
#define NPU_CORE_NUM    2
#define CPU_CORE_NUM	4
#define NPU_ALPHA (32 * 32)

#define MILLISECONDS_PER_SECOND 1000.0
#define MAX_NETWORK_PATH_LEN 256
#define MAX_NETWORK_PATH_SIZE 1
#define MAX_NPU_COUNT 2
#define DEFAULT_TEST_COUNT 1
#define DEFAULT_NETWORK_PATH "/usr/share/yolov5s_quantized"

#ifdef TELECHIPS_DEBUG
    #define app_debug_printf(...) printf(__VA_ARGS__)
#else
    #define app_debug_printf(...) do {} while (0)
#endif

#ifdef APP_INFO_PRINT
    #define app_info_printf(...) printf(__VA_ARGS__)
#else
    #define app_info_printf(...) do {} while (0)
#endif

#ifdef APP_ERROR_PRINT
    #define app_error_printf(...) printf(__VA_ARGS__)
#else
    #define app_error_printf(...) do {} while (0)
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))


enum
{
    NPU_TEST_SUCCESS = 0x0,
    NPU_TEST_FAIL    = 0x1,
};

typedef struct app_context_t
{
    char net_list[1][256];
    uint8_t net_list_count;

    uint8_t npu_count;
    uint8_t npu_list[2];
    npu_t *npuFd[2];

    uint8_t test_result;

    uint32_t test_count;
}app_context_t;

typedef struct {
    app_context_t* context;
    int npu_num;
} thread_arg_t;

typedef struct {
    float min_elapsed;
    float max_elapsed;
    float total_elapsed;
    float min_npu_util;
    float max_npu_util;
    float total_npu_util;
    uint32_t min_dma;
    uint32_t max_dma;
    uint32_t total_dma;
    uint32_t min_comp;
    uint32_t max_comp;
    uint32_t total_comp;
} npu_perf_stats_t;

typedef struct {
    char net_path[512];
    char ref_path[512];
    char *cmd_file_name;
    char *weight_file_name;
    char *input_file_name;
    char *so_file_name;
    uint32_t input_size;
    uint32_t output_size;
} network_files_t;

int AppNpuInit(uint8_t npu_count, uint8_t npu_list[2], npu_t *npu_fd[2]);
int AppNpuDeinit(uint8_t npu_count, uint8_t npu_list[2], npu_t *npu_fd[2]);
void AppNpuPrintErrStatus(npu_err_bits_t *err_status);
void AppUpdatePerfStats(npu_perf_stats_t *npu_perf_stats, npu_perf_t *npu_perf, uint32_t current_run_cnt, long long convMacNum);
void AppSetupNetworkPath(network_files_t *network_file, char *net_list);
int AppInferenceFunc(npu_t* npu, app_context_t *context, int npu_num);


#endif //__NPU_TEST_H__