/*
 * proj_utils.hpp
 *
 *  Created on: 2015-1-27
 *      Author: masanari
 */

#ifndef PROJ_UTILS_HPP_
#define PROJ_UTILS_HPP_

#include "../hlib/global.hpp"
#include "../hlib/hpasutils.hpp"
#include "../proj/ap_type2s.hpp"
#include "../hlib/scksvr.hpp"
#include "../proj/ewifi_client.hpp"

string read_mac(pByte buf);
void write_mac(pByte buf, string mac);
string read_ip(pByte buf);
void write_ip(pByte buf, string ip);
string read_devid(pByte buf);
void write_devid(pByte buf, string dev_id);
string read_hardseq(pByte buf);
void write_hardseq(pByte buf, string dev_id);

void BytesToHex(char* bytes, int bytelength, char *hexstring, int hexstrlength);
void HexToBytes(char* hexstring, char* bytes, int hexlength);
int HexToInt(char c);

#endif /* PROJ_UTILS_HPP_ */
