cmake_minimum_required(VERSION 3.0.0)

project(project_Log VERSION 0.1.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

aux_source_directory(./src PROJECT_SOURCES)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} pthread)
