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