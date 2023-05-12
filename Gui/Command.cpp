/***************************************************************************
 *   Copyright (c) YEAR YOUR NAME <Your e-mail address>                    *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"
#ifndef _PreComp_
#endif

# include <QApplication>
# include <qfileinfo.h>
# include <qinputdialog.h>
# include <qmessagebox.h>
# include <QPointer>
# include <qstringlist.h>

#include <Base/Console.h>
#include <App/Document.h>
#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Document.h>

#include <App/DocumentObject.h>
#include <App/GeoFeature.h>
#include <Base/Exception.h>
#include <Gui/MainWindow.h>
#include <Gui/Selection.h>
#include <Gui/SelectionObject.h>
#include <Gui/WaitCursor.h>
#include <Mod/Part/App/PartFeature.h>
#include <Mod/Part/App/TopoShape.h>

#include <Gui/Control.h>
#include <Mod/CADSimplifier/App/SimplifierTool.h>
#include <Mod/CADSimplifier/App/Test_MergeFaces.h>
#include <TopExp.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepFill_Filling.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include"DlgGetNeighborFaces.h"

#include <locale>
#include <codecvt>


#pragma execution_character_set("utf-8")
using namespace CADSimplifier;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//===========================================================================
// CmdCADSimplifierTest THIS IS JUST A TEST COMMAND
//===========================================================================
DEF_STD_CMD(CmdCADSimplifierTest)

CmdCADSimplifierTest::CmdCADSimplifierTest()
  :Command("CADSimplifier_Test")
{
    sAppModule    = "CADSimplifier";
    sGroup        = QT_TR_NOOP("CADSimplifier");
    sMenuText     = QT_TR_NOOP("Hello");
    sToolTipText  = QT_TR_NOOP("CADSimplifier Test function");
    sWhatsThis    = "CADSimplifier_Test";
    sStatusTip    = QT_TR_NOOP("CADSimplifier Test function");
    sPixmap       = "CADSimplifierWorkbench";
    sAccel        = "CTRL+H";
}

void CmdCADSimplifierTest::activated(int iMsg)
{
    SimplifierTool tool;
    Q_UNUSED(iMsg);
    Gui::WaitCursor wc;

    Base::Type partid = Base::Type::fromName("Part::Feature");
    std::vector<Gui::SelectionObject> objs = Gui::Selection().getSelectionEx(nullptr, partid);
    Base::Console().Message("选定obj数量：%d\n", objs.size());
    openCommand(QT_TRANSLATE_NOOP("Command", "Test"));


    std::vector<TopoDS_Shape> selected_faces;
    TopTools_ListOfShape twoFace_edges_list;//两个面的所有edge的list
    for (std::vector<Gui::SelectionObject>::iterator it = objs.begin(); it != objs.end(); ++it) {
        try {
            App::DocumentObject* pActiveDoc = it->getObject();
            Part::Feature* feat = static_cast<Part::Feature*>(pActiveDoc);

            TopoDS_Shape sh = feat->Shape.getShape().getShape();

            std::vector<TopoDS_Shape> Faces;
            std::vector<std::string> subnames = it->getSubNames();
            Base::Console().Message("选定的SelectionObject的subnames 大小：%d\n",subnames.size() );
            int i = 0;
            for (auto name : subnames) {
                Base::Console().Message("%s\n", name);
                TopoDS_Shape face= feat->Shape.getShape().getSubShape(name.c_str());
                bool isFace = (face.ShapeType() == TopAbs_FACE);
                char* type;
                double radius = 0;
                if (isFace == 1) {
                    for (TopExp_Explorer explorer(face, TopAbs_EDGE); explorer.More(); explorer.Next()) {
                        TopoDS_Edge edge = TopoDS::Edge(explorer.Current());
                        twoFace_edges_list.Append(edge);
                    }
                    selected_faces.emplace_back(face);
                    TopoDS_Face myFace = TopoDS::Face(face);
                    tool.checkFaceType(myFace, radius,type);
                }
                Base::Console().Message("面%d是否为面 ：%d\n", i++,isFace);
                Base::Console().Message("面为%s,半径为：%.2f\n",type,radius);

            }
            //for (std::vector<std::string>::iterator sub = subnames.begin(); sub != subnames.end();
            //     ++sub) {
            //    TopoDS_Shape ref = feat->Shape.getShape().getSubShape(sub->c_str());
            //    tool.FacesToMerge.Append(ref);
            //}
            //if (tool.Preform()) {
            //    TopoDS_Shape nsh = tool.FaceMerged;
            //    auto uiDoc = Gui::Application::Instance->activeDocument();
            //    Part::Feature* pNewFeat = (Part::Feature*)uiDoc->getDocument()->addObject(
            //        "Part::Feature", "MergedFace");
            //    pNewFeat->Shape.setValue(nsh);
            //}
        }
        catch (const Base::Exception& e) {
            Base::Console().Warning("%s: %s\n", it->getFeatName(), e.what());
        }
    }
    
    if (selected_faces.size() == 2) {
        TopoDS_Face topoFace1 = TopoDS::Face(selected_faces[0]);
        TopoDS_Face topoFace2 = TopoDS::Face(selected_faces[1]);
        for (auto topoface : selected_faces) {
            TopExp_Explorer explorer(topoface, TopAbs_EDGE);
            std::vector<TopoDS_Edge> edge_list;
            while (explorer.More()) {
                TopoDS_Edge edge = TopoDS::Edge(explorer.Current());
                // 处理边
                explorer.Next();
                edge_list.emplace_back(edge);
                
            }
            Base::Console().Message("%s %d\n", "有多少边：", edge_list.size());
        }

        //找到公共边
        TopTools_ListOfShape common_edges;
        tool.findCommonEdge(topoFace1, topoFace2, common_edges);
        Base::Console().Message("%s %d\n", "有多少公共边：", common_edges.Extent());

        int i=0;
        App::Document* doc = App::GetApplication().getActiveDocument();
        for (TopTools_ListIteratorOfListOfShape it(common_edges); it.More(); it.Next())
        {
            const TopoDS_Shape& common_shape = it.Value();//先获取shape
            TopoDS_Edge common_edge = TopoDS::Edge(common_shape);//再转换为edge
            Part::Feature* fusedFeature = new Part::Feature();
            //// 将合并后的形状赋值给新对象的Shape属性
            fusedFeature->Shape.setValue(common_edge);
            // 将新对象添加到文档中，并命名为"FusedFace"
            std::string name = "common_edge" + std::to_string(i++);
            const char* edgename = name.c_str();
            doc->addObject(fusedFeature, edgename);
        }

        BRepAlgoAPI_Fuse fuser(topoFace1, topoFace2);
        // Create a new face using BRepBuilderAPI_MakeFace
        if (fuser.IsDone()) {
            Base::Console().Message("%s", "BRepAlgoAPI_Fuse合并成功\n");
            // 如果合并成功，获取合并后的形状
            TopoDS_Shape fusedShape = fuser.Shape();
            ShapeUpgrade_UnifySameDomain unify(fusedShape, Standard_True, Standard_True, Standard_True);//合并面
            unify.Build();
            fusedShape = unify.Shape();

            //从两个面的所有edge的list中提取edge，再删除共用的edge，再从edge构造wire，再makeface
            TopTools_ListOfShape Edges; // 非公共边的一个list
            
            int i = 0;
            for (TopTools_ListIteratorOfListOfShape it0(twoFace_edges_list); it0.More(); it0.Next()) { // loop over the wires of F
                const TopoDS_Shape& edge_shape = it0.Value();
                TopoDS_Edge E = TopoDS::Edge(edge_shape);
                bool isCommonEdge = 0;
                for (TopTools_ListIteratorOfListOfShape it(common_edges); it.More(); it.Next())
                {
                    const TopoDS_Shape& common_shape = it.Value();
                    TopoDS_Edge common_edge = TopoDS::Edge(common_shape);
                    if (E.IsSame(common_edge)) {
                        isCommonEdge = 1;//判断是否为公共边，若是，则1
                        break;
                    }
                    
                }
                if (isCommonEdge == 0) {
                    Edges.Append(E);
                    Part::Feature* fusedFeature = new Part::Feature();
                    fusedFeature->Shape.setValue(E);
                    std::string name = "mergeFace_edge" + std::to_string(i++);
                    const char* edgename = name.c_str();
                    doc->addObject(fusedFeature, edgename);
                }
            }
            //BRepBuilderAPI_MakeFace来创建面
            BRepBuilderAPI_MakeWire wireMaker;
            wireMaker.Add(Edges);
            TopoDS_Wire wire = wireMaker.Wire();//所需的线框
            Base::Console().Message("%s %d\n", "线框是否闭合：", wire.Closed());
            Base::Console().Message("%s %d\n", "是否MakeFace成功：", wireMaker.IsDone());
            BRepBuilderAPI_MakeFace faceMaker(wire,Standard_False);
            TopoDS_Face mergeShape = faceMaker.Face();
            
            //BRepFill_Filling来创建面
            BRepFill_Filling BRepFill_faceMaker;
            for (TopTools_ListIteratorOfListOfShape it(Edges); it.More(); it.Next())
            {
                const TopoDS_Shape& shape = it.Value();
                TopoDS_Edge edge = TopoDS::Edge(shape);
                BRepFill_faceMaker.Add(edge, GeomAbs_C0);
            }
            BRepFill_faceMaker.Add(topoFace1, GeomAbs_C0);
            BRepFill_faceMaker.Add(topoFace2, GeomAbs_C0);
            BRepFill_faceMaker.SetApproxParam(1, 75);
            BRepFill_faceMaker.SetConstrParam(0.0001, 0.001, 0.1,1);
            BRepFill_faceMaker.SetResolParam(2,15,2, Standard_True);
            BRepFill_faceMaker.Build();
            TopoDS_Face fillFace = BRepFill_faceMaker.Face();


            if (1) {
                Part::Feature* fusedFeature = new Part::Feature();
                //// 将合并后的形状赋值给新对象的Shape属性
                fusedFeature->Shape.setValue(fusedShape);
                // 将新对象添加到文档中，并命名为"FusedFace"
                Part::Feature* fusedFeature2 = new Part::Feature();
                fusedFeature2->Shape.setValue(mergeShape);
                Part::Feature* fusedFeature3 = new Part::Feature();
                fusedFeature3->Shape.setValue(fillFace);
                App::Document* doc = App::GetApplication().getActiveDocument();
                doc->addObject(fusedFeature, "FusedFace");
                doc->addObject(fusedFeature2, "MergeFace");
                doc->addObject(fusedFeature3, "FillFace");
            }
        }
    }
    // Check for the errors/warnings
    /*   BOPTest::ReportAlerts(aRF.GetReport());

    if (BRepTest_Objects::IsHistoryNeeded())
        BRepTest_Objects::SetHistory(aRF.History());

    if (aRF.HasErrors())
        return 0;

    const TopoDS_Shape& aResult = aRF.Shape();
    DBRep::Set(theArgv[1], aResult);*/
    commitCommand();
    updateActive();

}

