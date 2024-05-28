//
// Created by Zhiyang Lu on 25/05/2024.
//

#include "../headers/facialrecognision.h"

namespace fs = std::__fs::filesystem;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = dlib::add_prev1<block<N,BN,1,dlib::tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = dlib::add_prev2<dlib::avg_pool<2,2,2,2,dlib::skip1<dlib::tag2<block<N,BN,2,dlib::tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  = BN<dlib::con<N,3,3,1,1,dlib::relu<BN<dlib::con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = dlib::relu<residual<block,N,dlib::affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = dlib::relu<residual_down<block,N,dlib::affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

using anet_type = dlib::loss_metric<dlib::fc_no_bias<128,dlib::avg_pool_everything<
                                             alevel0<
                                                     alevel1<
                                                             alevel2<
                                                                     alevel3<
                                                                             alevel4<
                                                                                     dlib::max_pool<3,3,2,2,dlib::relu<dlib::affine<dlib::con<32,7,7,2,2,
                                                                                     dlib::input_rgb_image_sized<150>
                                                                     >>>>>>>>>>>>;

int faceDescriptorThreshold = 0.315;

bool captureImages(const std::string& userName, const std::string& userDir, int amount, int threadCount) {
    cv::VideoCapture videoCapture(0);
    if (!videoCapture.isOpened()) {
        std::cout << "[Error] Could not open video capture.\n";
        return false;
    }

    int imageCount = 0;
    cv::Mat frame, gray;
    std::vector<cv::Rect> faces;

    while (imageCount < amount) {
        videoCapture >> frame;
        if (frame.empty()) {
            std::cout << "[Error] No captured frame." << std::endl;
            videoCapture.release();
            return false;
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
//            cv::Mat roi = gray(faces[0]);
//            if (roi.empty()) {
//                std::cout << "[Error]: ROI is empty, try again." << std::endl;
//                continue;
//            }
            std::string imgPath = userDir + "/" + userName + "_" + std::to_string(threadCount) + "." + std::to_string(imageCount) + ".png";
            cv::imwrite(imgPath, frame);
            std::cout << "[INFO] Image " << std::to_string(threadCount) + "." + std::to_string(imageCount) << " has been saved in folder: " << userName << "\n";

            std::this_thread::sleep_for(std::chrono::milliseconds (200));
            imageCount++;
        }
    }

    return true;
}

bool generateFaceset(const std::string& userName, int clicks, int amount) {
    std::string userDir = "data/facesets/" + userName;
    fs::create_directories(userDir);

    cv::VideoCapture videoCapture(0);
    if (!videoCapture.isOpened()) {
        std::cout << "[Error] Could not open video capture.\n";
        return false;
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
            videoCapture.release();
            cv::destroyAllWindows();
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
            videoCapture.release();
            cv::destroyAllWindows();
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

std::pair<std::vector<std::string>, int> loadImages(const std::string& directory, std::vector<dlib::matrix<dlib::rgb_pixel>>& faceChips) {
    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
    dlib::shape_predictor sp;
    dlib::deserialize(directoryPath + "/data/dlib_models/shape_predictor_68_face_landmarks.dat") >> sp;

    std::vector<std::string> names;
    int uniqueNames = 0;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_directory(entry) && fs::is_empty(entry)) {
            fs::remove(entry.path());  // Deletes the directory
            std::cout << "[Info] Deleted empty directory: " << entry.path() << "\n";
        } else if (fs::is_directory(entry)) {  // Check if the entry is a directory
            for (const auto& file : fs::directory_iterator(entry.path())) {
                dlib::matrix<dlib::rgb_pixel> img;
                try {
                    load_image(img, file.path().string());  // Attempt to load the image
                    if (img.size() > 0) {  // Check if the image was loaded successfully
                        std::vector<dlib::rectangle> faces = detector(img);

                        if (faces.size() != 1) {
                            std::cout << "[Error] Zero or multiple faces detected\n";
                            continue;
                        }

                        dlib::rectangle face = faces[0];

                        dlib::full_object_detection shape = sp(img, face);
                        dlib::matrix<dlib::rgb_pixel> faceChip;
                        extract_image_chip(img, get_face_chip_details(shape,150,0.25), faceChip);
                        faceChips.push_back(std::move(faceChip)); // Use std::move to avoid copying the image

                        names.push_back(entry.path().filename().string());  // Store the directory name as the label
                    }
                } catch (const dlib::image_load_error& e) {
                    std::cout << "[Error] Failed to load image: " << file.path() << " with error: " << e.what() << "\n";
                    continue;
                }
            }
            uniqueNames++;
        }
    }
    return {names, uniqueNames};
}

