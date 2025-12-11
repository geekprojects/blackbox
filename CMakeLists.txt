cmake_minimum_required(VERSION 4.0)
project(BlackBox)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS src/ui)

find_library(GEOVIEW_LIBRARY qgeoview)
find_library(SQLITE3_LIBRARY NAMES sqlite3 )

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
qt_standard_project_setup(REQUIRES 6.8)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libfswatch REQUIRED libfswatch)
#pkg_check_modules(libufc REQUIRED libufc)
#pkg_check_modules(nlohmann_json REQUIRED nlohmann_json)

add_definitions(
        -I/usr/local/include
        ${libfswatch_CFLAGS}
)

include_directories(${CMAKE_SOURCE_DIR}/include)

qt_add_executable(BlackBox
        WIN32
        MACOSX_BUNDLE
        src/common/datastore.cpp
        src/common/logger.cpp
        src/common/utils.cpp
        src/common/utils.h
        src/ui/aircrafttypes.cpp
        src/ui/aircrafttypes.h
        src/ui/blackbox.cpp
        src/ui/blackbox.h
        src/ui/widgets/liveindicator.cpp
        src/ui/widgets/liveindicator.h
        src/ui/main.cpp
        src/ui/mainwindow.cpp
        src/ui/mainwindow.h
        src/ui/map/landingicon.cpp
        src/ui/map/landingicon.h
        src/ui/map/route.cpp
        src/ui/map/route.h
        src/ui/map/routemap.cpp
        src/ui/map/routemap.h
        src/ui/map/screenshoticon.cpp
        src/ui/map/screenshoticon.h
        src/ui/navigraph.cpp
        src/ui/navigraph.h
        src/ui/widgets/flightdetails.cpp
        src/ui/widgets/flightdetails.h
        src/ui/settingswindow.cpp
        src/ui/settingswindow.h
)

qt_add_resources(BlackBox "images"
        PREFIX "/images"
        BASE "data/images"
        FILES
        data/images/camera.svg
        data/images/landing.png
        data/images/airport.png
        data/images/tower-control.svg
        data/images/cog.svg
        data/images/globe.svg
        data/images/waypoints.svg
        data/images/map-pin.svg
        data/images/plane.svg
        data/images/plane-marker.svg
        data/images/plane-red.png
        data/images/trending-up-down.svg
)

target_link_libraries(BlackBox PRIVATE
        ${GEOVIEW_LIBRARY}
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        ${SQLITE3_LIBRARY}
)

set_target_properties(BlackBox PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE_GUI_IDENTIFIER com.geekprojects.blackbox.ui
        MACOSX_BUNDLE_BUNDLE_NAME BlackBox
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
)

#set_source_files_properties(data/images/landing.png PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/images" )

if(UNIX AND APPLE)
    set(XPLM_LDFLAGS ${CMAKE_SOURCE_DIR}/3rdparty/XPlaneSDK/Libraries/Mac/XPLM.framework)
    set(XPLM_CFLAGS -DAPL=1)
elseif (UNIX AND NOT APPLE)
    set(XPLM_CFLAGS -DLIN=1)
endif()

set(XPLANE_INC ${CMAKE_SOURCE_DIR}/3rdparty/XPlaneSDK/CHeaders/XPLM)

message("XPLM_LDFLAGS: ${XPLM_LDFLAGS}")
message("XPLANE_INC: ${XPLANE_INC}")

add_library(bbplugin SHARED
        src/plugin/plugin.cpp
        src/plugin/plugin.h
        src/plugin/statuswindow.cpp
        src/plugin/statuswindow.h
        src/plugin/Writer.cpp
        src/plugin/Writer.h
        src/common/logger.cpp
        src/common/datastore.cpp
        include/blackbox/state.h
        src/common/utils.cpp
        src/common/utils.h
        src/plugin/screenshots.cpp
        src/plugin/screenshots.h
        include/blackbox/screenshot.h
)

target_compile_definitions(bbplugin PUBLIC -DPLUGIN=1 ${XPLM_CFLAGS})
target_include_directories(bbplugin PUBLIC ${XPLANE_INC})
target_link_libraries(bbplugin
        ${XPLM_LDFLAGS}
        ${SQLITE3_LIBRARY}
        ${libfswatch_LDFLAGS}
)
