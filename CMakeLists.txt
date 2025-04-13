cmake_minimum_required(VERSION 3.16)  # Or a version appropriate for your project
project(SurvivorsQuest VERSION 1.0 LANGUAGES CXX)

# Set C++ standard (adjust as needed)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/app/ui)

# Find the Qt package; list all modules you need, for example:
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
qt_standard_project_setup()

add_subdirectory(app)
