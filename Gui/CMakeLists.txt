include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Boost_INCLUDE_DIRS}
    ${COIN3D_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS}
    ${XercesC_INCLUDE_DIRS}
    ${OCC_INCLUDE_DIR}
)

set(CADSimplifierGui_LIBS
    CADSimplifier
    Part
    FreeCADGui
)

# if (BUILD_QT5)
    # qt5_add_resources(CADSimplifier_QRC_SRCS Resources/CADSimplifier.qrc)
# else()
    # qt4_add_resources(CADSimplifier_QRC_SRCS Resources/CADSimplifier.qrc)
# endif()
qt_add_resources(CADSimplifier_QRC_SRCS Resources/CADSimplifier.qrc)
SET(CADSimplifierGui_SRCS
    ${CADSimplifier_QRC_SRCS}
    AppCADSimplifierGui.cpp
    Command.cpp
    PreCompiled.cpp
    PreCompiled.h
    Workbench.cpp
    Workbench.h

    DlgGetNeighborFaces.h
    DlgGetNeighborFaces.cpp
)
set(CADSimplifierGui_Scripts
    ../InitGui.py
)

SET(CADSimplifierGuiIcon_SVG
    Resources/icons/CADSimplifierWorkbench.svg
    Resources/icons/CADSimplifier_GetNeighborFaces.svg
)
add_library(CADSimplifierGui SHARED ${CADSimplifierGui_SRCS} ${CADSimplifierGui_Scripts} ${CADSimplifierGuiIcon_SVG})
target_link_libraries(CADSimplifierGui ${CADSimplifierGui_LIBS})

fc_target_copy_resource_flat(CADSimplifierGui
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/Mod/CADSimplifier
    ${CADSimplifierGui_Scripts})

SET_BIN_DIR(CADSimplifierGui CADSimplifierGui /Mod/CADSimplifier)
SET_PYTHON_PREFIX_SUFFIX(CADSimplifierGui)
    
fc_copy_sources(CADSimplifierGui "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/Mod/CADSimplifier" ${CADSimplifierGuiIcon_SVG})

install(TARGETS CADSimplifierGui DESTINATION ${CMAKE_INSTALL_LIBDIR})
