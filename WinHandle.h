#pragma once

#include "Win32Def.h"

class WinHandle
{
public:
	WinHandle(HANDLE srcHandle = INVALID_HANDLE_VALUE) noexcept : _handle(srcHandle) {}
	~WinHandle()
	{
		if (_handle != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(_handle);
			_handle = INVALID_HANDLE_VALUE;
		}
	}

	void swap(WinHandle&& other) noexcept { std::swap(_handle, other._handle); }

	WinHandle(WinHandle&& moved) noexcept : _handle(INVALID_HANDLE_VALUE) { swap(std::move(moved)); }
	WinHandle(const WinHandle& copied) = delete;

	WinHandle& operator=(WinHandle&& moved) { swap(std::move(moved)); return *this; }
	WinHandle& operator=(const WinHandle& copied) = delete;

	HANDLE get() const { return _handle; }
	HANDLE release() { HANDLE retHandle = _handle; _handle = INVALID_HANDLE_VALUE; return retHandle; }
protected:
	HANDLE _handle;
};
