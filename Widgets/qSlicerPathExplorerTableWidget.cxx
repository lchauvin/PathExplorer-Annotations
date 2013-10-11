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

#include "vtkSlicerVersionConfigure.h"

// PathExplorer Widgets includes
#include "qSlicerPathExplorerTableWidget.h"
#include "ui_qSlicerPathExplorerTableWidget.h"

// Annotation logic
#include "vtkSlicerAnnotationModuleLogic.h"

// VTK includes
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

// SlicerQt includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerPathExplorerFiducialItem.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PathExplorer
class Q_SLICER_MODULE_PATHEXPLORER_WIDGETS_EXPORT qSlicerPathExplorerTableWidgetPrivate
  : public Ui_qSlicerPathExplorerTableWidget
{
  Q_DECLARE_PUBLIC(qSlicerPathExplorerTableWidget);
 protected:
  qSlicerPathExplorerTableWidget * const q_ptr;
  vtkMRMLAnnotationHierarchyNode* selectedHierarchyNode;
  vtkSlicerAnnotationModuleLogic* annotationLogic;

 public:
  qSlicerPathExplorerTableWidgetPrivate(
    qSlicerPathExplorerTableWidget& object);
  virtual void setupUi(qSlicerPathExplorerTableWidget*);
};

//-----------------------------------------------------------------------------
qSlicerPathExplorerTableWidgetPrivate
::qSlicerPathExplorerTableWidgetPrivate(
  qSlicerPathExplorerTableWidget& object)
  : q_ptr(&object)
{
  this->selectedHierarchyNode = NULL;
  this->annotationLogic = NULL;
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTableWidgetPrivate
::setupUi(qSlicerPathExplorerTableWidget* widget)
{
  this->Ui_qSlicerPathExplorerTableWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
qSlicerPathExplorerTableWidget
::qSlicerPathExplorerTableWidget(QWidget *parentWidget)
  : Superclass (parentWidget)
    , d_ptr( new qSlicerPathExplorerTableWidgetPrivate(*this) )
{
  Q_D(qSlicerPathExplorerTableWidget);
  d->setupUi(this);

  qSlicerAbstractCoreModule* annotationModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Annotations");
  if (annotationModule)
    {
    d->annotationLogic =
      vtkSlicerAnnotationModuleLogic::SafeDownCast(annotationModule->logic());
    }

  connect(d->AddButton, SIGNAL(toggled(bool)),
          this, SLOT(onAddButtonToggled(bool)));

  connect(d->DeleteButton, SIGNAL(clicked()),
          this, SLOT(onDeleteButtonClicked()));

  connect(d->ClearButton, SIGNAL(clicked()),
          this, SLOT(onClearButtonClicked()));

  connect(d->TableWidget, SIGNAL(itemSelectionChanged()),
          this, SLOT(onSelectionChanged()));

  connect(d->TableWidget, SIGNAL(cellChanged(int,int)),
          this, SLOT(onCellChanged(int,int)));

  this->addButtonStatus = false;
}

//-----------------------------------------------------------------------------
qSlicerPathExplorerTableWidget
::~qSlicerPathExplorerTableWidget()
{
}

//-----------------------------------------------------------------------------
QTableWidget* qSlicerPathExplorerTableWidget
::getTableWidget()
{
  Q_D(qSlicerPathExplorerTableWidget);
  if (d->TableWidget)
    {
    return d->TableWidget;
    }
  return NULL;
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTableWidget
::setSelectedHierarchyNode(vtkMRMLAnnotationHierarchyNode* selectedNode)
{
  Q_D(qSlicerPathExplorerTableWidget);

  if (!selectedNode)
    {
    return;
    }

  d->selectedHierarchyNode = selectedNode;
}

//-----------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode* qSlicerPathExplorerTableWidget
::selectedHierarchyNode()
{
  Q_D(qSlicerPathExplorerTableWidget);

  return d->selectedHierarchyNode;
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTableWidget
::onAddButtonToggled(bool pushed)
{
  Q_D(qSlicerPathExplorerTableWidget);

  if (!d->annotationLogic || !d->selectedHierarchyNode)
    {
    return;
    }

  if (pushed)
    {
    // if active hierarchy node is different from selected one
    // set selected one as active
    if (d->annotationLogic->GetActiveHierarchyNode() != d->selectedHierarchyNode)
      {
      d->annotationLogic->SetActiveHierarchyNodeID(d->selectedHierarchyNode->GetID());
      }

    // Set fiducial as annotation to drop in selection node
    vtkMRMLApplicationLogic *mrmlAppLogic = d->annotationLogic->GetMRMLApplicationLogic();
    if (mrmlAppLogic)
      {
      vtkMRMLInteractionNode *inode = mrmlAppLogic->GetInteractionNode();
      if (inode)
        {
        vtkMRMLSelectionNode *snode = mrmlAppLogic->GetSelectionNode();
        if (snode)
          {
#if (Slicer_VERSION_MAJOR == 4 && Slicer_VERSION_MINOR <= 2)
          snode->SetActiveAnnotationID("vtkMRMLAnnotationFiducialNode");
#else
          snode->SetActivePlaceNodeClassName("vtkMRMLAnnotationFiducialNode");
#endif
          }
        }
      }

    // Place fiducial (persistent)
    d->annotationLogic->StartPlaceMode(true);
    }
  else
    {
    d->annotationLogic->StopPlaceMode(true);
    }

  this->addButtonStatus = pushed;
  emit addButtonToggled(pushed);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTableWidget
::onDeleteButtonClicked()
{
  Q_D(qSlicerPathExplorerTableWidget);

  if (!d->annotationLogic->GetMRMLScene())
    {
    return;
    }

  int selectedRow = d->TableWidget->currentRow();
  if (selectedRow < 0)
    {
    return;
    }

  // Remove fiducial from scene
  qSlicerPathExplorerFiducialItem* itemToRemove =
    dynamic_cast<qSlicerPathExplorerFiducialItem*>(d->TableWidget->item(selectedRow, 0));

  if (itemToRemove)
    {
    vtkMRMLAnnotationFiducialNode* nodeToDelete = itemToRemove->getFiducialNode();

    // ISSUE: removeRow first change the selection to +or- 1 row
    // Which trigger the SLOT onSelectionChanged with the new row selected
    // If fiducial node has been removed before, then it crashes
    // As removeRow also delete items, itemToRemove is no longer accessible
    // after removeRow.
    // FIX: Get fiducial node before calling removeRow, then call removeRow
    // then delete fiducial
    d->TableWidget->removeRow(selectedRow);

    if (nodeToDelete)
      {
      // Signal to remove all trajectories with this fiducial
      emit itemDeleted(nodeToDelete);
      d->annotationLogic->GetMRMLScene()->RemoveNode(nodeToDelete);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTableWidget
::onClearButtonClicked()
{
  Q_D(qSlicerPathExplorerTableWidget);

  if (!d->selectedHierarchyNode)
    {
    return;
    }

  int numberOfRows = d->TableWidget->rowCount();
  for (int i = 0; i < numberOfRows; ++i)
    {
    this->onDeleteButtonClicked();
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTableWidget
::onSelectionChanged()
{
  Q_D(qSlicerPathExplorerTableWidget);

  int selectedRow = d->TableWidget->currentRow();
  if (selectedRow < 0)
    {
    return;
    }

  // Get fiducial
  qSlicerPathExplorerFiducialItem* selectedItem =
    dynamic_cast<qSlicerPathExplorerFiducialItem*>(d->TableWidget->item(selectedRow,0));

  if (!selectedItem)
    {
    return;
    }

  vtkMRMLAnnotationFiducialNode* selectedFiducial =
    selectedItem->getFiducialNode();

  if (!selectedFiducial ||
      !selectedFiducial->GetAnnotationPointDisplayNode())
    {
    return;
    }

  // Set opacity of selected fiducial to 1.0
  selectedFiducial->GetAnnotationPointDisplayNode()->SetOpacity(1.0);

  // Set opacity of non-selected fiducials to 0.3
  for(int i = 0; i < d->TableWidget->rowCount(); i++)
    {
    if (i != selectedRow)
      {
      qSlicerPathExplorerFiducialItem* currentItem =
        dynamic_cast<qSlicerPathExplorerFiducialItem*>(d->TableWidget->item(i,0));
      if (currentItem)
        {
        vtkMRMLAnnotationFiducialNode* currentFiducial =
          currentItem->getFiducialNode();
        if (currentFiducial)
          {
          if (currentFiducial->GetAnnotationPointDisplayNode())
            {
            currentFiducial->GetAnnotationPointDisplayNode()->SetOpacity(0.3);
            }
          }
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTableWidget
::onCellChanged(int row, int column)
{
  Q_D(qSlicerPathExplorerTableWidget);
  Q_UNUSED(column);

  if (!d->TableWidget->item(row,0) ||
      !d->TableWidget->item(row,1) ||
      !d->TableWidget->item(row,2) ||
      !d->TableWidget->item(row,3))
    {
    return;
    }

  qSlicerPathExplorerFiducialItem* currentItem =
    dynamic_cast<qSlicerPathExplorerFiducialItem*>(d->TableWidget->item(row,0));
  if (!currentItem)
    {
    return;
    }

  // Get coordinates
  QString xCoord = QString(d->TableWidget->item(row, 1)->text());
  QString yCoord = QString(d->TableWidget->item(row, 2)->text());
  QString zCoord = QString(d->TableWidget->item(row, 3)->text());

  if (xCoord.isEmpty() || yCoord.isEmpty() || zCoord.isEmpty())
    {
    return;
    }

  // Convert coordinates from QString to double
  double newFiducialCoordinates[3] = {
    xCoord.toDouble(),
    yCoord.toDouble(),
    zCoord.toDouble() };

  // Get fiducial and set new coordinates
  vtkMRMLAnnotationFiducialNode* currentFiducial =
    currentItem->getFiducialNode();
  if (!currentFiducial)
    {
    return;
    }

  currentFiducial->SetFiducialWorldCoordinates(newFiducialCoordinates);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTableWidget
::setAddButtonState(bool state)
{
  Q_D(qSlicerPathExplorerTableWidget);

  bool oldState = d->AddButton->blockSignals(true);
  d->AddButton->setChecked(state ? Qt::Checked : Qt::Unchecked);
  d->AddButton->blockSignals(oldState);
}
