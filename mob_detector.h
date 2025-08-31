#ifndef MOB_DETECTOR_H
#define MOB_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <map>

// Function to detect mobs in a given screenshot
std::vector<cv::Rect> detectMobs(const cv::Mat& screenshot, const std::map<std::string, cv::Mat>& mobTemplates, double threshold = 0.8);

#endif // MOB_DETECTOR_H


