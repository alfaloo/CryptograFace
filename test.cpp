// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*
    This is an example illustrating the use of the deep learning tools from the dlib C++
    Library.  In it, we will show how to do face recognition.  This example uses the
    pretrained dlib_face_recognition_resnet_model_v1 model which is freely available from
    the dlib web site.  This model has a 99.38% accuracy on the standard LFW face
    recognition benchmark, which is comparable to other state-of-the-art methods for face
    recognition as of February 2017.

    In this example, we will use dlib to do face clustering.  Included in the examples
    folder is an image, bald_guys.jpg, which contains a bunch of photos of action movie
    stars Vin Diesel, The Rock, aJason Statham, and Bruce Willis.   We will use dlib to
    automatically find their faces in the image and then to automatically determine how
    many people there are (4 in this case) as well as which faces belong to each person.

    Finally, this example uses a network with the loss_metric loss.  Therefore, if you want
    to learn how to train your own models, or to get a general introduction to this loss
    layer, you should read the dnn_metric_learning_ex.cpp and
    dnn_metric_learning_on_images_ex.cpp examples.
*/

#include <dlib/dnn.h>
#include <dlib/clustering.h>
#include <dlib/string.h>
#include <dlib/image_io.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

// Function to convert a vector of Dlib images to a single OpenCV Mat
cv::Mat tile_images(std::vector<dlib::matrix<dlib::rgb_pixel>>& images) {
    if (images.empty()) return cv::Mat();

    int image_size = 150; // Assume each face image is 150x150
    int tiles_per_row = std::sqrt(images.size());
    int num_rows = (int)std::ceil(images.size() / (double)tiles_per_row);
    cv::Mat tiled_image(num_rows * image_size, tiles_per_row * image_size, CV_8UC3, cv::Scalar(0, 0, 0));

    for (size_t i = 0; i < images.size(); ++i) {
        cv::Mat temp = dlib::toMat(images[i]);
        cv::Mat resized_temp;
        cv::resize(temp, resized_temp, cv::Size(image_size, image_size));

        int row = i / tiles_per_row;
        int col = i % tiles_per_row;
        resized_temp.copyTo(tiled_image(cv::Rect(col * image_size, row * image_size, image_size, image_size)));
    }

    return tiled_image;
}

void display_clusters(std::vector<std::vector<dlib::matrix<dlib::rgb_pixel>>>& clusters) {
    for (size_t cluster_id = 0; cluster_id < clusters.size(); ++cluster_id) {
        cv::Mat cv_image = tile_images(clusters[cluster_id]);
        std::string window_name = "Face cluster " + std::to_string(cluster_id);
        cv::imshow(window_name, cv_image);
    }
    cv::waitKey(0);
}


namespace fs = std::__fs::filesystem;
std::string directoryPath = fs::current_path();

using namespace dlib;
using namespace std;

// Function to check if the new face matches any existing faces
unsigned long is_face_known(const matrix<float,0,1>& new_face_descriptor, const std::vector<matrix<float,0,1>>& existing_descriptors, const std::vector<unsigned long>& labels, double threshold = 0.6) {
    for (int i = 0; i < existing_descriptors.size(); i++) {
        auto& descriptor = existing_descriptors[i];
        if (length(new_face_descriptor - descriptor) < threshold) {
            return labels[i];
        }
    }
    return 100;
}

// ----------------------------------------------------------------------------------------

// The next bit of code defines a ResNet network.  It's basically copied
// and pasted from the dnn_imagenet_ex.cpp example, except we replaced the loss
// layer with loss_metric and made the network somewhat smaller.  Go read the introductory
// dlib DNN examples to learn what all this stuff means.
//
// Also, the dnn_metric_learning_on_images_ex.cpp example shows how to train this network.
// The dlib_face_recognition_resnet_model_v1 model used by this example was trained using
// essentially the code shown in dnn_metric_learning_on_images_ex.cpp except the
// mini-batches were made larger (35x15 instead of 5x5), the iterations without progress
// was set to 10000, and the training dataset consisted of about 3 million images instead of
// 55.  Also, the input layer was locked to images of size 150.
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

// ----------------------------------------------------------------------------------------

std::vector<matrix<rgb_pixel>> jitter_image(
        const matrix<rgb_pixel>& img
);

// ----------------------------------------------------------------------------------------

