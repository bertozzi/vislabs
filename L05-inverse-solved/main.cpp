// std
#include <iostream>
#include <fstream>

// opencv
#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui.hpp>

// eigen
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>

// utils
#include "utils.h"

cv::Vec3b bilinear(const cv::Mat& src, float r, float c);


int main(int argc, char **argv) {

  if (argc < 3) 
  {
    std::cerr << "Usage lab5_2 <image_filename> <camera_params_filename>" << std::endl; 
    return 0;
  }

  // load image from file
  cv::Mat input;
  input = cv::imread(argv[1]);

  cv::namedWindow("Input", cv::WINDOW_AUTOSIZE );
  cv::imshow("Input", input);
  cv::waitKey(10);

  // load camera params
  CameraParams params;
  LoadCameraParams(argv[2], params);

  Eigen::Matrix<float, 3, 4> K;
  K << params.ku,   0.0,       params.u0, 0.0,
    0.0,         params.kv, params.v0, 0.0,
    0.0,         0.0,       1.0,       0.0;

  Eigen::Matrix<float, 4, 4> RT;
  cv::Affine3f RT_inv = params.RT.inv();
  RT << RT_inv.matrix(0,0), RT_inv.matrix(0,1), RT_inv.matrix(0,2), RT_inv.matrix(0,3),
     RT_inv.matrix(1,0), RT_inv.matrix(1,1), RT_inv.matrix(1,2), RT_inv.matrix(1,3),
     RT_inv.matrix(2,0), RT_inv.matrix(2,1), RT_inv.matrix(2,2), RT_inv.matrix(2,3),
     0,                  0,                  0,                  1;

  /*
   * YOUR CODE HERE: Choose a planar constraint and realize an inverse perspective mapping
   * 1) compute the perspective matrix P
   * 2) choose a planar constraint (ex. y=0) to be used during the inverse perspective mapping
   * 3) compute the inverse perspective matrix corresponding to the chosen constraint
   * 4) realize the inverse perspective mapping
   */ 

  //
  //
  // Voglio trovare la trasformazione inversa che porta da pixel (u,v) al corrispondente X,Y,Z assumendo che Y=0
  //
  //

  // Iniziamo calcolando la matrice M
  Eigen::Matrix<float, 3, 4> M;
  M = K*RT;


  // Quando trasformo da mondo (X,0,Z,1) a (u,v,w) tramite M vediamo che la seconda colonna e' irrilevante (Y=0)
  Eigen::Matrix3f M_r;
  M_r <<  M(0,0), M(0,2), M(0,3),
          M(1,0), M(1,2), M(1,3),
          M(2,0), M(2,2), M(2,3);

  // tmp e' quindi la matrice che porta da (X,Z,1) a (u,v,w), ed e' quella che dobbiamo invertire per fare il percorso contrario
  // (u,v,1) -> (X,Z,W)
  Eigen::Matrix3f IPM;
  IPM = M_r.inverse();

  // creiamo ora un'immagine che rappresenta una mappa sul piano y=0
  //
  //
  //
  //        --> colonne
  //       |
  //       |      ---------------------------
  //       V      |                         |
  //      righe   |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |            ^ Z          |
  //              |            |            |
  //              |            |            |
  //              |            - - > X      |
  //              ---------------------------
  //
  // c'e' una corrispondenza diretta tra colonne/X e righe/Z, a meno di un fatto di scala k e di un offset


  double k = 50.0;

  // output image
#ifndef OLD
  cv::Mat sparse_ipm(cv::Size(400, 400), CV_8UC3, cv::Scalar(0, 0, 0));
#else
  cv::Mat ipm_image(cv::Size(400, 400), CV_8UC3, cv::Scalar(0, 0, 0));
#endif


  // in questo caso parto dall'immagine e calcolo, grazie alla matrice inversa della  M a cui avevo tolto una colonna
  // il punto del piano Y=0 che da' origine a quel pixel
  // ha il difetto che punti immagine contingui potranno finire in punti mondo con Y=0 non contigui, lasciando di conseguenza dei "buchi" nell'immagine generata

  for(int ii = 0; ii < input.rows; ii++)
  {
    for(int jj = 0; jj < input.cols; jj++)
    {
#ifndef OLD
      Eigen::Vector3f proj = IPM * Eigen::Vector3f(jj, ii, 1.0);

      // applico un fattore di scala k (altrimenti la parte di piano che prendo ha le dimensioni del sensore immagine...)
      // aggiusto anche le coordinate di modo che sia centrato nell'immagine e con il giusto verso dell'asse verticale
      int X = proj.x()/proj.z()*k + sparse_ipm.cols/2.0;
      int Z = sparse_ipm.rows - proj.y()/proj.z()*k;
      if( X >=0 && X < sparse_ipm.cols && Z >=0 && Z < sparse_ipm.rows)
      {
	sparse_ipm.at<cv::Vec3b>(Z, X) = input.at<cv::Vec3b>(ii, jj); // l'asse verticale sarebbe orientato verso l'alto ma nelle immagini e' orientato verso il basso
      }

#else

      // metodo originario in questa soluzione, ho preferito "ammodernarlo"
      double w = (IPM(2,0)*jj+IPM(2,1)*ii+IPM(2,2));	        // omogenea mondo
      double x = (IPM(0,0)*jj+IPM(0,1)*ii+IPM(0,2))/w;		// x mondo
      double z = (IPM(1,0)*jj+IPM(1,1)*ii+IPM(1,2))/w;		// z mondo
      //fattore di scala a piacere
      x*=k;
      z*=k;

      // offset delle origini dei due sistemi di riferimento
      double z_ipm = ipm_image.rows - z;
      double x_ipm = ipm_image.cols/2 + x;

      //Creazione dell'immagine
      if(z_ipm >=0 && z_ipm < ipm_image.rows && x_ipm >=0 && x_ipm < ipm_image.cols)
      {
	for(int kk = 0; kk < ipm_image.channels(); kk++)
	{
	  ipm_image.data[((int)x_ipm+(int)z_ipm*ipm_image.cols)*ipm_image.elemSize()+kk]=input.data[(jj+ii*input.cols)*input.elemSize()+kk];
	}
      }	
#endif
    }
  }

  // se parto da immagine destinazione ottengo IPM densa, ovvero senza "buchi"
  // per ogni punto mondo del piano Y=0 posso calcolare dove va a finire nell'immagine a associarvi il relativo valore di colore
  cv::Mat dense_ipm(cv::Size(400, 400), CV_8UC3, cv::Scalar(0, 0, 0));
  cv::Mat bilinear_dense_ipm(dense_ipm.size(), dense_ipm.type(), cv::Scalar(0, 0, 0));

  for(int ii = 0; ii < dense_ipm.rows; ii++)
  {
    for(int jj = 0; jj < dense_ipm.cols; jj++)
    {
      // remember how the world reference system is oriented
      Eigen::Vector3f ipm_point((jj-dense_ipm.cols/2)/k, ii/k, 1.0);
      Eigen::Vector3f uv_point = M_r * ipm_point; 
      float u = uv_point.x()/float(uv_point.z());
      float v = uv_point.y()/float(uv_point.z());
      // u e v sono colonna e riga immagine "sorgente"
      // due possibilita'
      // 1. le uso sbiotte ma in questo modo ottengo una IPM densa ma a quadrettoni
      // 2. applico una interpolazione bilineare, che permette di ottenere una IPM "regolare"
      if(v >= 0 && v < input.rows - 1  && u >= 0 && u < input.cols - 1) // considero limiti immagine sorgente, il -1 serve solo per la bilineare
      {
	dense_ipm.at<cv::Vec3b>(dense_ipm.rows - ii, jj) = input.at<cv::Vec3b>(v, u); // l'asse verticale sarebbe orientato verso l'alto ma nelle immagini e' orientato verso il basso
	bilinear_dense_ipm.at<cv::Vec3b>(dense_ipm.rows - ii, jj) = bilinear(input, v, u); 
      }
    }
  }

#ifndef OLD
  cv::namedWindow("Sparse IPM", cv::WINDOW_NORMAL );
  imshow("Sparse IPM", sparse_ipm);
#else
  cv::namedWindow("IPM", cv::WINDOW_NORMAL );
  imshow("IPM", ipm_image);
#endif

  cv::namedWindow("Dense IPM", cv::WINDOW_NORMAL );
  imshow("Dense IPM", dense_ipm);

  cv::namedWindow("Bilinear Dense IPM", cv::WINDOW_NORMAL );
  imshow("Bilinear Dense IPM", bilinear_dense_ipm);

  std::cout << "Press any key to quit" << std::endl;

  cv::waitKey(0);

  return 0;
}

cv::Vec3b bilinear(const cv::Mat& src, float r, float c){

  float yDist = r - static_cast<long>(r);
  float xDist = c - static_cast<long>(c);

  // interpolazione bilineare (che sfrutta ampiamente le operazioni aritmetiche ridefinite per la cv::Vec3b)
  cv::Vec3b value =
    src.at<cv::Vec3b>(r,c)*(1-yDist)*(1-xDist) +
    src.at<cv::Vec3b>(r+1,c)*(yDist)*(1-xDist) +
    src.at<cv::Vec3b>(r,c+1)*(1-yDist)*(xDist) +
    src.at<cv::Vec3b>(r+1,c+1)*yDist*xDist;


  return value;
}

