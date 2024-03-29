/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 8P41EB015898

  ==============================================================================*/

// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerPathExplorerModuleWidget.h"
#include "ui_qSlicerPathExplorerModuleWidget.h"

// Annotation logic
#include "vtkSlicerAnnotationModuleLogic.h"

// Slicer
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerPathExplorerFiducialItem.h"
#include "qSlicerPathExplorerTrajectoryItem.h"
#include "qSlicerPathExplorerReslicingWidget.h"

// MRML
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLPathPlannerTrajectoryNode.h"
#include "vtkMRMLSliceNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPathExplorerModuleWidgetPrivate: public Ui_qSlicerPathExplorerModuleWidget
{
public:
  qSlicerPathExplorerModuleWidgetPrivate();

  vtkMRMLPathPlannerTrajectoryNode *selectedTrajectoryNode;
  double targetTableWidgetItemColor[3];
  double entryTableWidgetItemColor[3];
  typedef std::vector<qSlicerPathExplorerReslicingWidget*> ReslicerVector;
  ReslicerVector reslicerList;
};

//-----------------------------------------------------------------------------
// qSlicerPathExplorerModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerPathExplorerModuleWidgetPrivate::
qSlicerPathExplorerModuleWidgetPrivate()
{
  this->selectedTrajectoryNode = NULL;

  this->targetTableWidgetItemColor[0] = 68;
  this->targetTableWidgetItemColor[1] = 172;
  this->targetTableWidgetItemColor[2] = 100;

  this->entryTableWidgetItemColor[0] = 79;
  this->entryTableWidgetItemColor[1] = 148;
  this->entryTableWidgetItemColor[2] = 205;
}

