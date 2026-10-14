//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>

struct ArgumentList {
  std::string image_name;  //!< image file name
  int wait_t;              //!< waiting time
  int ksize;               //!< kernel size
};

bool ParseInputs(ArgumentList& args, int argc, char **argv);

int main(int argc, char **argv)
{
  int frame_number = 0;
  char frame_name[256];
  bool exit_loop = false;

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

    cv::Mat image = cv::imread(frame_name);
    if(image.empty())
    {
      std::cout<<"Unable to open "<<frame_name<<std::endl;
      return 1;
    }

    //display image
    cv::namedWindow("original image", cv::WINDOW_NORMAL);
    cv::imshow("original image", image);


    // PROCESSING

    // convert to grey scale for following processings
    cv::Mat grey;

    cv::cvtColor(image, grey, cv::COLOR_BGR2GRAY);
    cv::namedWindow("grey", cv::WINDOW_NORMAL);
    cv::imshow("grey", grey);

    cv::Mat blurred;

    // BOX FILTERING OpenCV way
    // void cv::boxFilter(InputArray src, OutputArray dst, int ddepth, Size ksize, Point anchor = Point(-1,-1), bool normalize = true, int borderType = BORDER_DEFAULT )
    int64 t_boxfilter_start = cv::getTickCount(); // start the counter (also std::chrono() can be used)
    cv::boxFilter(grey, blurred, CV_8U, cv::Size(args.ksize, args.ksize));
    double t_boxfilter_ms = 1000.0 * (cv::getTickCount() - t_boxfilter_start) / cv::getTickFrequency();
    std::cout << "cv::boxFilter()  elapsed time: " << t_boxfilter_ms << " ms" << std::endl;
    cv::namedWindow("Box filter Smoothing", cv::WINDOW_NORMAL);
    cv::imshow("Box filter Smoothing", blurred);

    // BOX FILTERING OpenCV using cv::filter2D()
    cv::Mat custom_kernel(args.ksize, args.ksize, CV_32FC1, 1.0/(args.ksize*args.ksize));
    // also possible as cv::Mat custom_kernel = 1.0/(args.ksize*args.ksize) * cv::Mat::ones(args.ksize, args.ksize, CV_32FC1);
    cv::Mat custom_blurred;
    int64 t_filter2D_start = cv::getTickCount();
    cv::filter2D(grey, custom_blurred, CV_32F, custom_kernel);
    double t_filter2D_ms = 1000.0 * (cv::getTickCount() - t_filter2D_start) / cv::getTickFrequency();
    std::cout << "cv::filter2D()   elapsed time: " << t_filter2D_ms << " ms" << std::endl;
    cv::convertScaleAbs(custom_blurred, custom_blurred); // from CV_32F to CV_8U
    cv::namedWindow("Box filter Smoothing (custom)", cv::WINDOW_NORMAL);
    cv::imshow("Box filter Smoothing (custom)", custom_blurred);
    blurred.copyTo(grey);

    // INTEGRAL IMAGE
    //
    // Confrontate qui il tempo delle vostre funzioni (immagine integrale +
    // calcolo del filtro di media) con t_boxfilter_ms e t_filter2D_ms misurati sopra,
    // usando lo stesso schema: cv::getTickCount()/cv::getTickFrequency() prima e dopo
    // la vostra elaborazione (oppure std::chrono::high_resolution_clock, se preferite).

    //wait for key or timeout
    unsigned char key = cv::waitKey(args.wait_t);
    std::cout<<"key "<<int(key)<<std::endl;

    //here you can implement some looping logic using key value:
    switch(key)
    {
      case 'q':
	exit(0);
	break;
    }

    frame_number++;
  }

  return 0;
}

#include <unistd.h>
bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.wait_t = 0;
  args.ksize  = 3;

  while ((c = getopt (argc, argv, "hi:t:k:")) != -1)
    switch (c)
    {
      case 'k':
	args.ksize  = atoi(optarg);
	break;
      case 't':
	args.wait_t = atoi(optarg);
	break;
      case 'i':
	args.image_name = optarg;
	break;
      case 'h':
      default:
	std::cout<<"usage: " << argv[0] << " -i <image_name>"<<std::endl;
	std::cout<<"exit:  type q"<<std::endl<<std::endl;
	std::cout<<"Allowed options:"<<std::endl<<
	  "   -h                       produce help message"<<std::endl<<
	  "   -i arg                   image name. Use %0xd format for multiple images."<<std::endl<<
	  "   -t arg                   wait before next frame (ms)"<<std::endl<<
	  "   -k arg                   kernel size (pixels)       "<<std::endl<<std::endl;
	return false;
    }
  return true;
}

