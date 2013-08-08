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

#include "qMRMLViewControllerBar_p.h"
#include <qMRMLViewControllerBar.h>

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
  : public qMRMLViewControllerBarPrivate, public Ui_qSlicerPathXplorerReslicingWidget
{
  Q_DECLARE_PUBLIC(qSlicerPathXplorerReslicingWidget);

 public:
  typedef qMRMLViewControllerBarPrivate Superclass;

  qSlicerPathXplorerReslicingWidgetPrivate(
    qSlicerPathXplorerReslicingWidget& object);
  virtual ~qSlicerPathXplorerReslicingWidgetPrivate();

  virtual void init();

  qSlicerPathXplorerTrajectoryItem* currentTrajectoryItem;
  vtkMRMLSliceNode* associatedSliceNode;
  double reslicingAngleValue;
  double reslicingPositionValue;
  
 protected:
  virtual void setupPopupUi();
};

//-----------------------------------------------------------------------------
qSlicerPathXplorerReslicingWidgetPrivate
::qSlicerPathXplorerReslicingWidgetPrivate(
  qSlicerPathXplorerReslicingWidget& object)
  : Superclass(object)
{
  this->currentTrajectoryItem = NULL;
  this->associatedSliceNode = NULL;
  this->reslicingAngleValue = 0.0;
  this->reslicingPositionValue = 0.0;
}

//-----------------------------------------------------------------------------
qSlicerPathXplorerReslicingWidgetPrivate
::~qSlicerPathXplorerReslicingWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidgetPrivate
::setupPopupUi()
{
  this->Superclass::setupPopupUi();
  this->PopupWidget->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
  this->Ui_qSlicerPathXplorerReslicingWidget::setupUi(this->PopupWidget);
}


//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidgetPrivate
::init()
{
  this->Superclass::init();
  this->ViewLabel->setText(qSlicerPathXplorerReslicingWidget::tr("1"));
  this->BarLayout->addStretch(1);
}

