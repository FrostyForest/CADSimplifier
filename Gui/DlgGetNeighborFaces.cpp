#include "PreCompiled.h"
#ifndef _PreComp_
#include <QHeaderView>
#include <QItemDelegate>
#include <QItemSelectionModel>
#include <QLocale>
#include <QMessageBox>
#include <QTimer>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <algorithm>
#include <climits>
#include <sstream>
#include <QLineEdit>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>


#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/details/SoFaceDetail.h>
#include <Inventor/details/SoLineDetail.h>
#endif

#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <Base/UnitsApi.h>
#include <Gui/Application.h>
#include <Gui/BitmapFactory.h>
#include <Gui/Command.h>
#include <Gui/QuantitySpinBox.h>
#include <Gui/Selection.h>
#include <Gui/SelectionFilter.h>
#include <Gui/SelectionObject.h>
#include <Gui/SoFCUnifiedSelection.h>
#include <Gui/ViewProvider.h>
#include <Gui/WaitCursor.h>
#include <Gui/Window.h>





#include <Mod/Part/App/PartFeature.h>
#include<Mod/Part/Gui/SoBrepPointSet.h>
#include<Mod/Part/Gui/SoBrepEdgeSet.h>
#include<Mod/Part/Gui/SoBrepFaceSet.h>
#include<Mod/CADSimplifier/APP/SimplifierTool.h>
#include "ui_DlgGetNeighborFaces.h"
#include "DlgGetNeighborFaces.h"


using namespace CADSimplifierGui;
namespace bp = boost::placeholders;

FilletRadiusDelegate::FilletRadiusDelegate(QObject* parent) : QItemDelegate(parent) {}

QWidget* FilletRadiusDelegate::createEditor(QWidget* parent,
                                            const QStyleOptionViewItem& /* option */,
                                            const QModelIndex& index) const
{
    if (index.column() < 1)
        return nullptr;

    Gui::QuantitySpinBox* editor = new Gui::QuantitySpinBox(parent);
    editor->setUnit(Base::Unit::Length);
    editor->setMinimum(0.0);
    editor->setMaximum(INT_MAX);
    editor->setSingleStep(0.1);

    return editor;
}

void FilletRadiusDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    Base::Quantity value = index.model()->data(index, Qt::EditRole).value<Base::Quantity>();

    Gui::QuantitySpinBox* spinBox = static_cast<Gui::QuantitySpinBox*>(editor);
    spinBox->setValue(value);
}

void FilletRadiusDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                        const QModelIndex& index) const
{
    Gui::QuantitySpinBox* spinBox = static_cast<Gui::QuantitySpinBox*>(editor);
    spinBox->interpretText();
    
    Base::Quantity value = spinBox->value();

    model->setData(index, QVariant::fromValue<Base::Quantity>(value), Qt::EditRole);
}

void FilletRadiusDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                                const QModelIndex& /* index */) const
{
    editor->setGeometry(option.rect);
}






FilletRadiusModel::FilletRadiusModel(QObject* parent) : QStandardItemModel(parent) {}

void FilletRadiusModel::updateCheckStates()
{
    // See http://www.qtcentre.org/threads/18856-Checkboxes-in-Treeview-do-not-get-refreshed?s=b0fea2bfc66da1098413ae9f2a651a68&p=93201#post93201
    Q_EMIT layoutChanged();
}

Qt::ItemFlags FilletRadiusModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags fl = QStandardItemModel::flags(index);
    if (index.column() == 0)
        fl = fl | Qt::ItemIsUserCheckable;
    return fl;
}

bool FilletRadiusModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool ok = QStandardItemModel::setData(index, value, role);
    if (role == Qt::CheckStateRole) {
        Q_EMIT toggleCheckState(index);
    }
    return ok;
}

QVariant FilletRadiusModel::data(const QModelIndex& index, int role) const
{
    QVariant value = QStandardItemModel::data(index, role);
    if (role == Qt::DisplayRole && index.column() >= 1) {
        Base::Quantity q = value.value<Base::Quantity>();
        QString str = q.getUserString();
        return str;
    }
    return value;
}



