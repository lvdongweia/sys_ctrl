/*************************************************
 Copyright (C), 2006-2014, Archermind. Co., Ltd.
 File name: rm_sysctrl_vbdev.h
 Author: wangkun
 Version: v1.0
 Date: 2014-9-22
 Description:  This program defined the sysctrl virtual battery device and
               interface to set the information into kernel driver.

 Others: None

 Function List:
   1. RmSysctrlSetACOnline() to set AC online into kernel driver.
   2. RmSysctrlSetBatteryStatus() to set battery status into kernel driver.
   3. RmSysctrlSetBatteryHealth() to set battery health into kernel driver.
   4. RmSysctrlSetBatteryPresent() to set battery present into kernel driver.
   5. RmSysctrlSetBatteryTechnology() to set battery present into kernel driver.
   6. RmSysctrlSetBatteryCapacity() to set battery capacity into kernel driver.
   7. RmSysctrlSetVoltageNow() to set voltage now into kernel driver.

 History:
   V1.0    2014-10-21    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_VBDEV_H_
#define RM_SYSCTRL_VBDEV_H_

#include <linux/ioctl.h>

#define VBDEV_IOC_MAGIC  'k'

#define VBDEV_IOCPRINT   		_IO(VBDEV_IOC_MAGIC, 1)
#define VBDEV_IOCGETDATA 		_IOR(VBDEV_IOC_MAGIC, 2, int)
#define VBDEV_IOCSETACONLINE 	_IOW(VBDEV_IOC_MAGIC, 3, int)
#define VBDEV_IOCSETBSTATUS 	_IOW(VBDEV_IOC_MAGIC, 4, int)
#define VBDEV_IOCSETBHEALTH 	_IOW(VBDEV_IOC_MAGIC, 5, int)
#define VBDEV_IOCSETBPRESENT 	_IOW(VBDEV_IOC_MAGIC, 6, int)
#define VBDEV_IOCSETBTECH 		_IOW(VBDEV_IOC_MAGIC, 7, int)
#define VBDEV_IOCSETBCAPACITY 	_IOW(VBDEV_IOC_MAGIC, 8, int)
#define VBDEV_IOCSETBVOLTAGE 	_IOW(VBDEV_IOC_MAGIC, 9, int)

#define VBDEV_IOC_MAXNR 9

/****************************************************************************
 Name: RmSysctrlSetACOnline
 Function: This function is to set AC online into kernel driver.
 Params: online - The AC online to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlSetACOnline(<AC ONLINE>);
******************************************************************************/
int RmSysctrlSetACOnline(int online);

/****************************************************************************
 Name: RmSysctrlSetBatteryStatus
 Function: This function is to set battery status into kernel driver.
 Params: status - The battery status to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlSetBatteryStatus(<BATTERY STATUS>);
******************************************************************************/
int RmSysctrlSetBatteryStatus(int status);

/****************************************************************************
 Name: RmSysctrlSetBatteryHealth
 Function: This function is to set battery health into kernel driver.
 Params: health - The battery health to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlSetBatteryHealth(<BATTERY HEALTH>);
******************************************************************************/
int RmSysctrlSetBatteryHealth(int health);

/****************************************************************************
 Name: RmSysctrlSetBatteryPresent
 Function: This function is to set battery present into kernel driver.
 Params: present - The battery present to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlSetBatteryPresent(<BATTERY PRESENT>);
******************************************************************************/
int RmSysctrlSetBatteryPresent(int present);

/****************************************************************************
 Name: RmSysctrlSetBatteryTechnology
 Function: This function is to set battery technology into kernel driver.
 Params: technology - The battery technology to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlSetBatteryTechnology(<BATTERY TECHNOLOGY>);
******************************************************************************/
int RmSysctrlSetBatteryTechnology(int technology);

/****************************************************************************
 Name: RmSysctrlSetBatteryCapacity
 Function: This function is to set battery capacity into kernel driver.
 Params: capacity - The battery capacity to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlSetBatteryCapacity(<BATTERY CAPACITY>);
******************************************************************************/
int RmSysctrlSetBatteryCapacity(int capacity);

/****************************************************************************
 Name: RmSysctrlSetVoltageNow
 Function: This function is to set voltage now into kernel driver.
 Params: voltage - The voltage to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlSetVoltageNow(<VOLTAGE>);
******************************************************************************/
int RmSysctrlSetVoltageNow(int voltage);

#endif /* RM_SYSCTRL_VBDEV_H_ */
