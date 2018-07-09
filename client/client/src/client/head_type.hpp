/*
 * head_type.hpp
 *
 *  Created on: 2018年4月13日
 *      Author: zhaoxi
 */

#ifndef CLIENT_CLIENT_HEAD_TYPE_HPP_
#define CLIENT_CLIENT_HEAD_TYPE_HPP_
#include "../hlib/global.hpp"

typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned int __u32;
typedef unsigned long long __u64;
#define H_BUFFER_LEN 4069
typedef __u8 msg_head_flag;
//
//const msg_head_flag mhf_NONE = 0;
//const msg_head_flag mhf_GZIP = 1;
//const msg_head_flag mhf_ENCRYPT = 2;
//const msg_head_flag mhf_ACK_REQED = 4;
//const msg_head_flag mhf_NOT_END = 8;
//const msg_head_flag mhf_ACK = 16;

typedef __u16 msg_cmd_type;
//const msg_cmd_type msg_cmd_ack2 = 11;
//消息type类型
typedef __u16 msg_cmd_type;
union MsgType {
	msg_cmd_type type;
};
typedef struct msg_head1 {
	__u32 type;
	__u32 len;		// total packet length in bytes
	__u32 sequence;
}*pmsg_head1;
//typedef struct msg_head2 {
//	__u16 logo;
//	__u8 ver;
//	msg_head_flag flag;
//	msg_cmd_type cmd;
//	__u16 len;
//	__u32 crc32c;
//	__u32 seq;
//}*pmsg_head2;

//union head {
//	msg_head2 head2;
//};
typedef struct {
	__u8 wan_mac[6];
	__u8 wan_ip[16];
	__u8 wan_gw[16];
} PacketWlan;
typedef struct
{
	__u8 url_len;
	__u8 mac[6];
	__u8 user_ip[16];
} PacketUser;
union Body {
	PacketWlan wlan;
	PacketUser user;
};
#endif /* CLIENT_CLIENT_HEAD_TYPE_HPP_ */
