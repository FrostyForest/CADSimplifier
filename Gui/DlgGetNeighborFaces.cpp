#include "PreCompiled.h"
#ifndef _PreComp_
#include <QHeaderView>
#include <QItemDelegate>
#include <QItemSelectionModel>
#include <QLocale>
#include <QMessageBox>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <climits>
#include <sstream>

#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

#include <TopoDS_Shape.hxx>

#include <Inventor/actions/SoSearchAction.h>
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
#include <Mod/CADSimplifier/APP/SimplifierTool.h>
//#include "TopExp_Explorer.hxx"
#include "ui_DlgGetNeighborFaces.h"
#include "DlgGetNeighborFaces.h"

using namespace CADSimplifierGui;
namespace bp = boost::placeholders;

namespace CADSimplifierGui
{
class EdgeFaceSelection: public Gui::SelectionFilterGate
{
    bool allowEdge;
    App::DocumentObject*& object;

public:
    explicit EdgeFaceSelection(App::DocumentObject*& obj)
        : Gui::SelectionFilterGate(nullPointer()), allowEdge(true), object(obj)
    {}
    void selectEdges() { allowEdge = true; }
    void selectFaces() { allowEdge = false; }
    bool allow(App::Document* /*pDoc*/, App::DocumentObject* pObj, const char* sSubName) override
    {
        if (pObj != this->object)
            return false;
        if (!sSubName || sSubName[0] == '\0')
            return false;
        std::string element(sSubName);
        if (allowEdge)
            return element.substr(0, 4) == "Edge";
        else
            return element.substr(0, 4) == "Face";
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
    QStandardItemModel* model = new QStandardItemModel();//修改
    connect(model, SIGNAL(toggleCheckState(const QModelIndex&)), this,
            SLOT(toggleCheckState(const QModelIndex&)));
    model->insertColumns(0, 3);

    // timer for highlighting
    d->highlighttimer = new QTimer(this);
    d->highlighttimer->setSingleShot(true);
    connect(d->highlighttimer, SIGNAL(timeout()), this, SLOT(onHighlightEdges()));

    d->shapeType = type;
    if (d->shapeType == DlgGetNeighborFaces::Faces) {      
        ui->parameterName->setTitle(tr("NeighborFaces Parameter"));
        ui->labelfillet->setText(tr("Faces type"));
        model->setHeaderData(0, Qt::Horizontal, tr("NeighborFaces to Get"), Qt::DisplayRole);
        model->setHeaderData(1, Qt::Horizontal, tr("Start radius"), Qt::DisplayRole);
        model->setHeaderData(2, Qt::Horizontal, tr("End radius"), Qt::DisplayRole);
    }
    ui->treeView->setRootIsDecorated(false);
    //ui->treeView->setItemDelegate(new FilletRadiusDelegate(this));
    ui->treeView->setModel(model);

    QHeaderView* header = ui->treeView->header();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setDefaultAlignment(Qt::AlignLeft);
    header->setSectionsMovable(false);
    //on_shapeType_activated(0);
    findShapes();
}




void DlgGetNeighborFaces::on_shapeObject_activated(int itemPos)
{ 
    d->object = nullptr;
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->treeView->model());
    model->removeRows(0, model->rowCount());
            
