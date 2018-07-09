#include "crbServer.h"
class server: public CRB3Server {
public:
	virtual int CheckService(void);
	virtual int GetStatus(void);virtual int DoService(
			IN CRB3_ADDRESS * ClientAddr,		// ip address of request client
			IN uint32_t CRId,// id of request
			IN CRB3Param * ParamIn,// parameters of request
			OUT CRB3Param ** pParamOut);// parameters of response

	virtual int DoServiceAsync(
			IN CRB3_ADDRESS * ClientAddr,		// ip address of request client
			IN uint32_t CRId,// id of request
			IN CRB3Param * ParamIn,// parameters of request (it must not be referenced when function return)
			IN uint64_t RspArg);// argument when response

	bool Config(IN crb_conf_t *conf_crb);
};
int main(int argc, char **argv) {
	return 0;
}
