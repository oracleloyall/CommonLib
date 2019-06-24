
#ifndef _PACKET_H
#define _PACKET_H

#include <asm/byteorder.h>

#define DMS_PORT		5000
#define PKT_SIZE		1280
#define RAND_LEN		1024
/*
unsigned char hmac_key[] = {
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
	0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36,
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
	0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36
};
*/
#define SEC_HEAD_LEN	8
#define BODY_HEAD_LEN	4

struct dms_pkt
{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8 	type:4,
		version:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8 	version:4,
		type:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__u8 company;
	__u8 hmac[2];
	__u32 sn;
	__u8 action;
	__u8 para;
	__u16 len;
	__u8 data[512];
}__attribute__((packed));

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#define HIPQUAD(addr) \
	((unsigned char *)&addr)[3], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[0]

#endif 

