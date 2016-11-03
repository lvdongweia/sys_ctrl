/*************************************************************************
FileName: rm_sysctrl_cli.c
Copy Right: Copyright (C), 2006-2014, Archermind. Co., Ltd.
System: RM
Module: SYSCTRL
Author: wangkun
Create Date: 2014-9-4
**************************************************************************/
/*************************************************************************
Editor: wangkun
Version: v1.0 init version
Edit Date: 2014-9-4
**************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <semaphore.h>

#define LOG_TAG "RM_SYSCTRL_CLI"
#include "cutils/log.h"
#include "rm_sysctrl.h"
#include "rm_sysctrl_cli.h"
#include "rm_sysctrl_fault.h"
#include "rm_sysctrl_log.h"
#include "rm_sysctrl_comm.h"
#include "rm_sysctrl_subsys.h"
#include "rm_sysctrl_fsm.h"
#include "rm_sysctrl_can.h"
#include "rm_sysctrl_conf.h"

/* Cli Server Section
 */
typedef void (* CliHandleMsg)(int, void *);

/* Community message handle */
void CliHandleCommunityRequest(int nfp, void *msg);

/* Send to can message handle */
void CliHandleSendToCanRequest(int nfp, void *msg);

/* Sub-system enable or disable message handle */
void CliHandleSubsysEnableRequest(int nfp, void *msg);

/* Show status message handle */
void CliHandleShowStatusRequest(int nfp, void *msg);

/* List config message handle */
void CliHandleListConfigRequest(int nfp, void *msg);

/* Global defination */
CliHandleMsg dhm[CLI_TYPE_MAX] = {
	CliHandleCommunityRequest,
	CliHandleSendToCanRequest,
	CliHandleSubsysEnableRequest,
	NULL,
	CliHandleShowStatusRequest,
	NULL,
	CliHandleListConfigRequest,
	NULL,
};

void RmSysctrlCliInit()
{
}

void RmSysctrlCliDeinit()
{
}

void dump_tlv(char *obj, int len, char *tlv_str)
{
    int i;
    char mtext[TEXT_SIZE] = {0};
    char *p = tlv_str;

    if (!obj)
    {
        RLOGE("Invalid parameter.");
        return;
    }

    memcpy(mtext, obj, len);

    for (i = 0; i < len; i++)
    {
        sprintf(p, "%02x", mtext[i]);
        p += 2;
    }
}

int CliCommunityRequest(char src_module_id,
                        void *data,
                        unsigned int len)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();
    struct RmSysctrlComm *comm = rm_sysctrl->comm;

    return RmSysctrlCommEnQueueMsgXX(comm,
                                  src_module_id,
                                  data,
                                  len);
}

int CliSendToCanRequest(char dst_module_id,
                        void *data,
                        unsigned int len)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();
    struct RmSysctrlComm *comm = rm_sysctrl->comm;

    return RmSysctrlCommSendMsgToCan(comm,
                                     MIDDLE,
                                     dst_module_id,
                                     data,
                                     len);
}

int CliSubsysEnableRequest(char sub_sys_id,
                           int enable)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();
    struct RmSysctrlSubSystem *subsys =
        &rm_sysctrl->sub_system[(int)sub_sys_id];

    if (enable)
        return RmSysctrlEnableSubSystem(subsys);
    else
        return RmSysctrlDisableSubSystem(subsys);
}

int CliCommonResponse(int nfp, int ret, int error_code)
{
  struct Cli dm;
  dm.header.type = CLI_COMMON_RESPONSE;
  dm.u.common_response.ret = ret;
  dm.u.common_response.error_code = error_code;
  dm.header.len = sizeof(dm);

  if(-1 == write(nfp, (void *)&dm, dm.header.len))
  {
    RLOGE("server:write fail!\r\n");
    return -1;
  }
  return 0;
}

