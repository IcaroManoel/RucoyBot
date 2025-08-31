
#include "screen_capture.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <iostream>

cv::Mat captureScreen() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Error: Could not open X display\n";
        return cv::Mat();
    }

    Window root = DefaultRootWindow(display);

    XWindowAttributes attributes;
    XGetWindowAttributes(display, root, &attributes);

    int width = attributes.width;
    int height = attributes.height;

    XImage* ximage = XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);
    if (!ximage) {
        std::cerr << "Error: Could not get XImage\n";
        XCloseDisplay(display);
        return cv::Mat();
    }

    cv::Mat img(height, width, CV_8UC4, ximage->data);
    cv::Mat converted_img;
    cv::cvtColor(img, converted_img, cv::COLOR_BGRA2BGR);

    XDestroyImage(ximage);
    XCloseDisplay(display);

    return converted_img;
}


