#ifndef INPUT_SIMULATOR_H
#define INPUT_SIMULATOR_H

#include <windows.h>
#include <string>

// Function to simulate a mouse click at a specific coordinate
void simulateMouseClick(int x, int y);

// Function to simulate a key press
void simulateKeyPress(WORD vkCode);

#endif // INPUT_SIMULATOR_H




// Function to find a window by its title and bring it to the foreground
bool focusWindow(const std::string& windowTitle);