//-----------------------------------------------------------------------------
// qSlicerPathExplorerModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerPathExplorerModuleWidget::
qSlicerPathExplorerModuleWidget(QWidget* _parent)
  : Superclass( _parent )
    , d_ptr( new qSlicerPathExplorerModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerPathExplorerModuleWidget::
~qSlicerPathExplorerModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
setup()
{
  Q_D(qSlicerPathExplorerModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Entry table widget
  connect(d->EntryPointListNodeSelector, SIGNAL(nodeActivated(vtkMRMLNode*)),
          this, SLOT(onEntryListNodeChanged(vtkMRMLNode*)));

  connect(d->EntryPointWidget->getTableWidget(), SIGNAL(itemSelectionChanged()),
          this, SLOT(onEntrySelectionChanged()));

  connect(d->EntryPointWidget, SIGNAL(itemDeleted(vtkMRMLAnnotationFiducialNode*)),
          this, SLOT(onEntryPointDeleted(vtkMRMLAnnotationFiducialNode*)));

  connect(d->EntryPointWidget, SIGNAL(addButtonToggled(bool)),
          this, SLOT(onEntryTableWidgetAddButtonToggled(bool)));

  std::stringstream entryBackgroundColor;
  entryBackgroundColor << "selection-background-color: rgba("
                       << d->entryTableWidgetItemColor[0] << ","
                       << d->entryTableWidgetItemColor[1] << ","
                       << d->entryTableWidgetItemColor[2] << ");";
  d->EntryPointWidget->getTableWidget()->setStyleSheet(entryBackgroundColor.str().c_str());

  // Target table widget
  connect(d->TargetPointListNodeSelector, SIGNAL(nodeActivated(vtkMRMLNode*)),
          this, SLOT(onTargetListNodeChanged(vtkMRMLNode*)));

  connect(d->TargetPointWidget->getTableWidget(), SIGNAL(itemSelectionChanged()),
          this, SLOT(onTargetSelectionChanged()));

  connect(d->TargetPointWidget, SIGNAL(itemDeleted(vtkMRMLAnnotationFiducialNode*)),
          this, SLOT(onTargetPointDeleted(vtkMRMLAnnotationFiducialNode*)));

  connect(d->TargetPointWidget, SIGNAL(addButtonToggled(bool)),
          this, SLOT(onTargetTableWidgetAddButtonToggled(bool)));

  std::stringstream targetBackgroundColor;
  targetBackgroundColor << "selection-background-color: rgba("
                        << d->targetTableWidgetItemColor[0] << ","
                        << d->targetTableWidgetItemColor[1] << ","
                        << d->targetTableWidgetItemColor[2] << ");";
  d->TargetPointWidget->getTableWidget()->setStyleSheet(targetBackgroundColor.str().c_str());

  // Trajectory table widget
  connect(d->TrajectoryListNodeSelector, SIGNAL(nodeActivated(vtkMRMLNode*)),
          this, SLOT(onTrajectoryListNodeChanged(vtkMRMLNode*)));

  connect(d->AddButton, SIGNAL(clicked()),
          this, SLOT(onAddButtonClicked()));

  connect(d->DeleteButton, SIGNAL(clicked()),
          this, SLOT(onDeleteButtonClicked()));

  connect(d->UpdateButton, SIGNAL(clicked()),
          this, SLOT(onUpdateButtonClicked()));

  connect(d->ClearButton, SIGNAL(clicked()),
          this, SLOT(onClearButtonClicked()));

  connect(d->TrajectoryTableWidget->selectionModel(),
          SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
          this,
          SLOT(onTrajectorySelectionChanged(const QItemSelection&, const QItemSelection&)));

  connect(d->TrajectoryTableWidget, SIGNAL(cellChanged(int,int)),
          this, SLOT(onTrajectoryCellChanged(int,int)));

  // mrmlScene
  connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
          this, SLOT(onMRMLSceneChanged(vtkMRMLScene*)));


}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onEntryListNodeChanged(vtkMRMLNode* newList)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!newList || !d->EntryPointWidget)
    {
    return;
    }

  vtkMRMLAnnotationHierarchyNode* entryList =
    vtkMRMLAnnotationHierarchyNode::SafeDownCast(newList);

  if (!entryList)
    {
    return;
    }

  // Observe new hierarchy node
  qvtkConnect(entryList, vtkMRMLAnnotationHierarchyNode::ChildNodeAddedEvent,
              this, SLOT(refreshEntryView()));
  qvtkConnect(entryList, vtkMRMLAnnotationHierarchyNode::ChildNodeRemovedEvent,
              this, SLOT(refreshEntryView()));

  // Update groupbox name
  std::stringstream groupBoxName;
  groupBoxName << "Entry Point : " << entryList->GetName();
  d->EntryGroupBox->setTitle(groupBoxName.str().c_str());

  // Update widget
  d->EntryPointWidget->setSelectedHierarchyNode(entryList);

  // Refresh view
  this->refreshEntryView();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onTargetListNodeChanged(vtkMRMLNode* newList)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!newList || !d->TargetPointWidget)
    {
    return;
    }

  vtkMRMLAnnotationHierarchyNode* targetList =
    vtkMRMLAnnotationHierarchyNode::SafeDownCast(newList);

  if (!targetList)
    {
    return;
    }

  // Observe new hierarchy node
  qvtkConnect(targetList, vtkMRMLAnnotationHierarchyNode::ChildNodeAddedEvent,
              this, SLOT(refreshTargetView()));
  qvtkConnect(targetList, vtkMRMLAnnotationHierarchyNode::ChildNodeRemovedEvent,
              this, SLOT(refreshTargetView()));

  // Update groupbox name
  std::stringstream groupBoxName;
  groupBoxName << "Target Point : " << targetList->GetName();
  d->TargetGroupBox->setTitle(groupBoxName.str().c_str());

  // Update widget
  d->TargetPointWidget->setSelectedHierarchyNode(targetList);

  // Refresh view
  this->refreshTargetView();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