void CliHandleCommunityRequest(int nfp, void *msg)
{
    int ret;
    char tlv_str[TEXT_SIZE * 2];
    struct Cli *pdm = (struct Cli *) msg;
    if (!pdm)
    {
      RLOGE("Invalid msg parameter.");
      return;
    }

    RLOGD("The simulation message received by CLI is:");
    RLOGD("  Source Module ID 0x%02x\n", pdm->u.community_request.src_module_id);
    RLOGD("  TLV Length %d\n", pdm->u.community_request.len);
    dump_tlv(pdm->u.community_request.tlv, pdm->u.community_request.len, tlv_str);
    RLOGD("  TLV: %s", tlv_str);


    ret = CliCommunityRequest(pdm->u.community_request.src_module_id,
                              pdm->u.community_request.tlv,
                              pdm->u.community_request.len);
    CliCommonResponse(nfp, ret, ret);
}

void CliHandleSendToCanRequest(int nfp, void *msg)
{
    int ret;
    char tlv_str[TEXT_SIZE * 2];
    struct Cli *pdm = (struct Cli *) msg;
    if (!pdm)
    {
      RLOGE("Invalid msg parameter.");
      return;
    }

    RLOGD("The simulation message send to can by CLI is:");
    RLOGD("  Destination Module ID 0x%02x\n", pdm->u.sendtocan_request.dst_module_id);
    RLOGD("  TLV Length %d\n", pdm->u.sendtocan_request.len);
    dump_tlv(pdm->u.sendtocan_request.tlv, pdm->u.sendtocan_request.len, tlv_str);
    RLOGD("  TLV: %s", tlv_str);


    ret = CliSendToCanRequest(pdm->u.sendtocan_request.dst_module_id,
                              pdm->u.sendtocan_request.tlv,
                              pdm->u.sendtocan_request.len);
    CliCommonResponse(nfp, ret, ret);
}

void CliHandleSubsysEnableRequest(int nfp, void *msg)
{
    int ret;
    char tlv_str[TEXT_SIZE * 2];
    struct Cli *pdm = (struct Cli *) msg;
    if (!pdm)
    {
      RLOGE("Invalid msg parameter.");
      return;
    }

    RLOGD("The Enable or disable sub-system CLI is:");
    RLOGD("  Sub-system ID 0x%02x\n", pdm->u.subsysenable_request.sub_sys_id);
    RLOGD("  Enable %d\n", pdm->u.subsysenable_request.enable);

    ret = CliSubsysEnableRequest(pdm->u.subsysenable_request.sub_sys_id,
                              pdm->u.subsysenable_request.enable);
    CliCommonResponse(nfp, ret, ret);
}

static void AddDelimiter(char *sys_status)
{
    if (!sys_status) return;
    strcat(sys_status, "------------------------------------------\n");
}

void dump(char *out, char *hint, char *str, int len)
{
  int i;
  char *out_str = out;
  int start = 0;

  sprintf(out_str + start, "%s", hint);
  start += strlen(hint);

  for (i = 0; i < len; i++)
  {
    sprintf(out_str + start, "%02x ", (unsigned char)str[i]);
    start += 3;
  }
}

void dump_tight(char *out, char *hint, char *str, int len)
{
  int i;
  char *out_str = out;
  int start = 0;

  sprintf(out_str + start, "%s", hint);
  start += strlen(hint);

  for (i = 0; i < len; i++)
  {
    sprintf(out_str + start, "%02X", (unsigned char)str[i]);
    start += 2;
  }
}

void get_weekday(int weekday, char *weekday_str)
{
    if (!weekday_str)
        return;

    switch (weekday)
    {
    case 0:
        strcpy(weekday_str, "Sun.");
        break;
    case 1:
        strcpy(weekday_str, "Mon.");
        break;
    case 2:
        strcpy(weekday_str, "Tues.");
        break;
    case 3:
        strcpy(weekday_str, "Wed.");
        break;
    case 4:
        strcpy(weekday_str, "Thur.");
        break;
    case 5:
        strcpy(weekday_str, "Fri.");
        break;
    case 6:
        strcpy(weekday_str, "Sat.");
        break;
    default:
        strcpy(weekday_str, "unk");
        break;
    }
}

