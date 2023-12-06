# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly

cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set( EXTRA_COMPONENT_DIRS  
	../UX-EES-ESP32CPPLIB/UxEspCppLibrary 
	../UX-EES-ESP32CPPLIB/PluginTasks/PluginTaskAt24c16/PluginTaskAt24c16
    ../UX-EES-ESP32CPPLIB/PluginTasks/PluginTaskWifiConnector/PluginTaskWifiConnector
	Application
	)

if ( "$ENV{PROJECT_NAME}" STREQUAL "" )
	project( HEM-EKX-EspFirmware )
else ()
	project( $ENV{PROJECT_NAME} )
endif()

# create version.h required for application build
add_custom_command( 
	OUTPUT version.h
	COMMAND make VERSION_JSON=EspFirmwareConfig.json -f ../version.mk;
	DEPENDS version.info
	COMMENT "Generating version.h file from version.info\n"
	VERBATIM)
add_custom_target( version DEPENDS version.h )
add_dependencies( ${PROJECT_NAME}.elf version )

