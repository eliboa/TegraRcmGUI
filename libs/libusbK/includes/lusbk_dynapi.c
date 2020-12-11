/*!********************************************************************
libusbK - Multi-driver USB library.
Copyright (C) 2012 Travis Lee Robinson. All Rights Reserved.
libusb-win32.sourceforge.net

Development : Travis Lee Robinson  (libusbdotnet@gmail.com)
Testing     : Xiaofan Chen         (xiaofanc@gmail.com)

At the discretion of the user of this library, this software may be
licensed under the terms of the GNU Public License v3 or a BSD-Style
license as outlined in the following files:
* LICENSE-gpl3.txt
* LICENSE-bsd.txt

License files are located in a license folder at the root of source and
binary distributions.
********************************************************************!*/

#include <windows.h>
#include "libusbk.h"

#define mLoadLibraryExA LoadLibraryExA
#define mFreeLibrary FreeLibrary

HMODULE mLibusbK_ModuleHandle = NULL;

VOID LibusbK_DynApi_Free(VOID);
INT LibusbK_DynApi_Init(_inopt LPCSTR DllFullPathName);

// Function typedefs:

typedef VOID KUSB_API LibK_GetVersion_T(_out PKLIB_VERSION Version);

typedef KLIB_USER_CONTEXT KUSB_API LibK_GetContext_T(
    _in KLIB_HANDLE Handle,
    _in KLIB_HANDLE_TYPE HandleType);

typedef BOOL KUSB_API LibK_SetContext_T(
    _in KLIB_HANDLE Handle,
    _in KLIB_HANDLE_TYPE HandleType,
    _in KLIB_USER_CONTEXT ContextValue);

typedef BOOL KUSB_API LibK_SetCleanupCallback_T(
    _in KLIB_HANDLE Handle,
    _in KLIB_HANDLE_TYPE HandleType,
    _in KLIB_HANDLE_CLEANUP_CB* CleanupCB);

typedef BOOL KUSB_API LibK_LoadDriverAPI_T(
    _out PKUSB_DRIVER_API DriverAPI,
    _in INT DriverID);

typedef BOOL KUSB_API LibK_CopyDriverAPI_T(
    _out PKUSB_DRIVER_API DriverAPI,
    _in KUSB_HANDLE UsbHandle);

typedef BOOL KUSB_API LibK_GetProcAddress_T(
    _out KPROC* ProcAddress,
    _in INT DriverID,
    _in INT FunctionID);

typedef BOOL KUSB_API LibK_SetDefaultContext_T(
    _in KLIB_HANDLE_TYPE HandleType,
    _in KLIB_USER_CONTEXT ContextValue);

typedef KLIB_USER_CONTEXT KUSB_API LibK_GetDefaultContext_T(
    _in KLIB_HANDLE_TYPE HandleType);

typedef BOOL KUSB_API LibK_Context_Init_T(
    _inopt HANDLE Heap,
    _in PVOID Reserved);

typedef VOID KUSB_API LibK_Context_Free_T(VOID);

typedef BOOL KUSB_API UsbK_Init_T (
    _out KUSB_HANDLE* InterfaceHandle,
    _in KLST_DEVINFO_HANDLE DevInfo);

typedef BOOL KUSB_API UsbK_Free_T (
    _in KUSB_HANDLE InterfaceHandle);

typedef BOOL KUSB_API UsbK_ClaimInterface_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex);

typedef BOOL KUSB_API UsbK_ReleaseInterface_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex);

typedef BOOL KUSB_API UsbK_SetAltInterface_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex,
    _in UCHAR AltSettingNumber);

typedef BOOL KUSB_API UsbK_GetAltInterface_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex,
    _out PUCHAR AltSettingNumber);

typedef BOOL KUSB_API UsbK_GetDescriptor_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR DescriptorType,
    _in UCHAR Index,
    _in USHORT LanguageID,
    _out PUCHAR Buffer,
    _in UINT BufferLength,
    _outopt PUINT LengthTransferred);

typedef BOOL KUSB_API UsbK_ControlTransfer_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in WINUSB_SETUP_PACKET SetupPacket,
    _refopt PUCHAR Buffer,
    _in UINT BufferLength,
    _outopt PUINT LengthTransferred,
    _inopt LPOVERLAPPED Overlapped);

typedef BOOL KUSB_API UsbK_SetPowerPolicy_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UINT PolicyType,
    _in UINT ValueLength,
    _in PVOID Value);

typedef BOOL KUSB_API UsbK_GetPowerPolicy_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UINT PolicyType,
    _ref PUINT ValueLength,
    _out PVOID Value);

typedef BOOL KUSB_API UsbK_SetConfiguration_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR ConfigurationNumber);

typedef BOOL KUSB_API UsbK_GetConfiguration_T (
    _in KUSB_HANDLE InterfaceHandle,
    _out PUCHAR ConfigurationNumber);

typedef BOOL KUSB_API UsbK_ResetDevice_T (
    _in KUSB_HANDLE InterfaceHandle);

typedef BOOL KUSB_API UsbK_Initialize_T (
    _in HANDLE DeviceHandle,
    _out KUSB_HANDLE* InterfaceHandle);

typedef BOOL KUSB_API UsbK_SelectInterface_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex);

typedef BOOL KUSB_API UsbK_GetAssociatedInterface_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR AssociatedInterfaceIndex,
    _out KUSB_HANDLE* AssociatedInterfaceHandle);

typedef BOOL KUSB_API UsbK_Clone_T (
    _in KUSB_HANDLE InterfaceHandle,
    _out KUSB_HANDLE* DstInterfaceHandle);

typedef BOOL KUSB_API UsbK_QueryInterfaceSettings_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR AltSettingIndex,
    _out PUSB_INTERFACE_DESCRIPTOR UsbAltInterfaceDescriptor);

typedef BOOL KUSB_API UsbK_QueryDeviceInformation_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UINT InformationType,
    _ref PUINT BufferLength,
    _ref PVOID Buffer);

typedef BOOL KUSB_API UsbK_SetCurrentAlternateSetting_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR AltSettingNumber);

typedef BOOL KUSB_API UsbK_GetCurrentAlternateSetting_T (
    _in KUSB_HANDLE InterfaceHandle,
    _out PUCHAR AltSettingNumber);

typedef BOOL KUSB_API UsbK_QueryPipe_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR AltSettingNumber,
    _in UCHAR PipeIndex,
    _out PWINUSB_PIPE_INFORMATION PipeInformation);

typedef BOOL KUSB_API UsbK_SetPipePolicy_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _in UINT PolicyType,
    _in UINT ValueLength,
    _in PVOID Value);

typedef BOOL KUSB_API UsbK_GetPipePolicy_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _in UINT PolicyType,
    _ref PUINT ValueLength,
    _out PVOID Value);

typedef BOOL KUSB_API UsbK_ReadPipe_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _out PUCHAR Buffer,
    _in UINT BufferLength,
    _outopt PUINT LengthTransferred,
    _inopt LPOVERLAPPED Overlapped);

typedef BOOL KUSB_API UsbK_WritePipe_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _in PUCHAR Buffer,
    _in UINT BufferLength,
    _outopt PUINT LengthTransferred,
    _inopt LPOVERLAPPED Overlapped);

typedef BOOL KUSB_API UsbK_ResetPipe_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID);

typedef BOOL KUSB_API UsbK_AbortPipe_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID);

typedef BOOL KUSB_API UsbK_FlushPipe_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID);

typedef BOOL KUSB_API UsbK_IsoReadPipe_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _out PUCHAR Buffer,
    _in UINT BufferLength,
    _in LPOVERLAPPED Overlapped,
    _refopt PKISO_CONTEXT IsoContext);

typedef BOOL KUSB_API UsbK_IsoWritePipe_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _in PUCHAR Buffer,
    _in UINT BufferLength,
    _in LPOVERLAPPED Overlapped,
    _refopt PKISO_CONTEXT IsoContext);