void get_sex_str(unsigned short sex, char *sex_str)
{
    if (!sex_str)
        return;

    switch (sex)
    {
    case INVALID_SEX:
        strcpy(sex_str, "Invalid");
        break;
    case MALE:
        strcpy(sex_str, "Male");
        break;
    case FEMALE:
        strcpy(sex_str, "Female");
        break;
    case UNKNOWN_SEX:
        strcpy(sex_str, "Unknown");
        break;
    default:
        strcpy(sex_str, "Invalid");
        break;
    }
}

int RmSysctrlShowStatus(struct RmSysctrl *rm_sysctrl, char *sys_status)
{
    int i = 0;
    char tmp_str[200] = {0};
    struct RmSysctrlSubSystem *subsys;
    struct RmSysctrlFault *fault;
    struct RmSysctrlTimer *timer;

    AddDelimiter(sys_status);

    /* System control center status */
    strcat(sys_status, "Current robot system status:\n");

    /* Status */
    memset(tmp_str, 0, sizeof(tmp_str));
    sprintf(tmp_str, "  [State] %s\n",
        RmSysctrlGetStateStr(rm_sysctrl->state));
    strcat(sys_status, tmp_str);
    if (ROBOT_STATUS_WORK == rm_sysctrl->state)
    {
        memset(tmp_str, 0, sizeof(tmp_str));
        sprintf(tmp_str, "  [SubState] %s\n",
            RmSysctrlGetWorkSubStateStr(rm_sysctrl->sub_state));
        strcat(sys_status, tmp_str);
    }
    else if (ROBOT_STATUS_SLEEP == rm_sysctrl->state)
    {
        memset(tmp_str, 0, sizeof(tmp_str));
        sprintf(tmp_str, "  [SubState] %s\n",
            RmSysctrlGetSleepSubStateStr(rm_sysctrl->sub_state));
        strcat(sys_status, tmp_str);
    }

    /* Fault */
    if (ROBOT_STATUS_ABNORMAL == rm_sysctrl->state)
    {
        fault = rm_sysctrl->fault;
        memset(tmp_str, 0, sizeof(tmp_str));
        sprintf(tmp_str, "  [LastFault] %s\n",
            RmSysctrlGetFaultStr(fault->last_fault_type));
        strcat(sys_status, tmp_str);

        memset(tmp_str, 0, sizeof(tmp_str));
        sprintf(tmp_str, "  [LastFaultDesc] %s\n",
            fault->last_fault_desc);
        strcat(sys_status, tmp_str);
    }

    /* Register */
    memset(tmp_str, 0, sizeof(tmp_str));
    sprintf(tmp_str, "  [IsRegistered] %s\n",
        rm_sysctrl->is_registered ? "YES" : "NO");
    strcat(sys_status, tmp_str);

    /* Login */
    memset(tmp_str, 0, sizeof(tmp_str));
    sprintf(tmp_str, "  [IsLogin] %s\n",
        rm_sysctrl->is_login ? "YES" : "NO");
    strcat(sys_status, tmp_str);


    if (ROBOT_STATUS_WORK == rm_sysctrl->state)
    {
        bool debug_en;
        int mode;
        char wday_str[6] = {0};
        char sex_str[8] = {0};

        /* Debug */
        memset(tmp_str, 0, sizeof(tmp_str));
        RmSysctrlGetDebug(rm_sysctrl->config, &debug_en);
        sprintf(tmp_str, "  [Debug] %s\n",
            debug_en ? "Enable" : "Disable");
        strcat(sys_status, tmp_str);

        /* Mode */
        memset(tmp_str, 0, sizeof(tmp_str));
        RmSysctrlGetMode(rm_sysctrl->config, &mode);
        sprintf(tmp_str, "  [Mode] %s\n",
            RmSysctrlGetModeStr(mode));
        strcat(sys_status, tmp_str);

        /* SN */
        memset(tmp_str, 0, sizeof(tmp_str));
        dump_tight(tmp_str, "  [CPUID] ", rm_sysctrl->cpuid, CPUID_LEN);
        strcat(sys_status, tmp_str);
        strcat(sys_status, "\n");
        memset(tmp_str, 0, sizeof(tmp_str));
        sprintf(tmp_str, "  [SN] %s\n",
            rm_sysctrl->sn);
        strcat(sys_status, tmp_str);
        memset(tmp_str, 0, sizeof(tmp_str));
        sprintf(tmp_str, "  [IsSNValid] %s\n",
            rm_sysctrl->is_valid_sn ? "YES" : "NO");
        strcat(sys_status, tmp_str);

        /* Datetime */
        memset(tmp_str, 0, sizeof(tmp_str));
        get_weekday(rm_sysctrl->datetime.weekday, wday_str);
        sprintf(tmp_str, "  [Datetime] %d-%02d-%02d %s %02d:%02d:%02d\n",
            rm_sysctrl->datetime.year + 2000,
            rm_sysctrl->datetime.month,
            rm_sysctrl->datetime.day,
            wday_str,
            rm_sysctrl->datetime.hour,
            rm_sysctrl->datetime.minute,
            rm_sysctrl->datetime.second);
        strcat(sys_status, tmp_str);

        /* Sex */
        memset(tmp_str, 0, sizeof(tmp_str));
        get_sex_str(rm_sysctrl->sex, sex_str);
        sprintf(tmp_str, "  [Sex] %s\n", sex_str);
        strcat(sys_status, tmp_str);

        /* Uptime */
        memset(tmp_str, 0, sizeof(tmp_str));
        sprintf(tmp_str, "  [Uptime] %d seconds\n", rm_sysctrl->uptime);
        strcat(sys_status, tmp_str);
    }

    /* Power */
    memset(tmp_str, 0, sizeof(tmp_str));
    sprintf(tmp_str, "  [BatteryCapacity] %d%%\n",
        rm_sysctrl->battery_capacity);
    strcat(sys_status, tmp_str);

    /* Timer */
    for (i = 0; i < TIMER_MAX; i++)
    {
        timer = &rm_sysctrl->timer[i];

        if (RM_SYSCTRL_TIMER_IS_ON(rm_sysctrl, i))
        {
            memset(tmp_str, 0, sizeof(tmp_str));
            sprintf(tmp_str, "  [Timer %s] open,%d,%d,%d\n",
                RmSysctrlGetTimerStr(i), timer->expire_time,
                timer->expire_conf, timer->expire_times);
            strcat(sys_status, tmp_str);
            if (i == TIMER_EXPECT_STATE)
            {
                memset(tmp_str, 0, sizeof(tmp_str));
                sprintf(tmp_str, "  [ExpectState] %s\n",
                    RmSysctrlGetStateStr(rm_sysctrl->expect_state));
                strcat(sys_status, tmp_str);
            }
        }
    }

    memset(tmp_str, 0, sizeof(tmp_str));
    sprintf(tmp_str, "  [HDMI] Plug %s\n",
        rm_sysctrl->is_hdmi_plug_in ? "in" : "out");
    strcat(sys_status, tmp_str);

    /* Sub-system status */
    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        subsys = &rm_sysctrl->sub_system[i];

        if (IS_SUBSYSTEM_ENABLE(subsys))
        {
            AddDelimiter(sys_status);

            memset(tmp_str, 0, sizeof(tmp_str));
            sprintf(tmp_str, "Current %s sub-system status:\n",
                RmSysctrlGetSubSystemStr(i));
            strcat(sys_status, tmp_str);
            memset(tmp_str, 0, sizeof(tmp_str));
            sprintf(tmp_str, "  [PowerOn] %s\n",
                subsys->poweron ? "YES" : "NO");
            strcat(sys_status, tmp_str);
            memset(tmp_str, 0, sizeof(tmp_str));
            sprintf(tmp_str, "  [Active] %s\n", subsys->active ? "YES" : "NO");
            strcat(sys_status, tmp_str);
            memset(tmp_str, 0, sizeof(tmp_str));
            sprintf(tmp_str, "  [DeadTimes] %d\n", subsys->dead_times);
            strcat(sys_status, tmp_str);
            memset(tmp_str, 0, sizeof(tmp_str));
            sprintf(tmp_str, "  [State] %s\n",
                RmSysctrlGetStateStr(subsys->state));
            strcat(sys_status, tmp_str);
            if (ROBOT_STATUS_WORK == subsys->state)
            {
                memset(tmp_str, 0, sizeof(tmp_str));
                sprintf(tmp_str, "  [SubState] %s\n",
                    RmSysctrlGetWorkSubStateStr(subsys->sub_state));
                strcat(sys_status, tmp_str);
            }
            else if (ROBOT_STATUS_SLEEP == subsys->state)
            {
                memset(tmp_str, 0, sizeof(tmp_str));
                sprintf(tmp_str, "  [SubState] %s\n",
                    RmSysctrlGetSleepSubStateStr(subsys->sub_state));
                strcat(sys_status, tmp_str);
            }
            memset(tmp_str, 0, sizeof(tmp_str));
            sprintf(tmp_str, "  [ExpireTime] %d\n", subsys->expire_time);
            strcat(sys_status, tmp_str);
            memset(tmp_str, 0, sizeof(tmp_str));
            sprintf(tmp_str, "  [ExpireConfig] %d\n", subsys->expire_conf);
            strcat(sys_status, tmp_str);
            memset(tmp_str, 0, sizeof(tmp_str));
            sprintf(tmp_str, "  [IsInitalized] %s\n",
                subsys->is_initalized ? "YES" : "NO");
            strcat(sys_status, tmp_str);
            if (ROBOT_STATUS_WORK == rm_sysctrl->state)
            {
                memset(tmp_str, 0, sizeof(tmp_str));
                sprintf(tmp_str, "  [Debug] %s\n",
                    subsys->debug_en ? "Enable" : "Disable");
                strcat(sys_status, tmp_str);

                memset(tmp_str, 0, sizeof(tmp_str));
                sprintf(tmp_str, "  [Mode] %s\n",
                    RmSysctrlGetModeStr(subsys->mode));
                strcat(sys_status, tmp_str);
            }
            memset(tmp_str, 0, sizeof(tmp_str));
            if (subsys->ver_str_used)
            {
                sprintf(tmp_str, "  [Version] %s\n", subsys->u.ver_str);
            }
            else
            {
                sprintf(tmp_str, "  [Version] %d.%d.%d.%d.%d%02d%02d.%d\n",
                    subsys->u.ver.sub_sys_id,
                    subsys->u.ver.major,
                    subsys->u.ver.minor,
                    subsys->u.ver.revision,
                    subsys->u.ver.building_year,
                    subsys->u.ver.building_month,
                    subsys->u.ver.building_day,
                    subsys->u.ver.rom_id);
            }
            strcat(sys_status, tmp_str);
        }
    }

    AddDelimiter(sys_status);
    return 0;
}