    Gui::WaitCursor wc;
    QByteArray name = ui->shapeObject->itemData(itemPos).toByteArray();
    App::Document* doc = App::GetApplication().getActiveDocument();
    if (!doc)
        return;
    App::DocumentObject* docObj = doc->getObject((const char*)name);
    if (docObj && docObj->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId())) {
        d->object = docObj;

    
    ////测试
    ////获取当前的solid
    //TopoDS_Shape myShape = static_cast<Part::Feature*>(docObj)->Shape.getValue();
    //TopTools_IndexedDataMapOfShapeListOfShape aEdgeFaceMap;
    //TopExp::MapShapesAndAncestors(myShape, TopAbs_EDGE, TopAbs_FACE, aEdgeFaceMap);

    ////获取选中当前面的所有边
    //std::vector<Gui::SelectionObject> selection = Gui::Selection().getSelectionEx();
    //auto it = selection.begin();
    //App::DocumentObject* pActiveDoc = it->getObject();
    //Part::Feature* feat = static_cast<Part::Feature*>(pActiveDoc);
    //TopoDS_Shape sh = feat->Shape.getShape().getShape();
    //std::vector<std::string> subnames = it->getSubNames();
    //std::vector<TopoDS_Shape> selectedFaces;
    //for (std::vector<std::string>::iterator sub = subnames.begin(); sub != subnames.end(); ++sub) {
    //    TopoDS_Shape ref = feat->Shape.getShape().getSubShape(sub->c_str());
    //    selectedFaces.push_back(ref);
    //}

    //std::vector<TopoDS_Shape> selectedEdgesOfFaces;
    //TopExp_Explorer ex(selectedFaces.at(0), TopAbs_EDGE);
    //while (ex.More()) {
    //    selectedEdgesOfFaces.emplace_back(ex.Current());   
    //    ex.Next();
    //}
    //d->face_ids.clear();
    //Standard_Integer nE = aEdgeFaceMap.Extent();
    //for (Standard_Integer i = 1; i <= nE; i++)
    //{
    //    //边
    //    const TopoDS_Edge& E = TopoDS::Edge(aEdgeFaceMap.FindKey(i));
    //    auto it = std::find(selectedEdgesOfFaces.begin(), selectedEdgesOfFaces.end(), E);       
    //    if (it != selectedEdgesOfFaces.end()) {
    //        //获取此边共享的所有面的列表，即相邻面
    //        const TopTools_ListOfShape& aListOfFaces = aEdgeFaceMap.FindFromIndex(i);
    //        //面的操作
    //        d->face_ids.emplace_back(i);
    //    }
    //}


    TopoDS_Shape myShape = static_cast<Part::Feature*>(docObj)->Shape.getValue();
     // build up map edge->face  边和面的绑定bind
    TopTools_IndexedDataMapOfShapeListOfShape edge2Face;
    TopExp::MapShapesAndAncestors(myShape, TopAbs_EDGE, TopAbs_FACE,edge2Face);//添加进入的结构原型是TopAbs_FACE的
    TopTools_IndexedMapOfShape mapOfShape;
    TopExp::MapShapes(myShape, TopAbs_FACE, mapOfShape);
    //获取选中的面
    std::vector<TopoDS_Shape> selectedFaces;
    std::vector<Gui::SelectionObject> selection = Gui::Selection().getSelectionEx();
    auto it = selection.begin();
    App::DocumentObject* pActiveDoc = it->getObject();
    Part::Feature* feat = static_cast<Part::Feature*>(pActiveDoc);
    std::vector<std::string> subnames = it->getSubNames();
    for (std::vector<std::string>::iterator sub = subnames.begin(); sub != subnames.end();
            ++sub) {
        TopoDS_Shape ref = feat->Shape.getShape().getSubShape(sub->c_str());
        selectedFaces.emplace_back(ref);
    }
    const TopoDS_Shape& sh = selectedFaces.at(0);
    //QMessageBox::about(NULL, tr(""),tr(std::to_string(sh.ShapeType()).c_str()));
     for (int i = 1; i <= edge2Face.Extent(); ++i) {               
        const TopTools_ListOfShape& los = edge2Face.FindFromIndex(i);
        const TopoDS_Shape& face1 = los.First();
        const TopoDS_Shape& face2 = los.Last();       
        if ((los.Extent() == 2) && (sh.IsEqual(face1) || sh.IsEqual(face2))) {           
            const TopoDS_Shape& edge = edge2Face.FindKey(i);                       
            //2个面有交线相连
            Standard_Boolean flag1 = BRep_Tool::HasContinuity(
                TopoDS::Edge(edge), TopoDS::Face(face1), TopoDS::Face(face2));        
            Standard_Boolean flag = false;
            //2个面只有一个顶点相连
            if (flag1!=true) {
                TopExp_Explorer Ex(TopoDS::Face(face1), TopAbs_EDGE);
                TopExp_Explorer Ex1(TopoDS::Face(face2), TopAbs_EDGE);
                TopoDS_Vertex V1, V2, V3, V4;
                for (; Ex.More(); Ex.Next()) {
                    TopExp::Vertices(TopoDS::Edge(Ex.Current()), V1, V2);
                    for (; Ex1.More(); Ex1.Next()) {
                        TopExp::Vertices(TopoDS::Edge(Ex1.Current()), V3, V4);
                        if ((V1.IsSame(V3) ||V2.IsSame(V4)) || (V1.IsSame(V4) ||V2.IsSame(V3))) {
                            flag = true;
                            break;
                        }                  
                    }
                    if (flag)
                        break;
                }
            }
             if (flag1 || flag) {
                if (sh.IsEqual(face1)) {                  
                    int id = mapOfShape.FindIndex(face2);
                    d->face_ids.emplace_back(id);
                }
                else if (sh.IsEqual(face2)) {
                    int id = mapOfShape.FindIndex(face1);
                    d->face_ids.emplace_back(id);
                }
            }
        }
    }







       // TopTools_IndexedDataMapOfShapeListOfShape allFace;  
      /*  TopTools_IndexedMapOfShape allFace;
        tool.getAllFacesInSolid(name, allFace);       
        std::vector<TopoDS_Shape> selectedFaces;
        tool.getSelectedFaces(selectedFaces);      
        d->face_ids.clear();
        d->face_ids = tool.getAllNeighborFacesId(selectedFaces, allFace);  */     
        model->insertRows(0, d->face_ids.size());
        int index = 0;
        for (std::vector<int>::iterator it = d->face_ids.begin(); it != d->face_ids.end(); ++it) {
              
            model->setData(model->index(index, 0),
                            QVariant(tr("Face%1").arg(*it)));
                
            model->setData(model->index(index, 0), QVariant(*it), Qt::UserRole);
            model->setData(
                model->index(index, 1),
                QVariant::fromValue<Base::Quantity>(Base::Quantity(1.0, Base::Unit::Length)));
            model->setData(
                model->index(index, 2),
                QVariant::fromValue<Base::Quantity>(Base::Quantity(1.0, Base::Unit::Length)));
            std::stringstream element;
            element << "Face" << *it;
            if (Gui::Selection().isSelected(docObj, element.str().c_str()))
                model->setData(model->index(index, 0), Qt::Checked, Qt::CheckStateRole);
            else
                model->setData(model->index(index, 0), Qt::Unchecked, Qt::CheckStateRole);
            index++;
        }
    }    
}


