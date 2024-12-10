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
  std::string left_image_name;		    //!< image file name
  std::string right_image_name;		    //!< image file name
  int wait_t;                     //!< waiting time
  int orb_t;                     //!< n. features for ORB
};

bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.wait_t=0;
  args.orb_t=500;

  while ((c = getopt (argc, argv, "hl:r:t:o:")) != -1)
    switch (c)
    {
      case 'r':
	args.right_image_name = optarg;
	break;
      case 'l':
	args.left_image_name = optarg;
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
  char lframe_name[256];
  char rframe_name[256];
  bool exit_loop = false;

  std::cout<<"Simple program."<<std::endl;

  //////////////////////
  //parse argument listd:
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
    if(args.left_image_name.find('%') != std::string::npos)
      sprintf(lframe_name,(const char*)(args.left_image_name.c_str()),frame_number);
    else //single frame case
      sprintf(lframe_name,"%s",args.left_image_name.c_str());

    //multi frame case
    if(args.right_image_name.find('%') != std::string::npos)
      sprintf(lframe_name,(const char*)(args.right_image_name.c_str()),frame_number);
    else //single frame case
      sprintf(rframe_name,"%s",args.right_image_name.c_str());


    //opening file
    std::cout<<"Opening " << lframe_name << " " << rframe_name << std::endl;

    cv::Mat limage = cv::imread(lframe_name, CV_8UC1);
    if(limage.empty())
    {
      std::cout << "Unable to open " << lframe_name << std::endl;
      return 1;
    }

    cv::Mat rimage = cv::imread(rframe_name, CV_8UC1);
    if(rimage.empty())
    {
      std::cout << "Unable to open " << rframe_name << std::endl;
      return 1;
    }



    std::vector<cv::KeyPoint> rsift_keypoints, lsift_keypoints, surf_keypoints, orb_keypoints, harris_keypoints;

    // HARRIS
    std::vector<cv::Point2f> corners;
    int maxCorners = 0;
    double qualityLevel = 0.01;
    double minDistance = 10;
    int blockSize = 3;
    bool useHarrisDetector = true;
    double k = 0.04;

    cv::goodFeaturesToTrack(rimage,corners,maxCorners,qualityLevel,minDistance,cv::noArray(),blockSize,useHarrisDetector,k ); 
    std::transform(corners.begin(), corners.end(), std::back_inserter(harris_keypoints), [](const cv::Point2f & p){ return cv::KeyPoint(p.x,p.y,3.0);} ); // applica funzione a range vector e memorizza in altro range 3->size del keypoint


    std::cout << "DEBUG: the number of Harris Corners is " << corners.size() << std::endl;

    // SIFT
    cv::Ptr<cv::SiftFeatureDetector> sift_detector = cv::SiftFeatureDetector::create();
    sift_detector->detect(rimage, rsift_keypoints);
    sift_detector->detect(limage, lsift_keypoints);
    std::cout << "DEBUG: the number of SIFT keypoints in img 1 is " << rsift_keypoints.size() << std::endl;
    std::cout << "DEBUG: the number of SIFT keypoints in img 2 is " << lsift_keypoints.size() << std::endl;


    // compute SIFT descriptors
    cv::Ptr<cv::SiftDescriptorExtractor> sift_extractor = cv::SiftDescriptorExtractor::create();
    cv::Mat rdes, ldes;

    sift_extractor->compute(rimage, rsift_keypoints, rdes);
    sift_extractor->compute(limage, lsift_keypoints, ldes);

    // compute matching
    cv::FlannBasedMatcher matcher;
    std::vector<cv::DMatch> sift_matches, sift_good_matches;

    matcher.match(rdes, ldes, sift_matches);

    double max_dist = 0; double min_dist = 1000;

    // compute min and max distances
    for( size_t i = 0; i < sift_matches.size(); i++ )
    { 
      double dist = sift_matches[i].distance;
      if( dist < min_dist ) min_dist = dist;
      if( dist > max_dist ) max_dist = dist;
    }
    printf("SIFT Max dist : %f \n", max_dist );
    printf("SIFT Min dist : %f \n", min_dist );

    // threshold matches 
    for( size_t i = 0; i < sift_matches.size(); i++ )
      if(sift_matches[i].distance <= std::max(min_dist * 2, .04))
	sift_good_matches.push_back(sift_matches[i]);

    // draw matches
    cv::Mat sift_match_result;
    cv::drawMatches(rimage, rsift_keypoints, limage, lsift_keypoints, sift_good_matches, sift_match_result); 

    // show matches
    cv::namedWindow("SIFT matches", cv::WINDOW_NORMAL);
    cv::imshow("SIFT matches", sift_match_result);




    // SURF
    // int minHessian = 400;
    // cv::Ptr<cv::xfeatures2d::SurfFeatureDetector> surf_detector = cv::xfeatures2d::SurfFeatureDetector::create(minHessian);
    // surf_detector->detect(rimage, surf_keypoints);
    // std::cout << "DEBUG: the number of SURF keypoints is " << surf_keypoints.size() << std::endl;


    // ORB
    cv::Ptr<cv::ORB> orb_detector = cv::ORB::create(rsift_keypoints.size());
    orb_detector->setScoreType(cv::ORB::HARRIS_SCORE);
    orb_detector->detect(rimage, orb_keypoints);
    std::cout << "DEBUG: the number of ORB keypoints is " << orb_keypoints.size() << std::endl;

    // Add results to rimages
    cv::Mat harris_output, rsift_output, lsift_output, surf_output, orb_output;
    cv::drawKeypoints(rimage, rsift_keypoints, rsift_output);
    cv::drawKeypoints(limage, lsift_keypoints, lsift_output);
    cv::drawKeypoints(rimage, surf_keypoints, surf_output);
    cv::drawKeypoints(rimage, orb_keypoints, orb_output);
    cv::drawKeypoints(rimage, harris_keypoints, harris_output);




    //display image
    cv::namedWindow("image 1", cv::WINDOW_NORMAL);
    cv::imshow("image 1", rimage);
    cv::namedWindow("image 2", cv::WINDOW_NORMAL);
    cv::imshow("image 2", limage);

    cv::namedWindow("Harris", cv::WINDOW_NORMAL);
    cv::imshow("Harris", harris_output);

    cv::namedWindow("ORB", cv::WINDOW_NORMAL);
    cv::imshow("ORB", orb_output);

    cv::namedWindow("SIFT 1", cv::WINDOW_NORMAL);
    cv::imshow("SIFT 1", rsift_output);

    cv::namedWindow("SIFT 2", cv::WINDOW_NORMAL);
    cv::imshow("SIFT 2", lsift_output);

    // cv::namedWindow("SURF", cv::WINDOW_NORMAL);
    // cv::imshow("SURF", surf_output);


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
