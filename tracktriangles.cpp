#include <iostream>
#include "opencv2/opencv.hpp"
//#include <time.h>
#include <math.h>

using namespace cv;
using namespace std;

#define USE_CAMERA  1             //If using camera, then make this 1 else 0
#define PC          1      //if running on computer (PC), then show images, use setup etc
#define SETUP_VAL   0             //Setup?  (Note: if not running on PC, this will create errors and is diabled)


#if PC==1
#define DeviceID 0         //My laptop has an inbuild camera. If yours doesn't, then make it 0
#else
#define DeviceID 0         //BB defines this as Video0
#endif

/* Detecting yellow */
#define DEF_MIN_H        40
#define DEF_MAX_H        80
#define DEF_MIN_S        125
#define DEF_MAX_S        255
#define DEF_MIN_V        50
#define DEF_MAX_V        255

//Resolution of the camera frame
#define WIDTH 1280
#define HEIGHT 720

#define SETUP SETUP_VAL & PC

#define _180_BY_PI 57.295779513

RNG rng(12345);

const int max_value_H = 360 / 2;
const int max_value = 255;
const String window_capture_name = "Video Capture";
const String window_detection_name = "Object Detection";

int low_H = 23, low_S = 80, low_V = 80;
int high_H = 40, high_S = 255, high_V = 255;

int main(int argc, char** argv) {
	struct timespec start, end;
	int width = WIDTH, height = HEIGHT;
	uint64_t diff;
	uint32_t framecount = 0;
	string errortext = "", fps = "";

	bool flag = 0;

	//get the centre of image
	const Point coi((width - 1) / 2, (height - 1) / 2);

	//int minh = DEF_MIN_H, maxh = DEF_MAX_H;
	//int mins = DEF_MIN_S, maxs = DEF_MAX_S;
	//int minv = DEF_MIN_V, maxv = DEF_MAX_V;
	Mat frame, hsv, threshold, working;

#if USE_CAMERA==1
	VideoCapture camera(DeviceID);

	camera.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
	camera.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);
	camera.set(CV_CAP_PROP_GAIN, 0);

	if (!camera.isOpened()) {
		cout << "ERROR! Unable to open camera.\n";
		return -1;
	}
#else
	frame = imread("images/yellow_triangles_star_red_rect.png");
	width = frame.cols;
	height = frame.rows;
#endif

#if defined(DISPLAY)
	namedWindow( "Camera", WINDOW_NORMAL );
#endif

	for (;;)
	{
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
		//camera.read(frame);
		camera >> frame;
		if (frame.empty()) {
			cout << "Camera capture error." << endl;
			return 1;
		}

		framecount++;
			
		//convert to hue/saturation/value
		cvtColor(frame, working, CV_BGR2HSV);

		//blur frame to make the "contours" work better
		//blur(hsv,hsv, Size(1,1));
		//GaussianBlur(hsv, hsv, Size(5, 5), 0);

		  //color pass through, get binary image (black and white)
		inRange(working, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), working);

		//storage vector for our contours
		//vector<Point> is one contour, held in another vector
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;

		//get the contours (using CV_RETR_LIST since we don't need heirarchy)
		findContours(working, contours, hierarchy, CV_RETR_LIST, CHAIN_APPROX_SIMPLE);
		//at this point there is a vector of contours, but we just want
		//2 triangles, make sure we have those.
		vector<vector<Point>> triangles;
		for (auto i : contours) {
			vector<Point> poly;
			approxPolyDP(i, poly, 0.05*arcLength(i, 1), 1);
			//did it find a triangle?
			if (poly.size() == 3 && contourArea(poly) > 300.0) {
				triangles.push_back(poly);
			}
		}

		// look for exactly 2 triangles
		if (triangles.size() != 2) {
			errortext = "pan error: -, tilt error: -, roll error: -";
		}
		else
		{
			//get the moments
			vector<Moments> mo(2);
			mo[0] = moments(triangles[0], 0);
			mo[1] = moments(triangles[1], 0);

			/* get the centers using the moments */
			vector<Point> centers(2);
			for (unsigned int i = 0; i < 2; i++) {
				//centers[i] = (rectangles[i].tl() + rectangles[i].br()) * .5;
				double CX, CY;
				CX = mo[i].m10 / mo[i].m00;
				CY = mo[i].m01 / mo[i].m00;
				centers[i] = Point((int)CX, (int)CY);
			}

			Point cots = (centers[0] + centers[1]) * .5;
			Point error = cots - coi;
			int ErrorPan = error.x;
			int ErrorTilt = -error.y;
			float ErrorRoll = _180_BY_PI * atanf(((float)centers[0].y - centers[1].y) / (centers[0].x - centers[1].x));
			ErrorRoll = -ErrorRoll;
			errortext = "pan error: " + to_string(ErrorPan) + ", tilt error: " + to_string(ErrorTilt) + ", roll error: " + to_string(ErrorRoll);
			
			//draw a line connecting the 2 triangles
			//another lines through the middle of the horizontal and vertical axis
			line(frame, centers[0], centers[1], Scalar(0, 255, 0));
			circle(frame, cots, 10, Scalar(0, 255, 0));
			flag = 1;	
		}

		rectangle(frame, Point(0,0), Point(width-1, 30), Scalar(0, 0, 0),  CV_FILLED); 
		line(frame, Point(width*.35, (height - 1) / 2), Point((width - 1)*.65, (height - 1) / 2), Scalar(0, 0, 255));
		line(frame, Point((width - 1) / 2, height*.35), Point((width - 1) / 2, (height - 1)*.65), Scalar(0, 0, 255));
		circle(frame, coi, 10, Scalar(0, 0, 255));
		
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
		diff = 1000000000 * (end.tv_sec - start.tv_sec) +
			end.tv_nsec - start.tv_nsec;
		diff = diff / 1000000;
		int fps = (int)(1000/diff);
		errortext += ", frame time: " + to_string(diff) + " ms, fps: " + to_string(fps);
		cout << errortext << endl;
		
		putText(frame, errortext, Point(10, 20), FONT_HERSHEY_SIMPLEX, .75, Scalar(0, 0, 255), 1);
#if defined(DISPLAY)
		imshow("Camera", frame);
		waitKey(1);
#endif
		if (flag == 1) {
			flag = 0;
			string filename = "out/" + to_string(framecount) + ".jpg";
			imwrite(filename, frame);
		}	
	}

	return 0;
}
