/*************************************************************************
FileName: rm_sysctrl_can.c
Copy Right: Copyright (C), 2006-2014, Archermind. Co., Ltd.
System: RM
Module: SYSCTRL
Author: wangkun
Create Date: 2014-9-11
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
#include <sys/ipc.h>
#include <malloc.h>
#include <semaphore.h>

#define LOG_TAG "RM_SYSCTRL_CAN"
#include "cutils/log.h"
#include "rm_sysctrl.h"
#include "rm_sysctrl_cli.h"
#include "rm_sysctrl_fault.h"
#include "rm_sysctrl_log.h"
#include "rm_sysctrl_comm.h"
#include "rm_sysctrl_subsys.h"
#include "rm_sysctrl_fsm.h"
#include "RmCANClient_c.h"
#include "rm_sysctrl_can.h"

extern void dump_tlv(char *obj, int len, char *tlv_str);

/* Receive callback */
void RmSysctrlCanRecvDataCB(int priority, int src_id, const void *pdata, int len);
void RmSysctrlCanServiceDiedCB();

int RmSysctrlCreateCanClient(struct can_client_callback **callback_addr)
{
    struct can_client_callback *callback_l;
    callback_l = (struct can_client_callback *)malloc(sizeof(struct can_client_callback));
    if (!callback_l)
    {
        RLOGE("can_client_callback create failed!\n");
        return -1;
    }
    memset(callback_l, 0, sizeof(struct can_client_callback));
    *callback_addr = callback_l;
    return 0;
}

int RmSysctrlCreateCan(struct RmSysctrlCan **can_addr)
{
    struct RmSysctrlCan *can_l;
    can_l = (struct RmSysctrlCan *)malloc(sizeof(struct RmSysctrlCan));
    if (!can_l)
    {
        RLOGE("RmSysctrlCan create failed!\n");
        return -1;
    }
    memset(can_l, 0, sizeof(struct RmSysctrlCan));
    *can_addr = can_l;
    return 0;
}

int RmSysctrlInitCan(struct RmSysctrlCan **can_addr)
{
    int ret = -1;
    struct RmSysctrlMsgQueue *msg_queue = NULL;
    struct RmSysctrlCan *can;
    struct can_client_callback *callback;

    RLOGI("Init RmSysctrlCan");


    /* Create can_client_callback */
    ret = RmSysctrlCreateCanClient(&callback);
    if (ret < 0)
    {
        RLOGE("can_client_callback create failed!");
        return -1;
    }

    callback->RmRecvCANData = RmSysctrlCanRecvDataCB;
    callback->RmCANServiceDied = RmSysctrlCanServiceDiedCB;

    ret = RmInitCANClient(RM_SYSCTRL, callback);
    if (ret < 0)
    {
        RLOGE("Init CAN Client failed!");
        return -1;
    }

    /* Create RmSysctrlCan */
    ret = RmSysctrlCreateCan(can_addr);
    if (ret < 0)
    {
        RLOGE("RmSysctrlCan create failed!");
        return -1;
    }

    can = *can_addr;
    can->callback = callback;

    return 0;
}

int RmSysctrlCanDataSend(unsigned int p, int s, int d, void *pdata, int len)
{
    char tlv_str[TEXT_SIZE * 2];

    dump_tlv((char *)pdata, len, tlv_str);
    RLOGD("The message to send by can is:");
    RLOGD("  Priority %d", p);
    RLOGD("  Source Module ID 0x%02x", s);
    RLOGD("  Destination Module ID 0x%02x", d);
    RLOGD("  TLV %s", tlv_str);
    RLOGD("  TLV Length %d", len);

    //RmSendCANData(RM_SYSCTRL, pdata, len, p); lo for test can forward module
    RmSendCANData(d, pdata, len, p);
    return 0;
}

void RmSysctrlCanRecvDataCB(int priority, int src_id, const void *pdata, int len)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();
    struct RmSysctrlComm *comm = rm_sysctrl->comm;
    char tlv_str[TEXT_SIZE * 2] = {0};

    dump_tlv((char *)pdata, len, tlv_str);

    RLOGD("pid: %d, priority: %d, src_id: %d, data: %s, len: %d",
                getpid(), priority, src_id, tlv_str, len);
    if (src_id == RM_SYSCTRL)
    {
        RLOGI("This message comes from own, ignore it.");
        return;
    }

    RmSysctrlCommEnQueueMsgXX(comm,
                              src_id,
                              (void *)pdata,
                              len);
}

void RmSysctrlCanServiceDiedCB()
{
    RLOGE("CAN Service is dead");
}
