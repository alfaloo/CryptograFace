//
// Created by Zhiyang Lu on 25/05/2024.
//

#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <thread>

void captureImages(const std::string& userName, const std::string& userDir, int amount, int threadCount);
bool generateFaceset(const std::string& userName, int clicks, int amount);
std::vector<cv::Mat> readImages(const std::string& directory, std::vector<int>& labels);
bool trainFaceset();
bool recogniseFaces();
bool authenticateFace(std::string username, int threshold);

extern cv::CascadeClassifier faceCascade;
extern std::unordered_set<std::string> currentUsers;
extern std::unordered_map<int, std::string> nameMappings;
