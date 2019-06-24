/*
 * sm3.h
 *
 *  Created on: 2011-5-31
 *      Author: zweib
 */

#ifndef SM3_H_
#define SM3_H_

#define SM3_CBLOCK    64
#define SM3_DIGEST_LENGTH 32
#define SM3_LONG_32 unsigned int

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct SM3state_st {
    SM3_LONG_32 V[8];
    SM3_LONG_32 Nl,Nh;
    unsigned int num;
    unsigned char data[SM3_CBLOCK];
} SM3_CTX;

unsigned char *SM3(const unsigned char *d, size_t n, unsigned char *md);
int SM3_Init(SM3_CTX *c);
int SM3_Update(SM3_CTX *c, const void *data_, size_t len);
int SM3_Final(unsigned char *md, SM3_CTX *c);

#ifdef  __cplusplus
}
#endif

#endif /* SM3_H_ */
