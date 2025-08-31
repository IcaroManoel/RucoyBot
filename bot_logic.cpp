
#include "bot_logic.h"
#include "screen_capture_windows.h"
#include "mob_detector.h"
#include "input_simulator.h"
#include <iostream>
#include <windows.h> // For Sleep function
#include <limits> // For numeric_limits
#include <chrono> // For high_resolution_clock

// Global flag to control bot execution from GUI
bool g_bot_running = false;

// Function to find the closest mob
cv::Rect findClosestMob(const std::vector<cv::Rect>& mobs, int screenCenterX, int screenCenterY) {
    if (mobs.empty()) {
        return cv::Rect();
    }

    double minDistance = std::numeric_limits<double>::max();
    cv::Rect closestMob;

    for (const auto& rect : mobs) {
        int mobCenterX = rect.x + rect.width / 2;
        int mobCenterY = rect.y + rect.height / 2;
        double distance = std::sqrt(std::pow(mobCenterX - screenCenterX, 2) + std::pow(mobCenterY - screenCenterY, 2));

        if (distance < minDistance) {
            minDistance = distance;
            closestMob = rect;
        }
    }
    return closestMob;
}

void runBotLogic(const std::string& gameWindowTitle, const std::map<std::string, bool>& mobEnabledStatus) {
    // Load mob templates based on configuration
    std::map<std::string, cv::Mat> mobTemplates;
    if (mobEnabledStatus.count("Minotaur") && mobEnabledStatus.at("Minotaur")) {
        mobTemplates["Minotaur"] = cv::imread("mob_references/minotaur_ingame.png", cv::IMREAD_COLOR);
    }
    if (mobEnabledStatus.count("Gargoyle") && mobEnabledStatus.at("Gargoyle")) {
        mobTemplates["Gargoyle"] = cv::imread("mob_references/gargoyle_ingame.png", cv::IMREAD_COLOR);
    }
    if (mobEnabledStatus.count("LizardWarrior") && mobEnabledStatus.at("LizardWarrior")) {
        mobTemplates["LizardWarrior"] = cv::imread("mob_references/lizard_warrior_ingame.png", cv::IMREAD_COLOR);
    }
    if (mobEnabledStatus.count("Djinn") && mobEnabledStatus.at("Djinn")) {
        mobTemplates["Djinn"] = cv::imread("mob_references/djinn.png", cv::IMREAD_COLOR); // Assuming Djinn sprite is available
    }

    if (mobTemplates.empty()) {
        std::cout << "No mobs configured to attack. Please check config.ini or GUI settings." << std::endl;
        g_bot_running = false; // Stop bot if no mobs are configured
        return;
    }

    for (const auto& pair : mobTemplates) {
        if (pair.second.empty()) {
            std::cout << "Error: Could not load mob template: " << pair.first << std::endl;
            g_bot_running = false; // Stop bot if a template fails to load
            return;
        }
    }

    std::cout << "Attempting to focus on window: " << gameWindowTitle << std::endl;
    if (!focusWindow(gameWindowTitle)) {
        std::cout << "Error: Could not find or focus Rucoy Online window. Make sure the game is running and the window title is correct." << std::endl;
        g_bot_running = false; // Stop bot if window not found
        return;
    }
    Sleep(1000); // Give some time for the window to focus

    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    int screenCenterX = screen_width / 2;
    int screenCenterY = screen_height / 2;

    auto lastActionTime = std::chrono::high_resolution_clock::now();

    while (g_bot_running) { // Main bot loop controlled by global flag
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastActionTime;

        if (elapsed.count() < 2.0) { // Limit actions to every 2 seconds
            Sleep(100); // Small sleep to avoid busy-waiting
            continue;
        }
        lastActionTime = currentTime;

        std::cout << "\n--- Bot Status ---" << std::endl;
        std::cout << "Capturing screen..." << std::endl;
        cv::Mat screenshot = captureScreenWindows();

        if (screenshot.empty()) {
            std::cout << "Error: Failed to capture screen. Retrying..." << std::endl;
            Sleep(1000);
            continue;
        }

        std::cout << "Screen captured. Detecting mobs..." << std::endl;
        std::vector<cv::Rect> detectedMobs = detectMobs(screenshot, mobTemplates, 0.8);

        if (!detectedMobs.empty()) {
            cv::Rect targetMobRect = findClosestMob(detectedMobs, screenCenterX, screenCenterY);
            if (targetMobRect.width > 0) {
                std::cout << "Detected " << detectedMobs.size() << " mobs. Attacking closest one..." << std::endl;
                int centerX = targetMobRect.x + targetMobRect.width / 2;
                int centerY = targetMobRect.y + targetMobRect.height / 2;
                simulateMouseClick(centerX, centerY);
                std::cout << "Action: Attacked mob at (" << centerX << ", " << centerY << ")" << std::endl;
                Sleep(500);
                std::cout << "Status: Mob attacked. Assuming it might be exhausted, looking for next..." << std::endl;
            }
        } else {
            std::cout << "Status: No mobs detected. Searching for mobs..." << std::endl;
            simulateKeyPress(VK_RIGHT);
            Sleep(500);
            simulateKeyPress(VK_UP);
            Sleep(500);
            std::cout << "Action: Simulating movement to find new mobs." << std::endl;
        }
    }
}