namespace CADSimplifierGui
{
class EdgeFaceSelection: public Gui:: SelectionFilterGate
{
    bool allowFace;
    App::DocumentObject*& object;

public:
    explicit EdgeFaceSelection(App::DocumentObject*& obj)
        : Gui::SelectionFilterGate(nullPointer()), allowFace(true), object(obj)
    {}
    void selectEdges() { allowFace = false; }
    void selectFaces() { allowFace = true; }
    bool allow(App::Document* /*pDoc*/, App::DocumentObject* pObj, const char* sSubName) override
    {
        if (pObj != this->object)
            return false;
        if (!sSubName || sSubName[0] == '\0')
            return false;
        std::string element(sSubName);
        if (allowFace)
            return element.substr(0, 4) == "Face";
        else
            return element.substr(0, 4) == "Edge";
    }
};
class DlgGetNeighborFaces::Private
{
public:
    App::DocumentObject* object;
    EdgeFaceSelection* selection;
    Part::FilletBase* fillet;
    QTimer* highlighttimer;
    ShapeType shapeType;
    std::vector<int> edge_ids;
    std::vector<int> face_ids;
    TopTools_IndexedMapOfShape all_edges;
    TopTools_IndexedMapOfShape all_faces;
    using Connection = boost::signals2::connection;
    Connection connectApplicationDeletedObject;
    Connection connectApplicationDeletedDocument;

    class SelectionObjectCompare
    {
    public:
        App::DocumentObject* obj;
        explicit SelectionObjectCompare(App::DocumentObject* obj) : obj(obj) {}
        bool operator()(const Gui::SelectionObject& sel) const { return (sel.getObject() == obj); }
    };
};
}

/* TRANSLATOR PartGui::DlgGetNeighborFaces */
DlgGetNeighborFaces::DlgGetNeighborFaces(ShapeType type, Part::FilletBase* fillet,QWidget* parent,Qt::WindowFlags fl)
    : QWidget(parent, fl), ui(new Ui_DlgGetNeighborFaces()), d(new Private())
{
    ui->setupUi(this);
    ui->groupBox->setVisible(0);
    ui->filletStartRadius->setMaximum(INT_MAX);
    ui->filletStartRadius->setMinimum(0);
    ui->filletStartRadius->setUnit(Base::Unit::Length);

    ui->filletEndRadius->setMaximum(INT_MAX);
    ui->filletEndRadius->setMinimum(0);
    ui->filletEndRadius->setUnit(Base::Unit::Length);

    d->object = nullptr;
    d->selection = new EdgeFaceSelection(d->object);
    Gui::Selection().addSelectionGate(d->selection);
    d->fillet = fillet;
    d->connectApplicationDeletedObject = App::GetApplication().signalDeletedObject.connect(
        boost::bind(&DlgGetNeighborFaces::onDeleteObject, this, bp::_1));
    d->connectApplicationDeletedDocument = App::GetApplication().signalDeleteDocument.connect(
        boost::bind(&DlgGetNeighborFaces::onDeleteDocument, this, bp::_1));
   
    // set tree view with three columns
    QStandardItemModel* model = new FilletRadiusModel(this);
    connect(model, SIGNAL(toggleCheckState(const QModelIndex&)), this,SLOT(toggleCheckState(const QModelIndex&)));
    model->insertColumns(0, 3);

    // timer for highlighting
    d->highlighttimer = new QTimer(this);
    d->highlighttimer->setSingleShot(true);
    connect(d->highlighttimer, SIGNAL(timeout()), this, SLOT(onHighlightFaces()));
    
    d->shapeType = type;
    if (d->shapeType == DlgGetNeighborFaces::Faces) {
        ui->parameterName->setTitle(QStringLiteral("邻接圆角面"));
        ui->labelfillet->setText(tr("Faces type"));
        model->setHeaderData(0, Qt::Horizontal, tr("ID"), Qt::DisplayRole);
        model->setHeaderData(1, Qt::Horizontal, tr("Start radius"), Qt::DisplayRole);
        model->setHeaderData(2, Qt::Horizontal, tr("End radius"), Qt::DisplayRole);
    }
    ui->treeView->setRootIsDecorated(false);
    ui->treeView->setItemDelegate(new CADSimplifierGui::FilletRadiusDelegate(this));
    ui->treeView->setModel(model);

    QHeaderView* header = ui->treeView->header();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setDefaultAlignment(Qt::AlignLeft);
    header->setSectionsMovable(false);

    ui->minRadius->insertPlainText(QString::fromLatin1("0"));
    ui->maxRadius->append(QString::fromLatin1("1"));
   

    ui->minRadius->installEventFilter(this);
    ui->maxRadius->installEventFilter(this);

    on_filletType_activated(0); 
    findShapes();
    on_selectFitButton_clicked();

    ui->selectFitButton->hide();
    ui->selectFitButton->hide();
    ui->selectEdges->hide();
    ui->selectFaces->hide();
    ui->labelfillet->hide();
    ui->filletType->hide();
    ui->labelRadius->hide();
    ui->filletStartRadius->hide();  
    ui->filletEndRadius->hide();
    
    
}