//===========================================================================
// CmdCADSimplifierRemoveFillets 删除圆角
//===========================================================================
DEF_STD_CMD_A(CmdCADSimplifierRemoveFillets)

CmdCADSimplifierRemoveFillets::CmdCADSimplifierRemoveFillets()
    : Command("CADSimplifier_RemoveFillets")
{
    sAppModule = "CADSimplifier";
    sGroup = QT_TR_NOOP("CADSimplifier");
    sMenuText = QT_TR_NOOP("删除圆角");
    sToolTipText = QT_TR_NOOP("自动筛选并删除圆角");
    sWhatsThis = "CADSimplifier_Test";
    sStatusTip = QT_TR_NOOP("CADSimplifier 删除圆角");
    sPixmap = "CADSimplifierWorkbench";
    sAccel = "CTRL+D";
}


//void CmdCADSimplifierRemoveFillets::activated(int iMsg)
//{
//    Q_UNUSED(iMsg);
//    Base::Console().Message("删除圆角  建设中....!\n");
//
//   
//    Gui::WaitCursor wc;
//    Base::Type partid = Base::Type::fromName("Part::Feature");
//    std::vector<Gui::SelectionObject> objs = Gui::Selection().getSelectionEx(nullptr, partid);
//    openCommand(QT_TRANSLATE_NOOP("Command", "RemoveFillet"));
//    for (std::vector<Gui::SelectionObject>::iterator it = objs.begin(); it != objs.end(); ++it) {
//        try {
//            std::string shape;
//            shape.append("sh=App.");
//            shape.append(it->getDocName());
//            shape.append(".");
//            shape.append(it->getFeatName());
//            shape.append(".Shape\n");
//
//            std::string faces;
//            std::vector<std::string> subnames = it->getSubNames();
//            for (std::vector<std::string>::iterator sub = subnames.begin(); sub != subnames.end();
//                 ++sub) {
//                faces.append("sh.");
//                faces.append(*sub);
//                faces.append(",");
//            }
//
//            doCommand(Doc,
//                      "\nsh = App.getDocument('%s').%s.Shape\n"
//                      "nsh = sh.defeaturing([%s])\n"
//                      "if not sh.isPartner(nsh):\n"
//                      "\t\tdefeat = "
//                      "App.ActiveDocument.addObject('Part::Feature','Defeatured').Shape = nsh\n"
//                      "\t\tGui.ActiveDocument.%s.hide()\n"
//                      "else:\n"
//                      "\t\tFreeCAD.Console.PrintError('Defeaturing failed\\n')",
//                      it->getDocName(), it->getFeatName(), faces.c_str(), it->getFeatName());
// /*           doCommand(Doc,
//                      "\nsh = App.getDocument('%s').%s.Shape\n"
//                      "nsh = CADSimplifier.SimplifierTool.RemoveFillet([%s])\n"
//                      "if not sh.isPartner(nsh):\n"
//                      "\t\tdefeat = "
//                      "App.ActiveDocument.addObject('CadSimplifier::Feature','Defeatured').Shape = nsh\n"
//                      "\t\tGui.ActiveDocument.%s.hide()\n"
//                      "else:\n"
//                      "\t\tFreeCAD.Console.PrintError('Defeaturing failed\\n')",
//                      it->getDocName(), it->getFeatName(), faces.c_str(), it->getFeatName());*/
//           
//        }
//        catch (const Base::Exception& e) {
//            Base::Console().Warning("%s: %s\n", it->getFeatName(), e.what());
//        }
//    }
//    commitCommand();
//    updateActive();
//}
void CmdCADSimplifierRemoveFillets::activated(int iMsg)
{
    Q_UNUSED(iMsg);
    Gui::WaitCursor wc;
    Base::Type partid = Base::Type::fromName("Part::Feature");
    std::vector<Gui::SelectionObject> objs = Gui::Selection().getSelectionEx(nullptr, partid);
    openCommand(QT_TRANSLATE_NOOP("Command", "RemoveFillet"));
    for (std::vector<Gui::SelectionObject>::iterator it = objs.begin(); it != objs.end(); ++it) {
        try {
            App::DocumentObject* pActiveDoc = it->getObject();
            Part::Feature* feat = static_cast<Part::Feature*>(pActiveDoc);
            TopoDS_Shape sh = feat->Shape.getShape().getShape();
            std::vector<TopoDS_Shape> Faces;
            std::vector<std::string> subnames = it->getSubNames();           
            for (std::vector<std::string>::iterator sub = subnames.begin(); sub != subnames.end();
                 ++sub) {
                TopoDS_Shape ref = feat->Shape.getShape().getSubShape(sub->c_str());
                Faces.push_back(ref);
            }

            SimplifierTool tool;
            tool.setShape(sh);
            //核心代码
            TopoDS_Shape nsh=tool.RemoveFillet(Faces);

            auto uiDoc = Gui::Application::Instance->activeDocument();
            Part::Feature* pNewFeat = (Part::Feature*) uiDoc->getDocument()->addObject("Part::Feature", "RemoveFillet");
            pNewFeat->Shape.setValue(nsh);
            feat->Visibility.setValue(false);
        }
        catch (const Base::Exception& e) {
            Base::Console().Warning("%s: %s\n", it->getFeatName(), e.what());
        }
    }
    // Check for the errors/warnings
 /*   BOPTest::ReportAlerts(aRF.GetReport());

    if (BRepTest_Objects::IsHistoryNeeded())
        BRepTest_Objects::SetHistory(aRF.History());

    if (aRF.HasErrors())
        return 0;

    const TopoDS_Shape& aResult = aRF.Shape();
    DBRep::Set(theArgv[1], aResult);*/
    commitCommand();
    updateActive();
}
bool CmdCADSimplifierRemoveFillets::isActive()
{ 
    Base::Type partid = Base::Type::fromName("Part::Feature");
    std::vector<Gui::SelectionObject> objs = Gui::Selection().getSelectionEx(nullptr, partid);
    for (std::vector<Gui::SelectionObject>::iterator it = objs.begin(); it != objs.end(); ++it) {
        std::vector<std::string> subnames = it->getSubNames();
        for (std::vector<std::string>::iterator sub = subnames.begin(); sub != subnames.end();
             ++sub) {
            if (sub->substr(0, 4) == "Face") {
                return true;
            }
        }
    }
    return false;
}


