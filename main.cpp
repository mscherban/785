#include <iostream>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

//#define USE_CAMERA

#define DEF_MIN_H		40
#define DEF_MAX_H		80
#define DEF_MIN_S		125
#define DEF_MAX_S		255
#define DEF_MIN_V		50
#define DEF_MAX_V		255

int main(int argc, char** argv) {
	int minh = DEF_MIN_H, maxh = DEF_MAX_H;
	int mins = DEF_MIN_S, maxs = DEF_MAX_S;
	int minv = DEF_MIN_V, maxv = DEF_MAX_V;
	cout << "OpenCV Version : " << CV_VERSION << endl;
	
	if (argc >= 3) {
		minh = atoi(argv[1]);
		maxh = atoi(argv[2]);
	}
	if (argc >= 5) {
		mins = atoi(argv[3]);
		maxs = atoi(argv[4]);
	}
	if (argc >= 7)	{
		minv = atoi(argv[5]);
		maxv = atoi(argv[6]);
	}
	
	cout << "hue: " << minh << "-" << maxh << endl;
	cout << "sat: " << mins << "-" << maxs << endl;
	cout << "val: " << minv << "-" << maxv << endl;
	
	
	Mat frame, hsv, threshold;
#if defined(USE_CAMERA)
	VideoCapture camera(0); // /dev/video0

	camera.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	camera.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	camera.set(CV_CAP_PROP_GAIN, 0);
	if (!camera.isOpened()) {
		cout << "Camera error." << endl;
		return 1;
	}
	
	camera >> frame; // capture first frame
#else
	frame = imread("yellow_triangles.png");
#endif

	
	if (frame.empty()) {
		cout << "Camera capture error." << endl;
		return 1;
	}
	
	cvtColor(frame, hsv, CV_BGR2HSV);
	inRange(hsv, Scalar(minh/2,mins,minv), Scalar(maxh/2,maxs,maxv), threshold);
	
	vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
	
	imwrite("capture.png", frame); // save capture
	imwrite("hsv.png", hsv);
	imwrite("threshold.png", threshold);
	
	cout << "Done." << endl;
	return 0;
}