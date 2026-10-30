cmake_minimum_required(VERSION 3.21)
project(BlueSkyPRO_HMI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Location Positioning Network WebEngineQuick WebChannel)

# Bundle the pinned MapLibre GL JS runtime into the Qt resource system.
# This removes the application's runtime dependency on a third-party CDN.
set(MAPLIBRE_VERSION "5.12.0")
set(MAPLIBRE_ARCHIVE "${CMAKE_CURRENT_BINARY_DIR}/maplibre-gl-${MAPLIBRE_VERSION}.tgz")
set(MAPLIBRE_EXTRACT_DIR "${CMAKE_CURRENT_BINARY_DIR}/maplibre-npm")
file(DOWNLOAD
    "https://registry.npmjs.org/maplibre-gl/-/maplibre-gl-${MAPLIBRE_VERSION}.tgz"
    "${MAPLIBRE_ARCHIVE}"
    TLS_VERIFY ON
    STATUS MAPLIBRE_DOWNLOAD_STATUS
)
list(GET MAPLIBRE_DOWNLOAD_STATUS 0 MAPLIBRE_DOWNLOAD_CODE)
if(NOT MAPLIBRE_DOWNLOAD_CODE EQUAL 0)
    list(GET MAPLIBRE_DOWNLOAD_STATUS 1 MAPLIBRE_DOWNLOAD_MESSAGE)
    message(FATAL_ERROR "Failed to download pinned MapLibre GL JS ${MAPLIBRE_VERSION}: ${MAPLIBRE_DOWNLOAD_MESSAGE}")
endif()
file(ARCHIVE_EXTRACT INPUT "${MAPLIBRE_ARCHIVE}" DESTINATION "${MAPLIBRE_EXTRACT_DIR}")
set(MAPLIBRE_JS "${MAPLIBRE_EXTRACT_DIR}/package/dist/maplibre-gl.js")
set(MAPLIBRE_CSS "${MAPLIBRE_EXTRACT_DIR}/package/dist/maplibre-gl.css")
if(NOT EXISTS "${MAPLIBRE_JS}" OR NOT EXISTS "${MAPLIBRE_CSS}")
    message(FATAL_ERROR "Pinned MapLibre GL JS package is missing its dist JavaScript or CSS file")
endif()
set_source_files_properties("${MAPLIBRE_JS}" PROPERTIES QT_RESOURCE_ALIAS "maplibre-gl.js")
set_source_files_properties("${MAPLIBRE_CSS}" PROPERTIES QT_RESOURCE_ALIAS "maplibre-gl.css")

qt_standard_project_setup(REQUIRES 6.5)

qt_policy(SET QTP0004 NEW)

