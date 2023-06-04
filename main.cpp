#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>

// Windows hook event constants
#define WH_KEYBOARD_LL 13

HHOOK keyboardHook;

bool KillProcessByName(const char *szProcessToKill)
{
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    if (hSnapShot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof(pEntry);
    bool bRet = false;

    if(Process32First(hSnapShot, &pEntry)) {
        do {
            if(!strcmp(pEntry.szExeFile, szProcessToKill)) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pEntry.th32ProcessID);
                if(hProcess != NULL) {
                    TerminateProcess(hProcess, 9);
                    CloseHandle(hProcess);
                    bRet = true;
                    break;
                }
            }
        } while(Process32Next(hSnapShot, &pEntry));
    }

    CloseHandle(hSnapShot);
    return bRet;
}

void RunSteamURL(const char *szURL)
{
    ShellExecute(0, 0, szURL, 0, 0, SW_SHOW);
}


// The callback
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // If this keydown event is prior to others and is valid
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* pKbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        // Print the virtual key code
        // std::cout << "Virtual key code: " << pKbdStruct->vkCode << std::endl;
        if (pKbdStruct->vkCode == VK_F5) {
            std::cout << "A key pressed" << std::endl;
            if(KillProcessByName("HITMAN3.exe")) {
                std::cout << "Process killed successfully." << std::endl;
                RunSteamURL("steam://run/1659040");
            } else {
                std::cout << "Failed to kill process." << std::endl;
            }
        }
    }
    // Call the next hook in the hook chain
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

void Start() {
    // Set up a keyboard hook
    keyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        LowLevelKeyboardProc,
        GetModuleHandle(NULL),
        0);

    if (keyboardHook == NULL) {
        std::cout << "Failed to install hook!" << std::endl;
        return;
    }
    
    // Message loop to keep the program running
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) != 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unhook and exit
    UnhookWindowsHookEx(keyboardHook);
}

int main() {
    Start();
    return 0;
}
