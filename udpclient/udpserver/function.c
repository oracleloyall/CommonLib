/*
 Date 2017 2 28
 */
#include"define.h"
#include"jsw_rbtree.h"
#include"packet.h"
#include"Callback.h"
#include"cert/crypt.h"
/*
 * Function: Malloc buff
 * Author:
 */
extern int fd;
char map_buf[1024 * 1024 * 20];  // balloc cache size
typedef struct jsw_rbtree centroidset_t;
struct jsw_rbtree *centroidset = NULL;
EVP_PKEY *sigkey = NULL;
struct sigcert_t{
	int len;
	char *data;
};
struct sigcert_t sigcert;
#define DEVICENUM 10000

int Malloc(void) {
	int rc;
	rc = bopen(map_buf, sizeof(map_buf), 0);
	if (rc < 0) {
		PLOG_ERROR("balloc failed\n");
		return -1;
	}
	return 0;

}

int device_insert(DEV *pt) {
	int ret;
	ret = jsw_rbinsert(centroidset, (void *) pt);
	if (ret == 0) {
		PLOG_ERROR("failed to insert the element\n");
		return -1;
	}

	return 0;
}
int centroidset_insert(centroidset_t *centroidset, DEV *pt) {
	int ret;

//	 MODEM *centroid;
//	 centroid = calloc(1, sizeof( MODEM));
	ret = jsw_rbinsert(centroidset, (void *) pt);
	if (ret == 0) {
		PLOG_ERROR("failed to insert the element\n");
		//free(centroid);
		return -1;
	}

	return 0;
}

//print
void centroidset_printset(centroidset_t *centroidset) {
	DEV *centroid;

	jsw_rbtrav_t *rbtrav;
	rbtrav = jsw_rbtnew();

	centroid = jsw_rbtfirst(rbtrav, centroidset);

	while ((centroid = jsw_rbtnext(rbtrav)) != NULL) {
		printf("Ip %d\n", centroid->remote_ip);
	}
}
//compare
static int centroid_cmp(const void *p1, const void *p2) {
	DEV *centroid1, *centroid2;

	centroid1 = (DEV*) p1;
	centroid2 = (DEV*) p2;

	if (centroid1->remote_ip > centroid2->remote_ip)
		return 1;

	else if (centroid1->remote_ip < centroid2->remote_ip)
		return -1;

	return 0;
}
//dup
static void *centroid_dup(void *p) {
	void *dup_p;
	dup_p = calloc(1, sizeof(DEV));
	memmove(dup_p, p, sizeof(DEV));
	return dup_p;
}
//free
static void centroid_rel(void *p) {
	bfree(p);
}

