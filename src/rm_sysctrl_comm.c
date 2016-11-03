/*************************************************************************
FileName: rm_sysctrl_comm.c
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
#include <sys/ipc.h>
#include <malloc.h>
#include <semaphore.h>

#define LOG_TAG "RM_SYSCTRL_COM"
#include "cutils/log.h"
#include "rm_sysctrl.h"
#include "rm_sysctrl_cli.h"
#include "rm_sysctrl_fault.h"
#include "rm_sysctrl_log.h"
#include "rm_sysctrl_comm.h"
#include "rm_sysctrl_subsys.h"
#include "rm_sysctrl_fsm.h"
#include "rm_sysctrl_can.h"

/* Common message */
static char *rm_sysctrl_common_msg_str[] =
{
    "COMMON_SYSCTRL_INIT_CMD",
    "COMMON_SYSCTRL_INIT_CMD_REP",
    "COMMON_SYSCTRL_SLEEP_CMD",
    "COMMON_SYSCTRL_SLEEP_CMD_REP",
    "COMMON_SYSCTRL_WAKE_CMD",
    "COMMON_SYSCTRL_WAKE_CMD_REP",
    "COMMON_SYSCTRL_SECURITY_CMD",
    "COMMON_SYSCTRL_SECURITY_CMD_REP",
    "COMMON_SYSCTRL_EXIT_SECURITY_CMD",
    "COMMON_SYSCTRL_EXIT_SECURITY_CMD_REP",
    "COMMON_SYSCTRL_HALT_CMD",
    "COMMON_SYSCTRL_HALT_CMD_REP",
    "COMMON_SYSCTRL_DEBUG_CMD",
    "COMMON_SYSCTRL_DEBUG_CMD_REP",
    "COMMON_SYSCTRL_MODE_CMD",
    "COMMON_SYSCTRL_MODE_CMD_REP",
    "COMMON_SYSCTRL_ABNORMAL_OCCUR_NOTI",
    "COMMON_SYSCTRL_ABNORMAL_RESOLVE_NOTI",
    "COMMON_SYSCTRL_LOW_POWER_NOTI",
    "COMMON_SYSCTRL_POWER_CHARGE_NOTI",
    "COMMON_SYSCTRL_POWER_CHARGE_END_NOTI",
    "COMMON_SYSCTRL_SUBSYS_STATE_CHANGE_NOTI",
    "COMMON_SYSCTRL_SYS_STATE_CHANGE_NOTI",
    "COMMON_SYSCTRL_SUBSYS_STATE_QUERY",
    "COMMON_SYSCTRL_SUBSYS_STATE_QUERY_REP",
    "COMMON_SYSCTRL_VERSION_QUERY",
    "COMMON_SYSCTRL_VERSION_QUERY_REP",
    "COMMON_SYSCTRL_PROD_MODE_QUERY",
    "COMMON_SYSCTRL_PROD_MODE_QUERY_REP",
    "COMMON_SYSCTRL_SWITCH_PROD_MODE_CMD",
    "COMMON_SYSCTRL_SWITCH_PROD_MODE_CMD_REP",
    "COMMON_SYSCTRL_UPGRADE_CMD",
    "COMMON_SYSCTRL_UPGRADE_CMD_REP",
    "COMMON_SYSCTRL_EXIT_UPGRADE_CMD",
    "COMMON_SYSCTRL_EXIT_UPGRADE_CMD_REP",
    "COMMON_SYSCTRL_NOT_LOW_POWER_NOTI",
};

/* RM message */
static char *rm_sysctrl_rm_msg_str[] =
{
    "RM_SYSCTRL_REGISTER_CMD",
    "RM_SYSCTRL_REGISTER_CMD_REP",
    "RM_SYSCTRL_LOGIN_CMD",
    "RM_SYSCTRL_LOGIN_CMD_REP",
    "RM_SYSCTRL_RESTORE_FACTORY_CMD",
    "RM_SYSCTRL_RESTORE_FACTORY_CMD_REP",
    "RM_SYSCTRL_REGISTERED_NOTI",
    "RM_SYSCTRL_LOGIN_NOTI",
    "RM_SYSCTRL_BATTERY_QUERY",
    "RM_SYSCTRL_BATTERY_QUERY_REP",
    "RM_SYSCTRL_REGISTERED_QUERY",
    "RM_SYSCTRL_REGISTERED_QUERY_REP",
    "RM_SYSCTRL_LOGIN_QUERY",
    "RM_SYSCTRL_LOGIN_QUERY_REP",
};

