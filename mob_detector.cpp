
#include "mob_detector.h"
#include <iostream>

std::vector<cv::Rect> detectMobs(const cv::Mat& screenshot, const std::map<std::string, cv::Mat>& mobTemplates, double threshold) {
    std::vector<cv::Rect> detectedMobs;

    for (const auto& pair : mobTemplates) {
        const std::string& mobName = pair.first;
        const cv::Mat& mobTemplate = pair.second;

        if (mobTemplate.empty()) {
            std::cerr << "Error: Mob template for " << mobName << " is empty.\n";
            continue;
        }
        if (screenshot.empty()) {
            std::cerr << "Error: Screenshot is empty.\n";
            return detectedMobs;
        }
        if (screenshot.cols < mobTemplate.cols || screenshot.rows < mobTemplate.rows) {
            std::cerr << "Error: Screenshot is smaller than mob template for " << mobName << ".\n";
            continue;
        }

        cv::Mat result;
        cv::matchTemplate(screenshot, mobTemplate, result, cv::TM_CCOEFF_NORMED);

        double minVal, maxVal;
        cv::Point minLoc, maxLoc;

        while (true) {
            cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

            if (maxVal >= threshold) {
                cv::Rect mobRect(maxLoc.x, maxLoc.y, mobTemplate.cols, mobTemplate.rows);
                detectedMobs.push_back(mobRect);

                // Fill the detected area with black to avoid re-detecting the same mob
                cv::rectangle(result, cv::Point(maxLoc.x, maxLoc.y), cv::Point(maxLoc.x + mobTemplate.cols, maxLoc.y + mobTemplate.rows), cv::Scalar(0), -1);
            } else {
                break;
            }
        }
    }
    return detectedMobs;
}


