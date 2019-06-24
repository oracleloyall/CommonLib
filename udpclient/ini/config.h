/*
 * config.h
 *
 *  Created on: Mar 10, 2017
 *      Author: oracle
 */

#ifndef INI_CONFIG_H_
#define INI_CONFIG_H_
#include<stdio.h>
#include"dictionary.h"
#include"iniparser.h"
#ifdef __cplusplus
extern "C" {
#endif
dictionary * init_config(const char *name);
int write_file(dictionary *dic,const char *name);
int ini_add(dictionary *dic,const char* ptr,const char *name);
int ini_modify_string(dictionary *dic,const char *sec,const char* ptr,const char *key,const char *filename);
int ini_modify_num(dictionary *dic,const char *sec,long num,const char *key,const char *filename);
int ini_find(dictionary *dic,const char* ptr);
int ini_del(dictionary *dic,const char* ptr);
typedef dictionary * (*FUNC_INIT)(const char *name);
typedef int (*FUNC_WRITE)(dictionary *dic,const char *name);
typedef int (*FUNC_ADD)(dictionary *dic,const char* ptr,const char *name);
typedef int  (*FUNC_MD_STR)(dictionary *dic,const char *sec,long num,const char *key,const char *filename);
typedef int (*FUNC_MD_NUM)(dictionary *dic,const char *sec,long num,const char *key,const char *filename);
typedef struct
{
	char devname[20];
    FUNC_INIT func_init;
    FUNC_WRITE func_write;
    FUNC_MD_STR func_string;
    FUNC_MD_NUM func_number;
    dictionary *dic;
}INI;
#ifdef __cplusplus
}
#endif
#endif /* INI_CONFIG_H_ */