/* RP message */
static char *rm_sysctrl_rp_msg_str[] =
{
    "RP_SYSCTRL_POWER_OFF_CMD",
    "RP_SYSCTRL_POWER_OFF_CMD_REP",
    "RP_SYSCTRL_POWER_ON_CMD",
    "RP_SYSCTRL_POWER_ON_CMD_REP",
    "RP_SYSCTRL_BATTERY_NOTI",
    "RP_SYSCTRL_POWER_OFF_NOTI",
    "RP_SYSCTRL_POWER_ON_NOTI",
    "RP_SYSCTRL_BATTERY_STATUS_QUERY",
    "RP_SYSCTRL_BATTERY_STATUS_QUERY_REP",
    "RP_SYSCTRL_GET_CPU_ID_CMD",
    "RP_SYSCTRL_GET_CPU_ID_CMD_REP",
    "RP_SYSCTRL_SAVE_ROBOT_SN_CMD",
    "RP_SYSCTRL_SAVE_ROBOT_SN_CMD_REP",
    "RP_SYSCTRL_READ_ROBOT_SN_CMD",
    "RP_SYSCTRL_READ_ROBOT_SN_CMD_REP",
    "RP_SYSCTRL_DATETIME_QUERY",
    "RP_SYSCTRL_DATETIME_QUERY_REP",
    "RP_SYSCTRL_SET_DATETIME_CMD",
    "RP_SYSCTRL_SET_DATETIME_CMD_REP",
    "RP_SYSCTRL_SEX_QUERY",
    "RP_SYSCTRL_SEX_QUERY_REP",
    "RP_SYSCTRL_SET_SEX_CMD",
    "RP_SYSCTRL_SET_SEX_CMD_REP",
    "RP_SYSCTRL_UPTIME_QUERY",
    "RP_SYSCTRL_UPTIME_QUERY_REP",
};

/* RF message */
static char *rm_sysctrl_rf_msg_str[] =
{
    "RF_SYSCTRL_WAKE_NOTI",
};

int RmSysctrlCreateComm(struct RmSysctrlComm **comm_addr)
{
    struct RmSysctrlComm *comm_l;
    comm_l = (struct RmSysctrlComm *)malloc(sizeof(struct RmSysctrlComm));
    if (!comm_l)
    {
        RLOGE("RmSysctrlComm create failed!\n");
        return -1;
    }
    memset(comm_l, 0, sizeof(struct RmSysctrlComm));
    *comm_addr = comm_l;
    return 0;
}

int RmSysctrlInitComm(struct RmSysctrlComm **comm_addr)
{
    int ret = -1;
    struct RmSysctrlMsgQueue *msg_queue = NULL;
    struct RmSysctrlComm *comm;

    RLOGI("Init RmSysctrlComm");

    /* Create RmSysctrlComm */
    ret = RmSysctrlCreateComm(comm_addr);
    if (ret < 0)
    {
        RLOGE("RmSysctrlComm create failed!");
        return -1;
    }

    comm = *comm_addr;

    RLOGI("Init RmSysctrlMsgQueue");

    /* Create and init RmSysctrlMsgQueue */
    msg_queue =
       (struct RmSysctrlMsgQueue *)malloc(sizeof(struct RmSysctrlMsgQueue));
    if (!msg_queue)
    {
        RLOGE("RmSysctrlMsgQueue create failed!\n");
        return -1;
    }
    msg_queue->prev = msg_queue;
    msg_queue->next = msg_queue;
    comm->msg_queue = msg_queue;

    /* Initialize the semaphore which counts msgs in the queue. Its
     * initial value should be zero.
     */
    sem_init(&comm->msg_queue_count, 0, 0);

    /* A mutex protecting msg_queue. */
    pthread_mutex_init(&comm->msg_queue_mutex, NULL);

    return 0;
}

int RmSysctrlCommRecvMsg(struct RmSysctrlComm *comm,
                         void *data,
                         unsigned int len)
{
    unsigned int recvlen = 0;
    struct RmSysctrlMsgQueue *next_msg = NULL;
    struct RmSysctrlMsgQueue *msg_queue = comm->msg_queue;

    recvlen = sizeof(struct RmSysctrlMessage);
    //RLOGD("len:%d, recvlen:%d", len, recvlen);
    if (len > recvlen)
    {
        RLOGE("The length is too longer.");
        return -1;
    }

    RLOGD("Wating messages ....");
    /* Wait on the msg queue semaphore. If its value is positive,
     * indicating that the queue is not empty, decrement the count.
     * If the queue is empty, block until a new msg is enqueued.
     */
    sem_wait (&comm->msg_queue_count);

    /* Lock the mutex on the msg queue. */
    pthread_mutex_lock (&comm->msg_queue_mutex);

    /* Because of the semaphore, we know the queue is not empty. Get
     * the next available msg. 
     */
    next_msg = msg_queue->next;

    /* Remove this msg from the head. */
    next_msg->next->prev = next_msg->prev;
    next_msg->prev->next = next_msg->next;
    next_msg->prev = NULL;
    next_msg->next = NULL;

    /* Unlock the mutex on the msg queue because we’re done with the
     * queue for now.
     */
    pthread_mutex_unlock (&comm->msg_queue_mutex);

    /* Copy the data. */
    memcpy(data, &next_msg->rscm, len);

    /* Clean up. */
    free(next_msg);

    return 0;
}

