/*
 * rto.h
 *
 *  Created on: Mar 24, 2017
 *      Author: oracle
 */

#ifndef RTO_RTO_H_
#define RTO_RTO_H_
#include <setjmp.h>
#include <time.h>
typedef unsigned int  uint32_t ;
typedef struct rtt_info{
    float rtt_rtt;      /*RTT 往返延时*/
    float rtt_srtt;     /*SRTT 平滑后的RTT估算因子*/
    float rtt_rttval;   /*rttval  平滑后的平均偏差估算因子*/
    float rtt_rto;      /*RTO 重传超时*/
    int rtt_nrexmt; /*重传计数器，每个请求都初始化为0*/
    uint32_t rtt_base;  /*时间戳基数*/
}RTT;
#define RTT_RXTMIN 3 /*最小重传超时时间*/
#define RTT_RXTMAX 60   /*最大重传超时时间*/
#define RTT_MAXNREXMT 3/*最大重传次数*/
#define RTT_RTOCALC(ptr)  (ptr)->rtt_srtt + 4*(ptr)->rtt_rttval  /*计算RTO宏*/
int rtt_init(struct rtt_info *info);
uint32_t rtt_ts(struct rtt_info *info);
void rtt_newpack(struct rtt_info *info);
float rtt_minmax(float rto);
void rtt_stop(struct rtt_info *info, uint32_t ms);
int rtt_timeout(struct rtt_info *info);
#endif /* RTO_RTO_H_ */
