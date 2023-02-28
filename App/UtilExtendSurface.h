#pragma once
#include <TopoDS_Face.hxx>

void DetermineExtendDirection(const TopoDS_Face& aF, Standard_Boolean& bExtendU,
                              Standard_Boolean& bExtendV);

void MyExtendFace(const TopoDS_Face& theF, const Standard_Real theExtVal,
                  const Standard_Boolean theExtUMin, const Standard_Boolean theExtUMax,
                  const Standard_Boolean theExtVMin, const Standard_Boolean theExtVMax,
                  TopoDS_Face& theFExtended);
