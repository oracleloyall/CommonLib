/*
 * hcp936.hpp
 *
 *  Created on: 2013-9-29
 *      Author: root
 */

#ifndef HCP936_HPP_
#define HCP936_HPP_

#ifndef H_ICONV
#include "global.hpp"

typedef int (*TUnicodeToCharID)(Cardinal Unicode);
extern string UTF8ToCP936(const string s);
extern Cardinal UTF8CharacterToUnicode(pByte p, int& CharLen);
extern string CP936ToUTF8(const string s);
extern int UnicodeToUTF8Inline(Cardinal CodePoint, pByte Buf);

#endif
#endif /* HCP936_HPP_ */
