//OpneCV
//OpneCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include <bits/stdc++.h>

void aperture(const cv::Mat &, const cv::Mat &, const cv::Point2i &, cv::Mat &);
void dilation(const cv::Mat &, const cv::Mat &, const cv::Point2i &, cv::Mat &);
void erosion(const cv::Mat &, const cv::Mat &, const cv::Point2i &, cv::Mat &);
void binarize(const cv::Mat &, cv::Mat &, unsigned int);
void label(const cv::Mat &, cv::Mat &);
double otsu(cv::Mat);

struct ArgumentList {
  std::string image_name;	  //!< image file name
  int wait_t;                     //!< waiting time
  int threshold;                  //!< threshold
  float alpha;                    //!< exponential running average weight
};

bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.wait_t=0;
  args.alpha=0.5;
  args.threshold=50;


  while ((c = getopt (argc, argv, "hi:t:a:k:")) != -1)
    switch (c)
    {
      case 'a':
	args.alpha = atof(optarg);
	break;
      case 'i':
	args.image_name = optarg;
	break;
      case 'k':
	args.threshold = atoi(optarg);
	break;
      case 't':
	args.wait_t = atoi(optarg);
	break;
      case 'h':
      default:
	std::cout<<"usage: " << argv[0] << " -i <image_name>"<<std::endl;
	std::cout<<"exit:  type q"<<std::endl<<std::endl;
	std::cout<<"Allowed options:"<<std::endl<<
	  "   -h                         produce help message"<<std::endl<<
	  "   -i arg                   image name. Use %0xd format for multiple images."<<std::endl<<
	  "   -t arg                   wait before next frame (ms) [default = 0]"<<
	  "   -k arg                   threshold [default = 50]"<<std::endl<<
	  "   -a arg                   alpha param [default = 0.5]"<<std::endl<<std::endl<<std::endl;
	return false;
    }
  return true;
}