int main() {
//    if (argc != 2)
//    {
//        cout << "Run this example by invoking it like this: " << endl;
//        cout << "   ./dnn_face_recognition_ex faces/bald_guys.jpg" << endl;
//        cout << endl;
//        cout << "You will also need to get the face landmarking model file as well as " << endl;
//        cout << "the face recognition model file.  Download and then decompress these files from: " << endl;
//        cout << "http://dlib.net/files/shape_predictor_5_face_landmarks.dat.bz2" << endl;
//        cout << "http://dlib.net/files/dlib_face_recognition_resnet_model_v1.dat.bz2" << endl;
//        cout << endl;
//        return 1;
//    }

    // The first thing we are going to do is load all our models.  First, since we need to
    // find faces in the image we will need a face detector:
    frontal_face_detector detector = get_frontal_face_detector();
    // We will also use a face landmarking model to align faces to a standard pose:  (see face_landmark_detection_ex.cpp for an introduction)
    shape_predictor sp;
    deserialize(directoryPath + "/data/dlib_models/shape_predictor_5_face_landmarks.dat") >> sp;
    // And finally we load the DNN responsible for face recognition.
    anet_type net;
    deserialize(directoryPath + "/data/dlib_models/dlib_face_recognition_resnet_model_v1.dat") >> net;

    matrix<rgb_pixel> img;
    load_image(img, "test.jpg");
    // Display the raw image on the screen
    cv::Mat cv_img = dlib::toMat(img);
    cv::imshow("Converted Image", cv_img);
    cv::waitKey(0);
    cv::destroyAllWindows();

    // Run the face detector on the image of our action heroes, and for each face extract a
    // copy that has been normalized to 150x150 pixels in size and appropriately rotated
    // and centered.
    std::vector<matrix<rgb_pixel>> faces;
    for (auto face : detector(img))
    {
        auto shape = sp(img, face);
        matrix<rgb_pixel> face_chip;
        extract_image_chip(img, get_face_chip_details(shape,150,0.25), face_chip);
        faces.push_back(std::move(face_chip));
        // Also put some boxes on the faces so we can see that the detector is finding
        // them.
        cv::Rect rect(cv::Point(face.left(), face.top()), cv::Point(face.right(), face.bottom()));

        // Draw the rectangle on OpenCV image
        cv::rectangle(cv_img, rect, cv::Scalar(0, 255, 0), 2);
        cv::imshow("Faces with Boxes", cv_img);
        cv::waitKey(0);
    }
    cv::destroyAllWindows();

    if (faces.size() == 0)
    {
        cout << "No faces found in image!" << endl;
        return 1;
    }

    // This call asks the DNN to convert each face image in faces into a 128D vector.
    // In this 128D vector space, images from the same person will be close to each other
    // but vectors from different people will be far apart.  So we can use these vectors to
    // identify if a pair of images are from the same person or from different people.
    std::vector<matrix<float,0,1>> face_descriptors = net(faces);


    // In particular, one simple thing we can do is face clustering.  This next bit of code
    // creates a graph of connected faces and then uses the Chinese whispers graph clustering
    // algorithm to identify how many people there are and which faces belong to whom.
    std::vector<sample_pair> edges;
    for (size_t i = 0; i < face_descriptors.size(); ++i)
    {
        for (size_t j = i; j < face_descriptors.size(); ++j)
        {
            // Faces are connected in the graph if they are close enough.  Here we check if
            // the distance between two face descriptors is less than 0.6, which is the
            // decision threshold the network was trained to use.  Although you can
            // certainly use any other threshold you find useful.
            if (length(face_descriptors[i]-face_descriptors[j]) < 0.6)
                edges.push_back(sample_pair(i,j));
        }
    }
    std::vector<unsigned long> labels;
    const auto num_clusters = chinese_whispers(edges, labels);
    // This will correctly indicate that there are 4 people in the image.
    cout << "number of people found in the image: "<< num_clusters << endl;


    // Now let's display the face clustering results on the screen.  You will see that it
    // correctly grouped all the faces.
    // Create clusters
    std::vector<std::vector<dlib::matrix<dlib::rgb_pixel>>> clusters(num_clusters);
    for (size_t j = 0; j < labels.size(); ++j) {
        clusters[labels[j]].push_back(faces[j]);
    }

    // Display clusters
    display_clusters(clusters);



    matrix<rgb_pixel> new_img;
    load_image(new_img, "test1.jpg");
    std::vector<matrix<rgb_pixel>> new_faces;
    for (auto face : detector(new_img)) {
        matrix<rgb_pixel> new_face_chip;
        auto shape = sp(new_img, face);
        extract_image_chip(new_img, get_face_chip_details(shape,150,0.25), new_face_chip);
        new_faces.push_back(std::move(new_face_chip));
    }

    // Extract descriptors for new faces
    std::vector<matrix<float,0,1>> new_face_descriptors = net(new_faces);

    // Check each new face against known faces
    for (const auto& new_face_descriptor : new_face_descriptors) {
        unsigned long known = is_face_known(new_face_descriptor, face_descriptors, labels);
        cout << known << endl;
    }




//    // Finally, let's print one of the face descriptors to the screen.
//    cout << "face descriptor for one face: " << trans(face_descriptors[0]) << endl;
//
//    // It should also be noted that face recognition accuracy can be improved if jittering
//    // is used when creating face descriptors.  In particular, to get 99.38% on the LFW
//    // benchmark you need to use the jitter_image() routine to compute the descriptors,
//    // like so:
//    matrix<float,0,1> face_descriptor = mean(mat(net(jitter_image(faces[0]))));
//    cout << "jittered face descriptor for one face: " << trans(face_descriptor) << endl;
//    // If you use the model without jittering, as we did when clustering the bald guys, it
//    // gets an accuracy of 99.13% on the LFW benchmark.  So jittering makes the whole
//    // procedure a little more accurate but makes face descriptor calculation slower.


    cout << "hit enter to terminate" << endl;
    cin.get();
}

// ----------------------------------------------------------------------------------------

std::vector<matrix<rgb_pixel>> jitter_image(
        const matrix<rgb_pixel>& img
)
{
    // All this function does is make 100 copies of img, all slightly jittered by being
    // zoomed, rotated, and translated a little bit differently. They are also randomly
    // mirrored left to right.
    thread_local dlib::rand rnd;

    std::vector<matrix<rgb_pixel>> crops;
    for (int i = 0; i < 100; ++i)
        crops.push_back(jitter_image(img,rnd));

    return crops;
}

// ----------------------------------------------------------------------------------------

