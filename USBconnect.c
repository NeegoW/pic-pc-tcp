//////////////////////////////////////////////////////////////////////////////////
// USB HID 関数群
//

//////////////////////////////////////////////////////////////////////////////////
// USB接続開始
#include "USBconnect.h"

BOOL USBConnect(USB *usb) {
    GUID hidGuid;
    HDEVINFO hDevInfo;

    // HID.DLLの呼び出し & 関数のアドレス取得
    if (!myLoadLibrary(usb)) {
        usb->message = "DLL ロード失敗";
        return FALSE;
    }

    // HIDデバイスのGUIDコードを取得
    HidD_GetHidGuid(&hidGuid);

    // HIDクラスのデバイスリストへのハンドルを取得
    hDevInfo = mySetupDiGetClassDevs(usb, hidGuid);                                // HIDクラスのデバイスリストへのハンドルを取得
    if (!hDevInfo) {
        FreeLibrary(usb->hModule);
        usb->message = "デバイスリストハンドル 取得失敗";
        return FALSE;
    }

    // 対象デバイス検索
    if (SearchDevice(usb, hidGuid, hDevInfo)) {
        // HIDディバイス能力取得
        GetBufferSize(usb);
        return TRUE;
    } else {
        usb->message = "デバイス 接続失敗";
        return FALSE;
    }
}


//////////////////////////////////////////////////////////////////////////////////
// 送受信用 メモリ確保

