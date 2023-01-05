
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${OCC_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
    ${XercesC_INCLUDE_DIRS}
)

set(CADSimplifier_LIBS
    FreeCADApp
)

SET(CADSimplifier_SRCS
    AppCADSimplifier.cpp
    PreCompiled.cpp
    PreCompiled.h
)

add_library(CADSimplifier SHARED ${CADSimplifier_SRCS})
target_link_libraries(CADSimplifier ${CADSimplifier_LIBS})


SET_BIN_DIR(CADSimplifier CADSimplifier /Mod/CADSimplifier)
SET_PYTHON_PREFIX_SUFFIX(CADSimplifier)

install(TARGETS CADSimplifier DESTINATION ${CMAKE_INSTALL_LIBDIR})
