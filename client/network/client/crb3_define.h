#ifndef _IN_crb3_defineh_
#define _IN_crb3_defineh_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"
#include "lstLib.h"
#include "soLib.h"

#ifdef NAMESPACE
namespace common{
#endif

#ifndef CRB3_MAX_SERVICE_THREADS
#define CRB3_MAX_SERVICE_THREADS	512
#endif

#ifndef CRB3_MAX_PACKET_SIZE
#define CRB3_MAX_PACKET_SIZE		4000
#endif

#ifndef CRB3_MAX_QUEUE_SIZE
#define CRB3_MAX_QUEUE_SIZE			5000
#endif

#define CRB3_NO_RSP					1
#define CRB3_OK						0
#define CRB3_FAILED					-1
#define CRB3_TIMEOUT				-2
#define CRB3_EMPTY_MEMORY			-3
#define CRB3_EMPTY_TABLE			-4
#define CRB3_EMPTY_QUEUE			-5
#define CRB3_INVALID_PARAMETER		-6
#define CRB3_INVALID_MODE			-7
#define CRB3_REQUEST_FAILED			-8
#define CRB3_NOT_CONNECTED			-9
#define CRB3_NOT_FIND				-10
#define CRB3_DUPLICATE				-11
#define CRB3_FULL					-12
#define CRB3_PEND					-13
#define CRB3_NOT_SERVICE			-14
#define CRB3_NOT_SUPPORT			-99

// fetch 8bit-integer
#define CRB3_FETCH_INT8(ptr, rc)			do{rc = *ptr;	ptr++;}while(0)

// fetch 16bit-integer without alignment
#define CRB3_FETCH_INT16(ptr, rc)			do{memcpy(&rc, ptr, 2);	rc = ntohs(rc);		ptr+=2;	}while(0)

// fetch 16bit-integer with alignment
#define CRB3_FETCH_INT16_A(ptr, rc)			do{rc = ntohs(*(int16_t *)(ptr));			ptr+=2;	}while(0)

// fetch 32bit-integer without alignment
#define CRB3_FETCH_INT32(ptr, rc)			do{memcpy(&rc, ptr, 4);	rc = ntohl(rc);		ptr+=4;	}while(0)

// fetch 32bit-integer with alignment
#define CRB3_FETCH_INT32_A(ptr, rc)			do{rc = ntohl(*(int32_t *)(ptr));			ptr+=4;	}while(0)

// fetch 64bit-integer without alignment
#define CRB3_FETCH_INT64(ptr, rc)			do{memcpy(&rc, ptr, 8);	rc = ntohl_64(rc);	ptr+=8;	}while(0)

// fetch 64bit-integer with alignment
#define CRB3_FETCH_INT64_A(ptr, rc)			do{rc = ntohl_64(*(int64_t *)(ptr));		ptr+=8;	}while(0)

// fetch octets without alignment
#define CRB3_FETCH_BYTE(ptr, dst, n)		do{memcpy(dst, ptr, (n));	ptr+=(n);				}while(0)

// fetch octets with 2 bytes alignment
#define CRB3_FETCH_BYTE_A2(ptr, dst, n)		do{memcpy(dst, ptr, (n));	ptr+=((n+1)&~1);		}while(0)

// fetch octets with 4 bytes alignment
#define CRB3_FETCH_BYTE_A4(ptr, dst, n)		do{memcpy(dst, ptr, (n));	ptr+=((n+3)&~3);		}while(0)

// fetch octets with 8 bytes alignment
#define CRB3_FETCH_BYTE_A8(ptr, dst, n)		do{memcpy(dst, ptr, (n));	ptr+=((n+7)&~7);		}while(0)

// submit 8bit-integer
#define CRB3_SUBMIT_INT8(ptr, rc)			do{*ptr = rc;	ptr++;}while(0)

// submit 16bit-integer without alignment
#define CRB3_SUBMIT_INT16(ptr, val)			do{int16_t s = htons(val);		memcpy(ptr, &s, 2);	ptr+=2;	}while(0)

// submit 16bit-integer with alignment
#define CRB3_SUBMIT_INT16_A(ptr, val)		do{*(int16_t *)ptr = htons(val);	ptr+=2;					}while(0)

// submit 32bit-integer without alignment
#define CRB3_SUBMIT_INT32(ptr, val)			do{int32_t i = htonl(val);		memcpy(ptr, &i, 4);	ptr+=4;	}while(0)

// submit 32bit-integer with alignment
#define CRB3_SUBMIT_INT32_A(ptr, val)		do{*(int32_t *)ptr = htonl(val);	ptr+=4;					}while(0)

// submit 64bit-integer without alignment
#define CRB3_SUBMIT_INT64(ptr, val)			do{int64_t q = htonl_64(val);	memcpy(ptr, &q, 8);	ptr+=8;	}while(0)

// submit 64bit-integer with alignment
#define CRB3_SUBMIT_INT64_A(ptr, val)		do{*(int64_t *)ptr = htonl_64(val);	ptr+=8;					}while(0)

// submit octets without alignment
#define CRB3_SUBMIT_BYTE(ptr, src, n)		do{memcpy(ptr, src, (n));	ptr+=(n);						}while(0)

// submit octets with 2 bytes alignment
#define CRB3_SUBMIT_BYTE_A2(ptr, src, n)	do{memcpy(ptr, src, (n));	ptr+=((n+1)&~1);				}while(0)

// submit octets with 4 bytes alignment
#define CRB3_SUBMIT_BYTE_A4(ptr, src, n)	do{memcpy(ptr, src, (n));	ptr+=((n+3)&~3);				}while(0)

// submit octets with 8 bytes alignment
#define CRB3_SUBMIT_BYTE_A8(ptr, src, n)	do{memcpy(ptr, src, (n));	ptr+=((n+7)&~7);				}while(0)

#define CRB3_PKT_STATUS_REQ			0x0001
#define CRB3_PKT_STATUS_RSP			0x8001
#define CRB3_PKT_CR_REQ				0x0002
#define CRB3_PKT_CR_RSP				0x8002
#define CRB3_PKT_ALIVE_REQ			0x0003
#define CRB3_PKT_ALIVE_RSP			0x8003

#define CRB3_STATUS_NULL			-1
#define CRB3_STATUS_WORKING			0
#define CRB3_STATUS_BACKUP			1
#define CRB3_STATUS_EXCEPTION		2

#define CRB3_FLAG_GO_WORKING		0x01
#define CRB3_FLAG_GO_BACKUP			0x02
#define CRB3_FLAG_GO_EXCEPTION		0x04

typedef SO_ADDRESS CRB3_ADDRESS;

typedef struct
{
	NODE node;
	CRB3_ADDRESS from_addr;
	int protocol;			// SOCK_DGRAM or SOCK_STREAM
	int packet_size;
	char packet_buf[CRB3_MAX_PACKET_SIZE];
}CRB3_PACKET_NODE;

#define CRB3_SOCKLEN(addr)		((addr)->sa.sa_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6))

#ifdef NAMESPACE
}
#endif
#endif

