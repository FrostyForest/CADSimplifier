include_directories(
	${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
	
	${FREETYPE_INCLUDE_DIRS}	
	${Boost_INCLUDE_DIRS}
    ${OCC_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
    ${XercesC_INCLUDE_DIRS}	
)
link_directories(${OCC_LIBRARY_DIR})

set(CADSimplifier_LIBS
    ${OCC_LIBRARIES}
    ${OCC_DEBUG_LIBRARIES}
    Part
    PartGui
	FreeCADApp
    FreeCADGui
)
if(FREETYPE_FOUND)
    set(CADSimplifier_LIBS
        ${CADSimplifier_LIBS}
        ${FREETYPE_LIBRARIES}
    )
endif(FREETYPE_FOUND)

generate_from_xml(SimplifierToolPy)
SET(Tmp_SRCS 
	SimplifierToolPy.xml
	SimplifierToolPyImp.cpp
)
SOURCE_GROUP("Xmltmp" FILES ${Tmp_SRCS})

SET(CADSimplifier_SRCS	
	${Tmp_SRCS}
    AppCADSimplifier.cpp
    PreCompiled.cpp
    PreCompiled.h
	
	BRepAlgoAPI_RemoveFillet.h
	BRepAlgoAPI_RemoveFillet.cpp
	BOPAlgo_RemoveFillets.h
	BOPAlgo_RemoveFillets.cpp		
    SimplifierTool.cpp
    SimplifierTool.h  	
    Test_MergeFaces.h
    Test_MergeFaces.cpp
    	UtilExtendSurface.h
	UtilExtendSurface.cpp
)

add_library(CADSimplifier SHARED ${CADSimplifier_SRCS})
target_link_libraries(CADSimplifier ${CADSimplifier_LIBS})


SET_BIN_DIR(CADSimplifier CADSimplifier /Mod/CADSimplifier)
SET_PYTHON_PREFIX_SUFFIX(CADSimplifier)

install(TARGETS CADSimplifier DESTINATION ${CMAKE_INSTALL_LIBDIR})
