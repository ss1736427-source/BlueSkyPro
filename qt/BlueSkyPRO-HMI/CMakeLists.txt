cmake_minimum_required(VERSION 3.21)
project(BlueSkyPRO_HMI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Location Positioning Network WebEngineQuick WebChannel)

qt_standard_project_setup(REQUIRES 6.5)

qt_policy(SET QTP0004 NEW)

set_source_files_properties(qml/Theme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_executable(appBlueSkyPRO
    src/main.cpp
    src/PlanningBridge.cpp
    src/PlanningBridge.h
    src/ManualGeometryBridge.cpp
    src/ManualGeometryBridge.h
    src/ManualMissionModelAdapter.cpp
    src/ManualMissionModelAdapter.h
    src/RouteController.h
    src/TileCacheManager.cpp
    src/TileCacheManager.h
)

qt_add_qml_module(appBlueSkyPRO
    URI BlueSky.PRO
    VERSION 1.0
    QML_FILES
        qml/App.qml
        qml/Main.qml
        qml/MainContent.qml
        qml/TopHeader.ui.qml
        qml/HeaderSector.ui.qml
        qml/LeftPanel.qml
        qml/FlightChart.ui.qml
        qml/GoogleMapView.qml
        qml/MapTilerWebMap.qml
        qml/RightPanel.ui.qml
        qml/RoleSelection.ui.qml
        qml/TaskCreation.ui.qml
        qml/MissionProfileWindow.qml
        qml/ThreeDMappingProfile.qml
        qml/UAVFleetPanel.qml
        qml/UAVFleetPanelForm.ui.qml
        qml/BottomToolbar.qml
        qml/ContextOverlay.ui.qml
        qml/ToolContext.ui.qml
        qml/IntegrationConsole.qml
        qml/VirtualFlightTestConsole.qml
        qml/PanelSettingsButton.ui.qml
        qml/PanelSettingsPopup.qml
        qml/Theme.qml
    RESOURCES
        qml/icons8-menu-24.svg
        qml/assets/uav_multirotor.svg
        qml/assets/uav_fixed_wing.svg
        qml/assets/uav_heavy_multirotor.svg
        qml/assets/uav_vtol.svg
)

target_include_directories(appBlueSkyPRO PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(appBlueSkyPRO PRIVATE Qt6::Core Qt6::Quick Qt6::Location Qt6::Positioning Qt6::Network Qt6::WebEngineQuick Qt6::WebChannel)

enable_testing()
add_executable(tile_cache_manager_test
    tests/TileCacheManagerTest.cpp
    src/TileCacheManager.cpp
    src/TileCacheManager.h
)
target_link_libraries(tile_cache_manager_test PRIVATE Qt6::Network)
add_test(NAME tile_cache_manager_contract COMMAND tile_cache_manager_test)

add_executable(route_controller_test
    tests/RouteControllerTest.cpp
    src/RouteController.h
)
target_include_directories(route_controller_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(route_controller_test PRIVATE Qt6::Core)
add_test(NAME route_controller_fail_closed_contract COMMAND route_controller_test)

add_executable(manual_mission_model_adapter_test
    tests/ManualMissionModelAdapterTest.cpp
    src/ManualMissionModelAdapter.cpp
    src/ManualMissionModelAdapter.h
)
target_include_directories(manual_mission_model_adapter_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(manual_mission_model_adapter_test PRIVATE Qt6::Core)
add_test(NAME manual_mission_model_adapter_contract COMMAND manual_mission_model_adapter_test)

set_target_properties(appBlueSkyPRO PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
