#include"Callback.h"
#include"jsw_rbtree.h"
#include"thpool.h"
#include"ini/dictionary.h"
#include"ini/iniparser.h"
#include"ini/config.h"
#include"ini/load.h"
#include"define.h"
//add 3/22
#include"timer/timer.h"
extern map_t TIMER_;
//#define KEY_MAX_LENGTH (16)
//#define KEY_COUNT (1024)
map_t mymap;
map_t map;
//head check
extern int fd;
extern threadpool thpool;
extern struct jsw_rbtree *centroidset;
dictionary * device_update = NULL;
INI ini;
INI copy;
#define WAITTIME 5

unsigned char hmac_key[] = {
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
	0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36,
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
	0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36
};

void task5() {
	sleep(8);
	DEV *centroid;
	jsw_rbtrav_t *rbtrav;
	rbtrav = jsw_rbtnew();
	if (!centroidset) {
		printf("%s:centroidset is null\n",__FUNCTION__);
		return;
	}
	strncpy(ini.devname, "device.ini", 10);
	ini.func_init = &init_config;
	ini.dic = ini.func_init(ini.devname);
	ini.func_write = &write_file;
	strncpy(copy.devname, "copy.ini", 8);
	copy.func_init = &init_config;
	copy.dic = ini.func_init(ini.devname);
	copy.func_write = &write_file;
	centroid = jsw_rbtfirst(rbtrav, centroidset);
	if (centroid == NULL)
		return;
	do {
		//
		long size = get_file_size("copy.ini");
		if (size > 0) {
			//printf("copy.ini return \n");
			return;
		}
		char key[16] = "\0";
		printf("%s:check trans %s\n",__FUNCTION__, inet_ntoa(centroid->addr.sin_addr));
		sprintf(key, "%s", inet_ntoa(centroid->addr.sin_addr));
		//modify by 3/16

		// if(-1==find_is_exist(copy.dic,key))
		//   continue;

		//  printf("new key %s has load \n",key);
		//
		char buff[1024] = "\0";
		sprintf(buff, "[%s]\nstate=\ntime=\noffset=\nfile=\nsize=\n", key);
		//printf("buff is %s\n", buff);
		ini_add(copy.dic, buff, "copy.ini");
	} while ((centroid = jsw_rbtnext(rbtrav)) != NULL);
	jsw_rbtdelete(rbtrav);
}
void task4() {
	while (1) {
		sleep(WAITTIME);
		DEV *centroid;
		jsw_rbtrav_t *rbtrav;
		rbtrav = jsw_rbtnew();
		if (!centroidset) {
			PLOG_ERROR("task 4 centroidset error\n");
			return;
		}
		centroid = jsw_rbtfirst(rbtrav, centroidset);
		if (centroid == NULL)
			return;
		do {
			char key[16] = "\0";
			UPDEV_INFO *ptr = NULL;
			PLOG_DEBUG("check trans %s\n", inet_ntoa(centroid->addr.sin_addr));
			sprintf(key, "%s", inet_ntoa(centroid->addr.sin_addr));
			int err = hashmap_get(map, key, (void**) (&ptr));
			if (err != MAP_MISSING)	 // find device
			{
				long now = time(NULL);
				PLOG_DEBUG("now %d ptr->time=%d\n", now, ptr->time);
				if (((now - ptr->time) > 10) && ptr->state != 4
						&& ptr->state != 2) {
					PLOG_DEBUG("Over time \n");
					//update info to ini file
					char buff[1024] = "\0";
					ptr->state = 2;	 //break
					sprintf(buff,
							"[%s]\nstate=%d\ntime=%ld\noffset=%d\nfile=%s\nsize=%d\n",
							key, ptr->state, ptr->time, ptr->offset, ptr->file,
							ptr->send_size);
					ini_modify_string(copy.dic, key, ptr->file, "file",
							copy.devname);
					ini_modify_num(copy.dic, key, ptr->offset, "offset",
							copy.devname);
					ini_modify_num(copy.dic, key, ptr->state, "state",
							copy.devname);
					ini_modify_num(copy.dic, key, ptr->time, "time",
							copy.devname);
					ini_modify_num(copy.dic, key, ptr->send_size, "size",
							copy.devname);
					//   ini_add(ini.dic,buff,"copy.ini");
					// ini.func_write(ini.dic,"copy.ini");
				} else if (((now - ptr->time > WAITTIME)
						& (now - ptr->time < 2 * WAITTIME))
						&& ((4 == ptr->state) || (3 == ptr->state)
								|| (5 == ptr->state))) {
					printf("%s:update write to copy.ini\n",__FUNCTION__);
					char buff[1024] = "\0";
					sprintf(buff,
							"[%s]\nstate=%d\ntime=%ld\noffset=%d\nfile=%s\nsize=%d\n",
							key, ptr->state, ptr->time, ptr->offset, ptr->file,
							ptr->send_size);
					ini_modify_string(copy.dic, key, ptr->file, "file",
							copy.devname);
					ini_modify_num(copy.dic, key, ptr->offset, "offset",
							copy.devname);
					ini_modify_num(copy.dic, key, ptr->state, "state",
							copy.devname);
					ini_modify_num(copy.dic, key, ptr->time, "time",
							copy.devname);
					ini_modify_num(copy.dic, key, ptr->send_size, "size",
							copy.devname);
				}
			} else {
				//printf("not find\n");
				continue;
			}

		} while ((centroid = jsw_rbtnext(rbtrav)) != NULL);
		jsw_rbtdelete(rbtrav);
	}
}
int send_restart(int ip) {
	DEV *ptr = NULL;
	ptr = find_device(ip);
	if (ptr == NULL) {
		printf("%s:Not find device \n",__FUNCTION__);
		return -1;
	} else
		printf("%s:find device :%d :%d \n",__FUNCTION__, ptr->remote_ip, ptr->sn);
	//F=0x04，T=0x02， P0=0，无数据D
	char buf[PKT_SIZE], hmac[64];
	struct dms_pkt *pkt = (struct dms_pkt *) buf;
	pkt->type = 0x2;
	pkt->version = 0x1;
	pkt->company = 0x4;

	unsigned int sn_return = sn_ret(ptr->remote_ip);
	if (-1 == sn_return) {
		printf("end return sn error\n");
		// sleep(5);
		return -1;
	}
	pkt->sn = htonl(++sn_return);
	sn_set(ptr->remote_ip, sn_return);
	pkt->action = 0x4;
	pkt->para = 0x0;
	pkt->len = htons(0);
	//pkt->data=(unsigned char *)malloc(sizeof(16));
	//memset(pkt->data,0,sizeof(pkt->data));
	//sm3_hmac(hmac_key, 32, buf + SEC_HEAD_LEN - 4, 4 + BODY_HEAD_LEN + rand_len, hmac);
	memset(pkt->hmac, 0, sizeof(pkt->hmac));
	memcpy(pkt->hmac, "zx", 2);
	printf("retart print\n");
	sendto(fd, buf, sizeof(struct dms_pkt), 0, (struct sockaddr*) &ptr->addr,
			sizeof(struct sockaddr));
	//free(pkt->data);
	return 0;
}
int send_devinfo(int ip) {
	//F=0x05，T=0x02， P0=0，无数据D
	DEV *ptr = NULL;
	ptr = find_device(ip);
	if (ptr == NULL) {
		printf("%s:Not find device \n",__FUNCTION__);
		return -1;
	} else
		printf("find device :%d :%d \n", ptr->remote_ip, ptr->sn);
	char buf[PKT_SIZE] = "\0", hmac[64] = "\0";
	struct dms_pkt *pkt = (struct dms_pkt *) buf;
	pkt->type = 0x2;
	pkt->version = 0x1;
	pkt->company = 0x5;

	unsigned int sn_return = sn_ret(ptr->remote_ip);
	if (-1 == sn_return) {
		printf("end return sn error\n");
		// sleep(5);
		return -1;
	}
	pkt->sn = htonl(++sn_return);
	sn_set(ptr->remote_ip, sn_return);
	pkt->action = 0x5;
	pkt->para = 0x0;
	pkt->len = htons(4);
	//pkt->data=(unsigned char *)malloc(sizeof(16));
	memset(pkt->data, 0, sizeof(pkt->data));
	memcpy(pkt->data, "randbufdata", strlen("randbufdata"));
	//sm3_hmac(hmac_key, 32, buf + SEC_HEAD_LEN - 4, 4 + BODY_HEAD_LEN + rand_len, hmac);
	memset(pkt->hmac, 0, sizeof(pkt->hmac));
	memcpy(pkt->hmac, "zx", 2);
	printf("send devinfo  print\n");
	sendto(fd, buf, sizeof(struct dms_pkt), 0, (struct sockaddr*) &ptr->addr,
			sizeof(struct sockaddr));

	return 0;
}
int send_sysinfo(int ip) {
	DEV *ptr = NULL;
	ptr = find_device(ip);
	if (ptr == NULL) {
		printf("%s:Not find device \n",__FUNCTION__);
		return -1;
	} else
		printf("find device :%d :%d \n", ptr->remote_ip, ptr->sn);
	char buf[PKT_SIZE] = "\0", hmac[64] = "\0";
	struct dms_pkt *pkt = (struct dms_pkt *) buf;
	pkt->type = 0x2;
	pkt->version = 0x1;
	pkt->company = 0x6;

	unsigned int sn_return = sn_ret(ptr->remote_ip);
	if (-1 == sn_return) {
		printf("end return sn error\n");
		// sleep(5);
		return -1;
	}
	pkt->sn = htonl(++sn_return);
	sn_set(ptr->remote_ip, sn_return);
	pkt->action = 0x6;
	pkt->para = 0x0;
	pkt->len = htons(4);
	memset(pkt->data, 0, sizeof(pkt->data));
	memset(pkt->hmac, 0, sizeof(pkt->hmac));
	memcpy(pkt->data, "randbufdata", strlen("randbufdata"));
	//sm3_hmac(hmac_key, 32, buf + SEC_HEAD_LEN - 4, 4 + BODY_HEAD_LEN + rand_len, hmac);
	memcpy(pkt->hmac, "zx", 2);
	printf("sysinfo  print\n");
	sendto(fd, buf, sizeof(struct dms_pkt), 0, (struct sockaddr*) &ptr->addr,
			sizeof(struct sockaddr));
	return 0;
}
int send_set_sysinfo(int ip, char * buff) {
	DEV *ptr = NULL;
	ptr = find_device(ip);
	if (ptr == NULL) {
		printf("%s:Not find device \n",__FUNCTION__);
		return -1;
	} else
		printf("find device :%d :%d \n", ptr->remote_ip, ptr->sn);
	//F=0x07，T=0x02， P0=0，无数据D
	char buf[PKT_SIZE] = "\0", hmac[64] = "\0";
	UDP *pkt = (UDP *) buf;
	memset(pkt->hmac, 0, sizeof(pkt->hmac));
	pkt->type = 0x2;
	pkt->version = 0x1;
	pkt->company = 0x7;

	unsigned int sn_return = sn_ret(ptr->remote_ip);
	if (-1 == sn_return) {
		printf("end return sn error\n");
		// sleep(5);
		return -1;
	}
	pkt->sn = htonl(++sn_return);
	sn_set(ptr->remote_ip, sn_return);
	pkt->action = 0x7;
	pkt->para = 0x0;
	pkt->len = htons(4);
	//pkt->data=(unsigned char *)malloc(sizeof(16));
	memcpy(&pkt->packet.devsys, "randbufdata", strlen("randbufdata"));
	//sm3_hmac(hmac_key, 32, buf + SEC_HEAD_LEN - 4, 4 + BODY_HEAD_LEN + rand_len, hmac);
	memcpy(pkt->hmac, "zx", 2);
	printf("retart print\n");
	sendto(fd, buf, sizeof(UDP), 0, (struct sockaddr*) &ptr->addr,
			sizeof(struct sockaddr));
	//
	return 0;
}
int send_upgrade(int ip) {
	//F=0x09，T=0x02， P0=0，数据D=升级文件信息。
//	char key_string[16]="\0";
	printf("send update device \n");
	PLOG_DEBUG("send update device \n");
	DEV *ptr = NULL;
	ptr = find_device(ip);
	if (ptr == NULL) {
		printf("%s:Not find device \n",__FUNCTION__);
		return -1;
	} else
		printf("find device :%d :%d \n", ptr->remote_ip, ptr->sn);
	char buf[PKT_SIZE] = "\0", hmac[64] = "\0";
	struct dms_update udp;
	udp.action = 0x09;
	udp.version = 0x1;
	udp.company = 0x4;
	unsigned int sn_return = sn_ret(ptr->remote_ip);
	if (-1 == sn_return) {
		printf("end return sn error\n");
		// sleep(5);
		return -1;
	}
	udp.sn = htonl(++sn_return);
	sn_set(ptr->remote_ip, sn_return);
	udp.para = 0x0;
	udp.len = htons(4);
	printf("size %ld\n", get_file_size("a.tar.gz"));
	udp.packet.reqmsg.len = htonl(get_file_size("a.tar.gz"));
	sprintf(udp.packet.reqmsg.msg, "zx", 2);
	printf("send packet %d %s\n", get_file_size("a.tar.gz"), udp.packet.reqmsg.msg);
	memcpy(buf, &udp, sizeof(UDP));
	sendto(fd, buf, sizeof(struct dms_update), 0, (struct sockaddr*) &ptr->addr,
			sizeof(struct sockaddr));
#ifndef TRANS
	TM_MAP *tra = NULL;
	int err = hashmapi_get(map, ip, (void**) (&tra));
	if (err == MAP_MISSING)	//not fond device
	{
		printf("TRANS timer first  register :%s %d \n", __FILE__, __LINE__);
		tra = (TM_MAP *) malloc(sizeof(TM_MAP));
		TM_DATA *tm_data = (TM_DATA *) balloc(sizeof(TM_DATA));
		tm_data->type = 0x09;
		memcpy(&tm_data->in, &ptr->addr, sizeof(struct sockaddr_in));
		memcpy(&tm_data->udp, &udp, sizeof(UDP));
		if (tra == NULL)
			printf("balloc error %s %d\n", __FILE__, __LINE__);
		tra->ip = ip;
		tra->t = register_reltimer(5, 0, retrans, (char *) tm_data);
		int error = hashmapi_put(TIMER_, tra->ip, tra);
		if (error == -1) {
			PLOG_WARNING("hash map insert device remote %d error", ip);
			return -1;
		}


	} else {
		printf("TRANS timer register :%s %d \n", __FILE__, __LINE__);
		TM_DATA *tm_data = (TM_DATA *) balloc(sizeof(TM_DATA));
		tm_data->type = 0x09;
		memcpy(&tm_data->in, &ptr->addr, sizeof(struct sockaddr_in));
		memcpy(&tm_data->udp, &udp, sizeof(UDP));
		tra->t = register_reltimer(5, 0, retrans, (char *)tm_data);

	}
#endif
}
int head_check(unsigned char *buff, struct sockaddr_in *from) {
	//check ok dispatch
	//printf("Head Check\n");
	CALLBACK * value = NULL;
	if (!buff)
		return 0;
	struct dms_pkt *pkt = (struct dms_pkt *) buff;
	//need check hmac
	//(pkt->action)
	char buf[10] = { 0 };
	sprintf(buf, "%d", pkt->action);
	if (hashmap_get(mymap, buf, (void**) (&value)) == MAP_MISSING) {
		PLOG_ERROR("Dispatch action error ");
		return -1;
	} else {
#if 0
		printf("%s packet type:%d\n", __FUNCTION__,pkt->action);
#endif
		value->func.function(pkt->action, buff, from);
		return 0;
	}

}
//function time return
int time_funciton(int type, unsigned char *buff, struct sockaddr_in *from) {
#if 0
	printf("dispatch time function enter  \n");
#endif
#if 1
	UDP pkt;
	memcpy(&pkt, buff, sizeof(UDP));
	time_t now;
	time(&now);
	pkt.type = 0x01;
	pkt.version = 0x01;
	pkt.company = 0x04;
	// pkt.hmac;
	memcpy(pkt.hmac, "zx", 2);
	pkt.sn = htonl(0);

	pkt.action = 0x01;
	pkt.para = 0;
	pkt.len = htons(4);
	pkt.packet.timepac.time = htonl(now);
	memcpy(buff, &pkt, sizeof(UDP));
	sendto(fd, buff, sizeof(UDP), 0, (struct sockaddr*) from,
			sizeof(struct sockaddr));
#endif
#if 0
	struct dms_pkt *pkt=(struct dms_pkt * )balloc(sizeof(struct dms_pkt ));
	memcpy(pkt,buff,sizeof(struct dms_pkt));
	time_t now;
	time(&now);
	memset(pkt->data,0,sizeof(pkt->data));
	sprintf(pkt->data,"%d",htonl(now));
	pkt->type=0x01;
	pkt->para = 0x0;
	pkt->len = htons(4);
	pkt->action=0x01;
	//sm3_hmac(hmac_key, 32, buf + SEC_HEAD_LEN - 4, 4 + BODY_HEAD_LEN + 4, hmac);
	//memcpy(pkt->hmac, hmac, 2);
	if(!fd)
	{
		bfree(pkt);
		PLOG_ERROR("Server fd is error\n");
		return -1;
	}
	sendto(fd, buff,sizeof(struct dms_pkt), 0, (struct sockaddr*)from, sizeof(struct sockaddr));
	bfree(pkt);
	printf("time function end\n");
#endif
	return 0;
}