BOOL USBmemset(USB *usb) {
    usb->SendBuf = malloc(usb->OutputReportByteLength);
    if (usb->SendBuf == NULL) {
        usb->message = "メモリが確保できませんでした．";
        return FALSE;
    }

    usb->RecvBuf = malloc(usb->InputReportByteLength);
    if (usb->RecvBuf == NULL) {
        usb->message = "メモリが確保できませんでした．";
        return FALSE;
    }

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////
// 送受

DWORD USBRead(USB *usb) {
    DWORD dwSize;

    ReadFile(usb->hDevHandle, usb->RecvBuf, usb->InputReportByteLength, &dwSize, NULL);

    return dwSize;
}


//////////////////////////////////////////////////////////////////////////////////
// 送信

void USBWrite(USB usb) {
    DWORD dwSize;

    WriteFile(usb.hDevHandle, usb.SendBuf, usb.OutputReportByteLength, &dwSize, NULL);
}


//////////////////////////////////////////////////////////////////////////////////
// USB接続終了

void USBDisConnect(USB usb) {
    free(usb.SendBuf);                                                            // メモリ開放
    free(usb.RecvBuf);

    if (usb.hDevHandle != NULL) {
        CloseHandle(usb.hDevHandle);                                            // ハンドルを閉じる
    }

    if (usb.hModule != NULL) {
        FreeLibrary(usb.hModule);                                                // HID.DLL開放
    }
}


//////////////////////////////////////////////////////////////////////////////////
// HID.DLLの呼び出し & 関数のアドレス取得

BOOL myLoadLibrary(USB *usb) {
    /* HID.dll の呼び出し */
    usb->hModule = LoadLibrary("HID.DLL");
    if (usb->hModule == NULL) {
        usb->message = "HID.DLLが見つかりませんでした．";
        return FALSE;
    }

    /* DLLの関数のアドレスを取得 */
    HidD_GetHidGuid = (HIDD_GETHIDGUID *) GetProcAddress(usb->hModule, "HidD_GetHidGuid");
    HidD_GetAttributes = (HIDD_GETATTRIBUTES *) GetProcAddress(usb->hModule, "HidD_GetAttributes");
    HidP_GetCaps = (HIDP_GETCAPS *) GetProcAddress(usb->hModule, "HidP_GetCaps");
    HidD_GetPreparsedData = (HIDD_GETPREPARSEDDATA *) GetProcAddress(usb->hModule, "HidD_GetPreparsedData");

    if (HidD_GetHidGuid == NULL || HidD_GetAttributes == NULL || HidP_GetCaps == NULL ||
        HidD_GetPreparsedData == NULL) {
        usb->message = "関数の呼び出しに失敗しました．";
        FreeLibrary(usb->hModule);
        return FALSE;
    }

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
// HIDクラスのデバイスリストへのハンドルを取得

HDEVINFO mySetupDiGetClassDevs(USB *usb, GUID HidGuid) {
    HDEVINFO hDevInfo;

    hDevInfo = SetupDiGetClassDevs(
            &HidGuid,                                                            // インターフェイスクラスのクラスGUIDへのポインタ
            NULL,                                                                // NULL:すべてのデバイスインスタンスに関するデバイス情報を取得
            NULL,                                                                // トップレベルウィンドウのハンドルを指定
            DIGCF_DEVICEINTERFACE |
            // ClassGuidで指定されたインターフェイスクラスに所属するインターフェイス
            DIGCF_PRESENT                                                        // 現在存在するデバイス
    );

    if (!hDevInfo) {
        usb->message = " HIDクラスのデバイスリストへのハンドルを取得できませんでした．";
        return 0;
    }

    return hDevInfo;
}


//////////////////////////////////////////////////////////////////////////////////
// 対象デバイス検索

BOOL SearchDevice(USB *usb, GUID hidGuid, HDEVINFO hDevInfo) {
    DWORD dwIndex;
    BOOL bRes;
    SP_DEVICE_INTERFACE_DATA spDid;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pspDidd;
    DWORD dwRequiredLength;
    HANDLE hDevHandle;
    HIDD_ATTRIBUTES Attributes;

    dwIndex = 0;
    while (TRUE) {
        //////////////////////////////////////////////////////////////////////////////
        // HIDクラスのデバイスの情報を取得

        memset(&spDid, 0, sizeof(SP_DEVICE_INTERFACE_DATA));
        spDid.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        bRes = SetupDiEnumDeviceInterfaces(
                hDevInfo,
                NULL,
                &hidGuid,
                dwIndex,
                &spDid
        );

        if (bRes == TRUE) {
            //////////////////////////////////////////////////////////////////////////////
            // Windows内部ディバイス名を取得

            //デバイス詳細用の長さを取得
            dwRequiredLength = 0;
            SetupDiGetDeviceInterfaceDetail(
                    hDevInfo,
                    &spDid,
                    NULL,
                    0,
                    &dwRequiredLength,
                    NULL
            );

            // デバイス詳細用メモリ確保
            pspDidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(dwRequiredLength);
            pspDidd->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            //デバイスの詳細を取得
            bRes = SetupDiGetDeviceInterfaceDetail(
                    hDevInfo,
                    &spDid,
                    pspDidd,
                    dwRequiredLength,
                    &dwRequiredLength,
                    NULL
            );

            if (bRes == TRUE) {
                //////////////////////////////////////////////////////////////////////////////
                // HIDにアクセスする為ファイルを作成しハンドルを取得

                hDevHandle = CreateFile(
                        pspDidd->DevicePath,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL
                );

                if (hDevHandle != INVALID_HANDLE_VALUE) {
                    //////////////////////////////////////////////////////////////////////////////
                    // HIDデバイスの属性を取得

                    Attributes.Size = sizeof(Attributes);
                    bRes = HidD_GetAttributes(
                            hDevHandle,
                            &Attributes
                    );

                    if (bRes != FALSE) {
                        if (Attributes.VendorID == usb->VendorID && Attributes.ProductID == usb->ProductID) {
                            usb->hDevHandle = hDevHandle;
                            free(pspDidd);
                            SetupDiDestroyDeviceInfoList(hDevInfo);
                            return TRUE;
                        }
                    }

                    CloseHandle(hDevHandle);                                    // ハンドルを閉じる
                }
            }

            free(pspDidd);                                                        // デバイス詳細用メモリ確保
            dwIndex++;
        } else {
            if (GetLastError() == ERROR_NO_MORE_ITEMS) {
                break;
            }
        }
    }

    usb->message = "デバイスは見つかりませんでした．";
    SetupDiDestroyDeviceInfoList(hDevInfo);

    return FALSE;
}


//////////////////////////////////////////////////////////////////////////////////
// HIDディバイス能力取得

void GetBufferSize(USB *usb) {

    PHIDP_PREPARSED_DATA ppd;
    HIDP_CAPS caps;

    HidD_GetPreparsedData(usb->hDevHandle, &ppd);
    HidP_GetCaps(ppd, &caps);

    usb->OutputReportByteLength = caps.OutputReportByteLength;
    usb->InputReportByteLength = caps.InputReportByteLength;
}


