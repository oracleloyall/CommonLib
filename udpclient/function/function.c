/*
 * function.c
 *
 *  Created on: Mar 24, 2017
 *      Author: oracle
 */
#include <sys/stat.h>
#include "function.h"
map_t mymap;
map_t timer;
#define CHECKHEAD 0

EVP_PKEY *sigkey = NULL;
void *sigkey_peer = NULL;

struct sigcert_t{
	int len;
	char *data;
}sigcert;

struct ikepara_t{
	int sn;
	char key[16];
	char iv[16];
	char hmac_key[32];
}ikepara;

unsigned char hmac_key[] = {
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
	0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36,
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
	0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36
};

extern udp_peer_t *g_udp_peer;
extern struct sockaddr_in addr;

static unsigned long get_file_size(const char *filename) {
	struct stat buf;
	if (stat(filename, &buf) < 0) {
		return 0;
	}
	return (unsigned long) buf.st_size;
}
void print_hex(unsigned char *buf, int len) {
	int i;
	if (len == 0)
		return;

	for (i = 0; i < len; i++) {
		if (i % 0x10 == 0) {
			printf("\n%08Xh: ", i);
		}
		printf("%02X ", buf[i]);
	}
	printf("\n");
}
int head_check(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
    CALLBACK * value = NULL;
    if (!message)
		return 0;
	UDP*pkt = (UDP *) message;
	//need check hmac
	//(pkt->action)
	char buf[10] = { 0 };
	sprintf(buf, "%d", pkt->action);
	if (hashmap_get(mymap, buf, (void**) (&value)) == MAP_MISSING) {
		log_debug("Dispatch action error ");
		return -1;
	} else {
#ifdef DEBUG
		log_debug("%s packet type:%d\n", __FUNCTION__,pkt->action);
#endif
		value->func.function(udp_peer,message, size, userdata, peer_addr);
		return 0;
	}
}
int time_funciton(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
#ifdef TRANS
	log_debug("alarm 0\n");
	 alarm(0);
#endif
#ifdef DEBUG
	log_debug("time function enter\n");
#endif
	UDP *udp=(UDP *)message;
#ifdef DEBUG
	log_debug("recv time:%d  now time:%d \n", ntohl(udp->packet.timepac.time), time(NULL));
#endif
}
int state_funciton(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
#ifdef DEBUG
	log_debug("state function enter\n");
#endif
	UDP udp;
	memset(&udp, 0, sizeof(UDP));
	udp.type = 0x01;
	udp.version = 0x01;
	udp.company = 0x04;
	memcpy(udp.hmac, "zx", 2);
	udp.sn = htonl(0);
	udp.action = 0x02;
	udp.para = 0;		//success
	udp.len = 0;
	char buf[1048] = { 0 };
	memcpy(buf, &udp, sizeof(UDP));
	udp_peer_send2(g_udp_peer, buf, sizeof(udp), &addr);
}
int negotiate_funciton(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
	log_debug("negotiate function enter\n");
#if 0
	int len = 0, inlen = 0, outlen = 0, pos = 0;
	int ret = 0;
	unsigned int signlen = 0;
	char buf[PKT_SIZE] = {0}, hmac[64], sign[64];
	EC_KEY *tmpkey;
	UDP *udp = (UDP *)message;
	UDP *pkt = (UDP *)buf;
	char *data = NULL;

	data = (char *)(&udp->packet.sndike);
	memcpy(&len, data, 4);
	pos += 4;
	tmpkey = ReadPublicKey(data + pos, ntohl(len));
	if(!tmpkey)
		return 0;
	pos += ntohl(len);

	ret = SM2DSA_verify(0, data + 4, ntohl(len), NULL, 0, data + pos, 64, tmpkey);
	if (ret != 1)
		return 0;
	pos += 64;

	if(sigkey_peer){
		EC_KEY_free(sigkey_peer);
		sigkey_peer = NULL;
	}
	sigkey_peer = tmpkey;

	data = (char *)(&pkt->packet.rcvike);
	pkt->type = 0x03;
	pkt->version = 0x01;
	pkt->company = 0x04;
	pkt->sn = htonl(++ikepara.sn);
	pkt->action = 0x03;
	pkt->para = 0;
	pos = 0;
	
	RAND_bytes(ikepara.key, 64);
	SM2_public_encrypt(ikepara.key, 64, data, &outlen, (EC_KEY *)sigkey_peer);
	if(outlen != data[1] + 2)
		return 0;
	pos += outlen;
	
	len = htonl(sigcert.len);
	memcpy(data + pos, &len, 4);
	pos += 4;
	memcpy(data + pos, sigcert.data, sigcert.len);
	pos += sigcert.len;

	ret = SM2DSA_sign(0, (unsigned char *)sigcert.data, sigcert.len, NULL, 0, (unsigned char*)sign, &signlen, sigkey->pkey.ec);
	if(ret != 1)
		return 0;
	memcpy(data + pos, sign, signlen);
	pos += signlen;
	
	pkt->len = outlen + 4 + sigcert.len + 64;
	sm3_hmac(hmac_key, 32, buf + SEC_HEAD_LEN - 4, 4 + BODY_HEAD_LEN + pos, hmac);
	memcpy(pkt->hmac, hmac, 2);
	udp_peer_send2(g_udp_peer, buf, sizeof(udp), &addr);
#endif	
}
int restart_funciton(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
	log_debug("restart  function enter\n");
	UDP udp;
	memset(&udp, 0, sizeof(UDP));
	udp.type = 0x02;
	udp.version = 0x01;
	udp.company = 0x04;
	memcpy(udp.hmac, "zx", 2);
	udp.sn = htonl(0);
	udp.action = 0x04;
	udp.para = 0;		//success
	udp.len = 0;
	char buf[1048] = { 0 };
	memcpy(buf, &udp, sizeof(UDP));
	udp_peer_send2(g_udp_peer, buf, sizeof(udp), &addr);
}
int devinfo_funciton(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
	log_debug("dev info function enter\n");
	UDP udp;
	memset(&udp, 0, sizeof(UDP));
	DEV_INFO info;
	memset(&info, 0, sizeof(DEV_INFO));
	memcpy(&info.dev_name, "device name", 11);
	int mm;
	char *p = &mm;
	p[0] = 0x2;
	p[1] = 0x3;
	p[2] = 0x4;
	p[3] = 0x5;
	//0000 0101 0000 0100 0000 0011 0000 0010
	info.dev_factory = htonl(0x3);
	info.dev_runtime = htonl(time(NULL));
	info.dev_type = htonl(0x3);
	info.dev_version = mm;
	memset(&udp, 0, sizeof(UDP));
	udp.type = 0x02;
	udp.version = 0x01;
	udp.company = 0x04;
	memcpy(udp.hmac, "zx", 2);
	udp.sn = htonl(0);
	udp.action = 0x05;
	udp.para = 0;		//success
	udp.len = 0;
	memcpy(&udp.packet.devinfo, &info, sizeof(DEV_INFO));
	char buf[1048] = { 0 };
	memcpy(buf, &udp, sizeof(UDP));
	udp_peer_send2(g_udp_peer, buf, sizeof(udp), &addr);
}
int sysinfo_function(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
	log_debug("sysinfo  function enter\n");
	UDP udp;
	memset(&udp, 0, sizeof(UDP));
	DEVSYS sys;
	sprintf(sys.devini,
			"[POLICY]\nisenc=1\ndtuip=10.1.1.1\nmainip=10.1.2.100\nserverip=10.1.2.1, 10.1.2.2\n[NET]\ndevip= 10.1.1.100\nnetmask=255.255.255.0\ndefaultgw=10.1.1.254");
	memset(&udp, 0, sizeof(UDP));
	udp.type = 0x02;
	udp.version = 0x01;
	udp.company = 0x04;
	memcpy(udp.hmac, "zx", 2);
	udp.sn = htonl(0);
	udp.action = 0x06;
	udp.para = 0;		//success
	udp.len = 0;
	char buf[1048] = { 0 };
	memcpy(buf, &udp, sizeof(UDP));
	udp_peer_send2(g_udp_peer, buf, sizeof(udp), &addr);
}
int set_sysinfo_function(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
	log_debug("set sysinfo  function enter\n");
	DEVSYS sy;
	memcpy(&sy, message + 12, sizeof(DEVSYS));
	log_debug("recv :%s\n", sy.devini);
}
//update
int start_response(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
	log_debug("start response  function enter\n");
	REQMSG msg;
	UDP packet;
	UDP udp;
	memset(&udp, sizeof(UDP), 0);
	memset(&packet, sizeof(UDP), 0);
	memcpy(&packet, message, sizeof(packet));
	memcpy(&msg, message + 12, sizeof(REQMSG));
	log_debug("********************sn=%d\n", ntohl(packet.sn));
	log_debug("size %lu     %s \n", packet.packet.reqmsg.len,
			packet.packet.reqmsg.msg);
	log_debug("size %lu     %s \n", msg.len, msg.msg);
	//send ok
	memset(&udp, 0, sizeof(UDP));
	udp.action = 0x09;
	udp.version = 0x1;
	udp.company = 0x4;
	udp.sn = htonl(ntohl(packet.sn));
	udp.para = 0x0;
	udp.len = htons(4);

	unsigned long m = get_file_size("a.tar.gz");
	udp.packet.rsqmsg.recv_len = m;
	udp.packet.rsqmsg.max_len = 1000;
	char buf[1048] = { 0 };
	memcpy(buf, &udp, sizeof(UDP));
	udp_peer_send2(g_udp_peer, buf, sizeof(udp), &addr);

}
long total=0;
int trans_data(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
	log_debug("trans data  function enter\n");
	FILE *fp = fopen("a.tar.gz", "a+");
    if (NULL == fp)
    {
    	printf("File:\t Can Not Open To Write\n");
    	return -1;
    }
    UDP udp;
    memset(&udp,0,sizeof(UDP));
	UDP packet;
	memset(&packet, 0, sizeof(UDP));
	memcpy(&packet, message, sizeof(UDP));
	log_debug("recv size %d  :%s\n", packet.packet.reqmsg.len,
			packet.packet.senpac.packet);
	int len = 0;
	int writesize = packet.packet.reqmsg.len;
	log_debug("********************sn=%d size=%d\n", ntohl(packet.sn),
			packet.packet.reqmsg.len);
	if ((len = fwrite(packet.packet.senpac.packet, sizeof(char), writesize, fp))
			< writesize) {
		log_debug("File:\t%s Write Failed\n");
	}
	total += len;
	log_debug("recv  size is %d\n", total);
	memset(&udp, 0, sizeof(UDP));
	udp.action = 0x08;
	udp.version = 0x1;
	udp.company = 0x4;
	udp.sn = htonl(ntohl(packet.sn));
	udp.para = 0x0;
	udp.len = htons(4);
	unsigned long m = get_file_size("a.tar.gz");
	udp.packet.rsqmsg.recv_len = m;//offset
	udp.packet.rsqmsg.max_len = 1024;
	char buf[1048] = { 0 };
	memcpy(buf, &udp, sizeof(UDP));
	int err=udp_peer_send2(g_udp_peer, buf, sizeof(udp), &addr);
	//log_debug("state err%d\n",err);
	fclose(fp);
}
int end_response(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
	log_debug("end response  function enter\n");
}
int end_res(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
	log_debug("end res  function enter\n");
	REQMSG msg;
	UDP packet, udp;
	memcpy(&packet, message, sizeof(packet));
	memcpy(&msg,message+12,sizeof(REQMSG));
	log_debug("sn :%d\n", packet.sn);
	log_debug("size %lu     %s \n", packet.packet.reqmsg.len,
			packet.packet.reqmsg.msg);
	log_debug("size %lu     %s \n", msg.len, msg.msg);
	//send ok
	memset(&udp, 0, sizeof(UDP));
	udp.action = 0x0A;
	udp.version = 0x1;
	udp.company = 0x4;
	udp.sn = htonl(ntohl(packet.sn));
	udp.para = 0x0;
	udp.len = htons(4);
	char buf[1048] = { 0 };
	memcpy(buf, &udp, sizeof(UDP));
	udp_peer_send2(g_udp_peer, buf, sizeof(udp), &addr);
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
//		value = malloc(sizeof(CALLBACK));
//		value->type = type;
//		value->func.function = test_funciton;
//		hashmap_put(mymap, "16", value);
		break;
	default:
		break;
	}
}
int init_register() {
	//modify 3/34
	mymap = hashmap_new(50);
#ifdef TRANS
	timer=hashmap_new(100);
#endif
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
}
int init_key() {
	if(sigkey){
		EVP_PKEY_free(sigkey);
		sigkey = NULL;
	}
	sigkey = ReadPrivateKey("key_sig.pem");
	read_sig_cert("cert_sig.pem", &sigcert);

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