void DlgGetNeighborFaces::on_filletStartRadius_valueChanged(const Base::Quantity& radius)
{
    QAbstractItemModel* model = ui->treeView->model();
    for (int i = 0; i < model->rowCount(); ++i) {
        QVariant value = model->index(i, 0).data(Qt::CheckStateRole);
        Qt::CheckState checkState = static_cast<Qt::CheckState>(value.toInt());
        // is item checked
        if (checkState & Qt::Checked) {
            model->setData(model->index(i, 1), QVariant::fromValue<Base::Quantity>(radius));
        }
    }
}

void DlgGetNeighborFaces::on_filletEndRadius_valueChanged(const Base::Quantity& radius)
{
    QAbstractItemModel* model = ui->treeView->model();
    for (int i = 0; i < model->rowCount(); ++i) {
        QVariant value = model->index(i, 0).data(Qt::CheckStateRole);
        Qt::CheckState checkState = static_cast<Qt::CheckState>(value.toInt());
        // is item checked
        if (checkState & Qt::Checked) {
            model->setData(model->index(i, 2), QVariant::fromValue<Base::Quantity>(radius));
        }
    }
}
void DlgGetNeighborFaces::on_filletType_activated(int index)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->treeView->model());
    if (index == 0) {
        if (d->shapeType == ShapeType::Faces)        
            model->setHeaderData(1, Qt::Horizontal, QStringLiteral("半径"), Qt::DisplayRole);
        ui->treeView->hideColumn(2);
        ui->filletEndRadius->hide();
    }
    else {
        if (d->shapeType == ShapeType::Faces)         
            model->setHeaderData(1, Qt::Horizontal, tr("Start radius"), Qt::DisplayRole);
        ui->treeView->showColumn(2);
        ui->filletEndRadius->show();
    }
    ui->treeView->resizeColumnToContents(0);
    ui->treeView->resizeColumnToContents(1);
    ui->treeView->resizeColumnToContents(2);
}

void DlgGetNeighborFaces::on_selectEdges_toggled(bool on)
{
    if (on)
        d->selection->selectEdges();
}

void DlgGetNeighborFaces::on_selectFaces_toggled(bool on)
{
    if (on)
        d->selection->selectFaces();
}


void DlgGetNeighborFaces::toggleCheckState(const QModelIndex& index)
{
    if (!d->object) return;
    QVariant check = index.data(Qt::CheckStateRole);
    int id = index.data(Qt::UserRole).toInt();
    QString name = QString::fromLatin1("Face%1").arg(id);
    Qt::CheckState checkState = static_cast<Qt::CheckState>(check.toInt());
    bool block = this->blockSelection(true);

    // is item checked
    if (checkState & Qt::Checked) {
        App::Document* doc = d->object->getDocument();
        Gui::Selection().addSelection(doc->getName(), d->object->getNameInDocument(),
                                      (const char*)name.toLatin1());
    }
    else {
        App::Document* doc = d->object->getDocument();
        Gui::Selection().rmvSelection(doc->getName(), d->object->getNameInDocument(),
                                      (const char*)name.toLatin1());
    }
    this->blockSelection(block);
}


void DlgGetNeighborFaces::onSelectShape(const QString& shapeTypeName, const QString& subelement,int type)
{
    Gui::SelectionChanges::MsgType msgType = Gui::SelectionChanges::MsgType(type);
    QAbstractItemModel* model = ui->treeView->model();
    for (int i = 0; i < model->rowCount(); ++i) {
        int id = model->data(model->index(i, 0), Qt::UserRole).toInt();
        QString name = shapeTypeName;           
        name += QString::fromLatin1("%1").arg(id);
        if (name == subelement) {
            // ok, check the selected sub-element
            Qt::CheckState checkState =
                (msgType == Gui::SelectionChanges::AddSelection ? Qt::Checked : Qt::Unchecked);
            QVariant value(static_cast<int>(checkState));
            QModelIndex index = model->index(i, 0);
            model->setData(index, value, Qt::CheckStateRole);
            // select the item
            ui->treeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
            QItemSelection selection(index, model->index(i, 1));
            ui->treeView->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
            ui->treeView->update();
            break;
        }
    }
}


