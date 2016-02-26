#include "stdafx.h"
#include "dk_guid.h"
#include <Objbase.h>

bool createGUIDStr( std::string& strGUID ) {
	GUID guidNew;
	char szGUID[39];

	memset( szGUID, 0, 39 );
	if ( S_OK != ::CoCreateGuid( &guidNew ) )
		return false;
    sprintf_s( szGUID, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"\
		, guidNew.Data1, guidNew.Data2, guidNew.Data3\
		, guidNew.Data4[0], guidNew.Data4[1] \
		, guidNew.Data4[2], guidNew.Data4[3] \
		, guidNew.Data4[4], guidNew.Data4[5] \
		, guidNew.Data4[6], guidNew.Data4[7] );
	strGUID = szGUID;
	return true;
}

