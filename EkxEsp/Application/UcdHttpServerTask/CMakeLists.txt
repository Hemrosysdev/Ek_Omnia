set(COMPONENT_SRCDIRS Sources )
set(COMPONENT_ADD_INCLUDEDIRS Interfaces )
set(COMPONENT_EMBED_FILES 	"${PROJECT_DIR}/build/index.html.stripped" 
                            "${PROJECT_DIR}/build/statistics.html.stripped" 
							"${PROJECT_DIR}/build/copyright.html.stripped" 
                            "${PROJECT_DIR}/build/includePopup.html.stripped.gz" 
                            "${PROJECT_DIR}/build/includeHeader.html.stripped.gz" 
                            "${PROJECT_DIR}/build/includeFooter.html.stripped.gz" 
                            "${PROJECT_DIR}/build/includeDevOptions.html.stripped.gz" 
                            "${PROJECT_DIR}/build/includeNoJavascript.html.stripped.gz" 

							"${PROJECT_DIR}/build/zepto.min.js.gz"
                            "${PROJECT_DIR}/build/aliveCheck.js.stripped.gz"
                            "${PROJECT_DIR}/build/popupManagement.js.stripped.gz"
                            "${PROJECT_DIR}/build/commonHelpers.js.stripped.gz"
                            "${PROJECT_DIR}/build/firmwareUpdateObserver.js.stripped.gz"

							"${PROJECT_DIR}/build/style.css.stripped.gz"

                            "${PROJECT_DIR}/build/favicon.ico.gz" 
                            "WebSite/images/ekx.jpg" 
                            "WebSite/images/wifi.png" 
                            "WebSite/images/home.png" 
							"WebSite/images/mahlkoenig_logo_210x120.png"
							"WebSite/images/qrcode.png" 
							)

set( COMPONENT_REQUIRES UxEspCppLibrary Main Common esp_wifi esp_http_server lwip esp_event esp_netif)

register_component()

set( MyPython "python" )

if(DEFINED ENV{windir})
set( MyPython "python.exe" )
endif()

# create index.html.stripped from index.html
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/index.html.stripped
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py index.html ${PROJECT_DIR}/build/index.html.stripped
    DEPENDS ${COMPONENT_DIR}/WebSite/index.html
    COMMENT "Generating index.html.stripped"
    VERBATIM )
add_custom_target( index_html_stripped DEPENDS ${PROJECT_DIR}/build/index.html.stripped )
add_dependencies( ${COMPONENT_LIB} index_html_stripped )

# create statistics.html.stripped from statistics.html
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/statistics.html.stripped
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py statistics.html ${PROJECT_DIR}/build/statistics.html.stripped
    DEPENDS ${COMPONENT_DIR}/WebSite/statistics.html
    COMMENT "Generating statistics.html.stripped"
    VERBATIM )
add_custom_target( statistics_html_stripped DEPENDS ${PROJECT_DIR}/build/statistics.html.stripped )
add_dependencies( ${COMPONENT_LIB} statistics_html_stripped )

# create copyright.html.stripped from copyright.html
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/copyright.html.stripped
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py copyright.html ${PROJECT_DIR}/build/copyright.html.stripped
    DEPENDS ${COMPONENT_DIR}/WebSite/copyright.html
    COMMENT "Generating copyright.html.stripped"
    VERBATIM )
add_custom_target( copyright_html_stripped DEPENDS ${PROJECT_DIR}/build/copyright.html.stripped )
add_dependencies( ${COMPONENT_LIB} copyright_html_stripped )

# create includeHeader.html.stripped.gz from includeHeader.html
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/includeHeader.html.stripped.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py includeHeader.html ${PROJECT_DIR}/build/includeHeader.html.stripped &&  ${MyPython} gzip_helper.py ${PROJECT_DIR}/build/includeHeader.html.stripped ${PROJECT_DIR}/build/includeHeader.html.stripped.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/includeHeader.html
    COMMENT "Generating includeHeader.html.stripped.gz"
    VERBATIM )
