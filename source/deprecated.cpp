#include "../headers/facialrecognision.h"

namespace fs = std::__fs::filesystem;

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
    std::vector<cv::Mat> images = readImages("data/facesets/", labels);

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
        return false;
    }

    cv::Mat frame;
    std::cout << "[Info] Starting facial recognition, press 'q' to quit." << std::endl;

    while (videoCapture.read(frame)) {
        if (frame.empty()) {
            std::cout << "[Error] No captured frame." << std::endl;
            videoCapture.release();
            cv::destroyAllWindows();
            return false;
        }

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, cv::Size(200, 200));

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

bool authenticateFace(std::string username, int threshold) {
    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer = cv::face::LBPHFaceRecognizer::create();
    recognizer->read("data/trained_models/face_classifier.yml");

    cv::VideoCapture videoCapture(0);
    if (!videoCapture.isOpened()) {
        std::cout << "[Error] Could not open video capture.\n";
        return false;
    }

    cv::Mat frame;
    std::cout << "[Info] Starting facial recognition, press 'q' to quit." << std::endl;

    while (videoCapture.read(frame)) {
        if (frame.empty()) {
            std::cout << "[Error] No captured frame." << std::endl;
            videoCapture.release();
            cv::destroyAllWindows();
            return false;
        }

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, cv::Size(200, 200));

        for (const cv::Rect_<int>& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
            cv::Mat faceROI = gray(face);
            int label;
            double confidence;
            recognizer->predict(faceROI, label, confidence);
            std::cout << confidence << "\n";
            std::string text = "";

            if (label >= 0 && label <= nameMappings.size()) {
                text = nameMappings[label];
            }

            if (text == username && confidence < threshold) {
                videoCapture.release();
                cv::destroyAllWindows();
                return true;
            }
        }

        cv::imshow("Recognise", frame);
        if (cv::waitKey(10) == 'q') {
            break;
        }
    }

    videoCapture.release();
    cv::destroyAllWindows();
    return false;
}