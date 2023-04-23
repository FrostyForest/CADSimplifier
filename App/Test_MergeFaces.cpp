#include "Test_MergeFaces.h"
#include <BOPAlgo_Builder.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <TopAbs.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_Array1OfShape.hxx>

#include <Geom_Circle.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Geom_ConicalSurface.hxx>


#include <GeomLProp_CLProps.hxx>
#include <GeomLProp_SLProps.hxx>

#include <GProp_GProps.hxx>
#include <GProp_PrincipalProps.hxx>

#include <TColStd_Array1OfReal.hxx>

#include <gp_Pln.hxx>
#include <gp_Lin.hxx>

#include <ShapeAnalysis.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_ShapeTolerance.hxx>

#include <ProjLib.hxx>
#include <ElSLib.hxx>

#include <vector>
#include <sstream>
#include <algorithm>

#include <Standard_Failure.hxx>
#include <Standard_NullObject.hxx>
#include <Standard_ErrorHandler.hxx> // CAREFUL ! position of this file is critic : see Lucien PIGNOLONI / OCC
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <Mod/Part/App/TopoShape.h>
#include <Mod/PartDesign/App/Feature.h>

#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <BRepAdaptor_Surface.hxx>
#include <GeomLib.hxx>
#include <BRepLib_MakeFace.hxx>
#include <BRep_Builder.hxx>

#include "UtilExtendSurface.h"
#include <Mod/CADSimplifier/App/SimplifierTool.h>

//bool Test_MergeFaces::Preform()
//{
//
//
//
//
//
//    BOPAlgo_Builder aBuilder;
//
//
//    // Setting arguments
//    aBuilder.SetArguments(FacesToMerge);
//
//    // Setting options for GF
//
//    // Set parallel processing mode (default is false)
//    Standard_Boolean bRunParallel = Standard_True;
//    aBuilder.SetRunParallel(bRunParallel);
//
//    // Set Fuzzy value (default is Precision::Confusion())
//    Standard_Real aFuzzyValue = 1.e-1;
//    aBuilder.SetFuzzyValue(aFuzzyValue);
//
//    // Set safe processing mode (default is false)
//    Standard_Boolean bSafeMode = Standard_True;
//    aBuilder.SetNonDestructive(bSafeMode);
//
//    // Set Gluing mode for coinciding arguments (default is off)
//    BOPAlgo_GlueEnum aGlue = BOPAlgo_GlueFull;
//    aBuilder.SetGlue(aGlue);
//
//    // Disabling/Enabling the check for inverted solids (default is true)
//    //Standard Boolean bCheckInverted = Standard_False;
//    //aBuilder.SetCheckInverted(bCheckInverted);
//
//    // Set OBB usage (default is false)
//    //Standard_Boolean bUseOBB = Standard_True;
//    //aBuilder.SetUseOBB(buseobb);
//
//    // Perform the operation
//    aBuilder.Perform();
//
//    // Check for the errors
//    if (aBuilder.HasErrors()) {
//        return false;
//    }
//
//    //// Check for the warnings
//    //if (aBuilder.HasWarnings()) {
//    //    // treatment of the warnings
//    //    ...
//    //}
//
//    // result of the operation
//    FaceMerged = aBuilder.Shape();
//    return true;
//}

