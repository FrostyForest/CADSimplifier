#include "UtilExtendSurface.h"
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeShape.hxx>
#include <Message_ProgressScope.hxx>
#include <TopoDS_Face.hxx>
#include <BRepLib.hxx>
#include <BRep_Tool.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <BRepAdaptor_Surface.hxx>

#include <Geom_BSplineSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <Geom_Surface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <GeomLib.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_Line.hxx>
#include <Geom_BSplineCurve.hxx>
# include <GeomConvert_CompCurveToBSplineCurve.hxx>


#include <BRepLib_MakeFace.hxx>
#include <BRep_Builder.hxx>

//原有的OPEN CASCADE的ExtendAdjacentFaces存在问题：
//1. 扩展的范围是整个theMFAdjacent的对角线的长度，在剪裁是范围过大。
//2. 对于一些圆柱面扩展之后会出现无法正常剪裁，比如一个圆柱面分为两个，互相扩展后，重叠部分的剪裁会出错。
//
//修改后的剪裁：
//1. 根据不同的曲面类型进行精细化剪裁

// Created by: Eugeny MALTCHIKOV
// Copyright (c) 2018 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

//void ExtendAdjacentFaces(const TopTools_IndexedMapOfShape& theMFAdjacent,
//                         TopTools_IndexedDataMapOfShapeShape& theFaceExtFaceMap,
//                         const Message_ProgressRange& theRange)
//{
//    // Get the extension value for the faces - half of the diagonal of bounding box of the feature
//    Bnd_Box aFeatureBox;
//    BRepBndLib::Add(myFeature, aFeatureBox);

//    const Standard_Real anExtLength = sqrt(aFeatureBox.SquareExtent());

//    const Standard_Integer aNbFA = theMFAdjacent.Extent();
//    Message_ProgressScope aPS(theRange, "Extending adjacent faces", aNbFA);
//    for (Standard_Integer i = 1; i <= aNbFA && aPS.More(); ++i, aPS.Next()) {
//        const TopoDS_Face& aF = TopoDS::Face(theMFAdjacent(i));
//        // Extend the face
//        TopoDS_Face aFExt;
//        BRepLib::ExtendFace(aF, anExtLength, Standard_True, Standard_True, Standard_True,
//                            Standard_True, aFExt);
//        theFaceExtFaceMap.Add(aF, aFExt);
//        myHistory->AddModified(aF, aFExt);
//    }
//}
//void GetExtendPara(const TopoDS_Face& aF, Standard_Boolean& bExtendU, Standard_Boolean& bExtendV,
//                   Standard_Real& anExtLength);
Standard_Boolean ExtendPCurve(const Handle(Geom_Curve) & aPCurve, const Standard_Real anEf,
                              const Standard_Real anEl, const Standard_Real a2Offset,
                              Handle(Geom_Curve) & NewPCurve);

