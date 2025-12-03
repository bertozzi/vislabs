//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <ctime>

//options

struct ArgumentList {
  std::string image_name;                   //!< image file name
  std::string points_name;                   //!< points file name
  int dist_t;                               //!< RANSAC distance
  int wait_t;                               //!< waiting time
};

bool ParseInputs(ArgumentList& args, int argc, char **argv);


double Distance2Line(cv::Point line_start, cv::Point line_end, cv::Point point)
{
  double normalLength = hypot(line_end.x - line_start.x, line_end.y - line_start.y);
  return fabs((double)((point.x - line_start.x) * (line_end.y - line_start.y) - (point.y - line_start.y) * (line_end.x - line_start.x)) / normalLength);
}

int main(int argc, char **argv)
{
  int frame_number = 0;
  char frame_name[256];
  bool exit_loop = false;
  int imreadflags = cv::IMREAD_COLOR; 

  std::cout<<"Simple program."<<std::endl;

  srand(time(0)); // initialize random number generator

  //////////////////////
  //parse argument list:
  //////////////////////
  ArgumentList args;
  if(!ParseInputs(args, argc, argv)) {
    exit(0);
  }

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

    cv::Mat image = cv::imread(frame_name, imreadflags);
    if(image.empty())
    {
      std::cout<<"Unable to open "<<frame_name<<std::endl;
      return 1;
    }

    std::cout << "The image has " << image.channels() << 
      " channels, the size is " << image.rows << "x" << image.cols << " pixels " <<
      " the type is " << image.type() <<
      " the pixel size is " << image.elemSize() <<
      " and each channel is " << image.elemSize1() << (image.elemSize1()>1?" bytes":" byte") << std::endl;

    // OUTPUT IMAGE
    cv::Mat out = image/3; 

    /**** YOUR CODE HERE ****/

    // STEP 1: read points 
    // STEP 2 & 3: RANSAC


    cv::namedWindow("output image", cv::WINDOW_NORMAL); // cv::WINDOW_AUTOSIZE if you want the window to adapt to image size
    cv::imshow("output image", out);

    //wait for key or timeout
    unsigned char key = cv::waitKey(args.wait_t);
    std::cout<<"key "<<int(key)<<std::endl;

    //here you can implement some looping logic using key value:
    // - pause
    // - stop
    // - step back
    // - step forward
    // - loop on the same frame

    switch(key)
    {
      case 'q':
	exit_loop = 1;
	break;
    }
    frame_number++;
  }

  return 0;
}


#include <unistd.h>
bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.dist_t = 30;
  args.wait_t = 0;

  while ((c = getopt (argc, argv, "hi:t:d:p:")) != -1)
    switch (c)
    {
      case 'd':
	args.dist_t = atoi(optarg);
	break;
      case 't':
	args.wait_t = atoi(optarg);
	break;
      case 'p':
	args.points_name = optarg;
	break;
      case 'i':
	args.image_name = optarg;
	break;
      case 'h':
      default:
	std::cout<<"Allowed options:"<<std::endl<<
	  "   -h                       produce help message"<<std::endl<<
	  "   -i arg                   image name. Use %0xd format for multiple images."<<std::endl<<
	  "   -p arg                   points file name. "<<std::endl<<
	  "   -d arg                   RANSAC distance (default 30)"<<std::endl<<
	  "   -t arg                   wait before next frame (ms)"<<std::endl<<std::endl;
	return false;
    }
  if(args.points_name.empty() or args.image_name.empty())
  {
    std::cerr << "Missing image file name or points file name" << std::endl;
    exit(1);
  }

  return true;
}



