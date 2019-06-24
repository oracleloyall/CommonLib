/*
 * sm2dh.h
 *
 *  Created on: 2012-8-14
 *      Author: zweib
 */

#ifndef SM2DH_H_
#define SM2DH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <openssl/ossl_typ.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/objects.h>
#include <openssl/err.h>
#include <openssl/sm3.h>

#define SM2_F_DH_STEP_1                                         101
#define SM2_F_DH_STEP_2                                         102
#define SM2_F_DH_STEP_3                                         103
#define SM2_F_DH_COMP_KEY                                       104
#define SM2_F_DH_MISC                                           105
#define SM2DH_F_SM2DH_DATA_NEW_METHOD                           106

typedef struct sm2dh_ctx_st {
    ENGINE *engine;
    CRYPTO_EX_DATA ex_data;
} SM2DH_CTX;

struct sm2dh_method {
    const char *name;
    int (*sm2dh_step1)(
            SM2DH_CTX *ctx,
            EC_POINT *pubkey_tmp_A,
            BIGNUM *prikey_tmp_A,
            EC_KEY *key_A);
    int (*sm2dh_step2)(
            const EC_POINT *pubkey_tmp_A,
            EC_POINT *pubkey_tmp_B,
            const EC_POINT *pubkey_A,
            EC_KEY *key_B,
            unsigned char *ID_A,
            int ID_A_len,
            unsigned char *ID_B,
            int ID_B_len,
            size_t klen,
            unsigned char *key);
    int (*sm2dh_step3)(
            SM2DH_CTX *ctx,
            const EC_POINT *pubkey_tmp_B,
            const EC_POINT *pubkey_B,
            EC_POINT *pubkey_tmp_A,
            BIGNUM *prikey_tmp_A,
            EC_KEY *key_A,
            unsigned char *ID_A,
            int ID_A_len,
            unsigned char *ID_B,
            int ID_B_len,
            size_t klen,
            unsigned char *key);
    int (*sm2dh_compute_key)(
            int A_is_Sponsor,
            const EC_POINT *pubkey_tmp_B,
            const EC_POINT *pubkey_B,
            EC_KEY *key_tmp_A,
            EC_KEY *key_A,
            unsigned char *ID_A,
            int ID_A_len,
            unsigned char *ID_B,
            int ID_B_len,
            size_t klen,
            unsigned char *key);
    int flags;
    char *app_data;
};

void SM2DH_CTX_init(SM2DH_CTX *ctx);
void SM2DH_CTX_cleanup(SM2DH_CTX *ctx);
void SM2DH_CTX_free(SM2DH_CTX *ctx);
SM2DH_CTX *SM2DH_CTX_new(void);
int SM2DH_CTX_set_ex_data(SM2DH_CTX *ctx, int idx, void *arg);
void *SM2DH_CTX_get_ex_data(const SM2DH_CTX *ctx, int idx);

const   SM2DH_METHOD *SM2DH_OpenSSL(void);

void    SM2DH_set_default_method(const SM2DH_METHOD *);
const   SM2DH_METHOD *SM2DH_get_default_method(void);
int     SM2DH_set_method(EC_KEY *, const SM2DH_METHOD *);

int     SM2DH_get_ex_new_index(long argl, void *argp, CRYPTO_EX_new
        *new_func, CRYPTO_EX_dup *dup_func, CRYPTO_EX_free *free_func);
int     SM2DH_set_ex_data(EC_KEY *d, int idx, void *arg);
void    *SM2DH_get_ex_data(EC_KEY *d, int idx);

void ERR_load_SM2DH_strings(void);

/* SM2 dh step 1: sponsor generate tmp key pair */
int SM2_DH_GenerateAgreementData(
        SM2DH_CTX *ctx,
        EC_POINT *pubkey_tmp_A,
        BIGNUM *prikey_tmp_A,
        EC_KEY *key_A);

/* SM2 dh step 2: responser generate tmp key pair & calculate syn key use tmp key pair from sponsor */
int SM2_DH_GenerateAgreementDataAndKey(
        const EC_POINT *pubkey_tmp_A,
        EC_POINT *pubkey_tmp_B,
        const EC_POINT *pubkey_A,
        EC_KEY *key_B,
        unsigned char *ID_A,
        int ID_A_len,
        unsigned char *ID_B,
        int ID_B_len,
        size_t klen,
        unsigned char *key);

/* SM2 dh step 3: sponsor calculate syn key use tmp key pair from responser */
int SM2_DH_GenerateKey(
        SM2DH_CTX *ctx,
        const EC_POINT *pubkey_tmp_B,
        const EC_POINT *pubkey_B,
        EC_POINT *pubkey_tmp_A,
        BIGNUM *prikey_tmp_A,
        EC_KEY *key_A,
        unsigned char *ID_A,
        int ID_A_len,
        unsigned char *ID_B,
        int ID_B_len,
        size_t klen,
        unsigned char *key);

int SM2_DH_ComputeKey(
        int A_is_Sponsor,
        const EC_POINT *pubkey_tmp_B,
        const EC_POINT *pubkey_B,
        EC_KEY *key_tmp_A,
        EC_KEY *key_A,
        unsigned char *ID_A,
        int ID_A_len,
        unsigned char *ID_B,
        int ID_B_len,
        size_t klen,
        unsigned char *key);

#ifdef __cplusplus
}
#endif

#endif /* SM2DH_H_ */
