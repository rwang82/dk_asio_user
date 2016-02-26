#ifndef __DK_ACP_ROOT_H__
#define __DK_ACP_ROOT_H__
#include "dk_socket_server.h"
#include "dk_acp_package_handler.h"
#include "dk_as_module.h"

class dk_acp_root {
public:
	dk_acp_root();
	~dk_acp_root();

private:
	void _init();
	void _unInit();

public:
    dk_acp_package_handler m_packageHandler;
	dk_as_module m_asManager;

}; 
















#endif //__DK_ACP_ROOT_H__