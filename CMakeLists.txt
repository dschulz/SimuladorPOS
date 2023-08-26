cmake_minimum_required(VERSION 3.14)

project(SimuladorPOS LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core HttpServer)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core HttpServer)

add_executable(SimuladorPOS
  main.cpp
)
target_link_libraries(SimuladorPOS Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::HttpServer)

qt_add_resources(SimuladorPOS "assets"
    PREFIX
        "/"
    FILES
        assets/info.txt
#        assets/cert.pem
#        assets/priv.pem
)


include(GNUInstallDirs)
install(TARGETS SimuladorPOS
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