addNewFiducialItem(QTableWidget* tableWidget, vtkMRMLAnnotationFiducialNode* fiducialNode)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!tableWidget || !fiducialNode)
    {
    return;
    }

  // Set fiducial properties
  // Opacity: 0.3 by default
  fiducialNode->GetAnnotationPointDisplayNode()->SetOpacity(0.3);

  // Set fiducial name and define background color
  QColor* newColor = new QColor();
  if (tableWidget == d->TargetPointWidget->getTableWidget())
    {
    std::stringstream targetName;
    targetName << "T" << d->TargetPointWidget->getTableWidget()->rowCount()+1;
    fiducialNode->SetName(targetName.str().c_str());
    newColor->setRgb(d->targetTableWidgetItemColor[0],
                     d->targetTableWidgetItemColor[1],
                     d->targetTableWidgetItemColor[2],
                     180);
    }
  else if (tableWidget == d->EntryPointWidget->getTableWidget())
    {
    std::stringstream entryName;
    entryName << "E" << d->EntryPointWidget->getTableWidget()->rowCount()+1;
    fiducialNode->SetName(entryName.str().c_str());
    newColor->setRgb(d->entryTableWidgetItemColor[0],
                     d->entryTableWidgetItemColor[1],
                     d->entryTableWidgetItemColor[2],
                     180);
    }

  int numberOfItems = tableWidget->rowCount();
  tableWidget->insertRow(numberOfItems);

  // Add row
  qSlicerPathExplorerFiducialItem* newItem =
    new qSlicerPathExplorerFiducialItem();
  tableWidget->setItem(numberOfItems, 0, newItem);
  tableWidget->setItem(numberOfItems, 1, new QTableWidgetItem());
  tableWidget->setItem(numberOfItems, 2, new QTableWidgetItem());
  tableWidget->setItem(numberOfItems, 3, new QTableWidgetItem());
  tableWidget->setItem(numberOfItems, 4, new QTableWidgetItem());
  newItem->setFiducialNode(fiducialNode);

  // Set background color
  for (int i = 0; i < 5; ++i)
    {
    tableWidget->item(numberOfItems, i)->setBackground(QBrush(*newColor));
    }

  // Automatic scroll and select last item added
  tableWidget->scrollToItem(tableWidget->item(numberOfItems,1));
  tableWidget->setCurrentCell(tableWidget->rowCount()-1,0);

  // Update item if changed
  connect(tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
          this, SLOT(onItemChanged(QTableWidgetItem*)));
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onItemChanged(QTableWidgetItem *item)
{
  if (!item)
    {
    return;
    }

  qSlicerPathExplorerFiducialItem *itemChanged
    = dynamic_cast<qSlicerPathExplorerFiducialItem*>(item);

  if (!itemChanged)
    {
    return;
    }

  itemChanged->updateItem();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
refreshEntryView()
{
  Q_D(qSlicerPathExplorerModuleWidget);

  vtkMRMLAnnotationHierarchyNode* entryList =
    d->EntryPointWidget->selectedHierarchyNode();

  if (!entryList)
    {
    return;
    }

  // Clear entry table widget
  d->EntryPointWidget->getTableWidget()->clearContents();
  d->EntryPointWidget->getTableWidget()->setRowCount(0);

  // Populate table with new items in the new list
  for(int i = 0; i < entryList->GetNumberOfChildrenNodes(); i++)
    {
    vtkMRMLAnnotationFiducialNode* fiducialPoint =
      vtkMRMLAnnotationFiducialNode::SafeDownCast(entryList->GetNthChildNode(i)->GetAssociatedNode());
    if (fiducialPoint)
      {
      // Set color to blue for entry points
      fiducialPoint->GetAnnotationPointDisplayNode()->SetColor(0,0,1);

      // Add fiducial
      this->addNewFiducialItem(d->EntryPointWidget->getTableWidget(), fiducialPoint);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
refreshTargetView()
{
  Q_D(qSlicerPathExplorerModuleWidget);

  vtkMRMLAnnotationHierarchyNode* targetList =
    d->TargetPointWidget->selectedHierarchyNode();

  if (!targetList)
    {
    return;
    }

  // Clear target table widget
  d->TargetPointWidget->getTableWidget()->clearContents();
  d->TargetPointWidget->getTableWidget()->setRowCount(0);

  // Populate table with new items in the new list
  for(int i = 0; i < targetList->GetNumberOfChildrenNodes(); i++)
    {
    vtkMRMLAnnotationFiducialNode* fiducialPoint =
      vtkMRMLAnnotationFiducialNode::SafeDownCast(targetList->GetNthChildNode(i)->GetAssociatedNode());
    if (fiducialPoint)
      {
      // Set color to blue for entry points
      fiducialPoint->GetAnnotationPointDisplayNode()->SetColor(0,1,0);

      // Add fiducial
      this->addNewFiducialItem(d->TargetPointWidget->getTableWidget(), fiducialPoint);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onTrajectoryListNodeChanged(vtkMRMLNode* newList)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!newList)
    {
    return;
    }

  vtkMRMLPathPlannerTrajectoryNode* trajectoryList =
    vtkMRMLPathPlannerTrajectoryNode::SafeDownCast(newList);

  if (!trajectoryList)
    {
    return;
    }

  // Update selected node
  d->selectedTrajectoryNode = trajectoryList;

  // Clear widget
  d->TrajectoryTableWidget->clearContents();
  d->TrajectoryTableWidget->setRowCount(0);

  // TODO: Populate table with trajectory in new node
  // How to know which fiducials have been used to create ruler ?
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onAddButtonClicked()
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!d->EntryPointWidget || !d->TargetPointWidget ||
      !d->TrajectoryTableWidget)
    {
    return;
    }

  if (!d->EntryPointWidget->getTableWidget() ||
      !d->TargetPointWidget->getTableWidget())
    {
    return;
    }

  // Get target position
  double targetRow = d->TargetPointWidget->getTableWidget()->currentRow();
  if (targetRow < 0)
    {
    return;
    }

  qSlicerPathExplorerFiducialItem* targetItem
    = dynamic_cast<qSlicerPathExplorerFiducialItem*>(d->TargetPointWidget->getTableWidget()->item(targetRow, 0));
  if (!targetItem)
    {
    return;
    }

  vtkMRMLAnnotationFiducialNode* targetFiducial
    = targetItem->getFiducialNode();
  if (!targetFiducial)
    {
    return;
    }

  // Get entry position
  double entryRow = d->EntryPointWidget->getTableWidget()->currentRow();
  if (entryRow < 0)
    {
    return;
    }

  qSlicerPathExplorerFiducialItem* entryItem
    = dynamic_cast<qSlicerPathExplorerFiducialItem*>(d->EntryPointWidget->getTableWidget()->item(entryRow, 0));
  if (!entryItem)
    {
    return;
    }

  vtkMRMLAnnotationFiducialNode* entryFiducial
    = entryItem->getFiducialNode();
  if (!entryFiducial)
    {
    return;
    }

  // Add new ruler
  this->addNewRulerItem(entryFiducial, targetFiducial);

  // Automatically select last trajectory created
  double rowCount = d->TrajectoryTableWidget->rowCount();
  d->TrajectoryTableWidget->selectRow(rowCount-1);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onDeleteButtonClicked()
{
  Q_D(qSlicerPathExplorerModuleWidget);

  int selectedRow = d->TrajectoryTableWidget->currentRow();
  if (selectedRow < 0)
    {
    return;
    }

  this->deleteTrajectory(selectedRow);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
deleteTrajectory(int trajectoryRow)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!this->mrmlScene())
    {
    return;
    }

  // Remove ruler from scene
  qSlicerPathExplorerTrajectoryItem* itemToRemove =
    dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(trajectoryRow,0));
  if (itemToRemove)
    {
    this->mrmlScene()->RemoveNode(itemToRemove->trajectoryNode());
    }

  // BUG: ModifiedEvent seems not to be triggered when removing a child node of hierarchy
  // Workaround: Call it manually
  qSlicerAbstractCoreModule* annotationModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Annotations");
  vtkSlicerAnnotationModuleLogic* annotationLogic;
  if (annotationModule)
    {
    annotationLogic =
      vtkSlicerAnnotationModuleLogic::SafeDownCast(annotationModule->logic());
    }
  annotationLogic->GetActiveHierarchyNode()->Modified();

  // Remove from widget
  d->TrajectoryTableWidget->removeRow(trajectoryRow);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onUpdateButtonClicked()
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!d->TrajectoryTableWidget)
    {
    return;
    }

  if (!d->TargetPointWidget ||
      !d->EntryPointWidget)
    {
    return;
    }

  QTableWidget* targetTable = d->TargetPointWidget->getTableWidget();
  QTableWidget* entryTable = d->EntryPointWidget->getTableWidget();

  if (!targetTable ||
      !entryTable)
    {
    return;
    }

  // Get selected rows
  int trajectoryRow = d->TrajectoryTableWidget->currentRow();
  int targetRow = targetTable->currentRow();
  int entryRow = entryTable->currentRow();

  if ((trajectoryRow < 0) ||
      (targetRow < 0) ||
      (entryRow < 0))
    {
    return;
    }

  // Get items
  qSlicerPathExplorerFiducialItem *targetItem =
    dynamic_cast<qSlicerPathExplorerFiducialItem*>(targetTable->item(targetRow,0));
  qSlicerPathExplorerFiducialItem *entryItem =
    dynamic_cast<qSlicerPathExplorerFiducialItem*>(entryTable->item(entryRow,0));
  qSlicerPathExplorerTrajectoryItem *trajectoryItem =
    dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(trajectoryRow,0));

  if (!targetItem ||
      !entryItem ||
      !trajectoryItem)
    {
    return;
    }

  // Update trajectory
  if (trajectoryItem->entryPoint() != entryItem->getFiducialNode())
    {
    trajectoryItem->setEntryPoint(entryItem->getFiducialNode());
    }

  if (trajectoryItem->targetPoint() != targetItem->getFiducialNode())
    {
    trajectoryItem->setTargetPoint(targetItem->getFiducialNode());
    }

  // Update trajectory name
  std::stringstream trajectoryName;
  trajectoryName << trajectoryItem->entryPoint()->GetName() << trajectoryItem->targetPoint()->GetName();
  trajectoryItem->setText(trajectoryName.str().c_str());

  d->UpdateButton->setEnabled(0);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onClearButtonClicked()
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!d->selectedTrajectoryNode)
    {
    return;
    }

  int numberOfRows = d->TrajectoryTableWidget->rowCount();
  for (int i = 0; i < numberOfRows; i++)
    {
    this->deleteTrajectory(0);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
addNewRulerItem(vtkMRMLAnnotationFiducialNode* entryPoint, vtkMRMLAnnotationFiducialNode* targetPoint)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!entryPoint || !targetPoint)
    {
    return;
    }

  if (!d->selectedTrajectoryNode)
    {
    return;
    }

  double rowCount = d->TrajectoryTableWidget->rowCount();

  // Check ruler not already existing
  for (int i = 0; i < rowCount; i++)
    {
    qSlicerPathExplorerTrajectoryItem* currentItem =
      dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(i,0));
    if (currentItem)
      {
      if (currentItem->entryPoint() == entryPoint)
        {
        if (currentItem->targetPoint() == targetPoint)
          {
          // Trajectory alread exists
          return;
          }
        }
      }
    }

  // Set active hierachy node
  qSlicerAbstractCoreModule* annotationModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Annotations");
  vtkSlicerAnnotationModuleLogic* annotationLogic;
  if (annotationModule)
    {
    annotationLogic =
      vtkSlicerAnnotationModuleLogic::SafeDownCast(annotationModule->logic());
    }

  if (annotationLogic->GetActiveHierarchyNode() != d->selectedTrajectoryNode)
    {
    annotationLogic->SetActiveHierarchyNodeID(d->selectedTrajectoryNode->GetID());
    }

  // Insert new row
  d->TrajectoryTableWidget->insertRow(rowCount);

  // Populate row
  QTableWidgetItem* checkboxItem = new QTableWidgetItem();
  checkboxItem->setCheckState(Qt::Checked);

  qSlicerPathExplorerTrajectoryItem* newTrajectory = new qSlicerPathExplorerTrajectoryItem();
  d->TrajectoryTableWidget->setItem(rowCount, 0, newTrajectory);
  d->TrajectoryTableWidget->setItem(rowCount, 1, new QTableWidgetItem());
  d->TrajectoryTableWidget->setItem(rowCount, 2, new QTableWidgetItem());
  d->TrajectoryTableWidget->setItem(rowCount, 3, checkboxItem);

  newTrajectory->setEntryPoint(entryPoint);
  newTrajectory->setTargetPoint(targetPoint);

  // Set only name editable
  d->TrajectoryTableWidget->item(rowCount,1)->setFlags(d->TrajectoryTableWidget->item(rowCount,1)->flags() & ~Qt::ItemIsEditable);
  d->TrajectoryTableWidget->item(rowCount,2)->setFlags(d->TrajectoryTableWidget->item(rowCount,2)->flags() & ~Qt::ItemIsEditable);

  // Automatic scroll to last item added
  d->TrajectoryTableWidget->scrollToItem(d->TrajectoryTableWidget->item(rowCount,0));

  // Update trajectory name
  std::stringstream trajectoryName;
  trajectoryName << entryPoint->GetName() << targetPoint->GetName();
  newTrajectory->setText(trajectoryName.str().c_str());
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onTrajectorySelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_D(qSlicerPathExplorerModuleWidget);
  Q_UNUSED(deselected);
  Q_UNUSED(selected);

  if (!d->TrajectoryTableWidget ||
      !d->EntryPointWidget ||
      !d->TargetPointWidget)
    {
    return;
    }

  int row = d->TrajectoryTableWidget->currentRow();

  //  Get item selected
  qSlicerPathExplorerTrajectoryItem* selectedTrajectory
    = dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(row,0));
  if (!selectedTrajectory)
    {
    return;
    }

  // Find target point
  vtkMRMLAnnotationFiducialNode* targetFiducial =
    selectedTrajectory->targetPoint();
  if (targetFiducial)
    {
    for (int i = 0; i < d->TargetPointWidget->getTableWidget()->rowCount(); i++)
      {
      qSlicerPathExplorerFiducialItem* currentFiducial =
        dynamic_cast<qSlicerPathExplorerFiducialItem*>(d->TargetPointWidget->getTableWidget()->item(i,0));
      if (currentFiducial)
        {
        if (currentFiducial->getFiducialNode() == targetFiducial)
          {
          // Target found. Select it.
          d->TargetPointWidget->getTableWidget()->setCurrentCell(i,0);
          break;
          }
        }
      }
    }

  // Find entry point
  vtkMRMLAnnotationFiducialNode* entryFiducial =
    selectedTrajectory->entryPoint();
  if (entryFiducial)
    {
    for (int i = 0; i < d->EntryPointWidget->getTableWidget()->rowCount(); i++)
      {
      qSlicerPathExplorerFiducialItem* currentFiducial =
        dynamic_cast<qSlicerPathExplorerFiducialItem*>(d->EntryPointWidget->getTableWidget()->item(i,0));
      if (currentFiducial)
        {
        if (currentFiducial->getFiducialNode() == entryFiducial)
          {
          // Entry found. Select it.
          d->EntryPointWidget->getTableWidget()->setCurrentCell(i,0);
          break;
          }
        }
      }
    }

  // Set trajectory items to all reslicer widgets
  for (qSlicerPathExplorerModuleWidgetPrivate::ReslicerVector::iterator it = d->reslicerList.begin();
       it != d->reslicerList.end(); ++it)
    {
    qSlicerPathExplorerReslicingWidget* currentReslicer
      = *it;
    if (currentReslicer)
      {
      currentReslicer->setTrajectoryItem(selectedTrajectory);
      }
    }
}


