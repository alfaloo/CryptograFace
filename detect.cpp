#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <filesystem>

using namespace std;
using namespace cv;

string executablePath = __fs::filesystem::current_path();

void detectAndDraw(Mat& img,
                   CascadeClassifier& cascade,
                   CascadeClassifier& nestedCascade,
                   double scale);

string cascadeName;
string nestedCascadeName;

int main()
{
    cout<<executablePath<<"\n";
    VideoCapture capture;
    Mat frame, image;
    CascadeClassifier cascade, nestedCascade;
    double scale;


    cascadeName = executablePath + "/data/haarcascades/haarcascade_frontalface_alt.xml";
    nestedCascadeName = executablePath + "/data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
    scale = 2;

    if (!nestedCascade.load(samples::findFileOrKeep(nestedCascadeName))) {
        cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
    } if (!cascade.load(samples::findFile(cascadeName))) {
        cerr << "ERROR: Could not load classifier cascade" << endl;
    } if (!capture.open(0)) {
        cout << "Capture from camera #" << 0 << " didn't work" << endl;
        return 1;
    }

    if(capture.isOpened()) {
        cout << "Video capturing has been started ..." << endl;

        while(true) {
            capture >> frame;
            if( frame.empty() )
                break;

            Mat frame1 = frame.clone();
            detectAndDraw(frame1, cascade, nestedCascade, scale);

            if (waitKey(10) >= 0) {
                break;
            }
        }
    }

    return 0;
}

void detectAndDraw(Mat& img,
                   CascadeClassifier& cascade,
                   CascadeClassifier& nestedCascade,
                   double scale) {
    double t = 0;
    vector<Rect> faces, faces2;
    const static Scalar colors[] = {Scalar(255,0,0),
                                    Scalar(255,128,0),
                                    Scalar(255,255,0),
                                    Scalar(0,255,0),
                                    Scalar(0,128,255),
                                    Scalar(0,255,255),
                                    Scalar(0,0,255),
                                    Scalar(255,0,255)};

    Mat gray, smallImg;

    cvtColor(img, gray, COLOR_BGR2GRAY);
    double fx = 1 / scale;
    resize(gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT);
    equalizeHist(smallImg, smallImg);

    t = (double)getTickCount();
    cascade.detectMultiScale(smallImg, faces,
                             1.1, 2, 0
                                     |CASCADE_SCALE_IMAGE,
                             Size(30, 30));

    t = (double)getTickCount() - t;
    printf( "detection time = %g ms\n", t*1000/getTickFrequency());
    for (size_t i = 0; i < faces.size(); i++) {
        Rect r = faces[i];
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];
        int radius;

        double aspect_ratio = (double) r.width/r.height;
        if (0.75 < aspect_ratio && aspect_ratio < 1.3) {
            center.x = cvRound((r.x + r.width*0.5)*scale);
            center.y = cvRound((r.y + r.height*0.5)*scale);
            radius = cvRound((r.width + r.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        } else {
            rectangle(img, Point(cvRound(r.x*scale), cvRound(r.y*scale)),
                      Point(cvRound((r.x + r.width-1)*scale), cvRound((r.y + r.height-1)*scale)),
                      color, 3, 8, 0);
        }
        if (nestedCascade.empty()) {
            continue;
        }
        smallImgROI = smallImg( r );
        nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
                                        1.1, 2, 0
                                                //|CASCADE_FIND_BIGGEST_OBJECT
                                                //|CASCADE_DO_ROUGH_SEARCH
                                                //|CASCADE_DO_CANNY_PRUNING
                                                |CASCADE_SCALE_IMAGE,
                                        Size(30, 30) );
        for (size_t j = 0; j < nestedObjects.size(); j++) {
            Rect nr = nestedObjects[j];
            center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
            center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
            radius = cvRound((nr.width + nr.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
    }
    imshow("result", img);
}
