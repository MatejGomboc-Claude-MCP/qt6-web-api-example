cmake_minimum_required(VERSION 3.18)

project(qt6-web-api-example VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network HttpServer)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/apiserver.h
    src/apiserver.cpp
    src/problemdetail.h
    src/problemdetail.cpp
    src/configmanager.h
    src/configmanager.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::HttpServer
)

# Install the executable
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install the config.json file
install(FILES config.json
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
)