void DlgGetNeighborFaces::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    // no object selected in the combobox or no sub-element was selected
    if (!d->object || !msg.pSubName)
        return;
    if (msg.Type == Gui::SelectionChanges::AddSelection
        || msg.Type == Gui::SelectionChanges::RmvSelection) {
        // when adding a sub-element to the selection check
        // whether this is the currently handled object
     
        App::Document* doc = d->object->getDocument();      
        std::string docname = doc->getName();
        std::string objname = d->object->getNameInDocument();
        if (docname == msg.pDocName && objname == msg.pObjectName) {
            QString subelement = QString::fromLatin1(msg.pSubName);
            if (subelement.startsWith(QLatin1String("Face"))) {   
                onSelectShape(QLatin1String("Face"), subelement, msg.Type);
               
            }
            else if (subelement.startsWith(QLatin1String("Edge"))) {
                onSelectShape(QLatin1String("Edge"), subelement, msg.Type);
                
            }
        }      
    }
    if (msg.Type != Gui::SelectionChanges::SetPreselect
        && msg.Type != Gui::SelectionChanges::RmvPreselect)
        d->highlighttimer->start(20);
}

void DlgGetNeighborFaces::onHighlightFaces()
{
    Gui::ViewProvider* view = Gui::Application::Instance->getViewProvider(d->object);
    if (view) {        
        // deselect all points
        {
            SoSearchAction searchAction;
            searchAction.setType(PartGui::SoBrepPointSet::getClassTypeId());
            searchAction.setInterest(SoSearchAction::FIRST);
            searchAction.apply(view->getRoot());
            SoPath* selectionPath = searchAction.getPath();
            if (selectionPath) {
                Gui::SoSelectionElementAction action(Gui::SoSelectionElementAction::None);
                action.apply(selectionPath);
            }
        }

        //在treeviw中检查选中的item，并将它们添加到SoSelectionElementAction中，以便在3D视图中显示选择的面 待完善中
         //select the faces  //连续选中都显示高亮
        //{
        //    SoSearchAction searchAction;
        //    searchAction.setType(PartGui::SoBrepFaceSet::getClassTypeId());
        //    searchAction.setInterest(SoSearchAction::FIRST);
        //    searchAction.apply(view->getRoot());
        //    SoPath* selectionPath = searchAction.getPath();
        //    if (selectionPath) {
        //        ParameterGrp::handle hGrp =
        //            Gui::WindowParameter::getDefaultParameter()->GetGroup("View");
        //        SbColor selectionColor(0.1f, 0.8f, 0.1f);
        //        unsigned long selection = (unsigned long)(selectionColor.getPackedValue());
        //        selection = hGrp->GetUnsigned("SelectionColor", selection);
        //        float transparency;
        //        selectionColor.setPackedValue((uint32_t)selection, transparency);

        //        // clear the selection first
        //        //Gui::SoSelectionElementAction clear(Gui::SoSelectionElementAction::None);
        //        //clear.apply(selectionPath);

        //        Gui::SoSelectionElementAction action(Gui::SoSelectionElementAction::Append);
        //        action.setColor(selectionColor);
        //        action.apply(selectionPath);

        //        QAbstractItemModel* model = ui->treeView->model();
        //        SoFaceDetail detail;
        //        action.setElement(&detail);
        //        for (int i = 0; i < model->rowCount(); ++i) {
        //            QVariant value = model->index(i, 0).data(Qt::CheckStateRole);
        //            Qt::CheckState checkState = static_cast<Qt::CheckState>(value.toInt());
        //            // is item checked
        //            if (checkState & Qt::Checked) {
        //                // the index value of the face
        //                int id = model->index(i, 0).data(Qt::UserRole).toInt();
        //                detail.setFaceIndex(id - 1);//设置索引
        //                action.apply(selectionPath);
        //            }
        //        }
        //    }
        //}
    }
}

