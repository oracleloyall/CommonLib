/* crypto/sm4/sm4.h
 *
 */

#ifndef HEADER_SM4_H
#define HEADER_SM4_H

#include <openssl/opensslconf.h>

#ifdef OPENSSL_NO_SM4
#error SM4 is disabled.
#endif

#include <stddef.h>

#define SM4_ENCRYPT    1
#define SM4_DECRYPT    0

/* Because array size can't be a const in C, the following two are macros.
   Both sizes are in bytes. */
#define SM4_MAXNR 14
#define SM4_BLOCK_SIZE       16
#define SM4_WORD_SIZE        sizeof(uint32)
#define SM4_RK_WORDS       32
#define SM4_BLOCK_WORDS    (SMS4_BLOCK_SZ/SMS4_WORD_SZ)

#ifdef  __cplusplus
extern "C" {
#endif

/* This should be a hidden type, but EVP requires that the size be known */
struct sm4_key_st {
#ifdef SM4_LONG
    unsigned long rd_key[4 *(SM4_MAXNR + 1)];
#else
    unsigned int rd_key[4 *(SM4_MAXNR + 1)];
#endif
    int rounds;
};
typedef struct sm4_key_st SM4_KEY;

const char *SM4_options(void);

int SM4_set_encrypt_key(const unsigned char *userKey, const int bits,
    SM4_KEY *key);
int SM4_set_decrypt_key(const unsigned char *userKey, const int bits,
    SM4_KEY *key);

void SM4_encrypt(const unsigned char *in, unsigned char *out,
    const SM4_KEY *key);
void SM4_decrypt(const unsigned char *in, unsigned char *out,
    const SM4_KEY *key);

void SM4_ecb_encrypt(const unsigned char *in, unsigned char *out,
    const SM4_KEY *key, const int enc);
void SM4_cbc_encrypt(const unsigned char *in, unsigned char *out,
    size_t length, const SM4_KEY *key,
    unsigned char *ivec, const int enc);
void SM4_cfb128_encrypt(const unsigned char *in, unsigned char *out,
    size_t length, const SM4_KEY *key,
    unsigned char *ivec, int *num, const int enc);
void SM4_cfb1_encrypt(const unsigned char *in, unsigned char *out,
    size_t length, const SM4_KEY *key,
    unsigned char *ivec, int *num, const int enc);
void SM4_cfb8_encrypt(const unsigned char *in, unsigned char *out,
    size_t length, const SM4_KEY *key,
    unsigned char *ivec, int *num, const int enc);
void SM4_ofb128_encrypt(const unsigned char *in, unsigned char *out,
    size_t length, const SM4_KEY *key,
    unsigned char *ivec, int *num);
void SM4_ctr128_encrypt(const unsigned char *in, unsigned char *out,
    size_t length, const SM4_KEY *key,
    unsigned char ivec[SM4_BLOCK_SIZE],
    unsigned char ecount_buf[SM4_BLOCK_SIZE],
    unsigned int *num);
/* NB: the IV is _two_ blocks long */
void SM4_ige_encrypt(const unsigned char *in, unsigned char *out,
             size_t length, const SM4_KEY *key,
             unsigned char *ivec, const int enc);
/* NB: the IV is _four_ blocks long */
void SM4_bi_ige_encrypt(const unsigned char *in, unsigned char *out,
            size_t length, const SM4_KEY *key,
            const SM4_KEY *key2, const unsigned char *ivec,
            const int enc);


#ifdef  __cplusplus
}
#endif

#endif /* !HEADER_SM4_H */
