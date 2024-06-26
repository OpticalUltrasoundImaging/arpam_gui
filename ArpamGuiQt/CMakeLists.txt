project(ArpamGuiQt VERSION 1.0.0 LANGUAGES CXX)

set(EXE_NAME ArpamGuiQt)

#find_package(Vulkan REQUIRED)
find_package(Qt6 CONFIG REQUIRED COMPONENTS Widgets Gui PrintSupport)
qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)

# Manually include QCustomPlot
find_path(QCUSTOMPLOT_INCLUDE_DIR qcustomplot.h PATH_SUFFIXES qcustomplot)
if (WIN32)
    find_library(QCUSTOMPLOT_LIBRARY NAMES qcustomplot2)
    find_library(QCUSTOMPLOT_LIBRARY_DEBUG NAMES qcustomplotd2)

    set(QCUSTOMPLOT_LIBRARY "$<IF:$<CONFIG:Debug>,${QCUSTOMPLOT_LIBRARY_DEBUG},${QCUSTOMPLOT_LIBRARY}>")
else ()
    find_library(QCUSTOMPLOT_LIBRARY NAMES qcustomplot)
endif()

if (QCUSTOMPLOT_INCLUDE_DIR AND QCUSTOMPLOT_LIBRARY)
    message(STATUS "Found QCustomPlot: ${QCUSTOMPLOT_LIBRARY}")
else()
    message(FATAL_ERROR "Could not find QCustomPlot")
endif()

### Icons
# https://doc.qt.io/qt-6/appicon.html
# The MACOSX_BUNDLE_ICON_FILE variable is added to the Info.plist
# generated by CMake. This variable contains the .icns file name,
# without the path.
set(MACOSX_BUNDLE_ICON_FILE ArpamGui.icns)
set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/ArpamGui.icns")
set_source_files_properties(${app_icon_macos} PROPERTIES 
    MACOSX_PACKAGE_LOCATION "Resources")

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/ArpamGui.rc")

### Embed some compile time information as macros
# Current date and time
string(TIMESTAMP CURRENT_DATE "%Y-%m-%d" UTC)
string(TIMESTAMP CURRENT_TIME "%H:%M:%S" UTC)
add_definitions(-DARPAM_COMPILE_DATE="${CURRENT_DATE}")
add_definitions(-DARPAM_COMPILE_TIME="${CURRENT_TIME}")

# Compiler name and version
add_definitions(-DARPAM_COMPILER_NAME="${CMAKE_CXX_COMPILER_ID}")
add_definitions(-DARPAM_COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}")

### Source files
qt_add_executable(${EXE_NAME}
    src/main.cpp
    src/MainWindow.hpp
    src/MainWindow.cpp
    src/About.hpp
    src/About.cpp
    src/Canvas.hpp
    src/Canvas.cpp
    src/AScanPlot.hpp
    src/AScanPlot.cpp
    src/Annotation/Annotation.hpp
    src/Annotation/Annotation.cpp
    src/Annotation/AnnotationModel.hpp
    src/Annotation/AnnotationModel.cpp
    src/Annotation/AnnotationJsonFile.hpp
    src/Annotation/AnnotationJsonFile.cpp
    src/Annotation/AnnotationView.hpp
    src/Annotation/AnnotationView.cpp
    src/Annotation/GraphicsItemBase.cpp
    src/Annotation/GraphicsItemBase.hpp
    src/Annotation/GraphicsItems.cpp
    src/Annotation/GraphicsItems.hpp
    src/CanvasCursorState.hpp
    src/CanvasTicks.hpp
    src/CanvasTicks.cpp
    src/CanvasOverlay.hpp
    src/CanvasOverlay.cpp
    src/CoregDisplay.hpp
    src/CoregDisplay.cpp
    src/DataProcWorker.hpp
    src/DataProcWorker.cpp
    src/ReconParamsController.hpp
    src/ReconParamsController.cpp
    src/FrameController.hpp
    src/FrameController.cpp
    ${app_icon_macos}
    ${app_icon_resource_windows}
)
target_sources(${EXE_NAME} PRIVATE
    src/geometryUtils.cpp
    src/geometryUtils.hpp
    src/jsonUtils.cpp
    src/jsonUtils.hpp
)
qt_add_resources(${EXE_NAME} imageresources
    PREFIX "/"
    FILES 
    resources/images/radial_380.png
)
qt_add_resources(${EXE_NAME} styles
    PREFIX "/"
    FILES
    resources/styles/styles.qss
)

target_include_directories(${EXE_NAME} PRIVATE 
    src
    ${QCUSTOMPLOT_INCLUDE_DIR}
)

target_link_libraries(${EXE_NAME} PRIVATE
    libuspam
    Qt::Widgets
    Qt::Gui
    Qt::PrintSupport
    ${QCUSTOMPLOT_LIBRARY}
    #Vulkan::Vulkan
)

set_target_properties(${EXE_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    # INTERPROCEDURAL_OPTIMIZATION TRUE
)

if (WIN32)
    add_compile_definitions(_USE_MATH_DEFINES)

    # On Windows, call windeployqt create a deployable folder
    # https://doc.qt.io/qt-6/windows-deployment.html
    set_target_properties(${EXE_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )

    # Determine the path to windeployqt
    set(WINDEPLOYQT_PATH "${Qt6_DIR}/../../tools/Qt6/bin")
    set(WINDEPLOYQT_EXE "${WINDEPLOYQT_PATH}/windeployqt.exe")
    set(WINDEPLOYQT_DEBUG_EXE "${WINDEPLOYQT_PATH}/windeployqt.debug.bat")

    # Add post build command to call windeployqt
    add_custom_command(TARGET ${EXE_NAME} POST_BUILD
        COMMAND "$<IF:$<CONFIG:Debug>,${WINDEPLOYQT_DEBUG_EXE},${WINDEPLOYQT_EXE}>"
        --verbose 1
        "$<TARGET_FILE:${EXE_NAME}>"
    )

elseif (APPLE)


    # Post-build steps for macOS
    # https://doc.qt.io/qt-6/macos-deployment.html

    set_target_properties(${EXE_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST cmake/MacOSXBundleInfo.plist.in
    )

    # Find macdeployqt
    get_target_property(QT_BIN_DIR Qt::qmake IMPORTED_LOCATION)
    find_program(MACDEPLOYQT NAMES macdeployqt HINTS "${QT_BIN_DIR}")

    # Use macdeployqt to bundle Qt libraries
    add_custom_command(TARGET ${EXE_NAME} POST_BUILD
        COMMAND ${MACDEPLOYQT}
        "$<TARGET_BUNDLE_DIR:${EXE_NAME}>"
        -verbose=1
        -always-overwrite
        -codesign=-
        -no-strip
        "$<IF:$<CONFIG:Release>,-dmg,-use-debug-libs>"
        COMMENT "Bundling Qt libraries"
    )

endif()