void MyExtendFace(const TopoDS_Face& theF, const Standard_Real theExtVal,
                  const Standard_Boolean theExtUMin, const Standard_Boolean theExtUMax,
                  const Standard_Boolean theExtVMin, const Standard_Boolean theExtVMax,
                  TopoDS_Face& theFExtended)
{
    // Get face bounds
    BRepAdaptor_Surface aBAS(theF);
    Standard_Real aFUMin = aBAS.FirstUParameter(), aFUMax = aBAS.LastUParameter(),
                  aFVMin = aBAS.FirstVParameter(), aFVMax = aBAS.LastVParameter();
    const Standard_Real aTol = BRep_Tool::Tolerance(theF);

    // Surface to build the face
    Handle(Geom_Surface) aS;

    const GeomAbs_SurfaceType aType = aBAS.GetType();
    // treat analytical surfaces first
    if (aType == GeomAbs_Plane || aType == GeomAbs_Sphere || aType == GeomAbs_Cylinder
        || aType == GeomAbs_Torus || aType == GeomAbs_Cone || aType == GeomAbs_SurfaceOfExtrusion) {
        // Get basis transformed basis surface
        Handle(Geom_Surface) aSurf =
            Handle(Geom_Surface)::DownCast(aBAS.Surface().Surface()->Transformed(aBAS.Trsf()));

        // Get bounds of the basis surface
        Standard_Real aSUMin, aSUMax, aSVMin, aSVMax;
        aSurf->Bounds(aSUMin, aSUMax, aSVMin, aSVMax);

        Standard_Boolean isUPeriodic = aBAS.IsUPeriodic();
        Standard_Real anUPeriod = isUPeriodic ? aBAS.UPeriod() : 0.0;
        if (isUPeriodic) {
            // Adjust face bounds to first period
            Standard_Real aDelta = aFUMax - aFUMin;
            aFUMin = Max(aSUMin, aFUMin + anUPeriod * Ceiling((aSUMin - aFUMin) / anUPeriod));
            aFUMax = aFUMin + aDelta;
        }

        Standard_Boolean isVPeriodic = aBAS.IsVPeriodic();
        Standard_Real aVPeriod = isVPeriodic ? aBAS.VPeriod() : 0.0;
        if (isVPeriodic) {
            // Adjust face bounds to first period
            Standard_Real aDelta = aFVMax - aFVMin;
            aFVMin = Max(aSVMin, aFVMin + aVPeriod * Ceiling((aSVMin - aFVMin) / aVPeriod));
            aFVMax = aFVMin + aDelta;
        }

        // Enlarge the face
        Standard_Real anURes = 0., aVRes = 0.;
        if (theExtUMin || theExtUMax)
            anURes = aBAS.UResolution(theExtVal);
        if (theExtVMin || theExtVMax)
            aVRes = aBAS.VResolution(theExtVal);

        if (theExtUMin)
            aFUMin = Max(aSUMin, aFUMin - anURes);
        if (theExtUMax)
            aFUMax = Min(isUPeriodic ? aFUMin + anUPeriod : aSUMax, aFUMax + anURes);
        if (theExtVMin)
            aFVMin = Max(aSVMin, aFVMin - aVRes);
        if (theExtVMax)
            aFVMax = Min(isVPeriodic ? aFVMin + aVPeriod : aSVMax, aFVMax + aVRes);

        // Check if the periodic surface should become closed.
        // In this case, use the basis surface with basis bounds.
        const Standard_Real anEps = Precision::PConfusion();
        if (isUPeriodic && Abs(aFUMax - aFUMin - anUPeriod) < anEps) {
            aFUMin = aSUMin;
            aFUMax = aSUMax;
        }
        if (isVPeriodic && Abs(aFVMax - aFVMin - aVPeriod) < anEps) {
            aFVMin = aSVMin;
            aFVMax = aSVMax;
        }

        aS = aSurf;
    }
    //else if (aType == GeomAbs_SurfaceOfExtrusion ) {
    //    Handle(Geom_Surface) s = BRep_Tool::Surface(theF);
    //    Handle(Geom_SurfaceOfLinearExtrusion) ext =
    //        Handle(Geom_SurfaceOfLinearExtrusion)::DownCast(s);
    //    Handle(Geom_Curve) curve = ext->BasisCurve();
    //    double dUMin = curve->FirstParameter();
    //    double dUMax = curve->LastParameter();
    //    Handle(Geom_Curve) newCurve;
    //    ExtendPCurve(curve, aFUMin, aFUMax, theExtVal, newCurve);

    //    ext->SetBasisCurve(newCurve);
    //    //Handle(Geom_BoundedCurve) bCurve = Handle(Geom_BoundedCurve)::DownCast(curve);
    //    //if (bCurve.IsNull()) {
    //    //    theFExtended = theF;
    //    //    return;
    //    //}
    //    Standard_Real aSUMin, aSUMax, aSVMin, aSVMax;
    //    ext->Bounds(aSUMin, aSUMax, aSVMin, aSVMax);
    //    aFUMin = aSUMin;
    //    aFUMax = aSUMax;
    //    //aFVMin = aSVMin;
    //    //aFVMax = aSVMax;
    //    aS = ext;

    //    }
    else {
        // General case

        Handle(Geom_BoundedSurface) aSB =
            Handle(Geom_BoundedSurface)::DownCast(BRep_Tool::Surface(theF));
        if (aSB.IsNull()) {
            theFExtended = theF;
            return;
        }

        // Get surfaces bounds
        Standard_Real aSUMin, aSUMax, aSVMin, aSVMax;
        aSB->Bounds(aSUMin, aSUMax, aSVMin, aSVMax);

        Standard_Boolean isUClosed = aSB->IsUClosed();
        Standard_Boolean isVClosed = aSB->IsVClosed();

        // Check if the extension in necessary directions is done
        Standard_Boolean isExtUMin = Standard_False, isExtUMax = Standard_False,
                         isExtVMin = Standard_False, isExtVMax = Standard_False;

        // UMin
        if (theExtUMin && !isUClosed && !Precision::IsInfinite(aSUMin)) {
            GeomLib::ExtendSurfByLength(aSB, theExtVal, 1, Standard_True, Standard_False);
            isExtUMin = Standard_True;
        }
        // UMax
        if (theExtUMax && !isUClosed && !Precision::IsInfinite(aSUMax)) {
            GeomLib::ExtendSurfByLength(aSB, theExtVal, 1, Standard_True, Standard_True);
            isExtUMax = Standard_True;
        }
        // VMin
        if (theExtVMin && !isVClosed && !Precision::IsInfinite(aSVMax)) {
            GeomLib::ExtendSurfByLength(aSB, theExtVal, 1, Standard_False, Standard_False);
            isExtVMin = Standard_True;
        }
        // VMax
        if (theExtVMax && !isVClosed && !Precision::IsInfinite(aSVMax)) {
            GeomLib::ExtendSurfByLength(aSB, theExtVal, 1, Standard_False, Standard_True);
            isExtVMax = Standard_True;
        }

        aS = aSB;

        // Get new bounds
        aS->Bounds(aSUMin, aSUMax, aSVMin, aSVMax);
        if (isExtUMin)
            aFUMin = aSUMin;
        if (isExtUMax)
            aFUMax = aSUMax;
        if (isExtVMin)
            aFVMin = aSVMin;
        if (isExtVMax)
            aFVMax = aSVMax;
    }

    BRepLib_MakeFace aMF(aS, aFUMin, aFUMax, aFVMin, aFVMax, aTol);
    theFExtended = *(TopoDS_Face*)&aMF.Shape();
    if (theF.Orientation() == TopAbs_REVERSED)
        theFExtended.Reverse();
}

