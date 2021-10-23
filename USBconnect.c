//////////////////////////////////////////////////////////////////////////////////
// USB HID �֐��Q
//

//////////////////////////////////////////////////////////////////////////////////
// USB�ڑ��J�n
#include "USBconnect.h"

BOOL USBConnect(USB *usb) {
    GUID hidGuid;
    HDEVINFO hDevInfo;

    // HID.DLL�̌Ăяo�� & �֐��̃A�h���X�擾
    if (!myLoadLibrary(usb)) {
        usb->message = "DLL ���[�h���s";
        return FALSE;
    }

    // HID�f�o�C�X��GUID�R�[�h���擾
    HidD_GetHidGuid(&hidGuid);

    // HID�N���X�̃f�o�C�X���X�g�ւ̃n���h�����擾
    hDevInfo = mySetupDiGetClassDevs(usb,
                                     hidGuid);                                // HID�N���X�̃f�o�C�X���X�g�ւ̃n���h�����擾
    if (!hDevInfo) {
        FreeLibrary(usb->hModule);
        usb->message = "�f�o�C�X���X�g�n���h�� �擾���s";
        return FALSE;
    }

    // �Ώۃf�o�C�X����
    if (SearchDevice(usb, hidGuid, hDevInfo)) {
        // HID�f�B�o�C�X�\�͎擾
        GetBufferSize(usb);
        return TRUE;
    } else {
        usb->message = "�f�o�C�X �ڑ����s";
        return FALSE;
    }
}


//////////////////////////////////////////////////////////////////////////////////
// ����M�p �������m��

BOOL USBmemset(USB *usb) {
    usb->SendBuf = malloc(usb->OutputReportByteLength);
    if (usb->SendBuf == NULL) {
        usb->message = "���������m�ۂł��܂���ł����D";
        return FALSE;
    }

    usb->RecvBuf = malloc(usb->InputReportByteLength);
    if (usb->RecvBuf == NULL) {
        usb->message = "���������m�ۂł��܂���ł����D";
        return FALSE;
    }

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////
// ����

DWORD USBRead(USB *usb) {
    DWORD dwSize;

    ReadFile(usb->hDevHandle, usb->RecvBuf, usb->InputReportByteLength, &dwSize, NULL);

    return dwSize;
}


//////////////////////////////////////////////////////////////////////////////////
// ���M

void USBWrite(USB usb) {
    DWORD dwSize;

    WriteFile(usb.hDevHandle, usb.SendBuf, usb.OutputReportByteLength, &dwSize, NULL);
}


//////////////////////////////////////////////////////////////////////////////////
// USB�ڑ��I��

void USBDisConnect(USB usb) {
    free(usb.SendBuf);                                                            // �������J��
    free(usb.RecvBuf);

    if (usb.hDevHandle != NULL) {
        CloseHandle(usb.hDevHandle);                                            // �n���h�������
    }

    if (usb.hModule != NULL) {
        FreeLibrary(usb.hModule);                                                // HID.DLL�J��
    }
}


//////////////////////////////////////////////////////////////////////////////////
// HID.DLL�̌Ăяo�� & �֐��̃A�h���X�擾

BOOL myLoadLibrary(USB *usb) {
    /* HID.dll �̌Ăяo�� */
    usb->hModule = LoadLibrary("HID.DLL");
    if (usb->hModule == NULL) {
        usb->message = "HID.DLL��������܂���ł����D";
        return FALSE;
    }

    /* DLL�̊֐��̃A�h���X���擾 */
    HidD_GetHidGuid = (HIDD_GETHIDGUID *) GetProcAddress(usb->hModule, "HidD_GetHidGuid");
    HidD_GetAttributes = (HIDD_GETATTRIBUTES *) GetProcAddress(usb->hModule, "HidD_GetAttributes");
    HidP_GetCaps = (HIDP_GETCAPS *) GetProcAddress(usb->hModule, "HidP_GetCaps");
    HidD_GetPreparsedData = (HIDD_GETPREPARSEDDATA *) GetProcAddress(usb->hModule, "HidD_GetPreparsedData");

    if (HidD_GetHidGuid == NULL || HidD_GetAttributes == NULL || HidP_GetCaps == NULL ||
        HidD_GetPreparsedData == NULL) {
        usb->message = "�֐��̌Ăяo���Ɏ��s���܂����D";
        FreeLibrary(usb->hModule);
        return FALSE;
    }

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
// HID�N���X�̃f�o�C�X���X�g�ւ̃n���h�����擾

HDEVINFO mySetupDiGetClassDevs(USB *usb, GUID HidGuid) {
    HDEVINFO hDevInfo;

    hDevInfo = SetupDiGetClassDevs(
            &HidGuid,                                                            // �C���^�[�t�F�C�X�N���X�̃N���XGUID�ւ̃|�C���^
            NULL,                                                                // NULL:���ׂẴf�o�C�X�C���X�^���X�Ɋւ���f�o�C�X�����擾
            NULL,                                                                // �g�b�v���x���E�B���h�E�̃n���h�����w��
            DIGCF_DEVICEINTERFACE |
            // ClassGuid�Ŏw�肳�ꂽ�C���^�[�t�F�C�X�N���X�ɏ�������C���^�[�t�F�C�X
            DIGCF_PRESENT                                                        // ���ݑ��݂���f�o�C�X
    );

    if (!hDevInfo) {
        usb->message = " HID�N���X�̃f�o�C�X���X�g�ւ̃n���h�����擾�ł��܂���ł����D";
        return 0;
    }

    return hDevInfo;
}


//////////////////////////////////////////////////////////////////////////////////
// �Ώۃf�o�C�X����

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
        // HID�N���X�̃f�o�C�X�̏����擾

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
            // Windows�����f�B�o�C�X�����擾

            //�f�o�C�X�ڍחp�̒������擾
            dwRequiredLength = 0;
            SetupDiGetDeviceInterfaceDetail(
                    hDevInfo,
                    &spDid,
                    NULL,
                    0,
                    &dwRequiredLength,
                    NULL
            );

            // �f�o�C�X�ڍחp�������m��
            pspDidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(dwRequiredLength);
            pspDidd->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            //�f�o�C�X�̏ڍׂ��擾
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
                // HID�ɃA�N�Z�X����׃t�@�C�����쐬���n���h�����擾

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
                    // HID�f�o�C�X�̑������擾

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

                    CloseHandle(hDevHandle);                                    // �n���h�������
                }
            }

            free(pspDidd);                                                        // �f�o�C�X�ڍחp�������m��
            dwIndex++;
        } else {
            if (GetLastError() == ERROR_NO_MORE_ITEMS) {
                break;
            }
        }
    }

    usb->message = "�f�o�C�X�͌�����܂���ł����D";
    SetupDiDestroyDeviceInfoList(hDevInfo);

    return FALSE;
}


//////////////////////////////////////////////////////////////////////////////////
// HID�f�B�o�C�X�\�͎擾

void GetBufferSize(USB *usb) {

    PHIDP_PREPARSED_DATA ppd;
    HIDP_CAPS caps;

    HidD_GetPreparsedData(usb->hDevHandle, &ppd);
    HidP_GetCaps(ppd, &caps);

    usb->OutputReportByteLength = caps.OutputReportByteLength;
    usb->InputReportByteLength = caps.InputReportByteLength;
}


