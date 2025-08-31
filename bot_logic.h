
#ifndef BOT_LOGIC_H
#define BOT_LOGIC_H

#include <string>
#include <map>
#include <opencv2/opencv.hpp>
#include "config_parser.h"

// Function to run the main bot logic
void runBotLogic(const std::string& gameWindowTitle, const std::map<std::string, bool>& mobEnabledStatus);

// Global flag to control bot execution from GUI
extern bool g_bot_running;

#endif // BOT_LOGIC_H