typedef BOOL KUSB_API UsbK_GetCurrentFrameNumber_T (
    _in KUSB_HANDLE InterfaceHandle,
    _out PUINT FrameNumber);

typedef BOOL KUSB_API UsbK_GetOverlappedResult_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in LPOVERLAPPED Overlapped,
    _out PUINT lpNumberOfBytesTransferred,
    _in BOOL bWait);

typedef BOOL KUSB_API UsbK_GetProperty_T (
    _in KUSB_HANDLE InterfaceHandle,
    _in KUSB_PROPERTY PropertyType,
    _ref PUINT PropertySize,
    _out PVOID Value);

typedef BOOL KUSB_API LstK_Init_T(
    _out KLST_HANDLE* DeviceList,
    _in KLST_FLAG Flags);

typedef BOOL KUSB_API LstK_InitEx_T(
    _out KLST_HANDLE* DeviceList,
    _in KLST_FLAG Flags,
    _in PKLST_PATTERN_MATCH PatternMatch);

typedef BOOL KUSB_API LstK_Free_T(
    _in KLST_HANDLE DeviceList);

typedef BOOL KUSB_API LstK_Enumerate_T(
    _in KLST_HANDLE DeviceList,
    _in KLST_ENUM_DEVINFO_CB* EnumDevListCB,
    _inopt PVOID Context);

typedef BOOL KUSB_API LstK_Current_T(
    _in KLST_HANDLE DeviceList,
    _out KLST_DEVINFO_HANDLE* DeviceInfo);

typedef BOOL KUSB_API LstK_MoveNext_T(
    _in KLST_HANDLE DeviceList,
    _outopt KLST_DEVINFO_HANDLE* DeviceInfo);

typedef VOID KUSB_API LstK_MoveReset_T(
    _in KLST_HANDLE DeviceList);

typedef BOOL KUSB_API LstK_FindByVidPid_T(
    _in KLST_HANDLE DeviceList,
    _in INT Vid,
    _in INT Pid,
    _out KLST_DEVINFO_HANDLE* DeviceInfo);

typedef BOOL KUSB_API LstK_Count_T(
    _in KLST_HANDLE DeviceList,
    _ref PUINT Count);

typedef BOOL KUSB_API HotK_Init_T(
    _out KHOT_HANDLE* Handle,
    _ref PKHOT_PARAMS InitParams);

typedef BOOL KUSB_API HotK_Free_T(
    _in KHOT_HANDLE Handle);

typedef VOID KUSB_API HotK_FreeAll_T(VOID);

typedef BOOL KUSB_API OvlK_Acquire_T(
    _out KOVL_HANDLE* OverlappedK,
    _in KOVL_POOL_HANDLE PoolHandle);

typedef BOOL KUSB_API OvlK_Release_T(
    _in KOVL_HANDLE OverlappedK);

typedef BOOL KUSB_API OvlK_Init_T (
    _out KOVL_POOL_HANDLE* PoolHandle,
    _in KUSB_HANDLE UsbHandle,
    _in INT MaxOverlappedCount,
    _inopt KOVL_POOL_FLAG Flags);

typedef BOOL KUSB_API OvlK_Free_T(
    _in KOVL_POOL_HANDLE PoolHandle);

typedef HANDLE KUSB_API OvlK_GetEventHandle_T(
    _in KOVL_HANDLE OverlappedK);

typedef BOOL KUSB_API OvlK_Wait_T(
    _in KOVL_HANDLE OverlappedK,
    _inopt INT TimeoutMS,
    _inopt KOVL_WAIT_FLAG WaitFlags,
    _out PUINT TransferredLength);

typedef BOOL KUSB_API OvlK_WaitOldest_T(
    _in KOVL_POOL_HANDLE PoolHandle,
    _outopt KOVL_HANDLE* OverlappedK,
    _inopt INT TimeoutMS,
    _inopt KOVL_WAIT_FLAG WaitFlags,
    _out PUINT TransferredLength);

typedef BOOL KUSB_API OvlK_WaitOrCancel_T(
    _in KOVL_HANDLE OverlappedK,
    _inopt INT TimeoutMS,
    _out PUINT TransferredLength);

typedef BOOL KUSB_API OvlK_WaitAndRelease_T(
    _in KOVL_HANDLE OverlappedK,
    _inopt INT TimeoutMS,
    _out PUINT TransferredLength);

typedef BOOL KUSB_API OvlK_IsComplete_T(
    _in KOVL_HANDLE OverlappedK);

typedef BOOL KUSB_API OvlK_ReUse_T(
    _in KOVL_HANDLE OverlappedK);

typedef BOOL KUSB_API StmK_Init_T(
    _out KSTM_HANDLE* StreamHandle,
    _in KUSB_HANDLE UsbHandle,
    _in UCHAR PipeID,
    _in INT MaxTransferSize,
    _in INT MaxPendingTransfers,
    _in INT MaxPendingIO,
    _inopt PKSTM_CALLBACK Callbacks,
    _inopt KSTM_FLAG Flags);

typedef BOOL KUSB_API StmK_Free_T(
    _in KSTM_HANDLE StreamHandle);

typedef BOOL KUSB_API StmK_Start_T(
    _in KSTM_HANDLE StreamHandle);

typedef BOOL KUSB_API StmK_Stop_T(
    _in KSTM_HANDLE StreamHandle,
    _in INT TimeoutCancelMS);

typedef BOOL KUSB_API StmK_Read_T(
    _in KSTM_HANDLE StreamHandle,
    _out PUCHAR Buffer,
    _in INT Offset,
    _in INT Length,
    _out PUINT TransferredLength);

typedef BOOL KUSB_API StmK_Write_T(
    _in KSTM_HANDLE StreamHandle,
    _in PUCHAR Buffer,
    _in INT Offset,
    _in INT Length,
    _out PUINT TransferredLength);

typedef BOOL KUSB_API IsoK_Init_T(
    _out PKISO_CONTEXT* IsoContext,
    _in INT NumberOfPackets,
    _inopt INT StartFrame);

typedef BOOL KUSB_API IsoK_Free_T(
    _in PKISO_CONTEXT IsoContext);

typedef BOOL KUSB_API IsoK_SetPackets_T(
    _in PKISO_CONTEXT IsoContext,
    _in INT PacketSize);

typedef BOOL KUSB_API IsoK_SetPacket_T(
    _in PKISO_CONTEXT IsoContext,
    _in INT PacketIndex,
    _in PKISO_PACKET IsoPacket);

typedef BOOL KUSB_API IsoK_GetPacket_T(
    _in PKISO_CONTEXT IsoContext,
    _in INT PacketIndex,
    _out PKISO_PACKET IsoPacket);

typedef BOOL KUSB_API IsoK_EnumPackets_T(
    _in PKISO_CONTEXT IsoContext,
    _in KISO_ENUM_PACKETS_CB* EnumPackets,
    _inopt INT StartPacketIndex,
    _inopt PVOID UserState);

typedef BOOL KUSB_API IsoK_ReUse_T(
    _ref PKISO_CONTEXT IsoContext);



///////////////////////////////////////////////////////////////////////

// Function pointers:

static LibK_GetVersion_T* pLibK_GetVersion = NULL;

static LibK_GetContext_T* pLibK_GetContext = NULL;

static LibK_SetContext_T* pLibK_SetContext = NULL;

static LibK_SetCleanupCallback_T* pLibK_SetCleanupCallback = NULL;

static LibK_LoadDriverAPI_T* pLibK_LoadDriverAPI = NULL;

static LibK_CopyDriverAPI_T* pLibK_CopyDriverAPI = NULL;

static LibK_GetProcAddress_T* pLibK_GetProcAddress = NULL;

static LibK_SetDefaultContext_T* pLibK_SetDefaultContext = NULL;

static LibK_GetDefaultContext_T* pLibK_GetDefaultContext = NULL;

static LibK_Context_Init_T* pLibK_Context_Init = NULL;

