cmake_minimum_required(VERSION 3.0.0)

project(project_server VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "Setting default CMAKE_INSTALL_PREFIX path to ${CMAKE_BINARY_DIR}/install")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/../../out/" CACHE STRING "The path to use for make install" FORCE)
endif()

include_directories(
    ../out/include
    ./include
)

file(GLOB SOURCES "./src/*.cpp" "./Demo.cpp")
file(GLOB HEADERS "./include/*.h")

link_directories(../out/lib/)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Copy file to INSTALL path
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME} Log Config ThreadPool Mysql)
