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
#include <dlib/opencv.h>
#include <dlib/dnn.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/image_io.h>

namespace fs = std::__fs::filesystem;
std::string directoryPath = fs::current_path();

cv::CascadeClassifier faceCascade;

std::unordered_set<std::string> currentUsers;
std::unordered_map<int, std::string> nameMappings;

bool generateFaceset(const std::string& userName, int clicks, int amount);
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
        if (!generateFaceset(userName, 5, 10)) {
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

void captureImages(const std::string& userName, const std::string& userDir, int amount, int threadCount) {
    cv::VideoCapture videoCapture(0);
    if (!videoCapture.isOpened()) {
        std::cout << "[Error] Could not open video capture.\n";
        return;
    }

    int imageCount = 0;
    cv::Mat frame, gray;
    std::vector<cv::Rect> faces;

    while (imageCount < amount) {
        videoCapture >> frame;
        if (frame.empty()) {
            std::cout << "[Error] No captured frame." << std::endl;
            return;
        }

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);
        faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, cv::Size(200, 200));

        if (faces.empty()) {
            std::cout << "[Note] No faces detected, please try again.\n";
            continue;
        } else if (faces.size() > 1) {
            std::cout << "[Note] Multiple faces detected, please try again.\n";
            continue;
        } else {
            cv::Mat roi = gray(faces[0]);
            if (roi.empty()) {
                std::cout << "[Error]: ROI is empty, try again." << std::endl;
                continue;
            }
            cv::Mat resizedRoi;
            cv::resize(roi, resizedRoi, cv::Size(150, 150));

            std::string imgPath = userDir + "/" + userName + "_" + std::to_string(threadCount) + "." + std::to_string(imageCount) + ".png";
            cv::imwrite(imgPath, resizedRoi);  // Save the resized ROI
            std::cout << "[INFO] Image " << std::to_string(threadCount) + "." + std::to_string(imageCount) << " has been saved in folder: " << userName << "\n";

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            imageCount++;
        }
    }
}

bool generateFaceset(const std::string& userName, int clicks, int amount) {
    std::string userDir = "facesets/" + userName;
    fs::create_directories(userDir);

    cv::VideoCapture videoCapture(0);
    if (!videoCapture.isOpened()) {
        std::cout << "[Error] Could not open video capture.\n";
        return -1;
    }

    std::cout << "[INFO] Facial registration initiating, please stay still.\n";
    std::cout << "[INFO] Press 's' to take facial capture 5 times.\n";

    int threadCount = 0;
    cv::Mat frame, gray;
    std::vector<cv::Rect> faces;

    std::vector<std::thread> imageCaptureThreads;

    while (threadCount < clicks) {
        videoCapture >> frame;
        if (frame.empty()) {
            std::cout << "[Error] No captured frame." << std::endl;
            return false;
        }

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, cv::Size(200, 200));

        for (const cv::Rect_<int>& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
        }

        cv::imshow("Identified Face", frame);
        char key = (char) cv::waitKey(1);

        if (key == 's') {
            imageCaptureThreads.push_back(std::thread(captureImages, userName, userDir, amount, threadCount));
            threadCount++;
        } else if (key == 'q') {
            fs::remove_all(userDir);
            return false;
        }
    }

    videoCapture.release();
    cv::destroyAllWindows();

    for (std::thread& thread : imageCaptureThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    std::cout << "[INFO] Dataset has been created for " << userName << std::endl;

    return true;
}

using namespace dlib;
using namespace std;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
        alevel0<
                alevel1<
                        alevel2<
                                alevel3<
                                        alevel4<
                                                max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
                                                        input_rgb_image_sized<150>
                                                >>>>>>>>>>>>;
anet_type net;


std::vector<dlib::matrix<dlib::rgb_pixel>> readImages(const std::string& directory, std::vector<double>& labels) {
    std::vector<dlib::matrix<dlib::rgb_pixel>> images;
    int idx = 0;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_directory(entry)) {
            for (const auto& file : fs::directory_iterator(entry.path())) {
                dlib::matrix<dlib::rgb_pixel> img;
                dlib::load_image(img, file.path().string());
                if (img.size() > 0) {
                    nameMappings[idx] = entry.path().filename().string();
                    images.push_back(std::move(img));
                    labels.push_back(idx);
                }
            }
            idx++;
        }
    }
    return images;
}

