/*************************************************************************
FileName: rm_sysctrl_conf.c
Copy Right: Copyright (C), 2006-2014, Archermind. Co., Ltd.
System: RM
Module: SYSCTRL
Author: wangkun
Create Date: 2014-12-3
**************************************************************************/
/*************************************************************************
Editor: wangkun
Version: v1.0 init version
Edit Date: 2014-12-3
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

#define LOG_TAG "RM_SYSCTRL_CNF"
#include "cutils/log.h"
#include "rm_sysctrl_conf.h"

static int RmSysctrlCreateConfig(struct RmSysctrlCnf **config_addr)
{
    struct RmSysctrlCnf *config_l;
    config_l = (struct RmSysctrlCnf *)malloc(sizeof(struct RmSysctrlCnf));
    if (!config_l)
    {
        RLOGE("RmSysctrlCnf create failed!\n");
        return -1;
    }
    memset(config_l, 0, sizeof(struct RmSysctrlCnf));
    *config_addr = config_l;
    return 0;
}

int RmSysctrlInitConfig(struct RmSysctrlCnf **config_addr)
{
    int ret = -1;
    int i = 0;
    struct RmSysctrlCnf *config = NULL;
    struct RmSysctrlCnfNode *node = NULL;

    /* Create RmSysctrlCnf */    
    ret = RmSysctrlCreateConfig(config_addr);
    if (ret < 0)
    {
        RLOGE("RmSysctrlCnf create failed!");
        return -1;
    }

    config = *config_addr;


    /* Mode node */
    node = &(config->nodes[RM_SYSCTRL_CONFIG_MODE]);
    node->id = RM_SYSCTRL_CONFIG_MODE;
    node->is_used = true;
    strcpy(node->name, MODE_NAME);
    strcpy(node->def_value, MODE_DEFAULT_VALUE);
    ret = read_config(node->name, node->cnf_value);
    if (ret < 0)
    {
        RLOGD("Read mode config failed, to use default value.");
        strcpy(node->cnf_value, node->def_value);
    }

    /* Debug node */
    node = &(config->nodes[RM_SYSCTRL_CONFIG_DEBUG]);
    node->id = RM_SYSCTRL_CONFIG_DEBUG;
    node->is_used = true;
    strcpy(node->name, DEBUG_NAME);
    strcpy(node->def_value, DEBUG_DEFAULT_VALUE);
    ret = read_config(node->name, node->cnf_value);
    if (ret < 0)
    {
        RLOGD("Read debug config failed, to use default value.");
        strcpy(node->cnf_value, node->def_value);
    }

    return 0;
}

static int RmSysctrlWriteConfig(char *field, char *value)
{
    int ret = -1;

    ret = write_config(field, value);
    if(ret < 0)
    {
      int retry = 3;
      while (retry--)
      {
         ret = write_config(field, value);
         if (0 <= ret) break;
         usleep(30 * 1000);
      }
    }
    return ret;
}

void RmSysctrlRestoreFactory(struct RmSysctrlCnf *config)
{
    int ret = -1;
    struct RmSysctrlCnfNode *node = NULL;

    /* Mode node */
    node = &(config->nodes[RM_SYSCTRL_CONFIG_MODE]);
    strcpy(node->cnf_value, node->def_value);
    ret = RmSysctrlWriteConfig(node->name, node->cnf_value);
    if(ret < 0)
    {
        RLOGE("Restore factory failed when reset mode node!");
        return;
    }

    /* Debug node */
    node = &(config->nodes[RM_SYSCTRL_CONFIG_DEBUG]);
    strcpy(node->cnf_value, node->def_value);
    ret = RmSysctrlWriteConfig(node->name, node->cnf_value);
    if(ret < 0)
    {
        RLOGE("Restore factory failed when reset debug node!");
        return;
    }
}