//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onMRMLSceneChanged(vtkMRMLScene* newScene)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!d->TargetPointListNodeSelector ||
      !d->EntryPointListNodeSelector ||
      !d->TrajectoryListNodeSelector)
    {
    return;
    }

  // Get Annotation Logic
  qSlicerAbstractCoreModule* annotationModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Annotations");
  vtkSlicerAnnotationModuleLogic* annotationLogic;
  if (annotationModule)
    {
    annotationLogic =
      vtkSlicerAnnotationModuleLogic::SafeDownCast(annotationModule->logic());
    if (annotationLogic)
      {
      const char* topLevelID = annotationLogic->GetTopLevelHierarchyNodeID();

      // Create Target Hierarchy Node
      annotationLogic->AddHierarchy();
      vtkMRMLAnnotationHierarchyNode* targetHierarchy
        = annotationLogic->GetActiveHierarchyNode();
      if (targetHierarchy)
        {
        targetHierarchy->SetName("Target List");
        d->TargetPointListNodeSelector->setCurrentNode(targetHierarchy);
        }
      annotationLogic->SetActiveHierarchyNodeID(topLevelID);

      // Create Entry Hierarchy Node
      annotationLogic->AddHierarchy();
      vtkMRMLAnnotationHierarchyNode* entryHierarchy
        = annotationLogic->GetActiveHierarchyNode();
      if (entryHierarchy)
        {
        entryHierarchy->SetName("Entry List");
        d->EntryPointListNodeSelector->setCurrentNode(entryHierarchy);
        }
      }
    }

  // Create new PathPlannerTrajectory Node
  d->TrajectoryListNodeSelector->addNode();

  // Clear reslicing widget layout
  QLayoutItem* item;
  while ( ( item = d->ReslicingWidgetLayout->takeAt(0)) != NULL)
    {
    delete item->widget();
    delete item;
    }

  // Add reslicing widgets
  vtkMRMLSliceNode* redViewer = vtkMRMLSliceNode::SafeDownCast(newScene->GetNodeByID("vtkMRMLSliceNodeRed"));
  this->addNewReslicer(redViewer);
  vtkMRMLSliceNode* yellowViewer = vtkMRMLSliceNode::SafeDownCast(newScene->GetNodeByID("vtkMRMLSliceNodeYellow"));
  this->addNewReslicer(yellowViewer);
  vtkMRMLSliceNode* greenViewer = vtkMRMLSliceNode::SafeDownCast(newScene->GetNodeByID("vtkMRMLSliceNodeGreen"));
  this->addNewReslicer(greenViewer);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
