#ifndef CADSimplifierGUI_DLGGetNeighborFaces_H
#define CADSimplifierGUI_DLGGetNeighborFaces_H

#include <QDialog>
#include <QItemDelegate>
#include <QStandardItemModel>

#include <Gui/Selection.h>
#include <Gui/TaskView/TaskDialog.h>
#include <Gui/TaskView/TaskView.h>

namespace Base
{
class Quantity;
}

namespace CADSimplifierGui
{
class Ui_DlgGetNeighborFaces;
class DlgGetNeighborFaces: public QWidget, public Gui::SelectionObserver
{
    Q_OBJECT

public:
    enum ShapeType
    {
        VERTICES,
        EDGES,
        Faces
    };

    DlgGetNeighborFaces(ShapeType type,Part::FilletBase*,QWidget* parent = nullptr,Qt::WindowFlags fl = Qt::WindowFlags());
    ~DlgGetNeighborFaces() override;
    bool accept();

protected:
    void findShapes();
    void setupGetNeighborFaces(const std::vector<App::DocumentObject*>&);
    void changeEvent(QEvent* e) override;
    virtual const char* getShapeType() const;

private:
    void onSelectionChanged(const Gui::SelectionChanges& msg) override { return; };
    void onDeleteObject(const App::DocumentObject&);
    void onDeleteDocument(const App::Document&);
//    void onSelectEdge(const QString& subelement, int type);
//    void onSelectEdgesOfFace(const QString& subelement, int type);

private Q_SLOTS:
    void on_shapeObject_activated(int);
//    void on_selectEdges_toggled(bool);
//    void on_selectFaces_toggled(bool);
    void on_selectAllButton_clicked();
    void on_selectNoneButton_clicked();
//    void on_shapeType_activated(int);
//    void on_filletStartRadius_valueChanged(const Base::Quantity&);
//    void on_filletEndRadius_valueChanged(const Base::Quantity&);
//    void toggleCheckState(const QModelIndex&);
//    void onHighlightEdges();

 
private:
    std::unique_ptr<Ui_DlgGetNeighborFaces> ui;
    class Private;
    std::unique_ptr<Private> d;
    CADSimplifier::SimplifierTool tool;
};

class GetNeighborFacesDialog: public QDialog
{
    Q_OBJECT
public:
    GetNeighborFacesDialog(DlgGetNeighborFaces::ShapeType type,Part::FilletBase*,QWidget* parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());
    ~GetNeighborFacesDialog() override;
    void accept() override;

private:
    DlgGetNeighborFaces* widget;
};


class TaskGetNeighborFaces: public Gui::TaskView::TaskDialog
{
    Q_OBJECT
public:
    explicit TaskGetNeighborFaces(Part::FilletBase* fillet);
    ~TaskGetNeighborFaces() override;

public:
    void open() override;
    void clicked(int) override;
    bool accept() override;
    bool reject() override;

    QDialogButtonBox::StandardButtons getStandardButtons() const override
    {
        return QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
    }
    bool needsFullSpace() const override { return true; }

private:
    DlgGetNeighborFaces* widget;
    Gui::TaskView::TaskBox* taskbox;
};
}// namespace CADSimplifierGui



#endif//CADSimplifierGUI_DLGGetNeighborFaces_H
