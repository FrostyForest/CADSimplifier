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
#include<qmessagebox.h>
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
    rf.SetRunParallel(true);//并行执行
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
 
 
 
 bool CADSimplifier::SimplifierTool::isHaveCommonEdge(const TopoDS_Face& face,
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
            /* TopoDS_Edge edge = TopoDS::Edge(Ex.Current());
             TopoDS_Edge edge1 = TopoDS::Edge(Ex1.Current());
             if (edge.IsEqual(edge1)) {
                 return true;
             }
             else {
                 continue;
             }*/


             TopExp::Vertices(TopoDS::Edge(Ex1.Current()), V3, V4);
             if ((V1.IsSame(V3) && V2.IsSame(V4)) || (V1.IsSame(V4) && V2.IsSame(V3))) {
                 return true;
             }
             else {
                 continue;
             }
  
 #ifdef FC_DEBUG            
             TopoDS_Edge edge = TopoDS::Edge(Ex.Current());
             TopoDS_Edge edge1 = TopoDS::Edge(Ex1.Current());
             if (edge.IsEqual(edge1)) {
                 return true;
             }
             else {
                 continue;
             }
#endif

         }
     }
     return false;
 }

 void CADSimplifier::SimplifierTool::getAllFacesInSolid(const QByteArray& name,
                                                        TopTools_IndexedMapOfShape& allFace)
 {
     App::Document* doc = App::GetApplication().getActiveDocument();
     if (!doc)
         return;
     App::DocumentObject* docObj = doc->getObject((const char*)name);
     //std::vector<TopoDS_Shape> selectedFaces;
     if (docObj && docObj->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId())) {
         TopoDS_Shape myShape = static_cast<Part::Feature*>(docObj) ->Shape.getValue();
         //TopExp::MapShapesAndAncestors(myShape, TopAbs_FACE, TopAbs_COMPOUND, allFace);       
        TopExp::MapShapes(myShape, TopAbs_FACE,allFace);       

     }
 }



 void CADSimplifier::SimplifierTool::getSelectedFaces(std::vector<TopoDS_Shape>& selectedFaces) {
     //单选
     std::vector<Gui::SelectionObject> selection = Gui::Selection().getSelectionEx();
     if (selection.size() != 1) {
         QMessageBox::warning(nullptr, QObject::tr("Wrong selection"),
             QObject::tr("Select an edge, face or body. Only one body is allowed."));
         return;
     }
    if (!selection[0].isObjectTypeOf(Part::Feature::getClassTypeId()))
     {
         QMessageBox::warning(nullptr, QObject::tr("Wrong object type"),QObject::tr("Fillet works only on parts"));
         return;
     }
    auto it = selection.begin();
    try {
        App::DocumentObject* pActiveDoc = it->getObject();
        Part::Feature* feat = static_cast<Part::Feature*>(pActiveDoc);
        //TopoDS_Shape sh = feat->Shape.getShape().getShape();////TopAbs_COMPSOLID类型
        std::vector<std::string> subnames = it->getSubNames();
        for (std::vector<std::string>::iterator sub = subnames.begin(); sub != subnames.end();
             ++sub) {
            TopoDS_Shape ref = feat->Shape.getShape().getSubShape(sub->c_str());
            selectedFaces.emplace_back(ref);
        }
    }
    catch (const Base::Exception& e) {
        Base::Console().Warning("%s: %s\n", it->getFeatName(), e.what());
    }
    return;

    //多选
     Base::Type partid = Base::Type::fromName("Part::Feature");
     std::vector<Gui::SelectionObject> objs = Gui::Selection().getSelectionEx(nullptr, partid);
     for (std::vector<Gui::SelectionObject>::iterator it = objs.begin(); it != objs.end(); ++it) {
         try {
             App::DocumentObject* pActiveDoc = it->getObject();
             Part::Feature* feat = static_cast<Part::Feature*>(pActiveDoc);
             TopoDS_Shape sh = feat->Shape.getShape().getShape();
             std::vector<std::string> subnames = it->getSubNames();
             for (std::vector<std::string>::iterator sub = subnames.begin(); sub != subnames.end();++sub) {
                 TopoDS_Shape ref = feat->Shape.getShape().getSubShape(sub->c_str());
                 selectedFaces.emplace_back(ref);
             }
         }
         catch (const Base::Exception& e) {
             Base::Console().Warning("%s: %s\n", it->getFeatName(), e.what());
         }
     }
 }


 std::vector<int> CADSimplifier::SimplifierTool::getAllNeighborFacesId(
     const std::vector<TopoDS_Shape>& selectedFaces, const TopTools_IndexedMapOfShape& allFace)
 {
     std::vector<int> NeighborFacesIndexSet;
     for (auto aSelectedface : selectedFaces) {       
         TopoDS_Face selectedFace = TopoDS::Face(aSelectedface);  
         for (Standard_Integer i = 1; i <= allFace.Extent(); ++i) {
             TopoDS_Face aFace = TopoDS::Face(allFace.FindKey(i));
             if (this->isHaveCommonEdge(aFace, selectedFace)) {
                 int id = allFace.FindIndex(aFace);
                 NeighborFacesIndexSet.emplace_back(id);
             }
         }
     }
     return NeighborFacesIndexSet;
 }

 //void CADSimplifier::SimplifierTool::findAdjacentFaces(TopTools_IndexedMapOfShape& theMFAdjacent,
 //                                                      const Message_ProgressRange& theRange)
 //{
 //    //// Map the faces of the feature to avoid them in the map of adjacent faces
 //    //TopoDS_Iterator aIt(myFeature);
 //    //for (; aIt.More(); aIt.Next())
 //    //    myFeatureFacesMap.Add(aIt.Value());
 //    //Message_ProgressScope aPSOuter(theRange, NULL, 2);
 //    //// Find faces adjacent to the feature using the connection map
 //    //aIt.Initialize(myFeature);
 //    //Message_ProgressScope aPSF(aPSOuter.Next(), "Looking for adjacent faces", 1, Standard_True);
 //    //for (; aIt.More(); aIt.Next(), aPSF.Next()) {
 //    //    if (!aPSF.More()) {
 //    //        return;
 //    //    }
 //    //    const TopoDS_Shape& aF = aIt.Value();
 //    //    TopExp_Explorer anExpE(aF, TopAbs_EDGE);
 //    //    for (; anExpE.More(); anExpE.Next()) {
 //    //        const TopoDS_Shape& aE = anExpE.Current();
 //    //        const TopTools_ListOfShape* pAdjacentFaces = myEFMap->Seek(aE);
 //    //        if (pAdjacentFaces) {
 //    //            TopTools_ListIteratorOfListOfShape itLFA(*pAdjacentFaces);
 //    //            for (; itLFA.More(); itLFA.Next()) {
 //    //                const TopoDS_Shape& anAF = itLFA.Value();
 //    //                if (!myFeatureFacesMap.Contains(anAF))
 //    //                    theMFAdjacent.Add(anAF);
 //    //            }
 //    //        }
 //    //    }      
 //    //}
 //}


 //


 