bool trainFaceDescriptors() {
    std::vector<dlib::matrix<dlib::rgb_pixel>> faces;
    std::pair<std::vector<std::string>, int> p = loadImages("data/facesets/", faces);
    std::vector<std::string> names = p.first;
    int uniqueNames = p.second;

    if (faces.size() == 0) {
        std::cout << "[ERROR] No images found for training.\n";
        return false;
    }

    anet_type net;
    dlib::deserialize(directoryPath + "/data/dlib_models/dlib_face_recognition_resnet_model_v1.dat") >> net;

    faceDescriptors = net(faces);

    std::vector<dlib::sample_pair> edges;
    for (size_t i = 0; i < faceDescriptors.size(); i++) {
        for (size_t j = i + 1; j < faceDescriptors.size(); j++) {
            std::cout << length(faceDescriptors[i] - faceDescriptors[j]) << "\n";
            if (length(faceDescriptors[i] - faceDescriptors[j]) < faceDescriptorThreshold)
                edges.push_back(dlib::sample_pair(i,j));
        }
    }

    const unsigned long numClusters = chinese_whispers(edges, labels);

    if (numClusters != uniqueNames) {
        std::cout << "[ERROR] Facial recognition training user count mismatch.\n";
    }

    for (int i = 0; i < labels.size(); i++) {
        if (nameMappings.count(labels[i]) && nameMappings[labels[i]] != names[i]) {
            std::cout << "[ERROR] Facial recognition training conflict.\n";
        }
        nameMappings[labels[i]] = names[i];
    }

    std::cout << "[INFO] Training Complete\n";
    return true;
}

std::string findFace(const dlib::matrix<float,0,1>& nfd,
                     const std::vector<dlib::matrix<float,0,1>>& faceDescriptors,
                     const std::vector<unsigned long>& labels,
                     double threshold = faceDescriptorThreshold) {
    for (int i = 0; i < faceDescriptors.size(); i++) {
        const dlib::matrix<float,0,1>& descriptor = faceDescriptors[i];
        std::cout<< length(nfd - descriptor) <<"\n";
        if (length(nfd - descriptor) < threshold) {
            return nameMappings[labels[i]];
        }
    }

    return "";
}

bool authenticate(std::string username) {
    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer = cv::face::LBPHFaceRecognizer::create();
    recognizer->read("data/trained_models/face_classifier.yml");

    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
    dlib::shape_predictor sp;
    dlib::deserialize(directoryPath + "/data/dlib_models/shape_predictor_68_face_landmarks.dat") >> sp;
    anet_type net;
    dlib::deserialize(directoryPath + "/data/dlib_models/dlib_face_recognition_resnet_model_v1.dat") >> net;

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

        dlib::cv_image<dlib::rgb_pixel> img(frame);

        std::vector<dlib::matrix<dlib::rgb_pixel>> dlibFaces;
        for (dlib::rectangle face : detector(img)) {
            dlib::matrix<dlib::rgb_pixel> faceChip;
            auto shape = sp(img, face);
            extract_image_chip(img, get_face_chip_details(shape,150,0.25), faceChip);
            dlibFaces.push_back(std::move(faceChip));
        }

        // Extract descriptors for new faces
        std::vector<dlib::matrix<float,0,1>> newFaceDescriptors = net(dlibFaces);

        // Check each new face against known faces
        for (const dlib::matrix<float,0,1>& nfd : newFaceDescriptors) {
            std::string identity = findFace(nfd, faceDescriptors, labels);
            if (identity == username) {
                videoCapture.release();
                cv::destroyAllWindows();
                return true;
            }
        }

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, cv::Size(200, 200));

        for (const cv::Rect_<int>& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
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