centroidset_t *centroidset_new() {
	jsw_rbtree_t *rbtree;
	rbtree = jsw_rbnew(centroid_cmp, centroid_dup, centroid_rel);

	return rbtree;
}
void update_key() {
	DEV *centroid;
	jsw_rbtrav_t *rbtrav;
	rbtrav = jsw_rbtnew();
	centroid = jsw_rbtfirst(rbtrav, centroidset);
	if (centroid == NULL)
		return;
	do {
		// 密钥协商请求
		int rand_len = 0;
		int pos = 0, ret = 0;
		int len = 0;
		unsigned int signlen = 0;
		char buf[PKT_SIZE] = {0}, hmac[64], hash[32], sign[64];
		UDP *pkt = (UDP *)buf;
		char *data = (char *)(&pkt->packet.sndike);

		pkt->type = 0x3;
		pkt->version = 0x1;
		pkt->company = 0x4;
		pkt->sn = htonl(++centroid->sn);
		pkt->action = 0x3;
		pkt->para = 0x0;
		pkt->len = 4 + sigcert.len + 64;
		
		len = htonl(sigcert.len);
		memcpy(data, (char *)(&len), sigcert.len);
//printf("len:%08X %d, %02X %02X %02X %02X\n", len, sigcert.len, data[0], data[1], data[2], data[3]);
		pos += 4;
		memcpy(data + pos, sigcert.data, sigcert.len);
		pos += sigcert.len;
		
		ret = SM2DSA_sign(0, (unsigned char *)sigcert.data, sigcert.len, NULL, 0, (unsigned char*)sign, &signlen, sigkey->pkey.ec);
		if(ret != 1)
			break;
		memcpy(data + pos, sign, signlen);
		pos += signlen;
		sm3_hmac(hmac_key, 32, buf + SEC_HEAD_LEN - 4, 4 + BODY_HEAD_LEN + pos, hmac);
		memcpy(pkt->hmac, hmac, 2);
		sendto(fd, buf, SEC_HEAD_LEN + BODY_HEAD_LEN + pos,
			0, (struct sockaddr*)&centroid->addr, sizeof(struct sockaddr));
		bfree(pkt);
		
		/*主动发起协商包 F=0x03，T=0x03，P0=0，D=密钥交换数据
		 int rand_len = 0;
		 char buf[PKT_SIZE], hmac[64];
		 struct dms_pkt *pkt = (struct dms_pkt *)buf;
		 //RAND_bytes(randbuf, sizeof randbuf);
		 //rand_len = (dlen == 0) ? (*(unsigned short *)randbuf) % 1024 : dlen;
		 pkt->type = 0x3;
		 pkt->version = 0x1;
		 pkt->company = 0x4;
		 pkt->sn = htonl(0);//++
		 pkt->action = 0x3;
		 pkt->para = 0x0;
		 pkt->len = htons(rand_len);
		 pkt->data=(unsigned char *)malloc(sizeof(100));
		 memcpy(pkt->data, "data", strlen("data"));
		 //sm3_hmac(hmac_key, 32, buf + SEC_HEAD_LEN - 4, 4 + BODY_HEAD_LEN + rand_len, hmac);
		 memcpy(pkt->hmac, "zx", 2);
		 printf("device print\n");
		 sendto(fd, buf,12+4, 0, (struct sockaddr*)&centroid->addr, sizeof(struct sockaddr));
		 free(pkt->data);
		 */
	} while ((centroid = jsw_rbtnext(rbtrav)) != NULL);
	jsw_rbtdelete(rbtrav);
}
void device_print() {
	DEV *centroid;

	jsw_rbtrav_t *rbtrav;
	rbtrav = jsw_rbtnew();

	centroid = jsw_rbtfirst(rbtrav, centroidset);
	if (centroid == NULL)
		return;
	do {
//printf("addr:%d.%d.%d.%d\n",NIPQUAD(centroid->addr.sin_addr.s_addr));
	//	PLOG_DEBUG("device heart  packet \n");
#ifdef T
		UDP pkt;
		char buf[PKT_SIZE];
		memcpy(&pkt,0,sizeof(UDP));
		pkt.type=0x01;
		pkt.version=0x01;
		pkt.company=0x04;
		// pkt.hmac;
		memcpy(pkt.hmac, "zx", 2);
		pkt.sn=htonl(0);
		pkt.action=0x02;
		pkt.para=0;
		pkt.len=htons(20);
		strncpy(pkt.packet.randpac.rand,"rand data data",strlen("rand data data"));
		memcpy(buf,&pkt,sizeof(UDP));
		sendto(fd, buf,sizeof(UDP), 0, (struct sockaddr*)&centroid->addr, sizeof(struct sockaddr));
		//sendto(fd, buf,sizeof(UDP), 0, (struct sockaddr*)from, sizeof(struct sockaddr));
#endif
#if 1
		int rand_len = 0;
		char buf[PKT_SIZE], hmac[64];
		UDP *pkt = (UDP*) balloc(sizeof(UDP));
		//  pkt->data=(unsigned char *)balloc(sizeof(100));
		pkt->type = 0x1;
		pkt->version = 0x1;
		pkt->company = 0x4;
		pkt->sn = htonl(0);
		pkt->action = 0x2;
		pkt->para = 0x0;
		rand_len = strlen("rand data");
		pkt->len = htons(rand_len);
		//pkt->data=(unsigned char *)malloc(sizeof(100));
		memcpy(pkt->packet.randpac.rand, "rand data", rand_len);
		//*(unsigned char *)pkt->data=bu;
		//sm3_hmac(hmac_key, 32, buf + SEC_HEAD_LEN - 4, 4 + BODY_HEAD_LEN + rand_len, hmac);
		memcpy(pkt->hmac, "zx", 2);
		memcpy(buf, pkt, sizeof(UDP));
		sendto(fd, buf, sizeof(UDP), 0, (struct sockaddr*) &centroid->addr,
				sizeof(struct sockaddr));
//		print_hex(buf, PKT_SIZE);
		bfree(pkt);
#endif
#if 0
		//rand data
		int rand_len = 0;
		char buf[PKT_SIZE], hmac[64];
		struct dms_pkt *pkt = (struct dms_pkt *)buf;
		//RAND_bytes(randbuf, sizeof randbuf);
		//rand_len = (dlen == 0) ? (*(unsigned short *)randbuf) % 1024 : dlen;
		pkt->type = 0x1;
		pkt->version = 0x1;
		pkt->company = 0x4;
		pkt->sn = htonl(0);
		pkt->action = 0x2;
		pkt->para = 0x0;
		pkt->len = htons(rand_len);
		//pkt->data=(unsigned char *)malloc(sizeof(100));
		memcpy(pkt->data, "data", strlen("data"));
		//*(unsigned char *)pkt->data=bu;
		//sm3_hmac(hmac_key, 32, buf + SEC_HEAD_LEN - 4, 4 + BODY_HEAD_LEN + rand_len, hmac);
		memcpy(pkt->hmac, "zx", 2);
		printf("device print\n");
		sendto(fd, buf,12+4, 0, (struct sockaddr*)&centroid->addr, sizeof(struct sockaddr));
		free(pkt->data);
#endif
	} while ((centroid = jsw_rbtnext(rbtrav)) != NULL);
	jsw_rbtdelete(rbtrav);
}
/*
 * Function:fing device
 * Author:
 */
