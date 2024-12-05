//OpneCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>

// Non necessario per l'esame
//#define USE_SGM

#ifdef USE_SGM
//richiede opencv_contrib/modules/stereo
#include <opencv2/calib3d.hpp>
#include <opencv2/stereo.hpp>

void testOpenCvSGM(const cv::Mat & left_image, const cv::Mat & right_image);
#endif



///////////////////////////////////////////
// ES. 1
//
//
// Per ogni pixel dell'immagine sinistra, cercare il suo corrispondente nell'immagine destra utilizzando la metrica SAD (Sum of Absolute Differences)
//
// Il corrispondente cercato e' il pixel sull'immagine destra tale per cui la somma delle differenze in valore assoluto di una finestra 7x7,
// centrata intorno ai relativi pixel destra/sinistra, e' minima
//
// In altre parole, per ogni pixel dell'immagine sx, confrontare il suo vicinato 7x7 con il corrispondente vicinato 7x7 di un pixel sulla destra.
//
// HINTS:
// - le immagini sono rettificate, quindi il corrispondente a destra su quale riga si trova?
// - dato un pixel (riga_l,colonna_l) sull'immagine sinistra, il corrispondente sulla destra (riga_r, colonna_r) si puo' trovare unicamente da uno specifico lato:
//   colonna_l < colonna_r? Oppure colonna_l > colonna_r? Quale dei due?
// - consideriamo spostamenti lungo la righa di massimo 128 colonne (cioe' disparita' massima 128)
//
//
// HINTS: si puo' fare con 5 cicli innestati
//
//
// REFERENCE: Lezioni 11_StereoMatching e 13_Feature2
//
void mySAD_Disparity7x7(const cv::Mat & left_image, const cv::Mat & right_image, cv::Mat & out)
{
	//immagine di uscita
	out = cv::Mat(left_image.rows, left_image.cols, CV_32FC1, cv::Scalar(0));

	/* YOUR CODE HERE
	 *
	 */
}
///////////////////////////////////////////


///////////////////////////////////////////
// ES. 2
//
// Creiamo una nuova immagine di altezza paria all'altezza della disparita', larghezza pari a 128
//
// Per ogni riga di questa nuova immagine, salviamo nella c-esima colonna il conteggio delle disparita' floor(disp)==c per quella stessa riga
//
// NON conteggiamo le disparita' floor(disp)==0
//
// HINTS:
// - la riga r-esima di questa nuova immagine dipende solo riga r-esima della disparita'
//
void VDisparity(const cv::Mat & disp, cv::Mat & out)
{
	// immagine di uscita
	//
	// e' una uint16 per essere sicuri di poter accumulare abbastanza valori negli istogrammi
	out = cv::Mat(disp.rows, 128, CV_16UC1, cv::Scalar(0));


	/* YOUR CODE HERE
	 *
	 */
}
///////////////////////////////////////////

