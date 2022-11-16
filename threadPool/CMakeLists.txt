cmake_minimum_required(VERSION 3.0.0)

project(project_threadPool VERSION 0.1.0)

file(GLOB SOURCES "./src/*.cpp")
file(GLOB HEADERS "./include/*.h")

include_directories(include ./include)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} pthread)
