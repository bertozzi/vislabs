// Esempio: moltiplicazione tra matrici con dimensioni non coerenti - OpenCV (cv::Mat)
//
// cv::Mat ha dimensioni "dinamiche", decise solo a runtime: il compilatore non ha
// modo di sapere, in fase di compilazione, se due cv::Mat sono compatibili per la
// moltiplicazione (A.cols deve essere uguale a B.rows). L'incompatibilita' viene
// quindi rilevata solo in fase di ESECUZIONE, tramite un'asserzione interna di
// OpenCV (CV_Assert) che lancia una cv::Exception.
//
// Questo file compila ed esegue SENZA alcun errore di compilazione: l'errore
// compare solo quando il programma gira, nell'istante in cui si esegue A*B.

#include <iostream>
#include <opencv2/core/core.hpp>

int main()
{
    cv::Mat A = cv::Mat::ones(3, 4, CV_32F); // 3x4
    cv::Mat B = cv::Mat::ones(3, 4, CV_32F); // 3x4 -> per moltiplicare A*B servirebbe B con 4 righe, non 3

    std::cout << "A: " << A.rows << "x" << A.cols << std::endl;
    std::cout << "B: " << B.rows << "x" << B.cols << std::endl;
    std::cout << "Tentativo di calcolare A*B (richiede A.cols == B.rows, ma 4 != 3)..." << std::endl;

    try
    {
        cv::Mat C = A * B; // <-- qui scatta l'asserzione di OpenCV, a RUNTIME
        std::cout << "C: " << C.rows << "x" << C.cols << std::endl; // non si arriva mai qui
    }
    catch (const cv::Exception& e)
    {
        std::cerr << "\ncv::Exception catturata a RUNTIME:\n" << e.what() << std::endl;
    }

    return 0;
}
