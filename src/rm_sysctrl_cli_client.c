/*************************************************************************
FileName: rm_sysctrl_cli.c
Copy Right: Copyright (C), 2006-2014, Archermind. Co., Ltd.
System: RM
Module: SYSCTRL
Author: wangkun
Create Date: 2014-9-4
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
#include <netdb.h>
#include <time.h>

#define LOG_TAG "RM_SYSCTRL_CLI_CLIENT"
#include "cutils/log.h"
#include "rm_sysctrl.h"
#include "rm_sysctrl_cli.h"
#include "rm_sysctrl_fault.h"
#include "rm_sysctrl_log.h"
#include "rm_sysctrl_comm.h"
#include "rm_sysctrl_subsys.h"
#include "rm_sysctrl_fsm.h"
#include "rm_sysctrl_client.h"

/* Cli Client Section
 */
int RmSysctrlCliConnectServer();
int RmSysctrlCliDisconnectServer(int cfd);
int RmSysctrlCliCommunityRequest(int cfd,
                                 char src_module_id,
                                 void *tlv,
                                 unsigned int len);
int RmSysctrlCliCommonResponse(int cfd);
int RmSysctrlCliShowStatusRequest(int cfd);
int RmSysctrlCliShowStatusResponse(int cfd, void *sso);

