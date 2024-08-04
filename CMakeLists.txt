project(serial_protocol)

# file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/*.cpp)
FILE(GLOB SOURCE_FILES ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/src/*.cpp )
include_directories(./inc)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})



