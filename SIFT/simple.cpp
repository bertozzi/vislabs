//OpneCV
//OpneCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

struct ArgumentList {
  std::string image_name;		    //!< image file name
  int wait_t;                     //!< waiting time
  int orb_t;                     //!< n. features for ORB
};

bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.wait_t=0;
  args.orb_t=500;

  while ((c = getopt (argc, argv, "hi:t:o:")) != -1)
    switch (c)
    {
      case 'i':
	args.image_name = optarg;
	break;
      case 't':
	args.wait_t = atoi(optarg);
	break;
      case 'o':
	args.orb_t = atoi(optarg);
	break;
      case 'h':
      default:
	std::cout<<"usage: " << argv[0] << " -i <image_name>"<<std::endl;
	std::cout<<"exit:  type q"<<std::endl<<std::endl;
	std::cout<<"Allowed options:"<<std::endl<<
	  "   -h                         produce help message"<<std::endl<<
	  "   -i arg                   image name. Use %0xd format for multiple images."<<std::endl<<
	  "   -t arg                   wait before next frame (ms) [default = 0]"<<std::endl<<std::endl<<std::endl;
	return false;
    }
  return true;
}

int main(int argc, char **argv)
{
  int frame_number = 0;
  char frame_name[256];
  bool exit_loop = false;

  std::cout<<"Simple program."<<std::endl;

  //////////////////////
  //parse argument list:
  //////////////////////
  ArgumentList args;
  if(!ParseInputs(args, argc, argv)) {
    return 1;
  }

  ////data
  //prev frame

  std::vector<cv::Mat> m_pframe_bg;

  while(!exit_loop)
  {
    //generating file name
    //
    //multi frame case
    if(args.image_name.find('%') != std::string::npos)
      sprintf(frame_name,(const char*)(args.image_name.c_str()),frame_number);
    else //single frame case
      sprintf(frame_name,"%s",args.image_name.c_str());

    //opening file
    std::cout<<"Opening "<<frame_name<<std::endl;

    cv::Mat image = cv::imread(frame_name, CV_8UC1);
    if(image.empty())
    {
      std::cout<<"Unable to open "<<frame_name<<std::endl;
      return 1;
    }




    std::vector<cv::KeyPoint> sift_keypoints, surf_keypoints, orb_keypoints, harris_keypoints;

    // HARRIS
    std::vector<cv::Point2f> corners;
    int maxCorners = 0;
    double qualityLevel = 0.01;
    double minDistance = 10;
    int blockSize = 3;
    bool useHarrisDetector = true;
    double k = 0.04;

    cv::goodFeaturesToTrack(image,corners,maxCorners,qualityLevel,minDistance,cv::noArray(),blockSize,useHarrisDetector,k ); 
    std::transform(corners.begin(), corners.end(), std::back_inserter(harris_keypoints), [](const cv::Point2f & p){ return cv::KeyPoint(p.x,p.y,3.0);} ); // applica funzione a range vector e memorizza in altro range 3->size del keypoint



    // SIFT
    cv::Ptr<cv::SiftFeatureDetector> sift_detector = cv::SiftFeatureDetector::create();
    sift_detector->detect(image, sift_keypoints);

    // SURF
    int minHessian = 400;
    cv::Ptr<cv::xfeatures2d::SurfFeatureDetector> surf_detector = cv::xfeatures2d::SurfFeatureDetector::create(minHessian);
    surf_detector->detect(image, surf_keypoints);


    // ORB
    cv::Ptr<cv::ORB> orb_detector = cv::ORB::create(sift_keypoints.size());
    orb_detector->setScoreType(cv::ORB::HARRIS_SCORE);
    orb_detector->detect(image, orb_keypoints);

    // Add results to images
    cv::Mat harris_output, sift_output, surf_output, orb_output;
    cv::drawKeypoints(image, sift_keypoints, sift_output);
    cv::drawKeypoints(image, surf_keypoints, surf_output);
    cv::drawKeypoints(image, orb_keypoints, orb_output);
    cv::drawKeypoints(image, harris_keypoints, harris_output);




    //display image
    cv::namedWindow("image", cv::WINDOW_NORMAL);
    cv::imshow("image", image);

    cv::namedWindow("Harris", cv::WINDOW_NORMAL);
    cv::imshow("Harris", harris_output);

    cv::namedWindow("ORB", cv::WINDOW_NORMAL);
    cv::imshow("ORB", orb_output);

    cv::namedWindow("SIFT", cv::WINDOW_NORMAL);
    cv::imshow("SIFT", sift_output);

    cv::namedWindow("SURF", cv::WINDOW_NORMAL);
    cv::imshow("SURF", surf_output);


    //wait for key or timeout
    unsigned char key = cv::waitKey(args.wait_t);
    std::cout<<"key "<<int(key)<<std::endl;

    //here you can implement some looping logic using key value:
    // - pause
    // - stop
    // - step back
    // - step forward
    // - loop on the same frame
    if(key == 'q')
      exit_loop = true;

    frame_number++;
  }

  return 0;
}
