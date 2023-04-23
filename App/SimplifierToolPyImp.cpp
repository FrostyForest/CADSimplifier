/***************************************************************************
 *   Copyright (c) 2008 Jürgen Riegel <juergen.riegel@web.de>              *
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


#include <App/PropertyStandard.h>
#include <Base/FileInfo.h>
#include <Base/GeometryPyCXX.h>
#include <Base/MatrixPy.h>
#include <Base/Rotation.h>
#include <Base/Stream.h>
#include <Base/Vector3D.h>
#include <Base/VectorPy.h>


//#include <mod/CADSimplifier/App/SimplifierToolPy.h>

#include "SimplifierTool.h"
// inclusion of the generated files (generated out of PointsPy.xml)
#include "SimplifierToolPy.h"
#include "SimplifierToolPy.cpp"


using namespace CADSimplifier;


#ifndef M_PI
    #define M_PI    3.14159265358979323846 /* pi */
#endif

#ifndef M_PI_2
    #define M_PI_2  1.57079632679489661923 /* pi/2 */
#endif

// returns a string which represents the object e.g. when printed in python
std::string SimplifierToolPy::representation() const
{
    std::stringstream str;
    str << "<Shape object at >";

    return str.str();
}

PyObject *SimplifierToolPy::PyMake(struct _typeobject *, PyObject *, PyObject *)  // Python wrapper
{
    // create a new instance of TopoShapePy and the Twin object
    return new SimplifierToolPy(new SimplifierTool);
}

int SimplifierToolPy::PyInit(PyObject* args, PyObject*)
{
    PyObject *pcObj=nullptr;
    if (!PyArg_ParseTuple(args, "|O", &pcObj))
        return -1;


    return 0;
}

PyObject* SimplifierToolPy::RemoveFillet(PyObject* args)
{
    PyObject* l;
    if (!PyArg_ParseTuple(args, "O", &l))
        return nullptr;
    return nullptr;

    //try {
    //    Py::Sequence list(l);
    //    std::vector<TopoDS_Shape> shapes;
    //    for (Py::Sequence::iterator it = list.begin(); it != list.end(); ++it) {
    //        Py::TopoShape sh(*it);
    //        shapes.push_back(sh.extensionObject()->getTopoShapePtr()->getShape());
    //    }
    //    PyTypeObject* type = this->GetType();
    //    PyObject* inst = type->tp_new(type, this, nullptr);
    //    //static_cast<TopoShapePy*>(inst)->getTopoShapePtr()->setShape(
    //    //    this->get()->defeaturing(shapes));
    //    return inst;
    //}
    //catch (const Standard_Failure& e) {
    //    //PyErr_SetString(PartExceptionOCCError, e.GetMessageString());
    //    return nullptr;
    //}
}

// End of Methods, Start of Attributes

Py::List SimplifierToolPy::getFaces() const
{
    Py::List list;
    return list;
    //getShapes<TopoShapeFacePy>(getTopoShapePtr());
}


PyObject* SimplifierToolPy::getCustomAttributes(const char* /*attr*/) const { return nullptr; }

int SimplifierToolPy::setCustomAttributes(const char* /*attr*/, PyObject* /*obj*/) { return 0; }