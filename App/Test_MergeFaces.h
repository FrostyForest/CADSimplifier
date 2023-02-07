#pragma once

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <BRepAlgoAPI_Algo.hxx>

class Test_MergeFaces
{
public:
    TopoDS_Shape Body;
    TopTools_ListOfShape FacesToMerge;
    TopoDS_Shape FaceMerged;

public:
    Standard_EXPORT bool Preform();
};
