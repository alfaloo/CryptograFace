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

namespace fs = std::__fs::filesystem;
std::string directoryPath = fs::current_path();

cv::CascadeClassifier faceCascade;

std::unordered_set<std::string> currentUsers;
std::unordered_map<int, std::string> nameMappings;

bool generateFaceset(const std::string& userName);
bool trainFaceset();
bool recogniseFaces();

int main() {

    if (!faceCascade.load(directoryPath + "/data/haarcascades/haarcascade_frontalface_alt.xml")) {
        std::cout << "[Error] Could not load face cascade.\n";
        return -1;
    }

    for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath + "/facesets")) {
        if (fs::is_directory(entry)) {
            currentUsers.insert(entry.path().filename().string());
        }
    }

    std::string userName;
    std::cout << "Please enter your name: ";
    std::getline(std::cin, userName);

    if (currentUsers.count(userName) == 0) {
        if (!generateFaceset(userName)) {
            std::cout << "[Error] Could not generate facial data.\n";
            return -1;
        }
    }

    if (!trainFaceset()) {
        std::cout << "[Error] Could not train facial data.\n";
        return -1;
    }

    recogniseFaces();

    return 0;
}

bool generateFaceset(const std::string& userName) {
    std::string userDir = "facesets/" + userName;
    fs::create_directories(userDir);

    int count = 0;
    cv::Mat frame, gray;
    std::vector<cv::Rect> faces;

    cv::VideoCapture videoCapture(0);
    if (!videoCapture.isOpened()) {
        std::cout << "[Error] Could not open video capture.\n";
        return -1;
    }

    std::cout << "[INFO] Facial registration initiating, please stay still.\n";
    std::cout << "[INFO] Press 's' to take facial capture 5 times.\n";

    while (count < 5) {
        videoCapture >> frame;
        if (frame.empty()) {
            std::cout << "[Error] No captured frame." << std::endl;
            return false;
        }

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, cv::Size(50, 50));

        for (const cv::Rect_<int>& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
        }

        cv::imshow("Identified Face", frame);
        char key = (char) cv::waitKey(1);

        if (key == 's' && count < 5) {
            if (faces.empty()) {
                std::cout << "[Note] No faces detected, please try again.\n";
                continue;
            } else if (faces.size() > 1) {
                std::cout << "[Note] Multiple faces detected, please try again.\n";
                continue;
            } else {
                cv::Mat roi = frame(faces[0]); // Ensure at least one face has been detected.
                std::string imgPath = userDir + "/" + userName + "_" + std::to_string(count) + ".png";
                if (roi.empty()) {
                    std::cout << "[Error]: ROI is empty, try again." << std::endl;
                    continue;
                }
                cv::imwrite(imgPath, roi);
                std::cout << "[INFO] Image " << count << " has been saved in folder: " << userName << "\n";
                count++;
            }
        } else if (key == 'q') {
            fs::remove_all(userDir);
            return false;
        }
    }

    std::cout << "[INFO] Dataset has been created for " << userName << std::endl;

    videoCapture.release();
    cv::destroyAllWindows();

    return true;
}

std::vector<cv::Mat> readImages(const std::string& directory, std::vector<int>& labels) {
    std::vector<cv::Mat> images;
    int idx = 0;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_directory(entry)) {  // Check if the entry is a directory
            for (const auto& file : fs::directory_iterator(entry.path())) {
                cv::Mat img = cv::imread(file.path().string(), cv::IMREAD_GRAYSCALE);
                if (!img.empty()) {
                    nameMappings[idx] = entry.path().filename().string();
                    images.push_back(img);
                    labels.push_back(idx);
                }
            }
            idx++;
        }
    }
    return images;
}

bool trainFaceset() {
    std::vector<int> labels;
    std::vector<cv::Mat> images = readImages("facesets/", labels);

    if (images.size() > 0) {
        std::cout << "[INFO] Initialising the classifier\n";

        cv::Ptr<cv::face::LBPHFaceRecognizer> model = cv::face::LBPHFaceRecognizer::create();
        model->train(images, labels);

        model->save("data/trained_models/face_classifier.yml");
        std::cout << "[INFO] Training Complete\n";
    } else {
        std::cout << "[ERROR] No images found for training.\n";
        return false;
    }

    return true;
}

bool recogniseFaces() {
    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer = cv::face::LBPHFaceRecognizer::create();
    recognizer->read("data/trained_models/face_classifier.yml");

    cv::VideoCapture videoCapture(0);
    if (!videoCapture.isOpened()) {
        std::cout << "[Error] Could not open video capture.\n";
        return -1;
    }

    cv::Mat frame;
    std::cout << "[Info] Starting facial recognition, press 'q' to quit." << std::endl;

    while (videoCapture.read(frame)) {
        if (frame.empty()) {
            std::cout << "[Error] No captured frame." << std::endl;
            return false;
        }

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.2, 5, 0, cv::Size(100, 100));

        for (const cv::Rect_<int>& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
            cv::Mat faceROI = gray(face);
            int label;
            double confidence;
            recognizer->predict(faceROI, label, confidence);
            std::cout << confidence << "\n";
            std::string text = "Unknown";

            if (label >= 0 && label <= nameMappings.size()) {
                text = nameMappings[label];
            }

            cv::putText(frame, text, cv::Point(face.x, face.y - 4), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
        }

        cv::imshow("Recognise", frame);
        if (cv::waitKey(10) == 'q') {
            break;
        }
    }

    videoCapture.release();
    cv::destroyAllWindows();
    return true;
}