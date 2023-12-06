
PWD						:= $(shell pwd)
ROOT_DIR				:= $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

BUILD_DIR				?= $(ROOT_DIR)/build
BUILD_NO				?= 9999
BUILD_DATE				?= $(shell date +%d.%m.%Y)

TARGET          		?= $(shell cat "$(VERSION_INFO)" | sed -n -E 's@Component=(.+)$$@\1@p' )
VERSIONED_TARGET		?= $(shell echo "$(TARGET)-V$(VERSION_NO)$(XDEV_STATE)" | sed -n -E 's@\.@\-@p' )

CURRENT_BRANCH			?= $(shell git branch 2>/dev/null | grep "\* " | sed -e "s/\* //g" | sed -e "s/ /_/g" | sed -e "s/(//g" | sed -e "s/)//g" )
CURRENT_SHA             ?= $(shell git log -1 --pretty=format:%H )
VERSION_INFO			?= $(ROOT_DIR)/version.info
VERSION_INCLUDE			?= $(BUILD_DIR)/version.h
VERSION_JSON 			?= $(BUILD_DIR)/$(TARGET)Version.json
VERSION_NO				?= $(shell cat "$(VERSION_INFO)" | sed -n -E 's@VersionNo=(.+)$$@\1@p' )
VERSION_MAJOR			?= $(shell cat "$(VERSION_INFO)" | sed -n -E 's@VersionNo=(.+)\..+$$@\1@p' )
VERSION_MINOR			?= $(shell cat "$(VERSION_INFO)" | sed -n -E 's@VersionNo=.+\.(.+)$$@\1@p' )
SW_VERSION				?= $(VERSION_MAJOR)$(VERSION_MINOR)
VERSION_DATE			?= $(shell cat "$(VERSION_INFO)" | sed -n -E 's@VersionDate=(.+)$$@\1@p' )
DEV_STATE				?= $(shell cat "$(VERSION_INFO)" | sed -n -E 's@DevState=(.+)$$@\1@p' )
XDEV_STATE				= -$(DEV_STATE)

ifeq ("$(DEV_STATE)","")
	XDEV_STATE	= ""
endif

all: $(VERSION_INCLUDE) $(VERSION_JSON)

clean:
	@rm -f $(VERSION_INCLUDE).tmp
	@rm -f $(VERSION_INCLUDE)
	@rm -f $(VERSION_JSON)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(VERSION_INCLUDE): .FORCE $(BUILD_DIR)
	@printf '#define TARGET        "%s"\n' "$(TARGET)"               >  $(VERSION_INCLUDE).tmp
	@printf '#define VERSION_NO    "%s"\n' "$(VERSION_NO)"           >> $(VERSION_INCLUDE).tmp
	@printf '#define VERSION_MAJOR %d\n'   "$(VERSION_MAJOR)"        >> $(VERSION_INCLUDE).tmp
	@printf '#define VERSION_MINOR %.0f\n' "$(VERSION_MINOR)"        >> $(VERSION_INCLUDE).tmp
	@printf '#define VERSION_DATE  "%s"\n' "$(VERSION_DATE)"         >> $(VERSION_INCLUDE).tmp
	@printf '#define DEV_STATE     "%s"\n' "$(DEV_STATE)"            >> $(VERSION_INCLUDE).tmp
	@printf '#define BUILD_NO      "%s"\n' "$(BUILD_NO)"             >> $(VERSION_INCLUDE).tmp
	@printf '#define BUILD_DATE    "%s"\n' "$(BUILD_DATE)"           >> $(VERSION_INCLUDE).tmp
	@printf '#define BRANCH        "%s"\n' "$(CURRENT_BRANCH)"       >> $(VERSION_INCLUDE).tmp
	@printf '#define SHA           "%s"\n' "$(CURRENT_SHA)"          >> $(VERSION_INCLUDE).tmp
	@printf '#define TARGET_FILE   "%s"\n' "$(VERSIONED_TARGET).bin" >> $(VERSION_INCLUDE).tmp

	@if `cmp -s "$(VERSION_INCLUDE)" "$(VERSION_INCLUDE).tmp"`; \
	then \
		rm -r "$(VERSION_INCLUDE).tmp"; \
	else \
		mv -f "$(VERSION_INCLUDE).tmp" "$(VERSION_INCLUDE)"; \
	fi

$(VERSION_JSON): .FORCE $(BUILD_DIR)
	@printf '{\n'                                                     >  $(VERSION_JSON).tmp
	@printf '    "TARGET"        : "%s",\n' "$(TARGET)"               >> $(VERSION_JSON).tmp
	@printf '    "VERSION_NO"    : "%s",\n' "$(VERSION_NO)"           >> $(VERSION_JSON).tmp
	@printf '    "VERSION_MAJOR" : %d,\n'   "$(VERSION_MAJOR)"        >> $(VERSION_JSON).tmp
	@printf '    "VERSION_MINOR" : %.0f,\n' "$(VERSION_MINOR)"        >> $(VERSION_JSON).tmp
	@printf '    "VERSION_DATE"  : "%s",\n' "$(VERSION_DATE)"         >> $(VERSION_JSON).tmp
	@printf '    "DEV_STATE"     : "%s",\n' "$(DEV_STATE)"            >> $(VERSION_JSON).tmp
	@printf '    "BUILD_NO"      : "%s",\n' "$(BUILD_NO)"             >> $(VERSION_JSON).tmp
	@printf '    "BUILD_DATE"    : "%s",\n' "$(BUILD_DATE)"           >> $(VERSION_JSON).tmp
	@printf '    "BRANCH"        : "%s",\n' "$(CURRENT_BRANCH)"       >> $(VERSION_JSON).tmp
	@printf '    "SHA"           : "%s",\n' "$(CURRENT_SHA)"          >> $(VERSION_JSON).tmp
	@printf '    "FILE"          : "%s"\n'  "$(VERSIONED_TARGET).bin" >> $(VERSION_JSON).tmp
	@printf '}\n'                                                     >> $(VERSION_JSON).tmp

	@if `cmp -s "$(VERSION_JSON)" "$(VERSION_JSON).tmp"`; \
	then \
		rm -r "$(VERSION_JSON).tmp"; \
	else \
		mv -f "$(VERSION_JSON).tmp" "$(VERSION_JSON)"; \
	fi

.FORCE:
