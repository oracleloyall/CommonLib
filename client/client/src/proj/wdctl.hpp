#ifndef _WDCTL_H_
#define _WDCTL_H_

#define DEFAULT_SOCK	"/tmp/wdctl.sock"

#define WDCTL_UNDEF		0
#define WDCTL_STATUS		1
#define WDCTL_STOP		2
#define WDCTL_KILL		3
#define WDCTL_RESTART	4
#define ALLLOW 5
#define DELETE 6

//static void usage(void);
void init_config(void);
//static void parse_commandline(int, char **);
int connect_to_server(const char *);
size_t send_request(int, const char *);
//static void wdctl_status(void);
//static void wdctl_stop(void);
//static void wdctl_reset(void);
//static void wdctl_restart(void);
char* wdctl_getdevid();
int wdctl_OnPowr();
int wdctl_down();
char *get_iface_ip(const char *ifname);
char *get_iface_mac(const char *ifname);

#endif