void DlgGetNeighborFaces::on_shapeObject_activated(int itemPos)
{ 

    d->object = nullptr; 
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->treeView->model());
    model->removeRows(0, model->rowCount());
    App::Document* doc = App::GetApplication().getActiveDocument();
    if (!doc) return;
    Gui::WaitCursor wc;
    QByteArray name = ui->shapeObject->itemData(itemPos).toByteArray();
    App::DocumentObject* docObj = doc->getObject((const char*)name);    
    if (docObj && docObj->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId())) {
        d->object = docObj;
  
        //// get current selection and their sub-elements
        /*std::vector<Gui::SelectionObject> selObj = Gui::Selection().getSelectionEx(doc->getName());
        std::vector<Gui::SelectionObject>::iterator selIt =
            std::find_if(selObj.begin(), selObj.end(), Private::SelectionObjectCompare(d->object));*/     
        TopTools_IndexedMapOfShape allFace;
        tool->getAllFacesOfASolidOfDocument(allFace, name, docObj);       
        std::vector<TopoDS_Shape> selectedFaces;
        tool->getSelectedFaces(selectedFaces);      
        Gui::Selection().rmvSelection(doc->getName(),docObj->getNameInDocument());
        std::vector<TopoDS_Shape> adjacentFacesOfNoPlane; 
        d->face_ids.clear();
        d->face_ids = tool->getAllNeighborFacesIdOfNoPlane(selectedFaces, allFace, adjacentFacesOfNoPlane);
        
        model->insertRows(0, d->face_ids.size());       
        int index = 0;//行数     
        for (auto it = d->face_ids.begin(); it != d->face_ids.end(); ++it) {             
            model->setData(model->index(index, 0),QVariant(tr("Face%1").arg(*it)));              
            model->setData(model->index(index, 0), QVariant(*it), Qt::UserRole);
            double curRadius = 0.0;
            bool flag = tool->getSurfaceGemoInfo(TopoDS::Face(adjacentFacesOfNoPlane.at(index)), curRadius);
            if (!flag) {
#ifdef FC_DEBUG
                QString text;
                text = QString::fromLatin1("get the curvature radius parameter of input face failed");
                QMessageBox::warning(this, QString::fromLatin1("Error Tip"), text);    
#endif
                return;
            }
            model->setData(model->index(index, 1),
                QVariant::fromValue<Base::Quantity>(Base::Quantity(curRadius, Base::Unit::Length)));
            model->setData(model->index(index, 2),
                QVariant::fromValue<Base::Quantity>(Base::Quantity(curRadius, Base::Unit::Length)));          
            std::stringstream element;
            element << "Face" << *it;
            if (Gui::Selection().isSelected(docObj, element.str().c_str()))
                model->setData(model->index(index, 0), Qt::Checked, Qt::CheckStateRole);
            else
                model->setData(model->index(index, 0), Qt::Unchecked, Qt::CheckStateRole);
            ++index;
        }      
    }       
}


bool DlgGetNeighborFaces::eventFilter(QObject* target, QEvent* event)
{
    QTextEdit* textEdit = static_cast<QTextEdit*>(target);
    if (textEdit == ui->minRadius || textEdit == ui->maxRadius) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* k = static_cast<QKeyEvent*>(event);
            if (k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter)
            {
                on_selectFitButton_clicked();
                return true;
            }          
        }
    }
    return QWidget::eventFilter(target, event);
}

