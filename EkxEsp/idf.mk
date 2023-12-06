EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/Application 
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/../UX-EES-ESP32CPPLIB/UxEspCppLibrary 
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/../UX-EES-ESP32CPPLIB/PluginTasks/PluginTaskAt24c16/PluginTaskAt24c16
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/../UX-EES-ESP32CPPLIB/PluginTasks/PluginTaskWifiConnector/PluginTaskWifiConnector

# parameter overwritten at call of idf.mk
PROJECT_NAME ?= wrong-default-idf-mk-name

include $(IDF_PATH)/make/project.mk

