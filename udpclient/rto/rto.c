/*
 * rto.c
 *
 *  Created on: Mar 24, 2017
 *      Author: oracle
 */
#include"rto.h"
#include<sys/time.h>
 #include <unistd.h>
int rtt_init(struct rtt_info *info){/*仅调用一次用于初始化*/
    struct timeval t;
    bzero(info, sizeof(struct rtt_info));
    info->rtt_rttval = 0.75;/*初始化RTO为3S*/
    info->rtt_rto = info->rtt_rtt +(info->rtt_rttval)*4;/*3秒*/
    if(gettimeofday(&t, NULL) < 0 )
        return -1;
    info->rtt_base = t.tv_sec; /*取得时间戳基数，用于计算RTT*/
    return 0;
}
uint32_t rtt_ts(struct rtt_info *info){/*取得当前的时间戳*/
    struct timeval *t;
    if(gettimeofday(&t, NULL) < 0)
        return -1;
    return  (t->tv_sec - info->rtt_base)*1000 + t->tv_usec / 1000; /*毫秒为单位*/
}
void rtt_newpack(struct rtt_info *info){/*在第一次发送请求时使用*/
    info->rtt_nrexmt = 0;
}
float rtt_minmax(float rto){ /*RTO的最大最小限制*/
    if(rto > RTT_RXTMAX)
        rto = RTT_RXTMAX;
    else if(rto < RTT_RXTMIN){
        rto = RTT_RXTMIN;
    }
    return rto;
}
void rtt_stop(struct rtt_info *info, uint32_t ms){/*收到应答后，计算RTT估算因子和RTO*/
    double delte;
    info->rtt_rtt = ms / 1000;
    delte = info->rtt_rtt - info->rtt_srtt;
    info->rtt_srtt += delte/8;
    info->rtt_rttval +=(abs(delte) - info->rtt_rttval)/4;
    info->rtt_rto = rtt_minmax(RTT_RTOCALC(info));
}

int rtt_timeout(struct rtt_info *info){
    info->rtt_rto *=2;/*指数回退*/
    if((info->rtt_nrexmt++) > RTT_MAXNREXMT)
        return -1;
    else
        return  0;
}