//===========================================================================
//  CADSimplifier_GetNeighborFaces
//===========================================================================
DEF_STD_CMD_A(CmdCADSimplifier_GetNeighborFaces)
CmdCADSimplifier_GetNeighborFaces::CmdCADSimplifier_GetNeighborFaces() 
    : Command("CADSimplifier_GetNeighborFaces")
{
    sAppModule = "CADSimplifier";
    sGroup = QT_TR_NOOP("CADSimplifier");
    sMenuText = QT_TR_NOOP("获取邻接面");
    sToolTipText = QT_TR_NOOP("自动获取所有的邻接面");
    sWhatsThis = "CADSimplifier_GetNeighborFaces";
    sStatusTip = sToolTipText;
    sPixmap = "CADSimplifier_GetNeighborFaces";   
    sAccel = "SHIFT+G"; 
}

void CmdCADSimplifier_GetNeighborFaces::activated(int iMsg)
{
    Q_UNUSED(iMsg);
    Gui::Control().showDialog(new CADSimplifierGui::TaskGetNeighborFaces(nullptr));

}

bool CmdCADSimplifier_GetNeighborFaces::isActive()
{ 
    Base::Type partid = Base::Type::fromName("Part::Feature");
    std::vector<Gui::SelectionObject> objs = Gui::Selection().getSelectionEx(nullptr, partid);
    for (std::vector<Gui::SelectionObject>::iterator it = objs.begin(); it != objs.end(); ++it) {
        std::vector<std::string> subnames = it->getSubNames();
        for (std::vector<std::string>::iterator sub = subnames.begin(); sub != subnames.end();
             ++sub) {
            if (sub->substr(0, 4) == "Face") {
                return true;
            }
        }
    }
    return false;
}



