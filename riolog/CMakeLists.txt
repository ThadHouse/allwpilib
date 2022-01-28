project(riolog)

include(CompileWarnings)
include(GenResources)
include(LinkMacOSGUI)

configure_file(src/main/generate/WPILibVersion.cpp.in WPILibVersion.cpp)
GENERATE_RESOURCES(src/main/native/resources generated/main/cpp RIOLOG riolog riolog_resources_src)

file(GLOB riolog_src src/main/native/cpp/*.cpp ${CMAKE_CURRENT_BINARY_DIR}/WPILibVersion.cpp)

if (WIN32)
    set(riolog_rc src/main/native/win/riolog.rc)
elseif(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE riolog.icns)
    set(APP_ICON_MACOSX src/main/native/mac/giolog.icns)
    set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

add_executable(roborioteamnumbersetter ${riolog_src} ${riolog_resources_src} ${riolog_rc} ${APP_ICON_MACOSX})
wpilib_link_macos_gui(riolog)
target_link_libraries(riolog libglass)

if (WIN32)
    set_target_properties(riolog PROPERTIES WIN32_EXECUTABLE YES)
elseif(APPLE)
    set_target_properties(riolog PROPERTIES MACOSX_BUNDLE YES OUTPUT_NAME "riolog")
endif()
