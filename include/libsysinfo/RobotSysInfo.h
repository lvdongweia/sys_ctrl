#ifndef __ROBOT_SYS_INFO_H_
#define __ROBOT_SYS_INFO_H_

#include <utils/RefBase.h>

namespace android {

class IBinder;

class RobotSysInfo
{
public:
    RobotSysInfo();
    ~RobotSysInfo();

    struct RobotSubSysState
    {
        bool is_enable;
        bool is_power_on;
        bool is_active;
        bool is_initalized;
        bool is_debug_en;
        int  state;
        int  sub_state;
        int  mode;
    };

    int GetSubSysVersion(int sub_sys_id, char *version, size_t len);
    int GetRobotSN(bool &is_valid, char *sn, size_t len);
    int isRobotMoving(bool &is_moving);
    int GetSubSysState(int sub_sys_id, struct RobotSubSysState &state);
    int GetRobotSex(bool &is_boy);
    int SetRobotSex(bool is_box);

    int GetRobotTime(int &year, int &month, int &day,
            int &weekday, int &hour, int &minute, int &second);
    int SetRobotTime(int year, int month, int day,
            int weekday, int hour, int minute, int second);
    int GetRobotUptime(int &uptime);

private:
    int GetRemoteService();
    bool CheckService();

    sp<IBinder> mRemoteService;
};

}; /* namespace android */


#endif /* __ROBOT_SYS_INFO_H_ */
