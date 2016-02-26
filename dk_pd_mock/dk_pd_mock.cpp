// dk_pd_mock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "dk_pd_root.h"

int _tmain(int argc, _TCHAR* argv[])
{
	if ( !dk_pd_start() ) {
	    printf( "dk_pd_start. failed.\n" );
		return 1;
	}

	while ( true )
	{
		::Sleep( 1000 );
	}

	dk_pd_stop();
	return 0;
}

