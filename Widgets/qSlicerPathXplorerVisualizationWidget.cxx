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

// SlicerQt includes
#include "qSlicerPathXplorerVisualizationWidget.h"
#include "ui_qSlicerPathXplorerVisualizationWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PathXplorer
class Q_SLICER_MODULE_PATHXPLORER_WIDGETS_EXPORT qSlicerPathXplorerVisualizationWidgetPrivate
  : public Ui_qSlicerPathXplorerVisualizationWidget
{
  Q_DECLARE_PUBLIC(qSlicerPathXplorerVisualizationWidget);
 protected:
  qSlicerPathXplorerVisualizationWidget * const q_ptr;
  qSlicerPathXplorerTrajectoryItem* trajectoryItem;
  std::map<std::string,vtkMRMLSliceNode*> ViewersMap;
  vtkMRMLSliceNode* selectedViewer;
  vtkMRMLLayoutLogic* layoutLogic;
  int currentLayout;

 public:
  qSlicerPathXplorerVisualizationWidgetPrivate(
    qSlicerPathXplorerVisualizationWidget& object);
  virtual void setupUi(qSlicerPathXplorerVisualizationWidget*);
};

//-----------------------------------------------------------------------------
qSlicerPathXplorerVisualizationWidgetPrivate
::qSlicerPathXplorerVisualizationWidgetPrivate(
  qSlicerPathXplorerVisualizationWidget& object)
  : q_ptr(&object)
{
  this->trajectoryItem = NULL;
  this->selectedViewer = NULL;
  this->layoutLogic = NULL;
  this->currentLayout = 0;
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidgetPrivate
::setupUi(qSlicerPathXplorerVisualizationWidget* widget)
{
  this->Ui_qSlicerPathXplorerVisualizationWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
qSlicerPathXplorerVisualizationWidget
::qSlicerPathXplorerVisualizationWidget(QWidget *parentWidget)
  : Superclass (parentWidget)
    , d_ptr( new qSlicerPathXplorerVisualizationWidgetPrivate(*this) )
{
  Q_D(qSlicerPathXplorerVisualizationWidget);
  d->setupUi(this);

  // Reslice  
  connect(d->ResliceComboBox, SIGNAL(currentIndexChanged(QString)),
	  this, SLOT(onResliceChanged(QString)));

  connect(d->PerpendicularCheckbox, SIGNAL(stateChanged(int)),
          this, SLOT(onPerpendicularChanged(int)));

  connect(d->ResliceSlider, SIGNAL(valueChanged(double)),
          this, SLOT(onAngleChanged(double)));

  // Display
  connect(d->DisplayPathCheckbox, SIGNAL(stateChanged(int)),
          this, SLOT(onDisplayPathChanged(int)));

  connect(d->DisplayEntryCheckbox, SIGNAL(stateChanged(int)),
          this, SLOT(onDisplayEntryChanged(int)));

  connect(d->DisplayTargetCheckbox, SIGNAL(stateChanged(int)),
          this, SLOT(onDisplayTargetChanged(int)));

  // Projection
  connect(d->ProjectionPathCheckbox, SIGNAL(stateChanged(int)),
          this, SLOT(onProjectionPathChanged(int)));

  connect(d->ProjectionEntryCheckbox, SIGNAL(stateChanged(int)),
          this, SLOT(onProjectionEntryChanged(int)));

  connect(d->ProjectionTargetCheckbox, SIGNAL(stateChanged(int)),
          this, SLOT(onProjectionTargetChanged(int)));

  // layoutManager
  qSlicerApplication* app = qSlicerApplication::application();
  if (app)
    {
    qSlicerLayoutManager* layoutManager = app->layoutManager();
    if (layoutManager)
      {
      connect(layoutManager, SIGNAL(layoutChanged(int)),
	      this, SLOT(onLayoutChanged(int)));

      vtkMRMLLayoutLogic* tmpLayoutLogic = layoutManager->layoutLogic();
      if (!tmpLayoutLogic)
	{
	return;
	}
      d->layoutLogic = tmpLayoutLogic;

      vtkMRMLLayoutNode* tmpLayoutNode =
	d->layoutLogic->GetLayoutNode();
      if (tmpLayoutNode)
	{
	d->currentLayout = tmpLayoutNode->GetViewArrangement();
	}
      }
    }

  // mrmlScene
  connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
	  this, SLOT(onMRMLSceneChanged(vtkMRMLScene*)));
}

//-----------------------------------------------------------------------------
qSlicerPathXplorerVisualizationWidget
::~qSlicerPathXplorerVisualizationWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::onMRMLSceneChanged(vtkMRMLScene* newScene)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);  

  // Layout number doesn't matter here
  this->populateLayouts();
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::setTrajectoryItem(qSlicerPathXplorerTrajectoryItem* item)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);
  
  // Disconnect previous signals if existing
  if (d->trajectoryItem)
    {
    disconnect(d->trajectoryItem, SIGNAL(resliceModified()),
               this, SLOT(updateReslice()));
    disconnect(d->trajectoryItem, SIGNAL(displayModified()),
               this, SLOT(updateDisplay()));
    disconnect(d->trajectoryItem, SIGNAL(projectionModified()),
               this, SLOT(updateProjection()));
    }
  
  if (!item)
    {
    d->trajectoryItem = NULL;
    return;
    }
  
  // Connect new object signals
  connect(item, SIGNAL(resliceModified()),
          this, SLOT(updateReslice()));

  connect(item, SIGNAL(displayModified()),
          this, SLOT(updateDisplay()));

  connect(item, SIGNAL(projectionModified()),
          this, SLOT(updateProjection()));
  
  // Set new trajectory item
  d->trajectoryItem = item;

  this->updateResliceMode(d->trajectoryItem->getReslicePerpendicular() ? PERPENDICULAR : TRANSVERSE);
  this->updateReslice();
  this->updateDisplay();
  this->updateProjection();
}

