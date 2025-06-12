cmake_minimum_required(VERSION 3.16)
project(ADUserManager VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем путь к Qt6
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/mingw_64")
set(Qt6_DIR "C:/Qt/6.9.1/mingw_64/lib/cmake/Qt6")

# Включаем только автоматическую MOC функцию Qt
set(CMAKE_AUTOMOC ON)  
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Source files
set(HEADERS
    include/models/ServerInfo.h
    include/models/UserInfo.h
    include/models/NormalizedUser.h
    include/services/ADManager.h
    include/services/LLMService.h
    include/services/PasswordGenerator.h
    include/services/ConfigManager.h
    include/ui/MainWindow.h
    include/ui/CreateUsersDialog.h
    include/ui/UserDetailsWidget.h
    include/ui/ServerTreeWidget.h
    include/utils/DataValidator.h
    include/utils/JsonHelper.h
    include/utils/StringUtils.h
)

set(SOURCES
    src/main.cpp
    src/models/ServerInfo.cpp
    src/models/UserInfo.cpp
    src/models/NormalizedUser.cpp
    src/services/ADManager.cpp
    src/services/LLMService.cpp
    src/services/PasswordGenerator.cpp
    src/services/ConfigManager.cpp
    src/ui/MainWindow.cpp
    src/ui/CreateUsersDialog.cpp
    src/ui/UserDetailsWidget.cpp
    src/ui/ServerTreeWidget.cpp
    src/utils/DataValidator.cpp
    src/utils/JsonHelper.cpp
    src/utils/StringUtils.cpp
)

set(UI_FILES
    ui/mainwindow.ui
    ui/createusersdialog.ui
    ui/userdetailswidget.ui
)

# Ресурсный файл Qt
set(RESOURCES
    resources/resources.qrc
)

# Создаём заглушки для UI файлов
foreach(UI_FILE ${UI_FILES})
    get_filename_component(UI_NAME ${UI_FILE} NAME_WE)
    set(UI_HEADER "${CMAKE_BINARY_DIR}/ui_${UI_NAME}.h")
    list(APPEND UI_HEADERS ${UI_HEADER})
    # Создаем пустые файлы заголовков UI
    file(WRITE ${UI_HEADER} "// Placeholder for UI file: ${UI_NAME}\n")
endforeach()

# Qt setup
add_executable(ADUserManager ${SOURCES} ${HEADERS})

target_link_libraries(ADUserManager PRIVATE 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Network
)

# Настройка копирования DLL в выходную директорию после сборки
if(WIN32)
    # Получение списка зависимостей Qt для копирования
    foreach(QT_LIB Core Gui Widgets Network)
        get_target_property(QT_DLL_PATH Qt6::${QT_LIB} IMPORTED_LOCATION_RELEASE)
        if(QT_DLL_PATH)
            # Добавляем команду копирования DLL в выходную директорию
            add_custom_command(TARGET ADUserManager POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${QT_DLL_PATH}
                $<TARGET_FILE_DIR:ADUserManager>
            )
        endif()
    endforeach()
    
    # Копирование необходимых папок плагинов
    foreach(PLUGIN_DIR platforms styles imageformats)
        file(GLOB PLUGIN_FILES "${Qt6Core_DIR}/../../../plugins/${PLUGIN_DIR}/*.dll")
        foreach(PLUGIN_FILE ${PLUGIN_FILES})
            # Создаем папку назначения, если она не существует
            add_custom_command(TARGET ADUserManager POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                $<TARGET_FILE_DIR:ADUserManager>/${PLUGIN_DIR}
            )
            # Копируем файл плагина
            add_custom_command(TARGET ADUserManager POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${PLUGIN_FILE}
                $<TARGET_FILE_DIR:ADUserManager>/${PLUGIN_DIR}/
            )
        endforeach()
    endforeach()
    
    # Создание qt.conf для правильного поиска плагинов
    file(WRITE "${CMAKE_BINARY_DIR}/qt.conf" 
        "[Paths]\n"
        "Plugins = ./\n"
        "Imports = ./qml\n"
        "Qml2Imports = ./qml\n"
    )
    # Копирование qt.conf в директорию сборки
    add_custom_command(TARGET ADUserManager POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/qt.conf"
        $<TARGET_FILE_DIR:ADUserManager>/qt.conf
    )
endif()

target_include_directories(ADUserManager PRIVATE ${CMAKE_BINARY_DIR})

# Windows-specific settings
if(WIN32)
    target_link_libraries(ADUserManager PRIVATE 
        activeds 
        adsiid 
        ole32 
        oleaut32
    )
    # Set app icon
    set(APP_ICON_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app.rc")
endif()

# Include directories
target_include_directories(ADUserManager PRIVATE include)

# Install rules
install(TARGETS ADUserManager
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Copy configuration files
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/config"
    DESTINATION share/ADUserManager
)

# Copy icons
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons"
    DESTINATION share/ADUserManager
    FILES_MATCHING PATTERN "*.svg" PATTERN "*.png"
)
