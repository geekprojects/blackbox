cmake_minimum_required(VERSION 4.0)
project(blackbox)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS src/ui)

find_library(GEOVIEW_LIBRARY qgeoview)
find_library(SQLITE3_LIBRARY NAMES sqlite3 )

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
qt_standard_project_setup()

find_package(PkgConfig REQUIRED)
#pkg_check_modules(libufc REQUIRED libufc)
pkg_check_modules(nlohmann_json REQUIRED nlohmann_json)
pkg_check_modules(libzmq REQUIRED libzmq)

add_definitions(
        -I/usr/local/include
        #${libufc_CFLAGS}
        ${nlohmann_json_CFLAGS}
        ${libzmq_CFLAGS}
)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(blackbox src/ui/main.cpp
        src/ui/navigraph.cpp
        src/ui/navigraph.h
        src/ui/map/route.cpp
        src/ui/map/route.h
        src/common/datastore.cpp
        src/common/logger.cpp
        src/ui/mainwindow.cpp
        src/ui/mainwindow.h
        src/ui/map/routemap.cpp
        src/ui/map/routemap.h
        src/ui/blackbox.cpp
        src/ui/blackbox.h
        src/ui/liveindicator.cpp
        src/ui/liveindicator.h
        src/ui/map/landingicon.cpp
        src/ui/map/landingicon.h
)
target_link_libraries(blackbox PRIVATE
        ${GEOVIEW_LIBRARY}
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        ${SQLITE3_LIBRARY}
)

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
)

target_compile_definitions(bbplugin PUBLIC ${XPLM_CFLAGS})
target_include_directories(bbplugin PUBLIC ${XPLANE_INC})
target_link_libraries(bbplugin
        ${XPLM_LDFLAGS}
        ${SQLITE3_LIBRARY}
)
