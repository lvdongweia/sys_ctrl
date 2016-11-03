/*************************************************************************
FileName: rm_sysctrl_gpio.c
Copy Right: Copyright (C), 2006-2014, Archermind. Co., Ltd.
System: RM
Module: SYSCTRL
Author: wangkun
Create Date: 2014-9-22
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

#define LOG_TAG "RM_SYSCTRL_GIO"
#include "cutils/log.h"
#include "rm_sysctrl.h"
#include "rm_sysctrl_cli.h"
#include "rm_sysctrl_fault.h"
#include "rm_sysctrl_log.h"
#include "rm_sysctrl_comm.h"
#include "rm_sysctrl_subsys.h"
#include "rm_sysctrl_fsm.h"
#include "rm_sysctrl_gpio.h"

#ifdef DEEP_SLEEP_SUPPORT
int RmSysctrlGpioCreate(struct RmSysctrlGpio **gpio_addr)
{
    struct RmSysctrlGpio *gpio_l;
    gpio_l = (struct RmSysctrlGpio *)malloc(sizeof(struct RmSysctrlGpio));
    if (!gpio_l)
    {
        RLOGE("RmSysctrlGpio create failed!\n");
        return -1;
    }
    memset(gpio_l, 0, sizeof(struct RmSysctrlGpio));
    *gpio_addr = gpio_l;
    return 0;
}

int RmSysctrlGpioInit(struct RmSysctrlGpio **gpio_addr)
{
    int ret = -1;
    char cmd[1024] = {0};
    struct RmSysctrlGpio *gpio = NULL;

    /* Create RmSysctrlGpio */
    ret = RmSysctrlGpioCreate(gpio_addr);
    if (ret < 0)
    {
        RLOGE("RmSysctrlGpio create failed!");
        return -1;
    }

    gpio = *gpio_addr;
    gpio->gpio_in = GPIO_IN;
    gpio->gpio_out = GPIO_OUT;

    sprintf(cmd, "echo %d > /sys/class/gpio/export", gpio->gpio_in);
    system(cmd);
    RLOGD("%s",cmd);

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo \"in\" > /sys/class/gpio/gpio%d/direction", gpio->gpio_in);
    system(cmd);
    RLOGD("%s",cmd);

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo 1 > /sys/class/gpio/gpio%d/active_low", gpio->gpio_in);
    system(cmd);
    RLOGD("%s",cmd);

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo  \"falling\" > /sys/class/gpio/gpio%d/edge", gpio->gpio_in);
    system(cmd);
    RLOGD("%s",cmd);

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo %d > /sys/class/gpio/export", gpio->gpio_out);
    system(cmd);
    RLOGD("%s",cmd);

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo \"out\" > /sys/class/gpio/gpio%d/direction", gpio->gpio_out);
    system(cmd);
    RLOGD("%s",cmd);

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo 1 > /sys/class/gpio/gpio%d/value", gpio->gpio_out);
    system(cmd);
    RLOGD("%s",cmd);

    return 0;
}

int RmSysctrlGpioGetValue(struct RmSysctrlGpio *gpio,
    unsigned int *value)
{
    int fd = -1;
    ssize_t nbytes = 0;
    char buf[10] = {0};
    char path[50] = {0};

    if (!gpio || !value)
    {
        RLOGE("Invalid parameter.");
        return -1;
    }

    sprintf(path, "/sys/class/gpio/gpio%d/value", gpio->gpio_in);
    /* Open and read the file contents.
     */
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        RLOGE("Open %s failed", path);
        return -1;
    }
    nbytes = read(fd, buf, 10);
    close(fd);
    if (nbytes < 0) {
        RLOGE("Read %s failed", path);
        return -1;
    }
    buf[nbytes] = '\0';
    *value = atoi(buf);
    gpio->s_poweron = *value;
    return 0;
}

int RmSysctrlGpioSetValue(struct RmSysctrlGpio *gpio,
    unsigned int value)
{
    int fd = -1;
    ssize_t num = 0;
    char path[50] = {0};
    char value_str[10] = {0};

    if (!gpio)
    {
        RLOGE("Invalid parameter.");
        return -1;
    }

    sprintf(path, "/sys/class/gpio/gpio%d/value", gpio->gpio_out);
    fd = open((char*)path, O_RDWR, S_IRUSR | S_IWUSR);
    if(fd <= 0)
    {
      RLOGE("open %s error: %s", path, strerror(errno));
      return -1;
    }

    sprintf(value_str, "%d", value);
    num = write(fd, value_str, strlen(value_str));
    if(num < 0)
    {
       RLOGE("write value %d into file error: %s", value, strerror(errno));
    }
    close(fd);

    gpio->s_powerdown = value;
    return 0;
}

void *RmSysctrlGpioThread(void *arg)
{
    int fd = -1;
    int ret = -1;
    ssize_t nbytes = 0;
    char buf[10] = {0};
    struct pollfd fds[1];
    char path[50] = {0};
    struct RmSysctrl *rm_sysctrl = arg;
    struct RmSysctrlGpio *gpio = rm_sysctrl->gpio;
    struct RmSysctrlComm *comm = rm_sysctrl->comm;
    char cmd[1024] = {0};

    sprintf(path, "/sys/class/gpio/gpio%d/value", gpio->gpio_in);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        RLOGE("Open %s failed", path);
        return NULL;
    }
    nbytes = read(fd, buf, 10);
    if (nbytes < 0) {
        RLOGE("Read %s failed", path);
        return NULL;
    }
    buf[nbytes] = '\0';
    RLOGD("value:%s", buf);

    fds[0].fd = fd;
    fds[0].events  = POLLPRI;

    while (1)
    {
        RLOGD("Poll start");
        ret = poll(fds, 1, -1);
        if( ret == -1 )
        {
            RLOGE("poll failed.");
            return NULL;
        }

        RLOGD("Poll a data");
        if( fds[0].revents & POLLPRI)
        {
            ret = lseek(fd,0,SEEK_SET);
            if (ret < 0) {
                RLOGE("lseek failed");
                return NULL;
            }
            nbytes = read(fd, buf, 10);
            if (nbytes < 0) {
                RLOGE("Read %s failed", path);
                return NULL;
            }
            buf[nbytes] = '\0';
            RLOGD("s_poweron value:%s", buf);

            gpio->s_poweron = atoi(buf);
            if (gpio->s_poweron == 1)
            {
                memset(cmd, 0, sizeof(cmd));
                sprintf(cmd, "echo  \"falling\" > /sys/class/gpio/gpio%d/edge", gpio->gpio_in);
                system(cmd);
                RLOGD("%s",cmd);
            }
            else
            {
                memset(cmd, 0, sizeof(cmd));
                sprintf(cmd, "echo  \"rising\" > /sys/class/gpio/gpio%d/edge", gpio->gpio_in);
                system(cmd);
                RLOGD("%s",cmd);
            }
            /* Send s_poweron change event */
            RmSysctrlCommSendSPoweronChange(comm);
        }
    }
    return arg;
}

#endif