#include <dlib/svm_threaded.h>


typedef dlib::one_vs_one_trainer<dlib::any_trainer<matrix<float,0,1>>> ovo_trainer;
typedef dlib::one_vs_one_decision_function<ovo_trainer> ovo_df;

typedef normalized_function<
        one_vs_one_decision_function<
                one_vs_one_trainer<any_trainer<matrix<float,0,1>>>, decision_function<ovo_df>
        >> LearnedFunction;

bool trainFaceset() {
    std::vector<double> labels;
    std::vector<matrix<rgb_pixel>> images = readImages("facesets/", labels);

    if (images.empty()) {
        cout << "[ERROR] No images found for training.\n";
        return false;
    }

    cout << "[INFO] Initialising the classifier\n";
    deserialize("data/dlib_models/dlib_face_recognition_resnet_model_v1.dat") >> net;  // Load pre-trained DNN model

    std::vector<matrix<float, 0, 1>> face_descriptors = net(images);  // Compute face descriptors

    ovo_trainer trainer;
    dlib::krr_trainer<radial_basis_kernel<matrix<float,0,1>>> rbf_trainer;
    trainer.set_trainer(rbf_trainer);

//// Train on the loaded face descriptors and corresponding labels
//    ovo_df ovo_func = trainer.train(face_descriptors, labels);
//
//// Save the trained model
//    dlib::serialize("face_recognizer.dat") << ovo_func;

    LearnedFunction learnedFunction;
    learnedFunction.function = trainer.train(face_descriptors, labels);

    serialize("saved_function.dat") << learnedFunction;

    cout << "[INFO] Training Complete\n";

    return true;
}

bool recogniseFaces() {
    deserialize("data/dlib_models/dlib_face_recognition_resnet_model_v1.dat") >> net;  // Load the model again if not globally loaded

    cv::VideoCapture videoCapture(0);
    if (!videoCapture.isOpened()) {
        cout << "[ERROR] Could not open video capture.\n";
        return false;
    }

    frontal_face_detector detector = get_frontal_face_detector();
    shape_predictor sp;
    deserialize("data/dlib_models/shape_predictor_68_face_landmarks.dat") >> sp;  // Load shape predictor

    cv::Mat temp;
    while (videoCapture.read(temp)) {
        cv_image<bgr_pixel> cimg(temp);
        std::vector<rectangle> faces = detector(cimg);
        std::vector<matrix<rgb_pixel>> faces_chip;

        // Extract face chips using the face landmarks and the shape predictor
        for (auto face : faces) {
            full_object_detection shape = sp(cimg, face);  // Get face landmarks
            matrix<rgb_pixel> face_chip;
            extract_image_chip(cimg, get_face_chip_details(shape,150,0.25), face_chip);  // Extract face chip
            faces_chip.push_back(std::move(face_chip));  // Add to face chips vector
        }

        deserialize("data/dlib_models/dlib_face_recognition_resnet_model_v1.dat") >> net;  // Load the recognition model
        ovo_df ovo_func;  // You need to load this from serialized data

        // For each face chip, predict the label
        for (size_t i = 0; i < faces_chip.size(); ++i) {
            matrix<float,0,1> face_descriptor = net(faces_chip[i]);
            int predicted_label = ovo_func(face_descriptor);
            string name = nameMappings[predicted_label];  // Assuming you have a mapping from labels to names

            // Display the name
            cv::rectangle(temp, cv::Rect(faces[i].left(), faces[i].top(), faces[i].width(), faces[i].height()), cv::Scalar(0, 255, 0), 2);
            cv::putText(temp, name, cv::Point(faces[i].left(), faces[i].top() - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
        }

        // Display the frame using OpenCV
        cv::imshow("Face Recognition", temp);
        char key = cv::waitKey(10);
        if (key == 'q' || key == 'Q') {
            break;
        }
    }
    videoCapture.release();
    cv::destroyAllWindows();
    return true;
}
