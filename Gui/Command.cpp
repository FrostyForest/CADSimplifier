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

//
#include <App/DocumentObject.h>
#include <App/GeoFeature.h>
#include <Base/Exception.h>
#include <Gui/MainWindow.h>
#include <Gui/Selection.h>
#include <Gui/SelectionObject.h>
#include <Gui/WaitCursor.h>
#include <Mod/Part/App/PartFeature.h>

#include <Mod/CADSimplifier/App/SimplifierTool.h>

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

void CmdCADSimplifierTest::activated(int)
{
    Base::Console().Message("Hello, World!\n");
}

//===========================================================================
// CmdCADSimplifierRemoveFillets É¾³ýÔ²½Ç
//===========================================================================
DEF_STD_CMD_A(CmdCADSimplifierRemoveFillets)

CmdCADSimplifierRemoveFillets::CmdCADSimplifierRemoveFillets()
    : Command("CADSimplifier_RemoveFillets")
{
    sAppModule = "CADSimplifier";
    sGroup = QT_TR_NOOP("CADSimplifier");
    sMenuText = QT_TR_NOOP("É¾³ýÔ²½Ç");
    sToolTipText = QT_TR_NOOP("×Ô¶¯É¸Ñ¡²¢É¾³ýÔ²½Ç");
    sWhatsThis = "CADSimplifier_Test";
    sStatusTip = QT_TR_NOOP("CADSimplifier É¾³ýÔ²½Ç");
    sPixmap = "CADSimplifierWorkbench";
    sAccel = "CTRL+D";
}


//void CmdCADSimplifierRemoveFillets::activated(int iMsg)
//{
//    Q_UNUSED(iMsg);
//    Base::Console().Message("É¾³ýÔ²½Ç  ½¨ÉèÖÐ....!\n");
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
            TopoDS_Shape nsh=tool.RemoveFillet(Faces);
            auto uiDoc = Gui::Application::Instance->activeDocument();
            Part::Feature* pNewFeat = (Part::Feature*) uiDoc->getDocument()->addObject("Part::Feature", "RemoveFillet");
            pNewFeat->Shape .setValue( nsh);
            feat->Visibility.setValue(false);
#ifdef FC_DEBUG
            Part::Feature* pInsectFaces =
                (Part::Feature*)uiDoc->getDocument()->addObject("Part::Feature", "InsectFaces");
            pInsectFaces->Shape.setValue(tool.ShapeOfIntersectResult);
            Part::Feature* pTrimFaces =
                (Part::Feature*)uiDoc->getDocument()->addObject("Part::Feature", "TrimedFaces");
            pTrimFaces->Shape.setValue(tool.ShapeOfTrimResult);

#endif
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
// Application ÖÐ¼ÓÈëÃüÁî
//===========================================================================
void CreateCADSimplifierCommands(void)
{
    Gui::CommandManager& rcCmdMgr = Gui::Application::Instance->commandManager();
    rcCmdMgr.addCommand(new CmdCADSimplifierTest());
    rcCmdMgr.addCommand(new CmdCADSimplifierRemoveFillets());
}
