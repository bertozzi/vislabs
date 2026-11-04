// Esempio: moltiplicazione tra matrici con dimensioni non coerenti - Eigen
//
// A differenza di cv::Mat, Eigen::Matrix<T,Rows,Cols> ha dimensioni FISSE e note
// gia' al compilatore. Se le dimensioni non sono compatibili per il prodotto,
// l'errore NON e' un'eccezione a runtime come in OpenCV: il programma non arriva
// nemmeno a compilare.
//
// ATTENZIONE: questo file e' scritto apposta per NON compilare, ed e' per questo
// escluso dalla build del laboratorio (non e' elencato in examples/CMakeLists.txt).
// Per vedere l'errore del compilatore, compilatelo a mano, ad es.:
//
//   g++ -std=c++14 -I/usr/include/eigen3 -c mismatch_eigen.cpp -o /dev/null
//
// (serve -std=c++14, non -std=c++11 come nel resto dei laboratori: la versione
// di Eigen installata su questo sistema richiede almeno C++14)
//
// L'errore riportato dal compilatore e':
//
//   static assertion failed:
//   INVALID_MATRIX_PRODUCT__IF_YOU_WANTED_A_COEFF_WISE_PRODUCT_YOU_MUST_USE_THE_EXPLICIT_FUNCTION
//
// generato dall'interno di Eigen nell'istanziare il template dell'operatore di
// moltiplicazione: l'incompatibilita' delle dimensioni e' nota a COMPILE-TIME.

#include <eigen3/Eigen/Core>

int main()
{
    Eigen::Matrix<float, 3, 4> A; // 3x4, dimensioni fisse a compile-time
    Eigen::Matrix<float, 3, 4> B; // 3x4 -> per moltiplicare A*B servirebbe B con 4 righe, non 3

    Eigen::Matrix<float, 3, 4> C = A * B; // <-- errore di COMPILAZIONE, non di esecuzione

    return 0;
}