DEV *find_device(int ip) {
	DEV *element, l_find;
	l_find.remote_ip = ip;
	element = jsw_rbfind(centroidset, &l_find);
	if (!element)
#ifndef NULL
#define NULL ((void *)0)
#endif
		return NULL;
	else
		return element;

}
unsigned int sn_ret(int ip) {
	DEV *element = find_device(ip);
	if (element && element->sn != 0xFFFFFFFF) {
		return (element->sn);
	} else if (element && element->sn == 0xFFFFFFFF) {
		return 0;
	} else
		return -1;
}
void sn_set(int ip, unsigned int num) {
	DEV *element = find_device(ip);
	if (element) {
		element->sn = num;
	}
}
/*
 * Function:for init tree data
 * Author:
 */
//extern map_t mymap;
int init() {
	if (Malloc())
		return -1;
	centroidset = centroidset_new();
	//  int i=0;
	// mymap = hashmap_new();
	read_cfg("t.txt");
	if(sigkey){
		EVP_PKEY_free(sigkey);
		sigkey = NULL;
	}
	sigkey = ReadPrivateKey("key_sig.pem");
	read_sig_cert("cert_sig.pem", &sigcert);
#ifndef NULL
#define NULL ((void *)0)
#endif
#ifdef TST
	DEV * device=NULL;
	device=(struct device*)balloc(sizeof( struct device));
	// device->sys=(DEV_SYS*)balloc(sizeof(DEV_SYS));
	device->remote_ip=122;
	device->sn=100;
	centroidset_insert(centroidset,device);
	DEV *ptr=NULL;
	ptr=find_device(122);
	if(ptr==NULL)
	printf("Not find device \n");
	else
	printf("find device :%d :%d \n",ptr->remote_ip,ptr->sn);
#endif
}
/*
 * 对重启返回状态记录定义相关的宏
 */
static int restart_packet(int fd, struct sockaddr *to) {
	char buf[PKT_SIZE];
	struct dms_pkt *pkt = (struct dms_pkt *) buf;
	pkt->type = 0x02;   //报文类型
	pkt->version = 0x1;   //
	pkt->company = 0x4;   //
	//从map中查找当前设备的sn,对sn自加1
	//pkt.sn = htonl(0);//
	pkt->action = pkt->type;	//
	pkt->para = 0;	//
	pkt->len = htons(0);	//
	//校验H,s3
	//send 包体
	//查找目的套接字的地址
	//sendto(fd, buf, 12 + pkt->len , (struct sockaddr *)&->addr, to, sizeof(struct sockaddr_in));
	return 0;
}
/*
 *   Function：
 *   desc： 设备基本信息(存放在map中的一个结构体)
 */