int restart_funciton(int type, unsigned char *buff, struct sockaddr_in *from) {
	printf("dispatch restart  function enter  \n");
	if (!buff)
		return 0;
	UDP *pkt = (UDP *) buff;
	int state = pkt->para;
	PLOG_WARNING("Restart device is  %d\n", ntohl(from->sin_addr.s_addr));
}
int devinfo_funciton(int type, unsigned char *buff, struct sockaddr_in *from) {
	printf("dispatch device info  function enter  \n");
	if (!buff) {
		PLOG_WARNING("Devinfo buff is NULL");
		return -1;
	}
	UDP *pkt = (UDP*) buff;
	DEV_INFO *info = (DEV_INFO*) balloc(sizeof(DEV_INFO));
	memcpy(info, buff + 12, sizeof(DEV_INFO));
	//DEV_INFO *info=pkt->data;
	//insert into map
	DEV *device = find_device(from->sin_addr.s_addr);
	if (device) {
		// device->info=(DEV*)malloc(sizeof(DEV));
		printf("find dev info device\n");
		memcpy(device->info.dev_name, info->dev_name, 40);
		device->info.dev_type = ntohl(info->dev_type);
		device->info.dev_runtime = ntohl(info->dev_runtime);
		device->info.dev_factory = ntohl(info->dev_factory);
		device->info.dev_version = info->dev_version;
		char *p = &info->dev_version;

		printf(
				"dev_name :%s dev_type:%d dev_runtime=%d dev_fac:%d dev_ver:%d \n",
				device->info.dev_name, device->info.dev_type,
				device->info.dev_runtime, device->info.dev_factory,
				device->info.dev_version);
		//printf("%d %d %d %d\n", p[0], p[1], p[2], p[3]);
		PLOG_WARNING(
				"dev_name :%s dev_type:%d dev_runtime=%d dev_fac:%d dev_ver:%d \n",
				device->info.dev_name, device->info.dev_type,
				device->info.dev_runtime, device->info.dev_factory,
				device->info.dev_version);
		bfree(info);
		return 0;
	} else {
		PLOG_WARNING("Device not in map \n");
		bfree(info);
		return -1;
	}

}
int sysinfo_function(int type, unsigned char *buff, struct sockaddr_in *from) {
	printf("dispatch sysinfo function enter  \n");
	if (!buff)
		return -1;
	UDP *pkt = (UDP *) buff;
	//modify 3/30
	/*
	 * struct dms_pkt *pkt = (struct dms_pkt *) buff;
	 if (!pkt->data)
	 return -1;
	 */
	DEVSYS sys;
	char name[40] = "\0";
	memcpy(&sys, buff + 12, sizeof(DEVSYS));
	int writesize = strlen(sys.devini);
	sprintf(name, "sysinfo/%s.ini", inet_ntoa(from->sin_addr));
	int fp = fopen(name, "w");
	if (fwrite(sys.devini, sizeof(char), writesize, fp) < writesize)
		printf("%s:write error",__FUNCTION__);
	fclose(fp);

//	DEV_SYS *sys = (DEV_SYS *) balloc(sizeof(DEV_SYS));
	//memcpy(sys, pkt->data, sizeof(DEV_SYS));
	//strcpy(sys,pkt->data);
//	DEV_INFO *info=pkt->data;
	//insert into map
	DEV *device = find_device(from->sin_addr.s_addr);
	if (device) {
		//modify 3/20
		/*
		 if (strlen(pkt->data) < sizeof(DEV_SYS))
		 return -1;
		 strcpy(device->sys.sys_type, sys->sys_type);
		 strcpy(device->sys.sys_dial, sys->sys_dial);
		 strcpy(device->sys.sys_net, sys->sys_net);
		 strcpy(device->sys.sys_serial, sys->sys_serial);
		 strcpy(device->sys.sys_policy, sys->sys_policy);
		 bfree(sys);
		 */
	} else
		;
	//bfree(sys);
}
int set_sysinfo_function(int type, unsigned char *buff,
		struct sockaddr_in *from) {
	printf("dispatch set sysinfo  function enter  \n");
	if (!buff)
		return 0;
	struct dms_pkt *pkt = (struct dms_pkt *) buff;
	int state = pkt->para;
}
int negotiate_funciton(int type, unsigned char *buff, struct sockaddr_in *from) {
//F=0x03，T=0x03，P0=0，D=密钥交换数据
	printf("dispatch negotiate function enter  \n");
	UDP *pkt = (UDP *) buff;
	char decbuf[100];
	int inlen = 0, outlen = 0, len = 0, pos = 0;
	int ret = 0;
	char *data = (char *)(&pkt->packet.rcvike);
	EC_KEY *tmpkey;

	DEV *device = find_device(from->sin_addr.s_addr);
	if (device) {
		inlen = data[1] + 2;
		outlen = SM2_private_decrypt(data, inlen, decbuf, sigkey);
		if (outlen != 64)
			return 0;
		pos += inlen;

		memcpy(&len, data + pos, 4);
		pos += 4;
		tmpkey = ReadPublicKey(data + pos, ntohl(len));
		if(!tmpkey)
			return 0;
		
		pos += ntohl(len);

		ret = SM2DSA_verify(0, data + inlen + 4, ntohl(len), NULL, 0, data + pos, 64, tmpkey);
		if (ret != 1)
			return 0;
		
		memcpy(device->key, decbuf, 16);
		memcpy(device->iv, decbuf + 16, 16);
		memcpy(device->hmac_key, decbuf + 32, 32);
		if(device->eckey){
			EC_KEY_free(device->eckey);
			device->eckey = NULL;
		}
		device->eckey = (void *)tmpkey;
	}
/*		pkt->para = 0x0;
	pkt->len = htons(4);
	//sm3_hmac(hmac_key, 32, buf + SEC_HEAD_LEN - 4, 4 + BODY_HEAD_LEN + 4, hmac);
	//memcpy(pkt->hmac, hmac, 2);
	if (!fd) {
		//	free(pkt->data);
		return 0;
	}
	sendto(fd, buff, sizeof(struct dms_pkt), 0, (struct sockaddr*) from,
			sizeof(struct sockaddr));
	//free(pkt->data);*/	

}
int state_funciton(int type, unsigned char *buff, struct sockaddr_in *from) {
#ifdef STATE
	printf("dispatch state function enter  \n");
#endif
	struct dms_pkt *pkt = (struct dms_pkt *) buff;
	//验证报文体 -》更新设备map状态
	DEV *ptr = NULL;
	ptr = find_device(from->sin_addr.s_addr);
	if (ptr == NULL)
		printf("%s:Not find device \n",__FUNCTION__);
	else {
		ptr->online = 1;
	}

}

