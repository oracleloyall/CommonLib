/*
 * ap_type2s.hpp
 *
 *  Created on: 2015骞�6鏈�17鏃�
 *      Author: sw
 */

#ifndef PROJ_AP_TYPE2S_HPP_
#define PROJ_AP_TYPE2S_HPP_

#include "../hlib/global.hpp"

typedef unsigned char  __u8;
typedef unsigned short __u16;
typedef unsigned int   __u32;
typedef unsigned long long __u64;

const __u16 H_PROTO_LOGO = 29559; 			// sw
const __u16	H_PROTO_UDP	 = 30000;

const __u8  H_PROTO_VER = 1;
const __u8 H_PROTO_NEO_VER = 2;				// !!NOTICE: 鐗堟湰v2 [鏂癩
const __u16 H_PROTO_MAX_LEN = 4096;
const __u16 H_LENGTH_NEED_ZIP = 500;      	   // 濡傛棤鐗规畩绾﹀畾锛岃秴杩囨闀垮害鑷姩鍘嬬缉

typedef __u8 msg_head_flag;

const msg_head_flag mhf_NONE = 0;
const msg_head_flag mhf_GZIP = 1;
const msg_head_flag mhf_ENCRYPT = 2;
const msg_head_flag mhf_ACK_REQED = 4;
const msg_head_flag mhf_NOT_END = 8;
const msg_head_flag mhf_ACK = 16;

typedef __u16 msg_cmd_type;

// 鐗堟湰v2
const msg_cmd_type msg_cmd_unknown = 0;				//
const msg_cmd_type msg_cmd_conn_req2 = 1;// FatAP鍙戣捣锛氳繛鎺ヨ璇侊紝瀵瑰簲struct ap_conn_req;
const msg_cmd_type msg_cmd_conn_resp2 = 2; // 绠＄悊鏈嶅姟鍣ㄨ繑鍥烇細鍒濆鍖栦俊鎭紝瀵瑰簲 struct ap_conn_resp;
const msg_cmd_type msg_cmd_ap_stat2 = 3; // FatAP鍙戝嚭锛欶atAP蹇冭烦锛屽搴� struct ap_stat;
const msg_cmd_type msg_cmd_ap_dns_white2 = 4; // 绠＄悊鏈嶅姟鍣ㄨ姹侳atAP淇敼鐧藉悕鍗�, 瀵瑰簲 struct ap_dns_white;
const msg_cmd_type msg_cmd_ap_dev_conf2 = 5; // 绠＄悊鏈嶅姟鍣ㄨ姹侳atAP淇敼閰嶇疆, 瀵瑰簲 struct ap_conf;
const msg_cmd_type msg_cmd_ap_net_conf2 = 6; // 绠＄悊鏈嶅姟鍣ㄨ姹侳atAP淇敼鐨勭綉缁滈厤缃紝瀵瑰簲ap_net_conf2
const msg_cmd_type msg_cmd_ap_upgrade2 = 7; // 璁剧疆FatAP鍗囩骇, 瀵瑰簲 struct ap_upgrade
const msg_cmd_type msg_cmd_user_stat2 = 8; // 鍙屾柟鍧囧彲鍙戝嚭锛氱鐞嗘湇鍔″櫒鍙戝嚭鍒欎慨鏀圭敤鎴蜂俊鎭� 锛孎atAP鍙戝嚭閫氬憡鐩墠鐘跺喌(涓婁笅绾裤�佸叾鍚庣殑蹇冭烦) 锛屽搴� struct user_stat;
const msg_cmd_type msg_cmd_user_action2 = 9; // FatAP鍙戝嚭锛氱敤鎴风殑鎿嶄綔锛屽搴攕truct user_action;
const msg_cmd_type msg_cmd_link_detection = 10; 	// 閾捐矾妫�娴嬪懡浠�
const msg_cmd_type msg_cmd_ack2 = 11; /* 鍥炲簲瀵规柟鐗瑰畾seq宸茬粡澶勭悊锛屾棤鍖呬綋銆�
 鍥炲簲鍗曚竴鐨勫寘鎺ユ敹锛宻eq娌跨敤瀵规柟seq銆俵en缃负0.
 鍥炲簲杩炵画澶氫釜鍖呮帴鏀讹紝seq缃负鍥炲簲鐨勯涓猻eq锛屼竴鐩村埌seq+len
 鍚屾椂缁欏畾鐩墠灏氭湭鎺ユ敹鐨勯娈祍eq鍦╟rc32c
												    */