int main(int argc, char **argv)
{
  int frame_number = 0;
  char frame_name[256];
  bool exit_loop = false;

  std::cout<<"Exprunning program."<<std::endl;

  //////////////////////
  //parse argument list:
  //////////////////////
  ArgumentList args;
  if(!ParseInputs(args, argc, argv)) {
    return 1;
  }

  ////data
  // exponential running
  cv::Mat m_pframe_bg;

  while(!exit_loop)
  {
    //generating file name
    //
    //multi frame case
    if(args.image_name.find('%') != std::string::npos)
      sprintf(frame_name,(const char*)(args.image_name.c_str()),frame_number);
    else //single frame case
    {
      std::cerr << "Error: you must use as input a sequence, single images are not allowed!" << std::endl;
      exit(EXIT_FAILURE);
    }

    //opening file
    std::cout<<"Opening "<<frame_name<<std::endl;

    cv::Mat image = cv::imread(frame_name, CV_8UC1);
    if(image.empty())
    {
      std::cout<<"Unable to open "<<frame_name<<std::endl;
      return 1;
    }


    //////////////////////
    //processing code here
    ////BACKGROUND SUBTRACTION

    //////////////
    // PREV FRAME
    if(m_pframe_bg.empty()) {
      image.copyTo(m_pframe_bg);
    } else {
      cv::Mat fg, bg;

      image.convertTo(fg, CV_16SC1);
      m_pframe_bg.convertTo(bg, CV_16SC1);

      //cv::threshold(cv::abs(fg-bg), fg, args.threshold, 255, cv::THRESH_TOZERO);
      //fg.convertTo(fg, CV_8UC1);

      cv::Mat sub = cv::abs(fg-bg);
      sub.convertTo(sub, CV_8UC1);

      cv::Mat bin;
      binarize(sub, bin, args.threshold);

      cv::Mat out;
      cv::Mat se(3,3,CV_8UC1,cv::Scalar(255));
      aperture(bin, se, cv::Point2i(1,1), out);

      cv::Mat labels;
      label(out, labels);


      cv::namedWindow("Exp Running-BG",cv::WINDOW_NORMAL);
      cv::namedWindow("Bin fg",cv::WINDOW_NORMAL);
      cv::namedWindow("Dil",cv::WINDOW_NORMAL);
      cv::imshow("Exp Running-BG", m_pframe_bg);
      cv::imshow("Bin fg", bin);
      cv::imshow("Dil", out);

      cv::Mat adjMap;
      labels.convertTo(adjMap, CV_8UC1);
      cv::Mat falseColorsMap;
      cv::applyColorMap(adjMap, falseColorsMap, cv::COLORMAP_HSV);

      cv::namedWindow("Debug", cv::WINDOW_NORMAL);
      cv::imshow("Debug", falseColorsMap);


      m_pframe_bg=args.alpha*m_pframe_bg+(1-args.alpha)*image;
    }
    //display image
    cv::namedWindow("image", cv::WINDOW_NORMAL);
    cv::imshow("image", image);

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
void binarize(const cv::Mat &in, cv::Mat &out, unsigned int th)
{
  if(in.type() != CV_8UC1)
  {
    std::cerr << "Error: binarize() image must be in CV_8UC1 format" << std::endl;
    exit(EXIT_FAILURE);
  }

  in.copyTo(out);
  unsigned int N = (out.rows*out.cols);
  for(size_t i=0; i < N; ++i)
    out.data[i] = (out.data[i]<th)?0:255;
}

void histogramshow(double *histo, size_t size=256)
{
  cv::Mat g(size, size, CV_8UC1, cv::Scalar(0));

  int sum = 0;
  for(size_t i=0; i<size; ++i)
    sum += histo[i];

  for(size_t c=0; c<size; ++c)
  {
    std::cout << 255*histo[c]/sum << " ";
    for(size_t r=0; r<(255*histo[c]/sum); ++r)
    {
      g.data[c + size*(size - 1 - r)] = 255;
    }
  }
    cv::namedWindow("histogram", cv::WINDOW_NORMAL);
    cv::imshow("histogram", g);

  g.release();
}

// I applied what is described in "A C++ Implementation of Otsu's Image Segmentation Method Juan Pablo Balarini, Sergio Nesmachnow"
double otsu(cv::Mat src)
{
  cv::Mat g;

  // cv::abs() returns a S16
  src.convertTo(g, CV_8UC1);

  unsigned int N = (g.rows*g.cols);
  const int max_lum = 256;

  double histo[max_lum] = {0};

  for(size_t i=0; i < N; ++i) // compute histogram
    ++histo[g.data[i]];

  histogramshow(histo);

  double sum=0, sumB=0, var, var_max=0, threshold=0;
  for(size_t i=0; i < max_lum; ++i)
    sum += i*histo[i]; // auxiliary value for mu2

  double w1=0, w2, mu1, mu2;
  for(size_t t=0; t < max_lum; ++t)
  {
    w1 += histo[t];
    if(!w1)
      continue;
    w2 = N - w1;  

    sumB += t*histo[t];
    mu1 = sumB/w1;
    mu2= (sum - sumB)/w2;

    var = w1*w2+(mu1-mu2)*(mu1-mu2);
    if(var > var_max)
    {
      var_max = var;
      threshold = t;
    }
  }

  std::cout << "Otsu's algorithm implementation thresholding result: " << threshold << std::endl;

  return threshold;
}

inline bool _morph_set(unsigned char a)
{
  return a;
}

inline void _morph_set255(unsigned char &x)
{
  x = 255;
}


inline bool _morph_notset(unsigned char a)
{
  return !a;
}

inline void _morph_set0(unsigned char &x)
{
  x = 0;
}


void morph(const cv::Mat &in, const cv::Mat &se, const cv::Point2i &origin, cv::Mat &out, bool (* check)(unsigned char), void (*set)(unsigned char &))
{
  // check image type
  if(in.type() != CV_8UC1)
  {
    std::cerr << "Error: " << __func__ << "() wrong image type as input" << std::endl;
    exit(EXIT_FAILURE);
  }

  // initially output can be considered as copy of input
  in.copyTo(out);

  // wrt convolution, dealing with padding is difficult, since we have to consider both shape and origin of SE
  // for border pixels we consider partial overlay

  // iterate on all output/input pixels
  for(int r = 0; r < in.rows; ++r)
    for(int c = 0; c < in.cols; ++c)
    {
      // iterate on all se "pixels"
      bool ok = false;
      for(int kr = 0; kr < se.rows && !ok; ++kr)
	for(int kc = 0; kc < se.cols; ++kc)
	{
	  unsigned char se_value = se.data[kc + se.cols*kr];
	  if(!se_value)
	    continue;  // optimization trick
	  // compute image coords where se point is superimposed
	  int origy = r + (kr - origin.y);
	  int origx = c + (kc - origin.x);

	  // check image boundaries
	  if(origy >= 0 and origy < in.rows and origx >= 0 and origx < in.cols)
	  {
	    if(check(in.data[origx + origy*in.cols])) // at least one SE element is superimposed on a '1' pixel image
	    {
	      set(out.data[c + r*out.cols]);
	      ok = true;
	      break;
	    }
	  }
	}
    }
}

void dilation(const cv::Mat &in, const cv::Mat &se, const cv::Point2i &origin, cv::Mat &out)
{
  morph(in, se, origin, out, _morph_set, _morph_set255);
}


void erosion(const cv::Mat &in, const cv::Mat &se, const cv::Point2i &origin, cv::Mat &out)
{
  morph(in, se, origin, out, _morph_notset, _morph_set0);
}

void aperture(const cv::Mat &in, const cv::Mat &se, const cv::Point2i &origin, cv::Mat &out)
{
  cv::Mat tmp;
  erosion(in, se, origin, tmp);
  dilation(tmp, se, origin, out);
}

void label(const cv::Mat &in, cv::Mat &out)
{
  if(in.type() != CV_8UC1)
  {
    std::cerr << "Error: " << __func__ << "() wrong image type as input" << std::endl;
    exit(EXIT_FAILURE);
  }

  // initially output can be considered as copy of input
  cv::Mat labels = cv::Mat::zeros(in.rows, in.cols, CV_16UC1);
  uint16_t *labels_data = (uint16_t *)labels.data; // more confortable than direct access + cast
  uint16_t label = 5; // initial label

  // FIRST PASS

  // 1st pixel
  labels_data[0] = label;
  
  // 1st row
  for(int c = 1; c < in.cols; ++c)
    if(in.data[c] == in.data[c-1])
    {
      labels_data[c] = labels_data[c-1];
    }
    else
    {
      labels_data[c] = ++label;
    }

  // other rows
  std::map<uint16_t, uint16_t> same_labels;
  std::map<uint16_t, uint16_t>::iterator it;
  for(int r = 1; r < in.rows; ++r)
  {
    if(in.data[r*in.cols] == in.data[(r-1)*in.cols]) // first pixel of each row
      labels_data[r*in.cols] = labels_data[(r-1)*in.cols];
    else
    {
      labels_data[r*in.cols] = ++label;
    }

    for(int c = 1; c < in.cols; ++c)
    {
      // not necessary, only to make code easier to understand
      uint8_t p   = in.data[c      + r*in.cols];
      uint8_t pup = in.data[c      + (r - 1)*in.cols];
      uint8_t psx = in.data[c - 1  + r*in.cols];

      uint16_t &plabel  = labels_data[(c + r*in.cols)];
      uint16_t plabelup = labels_data[(c + (r - 1)*in.cols)];
      uint16_t plabelsx = labels_data[(c - 1 + r*in.cols)];

      // same value up and left with same labels
      if(p == pup and p == psx and plabelup == plabelsx)
	plabel = plabelup; 
      // same value up and left with different labels
      else if(p == pup and p == psx and plabelup != plabelsx)
      {
	plabel = std::min(plabelup,plabelsx);
	same_labels[std::max(plabelup, plabelsx)] = std::min(plabelup, plabelsx); // track the correspondance
      }
      else if(p == pup and p != psx) // same value up only
	plabel = plabelup;
      else if(p != pup and  p == psx) // same value left only
	plabel = plabelsx;
      else
      {
	plabel = ++label;
      }

    }
  }

  // SECOND PASS
  // I need to associate same labels
  // given the way I created it, maybe we need more than a single lookup...
  for(int r = 0; r < in.rows; ++r)
  {
    for(int c = 0; c < in.cols; ++c)
    {
	uint16_t &plabel  = labels_data[(c + r*in.cols)];
	while( (it = same_labels.find(plabel)) != same_labels.end())
	  plabel = it->second;
    }
  }

  out = labels;



}