int CliShowStatusRequest(char *sys_status)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();

    return RmSysctrlShowStatus(rm_sysctrl, sys_status);
}

int CliShowStatusResponse(int nfp, void *sys_status, int ret, int error_code)
{
  struct Cli dm;
  dm.header.type = CLI_SHOW_STATUS_RESPONSE;
  dm.u.show_status_response.ret = ret;
  dm.u.show_status_response.error_code = error_code;
  memcpy(dm.u.show_status_response.system_status_output,
      sys_status, TEXT_SIZE);
  dm.header.len = sizeof(dm);

  if(-1 == write(nfp, (void *)&dm, dm.header.len))
  {
    RLOGE("server:write fail!\r\n");
    return -1;
  }
  return 0;
}

void CliHandleShowStatusRequest(int nfp, void *msg)
{
    int ret = 0;
    char sys_status_str[TEXT_SIZE] = {0};
    struct Cli *pdm = (struct Cli *) msg;
    if (!pdm)
    {
      RLOGE("Invalid msg parameter.");
      return;
    }

    ret = CliShowStatusRequest(sys_status_str);
    CliShowStatusResponse(nfp, sys_status_str, ret, ret);
}

int RmSysctrlListConfig(struct RmSysctrl *rm_sysctrl, char *list_config)
{
    int i = 0;
    char tmp_str[200] = {0};
    struct RmSysctrlCnf *config = NULL;
    struct RmSysctrlCnfNode *node = NULL;

    if (!rm_sysctrl) return -1;

    config = rm_sysctrl->config;
    if (!config) return -1;

    strcat(list_config, "Configs:\n");
    for (i = 0; i < RM_SYSCTRL_CONFIG_MAX; i++)
    {
        node = &(config->nodes[i]);
        if (node->is_used)
        {
            memset(tmp_str, 0, sizeof(tmp_str));
            sprintf(tmp_str, "  [%s] %s\n", node->name, node->cnf_value);
            strcat(list_config, tmp_str);
        }
    }

    return 0;
}