add_custom_target( include_header_html_stripped_gz DEPENDS ${PROJECT_DIR}/build/includeHeader.html.stripped.gz )
add_dependencies( ${COMPONENT_LIB} include_header_html_stripped_gz )

# create includeFooter.html.stripped.gz from includeFooter.html
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/includeFooter.html.stripped.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py includeFooter.html ${PROJECT_DIR}/build/includeFooter.html.stripped &&  ${MyPython} gzip_helper.py ${PROJECT_DIR}/build/includeFooter.html.stripped ${PROJECT_DIR}/build/includeFooter.html.stripped.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/includeFooter.html
    COMMENT "Generating includeFooter.html.stripped.gz"
    VERBATIM )
add_custom_target( include_footer_html_stripped_gz DEPENDS ${PROJECT_DIR}/build/includeFooter.html.stripped.gz )
add_dependencies( ${COMPONENT_LIB} include_footer_html_stripped_gz )

# create includeDevOptions.html.stripped.gz from includeDevOptions.html
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/includeDevOptions.html.stripped.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py includeDevOptions.html ${PROJECT_DIR}/build/includeDevOptions.html.stripped &&  ${MyPython} gzip_helper.py ${PROJECT_DIR}/build/includeDevOptions.html.stripped ${PROJECT_DIR}/build/includeDevOptions.html.stripped.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/includeDevOptions.html
    COMMENT "Generating includeDevOptions.html.stripped.gz"
    VERBATIM )
add_custom_target( include_dev_options_html_stripped_gz DEPENDS ${PROJECT_DIR}/build/includeDevOptions.html.stripped.gz )
add_dependencies( ${COMPONENT_LIB} include_dev_options_html_stripped_gz )

# create includePopup.html.stripped.gz from includePopup.html
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/includePopup.html.stripped.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py includePopup.html ${PROJECT_DIR}/build/includePopup.html.stripped &&  ${MyPython} gzip_helper.py ${PROJECT_DIR}/build/includePopup.html.stripped ${PROJECT_DIR}/build/includePopup.html.stripped.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/includePopup.html
    COMMENT "Generating includePopup.html.stripped.gz"
    VERBATIM )
add_custom_target( include_popup_html_stripped_gz DEPENDS ${PROJECT_DIR}/build/includePopup.html.stripped.gz )
add_dependencies( ${COMPONENT_LIB} include_popup_html_stripped_gz )

# create includeNoJavascript.html.stripped.gz from includeNoJavascript.html
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/includeNoJavascript.html.stripped.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py includeNoJavascript.html ${PROJECT_DIR}/build/includeNoJavascript.html.stripped &&  ${MyPython} gzip_helper.py ${PROJECT_DIR}/build/includeNoJavascript.html.stripped ${PROJECT_DIR}/build/includeNoJavascript.html.stripped.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/includeNoJavascript.html
    COMMENT "Generating includeNoJavascript.html.stripped.gz"
    VERBATIM )
add_custom_target( include_no_javascript_html_stripped_gz DEPENDS ${PROJECT_DIR}/build/includeNoJavascript.html.stripped.gz )
add_dependencies( ${COMPONENT_LIB} include_no_javascript_html_stripped_gz )

# create style.css.stripped.gz from style.css
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/style.css.stripped.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py css/style.css ${PROJECT_DIR}/build/style.css.stripped &&  ${MyPython} gzip_helper.py ${PROJECT_DIR}/build/style.css.stripped ${PROJECT_DIR}/build/style.css.stripped.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/css/style.css
    COMMENT "Generating style.css.stripped.gz"
    VERBATIM )
add_custom_target( style_css_stripped_gz DEPENDS ${PROJECT_DIR}/build/style.css.stripped.gz )
add_dependencies( ${COMPONENT_LIB} style_css_stripped_gz )

# create commonHelpers.js.stripped.gz from commonHelpers.css
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/commonHelpers.js.stripped.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py js/commonHelpers.js ${PROJECT_DIR}/build/commonHelpers.js.stripped &&  ${MyPython} gzip_helper.py ${PROJECT_DIR}/build/commonHelpers.js.stripped ${PROJECT_DIR}/build/commonHelpers.js.stripped.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/js/commonHelpers.js
    COMMENT "Generating commonHelpers.js.stripped.gz"
    VERBATIM )
