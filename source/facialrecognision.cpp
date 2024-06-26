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

double faceDescriptorThreshold = 0.315;

FacialAuthenticator::FacialAuthenticator()
        : keyPressed("")
        , logger(nullptr)
        , directoryPath(fs::current_path())
        , faceCascade()
        , currentUsers()
        , nameMappings()
        , faceDescriptors()
        , labels() {
    if (!faceCascade.load(directoryPath + "/data/haarcascades/haarcascade_frontalface_alt.xml")) {
        logInfo("[ERROR] Could not load face cascade.");
        return;
    }
}

void FacialAuthenticator::addLogger(QTextEdit *qTextEdit) {
    logger = qTextEdit;
}

void FacialAuthenticator::logInfo(std::string info) {
    std::cout << info << "\n";

    if (!logger) return;

    QString qString = QString::fromStdString(info);
    logger->append(qString);

    logger->moveCursor(QTextCursor::End);
    logger->ensureCursorVisible();
}

void FacialAuthenticator::uploadUsers(std::string path) {
    for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath + path)) {
        if (fs::is_directory(entry) && fs::is_empty(entry)) {
            fs::remove(entry.path());  // Deletes the directory
            logInfo("[INFO] Deleted empty directory: " + entry.path().string());
        } else if (fs::is_directory(entry)) {
            currentUsers.insert(entry.path().filename().string());
        }
    }
}

bool FacialAuthenticator::userExists(std::string username) {
    return currentUsers.count(username) != 0;
}

bool FacialAuthenticator::captureImages(const std::string& username, const std::string& userDir, int amount, int threadCount) {
    cv::VideoCapture videoCapture(0);
    if (!videoCapture.isOpened()) {
        logInfo("[ERROR] Could not open video capture.");
        return false;
    }

    int imageCount = 0;
    cv::Mat frame, gray;
    std::vector<cv::Rect> faces;

    while (imageCount < amount) {
        videoCapture >> frame;
        if (frame.empty()) {
            logInfo("[ERROR] No captured frame.");
            videoCapture.release();
            return false;
        }

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, cv::Size(200, 200));

        if (faces.empty()) {
            logInfo("[Warning] No faces detected, please try again.");
            continue;
        } else if (faces.size() > 1) {
            logInfo("[Warning] Multiple faces detected, please try again.");
            continue;
        } else {
            std::string imgPath = userDir + "/" + username + "_" + std::to_string(threadCount) + "." + std::to_string(imageCount) + ".png";
            cv::imwrite(imgPath, frame);
            logInfo("[INFO] Image " + std::to_string(threadCount) + "." + std::to_string(imageCount) + " has been saved in folder: " + username);

            std::this_thread::sleep_for(std::chrono::milliseconds (500));
            imageCount++;
        }
    }

    return true;
}