addNewReslicer(vtkMRMLSliceNode* sliceNode)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (sliceNode)
    {
    qSlicerPathExplorerReslicingWidget* reslicer =
      new qSlicerPathExplorerReslicingWidget(sliceNode, d->CollapsibleButton);
    if (reslicer)
      {
      d->ReslicingWidgetLayout->addWidget(reslicer);
      d->reslicerList.push_back(reslicer);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onTargetSelectionChanged()
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!d->TargetPointWidget->getTableWidget() ||
      !d->TrajectoryTableWidget)
    {
    return;
    }

  // Check if same fiducial
  int targetRow = d->TargetPointWidget->getTableWidget()->currentRow();
  int trajectoryRow = d->TrajectoryTableWidget->currentRow();

  // Get nodes
  qSlicerPathExplorerFiducialItem* targetItem =
    dynamic_cast<qSlicerPathExplorerFiducialItem*>(d->TargetPointWidget->getTableWidget()->item(targetRow,0));
  qSlicerPathExplorerTrajectoryItem* trajectoryItem =
    dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(trajectoryRow,0));

  if (targetItem && trajectoryItem)
    {
    if (targetItem->getFiducialNode() == trajectoryItem->targetPoint())
      {
      // Same. No update.
      d->UpdateButton->setEnabled(0);
      }
    else
      {
      // Different. Allow update.
      d->UpdateButton->setEnabled(1);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onEntrySelectionChanged()
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!d->EntryPointWidget->getTableWidget() ||
      !d->TrajectoryTableWidget)
    {
    return;
    }

  // Check if same fiducial
  int entryRow = d->EntryPointWidget->getTableWidget()->currentRow();
  int trajectoryRow = d->TrajectoryTableWidget->currentRow();

  // Get nodes
  qSlicerPathExplorerFiducialItem* entryItem =
    dynamic_cast<qSlicerPathExplorerFiducialItem*>(d->EntryPointWidget->getTableWidget()->item(entryRow,0));
  qSlicerPathExplorerTrajectoryItem* trajectoryItem =
    dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(trajectoryRow,0));

  if (entryItem && trajectoryItem)
    {
    if (entryItem->getFiducialNode() == trajectoryItem->entryPoint())
      {
      // Same. No update.
      d->UpdateButton->setEnabled(0);
      }
    else
      {
      // Different. Allow update.
      d->UpdateButton->setEnabled(1);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onTrajectoryCellChanged(int row, int column)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!d->TrajectoryTableWidget || !d->selectedTrajectoryNode ||
      !this->mrmlScene())
    {
    return;
    }

  // -- Trajectory name changed
  if (column == 0)
    {
    // Get item
    qSlicerPathExplorerTrajectoryItem* currentItem =
      dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(row,0));
    if (!currentItem)
      {
      return;
      }

    // Get ruler
    vtkMRMLAnnotationRulerNode* currentRuler =
      currentItem->trajectoryNode();
    if (!currentRuler)
      {
      return;
      }

    // VisuaLine compatibility
    std::stringstream offsetName;
    offsetName << currentRuler->GetName() << "_offset";
    vtkCollection* visuaLineOffsetNodes =
      this->mrmlScene()->GetNodesByClassByName("vtkMRMLAnnotationRulerNode",offsetName.str().c_str());

    // Set new name
    currentRuler->SetName(currentItem->text().toStdString().c_str());

    // Rename offset for VisuaLine compatibility
    int numberOfNodes = visuaLineOffsetNodes->GetNumberOfItems();
    if (numberOfNodes > 0)
      {
      vtkMRMLNode* offsetNode =
        vtkMRMLNode::SafeDownCast(visuaLineOffsetNodes->GetItemAsObject(numberOfNodes-1));

      if (offsetNode)
        {
        // Clear stringstream
        offsetName.str(std::string());
        offsetName.clear();

        // New name
        offsetName << currentRuler->GetName() << "_offset";
        offsetNode->SetName(offsetName.str().c_str());
        }
      }

    // Update hierarchy node
    d->selectedTrajectoryNode->Modified();

    visuaLineOffsetNodes->Delete();
    }

  // -- Trajectory visibility changed
  else if (column == 3)
    {
    qSlicerPathExplorerTrajectoryItem* currentItem =
      dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(row,0));
    if (!currentItem)
      {
      return;
      }

    bool checked = d->TrajectoryTableWidget->item(row, 3)->checkState() == Qt::Checked;
    currentItem->setDisplayPath(checked);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onEntryPointDeleted(vtkMRMLAnnotationFiducialNode* itemDeleted)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!d->TrajectoryTableWidget)
    {
    return;
    }

  int rowCount = d->TrajectoryTableWidget->rowCount();
  if (rowCount <= 0)
    {
    return;
    }

  int row = 0;
  while(row < d->TrajectoryTableWidget->rowCount())
    {
    qSlicerPathExplorerTrajectoryItem* currentItem =
      dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(row,0));
    if (currentItem)
      {
      if (currentItem->entryPoint() == itemDeleted)
        {
        // Remove trajectory
        this->deleteTrajectory(row);
        }
      else
        {
        // Only increment row here, because when removing row 6 for example
        // row 7 will become row 6, so we have to test row 6 again, and so on...
        row++;
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onTargetPointDeleted(vtkMRMLAnnotationFiducialNode* itemDeleted)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!d->TrajectoryTableWidget)
    {
    return;
    }

  int rowCount = d->TrajectoryTableWidget->rowCount();
  if (rowCount <= 0)
    {
    return;
    }

  int row = 0;
  while(row < d->TrajectoryTableWidget->rowCount())
    {
    qSlicerPathExplorerTrajectoryItem* currentItem =
      dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(row,0));
    if (currentItem)
      {
      if (currentItem->targetPoint() == itemDeleted)
        {
        // Remove trajectory
        this->deleteTrajectory(row);
        }
      else
        {
        // Only increment row here, because when removing row 6 for example
        // row 7 will become row 6, so we have to test row 6 again, and so on...
        row++;
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onEntryDisplayModified(vtkMRMLAnnotationFiducialNode* modifiedNode, bool visibility)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!d->TrajectoryTableWidget)
    {
    return;
    }

  int row = 0;
  while(row < d->TrajectoryTableWidget->rowCount())
    {
    qSlicerPathExplorerTrajectoryItem* currentItem =
      dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(row,0));
    if (currentItem)
      {
      if (currentItem->entryPoint() == modifiedNode)
        {
        currentItem->setDisplayEntry(visibility);
        }
      }
    row++;
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onTargetDisplayModified(vtkMRMLAnnotationFiducialNode* modifiedNode, bool visibility)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!d->TrajectoryTableWidget)
    {
    return;
    }

  int row = 0;
  while(row < d->TrajectoryTableWidget->rowCount())
    {
    qSlicerPathExplorerTrajectoryItem* currentItem =
      dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(row,0));
    if (currentItem)
      {
      if (currentItem->targetPoint() == modifiedNode)
        {
        currentItem->setDisplayTarget(visibility);
        }
      }
    row++;
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onEntryProjectionModified(vtkMRMLAnnotationFiducialNode* modifiedNode, bool projection)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!d->TrajectoryTableWidget)
    {
    return;
    }

  int row = 0;
  while(row < d->TrajectoryTableWidget->rowCount())
    {
    qSlicerPathExplorerTrajectoryItem* currentItem =
      dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(row,0));
    if (currentItem)
      {
      if (currentItem->entryPoint() == modifiedNode)
        {
        currentItem->setProjectionEntry(projection);
        }
      }
    row++;
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onTargetProjectionModified(vtkMRMLAnnotationFiducialNode* modifiedNode, bool projection)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!d->TrajectoryTableWidget)
    {
    return;
    }

  int row = 0;
  while(row < d->TrajectoryTableWidget->rowCount())
    {
    qSlicerPathExplorerTrajectoryItem* currentItem =
      dynamic_cast<qSlicerPathExplorerTrajectoryItem*>(d->TrajectoryTableWidget->item(row,0));
    if (currentItem)
      {
      if (currentItem->targetPoint() == modifiedNode)
        {
        currentItem->setProjectionTarget(projection);
        }
      }
    row++;
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onEntryTableWidgetAddButtonToggled(bool state)
{
  Q_D(qSlicerPathExplorerModuleWidget);
  Q_UNUSED(state);

  // Check if target button is also toggled
  if (d->TargetPointWidget->addButtonStatus == d->TargetPointWidget->addButtonStatus)
    {
    // Toggle it
    d->TargetPointWidget->setAddButtonState(false);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::
onTargetTableWidgetAddButtonToggled(bool state)
{
  Q_D(qSlicerPathExplorerModuleWidget);
  Q_UNUSED(state);

  // Check if target button is also toggled
  if (d->EntryPointWidget->addButtonStatus == d->EntryPointWidget->addButtonStatus)
    {
    // Toggle it
    d->EntryPointWidget->setAddButtonState(false);
    }
}
