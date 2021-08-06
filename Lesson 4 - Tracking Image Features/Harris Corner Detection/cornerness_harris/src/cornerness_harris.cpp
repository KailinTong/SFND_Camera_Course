#include <iostream>
#include <numeric>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>

using namespace std;

void cornernessHarris()
{
    // load image from file
    cv::Mat img;
    img = cv::imread("../images/img1.png");
    cv::cvtColor(img, img, cv::COLOR_BGR2GRAY); // convert to grayscale

    // Detector parameters
    int blockSize = 2;     // for every pixel, a blockSize × blockSize neighborhood is considered
    int apertureSize = 3;  // aperture parameter for Sobel operator (must be odd)
    int minResponse = 100; // minimum value for a corner in the 8bit scaled response matrix
    double k = 0.04;       // Harris parameter (see equation for details)

    // Detect Harris corners and normalize output
    cv::Mat dst, dst_norm, dst_norm_scaled;
    dst = cv::Mat::zeros(img.size(), CV_32FC1);
    cv::cornerHarris(img, dst, blockSize, apertureSize, k, cv::BORDER_DEFAULT);
    cv::normalize(dst, dst_norm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
    cv::convertScaleAbs(dst_norm, dst_norm_scaled);

    // visualize results
    string windowName = "Harris Corner Detector Response Matrix";
    cv::namedWindow(windowName, 4);
    cv::imshow(windowName, dst_norm_scaled);
    cv::waitKey(0);

    // TODO: Your task is to locate local maxima in the Harris response matrix 
    // and perform a non-maximum suppression (NMS) in a local neighborhood around 
    // each maximum. The resulting coordinates shall be stored in a list of keypoints 
    // of the type `vector<cv::KeyPoint>`.

    vector<cv::KeyPoint> keyPoints;

    for(int r = 0; r < dst_norm.rows; r++){
        for(int c = 0; c < dst_norm.cols; c++){
            auto response = dst_norm.at<int>(r, c);
            if(response > minResponse){
                cv::KeyPoint newKeyPoint{float(r), float(c), float(2 * apertureSize), -1, float(response)};

                bool bOverlap = false;

                for(auto & keyPoint : keyPoints){
                    double kptOverlap = cv::KeyPoint::overlap(newKeyPoint, keyPoint); // check point overlapping

                    if(kptOverlap > 0){ // overlapped
                        bOverlap = true;
                        if(newKeyPoint.response > keyPoint.response){ // check if the new key point response larger than the overlapped
                            keyPoint = newKeyPoint; // here it is a reference
                            break; // only replaces one keyPoint so there is no overlapped point
                        }
                    }

                }
                if(!bOverlap)
                    keyPoints.emplace_back(newKeyPoint);

            }
        } // eof loop over cols
    } // eof loop over rows
    cout << "Key point size is: " << keyPoints.size() << endl;
    // visualize keypoints
    windowName = "Harris Corner Detection Results";
    cv::namedWindow(windowName, 5);
    cv::Mat visImage = dst_norm_scaled.clone();
    cv::drawKeypoints(dst_norm_scaled, keyPoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    cv::imshow(windowName, visImage);
    cv::waitKey(0);
    cv::imwrite("01.png", visImage);


}

int main()
{
    cornernessHarris();
}