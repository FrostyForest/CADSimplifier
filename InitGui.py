# CADSimplifier gui init module
# (c) 2001 Juergen Riegel LGPL

class CADSimplifierWorkbench (Workbench):
    "CADSimplifier workbench object"
    MenuText = "CADSimplifier"
    ToolTip = "CADSimplifier workbench"

    def Initialize(self):
        # load the module
        import CADSimplifier
        import CADSimplifierGui

    def GetClassName(self):
        return "CADSimplifierGui::Workbench"

Gui.addWorkbench(CADSimplifierWorkbench())
