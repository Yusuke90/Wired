cmake_minimum_required(VERSION 3.16)
project(QtBrowser VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    WebEngineWidgets
    Sql
)

qt_standard_project_setup()

qt_add_executable(QtBrowser
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/custombrowserpage.h
    src/custombrowserpage.cpp
    src/downloadmanager.h
    src/downloadmanager.cpp
    src/apitesterwidget.h
    src/apitesterwidget.cpp
    src/historymanager.h
    src/historymanager.cpp
    src/bookmarkmanager.h
    src/bookmarkmanager.cpp
    src/historydialog.h
    src/historydialog.cpp
    src/bookmarkdialog.h
    src/bookmarkdialog.cpp
    resources.qrc
)

target_link_libraries(QtBrowser PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::Sql
)