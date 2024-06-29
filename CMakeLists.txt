cmake_minimum_required(VERSION 3.9)
project(CryptograFace VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Set the C++ compiler flags for Release build
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Find OpenCV
find_package(OpenCV REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Concurrent)


# Set the include and library paths for Crypto++ and dlib
set(CRYPTO++_INCLUDE_DIR "/opt/homebrew/Cellar/cryptopp/8.9.0/include/cryptopp")
set(CRYPTO++_LIBRARY "/opt/homebrew/Cellar/cryptopp/8.9.0/lib/libcryptopp.a")

set(DLIB_INCLUDE_DIR "/opt/homebrew/Cellar/dlib/19.24.4/include")
set(DLIB_LIBRARY "/opt/homebrew/Cellar/dlib/19.24.4/lib/libdlib.dylib")

set(PYTHON_INCLUDE_DIR "/Library/Frameworks/Python.framework/Versions/3.9/include/python3.9")
set(PYTHON_LIBRARY "/Library/Frameworks/Python.framework/Versions/3.9/lib/libpython3.9.dylib")

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CRYPTO++_INCLUDE_DIR})
include_directories(${DLIB_INCLUDE_DIR})
include_directories(${PYTHON_INCLUDE_DIR})

add_executable(Test test.cpp)

target_link_libraries(Test PRIVATE ${OpenCV_LIBS} ${CRYPTO++_LIBRARY} ${DLIB_LIBRARY} "-framework Accelerate")


set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

set(PROJECT_SOURCES
        source/main.cpp

        source/loginwindow.cpp
        headers/loginwindow.h
        ui/loginwindow.ui

        source/notepadwindow.cpp
        headers/notepadwindow.h
        ui/notepadwindow.ui

        source/directorywindow.cpp
        headers/directorywindow.h

        source/facialrecognision.cpp
        headers/facialrecognision.h

        source/cryptography.cpp
        headers/cryptography.h
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

target_link_libraries(CryptograFace PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Concurrent ${OpenCV_LIBS} ${CRYPTO++_LIBRARY} ${DLIB_LIBRARY} ${PYTHON_LIBRARY} "-framework Accelerate")

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
