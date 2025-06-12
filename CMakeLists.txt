cmake_minimum_required(VERSION 3.16)
project(ADUserManager VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

qt6_standard_project_setup()

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

# Qt setup
qt6_add_executable(ADUserManager ${SOURCES} ${HEADERS})
qt6_wrap_ui(UI_HEADERS ${UI_FILES})

target_link_libraries(ADUserManager PRIVATE 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Network
)

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
