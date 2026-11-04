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

# mismatch_eigen e' scritto apposta per NON compilare (vedi commenti nel file):
# l'incompatibilita' di dimensioni e' rilevata da Eigen a compile-time, non a
# runtime come per OpenCV. EXCLUDE_FROM_ALL lo tiene fuori da "make"/"make all",
# cosi' la build di questa cartella non si rompe mai; per vedere l'errore del
# compilatore basta lanciare "make mismatch_eigen" esplicitamente.
#
# Richiede C++14 (non C++11 come il resto dei laboratori): la versione di Eigen
# installata su questo sistema lo richiede.
add_executable(mismatch_eigen EXCLUDE_FROM_ALL mismatch_eigen.cpp)
set_target_properties(mismatch_eigen PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)
