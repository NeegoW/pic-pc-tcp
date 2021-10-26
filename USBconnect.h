//////////////////////////////////////////////////////////////////////////////////
// USB HID ヘッダーファイル
//

#ifndef __USBCONNECT_H_INCLUDED_
#define __USBCONNECT_H_INCLUDED_

#include <windows.h>
#include <stdio.h>
#include <setupapi.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "user32.lib")


typedef struct _USB
{
	HMODULE hModule;
	HANDLE hDevHandle;
	USHORT VendorID;
	USHORT ProductID;
	USHORT InputReportByteLength;
	USHORT OutputReportByteLength;
	BYTE   *SendBuf;
	BYTE   *RecvBuf;
	char   *message;
} USB;

typedef struct _HIDD_ATTRIBUTES
{
	ULONG   Size;
	USHORT  VendorID;
	USHORT  ProductID;
	USHORT  VersionNumber;
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

typedef struct _HIDP_PREPARSED_DATA * PHIDP_PREPARSED_DATA;

typedef USHORT USAGE, *PUSAGE;

typedef struct _HIDP_CAPS
{
	USAGE    Usage;
	USAGE    UsagePage;
	USHORT   InputReportByteLength;
	USHORT   OutputReportByteLength;
	USHORT   FeatureReportByteLength;
	USHORT   Reserved[17];
	
	USHORT   NumberLinkCollectionNodes;
	
	USHORT   NumberInputButtonCaps;
	USHORT   NumberInputValueCaps;
	USHORT   NumberInputDataIndices;
	
	USHORT   NumberOutputButtonCaps;
	USHORT   NumberOutputValueCaps;
	USHORT   NumberOutputDataIndices;
	
	USHORT   NumberFeatureButtonCaps;
	USHORT   NumberFeatureValueCaps;
	USHORT   NumberFeatureDataIndices;
} HIDP_CAPS, *PHIDP_CAPS;


typedef BOOL __stdcall HIDD_GETHIDGUID(GUID *pGuid);

typedef BOOL __stdcall HIDD_GETATTRIBUTES(HANDLE HidDeviceObject, PHIDD_ATTRIBUTES Attributes);

typedef BOOL __stdcall HIDP_GETCAPS(PHIDP_PREPARSED_DATA PreparsedData, PHIDP_CAPS Capabilities);

typedef BOOL __stdcall HIDD_GETPREPARSEDDATA(HANDLE HidDeviceObject, PHIDP_PREPARSED_DATA *PreparsedData);


HIDD_GETHIDGUID *HidD_GetHidGuid;
HIDD_GETATTRIBUTES *HidD_GetAttributes;
HIDP_GETCAPS *HidP_GetCaps;
HIDD_GETPREPARSEDDATA *HidD_GetPreparsedData;

BOOL USBConnect(USB *usb);
BOOL USBmemset(USB *usb);
DWORD USBRead(USB *usb);
void USBWrite(USB usb);
void USBDisConnect(USB usb);

BOOL myLoadLibrary(USB *usb);
HDEVINFO mySetupDiGetClassDevs(USB *usb, GUID HidGuid);
BOOL SearchDevice(USB *usb, GUID hidGuid, HDEVINFO hDevInfo);
void GetBufferSize(USB *usb);


#endif

//////////////////////////////////////////////////////////////////////////////////
