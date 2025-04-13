# Collect all source files from app/classes/sources
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/app/classes/sources/*.cpp")

# Collect header files (useful for IDEs)
file(GLOB HEADERS "${CMAKE_SOURCE_DIR}/app/classes/headers/*.h")

# Collect UI files from the app/ui directory
file(GLOB FORMS "${CMAKE_SOURCE_DIR}/app/ui/*.ui")

# Collect resource files from the app/resources directory
file(GLOB RESOURCES "${CMAKE_SOURCE_DIR}/app/resources/*.qrc")


# Create the executable target.
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}     # (Optional: including headers can help IDEs recognize the project structure.)
    ${FORMS}
    ${RESOURCES}
    ${TRANSLATIONS} # Including TS files makes them visible to CMake; see custom target below.
)

# Include the header directory so that #include directives work correctly.
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/app/classes/headers)

# Enable automatic handling of Qt-specific build steps:
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTORCC ON     # Automatically process .qrc files.
)

# Link the Qt libraries.
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)