void DlgGetNeighborFaces::findShapes()
{
    App::Document* activeDoc = App::GetApplication().getActiveDocument();
    if (!activeDoc) return;
    std::vector<App::DocumentObject*> objs = activeDoc->getObjectsOfType(Part::Feature::getClassTypeId());
    int index = 1;
    int current_index = 0;
    for (auto it = objs.begin(); it != objs.end();++it, ++index) {
        ui->shapeObject->addItem(QString::fromUtf8((*it)->Label.getValue()));
        ui->shapeObject->setItemData(index, QString::fromLatin1((*it)->getNameInDocument()));
        if (current_index == 0) {
            if (Gui::Selection().isSelected(*it)) { 
                current_index = index;
            }
        }
    }
    // if only one object is in the document then simply use that
    if (objs.size() == 1)
        current_index = 1;

    if (current_index > 0) {
        ui->shapeObject->setCurrentIndex(current_index);
        on_shapeObject_activated(current_index);
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
DlgGetNeighborFaces::~DlgGetNeighborFaces()
{
    // no need to delete child widgets, Qt does it all for us
    d->connectApplicationDeletedDocument.disconnect();
    d->connectApplicationDeletedObject.disconnect();
    Gui::Selection().rmvSelectionGate();
}


void DlgGetNeighborFaces::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::LanguageChange) {
        int index = ui->shapeObject->currentIndex();
        // only get the items from index 1 on since the first one will be added automatically
        int count = ui->shapeObject->count() - 1;
        QStringList text;
        QList<QVariant> data;
        for (int i = 0; i < count; i++) {
            text << ui->shapeObject->itemText(i + 1);
            data << ui->shapeObject->itemData(i + 1);
        }

        ui->retranslateUi(this);
        for (int i = 0; i < count; i++) {
            ui->shapeObject->addItem(text.at(i));
            ui->shapeObject->setItemData(i + 1, data.at(i));
        }

        ui->shapeObject->setCurrentIndex(index);
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->treeView->model());
        count = model->rowCount();
        for (int i = 0; i < count; i++) {
            int id = model->data(model->index(i, 0), Qt::UserRole).toInt();
            model->setData(model->index(i, 0), QVariant(tr("Face%1").arg(id)));
        }
    }
    else {
        QWidget::changeEvent(e);
    }
}


const char* DlgGetNeighborFaces::getShapeType() const
{
    //return "Fillet";
    return "Faces";
}

bool DlgGetNeighborFaces::accept()
{      
    if (!d->object) {
        QMessageBox::warning(this, tr("No shape selected"),
                             tr("No valid shape is selected.\n"
                                "Please select a valid shape in the drop-down box first."));
        return false;
    }

    App::Document* activeDoc = App::GetApplication().getActiveDocument();
    QAbstractItemModel* model = ui->treeView->model();
    bool end_radius = !ui->treeView->isColumnHidden(2);
    bool todo = false;
    
    QString shape, type, name;
    //std::string shapeType = getShapeType();
    std::string shapeType = "Fillet";
    int index = ui->shapeObject->currentIndex();
    shape = ui->shapeObject->itemData(index).toString();
    type = QString::fromLatin1("Part::%1").arg(QString::fromLatin1(shapeType.c_str()));

    if (d->fillet)
        name = QString::fromLatin1(d->fillet->getNameInDocument());
    else
        name = QString::fromLatin1(activeDoc->getUniqueObjectName(shapeType.c_str()).c_str());

    activeDoc->openTransaction(shapeType.c_str());
    QString code;
    if (!d->fillet) {
        code = QString::fromLatin1("FreeCAD.ActiveDocument.addObject(\"%1\",\"%2\")\n"
                                   "FreeCAD.ActiveDocument.%2.Base = FreeCAD.ActiveDocument.%3\n")
                   .arg(type, name, shape);
    }
    code += QString::fromLatin1("__fillets__ = []\n");
    for (int i = 0; i < model->rowCount(); ++i) {
        QVariant value = model->index(i, 0).data(Qt::CheckStateRole);
        Qt::CheckState checkState = static_cast<Qt::CheckState>(value.toInt());
        // is item checked
        if (checkState & Qt::Checked) {
            // the index value of the face
            int id = model->index(i, 0).data(Qt::UserRole).toInt();
            Base::Quantity r1 = model->index(i, 1).data(Qt::EditRole).value<Base::Quantity>();
            Base::Quantity r2 = r1;
            if (end_radius)
                r2 = model->index(i, 2).data(Qt::EditRole).value<Base::Quantity>();
            code += QString::fromLatin1("__fillets__.append((%1,%2,%3))\n")
                        .arg(id)
                        .arg(r1.getValue(), 0, 'f', Base::UnitsApi::getDecimals())
                        .arg(r2.getValue(), 0, 'f', Base::UnitsApi::getDecimals());
            todo = true;
        }
    }

    if (!todo) {
        QMessageBox::warning(this, tr("No face selected"),
                             tr("No face entity is checked to select.\n"
                                "Please check one or more face entities first."));
        return false;
    }

    return true;



    Gui::WaitCursor wc;
    /*code += QString::fromLatin1("FreeCAD.ActiveDocument.%1.Edges = __fillets__\n"
                                "del __fillets__\n"
                                "FreeCADGui.ActiveDocument.%2.Visibility = False\n")
                .arg(name, shape);*/
    code += QString::fromLatin1("FreeCAD.ActiveDocument.%1.Faces = __fillets__\n"
                                "del __fillets__\n"
                                "FreeCADGui.ActiveDocument.%2.Visibility = False\n")
                .arg(name, shape);
    Gui::Command::runCommand(Gui::Command::App, code.toLatin1());
    activeDoc->commitTransaction();
    activeDoc->recompute();
    if (d->fillet) {
        Gui::ViewProvider* vp;
        vp = Gui::Application::Instance->getViewProvider(d->fillet);
        if (vp)
            vp->show();
    }
    QByteArray to = name.toLatin1();
    QByteArray from = shape.toLatin1();
    Gui::Command::copyVisual(to, "LineColor", from);
    Gui::Command::copyVisual(to, "PointColor", from);
    return true;
}


