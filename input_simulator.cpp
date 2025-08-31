
#include "input_simulator.h"
#include <iostream>

void simulateMouseClick(int x, int y) {
    INPUT input[2];
    ZeroMemory(input, sizeof(input));

    // Mouse move
    input[0].type = INPUT_MOUSE;
    input[0].mi.dx = (x * 65535) / GetSystemMetrics(SM_CXSCREEN); // Convert to normalized absolute coordinates
    input[0].mi.dy = (y * 65535) / GetSystemMetrics(SM_CYSCREEN); // Convert to normalized absolute coordinates
    input[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

    // Mouse left button down
    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    SendInput(2, input, sizeof(INPUT));

    // Mouse left button up
    ZeroMemory(input, sizeof(input));
    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(1, input, sizeof(INPUT));
}

void simulateKeyPress(WORD vkCode) {
    INPUT input[2];
    ZeroMemory(input, sizeof(input));

    // Key down
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = vkCode;
    input[0].ki.dwFlags = 0; // 0 for key press

    // Key up
    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = vkCode;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, input, sizeof(INPUT));
}




bool focusWindow(const std::string& windowTitle) {
    HWND hwnd = FindWindowA(NULL, windowTitle.c_str());
    if (hwnd == NULL) {
        std::cerr << "Error: Window with title \"" << windowTitle << "\" not found.\n";
        return false;
    }

    if (!SetForegroundWindow(hwnd)) {
        std::cerr << "Warning: Could not bring window to foreground.\n";
        // Even if SetForegroundWindow fails, we might still be able to send input
        // if the window is not minimized and is visible.
    }
    return true;
}


