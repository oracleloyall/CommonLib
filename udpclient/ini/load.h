/*
 * load.h
 *
 *  Created on: Mar 16, 2017
 *      Author: oracle
 */

#ifndef INI_LOAD_H_
#define INI_LOAD_H_
#include<stdio.h>
#include"dictionary.h"
#include"iniparser.h"
#ifdef __cplusplus
extern "C" {
#endif


int find_is_exist(const dictionary * d,const char *s);
typedef int (*FUNC_EXIST)(const dictionary * d,const char *s);
typedef struct
{
    FUNC_EXIST func_exist;
}LOAD;
#ifdef __cplusplus
}
#endif
#endif /* INI_LOAD_H_ */
