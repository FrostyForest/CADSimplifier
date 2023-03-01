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

#include <Gui/Control.h>
#include <Mod/CADSimplifier/App/SimplifierTool.h>
#include <Mod/CADSimplifier/App/Test_MergeFaces.h>
#include"DlgGetNeighborFaces.h"

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
    Q_UNUSED(iMsg);
    Gui::WaitCursor wc;

    Base::Type partid = Base::Type::fromName("Part::Feature");
    std::vector<Gui::SelectionObject> objs = Gui::Selection().getSelectionEx(nullptr, partid);
    openCommand(QT_TRANSLATE_NOOP("Command", "Test"));


    for (std::vector<Gui::SelectionObject>::iterator it = objs.begin(); it != objs.end(); ++it) {
        try {
            App::DocumentObject* pActiveDoc = it->getObject();
            Part::Feature* feat = static_cast<Part::Feature*>(pActiveDoc);

            TopoDS_Shape sh = feat->Shape.getShape().getShape();

            std::vector<TopoDS_Shape> Faces;
            std::vector<std::string> subnames = it->getSubNames();

            Test_MergeFaces tool;
            tool.Body = sh;

            for (std::vector<std::string>::iterator sub = subnames.begin(); sub != subnames.end();
                 ++sub) {
                TopoDS_Shape ref = feat->Shape.getShape().getSubShape(sub->c_str());
                tool.FacesToMerge.Append(ref);
            }
            if (tool.Preform()) {
                TopoDS_Shape nsh = tool.FaceMerged;
                auto uiDoc = Gui::Application::Instance->activeDocument();
                Part::Feature* pNewFeat = (Part::Feature*)uiDoc->getDocument()->addObject(
                    "Part::Feature", "MergedFace");
                pNewFeat->Shape.setValue(nsh);
            }
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
