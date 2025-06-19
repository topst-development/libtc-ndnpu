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

#include "npu_test.h"

static void AppNpuPrintInfo(npu_t *npu)
{
    uint32_t val;
    uint8_t npu_num_core = 0;
    uint8_t npu_major_ver = 0;
    uint8_t npu_minor_ver = 0;
    uint16_t npu_product_id = 0;
    uint8_t npu_ecc = 0;

    npu_read_reg(npu, ADDR_NPU_ID_CODE, &val);

    npu_num_core = (val >> 24) & 0xF;
    npu_major_ver = (val >> 20) & 0xF;
    npu_minor_ver = (val >> 18) & 0x3;
    npu_product_id = (val >> 0) & 0xFFFF;
    npu_ecc = (val >> 16) & 0x1;
    npu_num_core += 1;

    if (((val >> 16) & 0xFF) < 0x21)
    {
        printf("Not Ndolphin CS\n");
    }

    printf("========================\n");
    printf("   NPU Device Info   \n");
    printf("========================\n");
    printf("NPU product id: 0x%04X\n", npu_product_id);
    printf("NPU major ver : 0x%X\n", npu_major_ver);
    printf("NPU minor ver : 0x%X\n", npu_minor_ver);
    printf("NPU ecc       : 0x%X\n", npu_ecc);
    printf("NPU core num  : 0x%X\n", npu_num_core);
    printf("========================\n\n");
}

int AppNpuInit(uint8_t npu_count, uint8_t npu_list[2], npu_t *npu_fd[2])
{
    for (int i = 0; i < npu_count; i++)
    {
        npu_fd[i] = npu_open(npu_list[i]);
        if (npu_fd[i] == NULL)
        {
            printf("npu [%d] open fail\n", npu_list[i]);
            return -1;
        }
        else
        {
            printf("npu %d open success\n", npu_list[i]);
            AppNpuPrintInfo(npu_fd[i]);
        }
    }
    return 0;
}

int AppNpuDeinit(uint8_t npu_count, uint8_t npu_list[2], npu_t *npu_fd[2])
{
    for (int i = 0; i < npu_count; i++)
    {
        printf("npu %d close\n", npu_list[i]);
        npu_close(npu_fd[i]);
    }
    return 0;
}

void AppNpuPrintErrStatus(npu_err_bits_t *err_status)
{
    if (err_status->as_field.ce_sram)
    {
        printf("\n0x%08x: MLX SRAM ECC CE error detected\n", err_status->as_word);
    }
    if (err_status->as_field.ue_sram)
    {
        printf("\n0x%08x: MLX SRAM ECC UE error detected\n", err_status->as_word);
    }
    if (err_status->as_field.ce_gbuf)
    {
        printf("\n0x%08x: GBUF ECC CE error detected\n", err_status->as_word);
    }
    if (err_status->as_field.ue_gbuf)
    {
        printf("\n0x%08x: GBUF ECC UE error detected\n", err_status->as_word);
    }
    if (err_status->as_field.ce_cbuf)
    {
        printf("\n0x%08x: Cmd BUF ECC CE error detected\n", err_status->as_word);
    }
    if (err_status->as_field.ue_cbuf)
    {
        printf("\n0x%08x: Cmd BUF ECC UE error detected\n", err_status->as_word);
    }
    if (err_status->as_field.wdt_to)
    {
        printf("\n0x%08x: WDT timeout detected\n", err_status->as_word);
    }
}

static int AppGetFileSize(char *path)
{
    FILE *fp = NULL;
    int file_size;

    if (access(path, F_OK))
        return -1;

    fp = fopen(path, "rb");

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    fclose(fp);

    return file_size;
}

static int AppLoadInput(npu_buf_t *in, char *input_file_name, int size)
{
    FILE *fp = NULL;
    int8_t *addr = (int8_t *)buffer_get_addr(in);

    fp = fopen(input_file_name, "rb");
    if (fp == NULL)
    {
        app_error_printf("file open failed: %s\n", input_file_name);
        exit(1);
    }
    fread(addr, sizeof(int8_t), size, fp);
    fclose(fp);

    return 0;
}