//≤‚ ‘Fix
//bool Test_MergeFaces::Preform()
//{  
//    Part::TopoShape tpsh;
//    tpsh.setShape(Body);
////    bool flag = tpsh.fix(1E-6, 1E-10, 1E-5);
//    bool flag = tpsh.fix(1E-6, 1E-8, 1E-1);
//
//    Part::Feature* pInsectFaces =
//        (Part::Feature*)App::GetApplication().getActiveDocument()->addObject("Part::Feature",
//                                                                             "FixShape");
//    pInsectFaces->Shape.setValue(tpsh.getShape());
//    pInsectFaces->Visibility.setValue(true);
//
//    //TopoDS_Face OCCface;
//    //TopoDS_Shape sh = FacesToMerge.First();
//    //TopTools_ListIteratorOfListOfShape itF(FacesToMerge);
//    //for (; itF.More(); itF.Next()) {
//    //    const TopoDS_Shape& sh = itF.Value();
//    //    TopoDS_Face OCCface = TopoDS::Face(sh);
//
//    //    Handle(Geom_Surface) S = BRep_Tool::Surface(OCCface);
//
//    //    if (S->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))) {
//    //        Handle(Geom_CylindricalSurface) SS = Handle(Geom_CylindricalSurface)::DownCast(S);
//    //    }
//    //    else if (S->IsKind(STANDARD_TYPE(Geom_SphericalSurface))) {
//    //        Handle(Geom_SphericalSurface) SS = Handle(Geom_SphericalSurface)::DownCast(S);
//    //    }
//    //    else if (S->IsKind(STANDARD_TYPE(Geom_ToroidalSurface))) {
//    //        Handle(Geom_ToroidalSurface) SS = Handle(Geom_ToroidalSurface)::DownCast(S);
//    //    }
//    //    else if (S->IsKind(STANDARD_TYPE(Geom_BSplineSurface))) {
//    //        Handle(Geom_BSplineSurface) SS = Handle(Geom_BSplineSurface)::DownCast(S);
//    //        Standard_Real U1, U2, V1, V2;
//    //        SS->Bounds(U1, U2, V1, V2);
//
//    //    }
//    //    else if (S->IsKind(STANDARD_TYPE(Geom_ConicalSurface))) {
//    //        Handle(Geom_ConicalSurface) SS = Handle(Geom_ConicalSurface)::DownCast(S);
//    //        gp_Pnt ptApex = SS->Apex();
//    //        double dRadiu = SS->RefRadius();
//    //        Standard_Real U1, U2, V1, V2;
//    //        SS->Bounds(U1, U2, V1, V2);
//
//    //        Standard_Real A1, A2, A3, B1, B2, B3, C1, C2, C3,D;
//    //        SS->Coefficients(A1, A2, A3, B1, B2, B3, C1, C2, C3, D);
//    //    }
//    //    else if (S->IsKind(STANDARD_TYPE(Geom_OffsetSurface))) {
//    //        Handle(Geom_OffsetSurface) SS = Handle(Geom_OffsetSurface)::DownCast(S);
//    //    }
//    //    else if (S->IsKind(STANDARD_TYPE(Geom_Plane))) {
//    //        Handle(Geom_Plane) SS = Handle(Geom_Plane)::DownCast(S);
//    //    }
//    //    else if (S->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface))) {
//    //        Handle(Geom_RectangularTrimmedSurface) SS =
//    //            Handle(Geom_RectangularTrimmedSurface)::DownCast(S);
//    //    }
//    //    else {
//    //        auto desc = S->get_type_descriptor();
//    //    }
//    //}
//
//
//
// /*   if (S->IsKind(STANDARD_TYPE(Geom_SphericalSurface))
//        || S->IsKind(STANDARD_TYPE(Geom_ToroidalSurface)) || S->IsUPeriodic()) {
//        const Standard_Boolean isShell =
//            (sh.ShapeType() == TopAbs_SHELL || sh.ShapeType() == TopAbs_FACE);
//
//        if (!isShell && S->IsKind(STANDARD_TYPE(Geom_SphericalSurface))) {
//            Handle(Geom_SphericalSurface) SS = Handle(Geom_SphericalSurface)::DownCast(S);
//            gp_Pnt PC = SS->Location();
//            return Standard_True;
//        }
//    }*/
//
//    return false;
//}

//bool Test_MergeFaces::Preform() 
//{ 
//    //https://dev.opencascade.org/doc/overview/html/occt_user_guides__shape_healing.html
//	ShapeUpgrade_UnifySameDomain aTool(Body, true, true, true);
//    aTool.Build();
//    // get the result
//    FaceMerged = aTool.Shape();
//    // Let theSh1 as a part of theSh
//    // get the new (probably unified) shape form the theSh1
//    //TopoDS_Shape aResSh1 = aTool.Generated(theSh1);
//    
//    return true;
//}

////≤‚ ‘√Êµƒ¿©’π
//bool Test_MergeFaces::Preform()
//{
//    BRep_Builder builder;
//    TopoDS_Compound ExtendedFaceShape; 
//    builder.MakeCompound(ExtendedFaceShape);
//
//    TopTools_ListIteratorOfListOfShape itF(FacesToMerge);
//    for (; itF.More(); itF.Next()) {
//        TopoDS_Face aFace = TopoDS::Face(itF.Value());
//        TopoDS_Face theFExtended;
//        MyExtendFace(aFace, 1.0, Standard_True, Standard_True, Standard_True, Standard_True,
//                     theFExtended);
//
//        BRep_Builder().Add(ExtendedFaceShape, theFExtended);
//    }
//    Part::Feature* pInsectFaces =
//        (Part::Feature*)App::GetApplication().getActiveDocument()->addObject("Part::Feature",
//                                                                             "FixShape");
//    pInsectFaces->Shape.setValue(ExtendedFaceShape);
//    pInsectFaces->Visibility.setValue(true);
//
//
//    return false;
//}
//≤‚ ‘‘≤Ω«∞Îæ∂
bool Test_MergeFaces::Preform()
{
    CADSimplifier::SimplifierTool tools;

    TopTools_ListIteratorOfListOfShape itF(FacesToMerge);
    for (; itF.More(); itF.Next()) {
        TopoDS_Face aFace = TopoDS::Face(itF.Value());
        TopoDS_Face theFExtended;

        tools.samplingGetRadiusOfFreeSurface(aFace, 3);

        MyExtendFace(aFace, 1.0, Standard_True, Standard_True, Standard_True, Standard_True,
                     theFExtended);

    }


    return false;
}
