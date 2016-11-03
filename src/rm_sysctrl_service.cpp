/*************************************************************************
FileName: rm_sysctrl_service.cpp
Copy Right: Copyright (C), 2006-2014, Archermind. Co., Ltd.
System: RM
Module: SYSCTRL
Author: wangkun
Create Date: 2015-5-19
**************************************************************************/
/*************************************************************************
Editor: wangkun
Version: v1.0 init version
Edit Date: 2014-5-19
**************************************************************************/
#include <binder/IServiceManager.h>
#include <binder/IBinder.h>
#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <semaphore.h>

#include "rm_sysctrl.h"
#include "rm_sysctrl_cli.h"
#include "rm_sysctrl_fault.h"
#include "rm_sysctrl_log.h"
#include "rm_sysctrl_comm.h"
#include "rm_sysctrl_subsys.h"
#include "rm_sysctrl_fsm.h"
#include "rm_sysctrl_can.h"
#ifdef DEEP_SLEEP_SUPPORT
#include "rm_sysctrl_gpio.h"
#endif
#include "rm_sysctrl_vbdev.h"
#include "rm_sysctrl_conf.h"
#include "rm_sysctrl_service.h"

using namespace android;
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RM_SYSCTRL_SRV"

#ifdef SUB_SYS_STATUS_QUERY_IN_DEMAND_SUPPORT
static unsigned int sub_sys_status_update_need;
pthread_mutex_t sssun_Mutex;
static void RmSysctrlInitNeedUpdateSubSysStatus()
{
    pthread_mutex_init(&sssun_Mutex, NULL);
    sub_sys_status_update_need = 0;
}

static void RmSysctrlSetNeedUpdateSubSysStatus(unsigned int sssun)
{
    pthread_mutex_lock(&sssun_Mutex);
    sub_sys_status_update_need = sssun;
    pthread_mutex_unlock(&sssun_Mutex);
}

static int RmSysctrlGetNeedUpdateSubSysStatus()
{
    unsigned int sssun;
    pthread_mutex_lock(&sssun_Mutex);
    sssun = sub_sys_status_update_need;
    pthread_mutex_unlock(&sssun_Mutex);
    return sssun;
}

void RmSysctrlDecreaseNeedUpdateSubSysStatus()
{
    pthread_mutex_lock(&sssun_Mutex);
    if (sub_sys_status_update_need > 0)
        sub_sys_status_update_need--;
    else
        sub_sys_status_update_need = 0;
    pthread_mutex_unlock(&sssun_Mutex);
}

static unsigned int enable_subsys_number(struct RmSysctrl *rm_sysctrl)
{
    int i = 0;
    unsigned int enable_subsyses = 0;

    for (i = 0; i < SUB_SYSTEM_ID_MAX; i++)
    {
        if (IS_SUBSYSTEM_ENABLE(&rm_sysctrl->sub_system[i]))
        {
            enable_subsyses++;
        }
    }
    return enable_subsyses;
}
#endif

static int RmSysctrlGetSubSysVersion(int sub_sys, char *ver)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();
    struct RmSysctrlSubSystem *subsys;
    char tmp_str[200] = {0};

    if (sub_sys > SUB_SYSTEM_ID_MAX)
    {
        RLOGE("Invalid sub-system id.");
        return -1;
    }
    if (!ver)
    {
        RLOGE("Invalid parameter ver that is NULL!");
        return -1;
    }
    subsys = &rm_sysctrl->sub_system[sub_sys];
    memset(tmp_str, 0, sizeof(tmp_str));
    if (subsys->ver_str_used)
    {
        sprintf(tmp_str, "%s", subsys->u.ver_str);
    }
    else
    {
        sprintf(tmp_str, "%d.%d.%d.%d.%d%02d%02d.%d",
            subsys->u.ver.sub_sys_id,
            subsys->u.ver.major,
            subsys->u.ver.minor,
            subsys->u.ver.revision,
            subsys->u.ver.building_year,
            subsys->u.ver.building_month,
            subsys->u.ver.building_day,
            subsys->u.ver.rom_id);
    }
    strcpy(ver, tmp_str);
    return 0;
}

