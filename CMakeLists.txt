
add_subdirectory(App)
if(BUILD_GUI)
    add_subdirectory(Gui)
endif(BUILD_GUI)

set(CADSimplifier_Scripts
    Init.py
)

if(BUILD_GUI)
    list (APPEND CADSimplifier_Scripts InitGui.py)
endif(BUILD_GUI)

add_custom_target(CADSimplifierScripts ALL
    SOURCES ${CADSimplifier_Scripts}
)

fc_target_copy_resource_flat(CADSimplifierScripts
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/Mod/CADSimplifier
    ${CADSimplifier_Scripts}
)

install(
    FILES
        ${CADSimplifier_Scripts}
    DESTINATION
        Mod/CADSimplifier
)