const msg_cmd_type msg_cmd_conn_heartbeat2 = 12; // 鍗忚鏂瑰鏀寔UDP锛屽簲瀹氭椂鍙戯紝鏃犻渶鍥炲簲
const msg_cmd_type msg_cmd_black_list2 = 13; // 绠＄悊鏈嶅姟鍣ㄨ姹侳atAP淇敼榛戝悕鍗�,瀵瑰簲鐨剆truct ap_dns_black
const msg_cmd_type msg_cmd_reboot2 = 14;			// 绠＄悊鏈嶅姟鍣ㄨ姹侳atAP閲嶆柊鍚姩

//
const msg_cmd_type msg_cmd_ap_auth = 50;		// 璁惧娉ㄥ唽鍛戒护(閫氳鍗忚閲囩敤udp鏂瑰紡)

const msg_cmd_type msg_cmd_max = 255;


typedef struct msg_head2
{
	__u16 logo; /*鏍囧織锛宻w = smartwifi */
	__u8 ver; /*鐗堟湰锛屽垵濮嬩负2 */
	msg_head_flag flag;/*鎵╁睍鏍囪瘑锛�
	 浣�1浣嶄负1琛ㄧず鍖呬綋宸插仛 gzip
	 浣�2浣嶄负1琛ㄧず鍖呬綋宸插姞瀵嗭紙鏆傛椂鏈鐞嗗姞瀵嗭級
	 浣�3浣嶄负1琛ㄧず璇ュ寘闇�瑕佺瓑寰呭搴旂殑seq鐨勬帴鏀跺洖搴旓紙UDP蹇呴』缃�1锛�
	 浣�4浣嶄负1琛ㄧず鍖呬綋灏氭湭瀹岀粨锛屽彂閫佹柟宸茶嚜鍔ㄥ垏鍒嗚秴闀挎暟鎹寘銆傛帴鏀惰繛缁�掑seq锛岀洿鍒版煇涓寘flag璇ヤ綅涓�0鏃讹紝姝ゅ寘浣撴柟瀹岀粨銆�
	 浣�5浣嶄负1琛ㄧずack鍛戒护鍥炴姤鐨剆eq鍜宻eq浠ュ墠鐨勫潎宸叉敹鍒帮紙鍙湪ack鍛戒护鏃舵湁鏁堬級
					 */
	msg_cmd_type cmd;/*鐢ㄤ簬鏍囪瘑鍖呬綋鍛戒护绫诲瀷锛屽彇鍊艰enum msg_cmd_type*/
	__u16 len; /*鍚庢帴鏁版嵁鍖呬綋鐨勯暱搴�*/
	__u32 crc32c; /*鍖呬綋鏍￠獙鍜岋紝浣跨敤SSE4.2鐨勬爣鍑嗐�傚鍖呬綋 绯籫zip澶勭悊鍚庣粨鏋滐紝姝ゆ暟鍊间负瀵筭zip缁撴灉鐨勮绠楀�� 銆�
	 濡傛灉cmd涓篴ck锛屽垯姝ゅ瓧娈佃〃绀烘湡鏈涘彂閫佺殑绗竴娈电殑seq*/
	__u32 seq; /*鍛戒护娴佹按銆傝捣濮嬪�间负0锛屽叾鍚庤嚜鐒跺闀裤�傜敤浜庤瘑鍒摢浜涜姹傚凡缁忚鍥炲簲锛岄伩鍏嶉噸澶嶅鐞嗭紱浠ュ強涓撻棬閫氱煡鐗瑰畾seq宸茬粡澶勭悊*/
} *pmsg_head2;

const int H_HEAD_LEN2 = sizeof(msg_head2);

typedef __u16 mi_type;
const mi_type mi_unknown = 0;
const mi_type mi_h3c = 1;
const mi_type mi_huawei = 2;
const mi_type mi_threenet = 3;
const mi_type mi_zdc = 4;
const mi_type mi_raisecom = 5;
const mi_type mi_wisechoice = 6;
const mi_type mi_commsky = 7;
const mi_type mi_ewifi = 255;
const mi_type mi_telin_daemon = 65535;

