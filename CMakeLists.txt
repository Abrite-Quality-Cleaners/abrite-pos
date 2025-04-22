cmake_minimum_required(VERSION 3.10)

project(abrite-pos VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Find MongoDB C++ Driver
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

# Add Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Project sources
set(PROJECT_SOURCES
    main.cpp
    CustomerDialog.cpp
    WindowController.cpp
    LoginWindow.cpp
    StoreSelectionWindow.cpp
    ClientSelectionWindow.cpp
    DropoffWindow.cpp
    PaymentDialog.cpp
    User.cpp
    Store.cpp
    Customer.cpp
    MongoManager.cpp
    PickupWindow.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(abrite-pos
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(abrite-pos SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(abrite-pos
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Link Qt libraries
target_link_libraries(abrite-pos PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Link MongoDB C++ Driver libraries
target_include_directories(abrite-pos PRIVATE /usr/local/include/mongocxx/v_noabi /usr/local/include/bsoncxx/v_noabi)
link_directories({/usr/local/lib})
target_link_libraries(abrite-pos PRIVATE mongocxx bsoncxx)

# Add the test executable
add_executable(MongoManagerTest MongoManagerTest.cpp MongoManager.cpp)
target_include_directories(MongoManagerTest PRIVATE /usr/local/include/mongocxx/v_noabi /usr/local/include/bsoncxx/v_noabi)
link_directories({/usr/local/lib})
target_link_libraries(MongoManagerTest PRIVATE GTest::GTest GTest::Main mongocxx bsoncxx Qt${QT_VERSION_MAJOR}::Widgets)

# Set target properties
set_target_properties(abrite-pos PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Install target
install(TARGETS abrite-pos
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(abrite-pos)
endif()
