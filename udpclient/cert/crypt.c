/*
 * crypt.c
 *
 *  Created on: Mar 15, 2017
 *      Author: oracle
 */
#include"crypt.h"

static void print_hex(const unsigned char *buf, int len) {
	int i;

	if (len == 0)
		return;
	printf("            0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");

	for (i = 0; i < len; i++) {
		if (i % 0x10 == 0) {
			printf("\n%08Xh: ", i);
		}
		printf("%02X ", buf[i]);
	}
	printf("\n");
}

int sm3_hmac(const unsigned char *key, int keylen, 
							const unsigned char *in, size_t inlen, unsigned char *out)
{
    int block_size = 0, i;
    unsigned int md_len = 0;
    unsigned char sum[64] = {0};      //EVP_MAX_MD_SIZE
    unsigned char tmpbuf[64] = {0};   //EVP_MAX_MD_SIZE
    unsigned char ipad[128] = {0};    //HMAC_MAX_MD_CBLOCK
    unsigned char opad[128] = {0};    //HMAC_MAX_MD_CBLOCK
    EVP_MD_CTX c;

    block_size = EVP_MD_block_size(EVP_sm3());
    EVP_MD_CTX_init(&c);

    if(keylen > block_size){
        EVP_DigestInit_ex(&c, EVP_sm3(), NULL);
        EVP_DigestUpdate(&c, key, keylen);
        EVP_DigestFinal_ex(&c, sum, &md_len);
        EVP_MD_CTX_cleanup(&c);
        keylen = md_len;
        key = sum;
    }

    memset(ipad, 0x36, block_size);
    memset(opad, 0x5C, block_size);

    for(i = 0; i < keylen; i++){
        ipad[i] = (unsigned char)(ipad[i] ^ key[i]);
        opad[i] = (unsigned char)(opad[i] ^ key[i]);
    }

    EVP_DigestInit_ex(&c, EVP_sm3(), NULL);
    EVP_DigestUpdate(&c, ipad, block_size);

    memset(sum, 0, sizeof sum);
    EVP_DigestUpdate(&c, in, inlen);
    EVP_DigestFinal_ex(&c, tmpbuf, &md_len);

    EVP_MD_CTX_cleanup(&c);
    EVP_DigestInit_ex(&c, EVP_sm3(), NULL);
    EVP_DigestUpdate(&c, opad, block_size);

    EVP_DigestUpdate(&c, tmpbuf, md_len);
    EVP_DigestFinal_ex(&c, out, &md_len);
    EVP_MD_CTX_cleanup(&c);

    memset(tmpbuf, 0, sizeof tmpbuf);
    return md_len;
}

EVP_PKEY * ReadPrivateKey(const char *certfile) {
	FILE *fp;
	EVP_PKEY *evp = NULL;
	EC_KEY *eckey = NULL;
	unsigned char *buf;
	int len;

	/* Read private key */
	/* if type is der*/
	fp = fopen(certfile, "rb");
	if (fp == NULL)
		return NULL;

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	rewind(fp);

	buf = (unsigned char*) malloc(sizeof(char) * len);
	fread(buf, sizeof(unsigned char), len, fp);
	eckey = d2i_ECPrivateKey(NULL, (const unsigned char**) &buf, len);
	fclose(fp);
	free(buf);

	if (eckey != NULL) {
		evp = EVP_PKEY_new();
		evp->pkey.ec = eckey;
	} else {
		/*if type is pem*/
		fp = fopen(certfile, "r");
		if (fp == NULL)
			return NULL;

		evp = PEM_read_PrivateKey(fp, NULL, NULL, NULL);

		fclose(fp);

		if (evp == NULL)
			return NULL;
	}

//	EC_KEY_print_fp(stderr, evp->pkey.ec, 0);

	return evp;

}

EVP_PKEY * ReadPublicKey(const char *data, unsigned int len) {
	X509 *x509;
	EVP_PKEY *pkey;
	X509_NAME *subj;
	int nid;
	char comname[20] = { 0 };
	x509 = d2i_X509(NULL, &data, len);

	if (x509 == NULL) {
		ERR_print_errors_fp(stderr);
		return NULL;
	}

	pkey = X509_extract_key(x509);
	X509_free(x509);

	if (pkey == NULL)
		ERR_print_errors_fp(stderr);

//	EC_KEY_print_fp(stderr, pkey->pkey.ec, 0);

	return pkey;
}

EVP_PKEY * ReadPublicKeyCert(const char *certfile) {
	FILE *fp = fopen(certfile, "rb");
	char *buf = NULL;
	int len = 0;

	if (!fp)
		return NULL;

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	rewind(fp);

	buf = (unsigned char*) malloc(sizeof(char) * len);
	fread(buf, sizeof(char), len, fp);
	fclose(fp);

	return ReadPublicKey(buf, len);
}