static int RmSysctrlGetSN(char *sn, bool *is_valid)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();

    if (!sn || !is_valid)
    {
        RLOGE("Invalid parameter sn that is NULL!");
        return -1;
    }
    strcpy(sn, rm_sysctrl->sn);
    *is_valid = rm_sysctrl->is_valid_sn;
    return 0;
}

static int RmSysctrlIsInMotion(bool *is_in_motion)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();

    if (!is_in_motion)
    {
        RLOGE("Invalid parameter is_in_motion that is NULL!");
        return -1;
    }
#ifdef SUB_SYS_STATUS_QUERY_IN_DEMAND_SUPPORT
    {
        unsigned int sssun = enable_subsys_number(rm_sysctrl);
        /* 1. To set the sub-system numbers to wait their status */
        RmSysctrlSetNeedUpdateSubSysStatus(sssun);
        /* 2. Query the sub-system status */
        /* Broadcast query sub-system state message into can */
        RmSysctrlBroadcastSubsysStateQueryMsg(rm_sysctrl);
        /* 3. To wait all sub-system status updated */
        int retry = 20;
        sssun = RmSysctrlGetNeedUpdateSubSysStatus();
        RLOGD("sssun:%u", sssun);
        while (sssun > 0)
        {
            if (retry-- <= 0) break;
            usleep(100000); //sleep 100ms
            sssun = RmSysctrlGetNeedUpdateSubSysStatus();
            RLOGD("sssun:%u", sssun);
        }
        if (retry <= 0)
        {
            RLOGD("Time out to wait all sub-system status updated,");
            RLOGD("  the returned status is not in real time.");
        }
    }
#endif
    if (!CHECK_SUBSTATE(&rm_sysctrl->sub_system[RC_SYSTEM_ID], WORK_IDLE) ||
        !CHECK_SUBSTATE(&rm_sysctrl->sub_system[RB_R_ARM_ID], WORK_IDLE) ||
        !CHECK_SUBSTATE(&rm_sysctrl->sub_system[RB_L_ARM_ID], WORK_IDLE) ||
        !CHECK_SUBSTATE(&rm_sysctrl->sub_system[RB_BODY_ID], WORK_IDLE))
    {
        *is_in_motion = true;
    }
    else
    {
        *is_in_motion = false;
    }
    return 0;
}

static int RmSysctrlGetSubSysState(
    int sub_sys,
    bool *enable,
    bool *poweron,
    bool *active,
    unsigned int *state,
    unsigned int *sub_state,
    bool *is_initalized,
    bool *debug_en,
    unsigned int *mode)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();
    struct RmSysctrlSubSystem *subsys;
    char tmp_str[200] = {0};

    if (sub_sys > SUB_SYSTEM_ID_MAX)
    {
        RLOGE("Invalid sub-system id.");
        return -1;
    }
    if (!enable || !poweron || !active || !state ||
        !sub_state || !is_initalized || !debug_en || !mode)
    {
        RLOGE("Invalid parameter that is NULL!");
        return -1;
    }
    subsys = &rm_sysctrl->sub_system[sub_sys];
    *enable = subsys->enable;
    *poweron = subsys->poweron;
    *active = subsys->active;
    *state = subsys->state;
    *sub_state = subsys->sub_state;
    *is_initalized = subsys->is_initalized;
    *debug_en = subsys->debug_en;
    *mode = subsys->mode;
    return 0;
}

static int RmSysctrlGetSex(int *sex)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();

    if (!sex)
    {
        RLOGE("Invalid parameter that is NULL!");
        return -1;
    }

    *sex = rm_sysctrl->sex;
    return 0;
}

static int RmSysctrlSetSex(int sex)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();

    if (sex != MALE && sex != FEMALE)
    {
        RLOGE("Invalid sex %d to set!", sex);
        return -1;
    }

    if (sex != rm_sysctrl->sex)
    {
        rm_sysctrl->sex = (enum RmSysctrlSex)sex;
        RmSysctrlCommSendSetSexCmdToCan(rm_sysctrl->comm,
             MIDDLE, RP_SYSCTRL, sex);
    }
    return 0;
}