void AppUpdatePerfStats(npu_perf_stats_t *npu_perf_stats, npu_perf_t *npu_perf, uint32_t current_run_cnt, long long convMacNum)
{
    float current_elapsed = (float)npu_perf->elapsed_in_us / MILLISECONDS_PER_SECOND;
    unsigned long long freq = current_elapsed * (NPU_CORE_CLOCK / 1000);
    freq = freq * NPU_ALPHA * NPU_CORE_NUM;
    float current_npu_utilization = (100 * (float)((float)(convMacNum) / (float)freq));

    if (current_run_cnt == 0)
    {
        npu_perf_stats->min_elapsed = current_elapsed;
        npu_perf_stats->max_elapsed = current_elapsed;

        npu_perf_stats->min_dma = npu_perf->dma;
        npu_perf_stats->max_dma = npu_perf->dma;

        npu_perf_stats->min_comp = npu_perf->comp;
        npu_perf_stats->max_comp = npu_perf->comp;

        npu_perf_stats->min_npu_util =  current_npu_utilization;
        npu_perf_stats->max_npu_util =  current_npu_utilization;
    }
    else
    {
        npu_perf_stats->min_elapsed = MIN(npu_perf_stats->min_elapsed, current_elapsed);
        npu_perf_stats->max_elapsed = MAX(npu_perf_stats->max_elapsed, current_elapsed);

        npu_perf_stats->min_dma = MIN(npu_perf_stats->min_dma, npu_perf->dma);
        npu_perf_stats->max_dma = MAX(npu_perf_stats->max_dma, npu_perf->dma);

        npu_perf_stats->min_comp = MIN(npu_perf_stats->min_comp, npu_perf->comp);
        npu_perf_stats->max_comp = MAX(npu_perf_stats->max_comp, npu_perf->comp);

        npu_perf_stats->min_npu_util = MIN(npu_perf_stats->min_npu_util, current_npu_utilization);
        npu_perf_stats->max_npu_util = MAX(npu_perf_stats->max_npu_util, current_npu_utilization);
    }
    npu_perf_stats->total_elapsed += current_elapsed;
    npu_perf_stats->total_dma += npu_perf->dma;
    npu_perf_stats->total_comp += npu_perf->comp;
    npu_perf_stats->total_npu_util += current_npu_utilization;
}

void AppSetupNetworkPath(network_files_t *network_file, char *net_list)
{
    sprintf(network_file->net_path, "%s", net_list);
    app_debug_printf("%s start \n", network_file->net_path);
    sprintf(network_file->ref_path, "%s/sample", network_file->net_path);

    sprintf(network_file->so_file_name, "%s/net.so", network_file->net_path);
    sprintf(network_file->cmd_file_name, "%s/npu_cmd.bin", network_file->net_path);
    sprintf(network_file->weight_file_name, "%s/quantized_network.bin", network_file->net_path);
    sprintf(network_file->input_file_name, "%s/input.ia.bin", network_file->ref_path);
}