typedef __u8 UDIC_type;		/* Unique device identification code */
const UDIC_type UDIC_unknown = 0; /* 杩樹负鑾峰彇璁惧鍞竴鏍囪瘑鐮� */

typedef struct ap_conn_req2
{
	__u32 ap_time; /* FatAP鏃堕棿 */
	__u32 soft_id; /* 璁惧杞欢鍨嬪彿 65535*/
	__u32 hard_id; /* 璁惧纭欢鍨嬪彿 0 */
	__u32 ability; /* 璁惧鑳藉姏鎻忚堪锛�

	 __u8 hard_seq[64]; /*璁惧搴忓垪鍙�*/
	__u8 ip[16]; /* ipv4 绌轰綑鍚庨潰12涓瓧鑺� */
	__u8  mac[6];
	__u16 reserved;			/*  */
	mi_type manu_id; /*鍘傚晢浠ｇ爜*/
	UDIC_type udic[12]; /*璁惧鍞竴鏍囪瘑鐮�*/
	__u8  dev_id[16];
} *pap_conn_req2;

typedef struct ap_auth
{
	bool if_test;		// true琛ㄧず鐢ㄤ簬娴嬭瘯娉ㄥ唽姝ラ
	__u16 port;			// 缃戝叧绔彛
	__u8 gw_mac[6];		// 缃戝叧MAC
	__u8 gw_ip[16];		// 缃戝叧鐨刬p
	__u8 accout_id[20];	// 璐︽埛
	__u8 passwd[20];		// 璐﹀彿瀵嗙爜
	__u8 soft_ver[20];	// 鐗堟湰
	__u8 seq[20];		// 涓插彿
} *pap_auth;

typedef struct ap_auth_sp
{
	__u8 devid[16];	// 璁惧ID
} *pap_auth_sp;

typedef __u16  udp_type;

typedef struct UDP_Msg
{
	udp_type type;
	union
	{
		ap_auth ap_reg;
		ap_auth_sp ap_req_resp;
	}data;
} *pUDP_Msg;

const int AP_AUTH_LEN = sizeof(UDP_Msg);

typedef __u8 cs_type;

const cs_type cs_unknown = 0;
const cs_type cs_telecom = 1;
const cs_type cs_cinema = 2;
const cs_type cs_supermarket = 3;
const cs_type cs_goverment = 4;
const cs_type cs_mall = 5;
const cs_type cs_cafe = 6;
const cs_type cs_bar = 7;
const cs_type cs_college = 8;
const cs_type cs_test = 254;
const cs_type cs_max = 255;

typedef __u8 ap_conn_ret;

const ap_conn_ret acr_OK = 0;
const ap_conn_ret acr_DENIED = 1;
const ap_conn_ret acr_REDIRECT = 2;

typedef __u8 ap_auth_mode;

const ap_auth_mode aam_PASS = 0;
const ap_auth_mode aam_AUTH = 1;

typedef __u8 ap_audit_mode;

const ap_audit_mode aam_NONE = 0;
const ap_audit_mode aam_AUDIT = 1;

