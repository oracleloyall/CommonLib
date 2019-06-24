/*
 * sm2dsa.h
 *
 *  Created on: 2012-8-22
 *      Author: zweib
 */

#ifndef HEADER_SM2DSA_H
#define HEADER_SM2DSA_H

#include <openssl/ecdsa.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sm2dsa_method {
    const char *name;
    ECDSA_SIG *(*sm2dsa_do_sign)(const unsigned char *dgst, int dgst_len,
            const unsigned char *id, int id_len,
            const BIGNUM *inv, const BIGNUM *rp, EC_KEY *eckey);
    int (*sm2dsa_sign_setup)(EC_KEY *eckey, BN_CTX *ctx, BIGNUM **kinv,
            BIGNUM **r);
    int (*sm2dsa_do_verify)(const unsigned char *dgst, int dgst_len,
            const unsigned char *id, int id_len,
            const ECDSA_SIG *sig, EC_KEY *eckey);
#if 0
    int (*init)(EC_KEY *eckey);
    int (*finish)(EC_KEY *eckey);
#endif
    int flags;
    char *app_data;
};

const unsigned char *OpenSSL_get_default_SM2_id(void);
int OpenSSL_get_default_SM2_id_len(void);
void OpenSSL_set_default_SM2_id(const unsigned char *id, int id_len);
void OpenSSL_free_default_SM2_id(void);

/** Computes the SM2DSA signature of the given hash value using
 *  the supplied private key and returns the created signature.
 *  \param  dgst      pointer to the hash value
 *  \param  dgst_len  length of the hash value
 *  \param  eckey     EC_KEY object containing a private EC key
 *  \return pointer to a SM2DSA_SIG structure or NULL if an error occurred
 */
ECDSA_SIG *SM2DSA_do_sign(const unsigned char *dgst,int dgst_len,
        const unsigned char *id,int id_len, EC_KEY *eckey);

/** Computes SM2DSA signature of a given hash value using the supplied
 *  private key (note: sig must point to SM2DSA_size(eckey) bytes of memory).
 *  \param  dgst     pointer to the hash value to sign
 *  \param  dgstlen  length of the hash value
 *  \param  kinv     BIGNUM with a pre-computed inverse k (optional)
 *  \param  rp       BIGNUM with a pre-computed rp value (optioanl), 
 *                   see SM2DSA_sign_setup
 *  \param  eckey    EC_KEY object containing a private EC key
 *  \return pointer to a SM2DSA_SIG structure or NULL if an error occurred
 */
ECDSA_SIG *SM2DSA_do_sign_ex(const unsigned char *dgst, int dgstlen,
        const unsigned char *id, int idlen,
        const BIGNUM *kinv, const BIGNUM *rp, EC_KEY *eckey);

/** Verifies that the supplied signature is a valid SM2DSA
 *  signature of the supplied hash value using the supplied public key.
 *  \param  dgst      pointer to the hash value
 *  \param  dgst_len  length of the hash value
 *  \param  sig       SM2DSA_SIG structure
 *  \param  eckey     EC_KEY object containing a public EC key
 *  \return 1 if the signature is valid, 0 if the signature is invalid
 *          and -1 on error
 */
int      SM2DSA_do_verify(const unsigned char *dgst, int dgst_len,
        const unsigned char *id, int id_len,
        const ECDSA_SIG *sig, EC_KEY* eckey);

const SM2DSA_METHOD *SM2DSA_OpenSSL(void);

/** Sets the default SM2DSA method
 *  \param  meth  new default SM2DSA_METHOD
 */
void      SM2DSA_set_default_method(const SM2DSA_METHOD *meth);

/** Returns the default SM2DSA method
 *  \return pointer to SM2DSA_METHOD structure containing the default method
 */
const SM2DSA_METHOD *SM2DSA_get_default_method(void);

/** Sets method to be used for the SM2DSA operations
 *  \param  eckey  EC_KEY object
 *  \param  meth   new method
 *  \return 1 on success and 0 otherwise 
 */
int       SM2DSA_set_method(EC_KEY *eckey, const SM2DSA_METHOD *meth);

/** Precompute parts of the signing operation
 *  \param  eckey  EC_KEY object containing a private EC key
 *  \param  ctx    BN_CTX object (optional)
 *  \param  kinv   BIGNUM pointer for the inverse of k
 *  \param  rp     BIGNUM pointer for x coordinate of k * generator
 *  \return 1 on success and 0 otherwise
 */
