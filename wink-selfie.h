//ECE 2574
//Programmer: MINH NGUYEN
//4/28//2017
//File: wink-selfie.h
#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <list>
#include <vector>
#include <queue>
#include <direct.h>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <windows.h>

using namespace std;
using namespace cv;

enum WinkEyeDetection //Filter left, right or NA
{
	LEFT,
	RIGHT,
	NA
};

struct FaceRecognition //Storing Face info
{
	cv::Rect * faceRectangle;
	int numberOfEyes;
	WinkEyeDetection winkSide;
};

const int FRAMES_TO_REMEMBER = 17;//17
const int FRAME_BLINK_DELAY = 1;
