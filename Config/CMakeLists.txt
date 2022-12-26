cmake_minimum_required(VERSION 3.0.0)

project(project_config VERSION 0.1.0)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "Setting default CMAKE_INSTALL_PREFIX path to ${CMAKE_BINARY_DIR}/install")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/../../out/" CACHE STRING "The path to use for make install" FORCE)
endif()

include_directories(include ../out/include)
include_directories(include ./include)

file(GLOB SOURCES "./src/*.cpp")
file(GLOB HEADERS "./include/*.h" "./include/*.hpp" "../out/include/*.h")

link_directories(../out/lib/)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Copy file to INSTALL path
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# # add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
# add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# # Copy header files to INSTALL path
# install(FILES ${HEADERS} DESTINATION include)

# # Copy library file to INSTALL path
# install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

target_link_libraries(${PROJECT_NAME} Log)

# target_link_libraries(${PROJECT_NAME} pthread)
