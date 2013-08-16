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

// PathXplorer Widgets includes
#include "qSlicerPathXplorerReslicingWidget.h"
#include "qSlicerPathXplorerTrajectoryItem.h"
#include "ui_qSlicerPathXplorerReslicingWidget.h"

#include <vtkMRMLAnnotationLineDisplayNode.h>
#include <vtkMRMLAnnotationRulerNode.h>
#include <vtkMRMLSliceNode.h>

#include "ctkPopupWidget.h"

// VTK includes
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class qSlicerPathXplorerReslicingWidget;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PathXplorer
class Q_SLICER_MODULE_PATHXPLORER_WIDGETS_EXPORT qSlicerPathXplorerReslicingWidgetPrivate
  : public Ui_qSlicerPathXplorerReslicingWidget
{
  Q_DECLARE_PUBLIC(qSlicerPathXplorerReslicingWidget);

 public:
  qSlicerPathXplorerReslicingWidgetPrivate(
    qSlicerPathXplorerReslicingWidget& object);
  virtual ~qSlicerPathXplorerReslicingWidgetPrivate();
  virtual void setupUi(qSlicerPathXplorerReslicingWidget*);

  void loadAttributesFromViewer();
  void saveAttributesToViewer();
  void updateWidget();

 protected:
  qSlicerPathXplorerReslicingWidget * const	q_ptr;
  qSlicerPathXplorerTrajectoryItem*		TrajectoryItem;
  vtkMRMLSliceNode*				SliceNode;
  std::string					DrivingRulerNodeID;
  std::string					DrivingRulerNodeName;
  double					ResliceAngle;
  double					ReslicePosition;
  bool						ReslicePerpendicular;
};

//-----------------------------------------------------------------------------
qSlicerPathXplorerReslicingWidgetPrivate
::qSlicerPathXplorerReslicingWidgetPrivate(
  qSlicerPathXplorerReslicingWidget& object)
  : q_ptr(&object)
{
  this->DrivingRulerNodeID.assign("");
  this->DrivingRulerNodeName.assign("");
  this->TrajectoryItem	     = NULL;
  this->SliceNode	     = NULL;
  this->ResliceAngle	     = 0.0;
  this->ReslicePosition	     = 0.0;
  this->ReslicePerpendicular = true;
}