DEF_STD_CMD(CmdCADSimplifier_ShapeHealing)
CmdCADSimplifier_ShapeHealing::CmdCADSimplifier_ShapeHealing()
    : Command("CADSimplifier_ShapeHealing")
{
    sAppModule = "CADSimplifier";
    sGroup = QT_TR_NOOP("CADSimplifier");
    sMenuText = QT_TR_NOOP("修补shape");
    sToolTipText = QT_TR_NOOP("自动修补选择的shape");
    sWhatsThis = "CADSimplifier_ShapeHealing";
    sStatusTip = sToolTipText;
    sPixmap = "CADSimplifier_ShapeHealing";
    sAccel = "SHIFT+H";
}

void CmdCADSimplifier_ShapeHealing::activated(int iMsg) {

    SimplifierTool tool;
    //std::vector<TopoDS_Shape> shapeVec;
    0.001, 0.001, 0.1;
    double precision = 0.001;
    double mintol = 0.001;
    double maxtol = 0.1;
    tool.fixShape(/*shapeVec*/ precision, mintol,maxtol);   
    //Base::Console().Message("Hello, World!\n");
}
#pragma region 命令：自动去除圆角
  DEF_STD_CMD_A(CmdCADSimplifier_AutoRemoveFillets)
CmdCADSimplifier_AutoRemoveFillets::CmdCADSimplifier_AutoRemoveFillets()
    : Command("CADSimplifier_AutoRemoveFillets")
{
    sAppModule = "CADSimplifier";
    sGroup = QT_TR_NOOP("CADSimplifier");
    sMenuText = QT_TR_NOOP("自动去除圆角");
    sToolTipText = QT_TR_NOOP("自动去除圆角");
    sWhatsThis = "自动搜索并去除模型中的圆角";
    sStatusTip = sToolTipText;
    sPixmap = "CADSimplifier_GetNeighborFaces";
    //sAccel = "SHIFT+G";
}

