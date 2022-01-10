cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(seqgen VERSION 2.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

configure_file(SigGen.h.in ${PROJECT_SOURCE_DIR}/include/SigGen.h)

add_executable( seqgen
    source/seqgen.cpp
    source/Sequence.cpp
    source/Signal.cpp
    source/DTMF.cpp )

target_include_directories(seqgen PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/../tinyxml2 /opt/homebrew/include)

target_link_libraries(seqgen PUBLIC ${PROJECT_SOURCE_DIR}/../tinyxml2/libtinyxml2.a)

install(TARGETS seqgen DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" )

target_include_directories(seqgen
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
          )