int RmSysctrlCommEnQueueMsg(struct RmSysctrlComm *comm,
                         void *data,
                         unsigned int len)
{
    unsigned int sendlen = 0;
    struct RmSysctrlMsgQueue *new_msg = NULL;
    struct RmSysctrlMsgQueue *msg_queue = comm->msg_queue;

    /* Allocate a new msg object. */
    new_msg = 
       (struct RmSysctrlMsgQueue*)malloc(sizeof (struct RmSysctrlMsgQueue));

    /* Set the message fields of the msg struct here... */
    memset(&new_msg->rscm, 0, sizeof(struct RmSysctrlMessage));

    sendlen = sizeof(struct RmSysctrlMessage);
    if (len > sendlen)
    {
        RLOGE("The length is too longer.");
        return -1;
    }
    memcpy(&new_msg->rscm, data, len);

    /* Lock the mutex on the msg queue before accessing it. */
    pthread_mutex_lock (&comm->msg_queue_mutex);

    /* Place the new msg at the tail of the queue. */
    new_msg->prev = msg_queue->prev;
    new_msg->prev->next = new_msg;
    new_msg->next = msg_queue;
    msg_queue->prev = new_msg;

    /* Post to the semaphore to indicate that another msg is available. If
     * threads are blocked, waiting on the semaphore, one will become
     * unblocked so it can process the msg_queue.
     */
    sem_post (&comm->msg_queue_count);

    /* Unlock the msg queue mutex. */
    pthread_mutex_unlock (&comm->msg_queue_mutex);

    return 0;
}

int RmSysctrlCommEnQueueMsgX(struct RmSysctrlComm *comm,
                         char mtype,
                         char src_module_id,
                         char src_system_id,
                         void *data,
                         unsigned int len)
{
    struct RmSysctrlMessage rscm;
    memset(&rscm, 0, sizeof(struct RmSysctrlMessage));
    rscm.mtype = mtype;
    rscm.src_module_id = src_module_id;
    rscm.src_system_id = src_system_id;
    memcpy(&rscm.tlv, data, len);
    return RmSysctrlCommEnQueueMsg(comm, &rscm, len + sizeof(long));
}

static int RmSysctrlCommMatchCommonMsg(char src_module_id,
                                       char tlv_type,
                                       char *mtype)
{
    char src_system_id = (src_module_id & 0xF0) >> 4;
    switch (tlv_type)
    {
        case COMMON_SYSCTRL_INIT_CMD_REP:
        {
            *mtype = COMMON_SYSCTRL_INIT_CMD_REP_MSG;
            break;
        }
        case COMMON_SYSCTRL_SLEEP_CMD:
        {
            if (src_module_id != RM_SCHEDULER)
            {
                RLOGE("The COMMON_SYSCTRL_SLEEP_CMD "
                      "should come from RmSchedule.");
                return -1;
            }
            *mtype = COMMON_SYSCTRL_SLEEP_CMD_MSG;
            break;
        }
        case COMMON_SYSCTRL_SLEEP_CMD_REP:
        {
            *mtype = COMMON_SYSCTRL_SLEEP_CMD_REP_MSG;
            break;
        }
        case COMMON_SYSCTRL_WAKE_CMD:
        {
            if (src_module_id != RM_SCHEDULER)
            {
                RLOGE("The COMMON_SYSCTRL_WAKE_CMD "
                      "should come from RmSchedule.");
                return -1;
            }
            *mtype = COMMON_SYSCTRL_WAKE_CMD_MSG;
            break;
        }
        case COMMON_SYSCTRL_WAKE_CMD_REP:
        {
            *mtype = COMMON_SYSCTRL_WAKE_CMD_REP_MSG;
            break;
        }
        case COMMON_SYSCTRL_SECURITY_CMD:
        {
            if (src_module_id != RM_SCHEDULER)
            {
                RLOGE("The COMMON_SYSCTRL_SECURITY_CMD "
                      "should come from RmSchedule.");
                return -1;
            }
            *mtype = COMMON_SYSCTRL_SECURITY_CMD_MSG;
            break;
        }
        case COMMON_SYSCTRL_SECURITY_CMD_REP:
        {
            *mtype = COMMON_SYSCTRL_SECURITY_CMD_REP_MSG;
            break;
        }
        case COMMON_SYSCTRL_EXIT_SECURITY_CMD:
        {
            if (src_module_id != RM_SCHEDULER)
            {
                RLOGE("The COMMON_SYSCTRL_EXIT_SECURITY_CMD "
                      "should come from RmSchedule.");
                return -1;
            }
            *mtype = COMMON_SYSCTRL_EXIT_SECURITY_CMD_MSG;
            break;
        }
        case COMMON_SYSCTRL_EXIT_SECURITY_CMD_REP:
        {
            *mtype = COMMON_SYSCTRL_EXIT_SECURITY_CMD_REP_MSG;
            break;
        }
        case COMMON_SYSCTRL_HALT_CMD:
        {
            if (src_module_id != RM_SCHEDULER)
            {
                RLOGE("The COMMON_SYSCTRL_HALT_CMD "
                      "should come from RmSchedule.");
                return -1;
            }
            *mtype = COMMON_SYSCTRL_HALT_CMD_MSG;
            break;
        }
        case COMMON_SYSCTRL_HALT_CMD_REP:
        {
            *mtype = COMMON_SYSCTRL_HALT_CMD_REP_MSG;
            break;
        }
        case COMMON_SYSCTRL_DEBUG_CMD:
        {
            if (src_module_id != RM_SCHEDULER)
            {
                RLOGE("The COMMON_SYSCTRL_DEBUG_CMD "
                      "should come from RmSchedule.");
                return -1;
            }
            *mtype = COMMON_SYSCTRL_DEBUG_CMD_MSG;
            break;
        }
        case COMMON_SYSCTRL_DEBUG_CMD_REP:
        {
            *mtype = COMMON_SYSCTRL_DEBUG_CMD_REP_MSG;
            break;
        }
        case COMMON_SYSCTRL_MODE_CMD:
        {
            if (src_module_id != RM_SCHEDULER)
            {
                RLOGE("The COMMON_SYSCTRL_MODE_CMD "
                      "should come from RmSchedule.");
                return -1;
            }
            *mtype = COMMON_SYSCTRL_MODE_CMD_MSG;
            break;
        }
        case COMMON_SYSCTRL_MODE_CMD_REP:
        {
            *mtype = COMMON_SYSCTRL_MODE_CMD_REP_MSG;
            break;
        }
        case COMMON_SYSCTRL_ABNORMAL_OCCUR_NOTI:
        {
            *mtype = COMMON_SYSCTRL_ABNORMAL_OCCUR_NOTI_MSG;
            break;
        }
        case COMMON_SYSCTRL_ABNORMAL_RESOLVE_NOTI:
        {
            *mtype = COMMON_SYSCTRL_ABNORMAL_RESOLVE_NOTI_MSG;
            break;
        }
        case COMMON_SYSCTRL_LOW_POWER_NOTI:
        {
            if (src_module_id != RM_SCHEDULER)
            {
                RLOGE("The COMMON_SYSCTRL_LOW_POWER_NOTI "
                      "should come from RmSchedule.");
                return -1;
            }
            *mtype = COMMON_SYSCTRL_LOW_POWER_NOTI_MSG;
            break;
        }
        case COMMON_SYSCTRL_POWER_CHARGE_NOTI:
        {
            if (src_system_id != RP_SYSTEM_ID)
            {
                RLOGE("The COMMON_SYSCTRL_POWER_CHARGE_NOTI "
                      "should come from Rp.");
                return -1;
            }
            *mtype = COMMON_SYSCTRL_POWER_CHARGE_NOTI_MSG;
            break;
        }
        case COMMON_SYSCTRL_POWER_CHARGE_END_NOTI:
        {
            if (src_system_id != RP_SYSTEM_ID)
            {
                RLOGE("The COMMON_SYSCTRL_POWER_CHARGE_END_NOTI "
                      "should come from Rp.");
                return -1;
            }
            *mtype = COMMON_SYSCTRL_POWER_CHARGE_END_NOTI_MSG;
            break;
        }
        case COMMON_SYSCTRL_SUBSYS_STATE_CHANGE_NOTI:
        {
            *mtype = COMMON_SYSCTRL_SUBSYS_STATE_CHANGE_NOTI_MSG;
            break;
        }
        case COMMON_SYSCTRL_SUBSYS_STATE_QUERY_REP:
        {
            *mtype = COMMON_SYSCTRL_SUBSYS_STATE_QUERY_REP_MSG;
            break;
        }
        case COMMON_SYSCTRL_VERSION_QUERY_REP:
        {
            *mtype = COMMON_SYSCTRL_VERSION_QUERY_REP_MSG;
            break;
        }
        case COMMON_SYSCTRL_UPGRADE_CMD:
        {
            if (src_module_id != RM_UPGRADE)
            {
                RLOGE("The COMMON_SYSCTRL_UPGRADE_CMD "
                      "should come from RmUpgrade.");
                return -1;
            }
            *mtype = COMMON_SYSCTRL_UPGRADE_CMD_MSG;
            break;
        }
        case COMMON_SYSCTRL_UPGRADE_CMD_REP:
        {
            *mtype = COMMON_SYSCTRL_UPGRADE_CMD_REP_MSG;
            break;
        }
        case COMMON_SYSCTRL_EXIT_UPGRADE_CMD:
        {
            if (src_module_id != RM_UPGRADE)
            {
                RLOGE("The COMMON_SYSCTRL_EXIT_UPGRADE_CMD "
                      "should come from RmUpgrade.");
                return -1;
            }
            *mtype = COMMON_SYSCTRL_EXIT_UPGRADE_CMD_MSG;
            break;
        }
        case COMMON_SYSCTRL_EXIT_UPGRADE_CMD_REP:
        {
            *mtype = COMMON_SYSCTRL_EXIT_UPGRADE_CMD_REP_MSG;
            break;
        }
        case COMMON_SYSCTRL_NOT_LOW_POWER_NOTI:
        {
            if (src_module_id != RM_SCHEDULER)
            {
                RLOGE("The COMMON_SYSCTRL_NOT_LOW_POWER_NOTI "
                      "should come from RmSchedule.");
                return -1;
            }
            *mtype = COMMON_SYSCTRL_NOT_LOW_POWER_NOTI_MSG;
            break;
        }
        default:
        {
            RLOGE("Invalid message!");
            return -1;
        }
    }
    return 0;
}

