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
#include <queue>
#include <thread>
#include <future>
#include <QTextEdit>

namespace fs = std::__fs::filesystem;

class FacialAuthenticator {
public:
    FacialAuthenticator();
    ~FacialAuthenticator() = default;

    bool generateFaceset(const std::string& userName, int clicks, int amount, bool showCamera);

    bool trainFaceDescriptors();

    bool authenticate(std::string username, bool showCamera);

    void addLogger(QTextEdit *qTextEdit);

    void uploadUsers(std::string path);

    bool userExists(std::string username);

    std::string keyPressed;

private:
    QTextEdit *logger;
    std::string directoryPath;
    cv::CascadeClassifier faceCascade;
    std::unordered_set<std::string> currentUsers;
    std::unordered_map<unsigned long, std::string> nameMappings;
    std::vector<dlib::matrix<float,0,1>> faceDescriptors;
    std::vector<unsigned long> labels;

    void logInfo(std::string info);

    bool captureImages(const std::string& userName, const std::string& userDir, int amount, int threadCount);

    std::pair<std::vector<std::string>, int> loadImages(const std::string& directory,
                                                        std::vector<dlib::matrix<dlib::rgb_pixel>>& faceChips);

    std::string findFace(const dlib::matrix<float,0,1>& nfd,
                         const std::vector<dlib::matrix<float,0,1>>& faceDescriptors,
                         const std::vector<unsigned long>& labels);
};