bool FacialAuthenticator::generateFaceset(const std::string& username, int clicks, int amount, bool showCamera) {
    logInfo("[INFO] New user detected, facial registration initiating.");

    std::string userDir = "data/facesets/" + username;
    fs::create_directories(userDir);

    cv::VideoCapture videoCapture(0);
    if (!videoCapture.isOpened()) {
        logInfo("[ERROR] Could not open video capture.");
        return false;
    }

    logInfo("[INSTRUCTION] Press 's' to take facial capture.");

    int threadCount = 0;
    cv::Mat frame, gray;
    std::vector<cv::Rect> faces;

    std::queue<std::future<bool>> imageCaptureThreads;
    bool allCompleted = false;

    keyPressed = "";

    while (!allCompleted) {
        if (threadCount >= clicks) {
            while (imageCaptureThreads.size() > 0) {
                if (imageCaptureThreads.front().wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    std::future<bool> fut = std::move(imageCaptureThreads.front());
                    imageCaptureThreads.pop();
                    try {
                        if (fut.get()) {
                            allCompleted = true;
                        } else {
                            logInfo("[ERROR] Image capture future failed to complete.");
                            allCompleted = false;
                            threadCount--;
                            break;
                        }
                    } catch (const std::exception& e) {
                        logInfo(std::string("[ERROR] Exception from image capture future: ") + e.what());
                        allCompleted = false;
                        threadCount--;
                        break;
                    }
                } else {
                    allCompleted = false;
                    break;
                }
            }
            if (allCompleted) {
                break;
            }
        }

        videoCapture >> frame;
        if (frame.empty()) {
            logInfo("[ERROR] No captured frame.");
            videoCapture.release();
            cv::destroyAllWindows();
            return false;
        }

        if (showCamera) {
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, cv::Size(200, 200));

            for (const cv::Rect_<int>& face : faces) {
                cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
            }

            cv::imshow("Identified Face", frame);
            keyPressed = (char) cv::waitKey(1);
        }

        if (threadCount < clicks && keyPressed == "s") {
            keyPressed = "";
            std::future<bool> future = std::async(
                std::launch::async,
                [this, &username, &userDir, &amount, &threadCount]() { return this->captureImages(username, userDir, amount, threadCount); }
            );
            imageCaptureThreads.push(std::move(future));
            threadCount++;
        } else if (keyPressed == "q") {
            fs::remove_all(userDir);
            videoCapture.release();
            cv::destroyAllWindows();
            logInfo("[ERROR] Could not generate facial data.");
            return false;
        }
    }

    videoCapture.release();
    cv::destroyAllWindows();

    logInfo("[INFO] Dataset has been created for " + username);

    return true;
}

