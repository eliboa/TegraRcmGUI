#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <SDKDDKVer.h>

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define _WIN32_WINNT	_WIN32_WINNT_WIN7

#ifdef WINVER
#undef WINVER
#endif

#define WINVER			_WIN32_WINNT

#include <windows.h>

#include <string>

#ifdef UNICODE
typedef std::wstring WinString;
#else
typedef std::string WinString;
#endif
