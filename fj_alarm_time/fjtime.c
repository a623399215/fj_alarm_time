
#include "fjtime.h"
#include "mylog.h"

/*
 * 检测 TIME 状态
 */
static int fj_time_deter_sta(fjtime_handler_t *fj_time)
{
	int ret =- 1;
	if(fj_time == NULL)
	{
		printf("fj_time_deter_sta is null\n");
		return  -1;
	}

	pthread_mutex_lock(&fj_time->tph.mutex);
   while(fj_time->cont.time_flag==FJ_TIME_WAIT)
	{
		pthread_cond_wait(&fj_time->tph.cond,&fj_time->tph.mutex);
	}

	if(fj_time->cont.time_flag == FJ_TIME_RUN)	
	{
		//printf("fj_time_deter_sta fj_time->cont.time_flag is %d\n",fj_time->cont.time_flag);
		ret = 1;
	}
   pthread_mutex_unlock(&fj_time->tph.mutex);
	return ret;
}

/*
 * 计时线程
 */
static void *time_thread(void *data)
{
	int time_start = 0;
	int time_heap = 0,time_num = FJ_TIME_NUM*1000;
	fjtime_handler_t *fj_time = (fjtime_handler_t *)data;
	int time_total = fj_time->val.sec * 1000 + fj_time->val.msec;
	int ret = -1;
	if(fj_time == NULL)
	{
		printf("time_thread is fj_time null\n");
		return ;
	}

	while(1)
	{
		ret = fj_time_deter_sta(fj_time);
		if(ret < 0)
		{
			printf("time_thread quit the start\n");
			break;
		}

		if(time_total <= time_heap)
		{
			printf("time out type=%d\n",fj_time->cont.trigger_type);
			
			if(fj_time->cont.trigger_type == TIME_TRI_TYPE_CONTINUE)
			{
				time_heap = 0;
				fj_time->sig_back();
			}else{
				fj_time->sig_back();
				break;
			}
		}
		
		usleep(time_num);
		time_heap += FJ_TIME_NUM;
	}

	//fj_time->cont.time_flag = FJ_TIME_STOP;

}


/*
 *初始化计时器
 */
fjtime_handler_t* init_fj_time()
{
	
	fjtime_handler_t *fj_time = (fjtime_handler_t *)malloc(sizeof(fjtime_handler_t));
	if(fj_time == NULL)
	{
		printf("init_fj_time malloc error\n");
		return NULL;
	}

	if(pthread_mutex_init(&fj_time->tph.mutex,NULL)!=NULL)
	{

		DEBUG_INFO("init_fj_time mutex init error\n");
		free(fj_time);	
		return NULL;
	}

	if(pthread_cond_init(&fj_time->tph.cond,NULL)!=NULL)
	{
		DEBUG_INFO("init_fj_time cond init error\n");
		free(fj_time);
		return NULL;
	}
	return fj_time;
}

/*
 * 回收计时器资源
 */
static _destroy_fj_time(fjtime_handler_t * fj_time)
{
	pthread_mutex_destroy(&fj_time->tph.mutex);
	pthread_cond_destroy(&fj_time->tph.cond);

	fj_time->cont.time_flag = FJ_TIME_STOP;
	fj_time->val.sec = 0;
	fj_time->val.msec = 0;
	fj_time->sig_back = NULL;
	free(fj_time);
	fj_time = NULL;
}

/*
 * 外部调用　停止计时器
 */
void stop_fj_time(fjtime_handler_t * fj_time)
{
	if(fj_time != NULL)
	{
		pthread_mutex_lock(&fj_time->tph.mutex);

		if(fj_time->cont.time_flag == FJ_TIME_RUN)
		{
			fj_time->cont.time_flag = FJ_TIME_STOP;

		}else if(fj_time->cont.time_flag == FJ_TIME_WAIT)
		{
			fj_time->cont.time_flag = FJ_TIME_STOP;
			pthread_cond_signal(&fj_time->tph.cond);
		}
		pthread_mutex_unlock(&fj_time->tph.mutex);
		usleep(FJ_TIME_NUM*1000);
		printf("stop_fj_time stop success\n");
	}else{
		printf("stop_fj_time is already stop\n");
	}
}

/*
 * 外部调用　回收资源
 */
void destroy_fj_time(fjtime_handler_t * fj_time)
{
	if(fj_time != NULL)
	{
		if(fj_time->cont.time_flag == FJ_TIME_STOP)
		{
			_destroy_fj_time(fj_time);
		}else{

			stop_fj_time(fj_time);
			usleep(FJ_TIME_NUM*1000);
			_destroy_fj_time(fj_time);
		}
		printf("destroy_fj_time is success\n");
	}else{
		printf("destroy_fj_time is realdy destroy\n");
	}
}

/*
 *　外部调用　开始计时
 */
int start_fjtime_value(fjtime_handler_t * fj_time,int type,int sec,int msec,fj_time_signal back)
{
	int ret = -1;

	if(fj_time == NULL || back == NULL)
	{
		printf("start_fjtime_value fj_time is null\n");
		return -1;
	}

	fj_time->val.sec = sec;
	fj_time->val.msec = msec;
	fj_time->sig_back = back;

	fj_time->cont.trigger_type = type;
	fj_time->cont.time_flag = FJ_TIME_RUN;
	ret = pthread_create(&fj_time->tph.th,NULL,(void *)time_thread,(void *)fj_time);
	if(ret != 0)
	{
		printf("start_fjtime_value  pthread_create error\n");
		return -1;
	}
	pthread_detach(fj_time->tph.th);
	return FJ_TIME_OK;
}


