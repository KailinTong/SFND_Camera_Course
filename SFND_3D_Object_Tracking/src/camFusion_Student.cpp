
#include <iostream>
#include <algorithm>
#include <numeric>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "camFusion.hpp"
#include "dataStructures.h"

using namespace std;


// Create groups of Lidar points whose projection into the camera falls into the same bounding box
void clusterLidarWithROI(std::vector<BoundingBox> &boundingBoxes, std::vector<LidarPoint> &lidarPoints, float shrinkFactor, cv::Mat &P_rect_xx, cv::Mat &R_rect_xx, cv::Mat &RT)
{
    // loop over all Lidar points and associate them to a 2D bounding box
    cv::Mat X(4, 1, cv::DataType<double>::type);
    cv::Mat Y(3, 1, cv::DataType<double>::type);

    for (auto it1 = lidarPoints.begin(); it1 != lidarPoints.end(); ++it1)
    {
        // assemble vector for matrix-vector-multiplication
        X.at<double>(0, 0) = it1->x;
        X.at<double>(1, 0) = it1->y;
        X.at<double>(2, 0) = it1->z;
        X.at<double>(3, 0) = 1;

        // project Lidar point into camera
        Y = P_rect_xx * R_rect_xx * RT * X;
        cv::Point pt;
        // pixel coordinates
        pt.x = Y.at<double>(0, 0) / Y.at<double>(2, 0); 
        pt.y = Y.at<double>(1, 0) / Y.at<double>(2, 0); 

        vector<vector<BoundingBox>::iterator> enclosingBoxes; // pointers to all bounding boxes which enclose the current Lidar point
        for (vector<BoundingBox>::iterator it2 = boundingBoxes.begin(); it2 != boundingBoxes.end(); ++it2)
        {
            // shrink current bounding box slightly to avoid having too many outlier points around the edges
            cv::Rect smallerBox;
            smallerBox.x = (*it2).roi.x + shrinkFactor * (*it2).roi.width / 2.0;
            smallerBox.y = (*it2).roi.y + shrinkFactor * (*it2).roi.height / 2.0;
            smallerBox.width = (*it2).roi.width * (1 - shrinkFactor);
            smallerBox.height = (*it2).roi.height * (1 - shrinkFactor);

            // check wether point is within current bounding box
            if (smallerBox.contains(pt))
            {
                enclosingBoxes.push_back(it2);
            }

        } // eof loop over all bounding boxes

        // check wether point has been enclosed by one or by multiple boxes
        if (enclosingBoxes.size() == 1)
        { 
            // add Lidar point to bounding box
            enclosingBoxes[0]->lidarPoints.push_back(*it1);
        }

    } // eof loop over all Lidar points
}

