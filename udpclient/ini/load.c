/*
 * load.c
 *
 *  Created on: Mar 16, 2017
 *      Author: oracle
 */
#include"load.h"
int find_is_exist(const dictionary * d,const char *s)
{
	if (! iniparser_find_entry(d, s))
	    	return 0;
	else
	        return -1;
}

