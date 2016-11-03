/*************************************************
 Copyright (C), 2006-2014, Archermind. Co., Ltd.
 File name: rm_sysctrl_fault.h
 Author: wangkun
 Version: v1.0
 Date: 2014-9-4
 Description: This program defined the sysctrl fault structure.
 Others: None
 
 Function List: 
   1. RmSysctrlInitFault() to initialize the fault.
   2. RmSysctrlFaultSetLastFaultType() to set the last fault type.
   3. RmSysctrlFaultGetLastFaultType() to get the last fault type.
   4. RmSysctrlFaultSetSubsysFaultType() to set the sub-system fault type.
   5. RmSysctrlFaultGetSubsysFaultType() to get the sub-system fault type.
   6. RmSysctrlFaultSetFaultSubsys() to set the fault sub-system ID.
   7. RmSysctrlFaultGetFaultSubsys() to get the fault sub-system ID.
   8. RmSysctrlFaultIsSubsysFaultFalt() to check if any sub-system is in fault.
   9. RmSysctrlIsValidFault() to check if the fault is valid.
  10. RmSysctrlGetFaultStr() to get the string of fault.
 
 History:
   V1.0    2014-9-4    wangkun    init version
*************************************************/
#ifndef RM_SYSCTRL_FAULT_H_
#define RM_SYSCTRL_FAULT_H_

#ifdef __cplusplus
extern "C"{
#endif

enum RmSysctrlFaultType {
    FAULT_SUBSYS_DEAD		= 0,
    FAULT_EXPECT_STATE_EXPIRE	= 1,
    FAULT_INIT_SUBSYS_EXPIRE	= 2,
#ifdef DEEP_SLEEP_SUPPORT
    FAULT_DEEP_SLEEP_EXPIRE	= 3,
    FAULT_DEEP_WAKE_EXPIRE	= 4,
#endif
    FAULT_DEBUG_SET_EXPIRE	= 5,
    FAULT_MODE_SET_EXPIRE	= 6,
    FAULT_SUBSYS_FAULT		= 7,
    FAULT_MAX
};

#define FAULT_NONE -1

enum RmSysctrlFaultLevel{
    ROBOT_FAULT_WARN,	//warning
    ROBOT_FAULT_ERROR,	//error
    ROBOT_FAULT_FATAL,	//fatal
};

struct RmSysctrlFault {
    int last_fault_type;
#define DESC_MAX_LEN 100
    char last_fault_desc[DESC_MAX_LEN];
    unsigned int fault_subsys;
    unsigned int subsys_fault_type;
    int level[FAULT_MAX];
};

/****************************************************************************
 Name: RmSysctrlInitFault
 Function: This function is to initialize the fault from the params.
 Params: fault - The point of point of a fault.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlInitFault(<POINT of POINT of FAULT>);
******************************************************************************/
int RmSysctrlInitFault(struct RmSysctrlFault **rscf_addr);

/****************************************************************************
 Name: RmSysctrlFaultSetLastFaultType
 Function: This function is to set the last fault type from the params.
 Params: fault - The point of a fault.
         fault_type - The last fault type to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlFaultSetLastFaultType(<POINT of FAULT>,
                                                  <LAST FAULT TYPE>);
******************************************************************************/
int RmSysctrlFaultSetLastFaultType(struct RmSysctrlFault *rscf,
                                   int fault_type);

/****************************************************************************
 Name: RmSysctrlFaultGetLastFaultType
 Function: This function is to get the last fault type from the params.
 Params: fault - The point of a fault.
         fault_type - The last fault type to get.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlFaultGetLastFaultType(<POINT of FAULT>,
                                                  <POINT of LAST FAULT TYPE>);
******************************************************************************/
int RmSysctrlFaultGetLastFaultType(struct RmSysctrlFault *rscf,
                                   int *fault_type);

/****************************************************************************
 Name: RmSysctrlFaultSetSubsysFaultType
 Function: This function is to set the sub-system fault type from the params.
 Params: fault - The point of a fault.
         fault_type - The sub-system fault type to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlFaultSetSubsysFaultType(<POINT of FAULT>,
                                                    <LAST SUBSYS FAULT TYPE>);
******************************************************************************/
int RmSysctrlFaultSetSubsysFaultType(struct RmSysctrlFault *rscf,
                                   unsigned int fault_type);

/****************************************************************************
 Name: RmSysctrlFaultGetSubsysFaultType
 Function: This function is to get the sub-system fault type from the params.
 Params: fault - The point of a fault.
         fault_type - The sub-system fault type to get.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlFaultGetSubsysFaultType(<POINT of FAULT>,
                                                  <POINT of SUBSYS FAULT TYPE>);
******************************************************************************/
int RmSysctrlFaultGetSubsysFaultType(struct RmSysctrlFault *rscf,
                                   unsigned int *fault_type);

/****************************************************************************
 Name: RmSysctrlFaultSetFaultSubsys
 Function: This function is to set the fault sub-system ID from the params.
 Params: fault - The point of a fault.
         fault_subsys - The sub-system fault ID to set.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlFaultSetFaultSubsys(<POINT of FAULT>,
                                                <FAULT SUBSYS ID>);
******************************************************************************/
int RmSysctrlFaultSetFaultSubsys(struct RmSysctrlFault *rscf,
                                   unsigned int fault_subsys);

/****************************************************************************
 Name: RmSysctrlFaultGetFaultSubsys
 Function: This function is to get the fault sub-system ID from the params.
 Params: fault - The point of a fault.
         fault_subsys - The pointer of sub-system fault ID to get.

 Return:
   0 - SUCCESS
  -1 - FAILED
 Systax:
  [RETURN VALUE] = RmSysctrlFaultGetFaultSubsys(<POINT of FAULT>,
                                                <POINTER OF FAULT SUBSYS ID>);
******************************************************************************/
int RmSysctrlFaultGetFaultSubsys(struct RmSysctrlFault *rscf,
                                   unsigned int *fault_subsys);

/****************************************************************************
 Name: RmSysctrlFaultIsSubsysFaultFalt
 Function: This function is to check if any sub-system is in fault.
 Params: fault - The point of a fault.

 Return:
  true  - have fault sub-system
  false - havn't fault sub-system
 Systax:
  [RETURN VALUE] = RmSysctrlFaultIsSubsysFaultFalt(<POINT of FAULT>);
******************************************************************************/
bool RmSysctrlFaultIsSubsysFaultFalt(struct RmSysctrlFault *rscf);

/****************************************************************************
 Name: RmSysctrlIsValidFault
 Function: This function is to check if the fault is valid.
 Params: fault - robot sysctrl fault

 Return:
   ture  - the fault is valid
   false - the fault is not valid
 Systax:
  [RETURN VALUE] = RmSysctrlIsValidFault(<FAULT>);
******************************************************************************/
bool RmSysctrlIsValidFault(int fault);

/****************************************************************************
 Name: RmSysctrlGetFaultStr
 Function: This function is to get the string of fault.
 Params: fault - robot sysctrl fault

 Return:
   The point of sysctrl fault string
 Systax:
  [RETURN VALUE] = RmSysctrlGetFaultStr(<FAULT>);
******************************************************************************/
char* RmSysctrlGetFaultStr(int fault);
#ifdef __cplusplus
}
#endif
#endif /*RM_SYSCTRL_FAULT_H_*/
