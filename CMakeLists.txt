cmake_minimum_required(VERSION 3.5)

project(gstreamer-log-viewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(gstreamer-log-viewer
    MANUAL_FINALIZATION
    main.cpp

    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    mainwindow.qrc

    gstreamerlogmodel.h
    gstreamerlogmodel.cpp

    gstreamerlogview.h
    gstreamerlogview.cpp

    gstreamerlogwidget.h
    gstreamerlogwidget.cpp
    gstreamerlogwidget.ui

    timestamp.h
    timestamp.cpp

    timestampview.h
    timestampview.cpp

    customfilterproxymodel.h
    customfilterproxymodel.cpp

    preferences.h
    preferences.cpp
    preferences.ui

    lineedit.h
    lineedit.cpp
    main.qrc
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gstreamer-log-viewer PRIVATE Qt6::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.gstreamer-log-viewer)
endif()
set_target_properties(gstreamer-log-viewer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS gstreamer-log-viewer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(gstreamer-log-viewer)
