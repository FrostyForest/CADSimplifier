#pragma once

#include <App/ComplexGeoData.h>
#include <Base/Exception.h>

#include <TopTools_ListOfShape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Wire.hxx>

#include <App/ComplexGeoData.h>
#include <Mod/CADSimplifier/CADSimplifierGlobal.h>
namespace CADSimplifier
{
class CADSimplifierExport SimplifierTool: public Data::ComplexGeoData
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

public:
    SimplifierTool() { ; }

    ~SimplifierTool() override { ; }

    /** @name Subelement management */
    //@{
    /** Sub type list
     *  List of different subelement types
     *  its NOT a list of the subelements itself
     */
    std::vector<const char*> getElementTypes() const override;
    unsigned long countSubElements(const char* Type) const override;
    /// get the subelement by type and number
    Data::Segment* getSubElement(const char* Type, unsigned long) const override;
    //@}

    inline void setTransform(const Base::Matrix4D& rclTrf) override { ; }
    inline Base::Matrix4D getTransform() const override
    {
        Base::Matrix4D mat;
        return mat;
    }
    void transformGeometry(const Base::Matrix4D& rclMat) override { ; }
    Base::BoundBox3d getBoundBox() const override;

    unsigned int getMemSize() const override { return 20; }

    void Save(Base::Writer& writer) const override;
    void Restore(Base::XMLReader& reader) override;

    TopoDS_Shape RemoveFillet(const std::vector<TopoDS_Shape>& s) ;

public :
    inline void setShape(const TopoDS_Shape& shape) { this->_Shape = shape; }

    inline const TopoDS_Shape& getShape() const { return this->_Shape; }

private:
    TopoDS_Shape _Shape;
#ifdef FC_DEBUG
public:
    TopoDS_Shape ShapeOfIntersectResult;
    TopoDS_Shape ShapeOfTrimResult;
#endif
};
}// namespace CADSimplifier