typedef struct ap_conn_resp2
{
	ap_conn_ret result; /* 璁よ瘉缁撴灉锛�0=閫氳繃锛�1=鎷掔粷锛寃hite涓哄鍚戝湴鍧�锛�2=瑕佹眰杞叾浠栫鐞嗘湇鍔″櫒锛�
	 搴旇浆鎺uth_srv涓潪璇ョ鐞嗘湇鍔″櫒鐨勫叾浠栨湇鍔″櫒 */

	ap_auth_mode auth_mode; /* 缁堢璁よ瘉妯″紡锛�0=鐩存帴鏀捐锛�1=瑕佹眰璁よ瘉 */
	ap_audit_mode audit_mode; /* 瀹¤妯″紡锛�0=涓嶄紶鐢ㄦ埛璁块棶鐨刄RL锛�1=鍥炰紶鐢ㄦ埛璁块棶鐨刄RL */
	cs_type serv_type; /* 鍦烘墍绫诲瀷锛屽彇鍊煎弬鑰僥num cs_type锛� 濡傦紝閰掑簵銆佽嵂搴椼�佸皬鍖虹瓑绛� */
	UDIC_type udic; /* 璁惧鍞竴琛ㄧず鐮� */
	__u16 reserved1; /* 鏈�澶т紶杈撳崟鍏� */
	__u16 ap_interval; /* FatAP涓庣鐞嗘湇鍔″櫒闂寸殑蹇冭烦鏃堕棿锛屽崟浣嶇锛岄粯璁や负60绉� */
	__u16 user_interval; /* 缁堢鐢ㄦ埛姹囨姤鐨勫績璺虫椂闂达紝鍗曚綅绉掞紝榛樿涓�60绉� */
	__u16 audit_interval; /* 鐢ㄦ埛琛屼负姹囨姤锛屽崟浣嶇锛岄粯璁や负60绉� */
	__u8 auth_srv[64]; /* 绠＄悊鏈嶅姟鍣ㄥ湴鍧�锛屾瘮濡�: www.auth_srv.com:8080, 0d鍒嗗壊锛岀浜屼釜鍙婁互鍚庝负beiyong */
	__u8 audit_srv[64]; /* 瀹¤鏈嶅姟鍣紝鐩墠鏆傛椂淇濈暀涓嶇敤锛�0d鍒嗗壊锛�*/
	__u8 def_302[128]; /* 榛樿璺宠浆鍦板潃锛屽湪struct user_stat 涓病鏈夋寚瀹氳烦杞湴鍧�鏃朵娇鐢� */
	__u16 white_len; /* 鍩熷悕榛戠櫧鍚嶅崟闀垮害	鏈�闀夸负65535 */
	__u16 black_len; /* 鍩熷悕榛戝悕鍗曢暱搴�	鏈�闀夸负65535 */
	__u8 white[0]; /* 鍩熷悕鐧藉悕鍗曪紝鍩熷悕闂翠互鍒嗗彿鍒嗛殧 */
	__u8 black[0]; /* 鍩熷悕榛戝悕鍗曪紝鍩熷悕闂翠互鍒嗗彿鍒嗛殧 */
} *pap_conn_resp2;

typedef __u8 user_stat_type;	// 鐢ㄦ埛鐘舵��

const user_stat_type user_unknown = 0;
const user_stat_type user_on = 1; 	// 鐢ㄦ埛宸蹭笂绾挎湭鏀捐
const user_stat_type user_pass = 2;	// 鐢ㄦ埛宸叉斁琛�
const user_stat_type user_off = 3;	// 鐢ㄦ埛宸蹭笅绾�
const user_stat_type user_max = 255;

typedef struct user_stat2
{
	user_stat_type stat;// __u8
	__u8 url_len;      // 瀵瑰簲鍖呯殑缁撳熬鏈夊闀縐RL
	__u8  mac[6];
	__u8 user_ip[16];	// IPv4鏃跺悗闈�12瀛楄妭鐣�0
	__u64 incoming;		// 涓嬭娴侀噺璁℃暟锛屽崟浣嶄负瀛楄妭
	__u64 outgoing;		// 涓婅娴侀噺璁℃暟锛屽崟浣嶄负瀛楄妭
	__u64 ibw_limit; 	// 涓嬭甯﹀闄愬埗锛屽崟浣嶄负瀛楄妭锛�-1琛ㄧず涓嶉檺
	__u64 obw_limit; 	// 涓婅甯﹀闄愬埗锛屽崟浣嶄负瀛楄妭锛�-1琛ㄧず涓嶉檺
	__u32 online_date;	// unix鏃堕棿鎴筹紝0琛ㄧず鏃犺褰�
	__u32 offline_date;	// unix鏃堕棿鎴筹紝0琛ㄧず鏃犺褰�
	__u32 duration; 	// 宸茬粡涓婄嚎鏃堕棿锛屽崟浣嶄负绉掞紝鐘舵�佹敼鍙橈紙涓婄嚎/鏀捐/韪㈠嚭锛夋椂褰�0
	__u16 ses_limit;    // tcp session 闄愬埗锛�0=涓嶉檺
	__u16 udp_limit;    // udp session 闄愬埗锛�0=涓嶉檺
	__u8 url_data[0];  // URL鏁版嵁,缁堢璁よ瘉鎴愬姛鍚庨渶瑕佽烦杞殑椤甸潰銆傝嫢涓嶅瓨鍦紝鍒欎娇鐢ㄩ粯璁よ烦杞〉闈�
} *puser_stat2;