void DlgGetNeighborFaces::findShapes()
{
    App::Document* activeDoc = App::GetApplication().getActiveDocument();
    if (!activeDoc)
        return;
    std::vector<App::DocumentObject*> objs =
        activeDoc->getObjectsOfType(Part::Feature::getClassTypeId());
    int index = 1;
    int current_index = 0;
    for (std::vector<App::DocumentObject*>::iterator it = objs.begin(); it != objs.end();
         ++it, ++index) {
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
            model->setData(model->index(i, 0), QVariant(tr("Edge%1").arg(id)));
        }
    }
    else {
        QWidget::changeEvent(e);
    }
}


const char* DlgGetNeighborFaces::getShapeType() const { return "Faces"; }

bool DlgGetNeighborFaces::accept()
{   
    return true;
}



void DlgGetNeighborFaces::setupGetNeighborFaces(const std::vector<App::DocumentObject*>& objs)
{
      
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
void DlgGetNeighborFaces::on_selectAllButton_clicked()
{
   
}

void DlgGetNeighborFaces::on_selectNoneButton_clicked()
{
    
}



TaskGetNeighborFaces::TaskGetNeighborFaces(Part::FilletBase* fillet)
{   
    widget = new DlgGetNeighborFaces(DlgGetNeighborFaces::ShapeType::Faces, fillet);
    taskbox = new Gui::TaskView::TaskBox(Gui::BitmapFactory().pixmap("CADSimplifier_GetNeighborFaces"),
                                         widget->windowTitle(), true, nullptr);
    taskbox->groupLayout()->addWidget(widget);
    Content.push_back(taskbox);
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
