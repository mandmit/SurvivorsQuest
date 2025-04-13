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

# Optional: Add a custom target for lrelease processing.
# This mimics CONFIG += lrelease and embed_translations in your qmake file.
# It will run lrelease on your .ts file to generate a .qm file.
find_program(LRELEASE_EXECUTABLE NAMES lrelease HINTS "${Qt6_DIR}/../../../bin")
if(LRELEASE_EXECUTABLE)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/SurvivorsQuest_uk_UA.qm
        COMMAND ${LRELEASE_EXECUTABLE} ${CMAKE_SOURCE_DIR}/SurvivorsQuest_uk_UA.ts -qm ${CMAKE_CURRENT_BINARY_DIR}/SurvivorsQuest_uk_UA.qm
        DEPENDS ${CMAKE_SOURCE_DIR}/SurvivorsQuest_uk_UA.ts
        COMMENT "Running lrelease on translation file"
    )
    add_custom_target(translations ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/SurvivorsQuest_uk_UA.qm)
endif()

# Installation rules
if(UNIX AND NOT ANDROID)
    # Mimic the qmake target.path = /opt/$${TARGET}/bin rule
    install(TARGETS ${PROJECT_NAME} DESTINATION /opt/${PROJECT_NAME}/bin)
endif()