typedef __u8 wlan_visiblity;

const wlan_visiblity wv_broadcast = 0;
const wlan_visiblity wv_hidden = 1;

typedef __u8 wlan_enable;

const wlan_enable we_enable = 1;
const wlan_enable we_disable = 0;

typedef __u8 wlan_ssid_encoding;

const wlan_ssid_encoding wse_gb2312 = 0; // for PC
const wlan_ssid_encoding wse_utf8 = 1;   // for Phone

typedef __u8 wlan_type;

const wlan_type wt_unknown = 0;
const wlan_type wt_b = 1;
const wlan_type wt_g = 2;
const wlan_type wt_n = 4;
const wlan_type wt_a = 8;

const wlan_type wt_bg = 3;
const wlan_type wt_bgn = 7;
const wlan_type wt_abgn = 15;
#define		DEL_WLAN		0x80
typedef struct wlan_entity2
{
	__u8 wlan_ssid_len;
	__u8 wlan_ssid[255];
	__u8 wlan_index; 				// 绱㈠紩锛岀敤浜庢棩鍚庢搷浣�    鏈�楂樹綅缃�1锛岃〃绀哄垹闄ょ浉搴攚lan
	wlan_visiblity wlan_hidden;		    // 1=hidden,0=broadcast
	wlan_enable wlan_enabled;			// 1=enable,0=disable
	wlan_ssid_encoding wlan_utf8;		// 1=utf8,0=gbk
	ap_auth_mode auth_mode;			// 缁堢璁よ瘉妯″紡锛�0=鐩存帴鏀捐锛�1=瑕佹眰璁よ瘉
	__u8 encrtpt_mode;					// 鍔犲瘑绠楁硶 0=涓嶅姞瀵� 1=AES 2=TKIP
	__u8 wlan_txpower;					//                        : 4
	__u8 wlan_channel;
	__u8 wlan_channel2; 				// 0=鏈睍棰�
	__u8 session_limit;					// 鎺ュ叆缁堢鏁伴噺闄愬埗
	wlan_type dev_type; 				// abgn = 15
	__u8 wlan_vlan;						//                        : 8
	wlan_enable dhcp_enabled;			// 1=enable,0=disable
	__u8 sofe_mode;						// 0=unkown 1=NO,2=WEp 3=WPA
	__u8 secret_key[20];				// 鏅�歸ifi妯″紡涓�,闇�瑕佽緭鍏ョ殑瀵嗙爜
	__u8 dhcp_start[16];				// DHCP 璧峰IP鍦板潃
	__u8 dhcp_end[16];					// DHCP 缁堟IP鍦板潃
	__u8 wlan_ip[16];					//
	__u16 white_len;					// 鍚庨殢鐧藉悕鍗曢暱搴�
	__u16 black_len;					// 鍚庨殢榛戝悕鍗曢暱搴�
	__u8  whitelist[0];
	__u8  blacklist[0];
} *pwlan_entity2;

typedef __u8 wan_stat;

const wan_stat ws_unknown = 0;
const wan_stat ws_connecting = 1;
const wan_stat ws_connected = 2;
const wan_stat ws_disabled = 3;

typedef __u8 wan_mode;

const wan_mode wm_unknown = 0;
const wan_mode wm_PPPoE = 1;
const wan_mode wm_DHCP = 2;
const wan_mode wm_static = 3;

