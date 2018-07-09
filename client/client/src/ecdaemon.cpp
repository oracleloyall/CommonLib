/*
 * ecdaemon.cpp
 *
 *  Created on: 2013-9-29
 *      Author: root
 */

#include "hlib/global.hpp"
#include "proj/ewifi_client.hpp"
#include "proj/randommac.hpp"
#include"client/client.hpp"
#define H_DEBUG
#ifdef POWER
TeWiFiClient* ec= NULL;
#endif
Client *client = NULL;
void StartService()
{
	printf("coming into startservice...\n");
	ec = new TeWiFiClient();
#ifdef POWER
	ec->power();
#endif
}
void StartService2() {
	printf("coming into startservice...\n");
	client = new Client();
#ifdef POWER
	client->Power_On();
//	client->SendDemoPacks(0);
#endif
}
int main()
{
#ifdef H_OPENWR_MAC
	RandomMac();
#endif
	#ifdef H_DEBUG
	StartService2();
	#else
	RunService("eWiFi daemon", StartService);
    #endif
	return 0;
}
