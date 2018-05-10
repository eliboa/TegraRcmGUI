#pragma once

#include "Types.h"
#include "ScopeGuard.h"
#include "WinHandle.h"
#include <assert.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include "libusbk_int.h"

class TegraRcmSmash
{
public:
	TegraRcmSmash();
	~TegraRcmSmash();
	static int RcmStatus();
	static int Smash(TCHAR* payload, CString coreboot = _T(""));
	static int SmashMain(int argc, TCHAR* argv[]);
};

