#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "ros/ros.h"
#include "std_msgs/String.h"
#include "sensor_msgs/Image.h"


#include <ros/console.h>

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

/** Function Headers */
void detectAndDisplay( Mat frame);

/** Global variables */
String face_cascade_name;
String eyes_cascade_name;
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
String window_name = "Capture - Face detection";



/** @function detectAndDisplay */
void detectAndDisplay( Mat frame)
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

    	rectangle(frame, Point( faces[i].x + faces[i].width + 20, faces[i].y + faces[i].height + 20),
    		  Point(faces[i].x - faces[i].width/2 + 10, faces[i].y - faces[i].height/2), Scalar(255,0, 0));
	
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


/** @function main */
int main(int argc, char **argv)
{
  if(argc != 5)
    {
      cout << "Error" << endl;
      exit(1);
    }
    VideoCapture capture;
    Mat frame;
    ROS_INFO_STREAM("1");
    face_cascade.load(argv[1]);
    eyes_cascade.load(argv[2]);
    ROS_INFO_STREAM("2");
    
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
        detectAndDisplay( frame);

        int c = waitKey(100);
        if( (char)c == 27 ) { break; } // escape
    }
    return 0;
}