//-----------------------------------------------------------------------------
qSlicerPathXplorerTrajectoryItem* qSlicerPathXplorerVisualizationWidget
::getTrajectoryItem()
{
  Q_D(qSlicerPathXplorerVisualizationWidget);
  
  return d->trajectoryItem;
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::onLayoutChanged(int newLayout)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!d->ResliceComboBox || !d->trajectoryItem ||
      !d->layoutLogic)
    {
    return;
    }

  vtkMRMLLayoutNode* layoutNode =
    d->layoutLogic->GetLayoutNode();
  if (!layoutNode)
    {
    return;
    }

  int viewArrangement = layoutNode->GetViewArrangement();
  if (viewArrangement != d->currentLayout)
    {
    d->currentLayout = viewArrangement;
    this->populateLayouts();
    }
}


//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::populateLayouts()
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  // Get number of viewers
  vtkCollection* visibleViews = d->layoutLogic->GetViewNodes();
  int numberOfViewers = visibleViews->GetNumberOfItems();

  // Clear previous items
  d->ResliceComboBox->clear();
  d->ViewersMap.clear();
  
  QStringList itemList = QStringList();
  itemList << "None";
  for (int i = 0; i < numberOfViewers; ++i)
    {
    // Add new item to viewer list
    vtkMRMLSliceNode* tmpNode = vtkMRMLSliceNode::SafeDownCast(visibleViews->GetItemAsObject(i));
    if (tmpNode)
      {
      itemList << tmpNode->GetName();
      d->ViewersMap.insert(std::pair<std::string,vtkMRMLSliceNode*>(std::string(tmpNode->GetName()),tmpNode));
      }
    }
  d->ResliceComboBox->addItems(itemList);
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::updateReslice()
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!d->trajectoryItem)
    {
    return;
    }

  // Set reslice
  int index = d->ResliceComboBox->findText(d->trajectoryItem->getResliceViewer());
  if (index == -1)
    {
    index = 0;
    }
  
  double sliderValue = d->trajectoryItem->getReslicePerpendicular() ? 
    d->trajectoryItem->getReslicePosition() :
    d->trajectoryItem->getResliceAngle(); 

  d->ResliceComboBox->setCurrentIndex(index);
  d->PerpendicularCheckbox->setCheckState(d->trajectoryItem->getReslicePerpendicular() ? Qt::Checked : Qt::Unchecked);
  d->ResliceSlider->setValue(sliderValue);
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::updateDisplay()
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!d->trajectoryItem)
    {
    return;
    }

  // Set Visualization
  d->DisplayPathCheckbox->setCheckState(d->trajectoryItem->getDisplayPath() ? Qt::Checked : Qt::Unchecked);
  d->DisplayEntryCheckbox->setCheckState(d->trajectoryItem->getDisplayEntry() ? Qt::Checked : Qt::Unchecked);
  d->DisplayTargetCheckbox->setCheckState(d->trajectoryItem->getDisplayTarget() ? Qt::Checked : Qt::Unchecked);
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::updateProjection()
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!d->trajectoryItem)
    {
    return;
    }

  // Set Projection
  d->ProjectionPathCheckbox->setCheckState(d->trajectoryItem->getProjectionPath() ? Qt::Checked : Qt::Unchecked);
  d->ProjectionEntryCheckbox->setCheckState(d->trajectoryItem->getProjectionEntry() ? Qt::Checked : Qt::Unchecked);
  d->ProjectionTargetCheckbox->setCheckState(d->trajectoryItem->getProjectionTarget() ? Qt::Checked : Qt::Unchecked);
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::onResliceChanged(QString newViewers)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!d->trajectoryItem)
    {
    return;
    }

  // Find SliceNode
  SliceNodeIt end = d->ViewersMap.end();
  SliceNodeIt it;
  vtkMRMLSliceNode* currentSelectedNode = NULL;

  it = d->ViewersMap.find(newViewers.toStdString());
  if (it == end)
    {
    if (d->selectedViewer)
      {
      // "None" selected
      d->selectedViewer->SetOrientation(d->selectedViewer->GetOrientationReference());
      d->selectedViewer = NULL;
      return;
      }
    }
  else
    {
    // Slice node found
    currentSelectedNode = (*it).second;
    if (!currentSelectedNode)
      {
      return;
      }
    }
  
  vtkMRMLAnnotationRulerNode* rulerNode
    = d->trajectoryItem->trajectoryNode();
  if (!rulerNode)
    {
    return;
    }
  
  // Reslice
  d->selectedViewer = currentSelectedNode;

  // Update node
  d->trajectoryItem->setResliceViewer(newViewers.toStdString().c_str());

  // Update reslicing
  this->updateReslicing();
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::onPerpendicularChanged(int state)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);
  
  if (!d->trajectoryItem)
    {
    return;
    }

  bool perpendicular = state == Qt::Checked;
  if (d->trajectoryItem->getReslicePerpendicular() == perpendicular)
    {
    return;
    }  

  this->updateResliceMode(perpendicular ? PERPENDICULAR : TRANSVERSE);

  d->trajectoryItem->setReslicePerpendicular(perpendicular);
  
  this->updateReslicing();
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::onAngleChanged(double angle)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!d->trajectoryItem)
    {
    return;
    }

  if (d->trajectoryItem->getReslicePerpendicular())
    {
    d->trajectoryItem->setReslicePosition(angle);
    }
  else
    {
    d->trajectoryItem->setResliceAngle(angle);
    }

  // Update reslicing
  this->updateReslicing();
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::onDisplayPathChanged(int state)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!d->trajectoryItem)
    {
    return;
    }

  vtkMRMLAnnotationRulerNode* ruler
    = d->trajectoryItem->trajectoryNode();
  if (!ruler)
    {
    return;
    }
  
  bool visibility = state == Qt::Checked;
  ruler->SetDisplayVisibility(visibility);
  d->trajectoryItem->setDisplayPath(visibility);
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::onDisplayEntryChanged(int state)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!d->trajectoryItem)
    {
    return;
    }

  vtkMRMLAnnotationFiducialNode* entryPoint
    = d->trajectoryItem->entryPoint();
  if (!entryPoint)
    {
    return;
    }
    
  bool visibility = state == Qt::Checked;
  entryPoint->SetDisplayVisibility(visibility);
  d->trajectoryItem->setDisplayEntry(visibility);

  // Update display of all other nodes having the same entry point
  emit entryDisplayModified(entryPoint, visibility);
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::onDisplayTargetChanged(int state)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!d->trajectoryItem)
    {
    return;
    }

  vtkMRMLAnnotationFiducialNode* targetPoint
    = d->trajectoryItem->targetPoint();
  if (!targetPoint)
    {
    return;
    }
    
  bool visibility = state == Qt::Checked;
  targetPoint->SetDisplayVisibility(visibility);
  d->trajectoryItem->setDisplayTarget(visibility);

  // Update all other nodes having the same target point
  emit targetDisplayModified(targetPoint, visibility);
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::onProjectionPathChanged(int state)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!d->trajectoryItem)
    {
    return;
    }

  bool projection = state == Qt::Checked;
  if (d->trajectoryItem->getProjectionPath() == projection)
    {
    return;
    }  
  
  vtkMRMLAnnotationRulerNode* ruler
    = d->trajectoryItem->trajectoryNode();
  if (!ruler)
    {
    return;
    }
  
  vtkMRMLAnnotationLineDisplayNode* line
    = ruler->GetAnnotationLineDisplayNode();
  if (!line)
    {
    return;
    } 

  if (projection)
    {
    line->SliceProjectionOn();
    }
  else
    {
    line->SliceProjectionOff();
    }
  d->trajectoryItem->setProjectionPath(projection);
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::onProjectionEntryChanged(int state)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!d->trajectoryItem)
    {
    return;
    }

  bool projection = state == Qt::Checked;
  if (d->trajectoryItem->getProjectionEntry() == projection)
    {
    return;
    }  

  vtkMRMLAnnotationFiducialNode* entryPoint
    = d->trajectoryItem->entryPoint();
  if (!entryPoint)
    {
    return;
    }
  
  vtkMRMLAnnotationPointDisplayNode* point
    = entryPoint->GetAnnotationPointDisplayNode();
  if (!point)
    {
    return;
    }

  if (projection)
    {
    point->SliceProjectionOn();
    }
  else
    {
    point->SliceProjectionOff();
    }
  d->trajectoryItem->setProjectionEntry(projection);

  // Update projection of all other nodes having same entry point
  emit entryProjectionModified(entryPoint, projection);
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::onProjectionTargetChanged(int state)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!d->trajectoryItem)
    {
    return;
    }

  bool projection = state == Qt::Checked;
  if (d->trajectoryItem->getProjectionTarget() == projection)
    {
    return;
    }  

  vtkMRMLAnnotationFiducialNode* targetPoint
    = d->trajectoryItem->targetPoint();
  if (!targetPoint)
    {
    return;
    }
  
  vtkMRMLAnnotationPointDisplayNode* point
    = targetPoint->GetAnnotationPointDisplayNode();
  if (!point)
    {
    return;
    }

  if (projection)
    {
    point->SliceProjectionOn();
    }
  else
    {
    point->SliceProjectionOff();
    }
  d->trajectoryItem->setProjectionTarget(projection);

  // Update projection of all other nodes having the same target point
  emit targetProjectionModified(targetPoint, projection);
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::updateReslicing()
{
  Q_D(qSlicerPathXplorerVisualizationWidget);
  
  if (!d->trajectoryItem)
    {
    return;
    }

  vtkMRMLAnnotationRulerNode* ruler
    = d->trajectoryItem->trajectoryNode();
  if (!ruler || !d->selectedViewer)
    {
    return;
    }
  double sliderValue = d->trajectoryItem->getReslicePerpendicular() ?
    d->trajectoryItem->getReslicePosition() :
    d->trajectoryItem->getResliceAngle();
  this->resliceWithRuler(ruler,
			 d->selectedViewer,
			 sliderValue,
			 d->trajectoryItem->getReslicePerpendicular());
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::resliceWithRuler(vtkMRMLAnnotationRulerNode* ruler,
		   vtkMRMLSliceNode* viewer,
		   double angleOrPosition, 
		   bool perpendicular)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!ruler || !viewer)
    {
    return;
    }

  // Get ruler points
  double point1[3];
  double point2[3];
  ruler->GetPositionWorldCoordinates1(point1);
  ruler->GetPositionWorldCoordinates2(point2);

  // Compute vectors
  double t[3];
  double n[3];
  double pos[3];
  if (perpendicular)
    {
    // Ruler vector is normal vector
    n[0] = point2[0] - point1[0];
    n[1] = point2[1] - point1[1];
    n[2] = point2[2] - point1[2];

    // Reslice at chosen position
    pos[0] = point1[0] + n[0] * angleOrPosition / 100;
    pos[1] = point1[1] + n[1] * angleOrPosition / 100;
    pos[2] = point1[2] + n[2] * angleOrPosition / 100;

    // Normalize
    double nlen = vtkMath::Normalize(n);
    n[0] /= nlen;
    n[1] /= nlen;
    n[2] /= nlen;

    // angle in radian
    vtkMath::Perpendiculars(n, t, NULL, 0);
    }
  else
    {
    // Ruler vector is transverse vector
    t[0] = point2[0] - point1[0];
    t[1] = point2[1] - point1[1];
    t[2] = point2[2] - point1[2];

    // Reslice at target position
    pos[0] = point2[0];
    pos[1] = point2[1];
    pos[2] = point2[2];

    // Normalize
    double tlen = vtkMath::Normalize(t);
    t[0] /= tlen;
    t[1] /= tlen;
    t[2] /= tlen;

    // angle in radian
    vtkMath::Perpendiculars(t, n, NULL, angleOrPosition*vtkMath::Pi()/180);
    }
    
  double nx = n[0];
  double ny = n[1];
  double nz = n[2];
  double tx = t[0];
  double ty = t[1];
  double tz = t[2];
  double px = pos[0];
  double py = pos[1];
  double pz = pos[2];

  viewer->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 0);
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerVisualizationWidget
::updateResliceMode(int mode)
{
  Q_D(qSlicerPathXplorerVisualizationWidget);

  if (!d->trajectoryItem)
    {
    return;
    }

  bool oldState = d->ResliceSlider->blockSignals(true);
  
  if (mode)
    {
    d->ResliceSliderLabel->setText("Position (%)");
    d->ResliceSlider->setMinimum(0);
    d->ResliceSlider->setMaximum(100);
    }
  else
    {
    d->ResliceSliderLabel->setText("Angle");
    d->ResliceSlider->setMinimum(-180);
    d->ResliceSlider->setMaximum(180);
    }

  d->ResliceSlider->blockSignals(oldState);
}