typedef struct wan_entity2
{
	__u8 wan_index;
	wan_stat wan_status;     // 1=enable,0=disable
	__u8 wan_mac[6];
	__u8 wan_ip[16];
	__u8 wan_gw[16];
	__u8 wan_mask[16];
	wan_mode conn_mode;      // 0=unknown,1=PPPoE,2=DHCP,3=static : 0
	wlan_enable auto_dns;    // 0=disabled,1=enabled
	__u8 dns_mode;			 // 1=鑷姩鑾峰彇,0=鎵嬪姩鑾峰彇
	__u16 wan_mtu;
	__u32 reserved;
	__u8 host_len;
	__u8 host[31];
	__u8 dns_count;			 // 璁板綍鏈夊灏慸ns鏈嶅姟鍣�
	__u8 dns[255];// 褰撴湁澶氫釜dns鏈嶅姟鍣ㄦ椂,ip鍦板潃绱у瘑鎺掑垪娌℃湁鍒嗛殧绗�,鍒ゆ柇涓嬩竴涓猧p鏄惁涓�0,鍒ゆ柇dns鍒楄〃鏄惁缁撴潫
	__u8 usr_len;            // pppoe name
	__u8 usr[31];
	__u8 pswd_len;
	__u8 pswd[31];
} *pwan_entity2;

typedef __u8 vlan_mode;

const vlan_mode vm_unknown = 0;
const vlan_mode vm_by_port = 1;
const vlan_mode vm_by_mac = 2;
const vlan_mode vm_by_prot = 3;			//鍗忚
const vlan_mode vn_by_ip = 4;

typedef union vlan_node
{
	__u16 port;
	__u8  mac[6];
	__u8  prot[8];
} *pvlan_node;
#define		DEL_VLAN	0x80
typedef struct vlan_entity
{
	__u8 vlan_index;					// 鏈�楂樹綅缃�1锛岃〃绀哄垹闄ゅ搴旂殑vlan
										// 1 0 0 0 0 0 0 2 琛ㄧず鍒犻櫎id涓�2 鐨剉lan
	ap_auth_mode auth_mode;				// 缁堢璁よ瘉妯″紡锛�0=鐩存帴鏀捐锛�1=瑕佹眰璁よ瘉
	ap_audit_mode audit_mode;		// 瀹¤妯″紡锛�0=涓嶄紶鐢ㄦ埛璁块棶鐨刄RL锛�1=鍥炰紶鐢ㄦ埛璁块棶鐨刄RL
	vlan_mode dev_mode;					// vlan 鍒掑垎渚濇嵁
	__u32 vlan_down_bw;				// vlan鏁翠綋涓嬭甯﹀,鍗曚綅瀛愯妭
	__u32 vlan_up_bw;					// vlan鏁翠綋涓婅甯﹀,鍗曚綅瀛愯妭
	__u32 sta_down_bw;				// 缁堢涓嬭甯﹀,鍗曚綅瀛愯妭
	__u32 sta_up_bw;					// 缁堢涓婅甯﹀,鍗曚綅瀛愯妭
	__u16 biz_ses_limit;    			// 鍟嗕笟Wifi鏁翠綋tcp session闄愬埗
	__u16 biz_udp_limit;    			// 鍟嗕笟Wifi鏁翠綋udp session闄愬埗
	__u8 ip_start[16];				// 鍙湁褰撴寜鐓p鍒掑垎鏃舵墠鏈夋晥
	__u8	ip_end[16];
	__u16	reserved;
	__u16 	node_len;					//
	vlan_node node[0];
} *pvlan_entity;

typedef struct lan_entity
{
	__u8 dhcp_mode;					// 1=寮�鍚痙hcp銆�0=鍏抽棴鐨刣hcp
	__u8	vlan_id;
	__u16	reserved1;
	__u32	leasetime;
	__u8	gw_mac[6];
	__u16	reserved2;
	__u8	dhcp_start[16];
	__u8	dhcp_end[16];
	__u8	gw_ip[16];
	__u8	mask[16];
} *plan_entity;

