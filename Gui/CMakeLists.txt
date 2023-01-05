
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Boost_INCLUDE_DIRS}
    ${COIN3D_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS}
    ${XercesC_INCLUDE_DIRS}
)

set(CADSimplifierGui_LIBS
    CADSimplifier
    FreeCADGui
)

# if (BUILD_QT5)
    # qt5_add_resources(CADSimplifier_QRC_SRCS Resources/CADSimplifier.qrc)
# else()
    # qt4_add_resources(CADSimplifier_QRC_SRCS Resources/CADSimplifier.qrc)
# endif()

 qt5_add_resources(CADSimplifier_QRC_SRCS Resources/CADSimplifier.qrc)
SET(CADSimplifierGui_SRCS
    ${CADSimplifier_QRC_SRCS}
    AppCADSimplifierGui.cpp
    Command.cpp
    PreCompiled.cpp
    PreCompiled.h
    Workbench.cpp
    Workbench.h
)

add_library(CADSimplifierGui SHARED ${CADSimplifierGui_SRCS})
target_link_libraries(CADSimplifierGui ${CADSimplifierGui_LIBS})

SET_BIN_DIR(CADSimplifierGui CADSimplifierGui /Mod/CADSimplifier)
SET_PYTHON_PREFIX_SUFFIX(CADSimplifierGui)
    
install(TARGETS CADSimplifierGui DESTINATION ${CMAKE_INSTALL_LIBDIR})