add_custom_target( common_helpers_js_stripped_gz DEPENDS ${PROJECT_DIR}/build/commonHelpers.js.stripped.gz )
add_dependencies( ${COMPONENT_LIB} common_helpers_js_stripped_gz )

# create firmwareUpdateObserver.js.stripped.gz from firmwareUpdateObserver.css
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/firmwareUpdateObserver.js.stripped.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py js/firmwareUpdateObserver.js ${PROJECT_DIR}/build/firmwareUpdateObserver.js.stripped &&  ${MyPython} gzip_helper.py ${PROJECT_DIR}/build/firmwareUpdateObserver.js.stripped ${PROJECT_DIR}/build/firmwareUpdateObserver.js.stripped.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/js/firmwareUpdateObserver.js
    COMMENT "Generating firmwareUpdateObserver.js.stripped.gz"
    VERBATIM )
add_custom_target( firmware_update_observer_js_stripped_gz DEPENDS ${PROJECT_DIR}/build/firmwareUpdateObserver.js.stripped.gz )
add_dependencies( ${COMPONENT_LIB} firmware_update_observer_js_stripped_gz )

# create popupManagement.js.stripped.gz from popupManagement.css
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/popupManagement.js.stripped.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py js/popupManagement.js ${PROJECT_DIR}/build/popupManagement.js.stripped &&  ${MyPython} gzip_helper.py ${PROJECT_DIR}/build/popupManagement.js.stripped ${PROJECT_DIR}/build/popupManagement.js.stripped.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/js/popupManagement.js
    COMMENT "Generating popupManagement.js.stripped.gz"
    VERBATIM )
add_custom_target( popup_management_js_stripped_gz DEPENDS ${PROJECT_DIR}/build/popupManagement.js.stripped.gz )
add_dependencies( ${COMPONENT_LIB} popup_management_js_stripped_gz )

# create aliveCheck.js.stripped.gz from aliveCheck.css
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/aliveCheck.js.stripped.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} strip_helper.py js/aliveCheck.js ${PROJECT_DIR}/build/aliveCheck.js.stripped &&  ${MyPython} gzip_helper.py ${PROJECT_DIR}/build/aliveCheck.js.stripped ${PROJECT_DIR}/build/aliveCheck.js.stripped.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/js/aliveCheck.js
    COMMENT "Generating aliveCheck.js.stripped.gz"
    VERBATIM )
add_custom_target( alive_check_js_stripped_gz DEPENDS ${PROJECT_DIR}/build/aliveCheck.js.stripped.gz )
add_dependencies( ${COMPONENT_LIB} alive_check_js_stripped_gz )

# create zepto.min.js.gz from zepto.min.js
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/zepto.min.js.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} gzip_helper.py js/zepto.min.js ${PROJECT_DIR}/build/zepto.min.js.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/js/zepto.min.js
    COMMENT "Generating zepto.min.js.gz"
    VERBATIM )
add_custom_target( zepto_min_js_gz DEPENDS ${PROJECT_DIR}/build/zepto.min.js.gz )
add_dependencies( ${COMPONENT_LIB} zepto_min_js_gz )

# create favicon.ico.gz from favicon.ico
add_custom_command( 
    OUTPUT ${PROJECT_DIR}/build/favicon.ico.gz
    COMMAND cd ${COMPONENT_DIR}/WebSite &&  ${MyPython} gzip_helper.py images/favicon.ico ${PROJECT_DIR}/build/favicon.ico.gz
    DEPENDS ${COMPONENT_DIR}/WebSite/images/favicon.ico
    COMMENT "Generating favicon.ico.gz"
    VERBATIM )
add_custom_target( favicon_ico_gz DEPENDS ${PROJECT_DIR}/build/favicon.ico.gz )
add_dependencies( ${COMPONENT_LIB} favicon_ico_gz )
