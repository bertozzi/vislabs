cmake_minimum_required(VERSION 2.8)
project(mismatch_examples)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# Solo l'esempio OpenCV viene compilato: l'errore e' a runtime, quindi il
# programma deve poter essere costruito normalmente.
#
# mismatch_eigen.cpp NON e' incluso qui: e' scritto apposta per fallire la
# COMPILAZIONE (vedi i commenti nel file), quindi aggiungerlo come target
# romperebbe sempre la build di questa cartella. Per vederne l'errore,
# compilatelo a mano (istruzioni nel file stesso).

add_executable(mismatch_opencv mismatch_opencv.cpp)
target_link_libraries( mismatch_opencv ${OpenCV_LIBS} )
