cmake_minimum_required(VERSION 3.9)
project(CryptograFace)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV)

# Include directories for OpenCV


# Set the include and library paths for Crypto++ and dlib
set(CRYPTO++_INCLUDE_DIR "/opt/homebrew/Cellar/cryptopp/8.9.0/include/cryptopp")
set(CRYPTO++_LIBRARY "/opt/homebrew/Cellar/cryptopp/8.9.0/lib/libcryptopp.a")
#
set(DLIB_INCLUDE_DIR "/opt/homebrew/Cellar/dlib/19.24.4/include")
set(DLIB_LIBRARY "/opt/homebrew/Cellar/dlib/19.24.4/lib/libdlib.dylib")

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CRYPTO++_INCLUDE_DIR})
include_directories(${DLIB_INCLUDE_DIR})

add_executable(FacialRecognision facial.cpp)
add_executable(Cryptography cryptography.cpp)
add_executable(Encrypt encrypt.cpp)
add_executable(Decrypt decrypt.cpp)
add_executable(Test test.cpp)

# Link both OpenCV and Crypto++ libraries
target_link_libraries(FacialRecognision ${OpenCV_LIBS} ${CRYPTO++_LIBRARY} ${DLIB_LIBRARY})
target_link_libraries(Cryptography ${OpenCV_LIBS} ${CRYPTO++_LIBRARY} ${DLIB_LIBRARY})
target_link_libraries(Encrypt ${OpenCV_LIBS} ${CRYPTO++_LIBRARY} ${DLIB_LIBRARY})
target_link_libraries(Decrypt ${OpenCV_LIBS} ${CRYPTO++_LIBRARY} ${DLIB_LIBRARY})
target_link_libraries(Test ${OpenCV_LIBS} ${CRYPTO++_LIBRARY} ${DLIB_LIBRARY} "-framework Accelerate")