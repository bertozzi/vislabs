//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

// Vedi slides/11_stereo_match.odp ("Disparity", d = f*B/Z) e
// slides/L08-vdisparity.odp per il dettaglio degli esercizi.
//
// Non implementiamo la ricerca di corrispondenze stile SAD a finestra 7x7
// (non ancora vista a lezione): calcoliamo la v-disparity direttamente dalle
// due immagini rettificate (data/left.png, data/right.png), confrontando le
// righe punto a punto a diversi offset.
//
// Poi usiamo la Hough transform - la STESSA gia' reimplementata in
// L06-hough, qui disponibile come primitiva OpenCV - per individuare: la
// retta principale obliqua (il piano strada) e i segmenti quasi verticali
// (gli ostacoli).

struct CameraParams {
  double f, u0, v0, baseline;
};

//////////////////////////////////////////////////////////////////
// Dato - lettura dei parametri della camera stereo rettificata
//////////////////////////////////////////////////////////////////
bool LoadParams(const std::string& filename, CameraParams& params)
{
  std::ifstream fh(filename.c_str());
  if(!fh.is_open())
    return false;

  std::string line;
  while(std::getline(fh, line))
  {
    if(line.empty() || line[0] == '#')
      continue;
    std::istringstream iss(line);
    if(iss >> params.f >> params.u0 >> params.v0 >> params.baseline)
      return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////
// Esercizio #1 - V-disparity direttamente dalle due immagini
//
// Date due immagini rettificate (h x w), consideriamo le loro righe una
// alla volta e le sovrapponiamo a differenti offset (limitiamoci da 0 a
// 128). Per ogni offset contiamo quanti pixel "corrispondono" (differenza
// in valore assoluto sotto una soglia) e normalizziamo il totale (il numero
// di pixel confrontabili cambia con l'offset!).
//
// Il risultato e' una immagine h x 128, CV_8UC1, con il valore normalizzato
// in [0,255] (0 = nessuna corrispondenza, 255 = tutti i pixel confrontabili
// corrispondono) - pronta per essere binarizzata con cv::threshold()
// (vedi slide, Otsu).
//////////////////////////////////////////////////////////////////
void VDisparity(const cv::Mat& left, const cv::Mat& right, cv::Mat& out, int maxDisp = 128)
{
  out = cv::Mat(left.rows, maxDisp, CV_8UC1, cv::Scalar(0));

  /* YOUR CODE HERE
   *
   */
}

//////////////////////////////////////////////////////////////////
// Esercizio #2 - Retta del piano strada
//
// Nella v-disparity, i punti del piano strada si dispongono lungo un'unica
// retta obliqua v = m*d + q (si puo' dimostrare partendo dalle equazioni di
// proiezione, vedi slide dedicata).
//
// Tra le rette trovate da cv::HoughLines() (vedi main), scartare quelle
// quasi verticali (sono i segmenti degli ostacoli, esercizio #3) e tenere
// quella con piu' voti: e' la retta del piano strada.
//
// Convertire da (rho,theta) [convenzione cv::HoughLines: d*cos(theta) +
// v*sin(theta) = rho, con theta~0 -> retta verticale] alla forma v = m*d + q
//////////////////////////////////////////////////////////////////
bool fitRoadLine(const std::vector<cv::Vec2f>& houghLines, cv::Vec2d& line /* (m,q) */)
{
  return false;
}

//////////////////////////////////////////////////////////////////
// Esercizio #2 (continua) - Inclinazione del piano strada e altezza camera
//
// Dalla retta v = m*d + q, dai parametri camera (f, v0, baseline) si puo'
// dimostrare che:
//
//   theta = atan2(v0 - q, f)                 (inclinazione, radianti)
//   height = m * baseline * cos(theta)       (altezza camera sul piano, metri)
//////////////////////////////////////////////////////////////////
void roadPlaneFromLine(const cv::Vec2d& line, const CameraParams& params, double& theta, double& height)
{
  theta = 0.0;
  height = 0.0;
}

//////////////////////////////////////////////////////////////////
// Esercizio #3 - Ostacoli
//
// Duale dell'esercizio #2: un ostacolo (oggetto verticale) produce nella
// v-disparity un segmento quasi verticale (disparita' pressoche' costante
// al variare di v), cioe' theta ~ 0 nella convenzione di cv::HoughLines().
//
// Tra le rette trovate da cv::HoughLines(), prendere quelle quasi
// verticali (il complementare dell'esercizio #2) e per ciascuna ricavare la
// disparita' rappresentativa (per theta~0: d*cos(theta) = rho, quindi
// d ~ rho / cos(theta))
//////////////////////////////////////////////////////////////////
std::vector<double> findObstacleDisparities(const std::vector<cv::Vec2f>& houghLines)
{
  return std::vector<double>();
}

struct ArgumentList {
  std::string left_filename;
  std::string right_filename;
  std::string params_filename;
};

bool ParseInputs(ArgumentList& args, int argc, char **argv);

int main(int argc, char **argv)
{
  ArgumentList args;
  if(!ParseInputs(args, argc, argv))
    exit(0);

  cv::Mat left = cv::imread(args.left_filename, cv::IMREAD_GRAYSCALE);
  cv::Mat right = cv::imread(args.right_filename, cv::IMREAD_GRAYSCALE);
  if(left.empty() || right.empty())
  {
    std::cerr << "Unable to open " << args.left_filename << " / " << args.right_filename << std::endl;
    return 1;
  }

  CameraParams params;
  if(!LoadParams(args.params_filename, params))
  {
    std::cerr << "Unable to open " << args.params_filename << std::endl;
    return 1;
  }

  // Esercizio #1
  cv::Mat vdisp;
  VDisparity(left, right, vdisp);

  // binarizzazione (vedi slide: cv::threshold con Otsu)
  cv::Mat mask;
  cv::threshold(vdisp, mask, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

  // estrazione delle rette principali (usata sia dall'esercizio #2 che #3)
  std::vector<cv::Vec2f> houghLines;
  cv::HoughLines(mask, houghLines, 1, CV_PI / 180.0, 60);

  // Esercizio #2
  cv::Vec2d roadLine;
  bool foundRoad = fitRoadLine(houghLines, roadLine);
  if(foundRoad)
  {
    double theta, height;
    roadPlaneFromLine(roadLine, params, theta, height);
    std::cout << "Retta piano strada: v = " << roadLine[0] << "*d + " << roadLine[1] << std::endl;
    std::cout << "Inclinazione camera/strada: " << (theta * 180.0 / CV_PI) << " gradi" << std::endl;
    std::cout << "Altezza camera sul piano strada: " << height << " m" << std::endl;
  }
  else
  {
    std::cout << "Retta del piano strada non trovata (esercizio da completare)" << std::endl;
  }

  // Esercizio #3
  std::vector<double> obstacles = findObstacleDisparities(houghLines);
  std::cout << "Ostacoli trovati: " << obstacles.size() << std::endl;
  for(size_t i = 0; i < obstacles.size(); ++i)
  {
    double Z = params.f * params.baseline / obstacles[i];
    std::cout << "  disparita' " << obstacles[i] << " -> distanza stimata " << Z << " m" << std::endl;
  }

  // Valori di riferimento noti (dataset sintetico)
  std::cout << "\nRiferimento (dataset sintetico): inclinazione 8 gradi, altezza camera 1.4 m,"
            << " ostacoli a ~12.0 m e ~20.0 m" << std::endl;

  cv::namedWindow("V-Disparity", cv::WINDOW_NORMAL);
  cv::imshow("V-Disparity", vdisp);
  cv::namedWindow("V-Disparity binaria", cv::WINDOW_NORMAL);
  cv::imshow("V-Disparity binaria", mask);
  std::cout << "Press any key to quit" << std::endl;
  cv::waitKey(0);

  return 0;
}

#include <unistd.h>
bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.left_filename = "../data/left.png";
  args.right_filename = "../data/right.png";
  args.params_filename = "../data/params.txt";

  while ((c = getopt (argc, argv, "hl:r:p:")) != -1)
    switch (c)
    {
      case 'l':
        args.left_filename = optarg;
        break;
      case 'r':
        args.right_filename = optarg;
        break;
      case 'p':
        args.params_filename = optarg;
        break;
      case 'h':
      default:
        std::cout << "usage: " << argv[0] << " [options]" << std::endl << std::endl;
        std::cout << "Allowed options:" << std::endl <<
          "   -h                       produce help message" << std::endl <<
          "   -l arg                   left image filename (default ../data/left.png)" << std::endl <<
          "   -r arg                   right image filename (default ../data/right.png)" << std::endl <<
          "   -p arg                   camera params filename (default ../data/params.txt)" << std::endl << std::endl;
        return false;
    }
  return true;
}
