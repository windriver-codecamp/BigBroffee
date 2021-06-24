
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/shape_utils.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#undef CV_VX_SHOW
#ifdef CV_VX_SHOW
#include "fboutput/cvvxdisplay.hpp"
#endif

using namespace cv;
using namespace std;

int motion_detect_avg(int cameraDevice, int contoursThresh) {
    VideoCapture cap;
    Mat frame, avg, gray, frameDelta, thresh, a1;
	vector<vector<Point> > cnts;
    int i, ret, init_avg, motion_detected;
    ofstream mfile("/bd0a/motion_log.txt");
    time_t ct, pt;

    ret = -1;
    init_avg = 0;
	motion_detected = 0;

    time(&pt);
    try {
        cap = VideoCapture(cameraDevice);
        if(!cap.isOpened()) {
            cout << "Couldn't find camera: " << cameraDevice << endl;
            return -1;
        }
        cap.set(CAP_PROP_FRAME_WIDTH, 640);

        for(i = 0; i < 1000; i++) {
		    cap >> frame;

		    printf("\033[2J");
			printf("\033[1;1H");

		    if (frame.empty())
		    {
		    	ret = -1;
		        break;
		    }

#ifdef __VXWORKS__
            cvtColor(frame, frame, COLOR_BGRA2BGR);
#endif

		    cvtColor(frame, gray, COLOR_BGR2GRAY);
		    GaussianBlur(gray, gray, Size(21, 21), 0);

            if (!init_avg) {
                init_avg = 1;
                gray.convertTo(avg, CV_32F);
            }

            accumulateWeighted(gray, avg, 0.5);
            convertScaleAbs(avg, a1);
		    absdiff(gray, a1, frameDelta);
		    threshold(frameDelta, thresh, 25, 255, THRESH_BINARY);
		    
		    dilate(thresh, thresh, Mat(), Point(-1,-1), 2);
		    findContours(thresh, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
            motion_detected = 0;

		    for(int i = 0; i< cnts.size(); i++) {
		        if(contourArea(cnts[i]) >= contoursThresh) {
					motion_detected = 1;
#if 0
		    		printf("%s:%d - contourArea(cnts[%d]) = %f motion_detected = %d\n", __FUNCTION__, __LINE__,
		    			i, contourArea(cnts[i]), motion_detected);
#endif
				    putText(frame, "Motion Detected", Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255),2);
				    break;
		        }
		    }
            if (motion_detected) {
                time(&ct);
                if (ct != pt) {
					cout << "Motion@" << ctime(&ct) << endl;
                    mfile << "Motion@" << ctime(&ct) << endl;
					pt = ct;
                }
            } else {
                cout << "No motion detected" << endl;
            }
#ifdef CV_VX_SHOW
		    imshow("thresh", thresh);
		    imshow("delta", frameDelta);
		    imshow("frame", frame);
#endif

		}
	} catch( cv::Exception& e )
	{
		const char* err_msg = e.what();
		cout << "exception: " << e.what() << endl;
		ret = -1;
	} catch (...) {
        cout << "generic exception" << endl;
		ret = -1;
	}
    cap.release();
    mfile.close();

    return ret;
}

int main(int argc, char **argv) {
    motion_detect_avg(0, 1000);
}

