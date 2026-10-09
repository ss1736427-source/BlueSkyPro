cmake_minimum_required(VERSION 3.21)
project(BlueSkyPRO_HMI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network)

qt_standard_project_setup(REQUIRES 6.5)

qt_policy(SET QTP0004 NEW)

set_source_files_properties(qml/Theme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_executable(appBlueSkyPRO
    src/main.cpp
    src/PlanningBridge.cpp
    src/PlanningBridge.h
    src/RouteGeometrySerializer.cpp
    src/RouteGeometrySerializer.h
    src/SelectedRouteMapHandoff.cpp
    src/SelectedRouteMapHandoff.h
    ../../04_SOFTWARE/PLANNING/selected_route_set.cpp
    ../../04_SOFTWARE/PLANNING/canonical_route_builder.cpp
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

target_link_libraries(appBlueSkyPRO PRIVATE
    Qt6::Quick
    Qt6::Network
)

set_target_properties(appBlueSkyPRO PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

enable_testing()
add_executable(route_geometry_serializer_test
    tests/RouteGeometrySerializerTest.cpp
    src/RouteGeometrySerializer.cpp
    src/RouteGeometrySerializer.h
)
target_link_libraries(route_geometry_serializer_test PRIVATE Qt6::Quick)
add_test(NAME route_geometry_serializer_contract COMMAND route_geometry_serializer_test)

add_executable(selected_route_map_handoff_test
    tests/SelectedRouteMapHandoffTest.cpp
    src/SelectedRouteMapHandoff.cpp
    src/RouteGeometrySerializer.cpp
    ../../04_SOFTWARE/PLANNING/selected_route_set.cpp
    ../../04_SOFTWARE/PLANNING/canonical_route_builder.cpp
)
target_link_libraries(selected_route_map_handoff_test PRIVATE Qt6::Quick)
add_test(NAME selected_route_map_handoff_contract COMMAND selected_route_map_handoff_test)
