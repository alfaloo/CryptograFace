//
// Created by Zhiyang Lu on 25/05/2024.
//

#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <dlib/dnn.h>
#include <dlib/clustering.h>
#include <dlib/string.h>
#include <dlib/image_io.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <thread>

bool generateFaceset(const std::string& userName, int clicks, int amount);
bool trainFaceDescriptors();
bool authenticate(std::string username);

extern std::string directoryPath;
extern cv::CascadeClassifier faceCascade;
extern std::unordered_set<std::string> currentUsers;
extern std::unordered_map<unsigned long, std::string> nameMappings;
extern std::vector<dlib::matrix<float,0,1>> faceDescriptors;
extern std::vector<unsigned long> labels;
