//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>

//options

struct ArgumentList {
  std::string image_name;                   //!< image file name
  int wait_t;                               //!< waiting time
};

bool ParseInputs(ArgumentList& args, int argc, char **argv);

int main(int argc, char **argv)
{
  int frame_number = 0;
  char frame_name[256];
  bool exit_loop = false;
  int imreadflags = cv::IMREAD_COLOR; 
  int k = 9;

  std::cout<<"Simple program."<<std::endl;

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

    cv::Mat orig_image = cv::imread(frame_name, imreadflags);
    if(orig_image.empty())
    {
      std::cout<<"Unable to open "<<frame_name<<std::endl;
      return 1;
    }

    cv::Mat image;
    orig_image.convertTo(image, CV_32F);

    std::cout << "The image has " << image.channels() << 
      " channels, the size is " << image.rows << "x" << image.cols << " (" << image.total() << ") pixels " <<
      " the type is " << image.type() <<
      " the pixel size is " << image.elemSize() <<
      " and each channel is " << image.elemSize1() << (image.elemSize1()>1?" bytes":" byte") << std::endl;
#if 0
    std::vector<cv::Mat> bgr;
    cv::split(image, bgr);
    cv::Mat img_data =cv::Mat::zeros(image.cols*image.rows, 3, CV_32F);
    for(int i=0; i<image.cols*image.rows; i++) {
        //img_data.at<float>(i,0) = (i/image.cols) / float(image.rows);
        //img_data.at<float>(i,1) = (i%image.cols) / float(image.cols);
        img_data.at<float>(i,0) = bgr[0].data[i] / 255.0;
        img_data.at<float>(i,1) = bgr[1].data[i] / 255.0;
        img_data.at<float>(i,2) = bgr[2].data[i] / 255.0;
    }
#endif

    std::vector<cv::Vec6f> img_data;
    for (int r = 0; r < image.rows; r++) {
      for (int c = 0; c < image.cols; c++) {
	cv::Vec3f pixel = image.at<cv::Vec3f>(r, c);
	img_data.push_back(cv::Vec6f(pixel[0], pixel[1], pixel[2], float(c)/image.cols*255, float(r)/image.rows*255, 0));
	std::cout << pixel[0] << " ";
      }
    }

#if 0
    std::cout << "The reshaped image has " << img_data.channels() << 
      " channels, the size is " << img_data.rows << "x" << img_data.cols << " (" << img_data.total() << ") pixels " <<
      " the type is " << img_data.type() <<
      " the pixel size is " << img_data.elemSize() <<
      " and each channel is " << img_data.elemSize1() << (img_data.elemSize1()>1?" bytes":" byte") << std::endl;
#endif

    cv::TermCriteria criteria(cv::TermCriteria::EPS  + cv::TermCriteria::MAX_ITER, 10, 1.0);
    cv::Mat labels, centers;

    //Run the k-means clustering algorithm on the pixel values
    double compactness = cv::kmeans(
	img_data,              // vector of values
	k,                     // K!
	labels,                // labels of different clusters
	criteria,              // stop after 10 iterations and 
        10,                    // attempts
	cv::KMEANS_PP_CENTERS, // K-Means++ selection of initial guess for centroids
       	centers
	);

    std::cout << "Compactness: " << compactness << std::endl;
    std::cout << "Centers size: " << centers.rows << "x" << centers.cols << "\n" << centers << std::endl;
    std::cout << "Labels size: " << labels.rows << "x" << labels.cols << " " << labels.channels() << " channel and type " << labels.type() <<  std::endl; // type 4 -> 32S

    // we repaint the original images assigning for each pixel
    // the same color value of the center of the group
    int *labels_data      = (int32_t *)labels.data;
    uint8_t *image_data   = (uint8_t *)orig_image.data;
    float *centroids_data = (float *)centers.data;
    for(int r=0; r<orig_image.rows; ++r)
      for(int c=0; c<orig_image.cols; ++c)
      {
	int label = labels_data[r*image.cols + c];
	image_data[(r*image.cols + c)*3 + 0] = centroids_data[label*3 + 0];
	image_data[(r*image.cols + c)*3 + 1] = centroids_data[label*3 + 1];
	image_data[(r*image.cols + c)*3 + 2] = centroids_data[label*3 + 2];
      }



    //display image
    cv::namedWindow("original image", cv::WINDOW_NORMAL); // cv::WINDOW_AUTOSIZE if you want the window to adapt to image size
    cv::imshow("original image", orig_image);

    //wait for key or timeout
    unsigned char key = cv::waitKey(args.wait_t);
    std::cout<<"key "<<int(key)<<std::endl;

    switch(key)
    {
      case 'K':
	++k;
	std::cout << "k = "<< k << std::endl;
	break;
      case 'k':
	--k;
	std::cout << "k = "<< k << std::endl;
	break;
      case 'q':
	exit_loop = 1;
	break;
    }

    frame_number++;
  }

  return 0;
}


bool ParseInputs(ArgumentList& args, int argc, char **argv);

#if NATIVE_OPTS
// cumbersome, it requires to use "=" for args, i.e. -i=../images/Lenna.pgm
bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  args.wait_t=0;

  cv::CommandLineParser parser(argc, argv,
      "{input   i|../images/Lenna.png|input image, Use %0xd format for multiple images.}"
      "{wait    t|0                  |wait before next frame (ms)}"
      "{help    h|<none>             |produce help message}"
      );

  if(parser.has("help"))
  {
    parser.printMessage();
    return false;
  }

  args.image_name = parser.get<std::string>("input");
  args.wait_t     = parser.get<int>("wait");

  return true;
}

#else

#include <unistd.h>
bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.wait_t = 0;

  while ((c = getopt (argc, argv, "hi:t:")) != -1)
    switch (c)
    {
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
          "   -t arg                   wait before next frame (ms)"<<std::endl<<std::endl;
        return false;
    }
  return true;
}

#endif

