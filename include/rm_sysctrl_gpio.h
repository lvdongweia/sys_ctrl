/*************************************************
 Copyright (C), 2006-2014, Archermind. Co., Ltd.
 File name: rm_sysctrl_subsys.h
 Author: wangkun
 Version: v1.0
 Date: 2014-9-22
 Description: This program defined GPIO_IN and GPIO_OUT for deep sleep.

 Others: None
 
 Function List: 
   1. RmSysctrlGpioInit() to initialize the gpio.
   2. RmSysctrlGpioGetValue() to get the gpio value.   
   3. RmSysctrlGpioSetValue() to set the gpio value.
   4. RmSysctrlGpioThread() to listen the input gpio.

 
 History:
   V1.0    2014-9-22    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_GPIO_H_
#define RM_SYSCTRL_GPIO_H_

#ifdef __cplusplus
extern "C"{
#endif

#define GPIO_IN		317
#define GPIO_OUT	319

struct RmSysctrlGpio {
    unsigned int gpio_in;
    unsigned int gpio_out;
    unsigned int s_poweron;
    unsigned int s_powerdown;
    pthread_t ntid_gpio;
};

/****************************************************************************
 Name: RmSysctrlGpioInit
 Function: This function is to initialize the GPIO from the params.
 Params: gpio_addr - The point of point of a GPIO.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlInitSubSystem(<POINT of POINT of GPIO>);
******************************************************************************/
int RmSysctrlGpioInit(struct RmSysctrlGpio **gpio_addr);

/****************************************************************************
 Name: RmSysctrlGpioGetValue
 Function: This function is to get the gpio value.
 Params: gpio - The point of a GPIO.
         value - the pointer of gpio value for getting value.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlGpioGetValue(<POINT of GPIO struct>,
                                         <POINT of GPIO value>);
******************************************************************************/
int RmSysctrlGpioGetValue(struct RmSysctrlGpio *gpio,
    unsigned int *value);

/****************************************************************************
 Name: RmSysctrlGpioSetValue
 Function: This function is to set the gpio value.
 Params: gpio - The point of a GPIO.
         value - the gpio value for setting value.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlGpioSetValue(<POINT of GPIO struct>,
                                         <GPIO value>);
******************************************************************************/
int RmSysctrlGpioSetValue(struct RmSysctrlGpio *gpio,
    unsigned int value);

/****************************************************************************
 Name: RmSysctrlGpioThread
 Function: This function is to listen the input gpio when an interruption occurs.
 Params:  - The point of a arg.

 Return:
   None
 Systax:
  ret = pthread_create(gpio->ntid_gpio,
                       NULL,
                       RmSysctrlGpioThread,
                       rm_sysctrl);
******************************************************************************/
void *RmSysctrlGpioThread(void *arg);

#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_SUBSYS_H_*/
