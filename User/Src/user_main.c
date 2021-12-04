/*
 * user_main.c
 *
 *  Created on: 2021Äê11ÔÂ25ÈÕ
 *      Author: Sky
 */
#include "user_main.h"
#include "main.h"
#include <string.h>

static rt_timer_t timer5ms;

static struct rt_mailbox mb1;
static char mb1_pool[40];

static struct rt_mailbox mb2;
static char mb2_pool[40];
static uint8_t count;


static char mb1_str[]="mailbox1 info";
static char mb2_str[]="mailbox2 info";

static struct  rt_messagequeue mq1;
static rt_uint8_t mq_pool[100];
typedef struct {
//	rt_dev_t dev;
	rt_uint8_t  data[12];
	rt_size_t len;
}rx_msg;

static void funcInvokedPer5ms(void *param){
//	HAL_GPIO_TogglePin(GPIOC,RED_LED_Pin);
//	HAL_GPIO_TogglePin(GPIOC,GREEN_LED_Pin);
	count++;
	if(count%50==0){
		rt_mb_send(&mb2,(rt_ubase_t)&mb2_str);
	}
	if(count%80==0){
//		rt_timer_stop(timer5ms);
//		HAL_GPIO_WritePin(GPIOC,GREEN_LED_Pin,GPIO_PIN_SET);
		rt_mb_send(&mb1,(rt_ubase_t)&mb1_str);
	}
	if(count%100==0){
//		rt_kprintf("Hello RT-Thread!\n");
		rx_msg msg;
		rt_memcpy(msg.data,"queue info",12);
//		msg.data="data received";
		msg.len=12;
		rt_mq_send(&mq1,&msg,sizeof(msg));
	}
}
/***********************************************************************************************************
  *  @brief
  *
  *  @param [in] :
  *
  *  @param [out] :
  *
  *  @return :
  *
  *  @note :
  ************************************************************************************************************/
void user_thread(void * param) {
	char * str = rt_malloc(40);
	while (1) {
		if (rt_mb_recv(&mb1, (rt_ubase_t *) &str, RT_WAITING_FOREVER) == RT_EOK) {
			if (strcmp(str, mb1_str) == 0) {
				HAL_GPIO_TogglePin(GPIOC, RED_LED_Pin);
			}
		}
	}
	rt_thread_exit();
}
/***********************************************************************************************************
  *  @brief
  *
  *  @param [in] :
  *
  *  @param [out] :
  *
  *  @return :
  *
  *  @note :
  ************************************************************************************************************/
void second_thread(void * param) {
	char * str = rt_malloc(50);
	while (1) {
		if (str != RT_NULL) {
			if (rt_mb_recv(&mb2, (rt_ubase_t *) &str, RT_WAITING_FOREVER) == RT_EOK) {
				HAL_GPIO_TogglePin(GPIOC, GREEN_LED_Pin);
//				rt_thread_mdelay(400);
			}
		}
	}
}


void messageQueueReceive(void * param){
	char * str = rt_malloc(50);
	rx_msg msg;
	while(1){
		if(rt_mq_recv(&mq1,&msg,sizeof(rx_msg),RT_WAITING_FOREVER)==RT_EOK){
			rt_kprintf("%s \n",msg.data);
		}
	}
}
/***********************************************************************************************************
 *  @brief
 *
 *  @param [in] :
 *
 *  @param [out] :
 *
 *  @return :
 *
 *  @note :
 ************************************************************************************************************/
void user_main(void) {
	static rt_thread_t thread_handler;
	timer5ms = rt_timer_create("timer5ms", funcInvokedPer5ms, RT_NULL, 5, RT_TIMER_FLAG_PERIODIC);
	if (timer5ms != RT_NULL) {
		rt_timer_start(timer5ms);
	}

	static rt_err_t result;
	result=rt_mb_init(&mb2,"mb2",mb2_pool,sizeof(mb2_pool)/4,RT_IPC_FLAG_FIFO);					//initial mailbox
	if(result!=RT_EOK){
		return;
	}
	result=rt_mb_init(&mb1,"mb1",mb1_pool,sizeof(mb2_pool)/4,RT_IPC_FLAG_FIFO);					//initial mailbox
	if(result!=RT_EOK){
		return;
	}

	result=rt_mq_init(&mq1,"messageQueue",mq_pool,sizeof(rx_msg),sizeof(mq_pool),RT_IPC_FLAG_FIFO);
	if(result!=RT_EOK){
		return;
	}

	thread_handler = rt_thread_create("user_thread", user_thread, RT_NULL, 512, 4, 10);
	if (thread_handler != RT_NULL) {
		rt_thread_startup(thread_handler);
	}

	thread_handler = rt_thread_create("second_thread", second_thread, RT_NULL, 512, 4, 10);
	if (thread_handler != RT_NULL) {
		rt_thread_startup(thread_handler);
	}

	thread_handler = rt_thread_create("messageQueueReceive", messageQueueReceive, RT_NULL, 512, 4, 10);
	if (thread_handler != RT_NULL) {
		rt_thread_startup(thread_handler);
	}else{
		rt_kprintf("messageQueueReceive task create fail");
	}
}
