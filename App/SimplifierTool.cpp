#ifndef _PreComp_
#include <Standard_Version.hxx>

#include <boost/math/special_functions/fpclassify.hpp>
#include <cmath>
#include <iostream>
#endif
#include <Base/BoundBox.h>

#include <Base/Matrix.h>
#include <Base/Stream.h>
#include <Base/Writer.h>


#include "SimplifierTool.h"

#include "Mod/CADSimplifier/App/BRepAlgoAPI_RemoveFillet.h"
#ifdef _MSC_VER
#include <ppl.h>
#endif
#include<QtWidgets/qmessagebox.h>
#include<BRep_Tool.hxx>
#include<ShapeFix_Shape.hxx>

#include<Standard_Type.hxx>
#include<Geom_Plane.hxx>
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


using namespace CADSimplifier;
using namespace std;
using namespace Base;

TYPESYSTEM_SOURCE(CADSimplifier::SimplifierTool, Data::ComplexGeoData)


std::vector<const char*> SimplifierTool::getElementTypes() const
{
    std::vector<const char*> temp;
    //temp.push_back("Segment");

    return temp;
}

unsigned long SimplifierTool::countSubElements(const char* /*Type*/) const { return 0; }

Data::Segment* SimplifierTool::getSubElement(const char* /*Type*/, unsigned long /*n*/) const
{
    //unsigned long i = 1;

    //if (strcmp(Type,"Segment") == 0) {
    //    // not implemented
    //    assert(0);
    //    return 0;
    //}

    return nullptr;
}
void SimplifierTool::Save(Base::Writer& writer) const
{
  
}
TopoDS_Shape SimplifierTool::RemoveFillet(const std::vector<TopoDS_Shape>& s) 
{
    //if (this->_Shape.IsNull())
    //    Standard_Failure::Raise("Base shape is null");
#if OCC_VERSION_HEX < 0x070300
    (void)s;
    throw Base::RuntimeError("Defeaturing is available only in OCC 7.3.0 and up.");
#else
    BRepAlgoAPI_RemoveFillet rf;
    rf.SetRunParallel(true);
    rf.SetShape(this->_Shape);
    for (std::vector<TopoDS_Shape>::const_iterator it = s.begin(); it != s.end(); ++it)
        rf.AddFaceToRemove(*it);
    rf.Build();
    if (!rf.IsDone()) {
        // error treatment
        Standard_SStream aSStream;
        rf.DumpErrors(aSStream);
        const std::string& resultstr = aSStream.str();
        const char* cstr2 = resultstr.c_str();
        throw Base::RuntimeError(cstr2);
    }
    if (rf.HasWarnings()) {
        // warnings treatment
        Standard_SStream aSStream;
        rf.DumpWarnings(aSStream);
    }
    return rf.Shape();
#endif
}

bool CADSimplifier::SimplifierTool::fixShape(
    double precision, double mintol, double maxtol /*const std::vector<TopoDS_Shape>& shapeVec*/)
{
   /* QString shapeTypeName = QString::fromStdString("Cube"); 
    QByteArray name = shapeTypeName.toLatin1();*/

    /*TopTools_IndexedMapOfShape allFace;
    getAllFacesInDocument(name, allFace);
    std::vector<TopoDS_Shape> selectedFaces;
    getSelectedFaces(selectedFaces);*/  

    /*ShapeFix_Shape fix;
    fix.Perform();*/


    TopoDS_Shape myShape;
    App::Document* activeDoc = App::GetApplication().getActiveDocument();
    if (!activeDoc)
        return false;
    bool flag = true;
    std::vector<App::DocumentObject*> objs = activeDoc->getObjectsOfType(Part::Feature::getClassTypeId());
    for (auto it = objs.begin(); it != objs.end(); ++it) {
        App::DocumentObject* docObj = *it;
        if (docObj && docObj->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId())) {
            myShape = static_cast<Part::Feature*>(docObj)->Shape.getValue();
            Part::TopoShape tpsh;
            tpsh.setShape(myShape);
            flag = tpsh.fix(precision, mintol, maxtol);
            if (!flag)
                return false;
            
        }

    }
    return flag;
   
}

