#include <iostream>
#include "opencv2/opencv.hpp"
#include <time.h>
#include <math.h>

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
		return 1;
	}

	//get the moments
	vector<Moments> mo(2);
	for (unsigned int i = 0; i < 2; i++) {
		mo[i] = moments(triangles[i], 0);
	}
	

	/* get the centers using the moments */
	vector<Point> centers(2);
	for (unsigned int i = 0; i < 2; i++) {
		double CX, CY;
		CX = mo[i].m10 / mo[i].m00;
		CY = mo[i].m01 / mo[i].m00;
		centers[i] = Point((int)CX, (int)CY);
	}

	Point coi((width-1)/2, (height-1)/2);
	Point cots = (centers[0] + centers[1]) * .5;
	Point error = cots - coi;
	int errorpan = error.x;
	int errortilt = -error.y;
	printf("error pan: %i, error tilt: %i\n", errorpan, errortilt);
	string errortext = "pan error: " + to_string(errorpan) + ", tilt error: " + to_string(errortilt);
	
	//draw a line connecting the 2 triangles
	//another lines through the middle of the horizontal and vertical axis
	line(frame, Point(width*.35, (height-1)/2), Point((width-1)*.65, (height-1)/2), Scalar( 0, 0, 255 ));
	line(frame, Point((width-1)/2, height*.35), Point((width-1)/2,(height-1)*.65), Scalar( 0, 0, 255 ));
	circle(frame, coi, 10, Scalar( 0, 0, 255 ));
	line(frame, centers[0], centers[1], Scalar( 0, 255, 0  ));
	circle(frame, cots, 10, Scalar( 0, 255, 0 ));
	putText(frame, errortext, Point(10, height-10),  FONT_HERSHEY_SIMPLEX, .75, Scalar( 0, 0, 255 ), 2);
	
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