/*************************************************************************
FileName: rm_sysctrl_uevent.c
Copy Right: Copyright (C), 2014-2016, Avatarmind. Co., Ltd.
System: RM
Module: SYSCTRL
Author: wangkun
Create Date: 2016-8-31
**************************************************************************/
/*************************************************************************
Editor: wangkun
Version: v1.0 init version
Edit Date: 2016-8-31
**************************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <netinet/in.h>

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
#include "rm_sysctrl_uevent.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RM_SYSCTRL_UEV"
#include "cutils/log.h"

#define HDMI_PATH "/sys/class/switch/hdmi/state"

#define UEVENT_MSG_LEN 4096
struct luther_gliethttp {
    const char *action;
    const char *path;
    const char *subsystem;
    const char *interface;
    const char *source;
    const char *switch_name;
    int switch_state;
    int seqnum;
};
static int open_luther_gliethttp_socket(void);
static void parse_event(const char *msg, struct luther_gliethttp *luther_gliethttp);

#define BUF_SIZE 1024
static int GetHDMIStateFromSystem()
{
    FILE *fp;
    char filepath[50];
    char buf[BUF_SIZE];
    int hdmi_state = 0;

    fp = fopen(HDMI_PATH, "r");
    if (NULL != fp)
    {
        if( fgets(buf, BUF_SIZE-1, fp)== NULL ) {
            fclose(fp);
            return -1;
        }
        sscanf(buf, "%d", &hdmi_state);
        fclose(fp);
    }
    return hdmi_state;
}

static void RmSysctrlUpdateHDMIState()
{
    int hdmi_state;
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();

    hdmi_state = GetHDMIStateFromSystem();
    if (hdmi_state < 0)
    {
        RLOGE("Update the HDMI State failed.");
        return;
    }

    if (hdmi_state)
    {
        RLOGD("hdmi plug in");
        rm_sysctrl->is_hdmi_plug_in = true;
    }
    else
    {
        RLOGD("hdmi plug out");
        rm_sysctrl->is_hdmi_plug_in = false;
    }
}

void *RmSysctrlUeventThread(void *arg)
{
    int device_fd = -1;
    char msg[UEVENT_MSG_LEN+2];
    int n;

    device_fd = open_luther_gliethttp_socket();
    //RLOGD("device_fd = %d", device_fd);
    if (device_fd < 0)
    {
        RLOGE("UEVENT thread failed.");
        return NULL;
    }

    RmSysctrlUpdateHDMIState();

    do {
        while((n = recv(device_fd, msg, UEVENT_MSG_LEN, 0)) > 0) {
            struct luther_gliethttp luther_gliethttp;

            if(n == UEVENT_MSG_LEN)
                continue;

            msg[n] = '\0';
            msg[n+1] = '\0';

            parse_event(msg, &luther_gliethttp);
        }
    } while(1);
    return arg;
}


static int open_luther_gliethttp_socket(void)
{
    struct sockaddr_nl addr;
    int sz = 64*1024;
    int s;

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0xffffffff;

    s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (s < 0)
        return -1;

    setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));

    if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(s);
        return -1;
    }

    return s;
}

static void parse_event(const char *msg, 
    struct luther_gliethttp *luther_gliethttp)
{
    struct RmSysctrl *rm_sysctrl = RmSysctrlSelf();

    luther_gliethttp->action = "";
    luther_gliethttp->path = "";
    luther_gliethttp->subsystem = "";
    luther_gliethttp->interface = "";
    luther_gliethttp->source = "";
    luther_gliethttp->switch_name = "";
    luther_gliethttp->switch_state = -1;
    luther_gliethttp->seqnum = -1;


    //RLOGD("========================================================\n");
    while (*msg) {

        //RLOGD("%s\n", msg);

        if (!strncmp(msg, "ACTION=", 7)) {
            msg += 7;
            luther_gliethttp->action = msg;
        } else if (!strncmp(msg, "DEVPATH=", 8)) {
            msg += 8;
            luther_gliethttp->path = msg;
        } else if (!strncmp(msg, "SUBSYSTEM=", 10)) {
            msg += 10;
            luther_gliethttp->subsystem = msg;
        } else if (!strncmp(msg, "INTERFACE=", 10)) {
            msg += 10;
            luther_gliethttp->interface = msg;
        } else if (!strncmp(msg, "SOURCE=", 7)) {
            msg += 7;
            luther_gliethttp->source = msg;
        } else if (!strncmp(msg, "SWITCH_NAME=", 12)) {
            msg += 12;
            luther_gliethttp->switch_name = msg;
        } else if (!strncmp(msg, "SWITCH_STATE=", 13)) {
            msg += 13;
            luther_gliethttp->switch_state = atoi(msg);
        } else if (!strncmp(msg, "SEQNUM=", 7)) {
            msg += 7;
            luther_gliethttp->seqnum = atoi(msg);
        }


        while(*msg++)
            ;
    }

    //RLOGD("event { '%s', '%s', '%s', '%s', '%s', '%s', %d, %d }\n",
    //    luther_gliethttp->action, luther_gliethttp->path,
    //    luther_gliethttp->subsystem, luther_gliethttp->interface,
    //    luther_gliethttp->source, luther_gliethttp->switch_name,
    //    luther_gliethttp->switch_state, luther_gliethttp->seqnum);

    if (!strcmp(luther_gliethttp->switch_name, "hdmi"))
    {
        if (luther_gliethttp->switch_state)
        {
            RLOGD("hdmi plug in");
            rm_sysctrl->is_hdmi_plug_in = true;
        }
        else
        {
            RLOGD("hdmi plug out");
            rm_sysctrl->is_hdmi_plug_in = false;
        }

        /* Broadcast HDMI state change message into can */
        RmSysctrlBroadcastHDMIStateChangeNotiMsg(rm_sysctrl,
            rm_sysctrl->is_hdmi_plug_in);

        /* Unicast HDMI state change message into sub-systems to avoid that
         * any sub-sustems donot receive the HDMI state change message by 
         * broadcasting.
         */
        usleep(200000);
        RmSysctrlUnicastHDMIStateChangeNotiMsg(rm_sysctrl,
            rm_sysctrl->is_hdmi_plug_in);
    }
}
