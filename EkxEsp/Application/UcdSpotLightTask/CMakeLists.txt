set(COMPONENT_SRCDIRS Sources ../UcdPwmOutTask/Sources )
set(COMPONENT_ADD_INCLUDEDIRS Interfaces )

set( COMPONENT_REQUIRES UxEspCppLibrary Main Common UcdPwmOutTask )

register_component()