int RmSysctrlCliConnectServer()
{
  int cfd;
  struct sockaddr_in s_add;
  unsigned short portnum = CLI_PORT_NUM;
  struct hostent *he = gethostbyname("localhost");

  if (he == NULL)
  {
    int retry = 10;
    while (retry--)
    {
       he = gethostbyname("localhost");
       if (he != NULL) break;
       sleep(1);
    }
    if (he == NULL)
    {
      perror("gethostbyname");
      return -1;
    }
  }

  cfd = socket(AF_INET, SOCK_STREAM, 0);
  if(-1 == cfd)
  {
    printf("socket fail ! \r\n");
    return -1;
  }

  bzero(&s_add,sizeof(struct sockaddr_in));
  s_add.sin_family=AF_INET;
  s_add.sin_addr.s_addr= *((unsigned long *)he->h_addr);
  s_add.sin_port=htons(portnum);

  if(-1 == connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
  {
    printf("connect fail !\r\n");
    return -1;
  }
  return cfd;
}

int RmSysctrlCliDisconnectServer(int cfd)
{
  close(cfd);
  return 0;
}

int RmSysctrlCliCommunityRequest(int cfd,
                                 char src_module_id,
                                 void *tlv,
                                 unsigned int len)
{
  struct Cli dm;
  dm.header.type = CLI_COMMUNITY_REQUEST;
  dm.u.community_request.src_module_id = src_module_id;
  memcpy(dm.u.community_request.tlv, tlv, len);
  dm.u.community_request.len = len;
  dm.header.len = sizeof(dm);

  if(-1 == write(cfd, (void *)&dm, dm.header.len))
  {
    printf("write fail!\r\n");
    return -1;
  }
  return 0;
}

int RmSysctrlCliSendToCanRequest(int cfd,
                                 char dst_module_id,
                                 void *tlv,
                                 unsigned int len)
{
  struct Cli dm;
  dm.header.type = CLI_SENDTOCAN_REQUEST;
  dm.u.sendtocan_request.dst_module_id = dst_module_id;
  memcpy(dm.u.sendtocan_request.tlv, tlv, len);
  dm.u.sendtocan_request.len = len;
  dm.header.len = sizeof(dm);

  if(-1 == write(cfd, (void *)&dm, dm.header.len))
  {
    printf("write fail!\r\n");
    return -1;
  }
  return 0;
}

int RmSysctrlCliSubsysEnableRequest(int cfd,
                                    char sub_sys_id,
                                    int enable)
{
  struct Cli dm;
  dm.header.type = CLI_SUBSYS_ENABLE_REQUEST;
  dm.u.subsysenable_request.sub_sys_id = sub_sys_id;
  dm.u.subsysenable_request.enable = enable;
  dm.header.len = sizeof(dm);

  if(-1 == write(cfd, (void *)&dm, dm.header.len))
  {
    printf("write fail!\r\n");
    return -1;
  }
  return 0;
}

int RmSysctrlCliCommonResponse(int cfd)
{
  int recbytes;
  char buffer[BUFFER_SIZE] = {0};
  struct Cli *pdm;

  if(-1 == (recbytes = read(cfd, buffer, BUFFER_SIZE)))
  {
    printf("read data fail !\r\n");
    return -1;
  }

  pdm = (struct Cli *) buffer;

  buffer[recbytes]='\0';
  if (pdm->header.type != CLI_COMMON_RESPONSE)
  {
    printf("Invalid messgage!\r\n");
    return -1;
  }
  //printf("ret: %d\n", pdm->u.common_response.ret);
  //printf("error_code: %d\n", pdm->u.common_response.error_code);
  if (-1 == pdm->u.common_response.ret)
  {
    printf("Remote procedure executed error!\r\n");
    return -1;
  }

  return 0;
}

int RmSysctrlCliShowStatusRequest(int cfd)
{
  struct Cli dm;
  dm.header.type = CLI_SHOW_STATUS_REQUEST;
  dm.header.len = sizeof(dm);

  if(-1 == write(cfd, (void *)&dm, dm.header.len))
  {
    printf("write fail!\r\n");
    return -1;
  }
  return 0;
}

int RmSysctrlCliShowStatusResponse(int cfd, void *system_status)
{
  int recbytes;
  char buffer[BUFFER_SIZE] = {0};
  struct Cli *pdm;

  if(-1 == (recbytes = read(cfd, buffer, BUFFER_SIZE)))
  {
    printf("read data fail !\r\n");
    return -1;
  }

  pdm = (struct Cli *) buffer;

  buffer[recbytes]='\0';
  if (pdm->header.type != CLI_SHOW_STATUS_RESPONSE)
  {
    printf("Invalid messgage!\r\n");
    return -1;
  }
  //printf("ret: %d\n", pdm->u.show_status_response.ret);
  //printf("error_code: %d\n", pdm->u.show_status_response.error_code);
  if (-1 == pdm->u.show_status_response.ret)
  {
    printf("Remote procedure executed error!\r\n");
    return -1;
  }
  memcpy(system_status,
         pdm->u.show_status_response.system_status_output,
         TEXT_SIZE);

  return 0;
}

int RmSysctrlCliListConfigRequest(int cfd)
{
  struct Cli dm;
  dm.header.type = CLI_LIST_CONFIG_REQUEST;
  dm.header.len = sizeof(dm);

  if(-1 == write(cfd, (void *)&dm, dm.header.len))
  {
    printf("write fail!\r\n");
    return -1;
  }
  return 0;
}

int RmSysctrlCliListConfigResponse(int cfd, void *list_config)
{
  int recbytes;
  char buffer[BUFFER_SIZE] = {0};
  struct Cli *pdm;

  if(-1 == (recbytes = read(cfd, buffer, BUFFER_SIZE)))
  {
    printf("read data fail !\r\n");
    return -1;
  }

  pdm = (struct Cli *) buffer;

  buffer[recbytes]='\0';
  if (pdm->header.type != CLI_LIST_CONFIG_RESPONSE)
  {
    printf("Invalid messgage!\r\n");
    return -1;
  }
  //printf("ret: %d\n", pdm->u.list_config_response.ret);
  //printf("error_code: %d\n", pdm->u.list_config_response.error_code);
  if (-1 == pdm->u.list_config_response.ret)
  {
    printf("Remote procedure executed error!\r\n");
    return -1;
  }
  memcpy(list_config,
         pdm->u.list_config_response.list_config_output,
         TEXT_SIZE);

  return 0;
}

void usage(void)
{
	printf("\r\n");
	printf("usage: rsc_cli <command> <sub-command>[options]                     \r\n");
	printf("  receive_from_can   <src_module_id>   <tlv>             <len>      \r\n");
	printf("  send_to_can        <dst_module_id>   <tlv>             <len>      \r\n");
	printf("  sub_system         <sub_system_id>   <enable|disable>             \r\n");
	printf("  show               status                                         \r\n");
	printf("  list               config                                         \r\n");
	printf("  set                datetime          <y>-<m>-<d>       <h>:<m>:<s>\r\n");
	printf("                                       e.g. 2014-10-23 14:23:00     \r\n");
	printf("                     sex               <male|female>                \r\n");

}

int is_string_all_digit(char* str, unsigned str_len)
{
    unsigned i;
    unsigned counter = 0;
    for (i=0; i<str_len; ++i, ++str)
    {
        if (*str < '0' || *str > '9')
        {
            ++counter;
        }
    }
    return counter;
}

int ConvertHexChar(char ch) 
{
    if((ch>='0')&&(ch<='9'))
        return ch-0x30;
    else if((ch>='A')&&(ch<='F'))
        return ch-'A'+10;
    else if((ch>='a')&&(ch<='f'))
        return ch-'a'+10;
    else
        return -1;
}

int ConvertToHex(char *str, void *data)
{
    char *p = str;
    char *d = data;
    int u, v, i;

    u = 0, v = 0, i = 0;
    while (*p != '\0')
    {
        v = ConvertHexChar(*p);
        if (v < 0) return -1;
        if ((p - str) % 2 == 0)
        {
            u = v;
        }
        else
        {
            u = u << 4 | v;
            d[i] = u;
            i++;
        }
        p++;
    }

    return 0;
}

int date_to_week(int y, int m, int d) {
    if (m<3) { m += 12; y--; }
    int c = y/100;
    y = y%100;
    int w = y + y/4 + c/4 - 2*c + 26*(m+1)/10 + d - 1;
    w %= 7;
    if (w<0)  w += 7;
    return w;
}

int SetSystemTime(char *dt)  
{  
    struct tm tm;
    struct tm _tm;
    struct timeval tv;
    time_t timep;

    sscanf(dt, "%d-%d-%d %d:%d:%d", &tm.tm_year,
        &tm.tm_mon, &tm.tm_mday,&tm.tm_hour,
        &tm.tm_min, &tm.tm_sec);

    _tm.tm_sec = tm.tm_sec;
    _tm.tm_min = tm.tm_min;
    _tm.tm_hour = tm.tm_hour;
    _tm.tm_mday = tm.tm_mday;
    _tm.tm_mon = tm.tm_mon - 1;
    _tm.tm_year = tm.tm_year - 1900;
  
    timep = mktime(&_tm);
    tv.tv_sec = timep;
    tv.tv_usec = 0;
    if(settimeofday (&tv, (struct timezone *) 0) < 0)
    {
        printf("Set system datatime error!\n");
        return -1;
    }
    return 0;
}

int main(int argc,char *argv[])
{
    int cfd;
    int ret;
    char cmd[20] = {0};

    cfd = RmSysctrlCliConnectServer();
    if (-1 == cfd)
    {
      printf("connect sever fail !\r\n");
      goto fail_connect_server;
    } 

    if (argc < 3)
    {
        usage();
        goto fail;
    }
    if (strlen(argv[1]) > 19)
    {
        printf("Invalid command.\r\n");
        usage();
        goto fail;
    }
    strcpy(cmd, argv[1]);
    if (!strcmp(cmd, "receive_from_can"))
    {
        int str_len;
        char src_module_id;
        char tlv[TEXT_SIZE + 2] = {0};
        int len;

        str_len = strlen(argv[2]);
        if (str_len != 2 || ConvertToHex(argv[2], &src_module_id) < 0)
        {
            printf("Invalid src_module_id.\r\n");
            usage();
            goto fail;
        }
        //printf("The src_module_id is %d\r\n", src_module_id);

        str_len = strlen(argv[4]);
        if (str_len > 2 || is_string_all_digit(argv[4], str_len) != 0)
        {
            printf("Invalid len.\r\n");
            usage();
            goto fail;
        }
        len = atoi(argv[4]);
        //printf("The len is %d\r\n", len);

        str_len = strlen(argv[3]);
	if (str_len > (TEXT_SIZE + 2) * 2 ||
            str_len != len * 2 ||
            ConvertToHex(argv[3], tlv) < 0)
	{
		printf("Invalid tlv.\r\n");
		usage();
		goto fail;
	}
        //printf("The tlv is %s\r\n", argv[3]);

	ret = RmSysctrlCliCommunityRequest(cfd, src_module_id, tlv, len);
  	if (-1 == ret)
  	{
    		printf("Send Cli request fail!\r\n");
		goto fail;
  	}

	ret = RmSysctrlCliCommonResponse(cfd);
	if (-1 == ret)
	{
		printf("Cli execute fail!\r\n");
		goto fail;
	}
    }
    else if (!strcmp(cmd, "send_to_can"))
    {
        int str_len;
        char dst_module_id;
        char tlv[TEXT_SIZE + 2] = {0};
        int len;

        str_len = strlen(argv[2]);
        if (str_len != 2 || ConvertToHex(argv[2], &dst_module_id) < 0)
        {
            printf("Invalid dst_module_id.\r\n");
            usage();
            goto fail;
        }
        //printf("The dst_module_id is %d\r\n", dst_module_id);

        str_len = strlen(argv[4]);
        if (str_len > 2 || is_string_all_digit(argv[4], str_len) != 0)
        {
            printf("Invalid len.\r\n");
            usage();
            goto fail;
        }
        len = atoi(argv[4]);
        //printf("The len is %d\r\n", len);

        str_len = strlen(argv[3]);
	if (str_len > (TEXT_SIZE + 2) * 2 ||
            str_len != len * 2 ||
            ConvertToHex(argv[3], tlv) < 0)
	{
		printf("Invalid tlv.\r\n");
		usage();
		goto fail;
	}
        //printf("The tlv is %s\r\n", argv[3]);

	ret = RmSysctrlCliSendToCanRequest(cfd, dst_module_id, tlv, len);
	if (-1 == ret)
	{
		printf("Send Cli request fail!\r\n");
		goto fail;
	}

	ret = RmSysctrlCliCommonResponse(cfd);
	if (-1 == ret)
	{
		printf("Cli execute fail!\r\n");
		goto fail;
	}
    }
    else if (!strcmp(cmd, "sub_system"))
    {
        int str_len;
        char sub_sys_id;
        char en_str[TEXT_SIZE + 2] = {0};
        char enable;

        str_len = strlen(argv[2]);
        sub_sys_id = ConvertHexChar(argv[2][0]);
        if (str_len != 1 || !(sub_sys_id < 16))
        {
            printf("Invalid sub_sys_id.\r\n");
            usage();
            goto fail;
        }
        //printf("The sub_sys_id is %d\r\n", sub_sys_id);


        strcpy(en_str, argv[3]);
	if (strcmp(en_str, "enable") && strcmp(en_str, "disable"))
	{
		printf("Invalid enable or disable operation.\r\n");
		usage();
		goto fail;
	}
        //printf("The op is %s\r\n", en_str);

        enable = (!strcmp(en_str, "enable")) ? (1) : (0);

	ret = RmSysctrlCliSubsysEnableRequest(cfd, sub_sys_id, enable);
	if (-1 == ret)
	{
		printf("Send Cli request fail!\r\n");
		goto fail;
	}

	ret = RmSysctrlCliCommonResponse(cfd);
	if (-1 == ret)
	{
		printf("Cli execute fail!\r\n");
		goto fail;
	}
    }
    else if (!strcmp(cmd, "show"))
    {
       char sub_cmd[20] = {0};
       int str_len = strlen(argv[2]);

       if (str_len > 19)
       {
           printf("Invalid sub command.\r\n");
           usage();
           goto fail;
       }

       strcpy(sub_cmd, argv[2]);
       if (!strcmp(sub_cmd, "status"))
       {
           int len;
           char system_status[TEXT_SIZE] = {0};

           ret = RmSysctrlCliShowStatusRequest(cfd);
           if (-1 == ret)
           {
                printf("Send Cli request fail!\r\n");
                goto fail;
           }

	   ret = RmSysctrlCliShowStatusResponse(cfd, system_status);
	   if (-1 == ret)
	   {
		printf("Cli execute fail!\r\n");
		goto fail;
	   }
           printf("%s\n", system_status);
       }
       else
       {
	   printf("The sub command %s doesn't support!\r\n", sub_cmd);
	   goto fail;
       }

    }
    else if (!strcmp(cmd, "list"))
    {
       char sub_cmd[20] = {0};
       int str_len = strlen(argv[2]);

       if (str_len > 19)
       {
           printf("Invalid sub command.\r\n");
           usage();
           goto fail;
       }

       strcpy(sub_cmd, argv[2]);
       if (!strcmp(sub_cmd, "config"))
       {
           int len;
           char list_config[TEXT_SIZE] = {0};

           ret = RmSysctrlCliListConfigRequest(cfd);
           if (-1 == ret)
           {
                printf("Send Cli request fail!\r\n");
                goto fail;
           }

	   ret = RmSysctrlCliListConfigResponse(cfd, list_config);
	   if (-1 == ret)
	   {
		printf("Cli execute fail!\r\n");
		goto fail;
	   }
           printf("%s\n", list_config);
       }
       else
       {
	   printf("The sub command %s doesn't support!\r\n", sub_cmd);
	   goto fail;
       }
    }
    else if (!strcmp(cmd, "set"))
    {
       char sub_cmd[20] = {0};
       int str_len = strlen(argv[2]);

       if (str_len > 19)
       {
           printf("Invalid sub command.\r\n");
           usage();
           goto fail;
       }

       strcpy(sub_cmd, argv[2]);
       if (!strcmp(sub_cmd, "sex"))
       {
           char sex_str[TEXT_SIZE + 2] = {0};
           int sex = 0;

           if (argc < 4)
           {
	       printf("Invalid sex.\r\n");
               usage();
               goto fail;
           }

           strcpy(sex_str, argv[3]);
	   if (strcmp(sex_str, "male") && strcmp(sex_str, "female"))
	   {
		printf("Invalid sex option.\r\n");
		usage();
		goto fail;
	   }
           printf("The sex to set is %s\r\n", sex_str);

           sex = (!strcmp(sex_str, "male")) ? (MALE) : (FEMALE);

	   ret = RmSysctrlSetSex(sex);
	   if (-1 == ret)
	   {
		printf("Set sex fail!\r\n");
		goto fail;
	   }
       }
       else if (!strcmp(sub_cmd, "datetime"))
       {
           char date_str[TEXT_SIZE + 2] = {0};
           char time_str[TEXT_SIZE + 2] = {0};
           char year_str[5] = {0};
           char month_str[3] = {0};
           char day_str[3] = {0};
           char hour_str[3] = {0};
           char minute_str[3] = {0};
           char second_str[3] = {0};
           int year, month, day, weekday, hour, minute, second;

           if (argc < 5)
           {
	       printf("Invalid datetime.\r\n");
               usage();
               goto fail;
           }

           strcpy(date_str, argv[3]);
           if (strlen(date_str) != 10 ||
               date_str[4] != '-' ||
               date_str[7] != '-')
           {
	       printf("Invalid date.\r\n");
               usage();
               goto fail;
           }
           
           strncpy(year_str, date_str, 4);
           strncpy(month_str, date_str + 5, 2);
           strncpy(day_str, date_str + 8, 2);
           if (is_string_all_digit(year_str, 4) != 0 ||
               is_string_all_digit(month_str, 2) != 0 ||
               is_string_all_digit(day_str, 2) != 0)
	   {
		printf("Invalid date.\r\n");
		usage();
		goto fail;
	   }
           year = atoi(year_str);
           month = atoi(month_str);
           day = atoi(day_str);
           weekday = date_to_week(year, month, day);

           strcpy(time_str, argv[4]);
           if (strlen(time_str) != 8 ||
               time_str[2] != ':' ||
               time_str[5] != ':')
           {
	       printf("Invalid time.\r\n");
               usage();
               goto fail;
           }
           
           strncpy(hour_str, time_str, 2);
           strncpy(minute_str, time_str + 3, 2);
           strncpy(second_str, time_str + 6, 2);
           if (is_string_all_digit(hour_str, 2) != 0 ||
               is_string_all_digit(minute_str, 2) != 0 ||
               is_string_all_digit(second_str, 2) != 0)
	   {
		printf("Invalid time.\r\n");
		usage();
		goto fail;
	   }
           hour = atoi(hour_str);
           minute = atoi(minute_str);
           second = atoi(second_str);

	   ret = RmSysctrlSetDatetime(year, month, day, weekday, hour,
               minute, second);
	   if (-1 == ret)
	   {
		printf("Set datetime fail!\r\n");
		goto fail;
	   }
       }
       else
       {
	   printf("The sub command %s doesn't support!\r\n", sub_cmd);
	   goto fail;
       }
    }
    else
    {
	printf("The command %s doesn't support!\r\n", cmd);
	goto fail;
    }

    RmSysctrlCliDisconnectServer(cfd);
    return 0;

fail:
    RmSysctrlCliDisconnectServer(cfd);
fail_connect_server:
    return -1;
}