//-----------------------------------------------------------------------------
qSlicerPathXplorerReslicingWidgetPrivate
::~qSlicerPathXplorerReslicingWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidgetPrivate
::setupUi(qSlicerPathXplorerReslicingWidget* widget)
{
  this->Ui_qSlicerPathXplorerReslicingWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidgetPrivate
::loadAttributesFromViewer()
{
  if (!this->TrajectoryItem)
    {
    return;
    }

  vtkMRMLAnnotationRulerNode* ruler = this->TrajectoryItem->trajectoryNode();
  if (!ruler)
    {
    return;
    }

  const char* drivingID = this->SliceNode->GetAttribute("PathXplorer.DrivingPathID");
  if (drivingID)
    {
    this->DrivingRulerNodeID.assign(drivingID);
    }

  const char* drivingName = this->SliceNode->GetAttribute("PathXplorer.DrivingPathName");
  if (drivingName)
    {
    this->DrivingRulerNodeName.assign(drivingName);
    }

  std::stringstream itemPosAttrStr;
  itemPosAttrStr << "PathXplorer." << ruler->GetName() << "_" << this->SliceNode->GetName() << "_Position";
  const char* posStr = this->SliceNode->GetAttribute(itemPosAttrStr.str().c_str());
  this->ReslicePosition = posStr ? 
    atof(posStr) : 
    0.0;
  
  std::stringstream itemAngleAttrStr;
  itemAngleAttrStr << "PathXplorer." << ruler->GetName() << "_" << this->SliceNode->GetName() << "_Angle";
  const char* angleStr = this->SliceNode->GetAttribute(itemAngleAttrStr.str().c_str());
  this->ResliceAngle = angleStr ? 
    atof(angleStr) : 
    0.0;
  
  std::stringstream itemPerpAttrStr;
  itemPerpAttrStr << "PathXplorer." << ruler->GetName() << "_" << this->SliceNode->GetName() << "_Perpendicular";
  const char* perpStr = this->SliceNode->GetAttribute(itemPerpAttrStr.str().c_str());
  if (perpStr)
    {
    this->ReslicePerpendicular = strcmp(perpStr, "ON") == 0;
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidgetPrivate
::saveAttributesToViewer()
{
  if (!this->TrajectoryItem)
    {
    return;
    }

  vtkMRMLAnnotationRulerNode* ruler = this->TrajectoryItem->trajectoryNode();
  if (!ruler)
    {
    return;
    }

  std::stringstream posAttrStr;
  posAttrStr << "PathXplorer." << ruler->GetName() << "_" << this->SliceNode->GetName() << "_Position";
  std::stringstream posValStr;
  posValStr << this->ReslicePosition;
  this->SliceNode->SetAttribute(posAttrStr.str().c_str(), posValStr.str().c_str());
  
  std::stringstream angleAttrStr;
  angleAttrStr << "PathXplorer." << ruler->GetName() << "_" << this->SliceNode->GetName() << "_Angle";
  std::stringstream angleValStr;
  angleValStr << this->ResliceAngle;
  this->SliceNode->SetAttribute(angleAttrStr.str().c_str(), angleValStr.str().c_str());
  
  std::stringstream perpAttrStr;
  perpAttrStr << "PathXplorer." << ruler->GetName() << "_" << this->SliceNode->GetName() << "_Perpendicular";
  this->SliceNode->SetAttribute(perpAttrStr.str().c_str(), this->ReslicePerpendicularRadioButton->isChecked() ? 
				"ON" : 
				"OFF");
}
//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidgetPrivate
::updateWidget()
{
  if (!this->SliceNode || !this->TrajectoryItem)
    {
    return;
    }

  vtkMRMLAnnotationRulerNode* ruler = this->TrajectoryItem->trajectoryNode();
  if (!ruler)
    {
    return;
    }
  vtkMRMLAnnotationLineDisplayNode* rulerDisplayNode = ruler->GetAnnotationLineDisplayNode();

  int sliderMinimum = 0;
  int sliderMaximum = this->ReslicePerpendicular ? 100 : 180;
  double sliderValue = this->ReslicePerpendicular ? this->ReslicePosition : this->ResliceAngle;
  const char* distanceLabel = this->ReslicePerpendicular ? "Distance: " : "Angle: ";
  const char* unitsLabel = this->ReslicePerpendicular ? "mm" : "deg";
  
  // Update reslice button
  bool bOldState = this->ResliceButton->blockSignals(true);
  this->ResliceButton->setChecked(0);
  this->ResliceSlider->setEnabled(0);
  this->ReslicePerpendicularRadioButton->setEnabled(0);
  this->ResliceInPlaneRadioButton->setEnabled(0);
  
  if (!this->DrivingRulerNodeID.empty())
    {
    this->ResliceButton->setText(this->DrivingRulerNodeName.c_str());
    if (this->DrivingRulerNodeID.compare(ruler->GetID()) == 0)
      {
      this->ResliceButton->setChecked(1);
      this->ResliceSlider->setEnabled(1);
      this->ReslicePerpendicularRadioButton->setEnabled(1);
      this->ResliceInPlaneRadioButton->setEnabled(1);
      }
    }
  this->ResliceButton->blockSignals(bOldState);

  // Update slider
  bool oldState = this->ResliceSlider->blockSignals(true);
  this->ResliceSlider->setMinimum(sliderMinimum);
  this->ResliceSlider->setMaximum(sliderMaximum);
  this->ResliceSlider->setValue(sliderValue);
  this->ResliceSlider->blockSignals(oldState);

  // Update labels
  this->DistanceAngleLabel->setText(distanceLabel);
  this->DistanceAngleUnitLabel->setText(unitsLabel);
  if (this->ReslicePerpendicular)
    {
    QString decimalValue;
    double distanceValue = ruler->GetDistanceMeasurement() * this->ReslicePosition / 100;
    decimalValue = decimalValue.setNum(distanceValue, 'f', 2);
    this->ResliceValueLabel->setText(decimalValue);
    }
  else
    {
    this->ResliceValueLabel->setNum(this->ResliceAngle);
    }
  
  // Update radio buttons
  bool test = this->ReslicePerpendicularRadioButton->blockSignals(true);
  bool test2 = this->ResliceInPlaneRadioButton->blockSignals(true);

  this->ReslicePerpendicularRadioButton->setChecked(this->ReslicePerpendicular);
  this->ResliceInPlaneRadioButton->setChecked(!this->ReslicePerpendicular);
  
  this->ReslicePerpendicularRadioButton->blockSignals(test);
  this->ResliceInPlaneRadioButton->blockSignals(test2);

  // Update slice intersection visibility
  if (rulerDisplayNode && this->ResliceButton->isChecked())
    {
    rulerDisplayNode->SetSliceIntersectionVisibility(this->ReslicePerpendicular);
    }
}

//-----------------------------------------------------------------------------
qSlicerPathXplorerReslicingWidget
::qSlicerPathXplorerReslicingWidget(vtkMRMLSliceNode* sliceNode, QWidget *parentWidget)
  : Superclass (parentWidget)
    , d_ptr ( new qSlicerPathXplorerReslicingWidgetPrivate(*this) )
{
  Q_D(qSlicerPathXplorerReslicingWidget);
  d->setupUi(this);

  if (!sliceNode)
    {
    return;
    }

  this->setEnabled(0);
  d->SliceNode = sliceNode;

  // Set text
  d->ResliceButton->setText(sliceNode->GetName());

  // Set color
  double color[3];
  sliceNode->GetLayoutColor(color);
  std::stringstream buttonBgColor;
  buttonBgColor << "QPushButton { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgba("
		<< color[0]*255 << ","
		<< color[1]*255 << ","
		<< color[2]*255 << ","
		<< "128), stop: 1 rgba("
		<< color[0]*255 << ","
		<< color[1]*255 << ","
		<< color[2]*255 << ","
		<< "128)); width:100%; border:1px solid black;} "
		<< ":checked { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgba("
		<< color[0]*255 << ","
		<< color[1]*255 << ","
		<< color[2]*255 << ","
		<< "255), stop: 1 rgba("
		<< color[0]*255 << ","
		<< color[1]*255 << ","
		<< color[2]*255 << ","
		<< "255)); width:100%; border:1px solid black;} ";

  d->ResliceButton->setAutoFillBackground(true);
  d->ResliceButton->setStyleSheet(buttonBgColor.str().c_str());

  connect(d->ResliceButton, SIGNAL(toggled(bool)),
	  this, SLOT(onResliceToggled(bool)));
  connect(d->ResliceSlider, SIGNAL(valueChanged(int)),
	  this, SLOT(onResliceValueChanged(int)));
  connect(d->ReslicePerpendicularRadioButton, SIGNAL(toggled(bool)),
	  this, SLOT(onPerpendicularToggled(bool)));
}

//-----------------------------------------------------------------------------
qSlicerPathXplorerReslicingWidget
::~qSlicerPathXplorerReslicingWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidget
::setTrajectoryItem(qSlicerPathXplorerTrajectoryItem* item)
{
  Q_D(qSlicerPathXplorerReslicingWidget);

  // Disable everything except button
  this->setEnabled(1);
  
  if (!d->SliceNode || !item)
    {
    return;
    }
  
  if (d->TrajectoryItem)
    {
    // If previous trajectory, save values as attributes before changing it
    d->saveAttributesToViewer();
    }

  // Load previous values of new trajectory if exists
  d->TrajectoryItem = item;
  vtkMRMLAnnotationRulerNode* newRuler = d->TrajectoryItem->trajectoryNode();
  if (newRuler)
    {
    d->loadAttributesFromViewer();
    d->updateWidget();
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidget
::onResliceToggled(bool buttonStatus)
{
  Q_D(qSlicerPathXplorerReslicingWidget);

  if (!d->SliceNode || !d->TrajectoryItem)
    {
    return;
    }

  vtkMRMLAnnotationRulerNode* ruler = d->TrajectoryItem->trajectoryNode();
  if (!ruler)
    {
    return;
    }

  if (buttonStatus)
    {
    // Reslice
    d->ResliceSlider->setEnabled(1);
    d->ReslicePerpendicularRadioButton->setEnabled(1);
    d->ResliceInPlaneRadioButton->setEnabled(1);

    d->DrivingRulerNodeID.assign(ruler->GetID());
    d->DrivingRulerNodeName.assign(ruler->GetName());
    d->SliceNode->SetAttribute("PathXplorer.DrivingPathID",ruler->GetID());
    d->SliceNode->SetAttribute("PathXplorer.DrivingPathName",ruler->GetName());
    d->updateWidget();

    this->resliceWithRuler(ruler,
			   d->SliceNode,
			   d->ReslicePerpendicular,
			   d->ReslicePerpendicular ? d->ReslicePosition : d->ResliceAngle);
    }
  else
    {
    // Reslice
    d->ResliceSlider->setEnabled(0);
    d->ReslicePerpendicularRadioButton->setEnabled(0);
    d->ResliceInPlaneRadioButton->setEnabled(0);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidget
::onPerpendicularToggled(bool status)
{
  Q_D(qSlicerPathXplorerReslicingWidget);

  if (!d->TrajectoryItem)
    {
    return;
    }
  vtkMRMLAnnotationRulerNode* ruler = d->TrajectoryItem->trajectoryNode();
  if (!ruler)
    {
    return;
    }

  d->ReslicePerpendicular = status;
  d->updateWidget();

  this->resliceWithRuler(ruler,
			 d->SliceNode,
			 d->ReslicePerpendicular,
			 d->ReslicePerpendicular ? d->ReslicePosition : d->ResliceAngle);
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidget
::onResliceValueChanged(int resliceValue)
{
  Q_D(qSlicerPathXplorerReslicingWidget);

  if (!d->TrajectoryItem)
    {
    return;
    }
  vtkMRMLAnnotationRulerNode* ruler = d->TrajectoryItem->trajectoryNode();
  if (!ruler)
    {
    return;
    }

  if (d->ReslicePerpendicular)
    {
    d->ReslicePosition = resliceValue;
    QString decimalValue;
    double distanceValue = ruler->GetDistanceMeasurement() * d->ReslicePosition / 100;
    decimalValue = decimalValue.setNum(distanceValue, 'f', 2);
    d->ResliceValueLabel->setText(decimalValue);
    }
  else
    {
    d->ResliceAngle = resliceValue;
    d->ResliceValueLabel->setNum(d->ResliceAngle);
    }

  if (d->ResliceButton->isChecked())
    {
    this->resliceWithRuler(ruler,
			   d->SliceNode,
			   d->ReslicePerpendicular,
			   d->ReslicePerpendicular ? d->ReslicePosition : d->ResliceAngle);
    }  
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidget
::resliceWithRuler(vtkMRMLAnnotationRulerNode* ruler,
		   vtkMRMLSliceNode* viewer,
		   bool perpendicular,
		   double resliceValue)
{
  if (!ruler || !viewer)
    {
    return;
    }

  // Get ruler points
  double point1[4] = {0,0,0,0};
  double point2[4] = {0,0,0,0};
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
    pos[0] = point1[0] + n[0] * resliceValue / 100;
    pos[1] = point1[1] + n[1] * resliceValue / 100;
    pos[2] = point1[2] + n[2] * resliceValue / 100;

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
    vtkMath::Perpendiculars(t, n, NULL, resliceValue*vtkMath::Pi()/180);
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