static int RmSysctrlCommMatchRmMsg(char tlv_type, char *mtype)
{
    switch (tlv_type)
    {
        case RM_SYSCTRL_REGISTER_CMD_REP:
        {
            *mtype = RM_SYSCTRL_REGISTER_CMD_REP_MSG;
            break;
        }
        case RM_SYSCTRL_LOGIN_CMD_REP:
        {
            *mtype = RM_SYSCTRL_LOGIN_CMD_REP_MSG;
            break;
        }
        case RM_SYSCTRL_RESTORE_FACTORY_CMD:
        {
            *mtype = RM_SYSCTRL_RESTORE_FACTORY_CMD_MSG;
            break;
        }
        case RM_SYSCTRL_REGISTERED_NOTI:
        {
            *mtype = RM_SYSCTRL_REGISTERED_NOTI_MSG;
            break;
        }
        case RM_SYSCTRL_LOGIN_NOTI:
        {
            *mtype = RM_SYSCTRL_LOGIN_NOTI_MSG;
            break;
        }
        case RM_SYSCTRL_BATTERY_QUERY:
        {
            *mtype = RM_SYSCTRL_BATTERY_QUERY_MSG;
            break;
        }
        case RM_SYSCTRL_REGISTERED_QUERY_REP:
        {
            *mtype = RM_SYSCTRL_REGISTERED_QUERY_REP_MSG;
            break;
        }
        case RM_SYSCTRL_LOGIN_QUERY_REP:
        {
            *mtype = RM_SYSCTRL_LOGIN_QUERY_REP_MSG;
            break;
        }
        default:
        {
            RLOGE("Invalid Rm %02x message!", tlv_type);
            return -1;
        }
    }
    return 0;
}

static int RmSysctrlCommMatchRpMsg(char tlv_type, char *mtype)
{
    switch (tlv_type)
    {
        case RP_SYSCTRL_POWER_OFF_CMD_REP:
        {
            *mtype = RP_SYSCTRL_POWER_OFF_CMD_REP_MSG;
            break;
        }
        case RP_SYSCTRL_POWER_ON_CMD_REP:
        {
            *mtype = RP_SYSCTRL_POWER_ON_CMD_REP_MSG;
            break;
        }
        case RP_SYSCTRL_BATTERY_NOTI:
        {
            *mtype = RP_SYSCTRL_BATTERY_NOTI_MSG;
            break;
        }
        case RP_SYSCTRL_POWER_OFF_NOTI:
        {
            *mtype = RP_SYSCTRL_POWER_OFF_NOTI_MSG;
            break;
        }
        case RP_SYSCTRL_POWER_ON_NOTI:
        {
            *mtype = RP_SYSCTRL_POWER_ON_NOTI_MSG;
            break;
        }
        case RP_SYSCTRL_BATTERY_STATUS_QUERY_REP:
        {
            *mtype = RP_SYSCTRL_BATTERY_STATUS_QUERY_REP_MSG;
            break;
        }
        case RP_SYSCTRL_GET_CPU_ID_CMD_REP:
        {
            *mtype = RP_SYSCTRL_GET_CPU_ID_CMD_REP_MSG;
            break;
        }
        case RP_SYSCTRL_SAVE_ROBOT_SN_CMD_REP:
        {
            *mtype = RP_SYSCTRL_SAVE_ROBOT_SN_CMD_REP_MSG;
            break;
        }
        case RP_SYSCTRL_READ_ROBOT_SN_CMD_REP:
        {
            *mtype = RP_SYSCTRL_READ_ROBOT_SN_CMD_REP_MSG;
            break;
        }
        case RP_SYSCTRL_DATETIME_QUERY_REP:
        {
            *mtype = RP_SYSCTRL_DATETIME_QUERY_REP_MSG;
            break;
        }
        case RP_SYSCTRL_SET_DATETIME_CMD_REP:
        {
            *mtype = RP_SYSCTRL_SET_DATETIME_CMD_REP_MSG;
            break;
        }
        case RP_SYSCTRL_SEX_QUERY_REP:
        {
            *mtype = RP_SYSCTRL_SEX_QUERY_REP_MSG;
            break;
        }
        case RP_SYSCTRL_SET_SEX_CMD_REP:
        {
            *mtype = RP_SYSCTRL_SET_SEX_CMD_REP_MSG;
            break;
        }
        case RP_SYSCTRL_UPTIME_QUERY_REP:
        {
            *mtype = RP_SYSCTRL_UPTIME_QUERY_REP_MSG;
            break;
        }
        default:
        {
            RLOGE("Invalid Rp %02x message!", tlv_type);
            return -1;
        }
    }
    return 0;
}

