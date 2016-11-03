/*************************************************
 Copyright (C), 2006-2014, Archermind. Co., Ltd.
 File name: rm_sysctrl_conf.h
 Author: wangkun
 Version: v1.0
 Date: 2014-12-3
 Description: This program defined Rm Sysctrl config.
 Others: None
 
 Function List: 
   1. RmSysctrlInitConfig() to initialize the config.
   2. RmSysctrlRestoreFactory() to restore the factory config.
   3. RmSysctrlSetMode() to set the mode config.
   4. RmSysctrlGetMode() to get the mode config.
   5. RmSysctrlGetDefaultMode() to get the default mode config.
   6. RmSysctrlSetDebug() to set the debug config.
   7. RmSysctrlGetDebug() to get the debug config.
   8. RmSysctrlGetDefaultDebug() to get the default debug config.
 
 History:
   V1.0    2014-12-3    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_CONF_H_
#define RM_SYSCTRL_CONF_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "robot_conf.h"

struct RmSysctrlCnfNode {
    unsigned int id;
    bool is_used;
    char name[CONF_FIELD_LENGTH];
    char def_value[CONF_VALUE_LENGTH];
    char cnf_value[CONF_VALUE_LENGTH];
};

/* Config ID */
enum _RM_SYSCTRL_CONFIG_ID
{
    RM_SYSCTRL_CONFIG_MODE	= 0,
    RM_SYSCTRL_CONFIG_DEBUG	= 1,
    RM_SYSCTRL_CONFIG_MAX
};
typedef enum _RM_SYSCTRL_CONFIG_ID RM_SYSCTRL_CONFIG_ID;

#define RM_SYSCTRL_CONF_MIN_ID		RM_SYSCTRL_CONFIG_MODE
#define RM_SYSCTRL_CONF_MAX_ID		(RM_SYSCTRL_CONFIG_MAX - 1)

/* Config Name */
#define MODE_NAME	"mode"
#define DEBUG_NAME	"debug"

/* Optional Value */
/* Mode Section */
enum RmSysctrlMode {
    DEVELOPER_MODE = 0,
    CHILDREN_MODE  = 1,
    GUARDER_MODE   = 2
};
#define MODE_VALUE_DEVELOPER_MODE	"developer"
#define MODE_VALUE_CHILDREN_MODE	"children"
#define MODE_VALUE_GUARDER_MODE		"guarder"
/* debug Section */
#define DEBUG_VALUE_ENABLE	"enable"
#define DEBUG_VALUE_DISABLE	"disable"

/* Default value */
#define MODE_DEFAULT_VALUE		MODE_VALUE_DEVELOPER_MODE
#define DEBUG_DEFAULT_VALUE		DEBUG_VALUE_DISABLE


struct RmSysctrlCnf {
    struct RmSysctrlCnfNode nodes[RM_SYSCTRL_CONFIG_MAX];
};


/* APIs*/
/****************************************************************************
 Name: RmSysctrlInitConf
 Function: This function is to initialize the config from the params.
 Params: config_addr - The point of point of the rm sysctrl config.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlInitConfig(<POINT of POINT of CONFIG>);
******************************************************************************/
int RmSysctrlInitConfig(struct RmSysctrlCnf **config_addr);

/****************************************************************************
 Name: RmSysctrlRestoreFactory
 Function: This function is to restore the factory config from the params.
 Params: config - The point of the rm sysctrl config.

 Return:
   none
 Systax:
  [RETURN VALUE] = RmSysctrlRestoreFactory(<POINT of CONFIG>);
******************************************************************************/
void RmSysctrlRestoreFactory(struct RmSysctrlCnf *config);

/****************************************************************************
 Name: RmSysctrlSetMode
 Function: This function is to set the mode config from the params.
 Params: config - The point of the rm sysctrl config.
         mode - The mode to set

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlSetMode(<POINT of CONFIG>, <MODE>);
******************************************************************************/
int RmSysctrlSetMode(struct RmSysctrlCnf *config, int mode);

/****************************************************************************
 Name: RmSysctrlGetMode
 Function: This function is to get the mode config from the params.
 Params: config - The point of the rm sysctrl config.
         mode - The mode to get

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlGetMode(<POINT of CONFIG>, <POINT of MODE>);
******************************************************************************/
int RmSysctrlGetMode(struct RmSysctrlCnf *config, int *mode);

/****************************************************************************
 Name: RmSysctrlGetDefaultMode
 Function: This function is to get the default mode config from the params.
 Params: config - The point of the rm sysctrl config.
         mode - The default mode to get

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlGetDefaultMode(<POINT of CONFIG>, <POINT of MODE>);
******************************************************************************/
int RmSysctrlGetDefaultMode(struct RmSysctrlCnf *config, int *mode);

/****************************************************************************
 Name: RmSysctrlSetDebug
 Function: This function is to set the debug config from the params.
 Params: config - The point of the rm sysctrl config.
         debug_en - The debug to set

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlSetDebug(<POINT of CONFIG>, <DEBUG>);
******************************************************************************/
int RmSysctrlSetDebug(struct RmSysctrlCnf *config, bool debug_en);

/****************************************************************************
 Name: RmSysctrlGetDebug
 Function: This function is to get the debug config from the params.
 Params: config - The point of the rm sysctrl config.
         debug_en - The debug_en to get

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlGetDebug(<POINT of CONFIG>, <POINT of DEBUG>);
******************************************************************************/
int RmSysctrlGetDebug(struct RmSysctrlCnf *config, bool *debug_en);

/****************************************************************************
 Name: RmSysctrlGetDefaultDebug
 Function: This function is to get the default debug config from the params.
 Params: config - The point of the rm sysctrl config.
         debug_en - The default debug to get

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlGetDefaultDebug(<POINT of CONFIG>, <POINT of DEBUG>);
******************************************************************************/
int RmSysctrlGetDefaultDebug(struct RmSysctrlCnf *config, bool *debug_en);


#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_CONF_H_*/