typedef struct ap_stat2
{
	__u32 free_mem;  			// 鍙敤鍐呭瓨锛屽崟浣嶄负K
	__u32 free_storage;			// 鍙敤瀛樺偍锛屽崟浣嶄负K
	__u32 ap_time;				// 绯荤粺鏃堕棿
	__u32 uptime;				// ap鍦ㄧ嚎鏃堕棿锛屽崟浣嶄负绉�
	__u64 incoming;				// 涓嬭鎬绘祦閲忥紝鍗曚綅涓哄瓧鑺�
	__u64 outgoing;				// 涓婅鎬绘祦閲忥紝鍗曚綅涓哄瓧鑺�
	__u16 cpu;		  			// 鍗曚綅涓轰竾鍒嗕箣涓�(鏁存暟閮ㄥ垎)
	ap_auth_mode auth_mode; /* 缁堢璁よ瘉妯″紡锛�0=鐩存帴鏀捐锛�1=瑕佹眰璁よ瘉 */
	ap_audit_mode audit_mode; /* 瀹¤妯″紡锛�0=涓嶄紶鐢ㄦ埛璁块棶鐨刄RL锛�1=鍥炰紶鐢ㄦ埛璁块棶鐨刄RL */
	__u16 online_user;			// 褰撳墠鐘跺喌
	__u16 authed_user;			// 褰撳墠鐘跺喌
	__u16 ap_interval; /* FatAP涓庣鐞嗘湇鍔″櫒闂寸殑蹇冭烦鏃堕棿锛屽崟浣嶇锛岄粯璁や负30绉� */
	__u16 user_interval; /* 缁堢鐢ㄦ埛姹囨姤鐨勫績璺虫椂闂达紝鍗曚綅绉掞紝榛樿涓�30绉� */
	__u16 audit_interval;		// 琛屼负涓婃姤闂撮殧
	__u16 reserved1;
	__u32 biz_ibw_limit;		// 鍟嗕笟Wifi鏁翠綋涓嬭甯﹀闄愬埗锛屽崟浣嶄负瀛楄妭
	__u32 biz_obw_limit;		// 鍟嗕笟Wifi鏁翠綋涓婅甯﹀闄愬埗锛屽崟浣嶄负瀛楄妭
	__u32 biz_ibw;
	__u32 biz_obw;
	__u16 biz_ses_limit;    	// 鍟嗕笟Wifi鏁翠綋tcp session闄愬埗
	__u16 biz_udp_limit;    	// 鍟嗕笟Wifi鏁翠綋udp session闄愬埗
	__u16 biz_ses;
	__u16 biz_udp;
	__u16 wlan_count;			// 涔嬪悗鏈夊灏憌lan_entity
	__u16 vlan_count;			// 涔嬪悗鏈夊灏憌an_entity
	__u16 wan_count;			// 涔嬪悗鏈夊灏憌an_entity
	__u16 reserved2;

	wlan_entity2 wlan[0];
	wan_entity2  wan[0];
	vlan_entity  vlan[0];
} *pap_stat2;

enum Ext_Dev_Conf			// 瀹氫箟璁惧鎵╁睍鍙互鏀寔閰嶇疆
{
	no_ext_dev_conf = 0,		// 琛ㄧずap_dev_conf2鍚庢病鏈夎窡浠讳綍鎵╁睍閰嶇疆
	base_dev_conf = 1,		// 琛ㄧずAP涓�浜涘熀纭�閰嶇疆,瀵瑰簲ap_dev_base_conf
	safe_dev_conf   = 2,		//
};

typedef	 __u16 dev_conf_type;

typedef	struct ap_dev_base_conf
{
	__u16 mtu;						// 鏈�澶т紶杈撳崟鍏�
	__u8 hostname[20];				// 淇敼涓绘満鍚�

	__u8 auth_srv[64];// 绠＄悊鏈嶅姟鍣ㄥ湴鍧�锛屾瘮濡�: www.auth_srv.com:8080, 0d鍒嗗壊锛岀浜屼釜鍙婁互鍚庝负beiyong
	__u8 audit_srv[64];			// 瀹¤鏈嶅姟鍣紝鐩墠鏆傛椂淇濈暀涓嶇敤锛�0d鍒嗗壊锛�
	__u8  plat_srv[64];
	__u8 def_302[128];		// 榛樿璺宠浆鍦板潃锛屽湪struct user_stat 涓病鏈夋寚瀹氳烦杞湴鍧�鏃朵娇鐢�

	__u16 ap_interval;				// FatAP涓庣鐞嗘湇鍔″櫒闂寸殑蹇冭烦鏃堕棿锛屽崟浣嶇锛岄粯璁や负60绉�
	__u16 user_interval;			// 缁堢鐢ㄦ埛姹囨姤鐨勫績璺虫椂闂达紝鍗曚綅绉掞紝榛樿涓�60绉�
	__u16 audit_interval;			// 鐢ㄦ埛琛屼负姹囨姤锛屽崟浣嶇锛岄粯璁や负60绉�
	__u16 gps_interval;				//

	__u32 biz_ibw_limit;			// 鍟嗕笟Wifi鏁翠綋涓嬭甯﹀闄愬埗锛屽崟浣嶄负瀛愯妭
	__u32 biz_obw_limit;			// 鍟嗕笟Wifi鏁翠綋涓婅甯﹀闄愬埗锛屽崟浣嶄负瀛愯妭
}*pap_dev_base_conf;

