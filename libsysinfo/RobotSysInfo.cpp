#include "RobotSysInfo.h"

//for rm_sysctrl.h
#include <pthread.h>

#include "rm_sysctrl_service.h"
#include "rm_sysctrl.h"

#include <binder/IServiceManager.h>
#include <binder/IBinder.h>
#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>

#include <cutils/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#undef LOG_TAG
#define LOG_TAG "RobotSysInfo"

namespace android {

RobotSysInfo::RobotSysInfo()
{
    GetRemoteService();
}

RobotSysInfo::~RobotSysInfo()
{
    if (mRemoteService.get() != NULL)
        mRemoteService.clear();
}

int RobotSysInfo::GetRemoteService()
{
    sp<IServiceManager> sm = defaultServiceManager();

    int retry_count = 3;
    do
    {
        mRemoteService = sm->getService(String16(SERVICE));
        if (mRemoteService.get() != NULL) return 0;

        LOGE("get sub-sys info service failed, retry...");
        usleep(500 * 1000);

        --retry_count;
    } while (retry_count > 0);

    return -1;
}

bool RobotSysInfo::CheckService()
{
    if (mRemoteService.get() == NULL)
    {
        int ret = GetRemoteService();
        return ((ret == 0) ? true : false);
    }

    return true;
}

int RobotSysInfo::GetSubSysVersion(int sub_sys_id, char *version, size_t len)
{
    if (!CheckService()|| !version) return -1;

    Parcel data, reply;
    data.writeInterfaceToken(String16(SERVICE));
    data.writeInt32(sub_sys_id);

    mRemoteService->transact(SUB_SYSTEM_VERSION, data, &reply, 0);
    int ret = reply.readInt32();
    if (ret != 0)
    {
        LOGE("Get sub system version failed.");
        return -1;
    }

    String8 version_str = reply.readString8();
    int size = len > version_str.size() ? version_str.size() : len;
    memcpy(version, version_str.string(), size);
    LOGD("The %d sub-system version:%s", sub_sys_id, version);

    return 0;
}

int RobotSysInfo::GetRobotSN(bool &is_valid, char *sn, size_t len)
{
    if (!CheckService() || !sn) return -1;

    Parcel data, reply;
    data.writeInterfaceToken(String16(SERVICE));

    mRemoteService->transact(SERIAL_NUMBER, data, &reply, 0);
    int ret = reply.readInt32();
    if (ret != 0)
    {
        LOGE("Get serial number failed.");
        return -1;
    }

    String8 sn_str = reply.readString8();
    int size = len > sn_str.size() ? sn_str.size() : len;
    memcpy(sn, sn_str.string(), size);

    is_valid = (reply.readInt32() == 0) ? false : true;
    LOGD("sn: %s, is_valid: %d", sn, is_valid);

    return 0;
}

int RobotSysInfo::isRobotMoving(bool &is_moving)
{
    if (!CheckService()) return -1;

    Parcel data, reply;
    data.writeInterfaceToken(String16(SERVICE));

    mRemoteService->transact(IS_IN_MOTION, data, &reply, 0);
    int ret = reply.readInt32();
    if (ret != 0)
    {
        LOGE("Check if the robot is in motion failed.");
        return -1;
    }

    is_moving = (reply.readInt32() == 0) ? false : true;

    return 0;
}

int RobotSysInfo::GetSubSysState(int sub_sys_id, struct RobotSubSysState &state)
{
    if (!CheckService()) return -1;

    Parcel data, reply;
    data.writeInterfaceToken(String16(SERVICE));
    data.writeInt32(sub_sys_id);

    mRemoteService->transact(SUB_SYSTEM_STATE, data, &reply, 0);
    int ret = reply.readInt32();
    if (ret != 0)
    {
        LOGE("Get sub system state failed.");
        return -1;
    }

    state.is_enable   = (reply.readInt32() == 0) ? false : true;
    state.is_power_on = (reply.readInt32() == 0) ? false : true;
    state.is_active   = (reply.readInt32() == 0) ? false : true;

    state.state     = reply.readInt32();
    state.sub_state = reply.readInt32();


    state.is_initalized  = (reply.readInt32() == 0) ? false : true;
    state.is_debug_en    = (reply.readInt32() == 0) ? false : true;

    state.mode = reply.readInt32();

    return 0;
}

int RobotSysInfo::GetRobotSex(bool &is_boy)
{
    if (!CheckService()) return -1;

    Parcel data, reply;
    data.writeInterfaceToken(String16(SERVICE));

    mRemoteService->transact(GET_SEX, data, &reply, 0);
    int ret = reply.readInt32();
    if (ret != 0)
    {
        LOGE("Get sex failed.");
        return -1;
    }

    is_boy = (reply.readInt32() == MALE) ? true : false;

    return 0;
}

int RobotSysInfo::SetRobotSex(bool is_box)
{
    if (!CheckService()) return -1;

    int sex = is_box ? MALE : FEMALE;

    Parcel data, reply;
    data.writeInterfaceToken(String16(SERVICE));
    data.writeInt32(sex);

    mRemoteService->transact(SET_SEX, data, &reply, 0);
    return reply.readInt32();
}

int RobotSysInfo::GetRobotTime(
        int &year, int &month, int &day, int &weekday,
        int &hour, int &minute, int &second)
{
    Parcel data, reply;
    data.writeInterfaceToken(String16(SERVICE));

    mRemoteService->transact(GET_DATETIME, data, &reply, 0);
    int ret = reply.readInt32();
    if (ret != 0)
    {
        LOGE("Get datetime failed.");
        return -1;
    }

    year    = reply.readInt32();
    month   = reply.readInt32();
    day     = reply.readInt32();
    weekday = reply.readInt32();
    hour    = reply.readInt32();
    minute  = reply.readInt32();
    second  = reply.readInt32();

    return 0;
}

int RobotSysInfo::SetRobotTime(
        int year, int month, int day, int weekday,
        int hour, int minute, int second)
{
    Parcel data, reply;
    data.writeInterfaceToken(String16(SERVICE));
    data.writeInt32(year);
    data.writeInt32(month);
    data.writeInt32(day);
    data.writeInt32(weekday);
    data.writeInt32(hour);
    data.writeInt32(minute);
    data.writeInt32(second);

    mRemoteService->transact(SET_DATETIME, data, &reply, 0);

    return reply.readInt32();
}

int RobotSysInfo::GetRobotUptime(int &uptime)
{
    if (!CheckService()) return -1;

    Parcel data, reply;
    data.writeInterfaceToken(String16(SERVICE));

    mRemoteService->transact(GET_UPTIME, data, &reply, 0);
    int ret = reply.readInt32();
    if (ret != 0)
    {
        LOGE("Get uptime failed.");
        return -1;
    }

    uptime = reply.readInt32();

    return 0;
}

} /* namespace android */