/* 
* The show3DObjects() function below can handle different output image sizes, but the text output has been manually tuned to fit the 2000x2000 size. 
* However, you can make this function work for other sizes too.
* For instance, to use a 1000x1000 size, adjusting the text positions by dividing them by 2.
*/
void show3DObjects(std::vector<BoundingBox> &boundingBoxes, cv::Size worldSize, cv::Size imageSize, bool bWait)
{
    // create topview image
    cv::Mat topviewImg(imageSize, CV_8UC3, cv::Scalar(255, 255, 255));

    for(auto it1=boundingBoxes.begin(); it1!=boundingBoxes.end(); ++it1)
    {
        // create randomized color for current 3D object
        cv::RNG rng(it1->boxID);
        cv::Scalar currColor = cv::Scalar(rng.uniform(0,150), rng.uniform(0, 150), rng.uniform(0, 150));

        // plot Lidar points into top view image
        int top=1e8, left=1e8, bottom=0.0, right=0.0; 
        float xwmin=1e8, ywmin=1e8, ywmax=-1e8;
        for (auto it2 = it1->lidarPoints.begin(); it2 != it1->lidarPoints.end(); ++it2)
        {
            // world coordinates
            float xw = (*it2).x; // world position in m with x facing forward from sensor
            float yw = (*it2).y; // world position in m with y facing left from sensor
            xwmin = xwmin<xw ? xwmin : xw;
            ywmin = ywmin<yw ? ywmin : yw;
            ywmax = ywmax>yw ? ywmax : yw;

            // top-view coordinates
            int y = (-xw * imageSize.height / worldSize.height) + imageSize.height;
            int x = (-yw * imageSize.width / worldSize.width) + imageSize.width / 2;

            // find enclosing rectangle
            top = top<y ? top : y;
            left = left<x ? left : x;
            bottom = bottom>y ? bottom : y;
            right = right>x ? right : x;

            // draw individual point
            cv::circle(topviewImg, cv::Point(x, y), 4, currColor, -1);
        }

        // draw enclosing rectangle
        cv::rectangle(topviewImg, cv::Point(left, top), cv::Point(right, bottom),cv::Scalar(0,0,0), 2);

        // augment object with some key data
        char str1[200], str2[200];
        sprintf(str1, "id=%d, #pts=%d", it1->boxID, (int)it1->lidarPoints.size());
        putText(topviewImg, str1, cv::Point2f(left-250, bottom+50), cv::FONT_ITALIC, 2, currColor);
        sprintf(str2, "xmin=%2.2f m, yw=%2.2f m", xwmin, ywmax-ywmin);
        putText(topviewImg, str2, cv::Point2f(left-250, bottom+125), cv::FONT_ITALIC, 2, currColor);  
    }

    // plot distance markers
    float lineSpacing = 2.0; // gap between distance markers
    int nMarkers = floor(worldSize.height / lineSpacing);
    for (size_t i = 0; i < nMarkers; ++i)
    {
        int y = (-(i * lineSpacing) * imageSize.height / worldSize.height) + imageSize.height;
        cv::line(topviewImg, cv::Point(0, y), cv::Point(imageSize.width, y), cv::Scalar(255, 0, 0));
    }

    // display image
    string windowName = "3D Objects";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 960, 960);
    cv::imshow(windowName, topviewImg);

    if(bWait)
    {
        cv::waitKey(0); // wait for key to be pressed
    }
}


// associate a given bounding box with the keypoints it contains
void clusterKptMatchesWithROI(BoundingBox &boundingBox, std::vector<cv::KeyPoint> &kptsPrev, std::vector<cv::KeyPoint> &kptsCurr, std::vector<cv::DMatch> &kptMatches)
{
    vector<cv::DMatch> filteredMatches;
    vector<double> d_data;
    boundingBox.kptMatches.clear();

    // filtered by containing in keypoints
    for(auto & kptMatch : kptMatches){
        auto curr_idx = kptMatch.trainIdx;
        auto prev_idx = kptMatch.queryIdx;
        if(boundingBox.roi.contains(kptsCurr.at(curr_idx).pt)){
            double dx = kptsCurr.at(curr_idx).pt.x - kptsPrev.at(prev_idx).pt.x;
            double dy = kptsCurr.at(curr_idx).pt.y - kptsPrev.at(prev_idx).pt.y;
            double d = sqrt(dx * dx + dy * dy);
            d_data.emplace_back(d);
            filteredMatches.emplace_back(kptMatch);
        }
    }

    // filtered by IQR
    pair<double, double> fence;
    setDataFence(d_data, fence);

    for(auto i = 0; i < filteredMatches.size(); i++){
        if(!isOutliers(d_data.at(i), fence))
            boundingBox.kptMatches.emplace_back(filteredMatches.at(i));
    }


}


