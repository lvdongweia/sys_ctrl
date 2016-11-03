/*************************************************************************
FileName: rm_sysctrl_vbdev.c
Copy Right: Copyright (C), 2006-2014, Archermind. Co., Ltd.
System: RM
Module: SYSCTRL
Author: wangkun
Create Date: 2014-9-22
**************************************************************************/
/*************************************************************************
Editor: wangkun
Version: v1.0 init version
Edit Date: 2014-9-22
**************************************************************************/
#include <stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#define LOG_TAG "RM_SYSCTRL_VBD"
#include "cutils/log.h"
#include "rm_sysctrl_vbdev.h"
#include "rm_sysctrl_log.h"

static int VBDeviceSet(int command, int argument)
{
    int fd = 0;
    int cmd = 0;
    int arg = 0;

    fd = open("/dev/vbdev",O_RDWR);
    if (fd < 0)
    {
        RLOGE("Open Dev Memdev Error!\n");
        goto bail;
    }

    cmd = command;
    arg = argument;
    if (ioctl(fd, cmd, &arg) < 0)
    {
        RLOGE("Call cmd %d fail\n", cmd);
        goto bail;
    }

bail:
    if (fd >= 0) close(fd);
    return 0;
}

int RmSysctrlSetACOnline(int online)
{
    RLOGD("<--- Call RmSysctrlSetACOnline --->\n");
    return VBDeviceSet(VBDEV_IOCSETACONLINE, online);
}

int RmSysctrlSetBatteryStatus(int status)
{
    RLOGD("<--- Call RmSysctrlSetBatteryStatus --->\n");
    return VBDeviceSet(VBDEV_IOCSETBSTATUS, status);
}

int RmSysctrlSetBatteryHealth(int health)
{
    RLOGD("<--- Call RmSysctrlSetBatteryHealth --->\n");
    return VBDeviceSet(VBDEV_IOCSETBHEALTH, health);
}

int RmSysctrlSetBatteryPresent(int present)
{
    RLOGD("<--- Call RmSysctrlSetBatteryPresent --->\n");
    return VBDeviceSet(VBDEV_IOCSETBPRESENT, present);
}

int RmSysctrlSetBatteryTechnology(int technology)
{
    RLOGD("<--- Call RmSysctrlSetBatteryTechnology --->\n");
    return VBDeviceSet(VBDEV_IOCSETBTECH, technology);
}

int RmSysctrlSetBatteryCapacity(int capacity)
{
    RLOGD("<--- Call RmSysctrlSetBatteryCapacity --->\n");
    return VBDeviceSet(VBDEV_IOCSETBCAPACITY, capacity);
}

int RmSysctrlSetVoltageNow(int voltage)
{
    RLOGD("<--- Call RmSysctrlSetVoltageNow --->\n");
    return VBDeviceSet(VBDEV_IOCSETBVOLTAGE, voltage);
}

