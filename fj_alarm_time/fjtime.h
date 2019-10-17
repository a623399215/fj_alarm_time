#ifndef _FJTIME_H_
#define _FJTIME_H_

#define FJ_TIME_OK	0
#define FJ_TIME_TAG	"FJ_TIME"

//TIME 的状态值
#define FJ_TIME_RUN 1
#define FJ_TIME_WAIT  2
#define FJ_TIME_STOP	 0

//每次计算块　50ms
#define FJ_TIME_NUM	50

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


typedef void (*fj_time_signal)();

/*
 *　TIME 触发类型
 */
typedef enum FJ_TIME_CON_TYPE
{
	TIME_TRI_TYPE_ONE,
	TIME_TRI_TYPE_CONTINUE,
}fjtime_con_type_t;

/*
 * 设置time 数值
 */
typedef struct FJ_TIME_VALUE
{
	int sec;
	int msec;
	
}fjtime_value_t;

/*
 * 设置control　控制器
 */
typedef struct FJ_TIME_CONTROL
{
	int  trigger_type;  //触发TYPE
		
	int  time_flag;		//计时器 标志位 

}fjtime_control_t;

/*
 *	TIME 的线程参数
 */
typedef struct FJ_TIME_PTHREAD
{
	pthread_t th;
	pthread_mutex_t mutex;
	pthread_cond_t cond;

}fjtime_thread_t;

/*
 * 设置 time handler
 */
typedef struct FJ_TIME_HANDLER
{
	fjtime_thread_t tph;
	
	fjtime_value_t val;

	fjtime_control_t cont;
	
	fj_time_signal sig_back;
	
}fjtime_handler_t;

extern int start_fjtime_value(fjtime_handler_t * fj_time,int type,int sec,int msec,fj_time_signal back);
extern void destroy_fj_time(fjtime_handler_t * fj_time);
extern void stop_fj_time(fjtime_handler_t * fj_time);
extern fjtime_handler_t* init_fj_time();


#endif