static LibK_Context_Free_T* pLibK_Context_Free = NULL;

static UsbK_Init_T* pUsbK_Init = NULL;

static UsbK_Free_T* pUsbK_Free = NULL;

static UsbK_ClaimInterface_T* pUsbK_ClaimInterface = NULL;

static UsbK_ReleaseInterface_T* pUsbK_ReleaseInterface = NULL;

static UsbK_SetAltInterface_T* pUsbK_SetAltInterface = NULL;

static UsbK_GetAltInterface_T* pUsbK_GetAltInterface = NULL;

static UsbK_GetDescriptor_T* pUsbK_GetDescriptor = NULL;

static UsbK_ControlTransfer_T* pUsbK_ControlTransfer = NULL;

static UsbK_SetPowerPolicy_T* pUsbK_SetPowerPolicy = NULL;

static UsbK_GetPowerPolicy_T* pUsbK_GetPowerPolicy = NULL;

static UsbK_SetConfiguration_T* pUsbK_SetConfiguration = NULL;

static UsbK_GetConfiguration_T* pUsbK_GetConfiguration = NULL;

static UsbK_ResetDevice_T* pUsbK_ResetDevice = NULL;

static UsbK_Initialize_T* pUsbK_Initialize = NULL;

static UsbK_SelectInterface_T* pUsbK_SelectInterface = NULL;

static UsbK_GetAssociatedInterface_T* pUsbK_GetAssociatedInterface = NULL;

static UsbK_Clone_T* pUsbK_Clone = NULL;

static UsbK_QueryInterfaceSettings_T* pUsbK_QueryInterfaceSettings = NULL;

static UsbK_QueryDeviceInformation_T* pUsbK_QueryDeviceInformation = NULL;

static UsbK_SetCurrentAlternateSetting_T* pUsbK_SetCurrentAlternateSetting = NULL;

static UsbK_GetCurrentAlternateSetting_T* pUsbK_GetCurrentAlternateSetting = NULL;

static UsbK_QueryPipe_T* pUsbK_QueryPipe = NULL;

static UsbK_SetPipePolicy_T* pUsbK_SetPipePolicy = NULL;

static UsbK_GetPipePolicy_T* pUsbK_GetPipePolicy = NULL;

static UsbK_ReadPipe_T* pUsbK_ReadPipe = NULL;

static UsbK_WritePipe_T* pUsbK_WritePipe = NULL;

static UsbK_ResetPipe_T* pUsbK_ResetPipe = NULL;

static UsbK_AbortPipe_T* pUsbK_AbortPipe = NULL;

static UsbK_FlushPipe_T* pUsbK_FlushPipe = NULL;

static UsbK_IsoReadPipe_T* pUsbK_IsoReadPipe = NULL;

static UsbK_IsoWritePipe_T* pUsbK_IsoWritePipe = NULL;

static UsbK_GetCurrentFrameNumber_T* pUsbK_GetCurrentFrameNumber = NULL;

static UsbK_GetOverlappedResult_T* pUsbK_GetOverlappedResult = NULL;

static UsbK_GetProperty_T* pUsbK_GetProperty = NULL;

static LstK_Init_T* pLstK_Init = NULL;

static LstK_InitEx_T* pLstK_InitEx = NULL;

static LstK_Free_T* pLstK_Free = NULL;

static LstK_Enumerate_T* pLstK_Enumerate = NULL;

static LstK_Current_T* pLstK_Current = NULL;

static LstK_MoveNext_T* pLstK_MoveNext = NULL;

static LstK_MoveReset_T* pLstK_MoveReset = NULL;

static LstK_FindByVidPid_T* pLstK_FindByVidPid = NULL;

static LstK_Count_T* pLstK_Count = NULL;

static HotK_Init_T* pHotK_Init = NULL;

static HotK_Free_T* pHotK_Free = NULL;

static HotK_FreeAll_T* pHotK_FreeAll = NULL;

static OvlK_Acquire_T* pOvlK_Acquire = NULL;

static OvlK_Release_T* pOvlK_Release = NULL;

static OvlK_Init_T* pOvlK_Init = NULL;

static OvlK_Free_T* pOvlK_Free = NULL;

static OvlK_GetEventHandle_T* pOvlK_GetEventHandle = NULL;

static OvlK_Wait_T* pOvlK_Wait = NULL;

static OvlK_WaitOldest_T* pOvlK_WaitOldest = NULL;

static OvlK_WaitOrCancel_T* pOvlK_WaitOrCancel = NULL;

static OvlK_WaitAndRelease_T* pOvlK_WaitAndRelease = NULL;

static OvlK_IsComplete_T* pOvlK_IsComplete = NULL;

static OvlK_ReUse_T* pOvlK_ReUse = NULL;

static StmK_Init_T* pStmK_Init = NULL;

static StmK_Free_T* pStmK_Free = NULL;

static StmK_Start_T* pStmK_Start = NULL;

static StmK_Stop_T* pStmK_Stop = NULL;

static StmK_Read_T* pStmK_Read = NULL;

static StmK_Write_T* pStmK_Write = NULL;

static IsoK_Init_T* pIsoK_Init = NULL;

static IsoK_Free_T* pIsoK_Free = NULL;

static IsoK_SetPackets_T* pIsoK_SetPackets = NULL;

static IsoK_SetPacket_T* pIsoK_SetPacket = NULL;

static IsoK_GetPacket_T* pIsoK_GetPacket = NULL;

static IsoK_EnumPackets_T* pIsoK_EnumPackets = NULL;

static IsoK_ReUse_T* pIsoK_ReUse = NULL;



///////////////////////////////////////////////////////////////////////


VOID LibusbK_DynApi_Free(VOID)
{
	if (mLibusbK_ModuleHandle)
	{
		mFreeLibrary(mLibusbK_ModuleHandle);
		mLibusbK_ModuleHandle = NULL;

		// Set all function pointers to null:

		pLibK_GetVersion = NULL;

		pLibK_GetContext = NULL;

		pLibK_SetContext = NULL;

		pLibK_SetCleanupCallback = NULL;

		pLibK_LoadDriverAPI = NULL;

		pLibK_CopyDriverAPI = NULL;

		pLibK_GetProcAddress = NULL;

		pLibK_SetDefaultContext = NULL;

		pLibK_GetDefaultContext = NULL;

		pLibK_Context_Init = NULL;

		pLibK_Context_Free = NULL;

		pUsbK_Init = NULL;

		pUsbK_Free = NULL;

		pUsbK_ClaimInterface = NULL;

		pUsbK_ReleaseInterface = NULL;

		pUsbK_SetAltInterface = NULL;

		pUsbK_GetAltInterface = NULL;

		pUsbK_GetDescriptor = NULL;

		pUsbK_ControlTransfer = NULL;

		pUsbK_SetPowerPolicy = NULL;

		pUsbK_GetPowerPolicy = NULL;

		pUsbK_SetConfiguration = NULL;

		pUsbK_GetConfiguration = NULL;

		pUsbK_ResetDevice = NULL;

		pUsbK_Initialize = NULL;

		pUsbK_SelectInterface = NULL;

		pUsbK_GetAssociatedInterface = NULL;

		pUsbK_Clone = NULL;

		pUsbK_QueryInterfaceSettings = NULL;

		pUsbK_QueryDeviceInformation = NULL;

		pUsbK_SetCurrentAlternateSetting = NULL;

		pUsbK_GetCurrentAlternateSetting = NULL;

		pUsbK_QueryPipe = NULL;

		pUsbK_SetPipePolicy = NULL;

		pUsbK_GetPipePolicy = NULL;

		pUsbK_ReadPipe = NULL;

		pUsbK_WritePipe = NULL;

		pUsbK_ResetPipe = NULL;

		pUsbK_AbortPipe = NULL;

		pUsbK_FlushPipe = NULL;

		pUsbK_IsoReadPipe = NULL;

		pUsbK_IsoWritePipe = NULL;

		pUsbK_GetCurrentFrameNumber = NULL;

		pUsbK_GetOverlappedResult = NULL;

		pUsbK_GetProperty = NULL;

		pLstK_Init = NULL;

		pLstK_InitEx = NULL;

		pLstK_Free = NULL;

		pLstK_Enumerate = NULL;

		pLstK_Current = NULL;

		pLstK_MoveNext = NULL;

		pLstK_MoveReset = NULL;

		pLstK_FindByVidPid = NULL;

		pLstK_Count = NULL;

		pHotK_Init = NULL;

		pHotK_Free = NULL;

		pHotK_FreeAll = NULL;

		pOvlK_Acquire = NULL;

		pOvlK_Release = NULL;

		pOvlK_Init = NULL;

		pOvlK_Free = NULL;

		pOvlK_GetEventHandle = NULL;

		pOvlK_Wait = NULL;

		pOvlK_WaitOldest = NULL;

		pOvlK_WaitOrCancel = NULL;

		pOvlK_WaitAndRelease = NULL;

		pOvlK_IsComplete = NULL;

		pOvlK_ReUse = NULL;

		pStmK_Init = NULL;

		pStmK_Free = NULL;

		pStmK_Start = NULL;

		pStmK_Stop = NULL;

		pStmK_Read = NULL;

		pStmK_Write = NULL;

		pIsoK_Init = NULL;

		pIsoK_Free = NULL;

		pIsoK_SetPackets = NULL;

		pIsoK_SetPacket = NULL;

		pIsoK_GetPacket = NULL;

		pIsoK_EnumPackets = NULL;

		pIsoK_ReUse = NULL;



		///////////////////////////////////////////////////////////////////////
	}
}

