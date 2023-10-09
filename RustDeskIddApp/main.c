#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <tchar.h>

#include "./IddController.h"

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")
#pragma comment(lib, "swdevice.lib")
#pragma comment(lib, "Cfgmgr32.lib")
#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "Newdev.lib")


int prompt_input()
{
    printf("Press  key                  execute:\n");
    printf("       1. 'q'               1. quit\n");
    printf("       2. 'c'               2. create device\n");
    printf("       3. 'd'               3. destroy device\n");
    printf("       4. 'i'               4. install or update driver\n");
    printf("       5. 'u'               5. uninstall driver\n");
    printf("       6. 'a'               6. plug in monitor\n");
    printf("       7. 'b'               7. plug out monitor\n");
    printf("       8. 'r'               8. change resolution\n");
    return _getch();
}

int __cdecl main(int argc, char* argv[])
{
    HSWDEVICE hSwDevice = NULL;
    BOOL bExit = FALSE;

    DWORD width = 1920;
    DWORD height = 1080;
    DWORD sync = 60;

    UINT index = 0;

    TCHAR exePath[1024] = { 0, };
    (void)GetModuleFileName(NULL, exePath, sizeof(exePath)/sizeof(exePath[0]) - 1);
    *_tcsrchr(exePath, _T('\\')) = _T('\0');
    PTCHAR infPath = _T("RustDeskIddDriver\\RustDeskIddDriver.inf");
    TCHAR infFullPath[1024] = { 0, };
    _sntprintf_s(infFullPath, sizeof(infFullPath) / sizeof(infFullPath[0]), _TRUNCATE, _T("%s\\%s"), exePath, infPath);

    do {
        int key = prompt_input();
        BOOL rebootRequired = FALSE;
        switch (key) {
        case 'i':
            printf("Install or update driver begin\n");
            if (!InstallUpdate(infFullPath, &rebootRequired)) {
                printf(GetLastMsg());
            } else {
                printf("Install or update driver done, reboot is %s required\n", (rebootRequired == TRUE ? "" : "not"));
            }
            break;
        case 'u':
            printf("Uninstall driver begin\n");
            if (!Uninstall(infFullPath, &rebootRequired)) {
                printf(GetLastMsg());
            } else {
                printf("Uninstall driver done, reboot is %s required\n", (rebootRequired == TRUE ? "" : "not"));
            }
            break;
        case 'c':
            printf("Create device begin\n");
            if (hSwDevice != NULL) {
                printf("Device created before\n");
                break;
            }
            if (!DeviceCreate(&hSwDevice)) {
                printf(GetLastMsg());
                DeviceClose(hSwDevice);
                hSwDevice = NULL;
            } else {
                printf("Create device done\n");
            }
            break;
        case 'd':
            printf("Close device begin\n");
            DeviceClose(hSwDevice);
            hSwDevice = NULL;
            printf("Close device done\n");
            break;
        case 'a':
            printf("Plug in monitor begin, current index %u\n", index);
            if (!MonitorPlugIn(index, 0, 25)) {
                printf(GetLastMsg());
            } else {
                printf("Plug in monitor done\n");
                MonitorMode modes[2] = { { 1920, 1080,  60 }, { 1024,  768,  60 }, };
                if (!MonitorModesUpdate(index, sizeof(modes)/sizeof(modes[0]), modes)) {
                    printf(GetLastMsg());
                }
                index += 1;
            }
            break;
        case 'b':
            if (index == 0) {
                printf("No virtual monitors\n");
                break;
            }

            printf("Plug out monitor begin, current index %u\n", index - 1);
            if (!MonitorPlugOut(index - 1)) {
                printf(GetLastMsg());
            } else {
                index -= 1;
                printf("Plug out monitor done\n");
            }
            break;
        case 'r': {
            printf("Change resolution, current index %u\n", index - 1);
            MonitorMode modes[] = {{800, 600, 30}, {600, 400, 30}};
            if (!MonitorModesUpdate(index - 1, sizeof(modes) / sizeof(modes[0]), modes)) {
                printf(GetLastMsg());
            } else {
                printf("Change resolution done\n");            
            }
        } break;
        case 'q':
            bExit = TRUE;
            break;
        default:
            break;
        }

        printf("\n\n");
    } while (!bExit);

    if (hSwDevice) {
        SwDeviceClose(hSwDevice);
    }

    return 0;
}