void CmdCADSimplifier_AutoRemoveFillets::activated(int iMsg)
{
    Q_UNUSED(iMsg);

    //后续增加去除圆角选项
   // Gui::Control().showDialog(new CADSimplifierGui::TaskGetNeighborFaces(nullptr));

    Gui::WaitCursor wc;

    Base::Type partid = Base::Type::fromName("Part::Feature");
    std::vector<Gui::SelectionObject> objs = Gui::Selection().getSelectionEx(nullptr, partid);
    openCommand(QT_TRANSLATE_NOOP("Command", "AutoRemoveFillets"));
    for (std::vector<Gui::SelectionObject>::iterator it = objs.begin(); it != objs.end(); ++it) {
        try {
            App::DocumentObject* pActiveDoc = it->getObject();
            Part::Feature* feat = static_cast<Part::Feature*>(pActiveDoc);
            TopoDS_Shape sh = feat->Shape.getShape().getShape();

            SimplifierTool tool;
            tool.setShape(sh);
            tool.AutoRemoveFillet();

            //auto uiDoc = Gui::Application::Instance->activeDocument();
            //Part::Feature* pNewFeat =
            //    (Part::Feature*)uiDoc->getDocument()->addObject("Part::Feature", "RemoveFillet");
            //pNewFeat->Shape.setValue(nsh);
            //feat->Visibility.setValue(false);
        }
        catch (const Base::Exception& e) {
            Base::Console().Warning("%s: %s\n", it->getFeatName(), e.what());
        }
    }

    commitCommand();
    updateActive();
}

bool CmdCADSimplifier_AutoRemoveFillets::isActive() {
    return true;
}

#pragma endregion


//===========================================================================
// Application 中加入命令
//===========================================================================
void CreateCADSimplifierCommands(void)
{
    Gui::CommandManager& rcCmdMgr = Gui::Application::Instance->commandManager();
    rcCmdMgr.addCommand(new CmdCADSimplifierTest());
    rcCmdMgr.addCommand(new CmdCADSimplifierRemoveFillets());
    rcCmdMgr.addCommand(new CmdCADSimplifier_AutoRemoveFillets());
    rcCmdMgr.addCommand(new CmdCADSimplifier_GetNeighborFaces());
    rcCmdMgr.addCommand(new CmdCADSimplifier_ShapeHealing());
}
