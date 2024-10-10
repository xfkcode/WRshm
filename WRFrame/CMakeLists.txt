cmake_minimum_required(VERSION 3.10)

project(WRSHMFrameLib)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/inc)

find_package(OpenCV REQUIRED)

set(LIB_SOURCES
    ${PROJECT_SOURCE_DIR}/src/SharedMemoryManager.cc
    ${PROJECT_SOURCE_DIR}/src/ReadSHM.cc
    ${PROJECT_SOURCE_DIR}/src/WriteSHM.cc
)

add_library(wrshm STATIC ${LIB_SOURCES})

target_link_libraries(wrshm ${OpenCV_LIBS})

add_executable(main ${PROJECT_SOURCE_DIR}/src/main.cc)

target_link_libraries(main wrshm)

add_executable(W_server ${PROJECT_SOURCE_DIR}/src/W_server.cc )

target_link_libraries(W_server wrshm ${OpenCV_LIBS})

add_executable(R_client ${PROJECT_SOURCE_DIR}/src/R_client.cc )

target_link_libraries(R_client wrshm ${OpenCV_LIBS})