int RmSysctrlSetMode(struct RmSysctrlCnf *config, int mode)
{
    int ret = -1;
    struct RmSysctrlCnfNode *node = &(config->nodes[RM_SYSCTRL_CONFIG_MODE]);

    switch (mode) {
    case DEVELOPER_MODE:
        strcpy(node->cnf_value, MODE_VALUE_DEVELOPER_MODE);
        break;
    case CHILDREN_MODE:
        strcpy(node->cnf_value, MODE_VALUE_CHILDREN_MODE);
        break;
    case GUARDER_MODE:
        strcpy(node->cnf_value, MODE_VALUE_GUARDER_MODE);
        break;
    default:
        RLOGE("Invalid mode!");
        break;
    }

    ret = RmSysctrlWriteConfig(node->name, node->cnf_value);
    if (ret < 0)
    {
        RLOGE("Write mode config failed!");
        return -1;
    }

    return 0;
}

int RmSysctrlGetMode(struct RmSysctrlCnf *config, int *mode)
{
    struct RmSysctrlCnfNode *node = &(config->nodes[RM_SYSCTRL_CONFIG_MODE]);

    if (!strcmp(node->cnf_value, MODE_VALUE_DEVELOPER_MODE))
        *mode = DEVELOPER_MODE;
    else if (!strcmp(node->cnf_value, MODE_VALUE_CHILDREN_MODE))
        *mode = CHILDREN_MODE;
    else if (!strcmp(node->cnf_value, MODE_VALUE_GUARDER_MODE))
        *mode = GUARDER_MODE;
    else
        return -1;

    return 0;
}

int RmSysctrlGetDefaultMode(struct RmSysctrlCnf *config, int *mode)
{
    struct RmSysctrlCnfNode *node = &(config->nodes[RM_SYSCTRL_CONFIG_MODE]);

    if (!strcmp(node->def_value, MODE_VALUE_DEVELOPER_MODE))
        *mode = DEVELOPER_MODE;
    else if (!strcmp(node->def_value, MODE_VALUE_CHILDREN_MODE))
        *mode = CHILDREN_MODE;
    else if (!strcmp(node->def_value, MODE_VALUE_GUARDER_MODE))
        *mode = GUARDER_MODE;
    else
        return -1;

    return 0;
}

int RmSysctrlSetDebug(struct RmSysctrlCnf *config, bool debug_en)
{
    int ret = -1;
    struct RmSysctrlCnfNode *node = &(config->nodes[RM_SYSCTRL_CONFIG_DEBUG]);

    if (debug_en)
    {
        strcpy(node->cnf_value, DEBUG_VALUE_ENABLE);
    }
    else
    {
        strcpy(node->cnf_value, DEBUG_VALUE_DISABLE);
    }

    ret = RmSysctrlWriteConfig(node->name, node->cnf_value);
    if (ret < 0)
    {
        RLOGE("Write debug config failed!");
        return -1;
    }

    return 0;
}

int RmSysctrlGetDebug(struct RmSysctrlCnf *config, bool *debug_en)
{
    struct RmSysctrlCnfNode *node = &(config->nodes[RM_SYSCTRL_CONFIG_DEBUG]);

    if (!strcmp(node->cnf_value, DEBUG_VALUE_ENABLE))
        *debug_en = true;
    else if (!strcmp(node->cnf_value, DEBUG_VALUE_DISABLE))
        *debug_en = false;
    else
        return -1;

    return 0;
}

int RmSysctrlGetDefaultDebug(struct RmSysctrlCnf *config, bool *debug_en)
{
    struct RmSysctrlCnfNode *node = &(config->nodes[RM_SYSCTRL_CONFIG_DEBUG]);

    if (!strcmp(node->def_value, DEBUG_VALUE_ENABLE))
        *debug_en = true;
    else if (!strcmp(node->def_value, DEBUG_VALUE_DISABLE))
        *debug_en = false;
    else
        return -1;

    return 0;
}