int test_funciton(int type, unsigned char *buff, struct sockaddr_in *from) {
	printf("%s,test function %s\n", __FUNCTION__,buff);
	start_response(type, buff, from);
#if 1
	struct dms_pkt *pkt = (struct dms_pkt *) balloc(sizeof(struct dms_pkt));
	// pkt->data=(unsigned char *)balloc(sizeof(200));
	memcpy(pkt, buff, sizeof(struct dms_pkt));
	printf("test:%d %d %d %d", pkt->action, pkt->sn, pkt->version, pkt->type);
	//char BUFF[1024]="\0";
	// pkt->data=(unsigned char *)malloc(1024);
	// strcpy(BUFF,pkt->data);
	//  memcpy(BUFF,pkt->data,strlen(pkt->data));
	//printf("recv data:%s\n", pkt->data);
	// bfree(pkt->data);
	// bfree(pkt);
#endif
#if 0
	struct dms_pkt *pkt = (struct dms_pkt *)buff;
	char BUFF[1024]="\0";
	// pkt->data=(unsigned char *)malloc(1024);
	memcpy(BUFF,pkt->data,sizeof(BUFF));
	printf("recv data:%s\n",BUFF);
#endif
}

unsigned long get_file_size(const char *filename) {
	struct stat buf;
	if (stat(filename, &buf) < 0) {
		return 0;
	}
	return (unsigned long) buf.st_size;
}
unsigned long get_file_pos(const FILE * file, unsigned int size) {
	fseek(file, size, SEEK_SET);
	return ftell(file);
}
char key_string[16] = "\0";
int start_response(int type, unsigned char *buff, struct sockaddr_in *from) {
	//UPDEV_INFO
	//RSPMSG
	printf("dispatch start response\n");
	UPDEV_INFO *ptr = NULL;
	RSPMSG msg;
	UDP packet;
	char key_string[16] = "\0";
	memcpy(&packet, buff, sizeof(UDP));
	memcpy(&msg, &packet.packet.rsqmsg, sizeof(RSPMSG));
#ifndef TRANS
	TM_MAP *tra = NULL;
	int eno=hashmapi_get(TIMER_,from->sin_addr.s_addr,(void **)(&tra));
	if (eno == MAP_MISSING)	//not fond device
	{
		printf("Not find %d:%s %d \n",from->sin_addr.s_addr, __FILE__, __LINE__);
	}
	else
	{
		printf("%s %d  Find %d:\n", __FILE__, __LINE__,from->sin_addr.s_addr);
		cancel_timer(tra->t);
		tra->t=NULL;
	}
#endif

	//sprintf(key_string,"%d",from->sin_addr.s_addr);
	sprintf(key_string, "%s", inet_ntoa(from->sin_addr));
	//memcpy(key_string,inet_ntoa(from->sin_addr),strlen(inet_ntoa(from->sin_addr)))
//	printf("start device  is %s\n", key_string);
	//printf("%s %d \n", __FILE__, __LINE__);

	int err = hashmap_get(map, key_string, (void**) (&ptr));
	//printf(":%s %d \n", __FILE__, __LINE__);
	if (err == MAP_MISSING)	//not fond device
	{
		//printf("insert device info \n");
		//sprintf(key_string,"%s",inet_ntoa(from->sin_addr));
		ptr = (UPDEV_INFO *) malloc(sizeof(UPDEV_INFO));
		ptr->ip = from->sin_addr.s_addr;
		ptr->offset = msg.recv_len;
		ptr->state = 1;
		ptr->len = msg.max_len;
		ptr->send_size = 0;
		memcpy(&ptr->Ip, from, sizeof(struct sockaddr_in));
		sprintf(ptr->file, "a.tar.gz");
		ptr->time = time(NULL);
		strcpy(ptr->key_string, key_string);
		int error = hashmap_put(map, ptr->key_string, ptr);
		if (error == -1) {
			PLOG_WARNING("hash map insert device remote %s error", key_string);
			return -1;
		}
		//   sleep(4);
	} else {
		//printf(":%s %d \n", __FILE__, __LINE__);
		// ptr->ip=from->sin_addr.s_addr;
		// ptr->offset=msg.recv_len;
		// ptr->state=1;
		// ptr->len=msg.max_len;
		// memcpy(&ptr->Ip,from,sizeof(struct sockaddr_in));
		//sprintf(ptr->file,"1");
		ptr->time = time(NULL);
		//strcpy(ptr->key_string,key_string);
		ptr->offset = msg.recv_len;
		ptr->state = 1;
		ptr->len = msg.max_len;
		//printf("start response 2 \n");
	}
	//trans data
//	memset(ptr->file,0,50);
	// sprintf(ptr->file,"1");
	PLOG_ERROR("zx Device:%s  recv len:%d\n  %d\n", key_string, ptr->offset,
			msg.recv_len);
	if (0x0C == packet.para) {
		ptr->state = 3;
		PLOG_ERROR("Device:%s  remote get 0x0c ", key_string);
		return -1;
	} else if (0x0D == packet.para) {
		ptr->state = 5;
		PLOG_ERROR("Device:%s  remote get 0x0D ", key_string);
		return -1;
	}
	if (ptr->file) {
		printf("%s: trans file to: %s\n",__FUNCTION__,key_string);
		FILE *fp = NULL;
		ptr->total_size = get_file_size(ptr->file);
		fp = fopen(ptr->file, "r");
		trans_data(type, buff, from);   // put thread job
#ifndef TRANS

#endif
		if (fp)
			free(fp);
		return 0;
	}

}
//long LEN=0;

