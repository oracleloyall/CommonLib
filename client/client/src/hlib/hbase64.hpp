/*
 * hbase64.hpp
 *
 *  Created on: 2013-9-28
 *      Author: root
 */

#ifndef HBASE64_HPP_
#define HBASE64_HPP_

#include "global.hpp"

const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
extern char* base64_encode(const char* data, int data_len);
extern char *base64_decode(const char* data, int data_len);

#endif /* HBASE64_HPP_ */
