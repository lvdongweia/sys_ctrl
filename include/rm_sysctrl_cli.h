/*************************************************
 Copyright (C), 2006-2014, Archermind. Co., Ltd.
 File name: rm_sysctrl_cli.h
 Author: wangkun
 Version: v1.0
 Date: 2014-9-4
 Description: This program defined the sysctrl cli.
 Others: None
 
 Function List: 
   1. RmSysctrlCliThread to implement the cli thread.

 
 History:
   V1.0    2014-9-4    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_CLI_H_
#define RM_SYSCTRL_CLI_H_

#ifdef __cplusplus
extern "C"{
#endif

/* Cli common section 
 */

/* Micro */
#define CLI_PORT_NUM 0x1111
#define BUFFER_SIZE  (TEXT_SIZE + 64)

enum CLI_TYPE {
    CLI_COMMUNITY_REQUEST,
    CLI_SENDTOCAN_REQUEST,
    CLI_SUBSYS_ENABLE_REQUEST,
    CLI_COMMON_RESPONSE,
    CLI_SHOW_STATUS_REQUEST,
    CLI_SHOW_STATUS_RESPONSE,
    CLI_LIST_CONFIG_REQUEST,
    CLI_LIST_CONFIG_RESPONSE,
    CLI_TYPE_MAX
};

struct CliHeader {
    int type;
    int len;
};

#define CLI_RESPONSE  \
    int ret;          \
    int error_code;

struct CliCommonResponse {
    CLI_RESPONSE
};

/* connect messages */
struct CliCommunityRequest {
    char src_module_id;
    char tlv[TEXT_SIZE + 2];
    unsigned int len;
};

/* Send to can messages */
struct CliSendToCanRequest {
    char dst_module_id;
    char tlv[TEXT_SIZE + 2];
    unsigned int len;
};

/* Sub-system enable or disable messages */
struct CliSubsysEnableRequest {
    char sub_sys_id;
    int enable;
};

/* Show status message */
struct CliShowStatusRequest {
    void *dump;
};

struct CliShowStatusResponse {
    CLI_RESPONSE
    char system_status_output[TEXT_SIZE];
};

/* List config message */
struct CliListConfigRequest {
    void *dump;
};

struct CliListConfigResponse {
    CLI_RESPONSE
    char list_config_output[TEXT_SIZE];
};

struct Cli {
    struct CliHeader header;
    union {
        struct CliCommunityRequest   community_request;
        struct CliSendToCanRequest   sendtocan_request;
        struct CliSubsysEnableRequest subsysenable_request;
        struct CliCommonResponse     common_response;
        struct CliShowStatusRequest  show_status_request;
        struct CliShowStatusResponse show_status_response;
        struct CliListConfigRequest  list_config_request;
        struct CliListConfigResponse list_config_response;
    }u;
};

/****************************************************************************
 Name: RmSysctrlCliThread
 Function: This function is to implement the cli thread.
 Params: arg - The parameter of thread like rm_sysctrl.

 Return:
   None

 Systax:
   ret = pthread_create(&rm_sysctrl->ntid_cli,
                        NULL,
                        RmSysctrlCliThread,
                        rm_sysctrl);
******************************************************************************/
void *RmSysctrlCliThread(void *arg);

#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_CLI_H_*/