int AppInferenceFunc(npu_t *npu, app_context_t *context, int npu_num)
{
    npu_perf_stats_t npu_perf_stats = {
        .total_elapsed = 0.0,
        .total_dma = 0,
        .total_comp = 0};
    npu_perf_t npu_perf;
    npu_err_bits_t err_status;
    network_files_t network_file;

    npu_buf_t *in = NULL;
    npu_buf_t *out = NULL;
    npu_net_t *net = NULL;

    network_file.so_file_name = malloc(512 * sizeof(char));
    network_file.cmd_file_name = malloc(512 * sizeof(char));
    network_file.weight_file_name = malloc(512 * sizeof(char));
    network_file.input_file_name = malloc(512 * sizeof(char));

    if (!network_file.so_file_name || !network_file.cmd_file_name || !network_file.weight_file_name || !network_file.input_file_name)
    {
        app_error_printf("malloc fail\n");
        goto test_fail;
    }

    for (uint8_t networkNum = 0; networkNum < context->net_list_count; networkNum++)
    {
        AppSetupNetworkPath(&network_file, context->net_list[networkNum]);
        if (AppGetFileSize(network_file.weight_file_name) <= 0)
        {
            app_error_printf("wrong network weight file: %s\n", network_file.weight_file_name);
        }

        app_debug_printf("so    : %s\n", network_file.so_file_name);
        app_debug_printf("cmd   : %s\n", network_file.cmd_file_name);
        app_debug_printf("wght  : %s\n", network_file.weight_file_name);
        app_debug_printf("iact  : %s\n", network_file.input_file_name);

        net = network_load_from_file(npu, network_file.so_file_name, network_file.cmd_file_name, network_file.weight_file_name);
        if (!net)
        {
            app_error_printf("%s - network loading failed\n", network_file.net_path);
            goto test_fail;
        }
        app_debug_printf("%s - %s loaded\n", network_file.net_path, net->methods->get_network_name());

        network_file.input_size = network_get_input_size(net);
        in = buffer_alloc(npu, network_file.input_size);

        if (access(network_file.input_file_name, F_OK) != -1)
        {
            AppLoadInput(in, network_file.input_file_name, network_file.input_size);
        }
        else
        {
            app_info_printf("Input file not found. Using zero padding.\n");
            memset(in->caddr, 0, network_file.input_size);
        }

        network_file.output_size = network_get_output_size(net);
        out = buffer_alloc(npu, network_file.output_size);
        app_debug_printf("%s - start inferencing\n", network_file.net_path);
        app_info_printf("NPU%d Network: %s\n", npu_num, network_file.net_path);
        app_info_printf("NPU%d Performance Statistics for %d runs\n", npu_num, context->test_count);

        for (uint32_t current_run_cnt = 0; current_run_cnt < context->test_count; current_run_cnt++)
        {
            if (network_run(net, in, out, &err_status, NPU_RUN_SYNC, &npu_perf, 1000) < 0)
            {
                app_error_printf("%s - inference failed\n", network_file.net_path);
                goto test_fail;
            }

            app_debug_printf("elapsed time : %.2f in ms\n", (float)npu_perf.elapsed_in_us / 1000);
            app_debug_printf("dma count    : %d @pclk\n", npu_perf.dma);
            app_debug_printf("comp count   : %d @pclk\n", npu_perf.comp);

            AppUpdatePerfStats(&npu_perf_stats, &npu_perf, current_run_cnt, net->methods->conv_mac_num);
            AppNpuPrintErrStatus(&err_status);
        }

        app_debug_printf("%s - inference done\n", network_file.net_path);

        if (net)
        {
            network_close(net);
            net = NULL;
        }

        if (in)
        {
            buffer_close(in);
            in = NULL;
        }
        if (out)
        {
            buffer_close(out);
            out = NULL;
        }
        app_info_printf("NPU%d Elapsed Time  (ms) - Min: %-9.2f Max: %-9.2f Avg: %-9.2f\n",
                        npu_num,
                        npu_perf_stats.min_elapsed,
                        npu_perf_stats.max_elapsed,
                        npu_perf_stats.total_elapsed / context->test_count);
        app_info_printf("NPU%d DMA Count  (@pclk) - Min: %-9d Max: %-9d Avg: %-9.2f\n",
                        npu_num,
                        npu_perf_stats.min_dma,
                        npu_perf_stats.max_dma,
                        (float)npu_perf_stats.total_dma / context->test_count);
        app_info_printf("NPU%d Comp Count (@pclk) - Min: %-9d Max: %-9d Avg: %-9.2f\n",
                        npu_num,
                        npu_perf_stats.min_comp,
                        npu_perf_stats.max_comp,
                        (float)npu_perf_stats.total_comp / context->test_count);
        app_info_printf("NPU%d Utilization    (%) - Min: %-9.2f Max: %-9.2f Avg: %-9.2f\n\n",
                        npu_num,
                        npu_perf_stats.min_npu_util,
                        npu_perf_stats.max_npu_util,
                        (float)npu_perf_stats.total_npu_util / context->test_count);
    }
    context->test_result = NPU_TEST_SUCCESS;

test_fail:

    if (net)
    {
        network_close(net);
    }
    if (in)
    {
        buffer_close(in);
    }
    if (out)
    {
        buffer_close(out);
    }

    if (network_file.so_file_name)
    {
        free(network_file.so_file_name);
    }
    if (network_file.cmd_file_name)
    {
        free(network_file.cmd_file_name);
    }
    if (network_file.weight_file_name)
    {
        free(network_file.weight_file_name);
    }
    if (network_file.input_file_name)
    {
        free(network_file.input_file_name);
    }

    if (context->test_result == NPU_TEST_FAIL)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}