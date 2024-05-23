cmake_minimum_required(VERSION 3.9)
project(CryptograFace)

set(CMAKE_CXX_STANDARD 11)

# Find OpenCV
find_package(OpenCV)

# Include directories for OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Set the include and library paths for Crypto++
set(CRYPTO++_INCLUDE_DIR "/opt/homebrew/Cellar/cryptopp/8.9.0/include/cryptopp")
set(CRYPTO++_LIBRARY "/opt/homebrew/Cellar/cryptopp/8.9.0/lib/libcryptopp.a")
include_directories(${CRYPTO++_INCLUDE_DIR})

link_directories(/opt/homebrew/Cellar/cryptopp/8.9.0/lib)

add_executable(FacialRecognision facial.cpp)
add_executable(Cryptography cryptography.cpp)
add_executable(Encrypt encrypt.cpp)
add_executable(Decrypt decrypt.cpp)
add_executable(Test test.cpp)

# Link both OpenCV and Crypto++ libraries
target_link_libraries(FacialRecognision ${OpenCV_LIBS} ${CRYPTO++_LIBRARY})
target_link_libraries(Cryptography ${OpenCV_LIBS} ${CRYPTO++_LIBRARY})
target_link_libraries(Encrypt ${OpenCV_LIBS} ${CRYPTO++_LIBRARY})
target_link_libraries(Decrypt ${OpenCV_LIBS} ${CRYPTO++_LIBRARY})
target_link_libraries(Test ${OpenCV_LIBS} ${CRYPTO++_LIBRARY})