#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;

Mat bgr_img, hsv_img;

int main(int argc, char *argv[])
{
	    // Open the video camera.
    std::string pipeline = "libcamerasrc"
        " ! video/x-raw, width=800, height=600" // camera needs to capture at a higher resolution
        " ! videoconvert"
        " ! videoscale"
        " ! video/x-raw, width=400, height=300" // can downsample the image after capturing
        " ! videoflip method=rotate-180" // remove this line if the image is upside-down
        " ! appsink drop=true max_buffers=2";

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);

    if(!cap.isOpened()) {
        printf("Could not open camera.\n");
        return 1;
    }

	// Create a control window
	namedWindow("Control", WINDOW_AUTOSIZE);
	int iLowH = 0;
	int iHighH = 179;

	int iLowS = 0;
	int iHighS = 255;

	int iLowV = 0;
	int iHighV = 255;

	int iKernel = 1;
	if (iKernel % 2 == 0) iKernel += 1; // Odd kernel size for easy centering

	// Create trackbars in "Control" window
	createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	createTrackbar("HighH", "Control", &iHighH, 179);

	createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	createTrackbar("HighS", "Control", &iHighS, 255);

	createTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	createTrackbar("HighV", "Control", &iHighV, 255);

	createTrackbar("Kernel", "Control", &iKernel, 10);

	// Create the display windows
	namedWindow("Control", WINDOW_AUTOSIZE);
	imshow("Display", WINDOW_AUTOSIZE);
	namedWindow("Thresholded", WINDOW_AUTOSIZE);

	Mat bgr_img, hsv_img, thresh_img;

	// Display the result of the current calibration settings
	while (true) {
		if (!cap.read(bgr_img)) {
            std::cerr << "Could not read a frame.\n";
            break;
        }

		// Convert to HSV colour space
		cvtColor(bgr_img, hsv_img, COLOR_BGR2HSV);

		// Threshold the image
		inRange(hsv_img, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), thresh_img);

		// Apply morphology to image
		morphologyEx(thresh_img, thresh_img, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(iKernel,iKernel)));
		morphologyEx(thresh_img, thresh_img, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(iKernel,iKernel)));

		// Object Tracking
		Moments m = moments(thresh_img, true);
		if (m.m00 > 0) {
			double x = m.m10 / m.m00;
			double y = m.m01 / m.m00;
			printf("Object at: (%.1f, %.1f)", x, y);

			Point centroid(static_cast<int>(x), static_cast<int>(y));
			char coordText[50];
			snprintf(coordText, sizeof(coordText), "(%d, %d)", centroid.x, centroid.y);
			putText(bgr_img, coordText,
				Point(centroid.x + 10, centroid.y - 10), 
				FONT_HERSHEY_SIMPLEX, 0.5, 
				Scalar(255, 255, 255),
				 1, LINE_AA);
		}

		// Show the thresholded image
		imshow("Thresholded", thresh_img);

		// Draw the final image
		imshow("Display", bgr_img);

		// Allow openCV to process GUI events
		waitKey(100);
	}

	cap.release();
	return 0;
}
