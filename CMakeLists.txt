cmake_minimum_required(VERSION 3.9)
project(CryptograFace VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)

# Set the C++ compiler flags for Release build
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Find OpenCV
find_package(OpenCV REQUIRED)

# Include directories for OpenCV


# Set the include and library paths for Crypto++ and dlib
set(CRYPTO++_INCLUDE_DIR "/opt/homebrew/Cellar/cryptopp/8.9.0/include/cryptopp")
set(CRYPTO++_LIBRARY "/opt/homebrew/Cellar/cryptopp/8.9.0/lib/libcryptopp.a")

set(DLIB_INCLUDE_DIR "/opt/homebrew/Cellar/dlib/19.24.4/include")
set(DLIB_LIBRARY "/opt/homebrew/Cellar/dlib/19.24.4/lib/libdlib.dylib")

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CRYPTO++_INCLUDE_DIR})
include_directories(${DLIB_INCLUDE_DIR})

add_executable(FacialRecognision facial.cpp headers/facialrecognision.h source/facialrecognision.cpp source/globals.cpp)
add_executable(Cryptography cryptography.cpp)
add_executable(Encrypt encrypt.cpp)
add_executable(Decrypt decrypt.cpp)
add_executable(Test test.cpp)

# Link both OpenCV and Crypto++ libraries
target_link_libraries(FacialRecognision ${OpenCV_LIBS} ${CRYPTO++_LIBRARY})
target_link_libraries(Cryptography ${OpenCV_LIBS} ${CRYPTO++_LIBRARY})
target_link_libraries(Encrypt ${OpenCV_LIBS} ${CRYPTO++_LIBRARY})
target_link_libraries(Decrypt ${OpenCV_LIBS} ${CRYPTO++_LIBRARY})
target_link_libraries(Test ${OpenCV_LIBS} ${CRYPTO++_LIBRARY} ${DLIB_LIBRARY} "-framework Accelerate")





cmake_minimum_required(VERSION 3.5)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

set(PROJECT_SOURCES
        source/main.cpp
        source/loginwindow.cpp
        headers/loginwindow.h
        ui/loginwindow.ui

        source/facialrecognision.cpp
        source/globals.cpp
        headers/facialrecognision.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CryptograFace
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
    )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET CryptograFace-GUI APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(CryptograFace SHARED
                ${PROJECT_SOURCES}
        )
        # Define properties for Android with Qt 5 after find_package() calls as:
        #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(CryptograFace
                ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(CryptograFace PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${OpenCV_LIBS} ${CRYPTO++_LIBRARY})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CryptograFace)
endif()
set_target_properties(CryptograFace PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS CryptograFace
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CryptograFace)
endif()
