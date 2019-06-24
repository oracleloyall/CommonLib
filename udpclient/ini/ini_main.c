
/*
   Date 3/8
   
*/
/******************************************Functions************************************************/
/*
int iniparser_getnsec(dictionary * d);  //获取dictionary对象的section个数  
char * iniparser_getsecname(dictionary * d, int n); //获取dictionary对象的第n个section的名字  
void iniparser_dump_ini(dictionary * d, FILE * f);  //保存dictionary对象到file  
void iniparser_dumpsection_ini(dictionary * d, char * s, FILE * f); //保存dictionary对象一个section到file  
void iniparser_dump(dictionary * d, FILE * f);  //保存dictionary对象到file  
int iniparser_getsecnkeys(dictionary * d, char * s);    //获取dictionary对象某个section下的key个数  
char ** iniparser_getseckeys(dictionary * d, char * s); //获取dictionary对象某个section下所有的key  
char * iniparser_getstring(dictionary * d, const char * key, char * def);   //返回dictionary对象的section:key对应的字串值  
int iniparser_getint(dictionary * d, const char * key, int notfound);   //返回idictionary对象的section:key对应的整形值  
double iniparser_getdouble(dictionary * d, const char * key, double notfound);  //返回dictionary对象的section:key对应的双浮点值  
int iniparser_getboolean(dictionary * d, const char * key, int notfound);   //返回dictionary对象的section:key对应的布尔值  
int iniparser_set(dictionary * ini, const char * entry, const char * val);  //设置dictionary对象的某个section:key的值  
void iniparser_unset(dictionary * ini, const char * entry); //删除dictionary对象中某个section:key  
int iniparser_find_entry(dictionary * ini, const char * entry) ;    //判断dictionary对象中是否存在某个section:key  
dictionary * iniparser_load(const char * ininame);  //解析dictionary对象并返回(分配内存)dictionary对象  
void iniparser_freedict(dictionary * d);    //释放dictionary对象(内存)  
unsigned dictionary_hash(const char * key); //计算关键词的hash值  
dictionary * dictionary_new(int size);  //创建dictionary对象  
void dictionary_del(dictionary * vd);   //删除dictionary对象  
char * dictionary_get(dictionary * d, const char * key, char * def);    //获取dictionary对象的key值  
int dictionary_set(dictionary * vd, const char * key, const char * val);    //设置dictionary对象的key值  
void dictionary_unset(dictionary * d, const char * key);    //删除dictionary对象的key值  
void dictionary_dump(dictionary * d, FILE * out);   //保存dictionary对象  
*/
#include<stdio.h>
#include"dictionary.h"
#include"iniparser.h"
#if 0
int main(int argc,char **argv)
{
    dictionary * ini ;
    int i;
    char**k;
    char sec[80];
    char *s;
    const char * keys[10];
    char key[80];
    ini = iniparser_load("pase.ini");
    if (ini==NULL)
    {
        fprintf(stderr, "cannot parse file:\n");
        return 1 ;
    }

    k = iniparser_getseckeys(ini, "POLICY",keys);
    printf("%s   %s\n",*(k+1),keys[0]);
    if (k==NULL)
   {
	fprintf(stderr, "cannot found section\n");
	iniparser_freedict(ini);
        return 1;
   }
	
    for(i=0; i<sizeof(k);i++)
   {
	sscanf(k[i], "%[^:]:%s", &sec, &key);
	printf("set %s=%s\n" , key, iniparser_getstring(ini, k[i], NULL));
   }
    int num=iniparser_getnsec(ini);
    printf("session :%d\n",iniparser_getnsec(ini));
    for(i=0;i<num;i++)
    printf("session name:%s\n",iniparser_getsecname(ini,i));

    FILE *fp = fopen("a.ini", "w");
    if(NULL == fp) 
    { 
      printf("File: Not Found.\n"); 
    } 
    else
    { 
     printf("open \n");
     iniparser_dump_ini(ini,fp);
     fclose(fp);
    }
   num=iniparser_getsecnkeys(ini,"NET");
   printf("NET keys number is %d\n",num);
   iniparser_freedict(ini);
   return 0;
}
#endif
