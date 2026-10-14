
cmake_minimum_required(VERSION 3.25)

project(CliSrvValidator
    VERSION 0.1.0
    DESCRIPTION "Minecraft client-server mod validator"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Widgets
    Network
)

qt_standard_project_setup()


qt_add_executable(cli-srv-validator
    # Точка входа приложения.
    src/main.cpp

    # Состояния и результаты валидации.
    src/core/ValidationState.hpp

    # Чтение и сохранение настроек приложения.
    src/config/AppSettings.hpp
    src/config/AppSettings.cpp

    # Тестовый клиент сервера.
    src/services/ServerClient.hpp

    # Главное окно.
    src/ui/MainWindow.hpp
    src/ui/MainWindow.cpp

    # Отдельное окно настроек.
    src/ui/settings/SettingsWindow.hpp
    src/ui/settings/SettingsWindow.cpp
)

target_link_libraries(cli-srv-validator PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

target_include_directories(cli-srv-validator PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

