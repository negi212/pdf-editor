cmake_minimum_required(VERSION 3.16)
project(PDFEditor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Use pkg-config for libraries if available, else fallbacks
find_package(PkgConfig REQUIRED)
pkg_check_modules(PODOFO REQUIRED libpodofo)
pkg_check_modules(QPDF REQUIRED libqpdf)

include_directories(${PODOFO_INCLUDE_DIRS} ${QPDF_INCLUDE_DIRS})
link_directories(${PODOFO_LIBRARY_DIRS} ${QPDF_LIBRARY_DIRS})

add_executable(PDFEditor
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/PdfWorkers.cpp
    src/PdfWorkers.h
)

target_link_libraries(PDFEditor PRIVATE
    Qt6::Widgets
    ${PODOFO_LIBRARIES}
    ${QPDF_LIBRARIES}
)