int CliListConfigRequest(char *list_config)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();

    return RmSysctrlListConfig(rm_sysctrl, list_config);
}

int CliListConfigResponse(int nfp, void *list_config, int ret, int error_code)
{
  struct Cli dm;
  dm.header.type = CLI_LIST_CONFIG_RESPONSE;
  dm.u.list_config_response.ret = ret;
  dm.u.list_config_response.error_code = error_code;
  memcpy(dm.u.list_config_response.list_config_output,
      list_config, TEXT_SIZE);
  dm.header.len = sizeof(dm);

  if(-1 == write(nfp, (void *)&dm, dm.header.len))
  {
    RLOGE("server:write fail!\r\n");
    return -1;
  }
  return 0;
}

void CliHandleListConfigRequest(int nfp, void *msg)
{
    int ret = 0;
    char list_config_str[TEXT_SIZE] = {0};
    struct Cli *pdm = (struct Cli *) msg;
    if (!pdm)
    {
      RLOGE("Invalid msg parameter.");
      return;
    }

    ret = CliListConfigRequest(list_config_str);
    CliListConfigResponse(nfp, list_config_str, ret, ret);
}

int RmSysctrlCliRev(int nfp)
{
    char buffer[BUFFER_SIZE]={0};
    int recbytes; 
    struct CliHeader *pdmh;

    if(-1 == (recbytes = read(nfp, buffer, BUFFER_SIZE)))
    {
        RLOGE("server:read data fail !\r\n");
        return -1;
    }

    if (recbytes <= 0) return -1;

    buffer[recbytes]='\0';
    pdmh = (struct CliHeader *) buffer;

    if (pdmh->type < 0 || pdmh->type >= CLI_TYPE_MAX)
    {
        RLOGE("Invalid message type !\r\n");
        return -1;
    }

    if (dhm[pdmh->type])
        dhm[pdmh->type](nfp, buffer);

    return 0;
}

void *RmSysctrlCliThread(void *arg)
{
    int sfp,nfp;
    struct sockaddr_in s_add,c_add;
    int sin_size;
    unsigned short portnum = CLI_PORT_NUM;

    sfp = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sfp)
    {
        RLOGE("socket fail ! \r\n");
        return NULL;
    }


    bzero(&s_add,sizeof(struct sockaddr_in));
    s_add.sin_family=AF_INET;
    s_add.sin_addr.s_addr=htonl(INADDR_ANY);
    s_add.sin_port=htons(portnum);

    if(-1 == bind(sfp,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
    {
        RLOGE("server:bind fail !\r\n");
        return NULL;
    }

    if(-1 == listen(sfp,5))
    {
        RLOGE("server:listen fail !\r\n");
        return NULL;
    }

    RmSysctrlCliInit();

    while(1)
    {
        sin_size = sizeof(struct sockaddr_in);

        nfp = accept(sfp, (struct sockaddr *)(&c_add), &sin_size);
        if(-1 == nfp)
        {
            RLOGE("server:accept fail !\r\n");
            return NULL;
        }

        RmSysctrlCliRev(nfp);

        close(nfp);
    }

    RmSysctrlCliDeinit();

    close(sfp);
    return arg;
}