void DlgGetNeighborFaces::onDeleteObject(const App::DocumentObject& obj)
{
    if (d->fillet == &obj) {
        d->fillet = nullptr;
    }
    else if (d->fillet && d->fillet->Base.getValue() == &obj) {
        d->fillet = nullptr;
        d->object = nullptr;
        ui->shapeObject->setCurrentIndex(0);
        on_shapeObject_activated(0);
    }
    else if (d->object == &obj) {
        d->object = nullptr;
        ui->shapeObject->removeItem(ui->shapeObject->currentIndex());
        ui->shapeObject->setCurrentIndex(0);
        on_shapeObject_activated(0);
    }
    else {
        QString shape = QString::fromLatin1(obj.getNameInDocument());
        // start from the second item
        for (int i = 1; i < ui->shapeObject->count(); i++) {
            if (ui->shapeObject->itemData(i).toString() == shape) {
                ui->shapeObject->removeItem(i);
                break;
            }
        }
    }
}


void DlgGetNeighborFaces::onDeleteDocument(const App::Document& doc)
{
    if (d->object) {
        if (d->object->getDocument() == &doc) {
            ui->shapeObject->setCurrentIndex(0);
            on_shapeObject_activated(0);
            setEnabled(false);
        }
    }
    else if (App::GetApplication().getActiveDocument() == &doc) {
        ui->shapeObject->setCurrentIndex(0);
        on_shapeObject_activated(0);
        setEnabled(false);
    }
}




void DlgGetNeighborFaces::on_selectFitButton_clicked()
{
    std::vector<std::string> removeElements;
    std::vector<std::string> subElements;
    FilletRadiusModel* model = static_cast<FilletRadiusModel*>(ui->treeView->model());
    bool block = model->blockSignals(true);// do not call toggleCheckState

    double lowBound = ui->minRadius->toPlainText().toDouble();
    double highBound = ui->maxRadius->toPlainText().toDouble();   
    if (lowBound > highBound) {
#ifdef FC_DEBUG
        QMessageBox::question(this, tr("input Valid"), tr("input minRadius greater than maxRadius "));
#endif
        return;
    }
    for (int i = 0; i < model->rowCount(); ++i) {
        QModelIndex index = model->index(i, 0);
        // is not yet checked?
        QVariant check = index.data(Qt::CheckStateRole);
        Qt::CheckState state = static_cast<Qt::CheckState>(check.toInt());

        QModelIndex index1 = model->index(i, 1);
        QVariant radiusVar = model->data(index1, Qt::EditRole);
        double curRadius = radiusVar.value<Base::Quantity>().getValue();                   
        if (curRadius < lowBound || curRadius > highBound) {
            if (state == Qt::Checked) {
                int id = index.data(Qt::UserRole).toInt();
                std::stringstream str;
                str << "Face" << id;
                removeElements.push_back(str.str());
            }
            Qt::CheckState checkState = Qt::Unchecked;
            QVariant value(static_cast<int>(checkState));
            model->setData(index, value, Qt::CheckStateRole);                         
        }
        else {
            if (state == Qt::Unchecked) {
                int id = index.data(Qt::UserRole).toInt();
                std::stringstream str;
                str << "Face" << id;
                subElements.push_back(str.str());
            }
            Qt::CheckState checkState = Qt::Checked;
            QVariant value(static_cast<int>(checkState));
            model->setData(index, value, Qt::CheckStateRole);       
        }
    }
    model->blockSignals(block);
    model->updateCheckStates();
    model->layoutChanged();
    if (d->object) {
        App::Document* doc = d->object->getDocument();
        std::string nameDocument = doc->getName();
        std::string objNameInDocument = d->object->getNameInDocument();
        for (auto str : removeElements) {
            Gui::Selection().rmvSelection(nameDocument.c_str(), objNameInDocument.c_str(),str.c_str());
        }
        Gui::Selection().addSelections(nameDocument.c_str(), objNameInDocument.c_str(),subElements);
    }
}


