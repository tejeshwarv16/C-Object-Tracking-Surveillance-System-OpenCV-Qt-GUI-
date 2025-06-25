# Minimum CMake version required (Using 3.16 for consistency)
cmake_minimum_required(VERSION 3.16)

# Project Name and Language
project(ObjectTrackingSystem VERSION 1.0 LANGUAGES CXX)

# *** Ensure Release Build for Performance ***
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)

# Include standard installation directory definitions
include(GNUInstallDirs)

# Set C++ standard (Using C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Qt5 Configuration ---
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# --- End Qt5 Configuration ---

# Explicitly tell the linker to use the console subsystem for MinGW
if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")
endif()

# --- OpenCV Configuration ---
list(APPEND CMAKE_PREFIX_PATH "C:/msys64/mingw64")
find_package(OpenCV REQUIRED COMPONENTS
    core highgui videoio imgproc objdetect tracking dnn
)
if(OpenCV_FOUND)
  message(STATUS "Found OpenCV version: ${OpenCV_VERSION}")
  include_directories(${OpenCV_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "OpenCV not found. Please check CMAKE_PREFIX_PATH or install OpenCV.")
endif()
# --- End OpenCV Configuration ---

# --- Project Sources ---
add_executable(ObjectTrackingApp src/main.cpp
                             src/MainWindow.cpp
                             src/MainWindow.h
                             src/VideoProcessor.cpp
                             src/VideoProcessor.h
                             )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Link against required libraries (OpenCV AND Qt5)
target_link_libraries(ObjectTrackingApp PRIVATE
    mingw32
    opencv_core opencv_highgui opencv_videoio opencv_imgproc
    opencv_objdetect opencv_tracking opencv_dnn
    Qt5::Core Qt5::Gui Qt5::Widgets
)
# --- End Project Sources ---

# --- Installation Rules ---
set(INSTALL_BIN_DIR ${CMAKE_INSTALL_BINDIR})
set(INSTALL_DATA_DIR data)
set(INSTALL_PLUGIN_DIR ${INSTALL_BIN_DIR}/platforms)
install(TARGETS ObjectTrackingApp RUNTIME DESTINATION ${INSTALL_BIN_DIR})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ DESTINATION ${INSTALL_DATA_DIR})
set(QT_PLUGIN_SOURCE_DIR ${CMAKE_PREFIX_PATH}/share/qt5/plugins/platforms)
if(EXISTS ${QT_PLUGIN_SOURCE_DIR})
    install(DIRECTORY ${QT_PLUGIN_SOURCE_DIR}/ DESTINATION ${INSTALL_PLUGIN_DIR})
else()
    message(WARNING "Qt platform plugin directory not found at ${QT_PLUGIN_SOURCE_DIR}.")
endif()
# --- End Installation Rules ---

message(STATUS "CMake configuration finished for ${PROJECT_NAME}.")