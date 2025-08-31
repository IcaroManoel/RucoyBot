
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#include "config_parser.h"
#include "bot_logic.h"

#include <d3d11.h> // For DirectX 11, if you choose this backend
#include <tchar.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <mutex>
#include <thread>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Data structure to hold log messages
struct Log {
    ImGuiTextBuffer     Buf;
    ImVector<int>       LineOffsets;        // Index to lines offset.
    bool                AutoScroll;         // Keep scrolling if already at the bottom.

    Log() {
        AutoScroll = true;
        Clear();
    }

    void    Clear()     { Buf.clear(); LineOffsets.clear(); LineOffsets.push_back(0); }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size + 1);
    }

    void    Draw(const char* title, bool* p_open = NULL)
    {
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }
        if (ImGui::Button("Clear")) Clear();
        ImGui::SameLine();
        bool copy_to_clipboard = ImGui::Button("Copy");
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &AutoScroll);
        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);
        if (copy_to_clipboard)
            ImGui::LogToClipboard();

        ImGui::TextUnformatted(Buf.begin());

        if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::End();
    }
};

// Global log instance
Log     g_log;

// Global mutex for thread-safe logging
std::mutex g_log_mutex;

// Redirect cout to ImGui log
class ImGuiStreamBuf : public std::stringbuf {
public:
    int sync() override {
        std::lock_guard<std::mutex> lock(g_log_mutex);
        g_log.AddLog("%s", str().c_str());
        str(""); // Clear the buffer
        return 0;
    }
};

ImGuiStreamBuf g_imgui_streambuf;
std::ostream g_cout_redirect(&g_imgui_streambuf);

// Global variables for bot state and configuration
bool botRunning = false;
std::string gameWindowTitle = "Rucoy Online";
ConfigParser* configParser = nullptr;
std::map<std::string, bool> mobEnabledStatus;

// Main window procedure
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_SIZE:
            if (ImGui_ImplOpenGL3_WndProcHandler(hWnd, msg, wParam, lParam)) // If using OpenGL
                return true;
            break;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Redirect std::cout to ImGui log
    std::cout.rdbuf(g_cout_redirect.rdbuf());

    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Rucoy Bot GUI"), NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, _T("Rucoy Bot GUI"), WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, wc.hInstance, NULL);

    // Initialize Dear ImGui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends (using OpenGL for example)
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL3_Init("#version 130"); // Adjust OpenGL version as needed

    // Load configuration
    configParser = new ConfigParser("config.ini");
    if (configParser->parse()) {
        // Populate mobEnabledStatus from config
        // This part needs to be dynamic based on your actual mob list
        // For now, hardcode based on config.ini example
        mobEnabledStatus["Minotaur"] = configParser->getBoolValue("Mobs", "Minotaur", false);
        mobEnabledStatus["Gargoyle"] = configParser->getBoolValue("Mobs", "Gargoyle", false);
        mobEnabledStatus["LizardWarrior"] = configParser->getBoolValue("Mobs", "LizardWarrior", false);
        mobEnabledStatus["Djinn"] = configParser->getBoolValue("Mobs", "Djinn", false);
    } else {
        g_log.AddLog("Error: Could not load config.ini. Using default settings.\n");
    }

    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Your GUI code goes here
        ImGui::Begin("Rucoy Bot Control");

        // Bot Control Section
        if (ImGui::Button(botRunning ? "Stop Bot" : "Start Bot")) {
            botRunning = !botRunning;
            g_bot_running = botRunning; // Update global flag in bot_logic.cpp
            if (botRunning) {
                g_log.AddLog("Bot started.\n");
                // Start bot logic in a new thread, passing current settings
                std::thread(runBotLogic, gameWindowTitle, mobEnabledStatus).detach();
            } else {
                g_log.AddLog("Bot stopped.\n");
            }
        }
        ImGui::SameLine();
        ImGui::Text("Status: %s", botRunning ? "Running" : "Stopped");

        ImGui::InputText("Game Window Title", &gameWindowTitle[0], gameWindowTitle.size() + 1, ImGuiInputTextFlags_CallbackResize, [](ImGuiInputTextCallbackData* data){ ((std::string*)data->UserData)->assign((char*)data->Buf, data->BufTextLen); return 0; }, &gameWindowTitle);

        ImGui::Separator();

        // Mob Configuration Section
        ImGui::Text("Mob Configuration:");
        for (auto& pair : mobEnabledStatus) {
            ImGui::Checkbox(pair.first.c_str(), &pair.second);
        }
        if (ImGui::Button("Save Configuration")) {
            // Update configParser with current mobEnabledStatus
            for (const auto& pair : mobEnabledStatus) {
                configParser->config_["Mobs"][pair.first] = pair.second ? "true" : "false";
            }
            if (configParser->save()) {
                g_log.AddLog("Configuration saved to config.ini\n");
            } else {
                g_log.AddLog("Error: Failed to save configuration.\n");
            }
        }

        ImGui::End();

        // Log Window
        g_log.Draw("Bot Log");

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Present your backbuffer (this part depends on your graphics API setup)
        // For OpenGL, you might use SwapBuffers(hdc);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    delete configParser;

    return 0;
}


