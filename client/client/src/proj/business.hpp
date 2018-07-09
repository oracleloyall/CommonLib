/*
 * business.hpp
 *
 *  Created on: 2013-9-27
 *      Author: root
 */

#ifndef BUSINESS_HPP_
#define BUSINESS_HPP_

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <unistd.h>
#include "../hlib/global.hpp"
#include "../hlib/hpasutils.hpp"
#include "../hlib/hhmsi.hpp"
#include "../hlib/hhmii.hpp"
#include "../proj/ap_type2s.hpp"
#include "../hlib/scksvr.hpp"

const string H_BOX_NAME = "SW-100";

typedef void (*THCmdResponder)(THSockContext* Context);

//{* framework *}
extern bool InitDB();
extern void FreeDB();
extern bool RegisterCommandHandler(const int cmd, THCmdResponder Responder);
extern bool DispatchCommand(THSockContext* Context, const int cmd);

//{* business *}

extern string GenerateProductID(const string Mac);

extern bool AddGUID(const string GID, int i);
extern void DelGUID(const string GID);
extern string GetGUID();
extern int SeekGUID(const string GID);
extern bool SetGUID(const string GID, int i);
extern int GUIDCount();

extern string GetCommandDisplayName(msg_cmd_type mct);

#endif /* BUSINESS_HPP_ */