static int RmSysctrlGetDatetime(int *year, int *month, int *day, int *weekday,
    int *hour, int *minute, int *second)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();
    struct RmSysctrlDatetime *datetime = &rm_sysctrl->datetime;

    if (!year || !month || !day || !weekday || !hour || !minute || !second)
    {
        RLOGE("Invalid parameter that is NULL!");
        return -1;
    }

    RLOGI("Send Datetime Query to Rp and wait 3 seconds for updating.");
    RmSysctrlCommSendDatetimeQueryToCan(rm_sysctrl->comm,
             MIDDLE, RP_SYSCTRL);
    sleep(3);
    *year = datetime->year + 2000;
    *month = datetime->month;
    *day = datetime->day;
    *weekday = datetime->weekday;
    *hour = datetime->hour;
    *minute = datetime->minute;
    *second = datetime->second;
    return 0;
}

static int RmSysctrlSetDatetime(int year, int month, int day, int weekday,
    int hour, int minute, int second)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();
    struct RmSysctrlDatetime *datetime = &rm_sysctrl->datetime;

    if (year < 2000)
    {
        RLOGE("Invalid year %d to set!", year);
        return -1;
    }
    if (month > 12 || month < 1)
    {
        RLOGE("Invalid month %d to set!", month);
        return -1;
    }
    if (day > 31 || day < 1)
    {
        RLOGE("Invalid day %d to set!", day);
        return -1;
    }
    if (weekday > 6 || weekday < 0)
    {
        RLOGE("Invalid weekday %d to set!", weekday);
        return -1;
    }
    if (hour > 23 || hour < 0)
    {
        RLOGE("Invalid hour %d to set!", hour);
        return -1;
    }
    if (minute > 59 || minute < 0)
    {
        RLOGE("Invalid minute %d to set!", minute);
        return -1;
    }
    if (second > 59 || second < 0)
    {
        RLOGE("Invalid second %d to set!", second);
        return -1;
    }

    if (!((year - 2000) == datetime->year &&
          month == datetime->month &&
          day == datetime->day &&
          weekday == datetime->weekday &&
          hour == datetime->hour &&
          minute == datetime->minute &&
          second == datetime->second
         ))
    {
        datetime->year = (char)(year - 2000);
        datetime->month = (char)month;
        datetime->day = (char)day;
        datetime->weekday = (char)weekday;
        datetime->hour = (char)hour;
        datetime->minute = (char)minute;
        datetime->second = (char)second;
        RmSysctrlCommSendSetDatetimeCmdToCan(rm_sysctrl->comm,
             MIDDLE, RP_SYSCTRL, datetime);

       //Update the datetime into system
       if (SetDatetimeIntoSystem(datetime) < 0)
       {
            RLOGD("Set the datetime into system failed.");
            return -1;
       }
    }
    return 0;
}

static int RmSysctrlGetUptime(int *uptime)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();

    if (!uptime)
    {
        RLOGE("Invalid parameter that is NULL!");
        return -1;
    }

    RLOGI("Send Uptime Query to Rp and wait 3 seconds for updating.");
    RmSysctrlCommSendUptimeQueryToCan(rm_sysctrl->comm,
             MIDDLE, RP_SYSCTRL);
    sleep(3);

    *uptime = rm_sysctrl->uptime;
    return 0;
}

class MyService : public BBinder
{
public:
    MyService()
    {
        mydescriptor = String16(SERVICE);
    }
    virtual ~MyService() {}
    //This function is used when call Parcel::checkInterface(IBinder*)
    virtual const String16& getInterfaceDescriptor() const
    {
        RLOGE("this is enter ==========getInterfaceDescriptor");
        return mydescriptor;
    }
protected:

    virtual status_t onTransact( uint32_t code,
        const Parcel& data,
        Parcel* reply,
        uint32_t flags = 0)
    {
        RLOGD("enter MyService onTransact and the code is %d",code);

        if (data.checkInterface(this))
            RLOGD("checkInterface OK");
        else
        {
            RLOGW("checkInterface failed");
            return BBinder::onTransact(code, data, reply, flags);
        }

        switch (code)
        {
        case SUB_SYSTEM_VERSION:
        {
            int ret = 0;
            int sub_sys = data.readInt32();
            char sub_ver[200] = {0};

            RLOGD("MyService interface %d sub_system version", code);
            ret = RmSysctrlGetSubSysVersion(sub_sys, sub_ver);
            reply->writeInt32(ret);
            reply->writeString8(String8(sub_ver));

            break;
        }
        break;
        case SERIAL_NUMBER:
        {
            int ret = 0;
            char sn[200] = {0};
            bool is_sn_valid;

            RLOGD("MyService interface %d serial number", code);
            ret = RmSysctrlGetSN(sn, &is_sn_valid);
            reply->writeInt32(ret);
            reply->writeString8(String8(sn));
            reply->writeInt32(is_sn_valid);
        }
        break;
        case IS_IN_MOTION:
        {
            int ret = 0;
            bool is_in_motion = false;

            RLOGD("MyService interface %d is in motion", code);
            ret = RmSysctrlIsInMotion(&is_in_motion);
            reply->writeInt32(ret);
            reply->writeInt32(is_in_motion);
        }
        break;
        case SUB_SYSTEM_STATE:
        {
            int ret = 0;
            int sub_sys = data.readInt32();
            bool enable = false;
            bool poweron = false;
            bool active = false;
            unsigned int state = 0;
            unsigned int sub_state = 0;
            bool is_initalized = false;
            bool debug_en = false;
            unsigned int mode = 0;

            RLOGD("MyService interface %d sub_system version", code);
            ret = RmSysctrlGetSubSysState(sub_sys, &enable, &poweron, &active,
                &state, &sub_state, &is_initalized, &debug_en, &mode);
            reply->writeInt32(ret);
            reply->writeInt32(enable);
            reply->writeInt32(poweron);
            reply->writeInt32(active);
            reply->writeInt32(state);
            reply->writeInt32(sub_state);
            reply->writeInt32(is_initalized);
            reply->writeInt32(debug_en);
            reply->writeInt32(mode);

            break;
        }
        case GET_SEX:
        {
            int ret = 0;
            int sex = 0;

            RLOGD("MyService interface %d get sex", code);
            ret = RmSysctrlGetSex(&sex);
            reply->writeInt32(ret);
            reply->writeInt32(sex);

            break;
        }
        case SET_SEX:
        {
            int ret = 0;
            int sex = data.readInt32();

            RLOGD("MyService interface %d set sex", code);
            ret = RmSysctrlSetSex(sex);
            reply->writeInt32(ret);

            break;
        }
        case GET_DATETIME:
        {
            int ret = 0;
            int year = 0;
            int month = 0;
            int day = 0;
            int weekday = 0;
            int hour = 0;
            int minute = 0;
            int second = 0;

            RLOGD("MyService interface %d get datetime", code);
            ret = RmSysctrlGetDatetime(&year, &month, &day, &weekday,
                &hour, &minute, &second);
            reply->writeInt32(ret);
            reply->writeInt32(year);
            reply->writeInt32(month);
            reply->writeInt32(day);
            reply->writeInt32(weekday);
            reply->writeInt32(hour);
            reply->writeInt32(minute);
            reply->writeInt32(second);

            break;
        }
        case SET_DATETIME:
        {
            int ret = 0;
            int year = data.readInt32();
            int month = data.readInt32();
            int day = data.readInt32();
            int weekday = data.readInt32();
            int hour = data.readInt32();
            int minute = data.readInt32();
            int second = data.readInt32();

            RLOGD("MyService interface %d set datetime", code);
            ret = RmSysctrlSetDatetime(year, month, day, weekday, hour,
                minute, second);
            reply->writeInt32(ret);

            break;
        }
        case GET_UPTIME:
        {
            int ret = 0;
            int uptime = 0;

            RLOGD("MyService interface %d get uptime", code);
            ret = RmSysctrlGetUptime(&uptime);
            reply->writeInt32(ret);
            reply->writeInt32(uptime);

            break;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
        }
        return 0;
    }
private:
    String16 mydescriptor;
};

void *RmSysctrlSrvThread(void *arg)
{
    sp<IServiceManager> sm = defaultServiceManager();
    status_t ret;
#ifdef SUB_SYS_STATUS_QUERY_IN_DEMAND_SUPPORT
    RmSysctrlInitNeedUpdateSubSysStatus();
#endif
    //register MyService to ServiceManager
    MyService* srv = new MyService();
    ret = sm->addService(String16(SERVICE), srv);
    RLOGD("addservice %s return %d", SERVICE, ret);
    //call binder thread pool to start
    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool(true);
    return arg;
}
