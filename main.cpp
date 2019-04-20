#include <iostream>
#include "opencv2/opencv.hpp"
#include <time.h>

using namespace cv;
using namespace std;

//#define USE_CAMERA

#define DEF_MIN_H		40
#define DEF_MAX_H		80
#define DEF_MIN_S		125
#define DEF_MAX_S		255
#define DEF_MIN_V		50
#define DEF_MAX_V		255

RNG rng(12345);

int main(int argc, char** argv) {
	struct timespec start, end;
	uint64_t diff;
	
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
	
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	
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
	frame = imread("images/yellow_triangles_star_red_rect.png");
#endif

	
	if (frame.empty()) {
		cout << "Camera capture error." << endl;
		return 1;
	}
	
	
	
	//convert to hue/saturation/value
	cvtColor(frame, hsv, CV_BGR2HSV); 
	//yellow pass through, get binary image (black and white)
	inRange(hsv, Scalar(minh/2,mins,minv), Scalar(maxh/2,maxs,maxv), threshold);
	
	//storage vector for our contours
	//vector<Point> is one contour, held in another vector
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	//get the contours
	findContours(threshold, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	
	//at this point there is a vector of contours, but we just want
	//2 triangles, make sure we have those.
	vector<vector<Point>> triangles;
	for (unsigned int i = 0; i < contours.size(); i++) {
		vector<Point> poly;
		approxPolyDP(contours[i], poly, 0.07 * arcLength(contours[i], 1), 1);
		if (poly.size() == 3) {
			triangles.push_back(poly);
		}
	}
	
	if (triangles.size() != 2) {
		cout << "Error, did not find 2 triangles." << endl;
	}
	
	Mat drawing = Mat::zeros( frame.size(), CV_8UC3 );
	
	vector<Rect> rectangles(2);
	for (unsigned int i = 0; i < 2; i++) {
		rectangles[i] = boundingRect(triangles[i]);
		
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		rectangle( drawing, rectangles[i].tl(), rectangles[i].br(), color, 2, 8, 0 );
	}
	
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
	imwrite("images/capture.png", frame); // save capture
	//imwrite("hsv.png", hsv);
	imwrite("images/threshold.png", threshold);
	imwrite("images/drawing.png", drawing);
	
	diff = 1000000000 * (end.tv_sec - start.tv_sec) +
			end.tv_nsec - start.tv_nsec;
	
	cout << "Done." << endl;
	cout << "Time: " << diff / 1000000 << " ms" << endl;
	return 0;
}