int device_info(int fd, struct sockaddr *to) {

}

int system_param(int fd, struct sockaddr *to) {
}
/*
 * Function :
 */
int send_packet(int fd, struct sockaddr *to, int type) {
	switch (type) {
	case 0x04:	// 设备重启命令
		restart_packet(fd, to);
		break;
	case 0x05:	//获取设备基本信息
		device_info(fd, to);
		break;
	case 0x06:	//读取系统参数
		system_param(fd, to);
		break;
	case 0x07:	//设置系统参数
		break;
	case 0x08:	//软件升级
		break;
	case 0x09:
		break;
	default:
		break;

	}
}
int read_cfg(const unsigned char *config) {
	FILE *fp = NULL;
	char str[256] = { 0 }, *w;
	fp = fopen(config, "r");
	if (!fp) {
		PLOG_ERROR("open cfg error\n");
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
		tmp->remote_ip = inet_addr(w);
		memset(&tmp->addr, 0, sizeof(struct sockaddr_in));
		tmp->addr.sin_family = AF_INET;
		tmp->addr.sin_addr.s_addr = tmp->remote_ip;
		tmp->addr.sin_port = htons(DMS_PORT);
		tmp->inuse = 1;
		tmp->sn = 0;
		tmp->old_time = 0;
		tmp->online = 0;
		tmp->mark = 0;
		centroidset_insert(centroidset, tmp);
#if 0
		tmp1=find_device(tmp->remote_ip);
		if(tmp1==NULL)
		printf("Not find device \n");
		else
		printf("find device :%d :%d \n",tmp->remote_ip,tmp->sn);
#endif
		continue;

		parse_error: if (tmp)
			bfree(tmp);
	}
	if (fp)
		fclose(fp);
	return 0;
}

int read_sig_cert(char *certfile, struct sigcert_t *cert)
{
	X509 *x509 = NULL, *x509_t = NULL;
	BIO *b = NULL;

	b = BIO_new_file(certfile,"r");
	x509 = PEM_read_bio_X509(b,NULL,NULL,NULL);
	BIO_free(b);
	
	b = BIO_new_file(certfile,"r");
	x509_t = PEM_read_bio_X509(b,NULL,NULL,NULL);
	BIO_free(b);
	
	cert->len = i2d_X509(x509_t, NULL);
	cert->data = OPENSSL_malloc(cert->len);
	i2d_X509(x509, &cert->data);
//printf("cer:\n");
//print_hex(cert->data, cert->len);

	X509_free(x509);
	X509_free(x509_t);
	return cert->len;
}

int check_param(int param, int ip, int type) {
	switch (param) {
	case P0:
		return 0;
		break;
	case P1:
		PLOG_ERROR("ip:%d type:%d 序列号错 ", ip, type);
		return -1;
		break;
	case P2:
		PLOG_ERROR("ip:%d type:%d 通信协议版本无效 ", ip, type);
		return -1;
		break;
	case P3:
		PLOG_ERROR("ip:%d type:%d 报文类型无效 ", ip, type);
		return -1;
		break;
	case P4:
		PLOG_ERROR("ip:%d type:%d 报文格式无效", ip, type);
		return -1;
		break;
	case P5:
		PLOG_ERROR("ip:%d type:%d 报文完整性校验失败 ", ip, type);
		return -1;
		break;
	case P6:
		PLOG_ERROR("ip:%d type:%d 不支持的管理命令 ", ip, type);
		return -1;
		break;
	case P7:
		PLOG_ERROR("ip:%d type:%d 无远程管理密钥 ", ip, type);
		return -1;
		break;
	case P8:
		PLOG_ERROR("ip:%d type:%d 签名证书无效 ", ip, type);
		return -1;
		break;
	case P9:
		PLOG_ERROR("ip:%d type:%d 验证签名失败 ", ip, type);
		return -1;
		break;
	case P10:
		PLOG_ERROR("ip:%d type:%d 处理失败 ", ip, type);
		return -1;
		break;
	case P11:
		PLOG_ERROR("ip:%d type:%d 升级包文件无效 ", ip, type);
		return -1;
		break;
	}
}