// Compute time-to-collision (TTC) based on keypoint correspondences in successive images
void computeTTCCamera(std::vector<cv::KeyPoint> &kptsPrev, std::vector<cv::KeyPoint> &kptsCurr, 
                      std::vector<cv::DMatch> kptMatches, double frameRate, double &TTC, cv::Mat *visImg)
{
    // removing outliers by Interquartile Ranges (IQR)
    vector<cv::KeyPoint> newKptsPrev, newKptsCurr;

    // filtering for previous  key points
    vector<double> x, y;
    for(const auto& pt: kptsPrev){
        x.emplace_back(pt.pt.x);
        y.emplace_back(pt.pt.y);
    }
    pair<double, double> xFence, yFence;
    setDataFence(x, xFence);
    setDataFence(y, yFence);
    for(const auto& pt: kptsPrev){
        if(isOutliers(pt.pt.x, xFence) || isOutliers(pt.pt.y, yFence))
            continue;
        newKptsPrev.emplace_back(pt);
    }

    // filtering for previous lidar points
    x.clear(); y.clear();
    for(const auto& pt: kptsCurr){
        x.emplace_back(pt.pt.x);
        y.emplace_back(pt.pt.y);
    }
    setDataFence(x, xFence);
    setDataFence(y, yFence);
    for(const auto& pt: kptsCurr){
        if(isOutliers(pt.pt.x, xFence) || isOutliers(pt.pt.y, yFence) )
            continue;
        newKptsCurr.emplace_back(pt);
    }


    // compute distance ratios between all matched keypoints
    vector<double> distRatios; // stores the distance ratios for all keypoints between curr. and prev. frame
    for (auto it1 = kptMatches.begin(); it1 != kptMatches.end() - 1; ++it1)
    { // outer keypoint loop

        // get current keypoint and its matched partner in the prev. frame
        cv::KeyPoint kpOuterCurr = kptsCurr.at(it1->trainIdx);
        cv::KeyPoint kpOuterPrev = kptsPrev.at(it1->queryIdx);

        for (auto it2 = kptMatches.begin() + 1; it2 != kptMatches.end(); ++it2)
        { // inner keypoint loop

            double minDist = 100.0; // min. required distance

            // get next keypoint and its matched partner in the prev. frame
            cv::KeyPoint kpInnerCurr = kptsCurr.at(it2->trainIdx);
            cv::KeyPoint kpInnerPrev = kptsPrev.at(it2->queryIdx);

            // compute distances and distance ratios
            double distCurr = cv::norm(kpOuterCurr.pt - kpInnerCurr.pt);
            double distPrev = cv::norm(kpOuterPrev.pt - kpInnerPrev.pt);

            if (distPrev > std::numeric_limits<double>::epsilon() && distCurr >= minDist)
            { // avoid division by zero

                double distRatio = distCurr / distPrev;
                    distRatios.push_back(distRatio);
            }
        } // eof inner loop over all matched kpts
    }     // eof outer loop over all matched kpts

    // only continue if list of distance ratios is not empty
    if (distRatios.empty())
    {
        TTC = NAN;
        return;
    }

    // compute camera-based TTC from distance ratios
//    double meanDistRatio = std::accumulate(distRatios.begin(), distRatios.end(), 0.0) / distRatios.size();

    double dT = 1 / frameRate;
//    TTC = -dT / (1 - meanDistRatio);

    // TODO: STUDENT TASK (replacement for meanDistRatio)
    double medianDistRatio;
    if(distRatios.size() % 2 == 0)
        medianDistRatio = (distRatios.at(distRatios.size() / 2 - 1) +  distRatios.at(distRatios.size() / 2 )) * 0.5;
    else
        medianDistRatio = distRatios.at( uint (distRatios.size() / 2));
    TTC = -dT / (1 - medianDistRatio);
}


void computeTTCLidar(std::vector<LidarPoint> &lidarPointsPrev,
                     std::vector<LidarPoint> &lidarPointsCurr, double frameRate, double &TTC)
{
    // removing outliers by Interquartile Ranges (IQR)
    vector<LidarPoint> newLidarPointsPrev, newLidarPointsCurr;

    // filtering for previous lidar points
    vector<double> x, y, z;
    for(const auto& pt: lidarPointsPrev){
        x.emplace_back(pt.x);
        y.emplace_back(pt.y);
        z.emplace_back(pt.z);
    }
    pair<double, double> xFence, yFence, zFence;
    setDataFence(x, xFence);
    setDataFence(y, yFence);
    setDataFence(z, zFence);
    for(const auto& pt: lidarPointsPrev){
        if(isOutliers(pt.x, xFence) || isOutliers(pt.y, yFence) || isOutliers(pt.z, zFence))
            continue;
        newLidarPointsPrev.emplace_back(pt);
    }

    // filtering for previous lidar points
    x.clear(); y.clear(); z.clear();
    for(const auto& pt: lidarPointsCurr){
        x.emplace_back(pt.x);
        y.emplace_back(pt.y);
        z.emplace_back(pt.z);
    }
    setDataFence(x, xFence);
    setDataFence(y, yFence);
    setDataFence(z, zFence);
    for(const auto& pt: lidarPointsCurr){
        if(isOutliers(pt.x, xFence) || isOutliers(pt.y, yFence) || isOutliers(pt.z, zFence))
            continue;
        newLidarPointsCurr.emplace_back(pt);
    }


    // auxiliary variables
    double dT = 1.0 / frameRate;        // time between two measurements in seconds
    double laneWidth = 4.0; // assumed width of the ego lane

    // find closest distance to Lidar points within ego lane
    double minXPrev = 1e9, minXCurr = 1e9;
    for (auto it = newLidarPointsPrev.begin(); it != newLidarPointsPrev.end(); ++it)
    {
        minXPrev = minXPrev > it->x ? it->x : minXPrev;
    }

    for (auto it = newLidarPointsCurr.begin(); it != newLidarPointsCurr.end(); ++it)
    {
        minXCurr = minXCurr > it->x ? it->x : minXCurr;
    }

    // compute TTC from both measurements
    TTC = minXCurr * dT / (minXPrev - minXCurr);}


