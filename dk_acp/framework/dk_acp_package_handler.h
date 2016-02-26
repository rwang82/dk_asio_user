#ifndef __DK_ACP_PACKAGE_HANDLER_H__
#define __DK_ACP_PACKAGE_HANDLER_H__
//
class dk_acp_root;
//
class dk_acp_package_handler {
public:
	dk_acp_package_handler( dk_acp_root* pRoot );
	~dk_acp_package_handler();

private:
	dk_acp_root* m_pRoot;
};




#endif //__DK_ACP_PACKAGE_HANDLER_H__