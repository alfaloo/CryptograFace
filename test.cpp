#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>

// Convert OpenCV BGR image to dlib's image format
dlib::cv_image<dlib::bgr_pixel> cimg(frame);

dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
dlib::shape_predictor sp;
dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> sp;

// Detect faces
std::vector<dlib::rectangle> faces = detector(cimg);

// Now process each face we found.
for (auto& face : faces)
{
// Find the pose of each face.
dlib::full_object_detection shape = sp(cimg, face);
dlib::matrix<dlib::rgb_pixel> face_chip;
dlib::extract_image_chip(cimg, dlib::get_face_chip_details(shape,150,0.25), face_chip);

// Convert back to cv::Mat and grayscale
cv::Mat alignedFace = dlib::toMat(face_chip);
cv::cvtColor(alignedFace, gray, cv::COLOR_BGR2GRAY);
cv::equalizeHist(gray, gray);  // Histogram Equalization

// Save or use the aligned and normalized image
cv::imwrite(userDir + "/face_" + std::to_string(count) + ".jpg", gray);
count++;
}
