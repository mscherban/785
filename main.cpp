#include <iostream>
#include "opencv2/opencv.hpp"
#include <time.h>

using namespace cv;
using namespace std;

//#define USE_CAMERA

/* yellow */
#define DEF_MIN_H		40
#define DEF_MAX_H		80
#define DEF_MIN_S		125
#define DEF_MAX_S		255
#define DEF_MIN_V		50
#define DEF_MAX_V		255

#define WIDTH 1280
#define HEIGHT 720

RNG rng(12345);

int main(int argc, char** argv) {
	struct timespec start, end;
	int width = WIDTH, height = HEIGHT;
	uint64_t diff;
	
	int minh = DEF_MIN_H, maxh = DEF_MAX_H;
	int mins = DEF_MIN_S, maxs = DEF_MAX_S;
	int minv = DEF_MIN_V, maxv = DEF_MAX_V;
	//cout << "OpenCV Version : " << CV_VERSION << endl;
	
	/* define HSV numbers for other colors */
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
	
	// cout << "hue: " << minh << "-" << maxh << endl;
	// cout << "sat: " << mins << "-" << maxs << endl;
	// cout << "val: " << minv << "-" << maxv << endl;
	
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	
	Mat frame, hsv, threshold;
#if defined(USE_CAMERA)
	VideoCapture camera(0); // /dev/video0

	camera.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
	camera.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);
	camera.set(CV_CAP_PROP_GAIN, 0);
	if (!camera.isOpened()) {
		cout << "Camera error." << endl;
		return 1;
	}
	
	camera >> frame; // capture first frame
#else
	frame = imread("images/yellow_triangles_star_red_rect.png");
	width = frame.cols;
	height = frame.rows;
#endif

	
	if (frame.empty()) {
		cout << "Camera capture error." << endl;
		return 1;
	}
	
	
	
	//convert to hue/saturation/value
	cvtColor(frame, hsv, CV_BGR2HSV); 
	//color pass through, get binary image (black and white)
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
		//did it find a triangle?
		if (poly.size() == 3) {
			triangles.push_back(poly);
		}
	}
	
	if (triangles.size() != 2) {
		cout << "Error, did not find 2 triangles." << endl;
	}
	
	//get bounding rectangles around the triangles, might be easier to work with
	vector<Rect> rectangles(2);
	for (unsigned int i = 0; i < 2; i++) {
		rectangles[i] = boundingRect(triangles[i]);
		
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		rectangle( frame, rectangles[i].tl(), rectangles[i].br(), color, 2, 8, 0 );
	}
	
	/* get the centers */
	vector<Point> centers(2);
	for (unsigned int i = 0; i < 2; i++) {
		centers[i] = (rectangles[i].tl() + rectangles[i].br()) * .5;
	}
	
	//draw a line connecting the 2 rectangles
	//another lines through the middle of the horizontal and vertical axis
	line(frame, centers[0], centers[1], Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) ));
	line(frame, Point(0, (height-1)/2), Point(width-1, (height-1)/2), Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) ));
	line(frame, Point((width-1)/2, 0), Point((width-1)/2,height-1), Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) ));
	
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
	imwrite("images/capture.png", frame); // save capture
	//imwrite("hsv.png", hsv);
	imwrite("images/threshold.png", threshold);
	
	diff = 1000000000 * (end.tv_sec - start.tv_sec) +
			end.tv_nsec - start.tv_nsec;
	
	cout << "Done." << endl;
	cout << "Time: " << diff / 1000000 << " ms" << endl;
	return 0;
}