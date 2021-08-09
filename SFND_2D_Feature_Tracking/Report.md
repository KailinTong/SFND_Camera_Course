# SFND 2D Feature Tracking

<img src="images/keypoints.png" width="820" height="248" />

The idea of the camera course is to build a collision detection system - that's the overall goal for the Final Project. As a preparation for this, you will now build the feature tracking part and test various detector / descriptor combinations to see which ones perform best. This mid-term project consists of four parts:

* First, you will focus on loading images, setting up data structures and putting everything into a ring buffer to optimize memory load. 
* Then, you will integrate several keypoint detectors such as HARRIS, FAST, BRISK and SIFT and compare them with regard to number of keypoints and speed. 
* In the next part, you will then focus on descriptor extraction and matching using brute force and also the FLANN approach we discussed in the previous lesson. 
* In the last part, once the code framework is complete, you will test the various algorithms in different combinations and compare them with regard to some performance measures. 

See the classroom instruction and code comments for more details on each of these parts. Once you are finished with this project, the keypoint matching part will be set up and you can proceed to the next lesson, where the focus is on integrating Lidar points and on object detection using deep-learning. 

## Dependencies for Running Locally
1. cmake >= 2.8
 * All OSes: [click here for installation instructions](https://cmake.org/install/)

2. make >= 4.1 (Linux, Mac), 3.81 (Windows)
 * Linux: make is installed by default on most Linux distros
 * Mac: [install Xcode command line tools to get make](https://developer.apple.com/xcode/features/)
 * Windows: [Click here for installation instructions](http://gnuwin32.sourceforge.net/packages/make.htm)

3. OpenCV >= 4.1
 * All OSes: refer to the [official instructions](https://docs.opencv.org/master/df/d65/tutorial_table_of_content_introduction.html)
 * This must be compiled from source using the `-D OPENCV_ENABLE_NONFREE=ON` cmake flag for testing the SIFT and SURF detectors. If using [homebrew](https://brew.sh/): `$> brew install --build-from-source opencv` will install required dependencies and compile opencv with the `opencv_contrib` module by default (no need to set `-DOPENCV_ENABLE_NONFREE=ON` manually). 
 * The OpenCV 4.1.0 source code can be found [here](https://github.com/opencv/opencv/tree/4.1.0)

4. gcc/g++ >= 5.4
  * Linux: gcc / g++ is installed by default on most Linux distros
  * Mac: same deal as make - [install Xcode command line tools](https://developer.apple.com/xcode/features/)
  * Windows: recommend using either [MinGW-w64](http://mingw-w64.org/doku.php/start) or [Microsoft's VCPKG, a C++ package manager](https://docs.microsoft.com/en-us/cpp/build/install-vcpkg?view=msvc-160&tabs=windows). VCPKG maintains its own binary distributions of OpenCV and many other packages. To see what packages are available, type `vcpkg search` at the command prompt. For example, once you've _VCPKG_ installed, you can install _OpenCV 4.1_ with the command:
```bash
c:\vcpkg> vcpkg install opencv4[nonfree,contrib]:x64-windows
```
Then, add *C:\vcpkg\installed\x64-windows\bin* and *C:\vcpkg\installed\x64-windows\debug\bin* to your user's _PATH_ variable. Also, set the _CMake Toolchain File_ to *c:\vcpkg\scripts\buildsystems\vcpkg.cmake*.


## Basic Build Instructions

1. Clone this repo.
2. Make a build directory in the top level directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./2D_feature_tracking`.

# **SFND: 2D Feature Tracking**

<img src="images/keypoints.png" width="820" height="248" />

## Dependencies for Running Locally
* cmake >= 2.8
    * All OSes: [click here for installation instructions](https://cmake.org/install/)
* make >= 4.1 (Linux, Mac), 3.81 (Windows)
    * Linux: make is installed by default on most Linux distros
    * Mac: [install Xcode command line tools to get make](https://developer.apple.com/xcode/features/)
    * Windows: [Click here for installation instructions](http://gnuwin32.sourceforge.net/packages/make.htm)
* OpenCV >= 4.1
    * This must be compiled from source using the `-D OPENCV_ENABLE_NONFREE=ON` cmake flag for testing the SIFT and SURF detectors.
    * The OpenCV 4.1.0 source code can be found [here](https://github.com/opencv/opencv/tree/4.1.0)
* gcc/g++ >= 5.4
    * Linux: gcc / g++ is installed by default on most Linux distros
    * Mac: same deal as make - [install Xcode command line tools](https://developer.apple.com/xcode/features/)
    * Windows: recommend using [MinGW](http://www.mingw.org/)

## Basic Build Instructions

1. Clone this repo.
2. Make a build directory in the top level directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./2D_feature_tracking`.

### **Rubric**

#### **MP1: Data Buffer Optimisation**
A ring buffer of size dataBufferSize is implemented.  The fisrt element is erased by` dataBuffer.erase(dataBuffer.begin())` if the buffer reaches the maximal size. 

#### **MP2: Keypoint Detection**
Implemented detectors [HARRIS, FAST, BRISK, ORB, AKAZE, SIFT] through following functions:


`````detKeypointsHarris(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis=false);

detKeypointsShiTomasi(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis=false);

detKeypointsFAST(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis=false);

detKeypointsBRISK(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis=false);

detKeypointsSIFT(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis=false);

detKeypointsORB(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis=false);

detKeypointsAKAZE(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis=false);`
`````

#### **MP3: Keypoint Removal**

The keypoints outside the pre-definded rectangle have been removed.

#### **MP4: Keypoint Descriptors**

Implemented descriptors [BRIEF, ORB, FREAK, AKAZE, SIFT]

#### **MP5: Descriptor Matching**

Implemented FLANN matching and K-Nearest Neighbour selection.

#### **MP6: Descriptor Distance Ratio**

Implemented distance ratio to filter points for KNN.

#### **MP7: Number of Keypoints on Preceding Vehicle**

| Detector      | Number of Keypoints in ROI |
| ------------- | -------------------------- |
| **AKAZE**     | -                      |
| **BRISK**     | -                      |
| **FAST**      | 5063                       |
| **HARRIS**    | 1836                       |
| **ORB**       | -                       |
| **SHITOMASI** | 1370                       |
| **SIFT**      | 1438                       |

#### **MP8: Number of Matched Keypoints**

| Detector\Descriptor | AKAZE | BRIEF | BRISK | FREAK | ORB  | SIFT |
| ------------------- | ----- | ----- | ----- | ----- | ---- | ---  |
| **AKAZE**           | - | -  | -  | -  | - | - |
| **BRISK**           | NaN   | -  | -  | -  | - | - |
| **FAST**            | NaN   | 320  | 256   | 251   | 306 | - |
| **HARRIS**          | NaN   | 56   | 18   | 32   | 39  | -  |
| **ORB**             | NaN   | -   | -   | -   | -  | -  |
| **SHITOMASI**       | NaN   | 115   | 95   | 90   | 104  | -  |
| **SIFT**            | NaN   | 86   | 64   | 64   | NaN  | -  |


#### **MP9:  Processing Time and Top Three Detector/Descriptor Combinations** 
<img src="time.jpg" width="820" height="1200" />



Reference Readme Template:
https://github.com/danielkelshaw/SensorFusionND/edit/main/p2-feature-tracking/README.md