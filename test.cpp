#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

int main() {
    // Load the facial landmark predictor
    std::string model_path = "data/dlib_models/shape_predictor_68_face_landmarks.dat";
    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
    dlib::shape_predictor predictor;
    dlib::deserialize(model_path) >> predictor;

    // Initialize video capture
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "[Error] Could not open video capture.\n";
        return -1;
    }

    // Set up display window
    cv::namedWindow("Output", cv::WINDOW_NORMAL);

    while (true) {
        // Capture frame from webcam
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "[Error] No captured frame.\n";
            break;
        }

        // Convert to grayscale
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // Detect faces
        dlib::cv_image<dlib::bgr_pixel> dlib_frame(frame);
        std::vector<dlib::rectangle> faces = detector(dlib_frame);

        // Loop over the face detections
        for (const auto& face : faces) {
            // Get the landmarks/parts for the face
            dlib::full_object_detection shape = predictor(dlib_frame, face);

            // Draw the landmarks on the image
            for (int i = 0; i < shape.num_parts(); ++i) {
                cv::circle(frame, cv::Point(shape.part(i).x(), shape.part(i).y()), 2, cv::Scalar(0, 255, 0), -1);
            }
        }

        // Display the output image
        cv::imshow("Output", frame);

        // Break the loop on 'ESC' key press
        char key = (char)cv::waitKey(1); // Reduce wait time
        if (key == 27) {
            break;
        }
    }

    // Cleanup
    cv::destroyAllWindows();
    cap.release();

    return 0;
}
