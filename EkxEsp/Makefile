SHELL					:= /bin/bash

ROOT_DIR				:= $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

BUILD_DIR				?= $(ROOT_DIR)/build
BUILD_NO				?= 9999
BUILD_DATE				:= $(shell date +%d.%m.%Y)

VERSION_INFO			:= $(ROOT_DIR)/version.info
VERSION_INCLUDE			:= $(BUILD_DIR)/version.h

TARGET					:= $(shell cat "$(VERSION_INFO)" | sed -n -E 's@Component=(.+)$$@\1@p' )
VERSIONED_TARGET		?= $(shell echo "$(TARGET)-V$(VERSION_NO)$(XDEV_STATE)" | sed -n -E 's@\.@\-@p' )

CURRENT_BRANCH			?= $(shell git branch 2>/dev/null | grep "\* " | sed -e "s/\* //g" | sed -e "s/ /_/g" | sed -e "s/(//g" | sed -e "s/)//g" )
CURRENT_SHA				?= $(shell git log -1 --pretty=format:%H )

PROJECT_IDF_VERSION		?= v5.0
PROJECT_IDF_PATH		:= /opt/esp/esp-idf-$(PROJECT_IDF_VERSION)

IDF_WARNING       :=
TOOLCHAIN_WARNING :=

OTA_INIT_BIN      		?= $(BUILD_DIR)/ota_data_initial.bin
BOOTLOADER_BIN    		?= $(BUILD_DIR)/bootloader/bootloader.bin
PARTITIONS_BIN    		?= $(BUILD_DIR)/partition_table/partition-table.bin
APP_BIN           		?= $(BUILD_DIR)/$(VERSIONED_TARGET).bin
ESPTOOL_PATH      		?= $(PROJECT_IDF_PATH)/components/esptool_py/esptool
ERASE_FLASH_SCRIPT		?= $(ROOT_DIR)/Scripts/do_erase_flash.sh
FLASH_SCRIPT      		?= $(ROOT_DIR)/Scripts/do_flash.sh
OTA_ESP_UPDATE_SCRIPT 	?= $(ROOT_DIR)/Scripts/do_ota_esp_update.sh
UXESPCPPLIBRARY_PATH	?= $(ROOT_DIR)/../UX-EES-ESP32CPPLIB
UXESPCPPLIBRARY_CHECK	=$(shell cd $(UXESPCPPLIBRARY_PATH) && ( git status | grep $(CPPLIB_TAG) ) )
HASH_LOG		  		:= hash.log

VERSION_INFO			?= $(ROOT_DIR)/version.info
VERSION_INCLUDE			?= $(BUILD_DIR)/version.h
VERSION_JSON 			?= $(BUILD_DIR)/EspFirmwareConfig.json
VERSION_NO				?= $(shell cat "$(VERSION_INFO)" | sed -n -E 's@VersionNo=(.+)$$@\1@p' )
VERSION_MAJOR			?= $(shell cat "$(VERSION_INFO)" | sed -n -E 's@VersionNo=(.+)\..+$$@\1@p' )
VERSION_MINOR			?= $(shell cat "$(VERSION_INFO)" | sed -n -E 's@VersionNo=.+\.(.+)$$@\1@p' )
SW_VERSION				?= $(VERSION_MAJOR)$(VERSION_MINOR)
VERSION_DATE			?= $(shell cat "$(VERSION_INFO)" | sed -n -E 's@VersionDate=(.+)$$@\1@p' )
DEV_STATE				?= $(shell cat "$(VERSION_INFO)" | sed -n -E 's@DevState=(.+)$$@\1@p' )
CPPLIB_TAG				?= $(shell cat "$(VERSION_INFO)" | sed -n -E 's@UxEsp32CppLibTag=(.+)$$@\1@p' )
CPPLIB_REPO				?= ssh://git@ux-git.u-experten.de:224/Ux-Eco/UX-MOVE-E/UX-EES-ESP32CPPLIB.git
XDEV_STATE				= -$(DEV_STATE)

ARCHIVE_NAME			?= $(VERSIONED_TARGET)
ARCHIVE_FILE			?= $(ARCHIVE_NAME).zip
ARCHIVE_PATH			?= $(BUILD_DIR)/$(ARCHIVE_FILE)
ARCHIVE_ZIP_PATH		?= $(BUILD_DIR)/$(ARCHIVE_NAME)

ifeq ("$(DEV_STATE)","")
	XDEV_STATE	= ""
endif

ifdef IDF_PATH
%:
	echo "."
	echo "Error!! You're using unsupported environment setting IDF_PATH."
	echo "Revert to project settings and remove IDF variables from environment!"
	echo "."
	exit 1
endif

ifdef XTENSA_TOOLCHAIN
%:
	echo "."
	echo "Error!! You're using unsupported environment setting XTENSA_TOOLCHAIN."
	echo "Revert to project settings and remove IDF variables from environment!"
	echo "."
	exit 1
endif

help:
	@echo "Targets:"
	@echo -e "\tclean       removes the build directory"
	@echo -e "\tidf         installs the compatible IDF version"
	@echo -e "\tclean_idf   removes the compatible IDF directory"
	@echo -e "\tdelivery    builds a zip package with all relevant artefacts and scripts"
	@echo -e "\tcpplib      installs compliant UX-EES-ESP32CPPLIB in parallel directory"
	@echo -e "\t%           all other targets provided by idf.py"