void matchBoundingBoxes(std::vector<cv::DMatch> &matches, std::map<int, int> &bbBestMatches, DataFrame &prevFrame, DataFrame &currFrame)
{
    // generteate a map from (previous_bouding box id, current_bouding box id) to the counts of matches
    map<pair<int, int>, int> bb_matches;
    for(const auto& bbp: prevFrame.boundingBoxes){
        for(const auto& bbc: currFrame.boundingBoxes){
            bb_matches.emplace(pair<int, int>(bbp.boxID, bbc.boxID), 0);
        }
    }

    for(const auto& match: matches){
        int queryIdx = match.queryIdx;
        int trainIdx = match.trainIdx;
        bool in_prev_bb(false), in_curr_bb(false);
        int prev_bb_idx(-1), curr_bb_idx(-1); // TODO:  maybe there is a better way?

        for(const auto& bbp: prevFrame.boundingBoxes){
            auto kpp = prevFrame.keypoints.at(queryIdx);
            prev_bb_idx = bbp.boxID;

            if(bbp.roi.contains(kpp.pt)){
                for(const auto& bbc: currFrame.boundingBoxes){
                    auto kpc = currFrame.keypoints.at(trainIdx);
                    if(bbc.roi.contains(kpc.pt)){
                        curr_bb_idx = bbc.boxID;
                        pair<int, int> index_pair{prev_bb_idx, curr_bb_idx};
                        bb_matches[index_pair] = bb_matches[index_pair] + 1;
                    }
                }

            }
        }


        // find the current bounding box with the highest number of matches for each previous bouding boxes
        for(const auto& bbp: prevFrame.boundingBoxes){
            prev_bb_idx = bbp.boxID;
            int max_matches = 0;
            pair<int, int> max_pair{-1, -1};

            for(const auto& bbc: prevFrame.boundingBoxes){
                curr_bb_idx = bbc.boxID;
                pair<int, int> index_pair{prev_bb_idx, curr_bb_idx};
                if(bb_matches[index_pair] > max_matches){
                    max_matches = bb_matches[index_pair];
                    max_pair = index_pair;
                }
            }

            if(max_matches > 0) // matches are found between previous bounding box and current bounding box
                bbBestMatches.emplace(max_pair.first, max_pair.second);
        }


    }

}

void setDataFence(std::vector<double> data, std::pair<double, double> &fence, double factor) {
    sort(data.begin(), data.end());
    double Q1 =  data.at(static_cast<int>(data.size() * 0.25)); // Q1
    double Q3 =  data.at(static_cast<int>(data.size() * 0.75)); // Q3
    double IQR = Q3 - Q1 ;
    fence.first = Q1 - factor * IQR;
    fence.second = Q3 + factor * IQR;
    // Ref: https://www.purplemath.com/modules/boxwhisk3.htm
    // Ref: Zwillinger, D., Kokoska, S. (2000) CRC Standard Probability and Statistics Tables and Formulae, CRC Press. ISBN 1-58488-059-7 page 18.

}

bool isOutliers(double value, std::pair<double, double> fences) {
    if(value > fences.second || value < fences.first)
        return true;
    else
        return false;
}