int main(int argc, char **argv)
{
	std::cout<<"Simple program."<<std::endl;

	//////////////////////
	//parse argument list:
	//////////////////////
    if (argc != 3)
    {
        std::cerr << "Usage ./simple <left_image_filename> <right_image_filename>" << std::endl;
        return 0;
    }

	//opening left file
	std::cout<<"Opening "<<argv[1]<<std::endl;
	cv::Mat left_image = cv::imread(argv[1], CV_8UC1);
	if(left_image.empty())
	{
		std::cout<<"Unable to open "<<argv[1]<<std::endl;
		return 1;
	}

	//opening right file
	std::cout<<"Opening "<<argv[2]<<std::endl;
	cv::Mat right_image = cv::imread(argv[2], CV_8UC1);
	if(right_image.empty())
	{
		std::cout<<"Unable to open "<<argv[2]<<std::endl;
		return 1;
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// ES.1
	//
	//immagine di disparita' di output
	cv::Mat imgDisparity32FC1;

	//CHIAMATA ALLA VOSTRA FUNZIONE
	mySAD_Disparity7x7(left_image, right_image, imgDisparity32FC1);
	////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// ES.2
	cv::Mat imgVDisparity16UC1;

	//CHIAMATA ALLA VOSTRA FUNZIONE
	VDisparity(imgDisparity32FC1, imgVDisparity16UC1);
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// WINDOWS
	//
	// DO NOT CHANGE
	//
	cv::Mat adjMap;
	double minVal; double maxVal;

	cv::minMaxLoc(imgDisparity32FC1, &minVal, &maxVal);
	std::cout<<"max min mySAD "<<maxVal<<" "<<minVal<<std::endl;
	cv::convertScaleAbs(imgDisparity32FC1, adjMap, 255 / (maxVal-minVal));
	cv::namedWindow("mySAD", cv::WINDOW_AUTOSIZE);
	cv::imshow("mySAD", adjMap);

	cv::Mat adjMapV;
	cv::minMaxLoc(imgVDisparity16UC1, &minVal, &maxVal);
	std::cout<<"max min VDisparity "<<maxVal<<" "<<minVal<<std::endl;
	cv::convertScaleAbs(imgVDisparity16UC1, adjMapV, 255 / (maxVal-minVal));
	cv::namedWindow("VDisparity", cv::WINDOW_AUTOSIZE);
	cv::imshow("VDisparity", adjMapV);


#ifdef USE_SGM
	/// Esempio SGM di OpenCv
	//
	//  Non necessario per l'esame
	testOpenCvSGM(left_image, right_image);
#endif

	//display images
	cv::namedWindow("left image", cv::WINDOW_NORMAL);
	cv::imshow("left image", left_image);

	cv::namedWindow("right image", cv::WINDOW_NORMAL);
	cv::imshow("right image", right_image);
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	//wait for key
	cv::waitKey();

	return 0;
}


///////////////////////////////////////////////////////////////////////////
////
//// EXTRA ESAME
////
//
#ifdef USE_SGM
void testOpenCvSGM(const cv::Mat & left_image, const cv::Mat & right_image)
{
	////////////////////////////////////////////////////////////////////////
	///
	/// Esempio di disparita' ottenuta tramite metodo Semi-Global-Matching (SGM)
	///
	/// E' lo stato dell'arte per quanto riguarda il calcolo della disparita' in tempo reale
	///
	/// Esistono altri metodi piu' accurati, ma piu' lenti
	///

	int kernel_size = 5, number_of_disparities = 128, P1 = 100, P2 = 1000;
	int binary_descriptor_type = 0;
	cv::Mat imgDisparity16U(left_image.rows, left_image.cols, CV_16U, cv::Scalar(0));

	// we set the corresponding parameters
	cv::Ptr<cv::stereo::StereoBinarySGBM> sgbm = cv::stereo::StereoBinarySGBM::create(1, number_of_disparities, kernel_size);

	// setting the penalties for sgbm
	sgbm->setP1(P1);
	sgbm->setP2(P2);
	sgbm->setMinDisparity(1);
	sgbm->setUniquenessRatio(5);
	sgbm->setSpeckleWindowSize(400);
	sgbm->setSpeckleRange(200);
	sgbm->setDisp12MaxDiff(1);
	sgbm->setBinaryKernelType(binary_descriptor_type);
	sgbm->setSpekleRemovalTechnique(cv::stereo::CV_SPECKLE_REMOVAL_AVG_ALGORITHM);
	sgbm->setSubPixelInterpolationMethod(cv::stereo::CV_SIMETRICV_INTERPOLATION);
	sgbm->compute(left_image, right_image, imgDisparity16U);

	cv::Mat adjMap, falseColorsMap;
	double minVal; double maxVal;

	imgDisparity16U/=16; //opencv restituisce la dispartia' in fixed point, con 4 bit per la parte frazionaria

	cv::minMaxLoc(imgDisparity16U, &minVal, &maxVal);
	std::cout<<"max min SGM "<<maxVal<<" "<<minVal<<std::endl;
	cv::convertScaleAbs(imgDisparity16U, adjMap, 255 / (maxVal-minVal));
	cv::namedWindow("SGM", cv::WINDOW_NORMAL);
	cv::imshow("SGM", adjMap);

	////////////////////////////////////////////////////////////////////////
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////