clean:
	@rm -rf build

idf:
	@./install_idf.sh $(PROJECT_IDF_VERSION)

clean_idf:
	@rm -rf $(PROJECT_IDF_VERSION)

cpplib:
	rm -rf $(ROOT_DIR)/../UX-EES-ESP32CPPLIB
	cd $(ROOT_DIR)/..;rm -rf $(ROOT_DIR)/../UX-EES-ESP32CPPLIB
	cd $(ROOT_DIR)/..;git clone $(CPPLIB_REPO);cd UX-EES-ESP32CPPLIB;git checkout $(CPPLIB_TAG)

delivery: $(BOOTLOADER_BIN) $(PARTITIONS_BIN) $(APP_BIN) $(ESPTOOL_PATH) $(FLASH_SCRIPT) $(ERASE_FLASH_SCRIPT)
	@if [ "$(VERSION_NO)" = "" ]; \
	then \
		echo "fatal error - please define VERSION=xx_yy"; \
		exit 1; \
	fi
	@rm -rf $(ARCHIVE_ZIP_PATH)
	@mkdir -p $(ARCHIVE_ZIP_PATH)
	@cp $(OTA_INIT_BIN) $(ARCHIVE_ZIP_PATH)
	echo $(BOOTLOADER_BIN) $(ARCHIVE_ZIP_PATH)
	@cp $(BOOTLOADER_BIN) $(ARCHIVE_ZIP_PATH)
	@cp $(PARTITIONS_BIN) $(ARCHIVE_ZIP_PATH)
	@cp $(APP_BIN) $(ARCHIVE_ZIP_PATH)/
	@cp $(VERSION_JSON) $(ARCHIVE_ZIP_PATH)/
	@cp -r $(ESPTOOL_PATH) $(ARCHIVE_ZIP_PATH)
	@cat $(FLASH_SCRIPT) | sed -r "s/EspFirmware.bin/$(VERSIONED_TARGET).bin/g" > $(ARCHIVE_ZIP_PATH)/do_flash.sh
	@cat $(OTA_ESP_UPDATE_SCRIPT) | sed -r "s/EspFirmware.bin/$(VERSIONED_TARGET).bin/g" > $(ARCHIVE_ZIP_PATH)/do_ota_esp_update.sh
	@git log -1 --pretty=format:%H > $(ARCHIVE_ZIP_PATH)/$(HASH_LOG)
	@chmod 777 $(ARCHIVE_ZIP_PATH)/*.sh
	@cd $(BUILD_DIR);rm -f $(ARCHIVE_FILE);zip -r $(ARCHIVE_FILE) $(ARCHIVE_NAME)

$(BOOTLOADER_BIN):
	@echo "."
	@echo "fatal error - no bootloader found at <$(BOOTLOADER_BIN)>"
	@echo "Please run 'make all' first ..."
	@echo "."
	@exit 1

$(PARTITIONS_BIN):
	@echo "."
	@echo "fatal error - no partitions binary found at <$(PARTITIONS_BIN)>"
	@echo "Please run 'make all' first ..."
	@echo "."
	@exit 1

$(APP_BIN):
	@echo "."
	@echo "fatal error - no app found at <$(APP_BIN)>"
	@echo "Please run 'make all' first ..."
	@echo "."
	@exit 1

$(UXESPCPPLIBRARY_PATH):
	@echo "."
	@echo "fatal error - UX ESP32 C++ library is required but not found at <$(UXESPCPPLIBRARY_PATH)>"
	@echo "Please checkout from git on same level as project."
	@echo "You can use 'make cpplib'!"
	@echo "."
	@exit 1

$(ESPTOOL_PATH):
	@echo "."
	@echo "fatal error - no esp tool path found at <$(ESPTOOL_PATH)>"
	@echo "Please use correct instal path or overwrite ESP_INSTALL_DIR ..."
	@echo "."
	@exit 1

$(FLASH_SCRIPT):
	@echo "."
	@echo "fatal error - no flash script found at <$(FLASH_SCRIPT)>"
	@echo "Please checkout project with 'repo' to ensure Scripts folder is present ..."
	@echo "."
	@exit 1

$(PROJECT_IDF_PATH):
	@echo "."
	@echo "fatal error - no compatible IDF found at <$(PROJECT_IDF_PATH)>"
	@echo "Please use 'make idf' to download and install the compatible IDF version ..."
	@echo "."
	@exit 1

ux_library_check:
	@if [ -z '$(UXESPCPPLIBRARY_CHECK)' ]; \
	then \
		echo "."; \
		echo "You're using an incompatible tag or branch of UX-EES-ESP32CPPLIB"; \
		echo "Please use 'make cpplib' to retrieve clean state"; \
		echo "."; \
	fi 

#####################################
# temp files relevant for build
#####################################

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

#####################################
# must allways be the last target / "catch all the rest"
#####################################

.DEFAULT: 
	@make $(PROJECT_IDF_PATH) $(UXESPCPPLIBRARY_PATH) $(BUILD_DIR)
	@make -f version.mk
	@mv build/$(TARGET)Version.json "$(VERSION_JSON)"
	#@source $(PROJECT_IDF_PATH)/export.sh && PROJECT_NAME=$(VERSIONED_TARGET) make -j16 -f idf.mk $@
	@source $(PROJECT_IDF_PATH)/export.sh && PROJECT_NAME="$(VERSIONED_TARGET)" idf.py $@
	@make ux_library_check