void SimplifierTool::Restore(Base::XMLReader& reader)
{
 
}

 Base::BoundBox3d SimplifierTool::getBoundBox() const 
{
     Base::BoundBox3d box;
    box.MinX = 0;
    box.MaxX = 0;
    box.MinY = 0;
    box.MaxY = 100;
    box.MinZ = 100;
    box.MaxZ = 100;


     return box;
 }

 
 
 
 
 bool CADSimplifier::SimplifierTool::isHaveCommonVertice(const TopoDS_Face& face,
                                                      const TopoDS_Face& face1)
 {
     if (face1.IsEqual(face))
         return false;
     TopExp_Explorer Ex(face, TopAbs_EDGE);
     TopExp_Explorer Ex1(face1, TopAbs_EDGE);
     TopoDS_Vertex V1, V2, V3, V4;
     for (Ex.Init(face, TopAbs_EDGE); Ex.More(); Ex.Next()) {
         TopExp::Vertices(TopoDS::Edge(Ex.Current()), V1, V2);
         for (Ex1.Init(face1, TopAbs_EDGE); Ex1.More(); Ex1.Next()) {           
             TopExp::Vertices(TopoDS::Edge(Ex1.Current()), V3, V4);
             if (V1.IsSame(V3) || V2.IsSame(V4) || V1.IsSame(V4) || V2.IsSame(V3)) {
                 return true;
             }
             else {
                 continue;
             }
         }
     }
     return false;
 }
 bool CADSimplifier::SimplifierTool::getFaceGemoInfo(const TopoDS_Face& OCCface, double& radius,...)
 {     
     Handle(Geom_Surface) S = BRep_Tool::Surface(OCCface);
     if (S->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))) {//Բ����
         Handle(Geom_CylindricalSurface) SS = Handle(Geom_CylindricalSurface)::DownCast(S);
         radius = SS->Radius();         
     }
     else if (S->IsKind(STANDARD_TYPE(Geom_SphericalSurface))) {//����
         Handle(Geom_SphericalSurface) SS = Handle(Geom_SphericalSurface)::DownCast(S);
         radius = SS->Radius();
     }     
     else if (S->IsKind(STANDARD_TYPE(Geom_ConicalSurface))) {//Բ׶��
         Handle(Geom_ConicalSurface) SS = Handle(Geom_ConicalSurface)::DownCast(S);            
         radius = SS->RefRadius();         
     }   
      else if (S->IsKind(STANDARD_TYPE(Geom_ToroidalSurface))) {//��������
         Handle(Geom_ToroidalSurface) SS = Handle(Geom_ToroidalSurface)::DownCast(S);
          radius = SS->MajorRadius();         
     }
     else {
         auto desc = S->get_type_descriptor();        
         QString text;
         text = QString::fromLatin1(
                 "No provide relevant methond for input face type,the input face type is (%1)")
                 .arg(QString::fromLatin1(desc->get_type_name()));
         QMessageBox::about(nullptr, QString::fromLatin1("Error Tip"),text);
       
         return false;
     }
     return true;    
 }

 void CADSimplifier::SimplifierTool::getAllFacesOfASolidofDocument(const QByteArray& featureName,
                                                        TopTools_IndexedMapOfShape& allFace)
 {
     App::Document* doc = App::GetApplication().getActiveDocument();
     if (!doc)
         return;
     App::DocumentObject* docObj = doc->getObject((const char*)featureName);
     if (docObj && docObj->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId())) {
         TopoDS_Shape myShape = static_cast<Part::Feature*>(docObj)->Shape.getValue();
         TopExp::MapShapes(myShape, TopAbs_FACE, allFace);
     }
 }


 void CADSimplifier::SimplifierTool::getSelectedFaces(std::vector<TopoDS_Shape>& selectedFaces)
 {
     ////��ѡ
     //std::vector<Gui::SelectionObject> selection = Gui::Selection().getSelectionEx();
     //if (selection.size() != 1) {
     //    QMessageBox::warning(
     //        nullptr, QObject::tr("Wrong selection"),
     //        QObject::tr("Select an edge, face or body. Only one body is allowed."));
     //    return;
     //}
     //if (!selection[0].isObjectTypeOf(Part::Feature::getClassTypeId())) {
     //    QMessageBox::warning(nullptr, QObject::tr("Wrong object type"),
     //                         QObject::tr("Fillet works only on parts"));
     //    return;
     //}
     //auto it = selection.begin();
     //try {
     //    App::DocumentObject* pActiveDoc = it->getObject();
     //    Part::Feature* feat = static_cast<Part::Feature*>(pActiveDoc);
     //    //TopoDS_Shape sh = feat->Shape.getShape().getShape();////TopAbs_COMPSOLID����
     //    std::vector<std::string> subnames = it->getSubNames();
     //    for (std::vector<std::string>::iterator sub = subnames.begin(); sub != subnames.end();
     //         ++sub) {
     //        TopoDS_Shape ref = feat->Shape.getShape().getSubShape(sub->c_str());
     //        selectedFaces.emplace_back(ref);
     //    }
     //}
     //catch (const Base::Exception& e) {
     //    Base::Console().Warning("%s: %s\n", it->getFeatName(), e.what());
     //}
     //return;

     //��ѡ  get current selection and their sub-elements
     Base::Type partid = Base::Type::fromName("Part::Feature");
     std::vector<Gui::SelectionObject> selObjs = Gui::Selection().getSelectionEx(nullptr, partid);
     for (auto it = selObjs.begin(); it != selObjs.end(); ++it) {
         try {
             App::DocumentObject* pActiveDoc = it->getObject();
             Part::Feature* feat = static_cast<Part::Feature*>(pActiveDoc);
             std::vector<std::string> subnames = it->getSubNames();
             for (auto sub :subnames) {
                 TopoDS_Shape ref = feat->Shape.getShape().getSubShape(sub.c_str());
                 selectedFaces.emplace_back(ref);
             }
         }
         catch (const Base::Exception& e) {
             Base::Console().Warning("%s: %s\n", it->getFeatName(), e.what());
         }
     } 
 }


 std::vector<int> CADSimplifier::SimplifierTool::getAllNeighborFacesIdOfNoPlane(
     const std::vector<TopoDS_Shape>& selectedFaces, const TopTools_IndexedMapOfShape& allFace,
     std::vector<TopoDS_Shape>& destFaces)
 {
     std::vector<int> NeighborFacesIDSet;
     for (auto aSelectedface : selectedFaces) {
         TopoDS_Face selectedFace = TopoDS::Face(aSelectedface);
         for (Standard_Integer i = 1; i <= allFace.Extent(); ++i) {
             TopoDS_Face aFace = TopoDS::Face(allFace.FindKey(i));
             if (this->isHaveCommonVertice(aFace, selectedFace)) {
                 Handle(Geom_Surface) S = BRep_Tool::Surface(aFace);
                 if (!(S->IsKind(STANDARD_TYPE(Geom_Plane)))) {
                     int id = allFace.FindIndex(aFace);
                     destFaces.emplace_back(aFace);
                     NeighborFacesIDSet.emplace_back(id);
                 }
             }
         }
     }
     return NeighborFacesIDSet;
 }

 


 


 
