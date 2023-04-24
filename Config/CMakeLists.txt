cmake_minimum_required(VERSION 3.0.0)

project(Config VERSION 0.1.0)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "Setting default CMAKE_INSTALL_PREFIX path to ${CMAKE_BINARY_DIR}/install")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/../../out/" CACHE STRING "The path to use for make install" FORCE)
endif()

include_directories(
    ./include
    ../out/include
)

file(GLOB SOURCES "./src/*.cpp")
file(GLOB HEADERS "./include/*.h" "./include/*.hpp")

link_directories(../out/lib/)

# 
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# 
# add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# # Copy header files to INSTALL path
install(FILES ${HEADERS} DESTINATION include)

# # Copy library file to INSTALL path
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

target_link_libraries(${PROJECT_NAME} pthread)