std::pair<std::vector<std::string>, int> FacialAuthenticator::loadImages(const std::string& directory, std::vector<dlib::matrix<dlib::rgb_pixel>>& faceChips) {
    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
    dlib::shape_predictor sp;
    dlib::deserialize(directoryPath + "/data/dlib_models/shape_predictor_68_face_landmarks.dat") >> sp;

    std::vector<std::string> names;
    int uniqueNames = 0;
    for (const fs::directory_entry& entry : fs::directory_iterator(directory)) {
        if (fs::is_directory(entry) && fs::is_empty(entry)) {
            fs::remove(entry.path());  // Deletes the directory
            logInfo("[INFO] Deleted empty directory: " + entry.path().string());
        } else if (fs::is_directory(entry)) {  // Check if the entry is a directory
            for (const fs::directory_entry& file : fs::directory_iterator(entry.path())) {
                dlib::matrix<dlib::rgb_pixel> img;
                try {
                    load_image(img, file.path().string());  // Attempt to load the image
                    if (img.size() > 0) {  // Check if the image was loaded successfully
                        std::vector<dlib::rectangle> faces = detector(img);

                        if (faces.size() != 1) {
                            logInfo("[ERROR] Zero or multiple faces detected");
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
                    logInfo("[ERROR] Failed to load image: " + file.path().string() + " with error: " + e.what());
                    continue;
                }
            }
            uniqueNames++;
        }
    }
    return {names, uniqueNames};
}

bool FacialAuthenticator::trainFaceDescriptors() {
    std::vector<dlib::matrix<dlib::rgb_pixel>> faces;
    std::pair<std::vector<std::string>, int> p = loadImages("data/facesets/", faces);
    std::vector<std::string> names = p.first;
    int uniqueNames = p.second;

    if (faces.size() == 0) {
        logInfo("[ERROR] No images found for training.");
        return false;
    }

    anet_type net;
    dlib::deserialize(directoryPath + "/data/dlib_models/dlib_face_recognition_resnet_model_v1.dat") >> net;

    faceDescriptors = net(faces);

    std::vector<dlib::sample_pair> edges;
    for (size_t i = 0; i < faceDescriptors.size(); i++) {
        for (size_t j = i + 1; j < faceDescriptors.size(); j++) {
            if (length(faceDescriptors[i] - faceDescriptors[j]) < faceDescriptorThreshold)
                edges.push_back(dlib::sample_pair(i,j));
        }
    }

    const unsigned long numClusters = chinese_whispers(edges, labels);

    if (numClusters != uniqueNames) {
        logInfo("[ERROR] Facial recognition training user count mismatch.");
    }

    for (int i = 0; i < labels.size(); i++) {
        if (nameMappings.count(labels[i]) && nameMappings[labels[i]] != names[i]) {
            logInfo("[ERROR] Facial recognition training conflict.");
        }
        nameMappings[labels[i]] = names[i];
    }

    logInfo("[INFO] Training Complete");
    return true;
}

std::string FacialAuthenticator::findFace(const dlib::matrix<float,0,1>& nfd,
                     const std::vector<dlib::matrix<float,0,1>>& faceDescriptors,
                     const std::vector<unsigned long>& labels) {
    for (int i = 0; i < faceDescriptors.size(); i++) {
        const dlib::matrix<float,0,1>& descriptor = faceDescriptors[i];
        if (length(nfd - descriptor) < faceDescriptorThreshold) {
            return nameMappings[labels[i]];
        }
    }

    return "";
}

bool FacialAuthenticator::authenticate(std::string username, bool showCamera, bool updateFaceset) {
    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer = cv::face::LBPHFaceRecognizer::create();
    recognizer->read("data/trained_models/face_classifier.yml");

    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
    dlib::shape_predictor sp;
    dlib::deserialize(directoryPath + "/data/dlib_models/shape_predictor_68_face_landmarks.dat") >> sp;
    anet_type net;
    dlib::deserialize(directoryPath + "/data/dlib_models/dlib_face_recognition_resnet_model_v1.dat") >> net;

    cv::VideoCapture videoCapture(0);
    if (!videoCapture.isOpened()) {
        logInfo("[ERROR] Could not open video capture.");
        return false;
    }

    cv::Mat frame;
    logInfo("[INSTRUCTION] Starting facial recognition, press 'q' to quit.");
    keyPressed = "";

    while (videoCapture.read(frame)) {
        if (frame.empty()) {
            logInfo("[ERROR] No captured frame.");
            videoCapture.release();
            cv::destroyAllWindows();
            return false;
        }

        dlib::cv_image<dlib::rgb_pixel> img(frame);

        std::vector<dlib::matrix<dlib::rgb_pixel>> dlibFaces;
        for (dlib::rectangle face : detector(img)) {
            dlib::matrix<dlib::rgb_pixel> faceChip;
            dlib::full_object_detection shape = sp(img, face);
            extract_image_chip(img, get_face_chip_details(shape,150,0.25), faceChip);
            dlibFaces.push_back(std::move(faceChip));
        }

        // Extract descriptors for new faces
        std::vector<dlib::matrix<float,0,1>> newFaceDescriptors = net(dlibFaces);

        // Check each new face against known faces
        for (const dlib::matrix<float,0,1>& nfd : newFaceDescriptors) {
            std::string identity = findFace(nfd, faceDescriptors, labels);
            if (identity == username) {
                if (updateFaceset) {
                    cv::Mat gray;
                    std::vector<cv::Rect> faces;

                    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
                    faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, cv::Size(200, 200));

                    if (faces.size() == 1) {
                        std::string userDir = "data/facesets/" + username;

                        std::vector<fs::path> facesetPaths;
                        for (const fs::directory_entry& entry : fs::directory_iterator(userDir)) {
                            if (fs::is_regular_file(entry)) {
                                facesetPaths.push_back(entry.path());
                            }
                        }

                        srand(time(nullptr));
                        int index = rand() % facesetPaths.size();
                        fs::path path = facesetPaths[index];
                        fs::remove(path);

                        std::string imgPath = path.string();
                        cv::imwrite(imgPath, frame);
                        logInfo("[INFO] Image " + path.filename().string() + " has been updated.");
                    }
                }

                videoCapture.release();
                cv::destroyAllWindows();
                return true;
            }
        }

        if (showCamera) {
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            std::vector<cv::Rect> faces;
            faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, cv::Size(200, 200));

            for (const cv::Rect_<int>& face : faces) {
                cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
            }

            cv::imshow("Recognise", frame);
            keyPressed = (char) cv::waitKey(1);
        }

        if (keyPressed == "q") {
            break;
        }
    }

    videoCapture.release();
    cv::destroyAllWindows();
    logInfo("[ERROR] Could not identify user.");

    return false;
}