typedef struct ap_safe_dev_conf
{
	__u8 webaccount[20];			// web绠＄悊骞冲彴璐﹀彿
	__u8 webpasswd[20];			// web绠＄悊骞冲彴瀵嗙爜
	__u16 webport;					// web绠＄悊骞冲彴绔彛
	__u8 telnetaccount[20];		// telnet鐧诲綍璐﹀彿
	__u8 telnetpasswd[20];			// telnet鐧诲綍瀵嗙爜
	__u16 telentport;				// telnet绔彛
	__u8 pingenable;				// 0鍏抽棴ping,1鍏抽棴ping
} *pap_safe_dev_conf;

typedef	struct ap_dev_conf2
{
	dev_conf_type type;  		// 琛ㄧず闅忓悗鎵╁睍閰嶇疆绫诲瀷锛堟墍鏈夊彲鎵╁睍閰嶇疆閮藉湪Ext_Dev_Conf锛�
	__u8	dev_conf[0];
}*pap_dev_conf2;

typedef	__u16 	Conf_Count;

typedef struct ap_net_conf2
{
	Conf_Count		lan_count;
	Conf_Count		vlan_count;
	Conf_Count		wan_count;
	Conf_Count		wlan_count;
	lan_entity 		lan[0];
	vlan_entity 	vlan[0];
	wan_entity2		wan[0];
	wlan_entity2	wlan[0];
} *pap_net_conf2;

typedef __u16 user_action_type;
const user_action_type uatURL = 0;
const user_action_type uatTCP = 1;
const user_action_type uatUDP = 2;

typedef struct user_action_entry
{
	__u32 action_date;
	user_action_type action;
	__u16 data_len;
	__u8 data[0];
} *puser_action_entry;

typedef struct user_action2
{
	__u8 user_ip[16];
	__u8 mac[6];
	__u16 entry_count; // 鍏跺悗鎵挎帴澶氫釜user_action_entry, 姣忎釜entry澶氶暱鐢卞叾鑷韩鐨刣ata_len鎵�鍐冲畾
	user_action_entry uae_list[0];
} *puser_action2;

typedef struct ap_dns_white2
{
	__u8 data[0]; // dns鐧藉悕鍗曟暟鎹紝浠ュ崐瑙掑垎鍙峰垎鍓层�傛敮鎸佸湴鍧�銆佸湴鍧�:绔彛銆佸煙鍚嶃�佸煙鍚�:绔彛銆丮AC鍦板潃
} *pap_dns_white;

typedef struct ap_upgrade2
{
	__u8 url[0];				// 鍗囩骇鍖呯殑url鍦板潃
} *pap_upgrade;

typedef struct ap_link_detection
{
#if 0
	__u8 te_ip[16];			// IPv4鏃跺悗闈�12瀛楄妭鐣�0
	__u16 transmitted;// 鍙戦�佺殑鍖呮暟
	__u16 rececived;// 鎺ュ彈鐨勫寘鏁�
	__u16 loss;// 涓㈠け鐨勫寘鏁�
	__u16 time;// 妫�娴嬫椂闂� 锛屽崟浣峬s
#endif
}*pap_link_detection;

typedef struct ap_dns_black
{
	__u8 data[0];			// 榛戝悕鍗�
} *pap_dns_black;

typedef struct ap_reboot
{
	__u32 time;			// 寤惰繜澶氬皯绉掗噸鍚澶� 0绔嬪埢閲嶅惎
} *pap_reboot;

typedef struct {
	__u32 type;
	__u32 len;		// total packet length in bytes
	__u32 sequence;
} HEAD;
#endif /* PROJ_AP_TYPE2S_HPP_ */
