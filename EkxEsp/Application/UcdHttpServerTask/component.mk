#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

include ../../application.mk

COMPONENT_EMBED_FILES := ../../build/index.html.stripped
COMPONENT_EMBED_FILES += ../../build/wifi.html.stripped
COMPONENT_EMBED_FILES += ../../build/login.html.stripped
COMPONENT_EMBED_FILES += ../../build/copyright.html.stripped
COMPONENT_EMBED_FILES += ../../build/x54_websocket_test.html.stripped
COMPONENT_EMBED_FILES += ../../build/favicon.ico.gz
COMPONENT_EMBED_FILES += ../../build/zepto.min.js.gz
COMPONENT_EMBED_FILES += ../../build/style.css.stripped.gz

COMPONENT_EMBED_FILES += WebSite/x54.jpg
COMPONENT_EMBED_FILES += WebSite/wifi.png
COMPONENT_EMBED_FILES += WebSite/home.png
COMPONENT_EMBED_FILES += WebSite/qrcode.png
COMPONENT_EMBED_FILES += WebSite/mahlkoenig_logo_210x120.png
