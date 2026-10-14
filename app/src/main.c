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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "npu_test.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                       Internal Function Declarations                       */
/* ========================================================================== */
static void AppPrintUsage(void);
void AppArgParse(int argc, char **argv, app_context_t *context);
void *AppThreadFunc(void *arg);
void AppCreateThread(app_context_t *context);

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */
static void AppPrintUsage(void)
{
    printf(" usage : options\n"
           " -d : device name number:   default : 0\n"
           "    0 for /dev/npu0\n"
           "    1 for /dev/npu1\n"
           "\n"
           " -n : npu network Path:         default: " DEFAULT_NETWORK_PATH "\n"
           "\n"
           " -t : test count:              default: 1\n"
           "\n"
           " -? : print this message\n"
           "\n");
}

void AppArgParse(int argc, char **argv, app_context_t *context)
{
    strcpy(context->net_list[0], DEFAULT_NETWORK_PATH);
    context->net_list_count = 0;
    context->npu_count = 0;
    context->test_count = DEFAULT_TEST_COUNT;

    int opt;
    while ((opt = getopt(argc, argv, "d:n:t:?")) != -1)
    {
        switch (opt)
        {
        case 'd':
            optind--;
            int npu_id;
            while (context->npu_count < MAX_NPU_COUNT && optind < argc && sscanf(argv[optind], "%d", &npu_id) == 1 && argv[optind][0] != '-')
            {
                if (npu_id == 0 || npu_id == 1)
                {
                    context->npu_list[context->npu_count++] = npu_id;
                    optind++;
                }
                else
                {
                    printf("Error: Only NPU Cluster 0 and 1 are allowed.\n");
                    exit(1);
                }
            }
            if (optind < argc && argv[optind][0] != '-' && sscanf(argv[optind], "%d", &npu_id) == 1)
            {
                printf("Error: Maximum %d NPUs can be specified.\n", MAX_NPU_COUNT);
                exit(1);
            }
            break;
        case 'n':
            if (context->net_list_count < MAX_NETWORK_PATH_SIZE)
            {
                strncpy(context->net_list[context->net_list_count], optarg, MAX_NETWORK_PATH_LEN - 1);
                context->net_list[context->net_list_count][MAX_NETWORK_PATH_LEN - 1] = '\0';
                context->net_list_count++;
            }
            else
            {
                printf("Error: Only one neural network can be entered.\n");
                printf("The test is conducted with the neural network that was initially entered.\n\n");
            }
            break;
        case 't':
            context->test_count = atoi(optarg);
            break;
        case '?':
            AppPrintUsage();
            exit(0);
            break;
        default:
            AppPrintUsage();
            exit(0);
            break;
        }
    }
    if (context->net_list_count == 0)
    {
        context->net_list_count = 1;
    }
    if (context->npu_count == 0)
    {
        context->npu_count = 1;
    }
}

void *AppThreadFunc(void *arg)
{
    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    app_context_t *context = thread_arg->context;
    int npu_num = thread_arg->npu_num;

    if (AppInferenceFunc(context->npuFd[npu_num], context, context->npu_list[npu_num]))
    {
        context->test_result = NPU_TEST_SUCCESS;
    }
    else
    {
        context->test_result = NPU_TEST_FAIL;
    }
    

    free(thread_arg);

    return NULL;
}

void AppCreateThread(app_context_t *context)
{
    pthread_t npuTestThreads[MAX_NPU_COUNT];
    thread_arg_t *thread_args[MAX_NPU_COUNT];

    for (int i = 0; i < context->npu_count; i++)
    {
        thread_args[i] = malloc(sizeof(thread_arg_t));
        thread_args[i]->context = context;
        thread_args[i]->npu_num = i;
        if (pthread_create(&npuTestThreads[i], NULL, AppThreadFunc, thread_args[i]) != 0)
        {
            perror("Failed to create thread");
            context->test_result = NPU_TEST_FAIL;
        }
    }

    for (int i = 0; i < context->npu_count; i++)
    {
        if (pthread_join(npuTestThreads[i], NULL) != 0)
        {
            perror("Failed to join thread");
            context->test_result = NPU_TEST_FAIL;
        }
    }
}

int main(int argc, char **argv)
{
    app_context_t *context = malloc(sizeof(app_context_t));
    context->test_result = NPU_TEST_FAIL;

    AppArgParse(argc, argv, context);
    AppNpuInit(context->npu_count, context->npu_list, context->npuFd);
    AppCreateThread(context);
    AppNpuDeinit(context->npu_count, context->npu_list, context->npuFd);

    if (context->test_result == NPU_TEST_FAIL) 
    {
        printf("========================\n");
        printf("npu test: Fail!\n\n");
        free(context);
        return 1;
    }
    printf("========================\n");
    printf("npu test: Success!\n\n");

    free(context);
    return 0;
}