set_source_files_properties(qml/Theme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_executable(appBlueSkyPRO
    src/main.cpp
    src/PlanningBridge.cpp
    src/PlanningBridge.h
    src/MissionTemplateResolver.cpp
    src/MissionTemplateResolver.h
    src/VirtualFlightAssessmentBridge.cpp
    src/VirtualFlightAssessmentBridge.h
    ../../04_SOFTWARE/PLANNING/manual_route_candidate_builder.cpp
    ../../04_SOFTWARE/PLANNING/manual_route_candidate_assessment.cpp
    ../../04_SOFTWARE/PLANNING/constrained_open_space.cpp
    ../../04_SOFTWARE/PLANNING/notam_prohibited_zone_validator.cpp
    ../../04_SOFTWARE/PLANNING/route_constraint_validator.cpp
    src/ManualGeometryBridge.cpp
    src/ManualGeometryBridge.h
    src/ManualMissionModelAdapter.cpp
    src/ManualMissionModelAdapter.h
    src/RouteController.h
    src/TileCacheManager.cpp
    src/TileCacheManager.h
    src/CredentialVault.cpp
    src/CredentialVault.h
    src/FaaNmsClient.cpp
    src/FaaNmsClient.h
    src/NotamGeometryEngine.cpp
    src/NotamGeometryEngine.h
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
        qml/ExternalIntegrationsWorkspace.qml
        qml/AdminWorkspace.qml
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

qt_add_resources(appBlueSkyPRO maplibre_vendor
    PREFIX "/"
    FILES
        "${MAPLIBRE_JS}"
        "${MAPLIBRE_CSS}"
)

target_compile_features(appBlueSkyPRO PRIVATE cxx_std_20)
target_include_directories(appBlueSkyPRO PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(appBlueSkyPRO PRIVATE Qt6::Core Qt6::Quick Qt6::Location Qt6::Positioning Qt6::Network Qt6::WebEngineQuick Qt6::WebChannel)
if(WIN32)
    target_link_libraries(appBlueSkyPRO PRIVATE Advapi32)
endif()

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

add_executable(notam_geometry_engine_test
    tests/NotamGeometryEngineTest.cpp
    src/NotamGeometryEngine.cpp
    src/NotamGeometryEngine.h
)
target_link_libraries(notam_geometry_engine_test PRIVATE Qt6::Core)
add_test(NAME notam_geometry_engine_contract COMMAND notam_geometry_engine_test)

add_executable(manual_mission_model_adapter_test
    tests/ManualMissionModelAdapterTest.cpp
    src/ManualMissionModelAdapter.cpp
    src/ManualMissionModelAdapter.h
)
target_include_directories(manual_mission_model_adapter_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(manual_mission_model_adapter_test PRIVATE Qt6::Core)
target_compile_options(manual_mission_model_adapter_test PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>"
    "$<$<CXX_COMPILER_ID:MSVC>:/UNDEBUG>"
)
add_test(NAME manual_mission_model_adapter_contract COMMAND manual_mission_model_adapter_test)

add_executable(mission_template_resolver_test
    tests/MissionTemplateResolverTest.cpp
    src/MissionTemplateResolver.cpp
    src/MissionTemplateResolver.h
)
target_include_directories(mission_template_resolver_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(mission_template_resolver_test PRIVATE Qt6::Core)
add_test(NAME mission_template_resolver_contract COMMAND mission_template_resolver_test)

add_executable(virtual_flight_assessment_bridge_test
    tests/VirtualFlightAssessmentBridgeTest.cpp
    src/VirtualFlightAssessmentBridge.cpp
    src/VirtualFlightAssessmentBridge.h
    src/ManualMissionModelAdapter.cpp
    src/ManualMissionModelAdapter.h
    ../../04_SOFTWARE/PLANNING/manual_route_candidate_builder.cpp
    ../../04_SOFTWARE/PLANNING/manual_route_candidate_assessment.cpp
    ../../04_SOFTWARE/PLANNING/constrained_open_space.cpp
    ../../04_SOFTWARE/PLANNING/notam_prohibited_zone_validator.cpp
    ../../04_SOFTWARE/PLANNING/route_constraint_validator.cpp
)
target_compile_features(virtual_flight_assessment_bridge_test PRIVATE cxx_std_20)
target_include_directories(virtual_flight_assessment_bridge_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(virtual_flight_assessment_bridge_test PRIVATE Qt6::Core)
target_compile_options(virtual_flight_assessment_bridge_test PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>"
    "$<$<CXX_COMPILER_ID:MSVC>:/UNDEBUG>"
)
add_test(NAME virtual_flight_assessment_bridge_contract COMMAND virtual_flight_assessment_bridge_test)

set_target_properties(appBlueSkyPRO PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

add_executable(credential_vault_test
    tests/CredentialVaultTest.cpp
    src/CredentialVault.cpp
    src/CredentialVault.h
)
target_link_libraries(credential_vault_test PRIVATE Qt6::Core)
if(WIN32)
    target_link_libraries(credential_vault_test PRIVATE Advapi32)
endif()
if(WIN32)
    add_test(NAME credential_vault_process_write COMMAND credential_vault_test write)
    add_test(NAME credential_vault_process_read COMMAND credential_vault_test read)
    add_test(NAME credential_vault_process_delete COMMAND credential_vault_test delete)
    set_tests_properties(credential_vault_process_read PROPERTIES DEPENDS credential_vault_process_write)
    set_tests_properties(credential_vault_process_delete PROPERTIES DEPENDS credential_vault_process_read)
else()
    add_test(NAME credential_vault_fail_closed COMMAND credential_vault_test)
endif()
