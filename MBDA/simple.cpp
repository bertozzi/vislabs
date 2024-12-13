//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>


typedef struct _cluster
{
	int num_pixels=0;  

	float ur = 0;       //Centro di massa R
	float ug = 0;       //Centro di massa G
	float ub = 0;       //Centro di massa B

	//Costruttore
	_cluster(int _num_pixels=0,float _ur=0, float _ug=0, float _ub=0):num_pixels(_num_pixels), ur(_ur), ug(_ug), ub(_ub) {}

	//Operatore di comparazione per confrontare i cluster
	bool operator<(const _cluster & a)
	{
		return num_pixels < a.num_pixels;
	}

}cluster;


int main(int argc, char **argv)
{
	//Path in cui si trova il dataset
	std::string path("/vostro_path/data/");

	//Immagine target
	std::string target("pond_1.ppm");

	//Vettore con i nomi delle immagini da confrontare
	std::vector<std::string> compare = {"boat_1.ppm", "boat_2.ppm", "boat_3.ppm", "stHelens_1.ppm", "stHelens_2.ppm", "stHelens_3.ppm", "crater_1.ppm", "crater_2.ppm", "crater_3.ppm", "pond_1.ppm", "pond_2.ppm", "pond_3.ppm"};

	//Modificare a piacere...
	int cluster_count = 5;

	std::cout<<"Main Program."<<std::endl;

	// 1.1 - 1.2 Aprire immagine target, etc...

	//1.3 Applicare kMeans

	// Etc, etc...

	//Output
	cv::namedWindow("image", cv::WINDOW_NORMAL);
	cv::imshow("image", image);

	//wait for key
	cv::waitKey();
	
	return 0;
}
