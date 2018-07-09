/*
 * driver.hpp
 *
 *  Created on: 2015-3-30
 *      Author: root
 */

#ifndef DRIVER_HPP_
#define DRIVER_HPP_

#include "../hlib/global.hpp"
#include "../hlib/scksvr.hpp"
#include "../proj/ap_type2s.hpp"
#include "../proj/ewifi_client.hpp"

// infrastructure
#define	CACHE_TWEAK_FACTOR	20
#define SMLBUFSIZ ( 256 + CACHE_TWEAK_FACTOR)

// These typedefs attempt to ensure consistent 'ticks' handling
typedef unsigned long long TIC_t;
typedef          long long SIC_t;

// This structure stores one piece of critical 'history'
// information from one frame to the next -- we don't calc
// and save data that goes unused
typedef struct HST_t {
   TIC_t tics;
   int   pid;
} HST_t;

// This structure stores a frame's cpu tics used in history
// calculations.  It exists primarily for SMP support but serves
// all environments.
typedef struct CPU_t {
   TIC_t u, n, s, i, w, x, y, z; // as represented in /proc/stat
   TIC_t u_sav, s_sav, n_sav, i_sav, w_sav, x_sav, y_sav, z_sav; // in the order of our display
   unsigned id;  // the CPU ID number
} CPU_t;

extern string get_dev_seq();
extern string get_dev_lan_ip();
extern string get_dev_wan_mac();
extern void get_dev_lan_ip(pByte buf);
extern void get_dev_wan_mac(pByte buf);

// event

extern void on_user_stat(string mac, string ip, bool pass, __u64 ibw_limit, __u64 obw_limit, string pop_url);
//extern void on_user_stat(string mac, string ip, bool pass, __u64 ibw_limit, __u64 obw_limit, string pop_url,puser_stat us);
extern void on_white_list(string wl);
extern void on_black_list(string bl);
extern void on_upgrade(string url);
extern void on_ap_initial_conf(PHOtherOption aic); // on_connect
extern void on_ap_runtime_conf(pap_net_conf2 arc);      // runtime conf
extern void on_ap_dev_conf(pap_dev_conf2 body);

// platform api
extern void ap_usraction(usr_action* action);
extern void ap_heartbeat(pap_stat2 as);
extern void sta_heartbeat(puser_stat2 us, int len);
extern void sta_audit(string ip, string mac, __u32 date, string url);

void Rotation();


#endif /* DRIVER_HPP_ */