void DlgGetNeighborFaces::on_selectAllButton_clicked()
{
  
    std::vector<std::string> subElements;
    FilletRadiusModel* model = static_cast<FilletRadiusModel*>(ui->treeView->model());
    
    bool block = model->blockSignals(true);// do not call toggleCheckState
    for (int i = 0; i < model->rowCount(); ++i) {
        QModelIndex index = model->index(i, 0);
        // is not yet checked?
        QVariant check = index.data(Qt::CheckStateRole);
        Qt::CheckState state = static_cast<Qt::CheckState>(check.toInt());
        if (state == Qt::Unchecked) {
            int id = index.data(Qt::UserRole).toInt();
            std::stringstream str;
            str << "Face" << id;
            subElements.push_back(str.str());
        }

        Qt::CheckState checkState = Qt::Checked;
        QVariant value(static_cast<int>(checkState));
        model->setData(index, value, Qt::CheckStateRole);
    }
    model->blockSignals(block);
    model->updateCheckStates();
    model->layoutChanged();
    if (d->object) {
        App::Document* doc = d->object->getDocument();
        std::string nameDocument = doc->getName();
        std::string objNameInDocument = d->object->getNameInDocument();
        Gui::Selection().addSelections(nameDocument.c_str(), objNameInDocument.c_str(), subElements);
    }   
}

void DlgGetNeighborFaces::on_selectNoneButton_clicked()
{
    FilletRadiusModel* model = static_cast<FilletRadiusModel*>(ui->treeView->model());
    bool block = model->blockSignals(true);// do not call toggleCheckState
    for (int i = 0; i < model->rowCount(); ++i) {
        Qt::CheckState checkState = Qt::Unchecked;
        QVariant value(static_cast<int>(checkState));
        model->setData(model->index(i, 0), value, Qt::CheckStateRole);
    }
    model->blockSignals(block);
    model->updateCheckStates();
    if (d->object) {
        App::Document* doc = d->object->getDocument();
        Gui::Selection().clearSelection(doc->getName());
    }   
}



TaskGetNeighborFaces::TaskGetNeighborFaces(Part::FilletBase* fillet)
{   
    widget = new DlgGetNeighborFaces(DlgGetNeighborFaces::ShapeType::Faces, fillet);
    taskbox = new Gui::TaskView::TaskBox(Gui::BitmapFactory().pixmap("CADSimplifier_GetNeighborFaces"),
                                         widget->windowTitle(), true, nullptr);
    taskbox->groupLayout()->addWidget(widget);
    Content.push_back(taskbox);
    //QPointer<QDialog> p = nullptr;
    //p=new QDialog(nullptr);
    //QVBoxLayout *layout=new QVBoxLayout(p);
    //QLineEdit* lineEdit = new QLineEdit(p);
    //layout->addWidget(lineEdit);
    //QLabel* label = new QLabel(p);
    //connect(lineEdit, &QLineEdit::returnPressed, [lineEdit,label]() {
    //    QString text = lineEdit->text();
    //    label->setText(QStringLiteral(text));
    //    });
    //layout->addWidget(label);
    //p->show();
    
}

TaskGetNeighborFaces::~TaskGetNeighborFaces()
{
    // automatically deleted in the sub-class
}

void TaskGetNeighborFaces::open() {}

void TaskGetNeighborFaces::clicked(int) {}

bool TaskGetNeighborFaces::accept()
{
    bool ok = widget->accept();
    if (ok)
        Gui::Command::doCommand(Gui::Command::Gui, "Gui.activeDocument().resetEdit()");
    return ok;
}

bool TaskGetNeighborFaces::reject()
{
    Gui::Command::doCommand(Gui::Command::Gui, "Gui.activeDocument().resetEdit()");
    return true;
}






#include "moc_DlgGetNeighborFaces.cpp"