/// <summary>
/// 确定一个面，需要延伸的方向和延伸量
/// </summary>
/// <param name="aF"></param>
/// <param name="bExtendU"></param>
/// <param name="bExtendV"></param>
/// <param name="anExtLength"></param>
void DetermineExtendDirection(const TopoDS_Face& aF, Standard_Boolean& bExtendU,
                              Standard_Boolean& bExtendV)
{
    bExtendU = Standard_True;
    bExtendV = Standard_True;

    Handle(Geom_Surface) S = BRep_Tool::Surface(aF);
    if (S->IsKind(STANDARD_TYPE(Geom_ConicalSurface))) {
        bExtendU = Standard_False;
    }
    else if (S->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))) {
        Handle(Geom_CylindricalSurface) SS = Handle(Geom_CylindricalSurface)::DownCast(S);
        double dRadiu = SS->Radius();
        //if (dRadiu < 10)
            bExtendU = Standard_False;
    }
    else if (S->IsKind(STANDARD_TYPE(Geom_BSplineSurface))) {
        bExtendU = Standard_False;
    }
    /*   if ()) {
        Handle(Geom_CylindricalSurface) SS = Handle(Geom_CylindricalSurface)::DownCast(S);
    }
    else if (S->IsKind(STANDARD_TYPE(Geom_SphericalSurface))) {
        Handle(Geom_SphericalSurface) SS = Handle(Geom_SphericalSurface)::DownCast(S);
    }
    else if (S->IsKind(STANDARD_TYPE(Geom_ToroidalSurface))) {
        Handle(Geom_ToroidalSurface) SS = Handle(Geom_ToroidalSurface)::DownCast(S);
    }
    else if (S->IsKind(STANDARD_TYPE(Geom_BSplineSurface))) {
        Handle(Geom_BSplineSurface) SS = Handle(Geom_BSplineSurface)::DownCast(S);
        Standard_Real U1, U2, V1, V2;
        SS->Bounds(U1, U2, V1, V2);
        Standard_Real u = (U1 + U2) / 2;
        Standard_Real v = (V1 + V2) / 2;
        SS->cu
    }
    else if (S->IsKind(STANDARD_TYPE(Geom_ConicalSurface))) {
        Handle(Geom_ConicalSurface) SS = Handle(Geom_ConicalSurface)::DownCast(S);
        gp_Pnt ptApex = SS->Apex();
        double dRadiu = SS->RefRadius();
        Standard_Real U1, U2, V1, V2;
        SS->Bounds(U1, U2, V1, V2);

        Standard_Real A1, A2, A3, B1, B2, B3, C1, C2, C3, D;
        SS->Coefficients(A1, A2, A3, B1, B2, B3, C1, C2, C3, D);
    }
    else if (S->IsKind(STANDARD_TYPE(Geom_OffsetSurface))) {
        Handle(Geom_OffsetSurface) SS = Handle(Geom_OffsetSurface)::DownCast(S);
    }
    else if (S->IsKind(STANDARD_TYPE(Geom_Plane))) {
        Handle(Geom_Plane) SS = Handle(Geom_Plane)::DownCast(S);
    }
    else if (S->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface))) {
        Handle(Geom_RectangularTrimmedSurface) SS =
            Handle(Geom_RectangularTrimmedSurface)::DownCast(S);
    }
    else {
        auto desc = S->get_type_descriptor();
    }*/
}
 
 Standard_Boolean ExtendPCurve(const Handle(Geom_Curve) & aPCurve, const Standard_Real anEf,
                                     const Standard_Real anEl, const Standard_Real a2Offset,
                              Handle(Geom_Curve) & NewPCurve)
{
    NewPCurve = aPCurve;
    //if (NewPCurve->IsInstance(STANDARD_TYPE(Geom_TrimmedCurve)))
    //    NewPCurve = Handle(Geom_TrimmedCurve)::DownCast(NewPCurve)->BasisCurve();

    Standard_Real FirstPar = NewPCurve->FirstParameter();
    Standard_Real LastPar = NewPCurve->LastParameter();

    if (NewPCurve->IsKind(STANDARD_TYPE(Geom_BoundedCurve))
        && (FirstPar > anEf - a2Offset || LastPar < anEl + a2Offset)) {
        if (NewPCurve->IsInstance(STANDARD_TYPE(Geom_BezierCurve))) {
            Handle(Geom_BezierCurve) aBezier = Handle(Geom_BezierCurve)::DownCast(NewPCurve);
            if (aBezier->NbPoles() == 2) {
                TColgp_Array1OfPnt thePoles(1, 2);
                aBezier->Poles(thePoles);
                gp_Vec aVec(thePoles(1), thePoles(2));
                NewPCurve = new Geom_Line(thePoles(1), aVec);
                return Standard_True;
            }
        }
        else if (NewPCurve->IsInstance(STANDARD_TYPE(Geom_BSplineCurve))) {
            Handle(Geom_BSplineCurve) aBSpline = Handle(Geom_BSplineCurve)::DownCast(NewPCurve);
            if (aBSpline->NbKnots() == 2 && aBSpline->NbPoles() == 2) {
                TColgp_Array1OfPnt thePoles(1, 2);
                aBSpline->Poles(thePoles);
                gp_Vec aVec(thePoles(1), thePoles(2));
                NewPCurve = new Geom_Line(thePoles(1), aVec);
                return Standard_True;
            }
        }
    }

    FirstPar = aPCurve->FirstParameter();
    LastPar = aPCurve->LastParameter();
    Handle(Geom_TrimmedCurve) aTrCurve = new Geom_TrimmedCurve(aPCurve, FirstPar, LastPar);

    // The curve is not prolonged on begin or end.
    // Trying to prolong it adding a segment to its bound.
    gp_Pnt aPBnd;
    gp_Vec aVBnd;
    gp_Pnt aPBeg;
    gp_Dir aDBnd;
    Handle(Geom_Line) aLin;
    Handle(Geom_TrimmedCurve) aSegment;
    GeomConvert_CompCurveToBSplineCurve aCompCurve(aTrCurve, Convert_RationalC1);
    Standard_Real aTol = Precision::Confusion();
    Standard_Real aDelta = Max(a2Offset, 1.);

    if (FirstPar > anEf - a2Offset) {
        aPCurve->D1(FirstPar, aPBnd, aVBnd);
        aDBnd.SetXYZ(aVBnd.XYZ());
        aPBeg = aPBnd.Translated(gp_Vec(-aDelta * aDBnd.XYZ()));
        aLin = new Geom_Line(aPBeg, aDBnd);
        aSegment = new Geom_TrimmedCurve(aLin, 0, aDelta);

        if (!aCompCurve.Add(aSegment, aTol))
            return Standard_False;
    }

    if (LastPar < anEl + a2Offset) {
        aPCurve->D1(LastPar, aPBeg, aVBnd);
        aDBnd.SetXYZ(aVBnd.XYZ());
        aLin = new Geom_Line(aPBeg, aDBnd);
        aSegment = new Geom_TrimmedCurve(aLin, 0, aDelta);

        if (!aCompCurve.Add(aSegment, aTol))
            return Standard_False;
    }

    NewPCurve = aCompCurve.BSplineCurve();
    return Standard_True;
}
