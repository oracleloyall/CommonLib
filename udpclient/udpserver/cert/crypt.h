/*
 * crypt.h
 *
 *  Created on: Mar 15, 2017
 *      Author: oracle
 */

#ifndef CERT_CRYPT_H_
#define CERT_CRYPT_H_



#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <syslog.h>

#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/pkcs12.h>
#include <openssl/ec.h>
#include <openssl/sm2dsa.h>

extern int sm3_hmac(const unsigned char *key, int keylen, 
							const unsigned char *in, size_t inlen, unsigned char *out);
extern EVP_PKEY * ReadPrivateKey(const char *certfile);


#endif /* CERT_CRYPT_H_ */