static int RmSysctrlCommMatchRfMsg(char tlv_type, char *mtype)
{
    switch (tlv_type)
    {
        case RF_SYSCTRL_WAKE_NOTI:
        {
            *mtype = RF_SYSCTRL_WAKE_NOTI_MSG;
            break;
        }
        default:
        {
            RLOGE("Invalid Rf %02x message!", tlv_type);
            return -1;
        }
    }
    return 0;
}

static int RmSysctrlCommMatchMsg(char src_module_id,
                                 char tlv_type,
                                 char *mtype)
{
    char src_system_id = (src_module_id & 0xF0) >> 4;
    if (src_module_id == RM_SYSCTRL)
    {
        /* Internal message */
        switch (tlv_type)
        {
            case INTERNAL_SYSCTRL_SUBSYS_DEAD:
            {
                *mtype = INTERNAL_SYSCTRL_SUBSYS_DEAD_MSG;
                break;
            }
            case INTERNAL_SYSCTRL_TIMER_EXPIRE:
            {
                *mtype = INTERNAL_SYSCTRL_TIMER_EXPIRE_MSG;
                break;
            }
#ifdef DEEP_SLEEP_SUPPORT
            case INTERNAL_SYSCTRL_S_POWERON_CHANGE:
            {
                *mtype = INTERNAL_SYSCTRL_S_POWERON_CHANGE_MSG;
                break;
            }
#endif
            default:
            {
                RLOGE("Invalid message!");
                return -1;
            }
        }
    }
    else
    {
        /* External message */
        int ret = -1;
        if (tlv_type > 0 && tlv_type < 63)
        {
            /* Common messages */
            ret = RmSysctrlCommMatchCommonMsg(src_module_id, tlv_type, mtype);
            if (ret < 0)
            {
                RLOGE("Match Common Message failed.");
                return -1;
            }
        }
        else
        {
            /* Only related to sub-system messages */
            if (src_module_id == RM_SCHEDULER || src_module_id == RM_POWER)
            {
                ret = RmSysctrlCommMatchRmMsg(tlv_type, mtype);
                if (ret < 0)
                {
                    RLOGE("Match Rm Message failed.");
                    return -1;
                }
            }
            else if (src_system_id == RP_SYSTEM_ID)
            {
                ret = RmSysctrlCommMatchRpMsg(tlv_type, mtype);
                if (ret < 0)
                {
                    RLOGE("Match Rp Message failed.");
                    return -1;
                }
            }
            else if (src_system_id == RF_SYSTEM_ID)
            {
                ret = RmSysctrlCommMatchRfMsg(tlv_type, mtype);
                if (ret < 0)
                {
                    RLOGE("Match Rf Message failed.");
                    return -1;
                }
            }
            else
            {
                RLOGE("Unknown sub-system message!");
                return -1;
            }            
        }
    }

    return 0;
}

static void RmSysctrlCommLogMsg(char src_module_id,
                               char tlv_type)
{
    char src_system_id = (src_module_id & 0xF0) >> 4;
    if (src_module_id != RM_SYSCTRL)
    {
        /* External message */
        int ret = -1;
        if (tlv_type > COMMON_SYSCTRL_MSG_BEGIN &&
            tlv_type < COMMON_SYSCTRL_MSG_END)
        {
            RLOGI("Received \"%s\" message",
                rm_sysctrl_common_msg_str[tlv_type - 1]);
        }
        else
        {
            /* Only related to sub-system messages */
            if (src_module_id == RM_SCHEDULER ||src_module_id == RM_POWER)
            {
                int base = RM_SYSCTRL_MSG_BEGIN;

                if (tlv_type > RM_SYSCTRL_MSG_BEGIN &&
                    tlv_type < RM_SYSCTRL_MSG_END)
                {
                    RLOGI("Received \"%s\" message",
                        rm_sysctrl_rm_msg_str[tlv_type - base - 1]);
                }
            }
            else if (src_system_id == RP_SYSTEM_ID)
            {
                int base = RP_SYSCTRL_MSG_BEGIN;

                if (tlv_type > RP_SYSCTRL_MSG_BEGIN &&
                    tlv_type < RP_SYSCTRL_MSG_END)
                {
                    RLOGI("Received \"%s\" message",
                        rm_sysctrl_rp_msg_str[tlv_type - base - 1]);
                }
            }
            else if (src_system_id == RF_SYSTEM_ID)
            {
                int base = RF_SYSCTRL_MSG_BEGIN;

                if (tlv_type > RF_SYSCTRL_MSG_BEGIN &&
                    tlv_type < RF_SYSCTRL_MSG_END)
                {
                    RLOGI("Received \"%s\" message",
                        rm_sysctrl_rf_msg_str[tlv_type - base - 1]);
                }
            }
            else
            {
                RLOGE("Unknown sub-system message!");
            }            
        }

        RLOGI("  from %s sub-system 0x%02x module",            
            RmSysctrlGetSubSystemStr((int)src_system_id),
            src_module_id);
    }
}

