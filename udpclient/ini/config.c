#include"config.h"
dictionary *  init_config(const char *name)
{
	return iniparser_load(name);
}
int write_file(dictionary *dic,const char *name)
{
	//open file
	//write dic to file
	FILE *fp = fopen(name, "w");
	if(NULL == fp)
	{
	   printf("File: Not Found.\n");
	   return -1;
	}
	else
	{
	   printf("open \n");
	   iniparser_dump_ini(dic,fp);
	   fclose(fp);
	   return 0;
	}
}
int ini_add(dictionary *dic,const char* ptr,const char *name)
{
	FILE    *   ini ;
	if ((ini=fopen(name, "a"))==NULL)
	{
	    fprintf(stderr, "iniparser: cannot open %s\n",name);
	    return -1;
	 }
	fprintf(ini,"%s",ptr);
	/*
	fprintf(ini,
	    "#\n"
	    "# This is an example of ini file\n"
	    "#\n"
	    "\n"
	    "[Pizza]\n"
	    "\n"
	    "Ham       = yes ;\n"
	    "Mushrooms = TRUE ;\n"
	    "Capres    = 0 ;\n"
	    "Cheese    = Non ;\n"
	    "\n"
	    "\n");
	    */
	    fclose(ini);
}
int ini_modify_string(dictionary *dic,const char *sec,const char* ptr,const char *key,const char *filename)
{
	//char *p=iniparser_getstring(dic,key,"not");
	//strncpy(p,ptr,strlen(ptr));
	//printf("%s %s\n",p,ptr);
	char sec_name[32];
	char key_name[64];
	char key_value[32];
	sprintf(sec_name, "%s", key);
	sprintf(key_name, "%s:%s", sec, sec_name);
	sprintf(key_value, "%s", ptr);
	dictionary_set(dic, key_name, key_value);
	write_file(dic,filename);
}
int ini_modify_num(dictionary *dic,const char *sec,long num,const char *key,const char *filename)
{
	char sec_name[32];
	char key_name[64];
	char key_value[32];
	sprintf(sec_name, "%s", key);
	sprintf(key_name, "%s:%s", sec, sec_name);
	sprintf(key_value, "%d", num);
	dictionary_set(dic, key_name, key_value);
	write_file(dic,filename);
}
int ini_find(dictionary *dic,const char* ptr)
{

}
int ini_del(dictionary *dic,const char* ptr)
{

}
