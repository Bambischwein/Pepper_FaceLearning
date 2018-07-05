#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "ros/ros.h"
#include "std_msgs/String.h"


#include <ros/console.h>

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

/** Function Headers */
void detectAndDisplay( Mat frame );

/** Global variables */
String face_cascade_name = "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml";
String eyes_cascade_name = "/usr/share/opencv/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
String window_name = "Capture - Face detection";

/** @function main */
int main(int argc, const char *argv[])
{
  if(argc != 5)
    {
      cout << "Error" << endl;
      exit(1);
    }


  // ros::init(argc, argv, "listener");

    VideoCapture capture;
    Mat frame;

    //-- 1. Load the cascades
    cout << "test" << endl;
    // if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading face cascade\n"); return -1; };
    // if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("--(!)Error loading eyes cascade\n"); return -1; };

    face_cascade.load(argv[1]);
    eyes_cascade.load(argv[2]);

    
    //-- 2. Read the video stream
    capture.open( -1 );
    if ( ! capture.isOpened() ) { printf("--(!)Error opening video capture\n"); return -1; }

    while (  capture.read(frame) )
    {
        if( frame.empty() )
        {
            printf(" --(!) No captured frame -- Break!");
            break;
        }

        //-- 3. Apply the classifier to the frame
        detectAndDisplay( frame );

        int c = waitKey(100);
        if( (char)c == 27 ) { break; } // escape
    }
    return 0;
}

/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
    std::vector<Rect> faces;
    Mat frame_gray;
    // Mat crop;
    
    cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );

    //-- Detect faces
    face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(80, 80) );

    for( size_t i = 0; i < faces.size(); i++ )
    {
        Point center( faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2 );
    	// rectangle(frame, faces[i], CV_RGB(0, 255,0), 1);

    	rectangle(frame, Point( faces[i].x + faces[i].width + 20, faces[i].y + faces[i].height + 20),
    		  Point(faces[i].x - faces[i].width/2 + 10, faces[i].y - faces[i].height/2), Scalar(255,0, 0));

	// crop = frame(Point( faces[i].x + faces[i].width + 20, faces[i].y + faces[i].height + 20),    		  Point(faces[i].x - faces[i].width/2 + 10, faces[i].y - faces[i].height/2));
    	Mat faceROI = frame_gray( faces[i] );
        // std::vector<Rect> eyes;
        // eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CASCADE_SCALE_IMAGE, Size(1, 1) );

        // for( size_t j = 0; j < eyes.size(); j++ )
        // {
        //     Point eye_center( faces[i].x + eyes[j].x + eyes[j].width/2, faces[i].y + eyes[j].y + eyes[j].height/2 );
    	//     int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
        //     circle( frame, eye_center, radius, Scalar( 255, 0, 0 ), 2, 8, 0 );
        // }
    }
    imshow( window_name, frame );
}