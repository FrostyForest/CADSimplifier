#pragma once
#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <BOPAlgo_BuilderShape.hxx>
#include <BRepTools_History.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
 #ifdef FC_DEBUG
#define CREATE_DEBUG_SHAPE
#endif
class BOPAlgo_RemoveFillets: public BOPAlgo_BuilderShape
{
public:
    DEFINE_STANDARD_ALLOC

public://! @name Constructors
    //! Empty constructor
    BOPAlgo_RemoveFillets() : BOPAlgo_BuilderShape() {}


public://! @name Setting input data for the algorithm
    //! Sets the shape for processing.
    //! @param theShape [in] The shape to remove the faces from.
    //!                      It should either be the SOLID, COMPSOLID or COMPOUND of Solids.
    void SetShape(const TopoDS_Shape& theShape) { myInputShape = theShape; }

    //! Returns the input shape
    const TopoDS_Shape& InputShape() const { return myInputShape; }

    //! Adds the face to remove from the input shape.
    //! @param theFace [in] The shape to extract the faces for removal.
    void AddFaceToRemove(const TopoDS_Shape& theFace) { myFacesToRemove.Append(theFace); }

    //! Adds the faces to remove from the input shape.
    //! @param theFaces [in] The list of shapes to extract the faces for removal.
    void AddFacesToRemove(const TopTools_ListOfShape& theFaces)
    {
        TopTools_ListIteratorOfListOfShape it(theFaces);
        for (; it.More(); it.Next())
            myFacesToRemove.Append(it.Value());
    }

    //! Returns the list of faces which have been requested for removal
    //! from the input shape.
    const TopTools_ListOfShape& FacesToRemove() const { return myFacesToRemove; }


public://! @name Performing the operation
    //! Performs the operation
    Standard_EXPORT virtual void
    Perform(const Message_ProgressRange& theRange = Message_ProgressRange()) Standard_OVERRIDE;


public://! @name Clearing the contents of the algorithm
    //! Clears the contents of the algorithm from previous run,
    //! allowing reusing it for following removals.
    virtual void Clear() Standard_OVERRIDE
    {
        BOPAlgo_BuilderShape::Clear();
        myInputShape.Nullify();
        myShape.Nullify();
        myFacesToRemove.Clear();
        myFeatures.Clear();
        myInputsMap.Clear();
    }


protected://! @name Protected methods performing the removal
    //! Checks the input data on validity for the algorithm:
    //! - The input shape must be either a SOLID, COMPSOLID or COMPOUND of Solids.
    //! If the input shape is not a solid, the method looks for the solids
    //! in <myInputShape> and uses only them. All other shapes are simply removed.
    //! If no solids were found, the Error of unsupported type is returned.
    Standard_EXPORT virtual void CheckData() Standard_OVERRIDE;

    //! Prepares the faces to remove:
    //! - Gets only faces contained in the input solids;
    //! - Builds connected blocks of faces creating separate features to remove.
    Standard_EXPORT void PrepareFeatures(const Message_ProgressRange& theRange);

    //! Removes the features and fills the created gaps by extension of the adjacent faces.
    //! Processes each feature separately.
    Standard_EXPORT void RemoveFeatures(const Message_ProgressRange& theRange);

    //! Remove the single feature from the shape.
    //! @param theFeature [in] The feature to remove;
    //! @param theSolids [in] The solids to be reconstructed after feature removal;
    //! @param theFeatureFacesMap [in] The map of feature faces;
    //! @param theHasAdjacentFaces [in] Shows whether the adjacent faces have been
    //!                                 found for the feature or not;
    //! @param theAdjFaces [in] The reconstructed adjacent faces covering the feature;
    //! @param theAdjFacesHistory [in] The history of the adjacent faces reconstruction;
    //! @param theSolidsHistoryNeeded [in] Defines whether the history of solids
    //!                                    modifications should be tracked or not.
    Standard_EXPORT void RemoveFeature(const TopoDS_Shape& theFeature,
                                       const TopTools_IndexedMapOfShape& theSolids,
                                       const TopTools_MapOfShape& theFeatureFacesMap,
                                       const Standard_Boolean theHasAdjacentFaces,
                                       const TopTools_IndexedDataMapOfShapeListOfShape& theAdjFaces,
                                       const Handle(BRepTools_History) & theAdjFacesHistory,
                                       const Standard_Boolean theSolidsHistoryNeeded,
                                       const Message_ProgressRange& theRange);

    //! Updates history with the removed features
    Standard_EXPORT void UpdateHistory(const Message_ProgressRange& theRange);

    //! Simplifies the result by removing extra edges and vertices created
    //! during removal of the features.
    Standard_EXPORT void SimplifyResult(const Message_ProgressRange& theRange);

    //! Post treatment - restore the type of the initial shape
    Standard_EXPORT void PostTreat();

    //! Filling steps for constant operations
    Standard_EXPORT void fillPIConstants(const Standard_Real theWhole,
                                         BOPAlgo_PISteps& theSteps) const Standard_OVERRIDE;

protected://! @name Fields
    // Inputs
    TopoDS_Shape myInputShape;           //!< Input shape
    TopTools_ListOfShape myFacesToRemove;//!< Faces to remove

    // Intermediate
    TopTools_ListOfShape myFeatures;       //!< List of not connected features to remove
                                           //! (each feature is a compound of faces)
    TopTools_IndexedMapOfShape myInputsMap;//!< Map of all sub-shapes of the input shape

#ifdef CREATE_DEBUG_SHAPE
private:
    TopoDS_Shape ShapeOfIntersectResult;
    TopoDS_Shape ShapeOfTrimResult;
#endif
};




