//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

void labelConnectedComponents(const cv::Mat& bin, cv::Mat& labels)
{
}

//////////////////////////////////////////////////////////////////
// Helper di visualizzazione: assegna un colore (pseudo-)casuale ma
// deterministico a ciascuna label (0 = background = nero) e produce
// un'immagine BGR mostrabile con cv::imshow().
// Non fa parte dell'esercizio, è già completa.
//////////////////////////////////////////////////////////////////
cv::Mat colorizeLabels(const cv::Mat& labels)
{
  double max_label_d;
  cv::minMaxLoc(labels, nullptr, &max_label_d);
  int max_label = static_cast<int>(max_label_d);

  std::vector<cv::Vec3b> lut(max_label + 1, cv::Vec3b(0, 0, 0));
  cv::RNG rng(12345); // seed fisso: colori stabili tra una run e l'altra
  for(int l = 1; l <= max_label; ++l)
    lut[l] = cv::Vec3b(rng.uniform(50, 256), rng.uniform(50, 256), rng.uniform(50, 256));

  cv::Mat out(labels.size(), CV_8UC3, cv::Scalar(0, 0, 0));
  for(int r = 0; r < labels.rows; ++r)
  {
    const int* lrow = labels.ptr<int>(r);
    cv::Vec3b* orow = out.ptr<cv::Vec3b>(r);
    for(int c = 0; c < labels.cols; ++c)
      orow[c] = lut[lrow[c]];
  }
  return out;
}

struct ArgumentList {
  std::string image_name;  //!< image file name
  int wait_t;              //!< waiting time
  int threshold;           //!< binarization threshold
  bool otsu;               //!< use Otsu's method instead of a fixed threshold
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

    cv::Mat grey = cv::imread(frame_name, cv::IMREAD_GRAYSCALE);
    if(grey.empty())
    {
      std::cout<<"Unable to open "<<frame_name<<std::endl;
      return 1;
    }

    cv::namedWindow("grey", cv::WINDOW_NORMAL);
    cv::imshow("grey", grey);

    // PROCESSING

    // BINARIZZAZIONE
    // (se l'input e' gia' binario, es. le maschere prodotte da L03-background-sub,
    // questa operazione e' innocua: riporta semplicemente tutto a 0/255)
    cv::Mat binary;
    if(args.otsu)
      cv::threshold(grey, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    else
      cv::threshold(grey, binary, args.threshold, 255, cv::THRESH_BINARY);

    cv::namedWindow("binary", cv::WINDOW_NORMAL);
    cv::imshow("binary", binary);

    // CONNECTED COMPONENTS LABELING (il vostro algoritmo, vedi sopra)
    cv::Mat labels = cv::Mat::zeros(binary.size(), CV_32SC1);
    int64 t_start = cv::getTickCount();
    labelConnectedComponents(binary, labels);
    double t_ms = 1000.0 * (cv::getTickCount() - t_start) / cv::getTickFrequency();
    std::cout << "labelConnectedComponents() elapsed time: " << t_ms << " ms" << std::endl;

    cv::namedWindow("labels", cv::WINDOW_NORMAL);
    cv::imshow("labels", colorizeLabels(labels));

    // CONFRONTO CON cv::connectedComponents() - solo per verifica/timing,
    // non usatela dentro labelConnectedComponents()!
    cv::Mat cv_labels;
    int64 t_cv_start = cv::getTickCount();
    /*
       int cv::connectedComponents(
             cv::InputArray  image,          // immagine binaria/8-bit single-channel in input
             cv::OutputArray labels,         // output: Mat delle stesse dimensioni, con l'etichetta per pixel
             int             connectivity = 8,   // 4 oppure 8
             int             ltype        = CV_32S,  // tipo delle label: CV_32S o CV_16U
             int             ccltype      = cv::CCL_DEFAULT  // algoritmo interno (DEFAULT, WU, GRANA, BOLELLI, SAUF, BBDT, SPAGHETTI)
       );
     */
    int n_cv = cv::connectedComponents(binary, cv_labels, 4, CV_32S);
    double t_cv_ms = 1000.0 * (cv::getTickCount() - t_cv_start) / cv::getTickFrequency();
    std::cout << "cv::connectedComponents() elapsed time: " << t_cv_ms << " ms, "
              << "found " << (n_cv - 1) << " components (background escluso)" << std::endl;

    cv::namedWindow("labels (cv::connectedComponents)", cv::WINDOW_NORMAL);
    cv::imshow("labels (cv::connectedComponents)", colorizeLabels(cv_labels));

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
  args.wait_t   = 0;
  args.threshold = 128;
  args.otsu      = false;

  while ((c = getopt (argc, argv, "hi:t:k:o")) != -1)
    switch (c)
    {
      case 'k':
	args.threshold = atoi(optarg);
	break;
      case 'o':
	args.otsu = true;
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
	  "   -k arg                   binarization threshold (default 128)"<<std::endl<<
	  "   -o                       use Otsu's method instead of -k"<<std::endl<<std::endl;
	return false;
    }
  return true;
}