int trans_data(int type, unsigned char *buff, struct sockaddr_in *from) {
	UPDEV_INFO *ptr = NULL;
	//ptr=(UPDEV_INFO*)malloc(sizeof(UPDEV_INFO));
	printf("dispatch enter function trans data\n");
#ifndef TRANS
//recv
#endif
	char key_string[16] = "\0";
	sprintf(key_string, "%s", inet_ntoa(from->sin_addr));
	//printf("key is %s\n", key_string);
	//sleep(5);
	int err = hashmap_get(map, key_string, (void**) (&ptr));
	if (err == MAP_MISSING) {
		PLOG_ERROR("Not find device %s", key_string);
		return 0;
	}
	FILE *fp = NULL;
	ptr->total_size = get_file_size(ptr->file);
	fp = fopen(ptr->file, "r");
	//memcpy(&ptr->Ip,from,sizeof(struct sockaddr_in));
	PLOG_INFO(
			"device:%s ip=%d len=%d offset=%d send_size=%d time=%d file=%s total_size=%d",
			key_string, ptr->ip, ptr->len, ptr->offset, ptr->send_size,
			ptr->time, ptr->file, ptr->total_size);
	//sleep(5);
	struct dms_update udp;
	int len = 0;
	char buf[PKT_SIZE];
	udp.type = 0x02;
	udp.action = 0x08;
	udp.version = 0x1;
	udp.company = 0x4;
	unsigned int sn_return = sn_ret(from->sin_addr.s_addr);
	if (-1 == sn_return) {
		PLOG_ERROR("return sn error");
		//sleep(5);
		return -1;
	}
	udp.sn = htonl(++sn_return);
	sn_set(from->sin_addr.s_addr, sn_return);
	udp.para = 0x0;
	udp.len = htons(4);
	udp.packet.senpac.digest=0;
	udp.packet.senpac.offset = ptr->offset;
	udp.packet.senpac.len = htonl(ptr->len);
	//printf("file name :%s \n", ptr->file);
	//   FILE *fp = fopen(ptr->file, "r");
	if (NULL == fp) {
		PLOG_ERROR("Remote file:%s Not Found.\n", ptr->file);
	} else {
		//
		printf("file offset is %d\n", ptr->offset);
		//  fseek(fp,ptr->offset,SEEK_SET);
		fseek(fp, ptr->offset, SEEK_SET);
		//  printf("offset is %d\n",LEN);
		memset(&udp.packet.senpac.packet, 0, sizeof(SEDPAC));
		if ((len = fread(udp.packet.senpac.packet, sizeof(char), 1000, fp))
				> 0) {
			//udp.packet.senpac.packet,
			//sprintf(udp.packet.reqmsg.msg,"zx",2);
			udp.packet.reqmsg.len = len;
			printf("send packet %d %s\n", udp.packet.senpac.len,
					udp.packet.senpac.packet);
			memcpy(buf, &udp, sizeof(UDP));
			sendto(fd, buf, sizeof(struct dms_update), 0,
					(struct sockaddr*) from, sizeof(struct sockaddr));
			ptr->offset += len;
			ptr->send_size += len;
			ptr->time = time(NULL);
			// LEN+=len;
		//	printf("121933 len %d :offset is %d\n", len, ptr->offset);
			printf(
					"ip=%d len=%d offset=%d send_size=%d time=%d file=%s total_size=%d\n",
					ptr->ip, ptr->len, ptr->offset, ptr->send_size, ptr->time,
					ptr->file, ptr->total_size);
			// sleep(4);
			if (fp)
				free(fp);
		} else if (len < 1024) {
			// if(ptr->offset==ptr->total_size)
			end_response(type, buff, from, ptr->total_size);
			ptr->state = 4;
			if (fp)
				free(fp);
			return 1;
		} else {
			PLOG_ERROR("Read error\n");
			printf("%s:Read error\n",__FUNCTION__);
			if (fp)
				free(fp);
		}
	}

}
int end_response(int type, unsigned char *buff, struct sockaddr_in *from,
		unsigned long size) {
	printf("send end response \n");
	struct dms_update udp;
	udp.type = 0x02;
	udp.action = 0x0A;
	udp.version = 0x0A;
	udp.company = 0x4;
	unsigned int sn_return = sn_ret(from->sin_addr.s_addr);
	if (-1 == sn_return) {
		PLOG_ERROR("end return sn error\n");
		// sleep(5);
		return -1;
	}

	udp.sn = htonl(++sn_return);
	sn_set(from->sin_addr.s_addr, sn_return);
	udp.para = 0x0;
	udp.len = htons(4);
	udp.packet.reqmsg.len = size;
	memcpy(udp.packet.reqmsg.msg, "zx", 40);
	char buf[PKT_SIZE];
	memcpy(buf, &udp, sizeof(UDP));
	sendto(fd, buf, sizeof(struct dms_update), 0, (struct sockaddr*) from,
			sizeof(struct sockaddr));

}
int end_res(int type, unsigned char *buff, struct sockaddr_in *from) {
	printf("dispatch end file trans packet \n");
	UPDEV_INFO *ptr = NULL;
	UDP packet;
	char key_string[16] = "\0";
	memcpy(&packet, buff, sizeof(UDP));
	sprintf(key_string, "%s", inet_ntoa(from->sin_addr));
	int err = hashmap_get(map, key_string, (void**) (&ptr));
	if (err == MAP_MISSING)    	   	//not fond device
	{
		PLOG_ERROR("device:%s not find in map", key_string);
		return -1;
	} else {
		//check po
		PLOG_INFO("device %s remote success time is %ld", key_string,
				time(NULL));
		ptr->state = 4;
	}
}