int RmSysctrlCommEnQueueMsgXX(struct RmSysctrlComm *comm,
                         char src_module_id,
                         void *data,
                         unsigned int len)
{
    int ret = -1;
    char *tlv = data;
    char tlv_type = 0;
    char tlv_len = 0;
    char mtype = 0;

    if (len >= 2)
    {
        tlv_type = tlv[0];
        tlv_len  = tlv[1];
    }

    //RLOGD("len%d,tlv_type:%d,tlv_len:%d", len, tlv_type, tlv_len);
    /* To check if the message is validate */
    if (len < 2 || len != tlv_len)
    {
        RLOGE("The length of message received from %d sub-system is invalid.",
            src_module_id);
        return -1;
    }

    /* Log received message for debug */
    RmSysctrlCommLogMsg(src_module_id, tlv_type);

    /* To match the message into local message */
    ret = RmSysctrlCommMatchMsg(src_module_id, tlv_type, &mtype);
    if (ret < 0)
    {
        RLOGE("Match the message into local message failed.");
        return -1;
    }

    return RmSysctrlCommEnQueueMsgX(comm,
                                 mtype,
                                 src_module_id,
                                 (src_module_id & 0xF0) >> 4,
                                 data,
                                 len);
}

int RmSysctrlCommEnQueueMsgFromCan(struct RmSysctrlComm *comm,
                         void *can_packet)
{
    int priority = 0;
    int s = 0;
    int d = 0;
    char *pData = NULL;
    int len = 0;
    struct RmSysctrlCanPacket *cp = NULL;

    cp = (struct RmSysctrlCanPacket *)can_packet;
    priority = cp->priority;
    s = cp->s;
    d = cp->d;
    pData = cp->pData;
    len = cp->len;

    if (d != RM_SYSCTRL)
    {
        RLOGE("The destination of message is not RmSysctrl.");
        return -1;
    }

    return RmSysctrlCommEnQueueMsgXX(comm, s, pData, len);
}

int RmSysctrlCommSendSubsysDead(struct RmSysctrlComm *comm,
                         unsigned int subsys_id)
{
    struct RmSysctrlTLV tlv;

    if (!comm || subsys_id > SUB_SYSTEM_ID_MAX)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }
    memset(&tlv, 0, sizeof(struct RmSysctrlTLV));
    tlv.type = INTERNAL_SYSCTRL_SUBSYS_DEAD;
    tlv.value[0] = subsys_id;
    tlv.len = 3;

    return RmSysctrlCommEnQueueMsgXX(comm, RM_SYSCTRL, &tlv, tlv.len);
}

int RmSysctrlCommSendTimerExpire(struct RmSysctrlComm *comm,
                         unsigned int timer_id)
{
    struct RmSysctrlTLV tlv;

    if (!comm || timer_id > TIMER_MAX)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }
    memset(&tlv, 0, sizeof(struct RmSysctrlTLV));
    tlv.type = INTERNAL_SYSCTRL_TIMER_EXPIRE;
    tlv.value[0] = timer_id;
    tlv.len = 3;

    return RmSysctrlCommEnQueueMsgXX(comm, RM_SYSCTRL, &tlv, tlv.len);
}

#ifdef DEEP_SLEEP_SUPPORT
int RmSysctrlCommSendSPoweronChange(struct RmSysctrlComm *comm)
{
    struct RmSysctrlTLV tlv;

    if (!comm)
    {
        RLOGE("Invalid parameter!");
        return -1;
    }
    memset(&tlv, 0, sizeof(struct RmSysctrlTLV));
    tlv.type = INTERNAL_SYSCTRL_S_POWERON_CHANGE;
    tlv.len = 2;

    return RmSysctrlCommEnQueueMsgXX(comm, RM_SYSCTRL, &tlv, tlv.len);
}
#endif

int RmSysctrlCommSendMsgToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         void *data,
                         unsigned int len)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();
    char *data_l = data;

    if (priority > 6 )
    {
        RLOGE("Invalid priority.");
        return -1;
    }

    if (rm_sysctrl->state == ROBOT_STATUS_SLEEP &&
        data_l[0] != COMMON_SYSCTRL_SLEEP_CMD &&
        data_l[0] != COMMON_SYSCTRL_WAKE_CMD &&
        data_l[0] != COMMON_SYSCTRL_SYS_STATE_CHANGE_NOTI)
    {
        RLOGE("Invalid command, current system state is sleep.");
        return -1;
    }

    return RmSysctrlCanDataSend(priority,
                                RM_SYSCTRL,
                                dst_module_id,
                                data,
                                len);
}

int RmSysctrlCommSendInitCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = COMMON_SYSCTRL_INIT_CMD;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendSysStateChangeNotiToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         unsigned int state)
{
    char len = 0;
    char data[3] = {0};

    data[0] = COMMON_SYSCTRL_SYS_STATE_CHANGE_NOTI;
    data[1] = len = 3;
    data[2] = state;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendRegQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = RM_SYSCTRL_REGISTERED_QUERY;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendRegCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = RM_SYSCTRL_REGISTER_CMD;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendLoginQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = RM_SYSCTRL_LOGIN_QUERY;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendLoginCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = RM_SYSCTRL_LOGIN_CMD;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendHaltCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = COMMON_SYSCTRL_HALT_CMD;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendPoweroffCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         char high,
                         char low)
{
    char len = 0;
    char data[4] = {0};

    data[0] = RP_SYSCTRL_POWER_OFF_CMD;
    data[1] = len = 4;
    data[2] = high;
    data[3] = low;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendPoweronCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         char high,
                         char low)
{
    char len = 0;
    char data[4] = {0};

    data[0] = RP_SYSCTRL_POWER_ON_CMD;
    data[1] = len = 4;
    data[2] = high;
    data[3] = low;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendChargeNotiToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = COMMON_SYSCTRL_POWER_CHARGE_NOTI;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendExitChargeNotiToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = COMMON_SYSCTRL_POWER_CHARGE_END_NOTI;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendSecurityCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = COMMON_SYSCTRL_SECURITY_CMD;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendExitSecurityCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = COMMON_SYSCTRL_EXIT_SECURITY_CMD;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendWakeCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = COMMON_SYSCTRL_WAKE_CMD;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendSleepCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         char sleep_level)
{
    char len = 0;
    char data[3] = {0};

    data[0] = COMMON_SYSCTRL_SLEEP_CMD;
    data[1] = len = 3;
    data[2] = sleep_level;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendDebugCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         unsigned int debug_en)
{
    char len = 0;
    char data[3] = {0};

    data[0] = COMMON_SYSCTRL_DEBUG_CMD;
    data[1] = len = 3;
    data[2] = debug_en;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendModeCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         unsigned int mode)
{
    char len = 0;
    char data[3] = {0};

    data[0] = COMMON_SYSCTRL_MODE_CMD;
    data[1] = len = 3;
    data[2] = mode;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendLowPowerNotiToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = COMMON_SYSCTRL_LOW_POWER_NOTI;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendNotLowPowerNotiToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = COMMON_SYSCTRL_NOT_LOW_POWER_NOTI;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendBatteryQueryRespToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         unsigned int battery_power)
{
    char len = 0;
    char data[3] = {0};

    data[0] = RM_SYSCTRL_BATTERY_QUERY_REP;
    data[1] = len = 3;
    data[2] = battery_power;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendRestoreFactoryCmdRespToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         int result)
{
    char len = 0;
    char data[3] = {0};

    data[0] = RM_SYSCTRL_RESTORE_FACTORY_CMD_REP;
    data[1] = len = 3;
    data[2] = result;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendSubsysStateQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = COMMON_SYSCTRL_SUBSYS_STATE_QUERY;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendBatteryStatusQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = RP_SYSCTRL_BATTERY_STATUS_QUERY;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendVersionQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = COMMON_SYSCTRL_VERSION_QUERY;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendGetCPUIDCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = RP_SYSCTRL_GET_CPU_ID_CMD;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendReadSNCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = RP_SYSCTRL_READ_ROBOT_SN_CMD;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendDatetimeQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = RP_SYSCTRL_DATETIME_QUERY;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendSetDatetimeCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         void *datetime)
{
    char len = 0;
    char data[9] = {0};
    struct RmSysctrlDatetime *dt = NULL;

    if (!datetime) return -1;
    dt = (struct RmSysctrlDatetime *) datetime;
    data[0] = RP_SYSCTRL_SET_DATETIME_CMD;
    data[1] = len = 9;
    data[2] = dt->year;
    data[3] = dt->month;
    data[4] = dt->day;
    data[5] = dt->weekday;
    data[6] = dt->hour;
    data[7] = dt->minute;
    data[8] = dt->second;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendSexQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = RP_SYSCTRL_SEX_QUERY;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendSetSexCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         unsigned short sex)
{
    char len = 0;
    char data[3] = {0};

    if (sex != MALE && sex != FEMALE)
    {
        RLOGE("Invalid sex to set.");
        return -1;
    }
    data[0] = RP_SYSCTRL_SET_SEX_CMD;
    data[1] = len = 3;
    data[2] = sex;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendUpgradeCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = COMMON_SYSCTRL_UPGRADE_CMD;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendExitUpgradeCmdToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = COMMON_SYSCTRL_EXIT_UPGRADE_CMD;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendUptimeQueryToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id)
{
    char len = 0;
    char data[2] = {0};

    data[0] = RP_SYSCTRL_UPTIME_QUERY;
    data[1] = len = 2;

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}

int RmSysctrlCommSendHDMIStateChangeNotiToCan(struct RmSysctrlComm *comm,
                         unsigned int priority,
                         char dst_module_id,
                         bool is_plug_in)
{
    char len = 0;
    char data[3] = {0};

    data[0] = COMMON_SYSCTRL_HDMI_STATE_CHANGE_NOTI;
    data[1] = len = 3;
    if (is_plug_in)
    {
        data[2] = 1;
    }
    else
    {
        data[2] = 0;
    }

    return RmSysctrlCommSendMsgToCan(comm,
                                     priority,
                                     dst_module_id,
                                     data,
                                     len);
}
