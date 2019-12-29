cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 11)
set(PROJECT_NAME "FaceVideo")

project(${PROJECT_NAME})
add_executable(${PROJECT_NAME} main.cpp)

configure_file("${CMAKE_SOURCE_DIR}/videos/1.mp4" "${CMAKE_BINARY_DIR}/videos/1.mp4" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/videos/2.mp4" "${CMAKE_BINARY_DIR}/videos/2.mp4" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/videos/3.mp4" "${CMAKE_BINARY_DIR}/videos/3.mp4" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/videos/4.mp4" "${CMAKE_BINARY_DIR}/videos/4.mp4" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/videos/5.mp4" "${CMAKE_BINARY_DIR}/videos/5.mp4" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/videos/6.mp4" "${CMAKE_BINARY_DIR}/videos/6.mp4" COPYONLY)

set(CMAKE_EXECUTABLE_SUFFIX ".html")
target_link_libraries(${PROJECT_NAME} "-s WASM=1")
target_link_libraries(${PROJECT_NAME} "-s USE_SDL=2")
target_link_libraries(${PROJECT_NAME} "-s USE_SDL_IMAGE=2")
target_link_libraries(${PROJECT_NAME} "-s SDL2_IMAGE_FORMATS='[\"png\",\"jpg\",\"bmp\"]'")
target_link_libraries(${PROJECT_NAME} "--preload-file videos")
target_link_libraries(${PROJECT_NAME} "-o ${PROJECT_NAME}.html")
target_link_libraries(${PROJECT_NAME} "-s DEMANGLE_SUPPORT=1")