int RegisterCommandHandler(int type) {
	CALLBACK * value = NULL;
	switch (type) {
	case CHECKHEAD:
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = head_check;
		hashmap_put(mymap, "0", value);
		break;
	case 0x01:    	   	//time return function register
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = time_funciton;
		hashmap_put(mymap, "1", value);
		break;
	case 0x02:    	   	//在线状态探测
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = state_funciton;
		hashmap_put(mymap, "2", value);
		break;
	case 0x03:    	   	//远程管理密钥协商
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = negotiate_funciton;
		hashmap_put(mymap, "3", value);
		break;
	case 0x04:    	   	//设备重启
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = restart_funciton;
		hashmap_put(mymap, "4", value);
		break;
	case 0x05:    	   	//获取设备基本信息
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = devinfo_funciton;
		hashmap_put(mymap, "5", value);
		break;
	case 0x06:    	   	//读取系统参数
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = sysinfo_function;
		hashmap_put(mymap, "6", value);
		break;
	case 0x07:    	   	//设置系统参数
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = set_sysinfo_function;
		hashmap_put(mymap, "7", value);
		break;
	case 0x08:    	   	//软件升级
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = trans_data;
		hashmap_put(mymap, "8", value);
		break;
	case 0x09:
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = start_response;
		hashmap_put(mymap, "9", value);
		break;
	case 0x0A:
		////升级end
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = end_res;
		hashmap_put(mymap, "10", value);
		break;
	case 0x0D:
		////升级文件已经存在
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = time_funciton;
		hashmap_put(mymap, "13", value);
		break;
	case 0x0c:
		//无效的升级文件分片
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = time_funciton;
		hashmap_put(mymap, "12", value);
		break;
	case 0x10:    	   	//for test
		value = malloc(sizeof(CALLBACK));
		value->type = type;
		value->func.function = test_funciton;
		hashmap_put(mymap, "16", value);
		break;
	default:
		break;
	}
}
//load file

