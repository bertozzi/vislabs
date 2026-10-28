//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

//////////////////////////////////////////////////////////////////
// Esercizio #2 - Accumulatore della Hough Transform (coordinate polari)
//////////////////////////////////////////////////////////////////
void houghAccumulate(const cv::Mat& edges, double thetaStepDeg, double rhoStep, double rhoMax, cv::Mat& accumulator)
{
}

//////////////////////////////////////////////////////////////////
// Esercizio #3 - Soppressione dei non-massimi sull'accumulatore
//////////////////////////////////////////////////////////////////
void houghNMS(const cv::Mat& accum, cv::Mat& nacc)
{
}

//////////////////////////////////////////////////////////////////
// Esercizio #4 - Estrazione delle rette dall'accumulatore
// Ritorna un vettore di (rho,theta): stessa convenzione di cv::HoughLines().
// Per la visualizzazione, usate la drawPolarLines() già fornita qui sotto.
//////////////////////////////////////////////////////////////////
std::vector<cv::Vec2f> houghExtractLines(const cv::Mat& nacc, int voteThreshold, double thetaStepDeg, double rhoStep, double rhoMax)
{
  return std::vector<cv::Vec2f>();
}

//////////////////////////////////////////////////////////////////
// Helper di visualizzazione: disegna sull'immagine le rette (rho,theta)
// (stessa convenzione di cv::HoughLines(): ogni retta e' un cv::Vec2f(rho,theta)).
// Non fa parte dell'esercizio, e' gia' completa.
//////////////////////////////////////////////////////////////////
void drawPolarLines(cv::Mat& img, const std::vector<cv::Vec2f>& lines, cv::Scalar color, int thickness = 2)
{
  for(size_t i = 0; i < lines.size(); ++i)
  {
    float rho = lines[i][0];
    float theta = lines[i][1];
    double a = cos(theta), b = sin(theta);
    double x0 = a*rho, y0 = b*rho;
    cv::Point pt1(cvRound(x0 - 2000*(-b)), cvRound(y0 - 2000*a));
    cv::Point pt2(cvRound(x0 + 2000*(-b)), cvRound(y0 + 2000*a));
    cv::line(img, pt1, pt2, color, thickness, cv::LINE_AA);
  }
}

struct ArgumentList {
  std::string image_name;  //!< image file name
  int wait_t;              //!< waiting time
  int canny_low, canny_high; //!< Canny hysteresis thresholds
  int votes;                 //!< accumulator vote threshold
  double theta_step;         //!< theta quantization step (degrees)
  double rho_step;           //!< rho quantization step (pixels)
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

    cv::Mat image = cv::imread(frame_name, cv::IMREAD_COLOR);
    if(image.empty())
    {
      std::cout<<"Unable to open "<<frame_name<<std::endl;
      return 1;
    }

    cv::Mat grey;
    cv::cvtColor(image, grey, cv::COLOR_BGR2GRAY);

    // EDGE DETECTION (data, non reimplementata qui - vedi L05-edges)
    // USARE cv::Canny()
    cv::Mat edges;
    // INSERIRE QUI CHIAMATA cv::Canny() e visualizzazione

    // HOUGH TRANSFORM (il vostro algoritmo, vedi sopra)
    cv::Mat accumulator;
    int64 t_start = cv::getTickCount();
    double rhoMax = 0; //TODO calcolarla
    houghAccumulate(edges, args.theta_step, args.rho_step, rhoMax, accumulator);
    double t_acc_ms = 1000.0 * (cv::getTickCount() - t_start) / cv::getTickFrequency();
    std::cout << "houghAccumulate() elapsed time: " << t_acc_ms << " ms" << std::endl;

    if(!accumulator.empty())
    {
      cv::Mat accDisplay;
      // usare cv::normalize()
      accDisplay.convertTo(accDisplay, CV_8U);
      cv::namedWindow("Hough accumulator", cv::WINDOW_NORMAL);
      cv::imshow("Hough accumulator", accDisplay);
    }

    cv::Mat nacc;
    int64 t_nms_start = cv::getTickCount();
    houghNMS(accumulator, nacc);
    double t_nms_ms = 1000.0 * (cv::getTickCount() - t_nms_start) / cv::getTickFrequency();
    std::cout << "houghNMS() elapsed time: " << t_nms_ms << " ms" << std::endl;

    if(!nacc.empty())
    {
      cv::Mat naccDisplay;
      // anche qui usare cv::normalize()
      naccDisplay.convertTo(naccDisplay, CV_8U);
      cv::namedWindow("Hough accumulator (NMS)", cv::WINDOW_NORMAL);
      cv::imshow("Hough accumulator (NMS)", naccDisplay);
    }

    int64 t_extract_start = cv::getTickCount();
    std::vector<cv::Vec2f> lines = houghExtractLines(nacc, args.votes, args.theta_step, args.rho_step, rhoMax);
    double t_extract_ms = 1000.0 * (cv::getTickCount() - t_extract_start) / cv::getTickFrequency();
    std::cout << "houghExtractLines() elapsed time: " << t_extract_ms << " ms, found " << lines.size() << " lines" << std::endl;

    cv::Mat result;
    image.copyTo(result);
    drawPolarLines(result, lines, cv::Scalar(0, 0, 255));
    cv::namedWindow("Detected lines", cv::WINDOW_NORMAL);
    cv::imshow("Detected lines", result);

    // CONFRONTO CON cv::HoughLines() - solo per verifica/timing,
    // non usatela dentro houghAccumulate()/houghExtractLines()!
    std::vector<cv::Vec2f> cv_lines;
    int64 t_cv_start = cv::getTickCount();
    cv::HoughLines(edges, cv_lines, args.rho_step, args.theta_step*CV_PI/180.0, args.votes);
    double t_cv_ms = 1000.0 * (cv::getTickCount() - t_cv_start) / cv::getTickFrequency();
    std::cout << "cv::HoughLines()  elapsed time: " << t_cv_ms << " ms, found " << cv_lines.size() << " lines" << std::endl;

    cv::Mat cv_result;
    image.copyTo(cv_result);
    drawPolarLines(cv_result, cv_lines, cv::Scalar(0, 255, 0));
    cv::namedWindow("Detected lines (cv::HoughLines)", cv::WINDOW_NORMAL);
    cv::imshow("Detected lines (cv::HoughLines)", cv_result);

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
  args.wait_t     = 0;
  args.canny_low  = 50;
  args.canny_high = 150;
  args.votes      = 100;
  args.theta_step = 1.0;
  args.rho_step   = 1.0;

  while ((c = getopt (argc, argv, "hi:t:L:H:v:a:r:")) != -1)
    switch (c)
    {
      case 'L':
	args.canny_low = atoi(optarg);
	break;
      case 'H':
	args.canny_high = atoi(optarg);
	break;
      case 'v':
	args.votes = atoi(optarg);
	break;
      case 'a':
	args.theta_step = atof(optarg);
	break;
      case 'r':
	args.rho_step = atof(optarg);
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
	  "   -L arg                   Canny low threshold (default 50)"<<std::endl<<
	  "   -H arg                   Canny high threshold (default 150)"<<std::endl<<
	  "   -v arg                   accumulator vote threshold (default 100)"<<std::endl<<
	  "   -a arg                   theta quantization step, degrees (default 1.0)"<<std::endl<<
	  "   -r arg                   rho quantization step, pixels (default 1.0)"<<std::endl<<std::endl;
	return false;
    }
  return true;
}
