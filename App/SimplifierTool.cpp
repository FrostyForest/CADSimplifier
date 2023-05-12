#ifndef _PreComp_
#include <Standard_Version.hxx>

#include <boost/math/special_functions/fpclassify.hpp>
#include <cmath>
#include <iostream>
#include <BRepAdaptor_Surface.hxx>
#include <BRepLProp_SLProps.hxx>

#endif
#include <Base/BoundBox.h>

#include <Base/Matrix.h>
#include <Base/Stream.h>
#include <Base/Writer.h>


#include "SimplifierTool.h"

//#include <Mod/Part/App/Geometry.h>
#include "Mod/CADSimplifier/App/BRepAlgoAPI_RemoveFillet.h"
#ifdef _MSC_VER
#include <ppl.h>
#endif
//#include<QtWidgets/qmessagebox.h>
#include<QMessageBox>
#include<BRep_Tool.hxx>
#include<ShapeFix_Shape.hxx>

#include<Standard_Type.hxx>

#include <Geom_Circle.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <GeomPlate_Surface.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <TopOpeBRepTool_AncestorsTool.hxx>
using namespace CADSimplifier;
using namespace std;
using namespace Base;

#pragma execution_character_set("utf-8")

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
TopoDS_Shape SimplifierTool::AutoRemoveFillet()
{
    //if (this->_Shape.IsNull())
    //    Standard_Failure::Raise("Base shape is null");
#if OCC_VERSION_HEX < 0x070300
    (void)s;
    throw Base::RuntimeError("Defeaturing is available only in OCC 7.3.0 and up.");
#else
    BRepAlgoAPI_RemoveFillet rf;
    //rf.SetRunParallel(true);
    //rf.SetShape(this->_Shape);
    //for (std::vector<TopoDS_Shape>::const_iterator it = s.begin(); it != s.end(); ++it)
    //    rf.AddFaceToRemove(*it);
    //rf.Build();
    //if (!rf.IsDone()) {
    //    // error treatment
    //    Standard_SStream aSStream;
    //    rf.DumpErrors(aSStream);
    //    const std::string& resultstr = aSStream.str();
    //    const char* cstr2 = resultstr.c_str();
    //    throw Base::RuntimeError(cstr2);
    //}
    //if (rf.HasWarnings()) {
    //    // warnings treatment
    //    Standard_SStream aSStream;
    //    rf.DumpWarnings(aSStream);
    //}
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

 
 
 
 
 bool CADSimplifier::SimplifierTool::isHaveCommonVertice(const TopoDS_Face& face,const TopoDS_Face& face1)
 {
     if (face.IsNull() || face1.IsNull()) return false;
     if (face1.IsEqual(face)) return false;
     TopExp_Explorer Ex(face, TopAbs_EDGE);
     TopExp_Explorer Ex1(face1, TopAbs_EDGE);
     TopoDS_Vertex V1, V2, V3, V4;
     for (Ex.Init(face, TopAbs_EDGE); Ex.More(); Ex.Next()) {
         TopExp::Vertices(TopoDS::Edge(Ex.Current()), V1, V2);
         for (Ex1.Init(face1, TopAbs_EDGE); Ex1.More(); Ex1.Next()) {           
             TopExp::Vertices(TopoDS::Edge(Ex1.Current()), V3, V4);             
             //if (V1.IsEqual(V3) || V2.IsEqual(V4) || V1.IsEqual(V4) || V2.IsEqual(V3)) {//Orientatin方向不同
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
 bool CADSimplifier::SimplifierTool::getSurfaceGemoInfo(const TopoDS_Face& OCCface, double& radius, ...)
 {     
     Handle(Geom_Surface) S = BRep_Tool::Surface(OCCface);
     if (S->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))) {//圆柱面
         Handle(Geom_CylindricalSurface) SS = Handle(Geom_CylindricalSurface)::DownCast(S);
         radius = SS->Radius();         
     }
     else if (S->IsKind(STANDARD_TYPE(Geom_SphericalSurface))) {//球面
         Handle(Geom_SphericalSurface) SS = Handle(Geom_SphericalSurface)::DownCast(S);
         radius = SS->Radius();
     }     
     else if (S->IsKind(STANDARD_TYPE(Geom_ConicalSurface))) {//圆锥面
         Handle(Geom_ConicalSurface) SS = Handle(Geom_ConicalSurface)::DownCast(S);            
         radius = SS->RefRadius();         
     }   
      else if (S->IsKind(STANDARD_TYPE(Geom_ToroidalSurface))) {//环形曲面
         Handle(Geom_ToroidalSurface) SS = Handle(Geom_ToroidalSurface)::DownCast(S);
         radius = SS->MinorRadius();//取最大曲率         
     }
      else if (S->IsKind(STANDARD_TYPE(Geom_BSplineSurface))) {//样条曲面
	     radius =  samplingGetRadiusOfFreeSurface(OCCface, 4);//3*3 9点采样                     
      }
      else if (S->IsKind(STANDARD_TYPE(Geom_BezierSurface))) {//贝塞尔曲面
          radius = samplingGetRadiusOfFreeSurface(OCCface, 1e2 + 1);         
      }
      else if (S->IsKind(STANDARD_TYPE(Geom_OffsetSurface))) {
          radius = samplingGetRadiusOfFreeSurface(OCCface, 1e2 + 1);         
      }
      else if (S->IsKind(STANDARD_TYPE(Geom_SurfaceOfRevolution))) {
          radius = samplingGetRadiusOfFreeSurface(OCCface, 4);        
      }
      else if (S->IsKind(STANDARD_TYPE(Geom_SurfaceOfLinearExtrusion))) {
          radius = samplingGetRadiusOfFreeSurface(OCCface, 1e2 + 1);       
      }
      else if (S->IsKind(STANDARD_TYPE(GeomPlate_Surface))) {
          radius = samplingGetRadiusOfFreeSurface(OCCface, 1e2 + 1);
      }
      else if (S->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface))) {
          radius = samplingGetRadiusOfFreeSurface(OCCface, 1e2 + 1);
      }
     else {
         //radius = samplingGetRadiusOfFreeSurface(OCCface, 1e2+1);
         //if (radius < 0) radius = Abs(radius);
         //return true;
#ifdef FC_DEBUG
         std::string err = "Unhandled surface type ";
         err += S->DynamicType()->Name();
         QMessageBox::about(nullptr,QObject::tr("Error Tip"), QString::fromStdString(err));
         //auto desc = S->get_type_descriptor();         
         //std::ostringstream info;
         //desc->Print(info);  
         //std::string sp = info.str();
         //QMessageBox::about(nullptr, QString::fromLatin1("Error Info"),QString::fromStdString(sp));
#endif  
         return false;
     }
     if (radius < 0)radius = Abs(radius);
     return true;    
 }

 //double CADSimplifier::SimplifierTool::samplingGetRadiusOfFreeSurface(const TopoDS_Face& face,int n)
 //{    
 //    Handle(Geom_Surface) S = BRep_Tool::Surface(face);
 //    Standard_Real U1,U2,V1,V2;
 //    S->Bounds(U1,U2,V1,V2);  
 //    Standard_Real uLength = std::abs(U1 - U2);
 //    Standard_Real vLength = std::abs(V1 - V2);
 //    BRepAdaptor_Surface adapt(face);
 //    std::vector<double> vecRadius;
 //    for (int i = 1; i <= n - 1; ++i) {
 //        double u = U1 + i * uLength / n;
 //        for (int j = 1; j <= n - 1; ++j) {
 //            double v = V1 + j * vLength / n;
 //            BRepLProp_SLProps prop(adapt, u, v, 2,Precision::Confusion());              
 //            vecRadius.emplace_back(1 / prop.MaxCurvature());
 //        }
 //    }
 //    double sum = std::accumulate(vecRadius.begin(), vecRadius.end(), 0.0);
 //    double radius = sum / vecRadius.size();            
 //    return radius; 
 //}
 
 /// <summary>
 ///    
 /// </summary>
 /// <param name="face"></param>
 /// <param name="n">取点数目</param>
 /// <returns></returns>
 double CADSimplifier::SimplifierTool::samplingGetRadiusOfFreeSurface(const TopoDS_Face& face,
                                                                      int n )
 {
     Handle(Geom_Surface) S = BRep_Tool::Surface(face);
     Standard_Real U1, U2, V1, V2;
     S->Bounds(U1, U2, V1, V2);
     Standard_Real uStep = (U2 - U1) / (n + 1);
     Standard_Real vStep = (V2 - V1) / (n + 1);
     BRepAdaptor_Surface adapt(face);
     std::vector<double> vecRadius;
     for (int i = 1; i <= n ; ++i) {
         double u = U1 + i * uStep;
         for (int j = 1; j <= n ; ++j) {
             double v = V1 + j * vStep;
             BRepLProp_SLProps prop(adapt, u, v, 2, Precision::Confusion());

             double dRMax = Abs(prop.MaxCurvature());
             double dRMin = Abs( prop.MinCurvature());
             double dAbsMaxRadius = dRMax > dRMin ? dRMax : dRMin;

             vecRadius.emplace_back(1 / dAbsMaxRadius);
         }
     }
     double sum = std::accumulate(vecRadius.begin(), vecRadius.end(), 0.0);
     double radius = sum / vecRadius.size();
     return radius;
 }


 void CADSimplifier::SimplifierTool::getAllFacesOfASolidOfDocument(TopTools_IndexedMapOfShape& allFace, 
     const QByteArray& featureName,const App::DocumentObject* obj)
 {
     auto doc = obj ? obj->getDocument() : App::GetApplication().getActiveDocument();
     assert(doc);
     App::DocumentObject* docObj = doc->getObject((const char*)featureName);
     if (docObj && docObj->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId())) {
         TopoDS_Shape myShape = static_cast<Part::Feature*>(docObj)->Shape.getValue();
         TopExp::MapShapes(myShape, TopAbs_FACE, allFace);
     }
 }


 void CADSimplifier::SimplifierTool::getSelectedFaces(std::vector<TopoDS_Shape>& selectedFaces)
 {
     ////单选
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
     //    //TopoDS_Shape sh = feat->Shape.getShape().getShape();////TopAbs_COMPSOLID类型
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

     //多选  get current selection and their sub-elements
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
             Handle(Geom_Surface) S = BRep_Tool::Surface(aFace);
             if (S->IsKind(STANDARD_TYPE(Geom_Plane))) continue;              
             auto it = std::find(selectedFaces.begin(), selectedFaces.end(), aFace);
             if (it != selectedFaces.end()) continue;
             if (this->isHaveCommonVertice(aFace, selectedFace)) {  
             //if (Part::checkIntersection(aFace, selectedFace, true, true)) {  //Solid
                 int id = allFace.FindIndex(aFace);
                 auto it = std::find(NeighborFacesIDSet.begin(), NeighborFacesIDSet.end(), id);
                 if (it == NeighborFacesIDSet.end()) {
                     NeighborFacesIDSet.emplace_back(id);
                     destFaces.emplace_back(aFace);                                                                                                                                                   
                }                                       
             }
         }
     }
     return NeighborFacesIDSet;
 }

 char* CADSimplifier::SimplifierTool::checkFaceType(const TopoDS_Face& OCCface, double& radius,char* &str ...)
 {
     Handle(Geom_Surface) S = BRep_Tool::Surface(OCCface);
     if (S->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))) {//圆柱面
         Handle(Geom_CylindricalSurface) SS = Handle(Geom_CylindricalSurface)::DownCast(S);
         radius = SS->Radius();
         str =("圆柱面");
     }
     else if (S->IsKind(STANDARD_TYPE(Geom_SphericalSurface))) {//球面
         Handle(Geom_SphericalSurface) SS = Handle(Geom_SphericalSurface)::DownCast(S);
         radius = SS->Radius();
         str = ("球面") ;
     }
     else if (S->IsKind(STANDARD_TYPE(Geom_ConicalSurface))) {//圆锥面
         Handle(Geom_ConicalSurface) SS = Handle(Geom_ConicalSurface)::DownCast(S);
         radius = SS->RefRadius();
         str = ("圆锥面");
     }
     else if (S->IsKind(STANDARD_TYPE(Geom_ToroidalSurface))) {//环形曲面
         Handle(Geom_ToroidalSurface) SS = Handle(Geom_ToroidalSurface)::DownCast(S);
         radius = SS->MinorRadius();//取最大曲率  
         str = ("环形曲面");
     }
     else if (S->IsKind(STANDARD_TYPE(Geom_BSplineSurface))) {//样条曲面
         radius = samplingGetRadiusOfFreeSurface(OCCface, 4);//3*3 9点采样
         str = ("样条曲面");
     }
     else if (S->IsKind(STANDARD_TYPE(Geom_BezierSurface))) {//贝塞尔曲面
         radius = samplingGetRadiusOfFreeSurface(OCCface, 1e2 + 1);
         str = ("贝塞尔曲面");
     }
     else if (S->IsKind(STANDARD_TYPE(Geom_OffsetSurface))) {
         radius = samplingGetRadiusOfFreeSurface(OCCface, 1e2 + 1);
         str = ("球面");
     }
     else if (S->IsKind(STANDARD_TYPE(Geom_SurfaceOfRevolution))) {
         radius = samplingGetRadiusOfFreeSurface(OCCface, 4);
         str = ("围绕某个轴旋转曲线所形成的曲面");
     }
     else if (S->IsKind(STANDARD_TYPE(Geom_SurfaceOfLinearExtrusion))) {
         radius = samplingGetRadiusOfFreeSurface(OCCface, 1e2 + 1);
         str = ("过沿着某个方向延伸曲线而形成的曲面");
     }
     else if (S->IsKind(STANDARD_TYPE(GeomPlate_Surface))) {
         radius = samplingGetRadiusOfFreeSurface(OCCface, 1e2 + 1);
         str = ("过沿着某个方向延伸曲线而形成的曲面");
     }
     else if (S->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface))) {
         radius = samplingGetRadiusOfFreeSurface(OCCface, 1e2 + 1);
         str = ("由矩形边界限制的二维曲面");
     }
     else if (S->IsKind(STANDARD_TYPE(Geom_Plane))) {
         str = ("平面");
     }
     else {
         //radius = samplingGetRadiusOfFreeSurface(OCCface, 1e2+1);
         //if (radius < 0) radius = Abs(radius);
         //return true;
#ifdef FC_DEBUG
         std::string err = "Unhandled surface type ";
         err += S->DynamicType()->Name();
         QMessageBox::about(nullptr, QObject::tr("Error Tip"), QString::fromStdString(err));
         //auto desc = S->get_type_descriptor();         
         //std::ostringstream info;
         //desc->Print(info);  
         //std::string sp = info.str();
         //QMessageBox::about(nullptr, QString::fromLatin1("Error Info"),QString::fromStdString(sp));
#endif  
         return "error";
     }
 
     if (radius < 0)radius = Abs(radius);
     return "error";
 }

 void CADSimplifier::SimplifierTool::findCommonEdge(const TopoDS_Face& F1, const TopoDS_Face& F2,TopTools_ListOfShape &Edge_list)
 {
     TopExp_Explorer Exp;
     TopExp_Explorer Exp2;
     for (Exp.Init(F1, TopAbs_EDGE); Exp.More(); Exp.Next()) {
         TopoDS_Edge E1 = TopoDS::Edge(Exp.Current());
         for (Exp2.Init(F2, TopAbs_EDGE); Exp2.More(); Exp2.Next()) {
             TopoDS_Edge E2 = TopoDS::Edge(Exp2.Current());
             if (E1.IsSame(E2)) { // check if they are the same edge
                 Edge_list.Append(E1);
                 continue;
             }
             else { // check if they share a common vertex or are geometrically coincident
                 //TopoDS_Vertex V1, V2;
                 //TopExp::Vertices(E1, V1, V2); // get the vertices of E1
                 //gp_Pnt P1 = BRep_Tool::Pnt(V1); // get the point of V1
                 //gp_Pnt P2 = BRep_Tool::Pnt(V2); // get the point of V2
                 //Standard_Real Tol = BRep_Tool::Tolerance(E1); // get the tolerance of E1
                 //BRepExtrema_DistShapeShape DSS(E2, V1); // compute the distance between E2 and P1
                 //if (DSS.Value() <= Tol) { // check if P1 is on E2
                 //    Edge_list.Append(E1);
                 //    continue;
                 //}
                 //DSS.LoadS2(V2); // compute the distance between E2 and P2
                 //if (DSS.Value() <= Tol) { // check if P2 is on E2
                 //    Edge_list.Append(E1);
                 //    continue;
                 //}

             }
         }
         
     }
 }

 void CADSimplifier::SimplifierTool::getEdgesOfSelectedFace() {//获取包含选定面的边并添加到freecad中
     //Base::Type partid = Base::Type::fromName("Part::Feature");
     //std::vector<Gui::SelectionObject> objs = Gui::Selection().getSelectionEx(nullptr, partid);
     //App::Document* doc = App::GetApplication().getActiveDocument();
     //int i = 0;
     //for (std::vector<Gui::SelectionObject>::iterator it = objs.begin(); it != objs.end(); ++it) {
     //       App::DocumentObject* pActiveDoc = it->getObject();
     //       std::vector<std::string> subnames = it->getSubNames();
     //       Part::Feature* feat = static_cast<Part::Feature*>(pActiveDoc);
     //       for (auto name : subnames) {
     //           TopoDS_Shape face = feat->Shape.getShape().getSubShape(name.c_str());
     //           for (TopExp_Explorer explorer(face, TopAbs_EDGE); explorer.More(); explorer.Next()) {
     //               TopoDS_Edge edge = TopoDS::Edge(explorer.Current());
     //               Part::Feature* edgeFeature = new Part::Feature();
     //               //// 将合并后的形状赋值给新对象的Shape属性
     //               edgeFeature->Shape.setValue(edge);
     //               // 将新对象添加到文档中，并命名为"FusedFace"
     //               std::string name = "edge" + std::to_string(i++);
     //               const char* edgename = name.c_str();
     //               doc->addObject(edgeFeature, edgename);

     //           }
     //       }
     //    
     //}
     // 获取选择对象列表
     std::vector<Gui::SelectionObject> sel = Gui::Selection().getSelectionEx();

     // 遍历选择对象列表
      App::Document* doc = App::GetApplication().getActiveDocument();
     for (auto& obj : sel) {
         // 获取对象的类型
         std::string type = obj.getTypeName();
         // 如果对象是Part::Feature类型
         if (type == "Part::Feature") {
             // 获取对象的形状
             Part::Feature* feat = static_cast<Part::Feature*>(obj.getObject());
             TopoDS_Shape shape = feat->Shape.getValue();
             // 获取形状的所有面
             TopTools_IndexedMapOfShape faces;
             TopExp::MapShapes(shape, TopAbs_FACE, faces);
             // 遍历所有面
             for (int i = 1; i <= faces.Extent(); i++) {
                 // 获取当前面
                 TopoDS_Face face = TopoDS::Face(faces.FindKey(i));
                 // 判断当前面是否被选中
                 bool selected = false;
                 for (auto& sub : obj.getSubNames()) {
                     if (sub == std::string("Face") + std::to_string(i)) {
                         selected = true;
                         break;
                     }
                 }
                 // 如果当前面被选中
                 if (selected) {
                     // 获取当前面的外部线框
                     //TopoDS_Wire wire = face.wire();
                     // 获取线框的所有边
                     TopTools_IndexedMapOfShape edges;
                     TopExp::MapShapes(face, TopAbs_EDGE, edges);
                     // 遍历所有边
                     for (int j = 1; j <= edges.Extent(); j++) {
                         // 获取当前边
                         TopoDS_Edge edge = TopoDS::Edge(edges.FindKey(j));
                         Part::Feature* edgeFeature = new Part::Feature();
                        //// 将合并后的形状赋值给新对象的Shape属性
                        edgeFeature->Shape.setValue(edge);
                        // 将新对象添加到文档中，并命名为"FusedFace"
                        std::string name = "edge" + std::to_string(i);
                        const char* edgename = name.c_str();
                        doc->addObject(edgeFeature, edgename);
                         
                     }
                 }
             }
         }
     }

 }

 void CADSimplifier::SimplifierTool::getFacesOfSelectedEdge() {//获取包含选定边的面并添加到freecad中
     // 获取选择对象列表
     std::vector<Gui::SelectionObject> sel = Gui::Selection().getSelectionEx();

     // 遍历选择对象列表
     App::Document* doc = App::GetApplication().getActiveDocument();
     for (auto& obj : sel) {
         // 获取对象的类型
         std::string type = obj.getTypeName();
         for (auto& sub : obj.getSubNames()) {
             Base::Console().Message(sub.c_str());
             Base::Console().Message("\n");
         }
         // 如果对象是Part::Feature类型
         if (1) {
             // 获取对象的形状
             Part::Feature* feat = static_cast<Part::Feature*>(obj.getObject());
             TopoDS_Shape shape = feat->Shape.getValue();
             // 获取形状的所有边
             TopTools_IndexedMapOfShape edges;
             TopExp::MapShapes(shape, TopAbs_EDGE, edges);
             // 遍历所有边
             for (int i = 1; i <= edges.Extent(); i++) {
                 // 获取当前边
                 TopoDS_Edge edge = TopoDS::Edge(edges.FindKey(i));
                 // 判断当前边是否被选中
                 bool selected = false;
                 for (auto& sub : obj.getSubNames()) {
                     std::string name = std::string("Edge") + std::to_string(i);
                     if (sub == name) {
                         selected = true;
                         break;
                     }
                 }
                 
                 // 如果当前边被选中
                 TopTools_IndexedMapOfShape faces;
                 if (selected) {
                     TopExp::MapShapes(shape, TopAbs_FACE, faces);
                     for (int k = 1; k <= faces.Extent(); k++)
                     {
                         int j = 0;
                         TopoDS_Face aFace = TopoDS::Face(faces.FindKey(k));

                         // 获取面上的所有边，并检查是否包含目标边
                         for (TopExp_Explorer anEdgeExp(aFace, TopAbs_EDGE); anEdgeExp.More(); anEdgeExp.Next())
                         {
                             TopoDS_Edge aFaceEdge = TopoDS::Edge(anEdgeExp.Current());
                             if (aFaceEdge.IsSame(edge))
                             {
                                 Part::Feature* edgeFeature = new Part::Feature();
                                 //// 将合并后的形状赋值给新对象的Shape属性
                                 edgeFeature->Shape.setValue(aFace);
                                 // 将新对象添加到文档中，并命名为"FusedFace"
                                 std::string name = "face" + std::to_string(j++);
                                 const char* facename = name.c_str();
                                 doc->addObject(edgeFeature, facename);
                                 break;
                             }
                         }
                     }
                     
                 }
             }
         }
     }

 }





 


 


 