int load_cfg(const unsigned char *config) {
	FILE *fp = NULL;
	char str[256] = { 0 }, *w;
	fp = fopen(config, "r");
	if (!fp) {
		printf("read error\n");
		return -1;
	}
	DEV * tmp = NULL;
	DEV * tmp1 = NULL;
	while (fgets(str, sizeof str, fp)) {
		tmp = (struct device*) balloc(sizeof(struct device));
		if (!tmp)
			goto parse_error;
		memset(tmp, 0, sizeof(struct device));

		if (!(w = strtok(str, ",")))
			goto parse_error;
		if (!(w = strtok(NULL, ",")))
			goto parse_error;
		if (!(w = strtok(NULL, ",")))
			goto parse_error;
		if (!(w = strtok(NULL, ",")))
			goto parse_error;

		if (!(w = strtok(NULL, ",")))
			goto parse_error;
		printf("ip is %s\n", w);
		tmp->remote_ip = inet_addr(w);

		memset(&tmp->addr, 0, sizeof(struct sockaddr_in));
		tmp->addr.sin_family = AF_INET;
		tmp->addr.sin_addr.s_addr = tmp->remote_ip;
		tmp->addr.sin_port = htons(DMS_PORT);
		tmp->inuse = 1;

		// printf("find device :%d :%d \n",tmp->remote_ip,tmp->sn);
		continue;

		parse_error: if (tmp)
			bfree(tmp);
	}

	if (fp)
		fclose(fp);
	return 0;
}
int init_register() {
	PLOG_INFO("register function \n");
	//move to function.c init(),modify 3/10
	mymap = hashmap_new(50);
	map = hashmap_new(2000);
	RegisterCommandHandler(CHECKHEAD);
	RegisterCommandHandler(0x01);
	RegisterCommandHandler(0x02);
	RegisterCommandHandler(0x03);
	RegisterCommandHandler(0x04);
	RegisterCommandHandler(0x05);
	RegisterCommandHandler(0x06);
	RegisterCommandHandler(0x07);
	RegisterCommandHandler(0x08);
	RegisterCommandHandler(0x09);
	RegisterCommandHandler(0x0A);
	RegisterCommandHandler(0x10);
	RegisterCommandHandler(0x0c);
	RegisterCommandHandler(0x0d);
	//modify 3 13
	thpool_add_work(thpool, (void*) task4, NULL);
	thpool_add_work(thpool, (void*) task5, NULL);
	return 1;

}