//-----------------------------------------------------------------------------
qSlicerPathXplorerReslicingWidget
::qSlicerPathXplorerReslicingWidget(vtkMRMLSliceNode* sliceNode, QWidget *parentWidget)
  : Superclass ( new qSlicerPathXplorerReslicingWidgetPrivate(*this), parentWidget)
{
  Q_D(qSlicerPathXplorerReslicingWidget);

  if (!sliceNode)
    {
    return;
    }

  d->init();
  this->setEnabled(0);
  d->associatedSliceNode = sliceNode;
  d->ViewLabel->setText(sliceNode->GetLayoutLabel());
  d->ActivateReslicingButton->setText(sliceNode->GetName());
  double color[3];
  sliceNode->GetLayoutColor(color);
  d->setColor(QColor::fromRgbF(color[0],color[1],color[2]));

  connect(d->ActivateReslicingButton, SIGNAL(toggled(bool)),
	  this, SLOT(onResliceToggled(bool)));
  connect(d->ResliceAnglePositionSlider, SIGNAL(valueChanged(int)),
	  this, SLOT(onResliceValueChanged(int)));
  connect(d->PerpendicularRadioButton, SIGNAL(toggled(bool)),
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

  d->ActivateReslicingButton->setChecked(false);
  this->setEnabled(1);
  d->ResliceAnglePositionSlider->setEnabled(0);

  if (!d->associatedSliceNode)
    {
    return;
    }

  if (d->currentTrajectoryItem)
    {
    // Save values as attribute before changing item    
    vtkMRMLAnnotationRulerNode* ruler = d->currentTrajectoryItem->trajectoryNode();
    if (!ruler)
      {
      return;
      }
    
    // TODO: Create SaveAttributes function
    std::stringstream posAttrStr;
    posAttrStr << "PathXplorer." << ruler->GetName() << "_" << d->associatedSliceNode->GetName() << "_Position";
    std::stringstream posValStr;
    posValStr << d->reslicingPositionValue;
    d->associatedSliceNode->SetAttribute(posAttrStr.str().c_str(), posValStr.str().c_str());
    
    std::stringstream angleAttrStr;
    angleAttrStr << "PathXplorer." << ruler->GetName() << "_" << d->associatedSliceNode->GetName() << "_Angle";
    std::stringstream angleValStr;
    angleValStr << d->reslicingAngleValue;
    d->associatedSliceNode->SetAttribute(angleAttrStr.str().c_str(), angleValStr.str().c_str());

    std::stringstream perpAttrStr;
    perpAttrStr << "PathXplorer." << ruler->GetName() << "_" << d->associatedSliceNode->GetName() << "_Perpendicular";
    d->associatedSliceNode->SetAttribute(perpAttrStr.str().c_str(), d->PerpendicularRadioButton->isChecked() ? 
					 "ON" : 
					 "OFF");
    }

  // Load previous values of new trajectory if exists
  vtkMRMLAnnotationRulerNode* newRuler = item->trajectoryNode();
  if (newRuler)
    {
    // TODO: Create LoadAttributes function
    std::stringstream itemPosAttrStr;
    itemPosAttrStr << "PathXplorer." << newRuler->GetName() << "_" << d->associatedSliceNode->GetName() << "_Position";
    const char* posStr = d->associatedSliceNode->GetAttribute(itemPosAttrStr.str().c_str());
    d->reslicingPositionValue = posStr ? 
      atof(posStr) : 
      0.0;

    std::stringstream itemAngleAttrStr;
    itemAngleAttrStr << "PathXplorer." << newRuler->GetName() << "_" << d->associatedSliceNode->GetName() << "_Angle";
    const char* angleStr = d->associatedSliceNode->GetAttribute(itemAngleAttrStr.str().c_str());
    d->reslicingAngleValue = angleStr ? 
      atof(angleStr) : 
      0.0;

    std::stringstream itemPerpAttrStr;
    itemPerpAttrStr << "PathXplorer." << newRuler->GetName() << "_" << d->associatedSliceNode->GetName() << "_Perpendicular";
    const char* perpStr = d->associatedSliceNode->GetAttribute(itemPerpAttrStr.str().c_str());
    if (perpStr)
      {
      if (strcmp(perpStr, "ON") == 0)
	{
	d->PerpendicularRadioButton->setChecked(true);
	}
      else
	{
	d->InPlaneRadioButton->setChecked(true);
	}
      }
    else
      {
      // Perpendicular by default
      d->PerpendicularRadioButton->setChecked(true);
      }

    d->ResliceAnglePositionSlider->setValue(d->PerpendicularRadioButton->isChecked() ? 
					    d->reslicingPositionValue : 
					    d->reslicingAngleValue);
    }

  d->currentTrajectoryItem = item;
  this->isTrajectoryDrivingSliceNode();
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidget
::onResliceToggled(bool buttonStatus)
{
  Q_D(qSlicerPathXplorerReslicingWidget);

  if (!d->associatedSliceNode || !d->currentTrajectoryItem)
    {
    return;
    }

  vtkMRMLAnnotationRulerNode* ruler = d->currentTrajectoryItem->trajectoryNode();
  if (!ruler)
    {
    return;
    }
  vtkMRMLAnnotationLineDisplayNode* displayNode = ruler->GetAnnotationLineDisplayNode();

  if (buttonStatus)
    {
    // Reslice
    d->associatedSliceNode->SetAttribute("PathXplorer.DrivingPathID",ruler->GetID());
    d->associatedSliceNode->SetAttribute("PathXplorer.DrivingPathName",ruler->GetName());
    this->isTrajectoryDrivingSliceNode();
    d->ResliceAnglePositionSlider->setEnabled(1);

    this->resliceWithRuler(ruler,
			   d->associatedSliceNode,
			   d->ResliceAnglePositionSlider->value(),
			   d->PerpendicularRadioButton->isChecked());

    // Turn on slice intersection
    if (d->PerpendicularRadioButton->isChecked())
      {
      if (displayNode)
	{
	displayNode->SetSliceIntersectionVisibility(1);
	}
      }
    }
  else
    {
    // Stop reslicing
    d->ResliceAnglePositionSlider->setEnabled(0);
    if (displayNode)
      {
      displayNode->SetSliceIntersectionVisibility(0);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidget
::isTrajectoryDrivingSliceNode()
{
  Q_D(qSlicerPathXplorerReslicingWidget);

  if (!d->currentTrajectoryItem ||
      !d->associatedSliceNode)
    {
    return;
    }

  vtkMRMLAnnotationRulerNode* ruler = d->currentTrajectoryItem->trajectoryNode();
  if (!ruler)
    {
    return;
    }

  // Check if trajectory is driving reslicing
  const char* sliceNodeDrivingPathID = d->associatedSliceNode->GetAttribute("PathXplorer.DrivingPathID");
  const char* sliceNodeDrivingPathName = d->associatedSliceNode->GetAttribute("PathXplorer.DrivingPathName");
  if (sliceNodeDrivingPathName)
    {
    std::stringstream barTitle;
    barTitle << d->associatedSliceNode->GetLayoutLabel() << " : " << sliceNodeDrivingPathName;
    d->ViewLabel->setText(barTitle.str().c_str());      
    if (sliceNodeDrivingPathID && sliceNodeDrivingPathName)
      {
      if (strcmp(sliceNodeDrivingPathID, ruler->GetID()) == 0)
	{
	d->ActivateReslicingButton->setAutoFillBackground(true);
	d->ActivateReslicingButton->setStyleSheet("background-color:#00CCFF");
	}
      else
	{
	d->ActivateReslicingButton->setStyleSheet("");      
	}
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidget
::onPerpendicularToggled(bool status)
{
  Q_D(qSlicerPathXplorerReslicingWidget);

  if (!d->associatedSliceNode || !d->currentTrajectoryItem)
    {
    return;
    }
  vtkMRMLAnnotationRulerNode* ruler = d->currentTrajectoryItem->trajectoryNode();
  if (!ruler)
    {
    return;
    }
  vtkMRMLAnnotationLineDisplayNode* displayNode = ruler->GetAnnotationLineDisplayNode();

  std::stringstream attr;
  bool oldState = d->ResliceAnglePositionSlider->blockSignals(true);
  if (status)
    {
    d->DistancePositionLabel->setText("Distance:");
    d->DistanceAngleUnitLabel->setText("mm");

    d->ResliceAnglePositionSlider->setMinimum(0);
    d->ResliceAnglePositionSlider->setMaximum(100);
    d->ResliceAnglePositionSlider->blockSignals(oldState);
    d->ResliceAnglePositionSlider->setValue(d->reslicingPositionValue);

    // Turn on slice intersection
    if (d->ActivateReslicingButton->isChecked())
      {
      if (displayNode)
	{
	displayNode->SetSliceIntersectionVisibility(1);
	}
      }
    }
  else
    {
    d->DistancePositionLabel->setText("Angle:");
    d->DistanceAngleUnitLabel->setText("deg");
    d->ResliceAnglePositionSlider->setMinimum(0);
    d->ResliceAnglePositionSlider->setMaximum(360);
    d->ResliceAnglePositionSlider->blockSignals(oldState);
    d->ResliceAnglePositionSlider->setValue(d->reslicingAngleValue);

    // Turn off slice intersection
    if (d->ActivateReslicingButton->isChecked())
      {
      if (displayNode)
	{
	displayNode->SetSliceIntersectionVisibility(0);
	}
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidget
::onResliceValueChanged(int resliceValue)
{
  Q_D(qSlicerPathXplorerReslicingWidget);

  if (!d->currentTrajectoryItem)
    {
    return;
    }
  vtkMRMLAnnotationRulerNode* ruler = d->currentTrajectoryItem->trajectoryNode();
  if (!ruler)
    {
    return;
    }

  if (d->PerpendicularRadioButton->isChecked())
    {
    d->reslicingPositionValue = resliceValue;
    std::ostringstream out;
    out.setf(ios::fixed);
    out << std::setprecision(2) << ruler->GetDistanceMeasurement()*resliceValue/100;
    d->DistanceAngleValueLabel->setText(out.str().c_str());
    }
  else
    {
    d->reslicingAngleValue = resliceValue;
    d->DistanceAngleValueLabel->setNum(d->reslicingAngleValue);
    }

  if (d->ActivateReslicingButton->isChecked())
    {
    this->resliceWithRuler(ruler, 
			   d->associatedSliceNode, 
			   d->ResliceAnglePositionSlider->value(),
			   d->PerpendicularRadioButton->isChecked());
    }
  
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerReslicingWidget
::resliceWithRuler(vtkMRMLAnnotationRulerNode* ruler,
		   vtkMRMLSliceNode* viewer,
		   double angleOrPosition, 
		   bool perpendicular)
{
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
