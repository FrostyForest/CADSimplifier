#ifndef CADSimplifierGUI_DLGGetNeighborFaces_H
#define CADSimplifierGUI_DLGGetNeighborFaces_H

#include <QDialog>
#include <QItemDelegate>
#include <QStandardItemModel>

#include <Gui/Selection.h>
#include <Gui/TaskView/TaskDialog.h>
#include <Gui/TaskView/TaskView.h>

#include <QItemSelection>
#include <QStandardItemModel>

namespace Base
{
class Quantity;
}

namespace CADSimplifierGui
{
class Ui_DlgGetNeighborFaces;
class FilletRadiusDelegate: public QItemDelegate
{
    Q_OBJECT

public:
    explicit FilletRadiusDelegate(QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;
};


class FilletRadiusModel: public QStandardItemModel
{
    Q_OBJECT

public:
    explicit FilletRadiusModel(QObject* parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;
    void updateCheckStates();

Q_SIGNALS:
    void toggleCheckState(const QModelIndex&);
};


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
    bool eventFilter(QObject* target, QEvent* event);
    void findShapes();
    void changeEvent(QEvent* e) override;
    virtual const char* getShapeType() const;
private:
    void onSelectionChanged(const Gui::SelectionChanges& msg) override;
    void onSelectShape(const QString& shapeTypeName, const QString& subelement, int type);
    void onDeleteObject(const App::DocumentObject&);
    void onDeleteDocument(const App::Document&);
private Q_SLOTS:
    void on_shapeObject_activated(int);
    void on_selectEdges_toggled(bool);
    void on_selectFaces_toggled(bool);

    void on_selectAllButton_clicked();
    void on_selectNoneButton_clicked();
    void on_selectFitButton_clicked();

    void on_filletType_activated(int);
    void on_filletStartRadius_valueChanged(const Base::Quantity&);
    void on_filletEndRadius_valueChanged(const Base::Quantity&);
    void toggleCheckState(const QModelIndex&);
    void onHighlightFaces();
private:
    std::unique_ptr<Ui_DlgGetNeighborFaces> ui;
    class Private;
    std::unique_ptr<Private> d;//implģʽ
    std::unique_ptr<CADSimplifier::SimplifierTool> tool;
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