INT LibusbK_DynApi_Init(_inopt LPCSTR DllFullPathName)
{
	LPCSTR dllFullPathName = (DllFullPathName == NULL) ? "libusbK.dll" : DllFullPathName;
	INT funcLoadFailCount = 0;

	if (mLibusbK_ModuleHandle) LibusbK_DynApi_Free();

	if (DllFullPathName)
		mLibusbK_ModuleHandle = mLoadLibraryExA(dllFullPathName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	else
		mLibusbK_ModuleHandle = mLoadLibraryExA(dllFullPathName, NULL, 0);

	if (mLibusbK_ModuleHandle == NULL) return -1;


	// Function loads:

	if ((pLibK_GetVersion = (LibK_GetVersion_T*)GetProcAddress(mLibusbK_ModuleHandle, "LibK_GetVersion")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LibK_GetVersion.\n");
	}

	if ((pLibK_GetContext = (LibK_GetContext_T*)GetProcAddress(mLibusbK_ModuleHandle, "LibK_GetContext")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LibK_GetContext.\n");
	}

	if ((pLibK_SetContext = (LibK_SetContext_T*)GetProcAddress(mLibusbK_ModuleHandle, "LibK_SetContext")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LibK_SetContext.\n");
	}

	if ((pLibK_SetCleanupCallback = (LibK_SetCleanupCallback_T*)GetProcAddress(mLibusbK_ModuleHandle, "LibK_SetCleanupCallback")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LibK_SetCleanupCallback.\n");
	}

	if ((pLibK_LoadDriverAPI = (LibK_LoadDriverAPI_T*)GetProcAddress(mLibusbK_ModuleHandle, "LibK_LoadDriverAPI")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LibK_LoadDriverAPI.\n");
	}

	if ((pLibK_CopyDriverAPI = (LibK_CopyDriverAPI_T*)GetProcAddress(mLibusbK_ModuleHandle, "LibK_CopyDriverAPI")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LibK_CopyDriverAPI.\n");
	}

	if ((pLibK_GetProcAddress = (LibK_GetProcAddress_T*)GetProcAddress(mLibusbK_ModuleHandle, "LibK_GetProcAddress")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LibK_GetProcAddress.\n");
	}

	if ((pLibK_SetDefaultContext = (LibK_SetDefaultContext_T*)GetProcAddress(mLibusbK_ModuleHandle, "LibK_SetDefaultContext")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LibK_SetDefaultContext.\n");
	}

	if ((pLibK_GetDefaultContext = (LibK_GetDefaultContext_T*)GetProcAddress(mLibusbK_ModuleHandle, "LibK_GetDefaultContext")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LibK_GetDefaultContext.\n");
	}

	if ((pLibK_Context_Init = (LibK_Context_Init_T*)GetProcAddress(mLibusbK_ModuleHandle, "LibK_Context_Init")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LibK_Context_Init.\n");
	}

	if ((pLibK_Context_Free = (LibK_Context_Free_T*)GetProcAddress(mLibusbK_ModuleHandle, "LibK_Context_Free")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LibK_Context_Free.\n");
	}

	if ((pUsbK_Init = (UsbK_Init_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_Init")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_Init.\n");
	}

	if ((pUsbK_Free = (UsbK_Free_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_Free")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_Free.\n");
	}

	if ((pUsbK_ClaimInterface = (UsbK_ClaimInterface_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_ClaimInterface")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_ClaimInterface.\n");
	}

	if ((pUsbK_ReleaseInterface = (UsbK_ReleaseInterface_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_ReleaseInterface")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_ReleaseInterface.\n");
	}

	if ((pUsbK_SetAltInterface = (UsbK_SetAltInterface_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_SetAltInterface")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_SetAltInterface.\n");
	}

	if ((pUsbK_GetAltInterface = (UsbK_GetAltInterface_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_GetAltInterface")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_GetAltInterface.\n");
	}

	if ((pUsbK_GetDescriptor = (UsbK_GetDescriptor_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_GetDescriptor")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_GetDescriptor.\n");
	}

	if ((pUsbK_ControlTransfer = (UsbK_ControlTransfer_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_ControlTransfer")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_ControlTransfer.\n");
	}

	if ((pUsbK_SetPowerPolicy = (UsbK_SetPowerPolicy_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_SetPowerPolicy")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_SetPowerPolicy.\n");
	}

	if ((pUsbK_GetPowerPolicy = (UsbK_GetPowerPolicy_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_GetPowerPolicy")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_GetPowerPolicy.\n");
	}

	if ((pUsbK_SetConfiguration = (UsbK_SetConfiguration_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_SetConfiguration")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_SetConfiguration.\n");
	}

	if ((pUsbK_GetConfiguration = (UsbK_GetConfiguration_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_GetConfiguration")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_GetConfiguration.\n");
	}

	if ((pUsbK_ResetDevice = (UsbK_ResetDevice_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_ResetDevice")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_ResetDevice.\n");
	}

	if ((pUsbK_Initialize = (UsbK_Initialize_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_Initialize")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_Initialize.\n");
	}

	if ((pUsbK_SelectInterface = (UsbK_SelectInterface_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_SelectInterface")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_SelectInterface.\n");
	}

	if ((pUsbK_GetAssociatedInterface = (UsbK_GetAssociatedInterface_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_GetAssociatedInterface")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_GetAssociatedInterface.\n");
	}

	if ((pUsbK_Clone = (UsbK_Clone_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_Clone")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_Clone.\n");
	}

	if ((pUsbK_QueryInterfaceSettings = (UsbK_QueryInterfaceSettings_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_QueryInterfaceSettings")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_QueryInterfaceSettings.\n");
	}

	if ((pUsbK_QueryDeviceInformation = (UsbK_QueryDeviceInformation_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_QueryDeviceInformation")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_QueryDeviceInformation.\n");
	}

	if ((pUsbK_SetCurrentAlternateSetting = (UsbK_SetCurrentAlternateSetting_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_SetCurrentAlternateSetting")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_SetCurrentAlternateSetting.\n");
	}

	if ((pUsbK_GetCurrentAlternateSetting = (UsbK_GetCurrentAlternateSetting_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_GetCurrentAlternateSetting")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_GetCurrentAlternateSetting.\n");
	}

	if ((pUsbK_QueryPipe = (UsbK_QueryPipe_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_QueryPipe")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_QueryPipe.\n");
	}

	if ((pUsbK_SetPipePolicy = (UsbK_SetPipePolicy_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_SetPipePolicy")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_SetPipePolicy.\n");
	}

	if ((pUsbK_GetPipePolicy = (UsbK_GetPipePolicy_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_GetPipePolicy")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_GetPipePolicy.\n");
	}

	if ((pUsbK_ReadPipe = (UsbK_ReadPipe_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_ReadPipe")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_ReadPipe.\n");
	}

	if ((pUsbK_WritePipe = (UsbK_WritePipe_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_WritePipe")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_WritePipe.\n");
	}

	if ((pUsbK_ResetPipe = (UsbK_ResetPipe_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_ResetPipe")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_ResetPipe.\n");
	}

	if ((pUsbK_AbortPipe = (UsbK_AbortPipe_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_AbortPipe")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_AbortPipe.\n");
	}

	if ((pUsbK_FlushPipe = (UsbK_FlushPipe_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_FlushPipe")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_FlushPipe.\n");
	}

	if ((pUsbK_IsoReadPipe = (UsbK_IsoReadPipe_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_IsoReadPipe")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_IsoReadPipe.\n");
	}

	if ((pUsbK_IsoWritePipe = (UsbK_IsoWritePipe_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_IsoWritePipe")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_IsoWritePipe.\n");
	}

	if ((pUsbK_GetCurrentFrameNumber = (UsbK_GetCurrentFrameNumber_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_GetCurrentFrameNumber")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_GetCurrentFrameNumber.\n");
	}

	if ((pUsbK_GetOverlappedResult = (UsbK_GetOverlappedResult_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_GetOverlappedResult")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_GetOverlappedResult.\n");
	}

	if ((pUsbK_GetProperty = (UsbK_GetProperty_T*)GetProcAddress(mLibusbK_ModuleHandle, "UsbK_GetProperty")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function UsbK_GetProperty.\n");
	}

	if ((pLstK_Init = (LstK_Init_T*)GetProcAddress(mLibusbK_ModuleHandle, "LstK_Init")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LstK_Init.\n");
	}

	if ((pLstK_InitEx = (LstK_InitEx_T*)GetProcAddress(mLibusbK_ModuleHandle, "LstK_InitEx")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LstK_InitEx.\n");
	}

	if ((pLstK_Free = (LstK_Free_T*)GetProcAddress(mLibusbK_ModuleHandle, "LstK_Free")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LstK_Free.\n");
	}

	if ((pLstK_Enumerate = (LstK_Enumerate_T*)GetProcAddress(mLibusbK_ModuleHandle, "LstK_Enumerate")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LstK_Enumerate.\n");
	}

	if ((pLstK_Current = (LstK_Current_T*)GetProcAddress(mLibusbK_ModuleHandle, "LstK_Current")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LstK_Current.\n");
	}

	if ((pLstK_MoveNext = (LstK_MoveNext_T*)GetProcAddress(mLibusbK_ModuleHandle, "LstK_MoveNext")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LstK_MoveNext.\n");
	}

	if ((pLstK_MoveReset = (LstK_MoveReset_T*)GetProcAddress(mLibusbK_ModuleHandle, "LstK_MoveReset")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LstK_MoveReset.\n");
	}

	if ((pLstK_FindByVidPid = (LstK_FindByVidPid_T*)GetProcAddress(mLibusbK_ModuleHandle, "LstK_FindByVidPid")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LstK_FindByVidPid.\n");
	}

	if ((pLstK_Count = (LstK_Count_T*)GetProcAddress(mLibusbK_ModuleHandle, "LstK_Count")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function LstK_Count.\n");
	}

	if ((pHotK_Init = (HotK_Init_T*)GetProcAddress(mLibusbK_ModuleHandle, "HotK_Init")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function HotK_Init.\n");
	}

	if ((pHotK_Free = (HotK_Free_T*)GetProcAddress(mLibusbK_ModuleHandle, "HotK_Free")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function HotK_Free.\n");
	}

	if ((pHotK_FreeAll = (HotK_FreeAll_T*)GetProcAddress(mLibusbK_ModuleHandle, "HotK_FreeAll")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function HotK_FreeAll.\n");
	}

	if ((pOvlK_Acquire = (OvlK_Acquire_T*)GetProcAddress(mLibusbK_ModuleHandle, "OvlK_Acquire")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function OvlK_Acquire.\n");
	}

	if ((pOvlK_Release = (OvlK_Release_T*)GetProcAddress(mLibusbK_ModuleHandle, "OvlK_Release")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function OvlK_Release.\n");
	}

	if ((pOvlK_Init = (OvlK_Init_T*)GetProcAddress(mLibusbK_ModuleHandle, "OvlK_Init")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function OvlK_Init.\n");
	}

	if ((pOvlK_Free = (OvlK_Free_T*)GetProcAddress(mLibusbK_ModuleHandle, "OvlK_Free")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function OvlK_Free.\n");
	}

	if ((pOvlK_GetEventHandle = (OvlK_GetEventHandle_T*)GetProcAddress(mLibusbK_ModuleHandle, "OvlK_GetEventHandle")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function OvlK_GetEventHandle.\n");
	}

	if ((pOvlK_Wait = (OvlK_Wait_T*)GetProcAddress(mLibusbK_ModuleHandle, "OvlK_Wait")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function OvlK_Wait.\n");
	}

	if ((pOvlK_WaitOldest = (OvlK_WaitOldest_T*)GetProcAddress(mLibusbK_ModuleHandle, "OvlK_WaitOldest")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function OvlK_WaitOldest.\n");
	}

	if ((pOvlK_WaitOrCancel = (OvlK_WaitOrCancel_T*)GetProcAddress(mLibusbK_ModuleHandle, "OvlK_WaitOrCancel")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function OvlK_WaitOrCancel.\n");
	}

	if ((pOvlK_WaitAndRelease = (OvlK_WaitAndRelease_T*)GetProcAddress(mLibusbK_ModuleHandle, "OvlK_WaitAndRelease")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function OvlK_WaitAndRelease.\n");
	}

	if ((pOvlK_IsComplete = (OvlK_IsComplete_T*)GetProcAddress(mLibusbK_ModuleHandle, "OvlK_IsComplete")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function OvlK_IsComplete.\n");
	}

	if ((pOvlK_ReUse = (OvlK_ReUse_T*)GetProcAddress(mLibusbK_ModuleHandle, "OvlK_ReUse")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function OvlK_ReUse.\n");
	}

	if ((pStmK_Init = (StmK_Init_T*)GetProcAddress(mLibusbK_ModuleHandle, "StmK_Init")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function StmK_Init.\n");
	}

	if ((pStmK_Free = (StmK_Free_T*)GetProcAddress(mLibusbK_ModuleHandle, "StmK_Free")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function StmK_Free.\n");
	}

	if ((pStmK_Start = (StmK_Start_T*)GetProcAddress(mLibusbK_ModuleHandle, "StmK_Start")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function StmK_Start.\n");
	}

	if ((pStmK_Stop = (StmK_Stop_T*)GetProcAddress(mLibusbK_ModuleHandle, "StmK_Stop")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function StmK_Stop.\n");
	}

	if ((pStmK_Read = (StmK_Read_T*)GetProcAddress(mLibusbK_ModuleHandle, "StmK_Read")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function StmK_Read.\n");
	}

	if ((pStmK_Write = (StmK_Write_T*)GetProcAddress(mLibusbK_ModuleHandle, "StmK_Write")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function StmK_Write.\n");
	}

	if ((pIsoK_Init = (IsoK_Init_T*)GetProcAddress(mLibusbK_ModuleHandle, "IsoK_Init")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function IsoK_Init.\n");
	}

	if ((pIsoK_Free = (IsoK_Free_T*)GetProcAddress(mLibusbK_ModuleHandle, "IsoK_Free")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function IsoK_Free.\n");
	}

	if ((pIsoK_SetPackets = (IsoK_SetPackets_T*)GetProcAddress(mLibusbK_ModuleHandle, "IsoK_SetPackets")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function IsoK_SetPackets.\n");
	}

	if ((pIsoK_SetPacket = (IsoK_SetPacket_T*)GetProcAddress(mLibusbK_ModuleHandle, "IsoK_SetPacket")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function IsoK_SetPacket.\n");
	}

	if ((pIsoK_GetPacket = (IsoK_GetPacket_T*)GetProcAddress(mLibusbK_ModuleHandle, "IsoK_GetPacket")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function IsoK_GetPacket.\n");
	}

	if ((pIsoK_EnumPackets = (IsoK_EnumPackets_T*)GetProcAddress(mLibusbK_ModuleHandle, "IsoK_EnumPackets")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function IsoK_EnumPackets.\n");
	}

	if ((pIsoK_ReUse = (IsoK_ReUse_T*)GetProcAddress(mLibusbK_ModuleHandle, "IsoK_ReUse")) == NULL)
	{
		funcLoadFailCount++;
		OutputDebugStringA("Failed loading function IsoK_ReUse.\n");
	}



	///////////////////////////////////////////////////////////////////////

	return funcLoadFailCount;
}

// Function wrappers:

KUSB_EXP VOID KUSB_API LibK_GetVersion(_out PKLIB_VERSION Version)
{
	pLibK_GetVersion(Version);
}

KUSB_EXP KLIB_USER_CONTEXT KUSB_API LibK_GetContext(
    _in KLIB_HANDLE Handle,
    _in KLIB_HANDLE_TYPE HandleType)
{
	return pLibK_GetContext(Handle, HandleType);
}

KUSB_EXP BOOL KUSB_API LibK_SetContext(
    _in KLIB_HANDLE Handle,
    _in KLIB_HANDLE_TYPE HandleType,
    _in KLIB_USER_CONTEXT ContextValue)
{
	return pLibK_SetContext(Handle, HandleType, ContextValue);
}

KUSB_EXP BOOL KUSB_API LibK_SetCleanupCallback(
    _in KLIB_HANDLE Handle,
    _in KLIB_HANDLE_TYPE HandleType,
    _in KLIB_HANDLE_CLEANUP_CB* CleanupCB)
{
	return pLibK_SetCleanupCallback(Handle, HandleType, CleanupCB);
}

KUSB_EXP BOOL KUSB_API LibK_LoadDriverAPI(
    _out PKUSB_DRIVER_API DriverAPI,
    _in INT DriverID)
{
	return pLibK_LoadDriverAPI(DriverAPI, DriverID);
}

KUSB_EXP BOOL KUSB_API LibK_CopyDriverAPI(
    _out PKUSB_DRIVER_API DriverAPI,
    _in KUSB_HANDLE UsbHandle)
{
	return pLibK_CopyDriverAPI(DriverAPI, UsbHandle);
}

KUSB_EXP BOOL KUSB_API LibK_GetProcAddress(
    _out KPROC* ProcAddress,
    _in INT DriverID,
    _in INT FunctionID)
{
	return pLibK_GetProcAddress(ProcAddress, DriverID, FunctionID);
}

KUSB_EXP BOOL KUSB_API LibK_SetDefaultContext(
    _in KLIB_HANDLE_TYPE HandleType,
    _in KLIB_USER_CONTEXT ContextValue)
{
	return pLibK_SetDefaultContext(HandleType, ContextValue);
}

KUSB_EXP KLIB_USER_CONTEXT KUSB_API LibK_GetDefaultContext(
    _in KLIB_HANDLE_TYPE HandleType)
{
	return pLibK_GetDefaultContext(HandleType);
}

KUSB_EXP BOOL KUSB_API LibK_Context_Init(
    _inopt HANDLE Heap,
    _in PVOID Reserved)
{
	return pLibK_Context_Init(Heap, Reserved);
}

KUSB_EXP VOID KUSB_API LibK_Context_Free(VOID)
{
	pLibK_Context_Free();
}

KUSB_EXP BOOL KUSB_API UsbK_Init (
    _out KUSB_HANDLE* InterfaceHandle,
    _in KLST_DEVINFO_HANDLE DevInfo)
{
	return pUsbK_Init(InterfaceHandle, DevInfo);
}

KUSB_EXP BOOL KUSB_API UsbK_Free (
    _in KUSB_HANDLE InterfaceHandle)
{
	return pUsbK_Free(InterfaceHandle);
}

KUSB_EXP BOOL KUSB_API UsbK_ClaimInterface (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex)
{
	return pUsbK_ClaimInterface(InterfaceHandle, NumberOrIndex, IsIndex);
}

KUSB_EXP BOOL KUSB_API UsbK_ReleaseInterface (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex)
{
	return pUsbK_ReleaseInterface(InterfaceHandle, NumberOrIndex, IsIndex);
}

KUSB_EXP BOOL KUSB_API UsbK_SetAltInterface (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex,
    _in UCHAR AltSettingNumber)
{
	return pUsbK_SetAltInterface(InterfaceHandle, NumberOrIndex, IsIndex, AltSettingNumber);
}

KUSB_EXP BOOL KUSB_API UsbK_GetAltInterface (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex,
    _out PUCHAR AltSettingNumber)
{
	return pUsbK_GetAltInterface(InterfaceHandle, NumberOrIndex, IsIndex, AltSettingNumber);
}

KUSB_EXP BOOL KUSB_API UsbK_GetDescriptor (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR DescriptorType,
    _in UCHAR Index,
    _in USHORT LanguageID,
    _out PUCHAR Buffer,
    _in UINT BufferLength,
    _outopt PUINT LengthTransferred)
{
	return pUsbK_GetDescriptor(InterfaceHandle, DescriptorType, Index, LanguageID, Buffer, BufferLength, LengthTransferred);
}

KUSB_EXP BOOL KUSB_API UsbK_ControlTransfer (
    _in KUSB_HANDLE InterfaceHandle,
    _in WINUSB_SETUP_PACKET SetupPacket,
    _refopt PUCHAR Buffer,
    _in UINT BufferLength,
    _outopt PUINT LengthTransferred,
    _inopt LPOVERLAPPED Overlapped)
{
	return pUsbK_ControlTransfer(InterfaceHandle, SetupPacket, Buffer, BufferLength, LengthTransferred, Overlapped);
}

KUSB_EXP BOOL KUSB_API UsbK_SetPowerPolicy (
    _in KUSB_HANDLE InterfaceHandle,
    _in UINT PolicyType,
    _in UINT ValueLength,
    _in PVOID Value)
{
	return pUsbK_SetPowerPolicy(InterfaceHandle, PolicyType, ValueLength, Value);
}

KUSB_EXP BOOL KUSB_API UsbK_GetPowerPolicy (
    _in KUSB_HANDLE InterfaceHandle,
    _in UINT PolicyType,
    _ref PUINT ValueLength,
    _out PVOID Value)
{
	return pUsbK_GetPowerPolicy(InterfaceHandle, PolicyType, ValueLength, Value);
}

KUSB_EXP BOOL KUSB_API UsbK_SetConfiguration (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR ConfigurationNumber)
{
	return pUsbK_SetConfiguration(InterfaceHandle, ConfigurationNumber);
}

KUSB_EXP BOOL KUSB_API UsbK_GetConfiguration (
    _in KUSB_HANDLE InterfaceHandle,
    _out PUCHAR ConfigurationNumber)
{
	return pUsbK_GetConfiguration(InterfaceHandle, ConfigurationNumber);
}

KUSB_EXP BOOL KUSB_API UsbK_ResetDevice (
    _in KUSB_HANDLE InterfaceHandle)
{
	return pUsbK_ResetDevice(InterfaceHandle);
}

KUSB_EXP BOOL KUSB_API UsbK_Initialize (
    _in HANDLE DeviceHandle,
    _out KUSB_HANDLE* InterfaceHandle)
{
	return pUsbK_Initialize(DeviceHandle, InterfaceHandle);
}

KUSB_EXP BOOL KUSB_API UsbK_SelectInterface (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex)
{
	return pUsbK_SelectInterface(InterfaceHandle, NumberOrIndex, IsIndex);
}

KUSB_EXP BOOL KUSB_API UsbK_GetAssociatedInterface (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR AssociatedInterfaceIndex,
    _out KUSB_HANDLE* AssociatedInterfaceHandle)
{
	return pUsbK_GetAssociatedInterface(InterfaceHandle, AssociatedInterfaceIndex, AssociatedInterfaceHandle);
}

KUSB_EXP BOOL KUSB_API UsbK_Clone (
    _in KUSB_HANDLE InterfaceHandle,
    _out KUSB_HANDLE* DstInterfaceHandle)
{
	return pUsbK_Clone(InterfaceHandle, DstInterfaceHandle);
}

KUSB_EXP BOOL KUSB_API UsbK_QueryInterfaceSettings (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR AltSettingIndex,
    _out PUSB_INTERFACE_DESCRIPTOR UsbAltInterfaceDescriptor)
{
	return pUsbK_QueryInterfaceSettings(InterfaceHandle, AltSettingIndex, UsbAltInterfaceDescriptor);
}

KUSB_EXP BOOL KUSB_API UsbK_QueryDeviceInformation (
    _in KUSB_HANDLE InterfaceHandle,
    _in UINT InformationType,
    _ref PUINT BufferLength,
    _ref PVOID Buffer)
{
	return pUsbK_QueryDeviceInformation(InterfaceHandle, InformationType, BufferLength, Buffer);
}

KUSB_EXP BOOL KUSB_API UsbK_SetCurrentAlternateSetting (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR AltSettingNumber)
{
	return pUsbK_SetCurrentAlternateSetting(InterfaceHandle, AltSettingNumber);
}

KUSB_EXP BOOL KUSB_API UsbK_GetCurrentAlternateSetting (
    _in KUSB_HANDLE InterfaceHandle,
    _out PUCHAR AltSettingNumber)
{
	return pUsbK_GetCurrentAlternateSetting(InterfaceHandle, AltSettingNumber);
}

KUSB_EXP BOOL KUSB_API UsbK_QueryPipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR AltSettingNumber,
    _in UCHAR PipeIndex,
    _out PWINUSB_PIPE_INFORMATION PipeInformation)
{
	return pUsbK_QueryPipe(InterfaceHandle, AltSettingNumber, PipeIndex, PipeInformation);
}

KUSB_EXP BOOL KUSB_API UsbK_SetPipePolicy (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _in UINT PolicyType,
    _in UINT ValueLength,
    _in PVOID Value)
{
	return pUsbK_SetPipePolicy(InterfaceHandle, PipeID, PolicyType, ValueLength, Value);
}

KUSB_EXP BOOL KUSB_API UsbK_GetPipePolicy (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _in UINT PolicyType,
    _ref PUINT ValueLength,
    _out PVOID Value)
{
	return pUsbK_GetPipePolicy(InterfaceHandle, PipeID, PolicyType, ValueLength, Value);
}

KUSB_EXP BOOL KUSB_API UsbK_ReadPipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _out PUCHAR Buffer,
    _in UINT BufferLength,
    _outopt PUINT LengthTransferred,
    _inopt LPOVERLAPPED Overlapped)
{
	return pUsbK_ReadPipe(InterfaceHandle, PipeID, Buffer, BufferLength, LengthTransferred, Overlapped);
}

KUSB_EXP BOOL KUSB_API UsbK_WritePipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _in PUCHAR Buffer,
    _in UINT BufferLength,
    _outopt PUINT LengthTransferred,
    _inopt LPOVERLAPPED Overlapped)
{
	return pUsbK_WritePipe(InterfaceHandle, PipeID, Buffer, BufferLength, LengthTransferred, Overlapped);
}

KUSB_EXP BOOL KUSB_API UsbK_ResetPipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID)
{
	return pUsbK_ResetPipe(InterfaceHandle, PipeID);
}

KUSB_EXP BOOL KUSB_API UsbK_AbortPipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID)
{
	return pUsbK_AbortPipe(InterfaceHandle, PipeID);
}

KUSB_EXP BOOL KUSB_API UsbK_FlushPipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID)
{
	return pUsbK_FlushPipe(InterfaceHandle, PipeID);
}

KUSB_EXP BOOL KUSB_API UsbK_IsoReadPipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _out PUCHAR Buffer,
    _in UINT BufferLength,
    _in LPOVERLAPPED Overlapped,
    _refopt PKISO_CONTEXT IsoContext)
{
	return pUsbK_IsoReadPipe(InterfaceHandle, PipeID, Buffer, BufferLength, Overlapped, IsoContext);
}

KUSB_EXP BOOL KUSB_API UsbK_IsoWritePipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _in PUCHAR Buffer,
    _in UINT BufferLength,
    _in LPOVERLAPPED Overlapped,
    _refopt PKISO_CONTEXT IsoContext)
{
	return pUsbK_IsoWritePipe(InterfaceHandle, PipeID, Buffer, BufferLength, Overlapped, IsoContext);
}

KUSB_EXP BOOL KUSB_API UsbK_GetCurrentFrameNumber (
    _in KUSB_HANDLE InterfaceHandle,
    _out PUINT FrameNumber)
{
	return pUsbK_GetCurrentFrameNumber(InterfaceHandle, FrameNumber);
}

KUSB_EXP BOOL KUSB_API UsbK_GetOverlappedResult (
    _in KUSB_HANDLE InterfaceHandle,
    _in LPOVERLAPPED Overlapped,
    _out PUINT lpNumberOfBytesTransferred,
    _in BOOL bWait)
{
	return pUsbK_GetOverlappedResult(InterfaceHandle, Overlapped, lpNumberOfBytesTransferred, bWait);
}

KUSB_EXP BOOL KUSB_API UsbK_GetProperty (
    _in KUSB_HANDLE InterfaceHandle,
    _in KUSB_PROPERTY PropertyType,
    _ref PUINT PropertySize,
    _out PVOID Value)
{
	return pUsbK_GetProperty(InterfaceHandle, PropertyType, PropertySize, Value);
}

KUSB_EXP BOOL KUSB_API LstK_Init(
    _out KLST_HANDLE* DeviceList,
    _in KLST_FLAG Flags)
{
	return pLstK_Init(DeviceList, Flags);
}

KUSB_EXP BOOL KUSB_API LstK_InitEx(
    _out KLST_HANDLE* DeviceList,
    _in KLST_FLAG Flags,
    _in PKLST_PATTERN_MATCH PatternMatch)
{
	return pLstK_InitEx(DeviceList, Flags, PatternMatch);
}

KUSB_EXP BOOL KUSB_API LstK_Free(
    _in KLST_HANDLE DeviceList)
{
	return pLstK_Free(DeviceList);
}

KUSB_EXP BOOL KUSB_API LstK_Enumerate(
    _in KLST_HANDLE DeviceList,
    _in KLST_ENUM_DEVINFO_CB* EnumDevListCB,
    _inopt PVOID Context)
{
	return pLstK_Enumerate(DeviceList, EnumDevListCB, Context);
}

KUSB_EXP BOOL KUSB_API LstK_Current(
    _in KLST_HANDLE DeviceList,
    _out KLST_DEVINFO_HANDLE* DeviceInfo)
{
	return pLstK_Current(DeviceList, DeviceInfo);
}

KUSB_EXP BOOL KUSB_API LstK_MoveNext(
    _in KLST_HANDLE DeviceList,
    _outopt KLST_DEVINFO_HANDLE* DeviceInfo)
{
	return pLstK_MoveNext(DeviceList, DeviceInfo);
}

KUSB_EXP VOID KUSB_API LstK_MoveReset(
    _in KLST_HANDLE DeviceList)
{
	pLstK_MoveReset(DeviceList);
}

KUSB_EXP BOOL KUSB_API LstK_FindByVidPid(
    _in KLST_HANDLE DeviceList,
    _in INT Vid,
    _in INT Pid,
    _out KLST_DEVINFO_HANDLE* DeviceInfo)
{
	return pLstK_FindByVidPid(DeviceList, Vid, Pid, DeviceInfo);
}

KUSB_EXP BOOL KUSB_API LstK_Count(
    _in KLST_HANDLE DeviceList,
    _ref PUINT Count)
{
	return pLstK_Count(DeviceList, Count);
}

KUSB_EXP BOOL KUSB_API HotK_Init(
    _out KHOT_HANDLE* Handle,
    _ref PKHOT_PARAMS InitParams)
{
	return pHotK_Init(Handle, InitParams);
}

KUSB_EXP BOOL KUSB_API HotK_Free(
    _in KHOT_HANDLE Handle)
{
	return pHotK_Free(Handle);
}

KUSB_EXP VOID KUSB_API HotK_FreeAll(VOID)
{
	pHotK_FreeAll();
}

KUSB_EXP BOOL KUSB_API OvlK_Acquire(
    _out KOVL_HANDLE* OverlappedK,
    _in KOVL_POOL_HANDLE PoolHandle)
{
	return pOvlK_Acquire(OverlappedK, PoolHandle);
}

KUSB_EXP BOOL KUSB_API OvlK_Release(
    _in KOVL_HANDLE OverlappedK)
{
	return pOvlK_Release(OverlappedK);
}

KUSB_EXP BOOL KUSB_API OvlK_Init (
    _out KOVL_POOL_HANDLE* PoolHandle,
    _in KUSB_HANDLE UsbHandle,
    _in INT MaxOverlappedCount,
    _inopt KOVL_POOL_FLAG Flags)
{
	return pOvlK_Init(PoolHandle, UsbHandle, MaxOverlappedCount, Flags);
}

KUSB_EXP BOOL KUSB_API OvlK_Free(
    _in KOVL_POOL_HANDLE PoolHandle)
{
	return pOvlK_Free(PoolHandle);
}

KUSB_EXP HANDLE KUSB_API OvlK_GetEventHandle(
    _in KOVL_HANDLE OverlappedK)
{
	return pOvlK_GetEventHandle(OverlappedK);
}

KUSB_EXP BOOL KUSB_API OvlK_Wait(
    _in KOVL_HANDLE OverlappedK,
    _inopt INT TimeoutMS,
    _inopt KOVL_WAIT_FLAG WaitFlags,
    _out PUINT TransferredLength)
{
	return pOvlK_Wait(OverlappedK, TimeoutMS, WaitFlags, TransferredLength);
}

KUSB_EXP BOOL KUSB_API OvlK_WaitOldest(
    _in KOVL_POOL_HANDLE PoolHandle,
    _outopt KOVL_HANDLE* OverlappedK,
    _inopt INT TimeoutMS,
    _inopt KOVL_WAIT_FLAG WaitFlags,
    _out PUINT TransferredLength)
{
	return pOvlK_WaitOldest(PoolHandle, OverlappedK, TimeoutMS, WaitFlags, TransferredLength);
}

KUSB_EXP BOOL KUSB_API OvlK_WaitOrCancel(
    _in KOVL_HANDLE OverlappedK,
    _inopt INT TimeoutMS,
    _out PUINT TransferredLength)
{
	return pOvlK_WaitOrCancel(OverlappedK, TimeoutMS, TransferredLength);
}

KUSB_EXP BOOL KUSB_API OvlK_WaitAndRelease(
    _in KOVL_HANDLE OverlappedK,
    _inopt INT TimeoutMS,
    _out PUINT TransferredLength)
{
	return pOvlK_WaitAndRelease(OverlappedK, TimeoutMS, TransferredLength);
}

KUSB_EXP BOOL KUSB_API OvlK_IsComplete(
    _in KOVL_HANDLE OverlappedK)
{
	return pOvlK_IsComplete(OverlappedK);
}

KUSB_EXP BOOL KUSB_API OvlK_ReUse(
    _in KOVL_HANDLE OverlappedK)
{
	return pOvlK_ReUse(OverlappedK);
}

KUSB_EXP BOOL KUSB_API StmK_Init(
    _out KSTM_HANDLE* StreamHandle,
    _in KUSB_HANDLE UsbHandle,
    _in UCHAR PipeID,
    _in INT MaxTransferSize,
    _in INT MaxPendingTransfers,
    _in INT MaxPendingIO,
    _inopt PKSTM_CALLBACK Callbacks,
    _inopt KSTM_FLAG Flags)
{
	return pStmK_Init(StreamHandle, UsbHandle, PipeID, MaxTransferSize, MaxPendingTransfers, MaxPendingIO, Callbacks, Flags);
}

KUSB_EXP BOOL KUSB_API StmK_Free(
    _in KSTM_HANDLE StreamHandle)
{
	return pStmK_Free(StreamHandle);
}

KUSB_EXP BOOL KUSB_API StmK_Start(
    _in KSTM_HANDLE StreamHandle)
{
	return pStmK_Start(StreamHandle);
}

KUSB_EXP BOOL KUSB_API StmK_Stop(
    _in KSTM_HANDLE StreamHandle,
    _in INT TimeoutCancelMS)
{
	return pStmK_Stop(StreamHandle, TimeoutCancelMS);
}

KUSB_EXP BOOL KUSB_API StmK_Read(
    _in KSTM_HANDLE StreamHandle,
    _out PUCHAR Buffer,
    _in INT Offset,
    _in INT Length,
    _out PUINT TransferredLength)
{
	return pStmK_Read(StreamHandle, Buffer, Offset, Length, TransferredLength);
}

KUSB_EXP BOOL KUSB_API StmK_Write(
    _in KSTM_HANDLE StreamHandle,
    _in PUCHAR Buffer,
    _in INT Offset,
    _in INT Length,
    _out PUINT TransferredLength)
{
	return pStmK_Write(StreamHandle, Buffer, Offset, Length, TransferredLength);
}

KUSB_EXP BOOL KUSB_API IsoK_Init(
    _out PKISO_CONTEXT* IsoContext,
    _in INT NumberOfPackets,
    _inopt INT StartFrame)
{
	return pIsoK_Init(IsoContext, NumberOfPackets, StartFrame);
}

KUSB_EXP BOOL KUSB_API IsoK_Free(
    _in PKISO_CONTEXT IsoContext)
{
	return pIsoK_Free(IsoContext);
}

KUSB_EXP BOOL KUSB_API IsoK_SetPackets(
    _in PKISO_CONTEXT IsoContext,
    _in INT PacketSize)
{
	return pIsoK_SetPackets(IsoContext, PacketSize);
}

KUSB_EXP BOOL KUSB_API IsoK_SetPacket(
    _in PKISO_CONTEXT IsoContext,
    _in INT PacketIndex,
    _in PKISO_PACKET IsoPacket)
{
	return pIsoK_SetPacket(IsoContext, PacketIndex, IsoPacket);
}

KUSB_EXP BOOL KUSB_API IsoK_GetPacket(
    _in PKISO_CONTEXT IsoContext,
    _in INT PacketIndex,
    _out PKISO_PACKET IsoPacket)
{
	return pIsoK_GetPacket(IsoContext, PacketIndex, IsoPacket);
}

KUSB_EXP BOOL KUSB_API IsoK_EnumPackets(
    _in PKISO_CONTEXT IsoContext,
    _in KISO_ENUM_PACKETS_CB* EnumPackets,
    _inopt INT StartPacketIndex,
    _inopt PVOID UserState)
{
	return pIsoK_EnumPackets(IsoContext, EnumPackets, StartPacketIndex, UserState);
}

KUSB_EXP BOOL KUSB_API IsoK_ReUse(
    _ref PKISO_CONTEXT IsoContext)
{
	return pIsoK_ReUse(IsoContext);
}



///////////////////////////////////////////////////////////////////////
