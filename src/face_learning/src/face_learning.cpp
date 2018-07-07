#include "opencv2/core.hpp"
#include "opencv2/face.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/aruco.hpp"
#include "ros/ros.h"
#include "std_msgs/String.h"

#include <ros/console.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace cv;
using namespace cv::face;
using namespace std;

static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';')
{
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file)
    {
      cout << "keine datei" << file << "aus: " << filename << endl;
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path, classlabel;
    while (getline(file, line))
    {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty())
	{
            images.push_back(imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
}

static void trainModel(vector<Mat>& images, vector<int>& labels, Ptr<FisherFaceRecognizer>& model)
{
      model->train(images, labels);
}


static void chatterCallback(const std_msgs::String::ConstPtr& msg)
{
}


int main(int argc, char **argv)
{
    // Check for valid command line arguments, print usage if no arguments were given.
    if (argc != 6) {
      cout << "test anzahl: " << argc << endl;
      cout << "1: " << argv[0] << endl << " 2: " << argv[1] << endl << " 3: " << argv[2] <<
	endl << "4: " << argv[3] << endl << "5: " << argv[4] << endl << "6: " << argv[5] << endl << "7: " << argv[6] << endl;
      cout << "usage: " << argv[0] << " </path/to/haar_cascade> </path/to/csv.ext> </path/to/device id>" << endl;
      cout << "\t </path/to/haar_cascade> -- Path to the Haar Cascade for face detection." << endl;
      cout << "\t </path/to/csv.ext> -- Path to the CSV file with the face database." << endl;
      cout << "\t <advice id> -- The webcam device id to grab frames from." << endl;
      exit(1);
    }
    cout << "Argumente stimmen" << endl;

    ros::init(argc, argv, "Listener");
    ros::NodeHandle n;
    ros::Subscriber sub = n.subscribe("chatter", 1000, chatterCallback);

    
    cv::Ptr<cv::aruco::Dictionary> test;
    test = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_250);
    //test
    cout << "Variablen anlegen" << endl;
   // Get the path to your CSV:
    string fn_haar = string(argv[1]);
    string fn_csv = string(argv[2]);
    int deviceId = atoi(argv[3]);
    cout << "Argumente in Variablen geschrieben" << endl;
    cout << "haarcascade: " << fn_haar  << endl;
    cout << "csv: " << fn_csv << endl;
    cout << "device id: " << deviceId << endl;
    // These vectors hold the images and corresponding labels:
    vector<Mat> images;
    vector<int> labels;
    // Read in the data (fails if no valid input filename is given, but you'll get an error message):
    try
    {
        read_csv(fn_csv, images, labels);
	cout << "CSV gelesen" << endl;
    }
    catch (cv::Exception& e)
    {
        cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
        // nothing more we can do
        exit(1);
    }
    // Get the height from the first image. We'll need this later in code to reshape the images to their original
    // size AND we need to reshape incoming faces to this size:
    int im_width = images[0].cols;
    int im_height = images[0].rows;
    // Create a FaceRecognizer and train it on the given images:
   Ptr<FisherFaceRecognizer> model = FisherFaceRecognizer::create();
   trainModel(images, labels, model);
    // That's it for learning the Face Recognition model. You now need to create the classifier for the task of Face Detection.
    // We are going to use the haar cascade you have specified in the command line arguments:
    CascadeClassifier haar_cascade;
    haar_cascade.load(fn_haar);
    // Get a handle to the Video device:
    VideoCapture cap(deviceId);
    // Check if we can use this device at all: // hier die kinect rein basteln
    if(!cap.isOpened())
    {
        cerr << "Capture Device ID " << deviceId << "cannot be opened." << endl;
        return -1;
    }
    // Holds the current frame from the Video device:
    // am besten immer am anfang einen frame auslesen bei for(;;)(?)
    Mat frame;
    for(;;)
    {
        cap >> frame;
        // Clone the current frame:
        Mat original = frame.clone();
        // Convert the current frame to grayscale:
        Mat gray;
        cvtColor(original, gray, CV_BGR2GRAY);
        // Find the faces in the frame:
        vector< Rect_<int> > faces;
        haar_cascade.detectMultiScale(gray, faces);
        // At this point you have the position of the faces in
        // faces. Now we'll get the faces, make a prediction and
        // annotate it in the video. Cool or what?
        for(int i = 0; i < faces.size(); i++)
	{
            // Process face by face:
            Rect face_i = faces[i];
            // Crop the face from the image. So simple with OpenCV C++:
            Mat face = gray(face_i);
            // Resizing the face is necessary for Eigenfaces and Fisherfaces. You can easily
            // verify this, by reading through the face recognition tutorial coming with OpenCV.
            // Resizing IS NOT NEEDED for Local Binary Patterns Histograms, so preparing the
            // input data really depends on the algorithm used.
            //
            // I strongly encourage you to play around with the algorithms. See which work best
            // in your scenario, LBPH should always be a contender for robust face recognition.
            //
            // Since I am showing the Fisherfaces algorithm here, I also show how to resize the
            // face you have just found:
            Mat face_resized;
            cv::resize(face, face_resized, Size(im_width, im_height), 1.0, 1.0, INTER_CUBIC);
            // Now perform the prediction, see how easy that is:
            int prediction = model->predict(face_resized);
            // And finally write all we've found out to the original image!
            // First of all draw a green rectangle around the detected face:
            rectangle(original, face_i, CV_RGB(0, 255,0), 1);
            // Create the text we will annotate the box with:
            string box_text = format("Prediction = %d", prediction);
            // Calculate the position for annotated text (make sure we don't
            // put illegal values in there):
            int pos_x = std::max(face_i.tl().x - 10, 0);
            int pos_y = std::max(face_i.tl().y - 10, 0);
            // And now put it into the image:
            putText(original, box_text, Point(pos_x, pos_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
        }
        // Show the result:
        imshow("face_recognizer", original);
        // And display it:
        char key = (char) waitKey(20);
        // Exit this loop on escape:
        if(key == 27)
            break;
    }
    return 0;
}