int       SM2DSA_sign_setup(EC_KEY *eckey, BN_CTX *ctx, BIGNUM **kinv,
        BIGNUM **rp);

/** Computes SM2DSA signature of a given hash value using the supplied
 *  private key (note: sig must point to ECDSA_size(eckey) bytes of memory).
 *  \param  type     this parameter is ignored
 *  \param  dgst     pointer to the hash value to sign
 *  \param  dgstlen  length of the hash value
 *  \param  sig      memory for the DER encoded created signature
 *  \param  siglen   pointer to the length of the returned signature
 *  \param  eckey    EC_KEY object containing a private EC key
 *  \return 1 on success and 0 otherwise
 */
int      SM2DSA_sign(int type, const unsigned char *dgst, int dgstlen,
        const unsigned char *id, int idlen,
        unsigned char *sig, unsigned int *siglen, EC_KEY *eckey);


/** Computes SM2DSA signature of a given hash value using the supplied
 *  private key (note: sig must point to ECDSA_size(eckey) bytes of memory).
 *  \param  type     this parameter is ignored
 *  \param  dgst     pointer to the hash value to sign
 *  \param  dgstlen  length of the hash value
 *  \param  sig      buffer to hold the DER encoded signature
 *  \param  siglen   pointer to the length of the returned signature
 *  \param  kinv     BIGNUM with a pre-computed inverse k (optional)
 *  \param  rp       BIGNUM with a pre-computed rp value (optioanl), 
 *                   see SM2DSA_sign_setup
 *  \param  eckey    EC_KEY object containing a private EC key
 *  \return 1 on success and 0 otherwise
 */
int      SM2DSA_sign_ex(int type, const unsigned char *dgst, int dgstlen,
        const unsigned char *id, int idlen,
        unsigned char *sig, unsigned int *siglen, const BIGNUM *kinv,
        const BIGNUM *rp, EC_KEY *eckey);

/** Verifies that the given signature is valid SM2DSA signature
 *  of the supplied hash value using the specified public key.
 *  \param  type     this parameter is ignored
 *  \param  dgst     pointer to the hash value 
 *  \param  dgstlen  length of the hash value
 *  \param  sig      pointer to the DER encoded signature
 *  \param  siglen   length of the DER encoded signature
 *  \param  eckey    EC_KEY object containing a public EC key
 *  \return 1 if the signature is valid, 0 if the signature is invalid
 *          and -1 on error
 */
int       SM2DSA_verify(int type, const unsigned char *dgst, int dgstlen,
        const unsigned char *id, int idlen,
        const unsigned char *sig, int siglen, EC_KEY *eckey);

/* the standard ex_data functions */
int       SM2DSA_get_ex_new_index(long argl, void *argp, CRYPTO_EX_new
        *new_func, CRYPTO_EX_dup *dup_func, CRYPTO_EX_free *free_func);
int       SM2DSA_set_ex_data(EC_KEY *d, int idx, void *arg);
void       *SM2DSA_get_ex_data(EC_KEY *d, int idx);


/* BEGIN ERROR CODES */
/* The following lines are auto generated by the script mkerr.pl. Any changes
 * made after this point may be overwritten when the script is next run.
 */
void ERR_load_SM2DSA_strings(void);

/* Error codes for the SM2DSA functions. */

/* Function codes. */
#define SM2DSA_F_SM2DSA_DATA_NEW_METHOD        100
#define SM2DSA_F_SM2DSA_DO_SIGN                101
#define SM2DSA_F_SM2DSA_DO_VERIFY              102
#define SM2DSA_F_SM2DSA_SIGN_SETUP             103
#define SM2DSA_F_SM2DSA_SIGN_EX                104
#define SM2DSA_F_SM2DSA_VERIFY_EX               105

/* Reason codes. */
#define SM2DSA_R_BAD_SIGNATURE                100
#define SM2DSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE      101
#define SM2DSA_R_ERR_SM2_LIB               102
#define SM2DSA_R_MISSING_PARAMETERS           103
#define SM2DSA_R_NEED_NEW_SETUP_VALUES            106
#define SM2DSA_R_RANDOM_NUMBER_GENERATION_FAILED      104
#define SM2DSA_R_SIGNATURE_MALLOC_FAILED          105
#define SM2DSA_R_SIGNATURE_INPUT_TOO_LONG          106

#ifdef  __cplusplus
}
#endif
#endif
