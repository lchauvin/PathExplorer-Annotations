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
#include "qSlicerApplication.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerPathExplorerTrajectoryItem.h"

// MRML includes
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLScene.h"

// --------------------------------------------------------------------------
qSlicerPathExplorerTrajectoryItem
::qSlicerPathExplorerTrajectoryItem() : QTableWidgetItem()
{
  // Trajectory/Fiducials
  this->EntryPoint  = NULL;
  this->TargetPoint = NULL;
  this->Trajectory  = NULL;

  // Reslice
  this->ResliceViewer.assign("None");
  this->ReslicePerpendicular = false;
  this->ResliceAngle = 0.0;
  this->ReslicePosition = 0.0;

  // Visualization
  this->DisplayPath = true;
  this->DisplayEntry = true;
  this->DisplayTarget = true;

  // Projection
  this->ProjectionPath = false;
  this->ProjectionEntry = false;
  this->ProjectionTarget = false;
}

// --------------------------------------------------------------------------
qSlicerPathExplorerTrajectoryItem::
~qSlicerPathExplorerTrajectoryItem()
{
  if (this->Trajectory)
    {
    this->Trajectory->Delete();
    }
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
setEntryPoint(vtkMRMLAnnotationFiducialNode* entryPoint)
{
  if (entryPoint && entryPoint != this->EntryPoint)
    {
    qvtkReconnect(this->EntryPoint, entryPoint, vtkCommand::ModifiedEvent,
                  this, SLOT(updateItem()));
    entryPoint->GetAnnotationPointDisplayNode()->SetGlyphType(vtkMRMLAnnotationPointDisplayNode::Sphere3D);
    this->EntryPoint = entryPoint;
    this->updateItem();
    }
}

// --------------------------------------------------------------------------
vtkMRMLAnnotationFiducialNode* qSlicerPathExplorerTrajectoryItem::
entryPoint()
{
  return this->EntryPoint;
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
setTargetPoint(vtkMRMLAnnotationFiducialNode* targetPoint)
{
  if (targetPoint && targetPoint != this->TargetPoint)
    {
    qvtkReconnect(this->TargetPoint, targetPoint, vtkCommand::ModifiedEvent,
                  this, SLOT(updateItem()));
    targetPoint->GetAnnotationPointDisplayNode()->SetGlyphType(vtkMRMLAnnotationPointDisplayNode::Sphere3D);
    this->TargetPoint = targetPoint;
    this->updateItem();
    }
}

// --------------------------------------------------------------------------
vtkMRMLAnnotationFiducialNode* qSlicerPathExplorerTrajectoryItem::
targetPoint()
{
  return this->TargetPoint;
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
setTrajectory(vtkMRMLAnnotationRulerNode* trajectory)
{
  if (trajectory)
    {
    qvtkReconnect(this->Trajectory, trajectory, vtkCommand::ModifiedEvent,
                  this, SLOT(updateItem()));
    this->Trajectory = trajectory;
    this->updateItem();
    }
}

// --------------------------------------------------------------------------
vtkMRMLAnnotationRulerNode* qSlicerPathExplorerTrajectoryItem::
trajectoryNode()
{
  return this->Trajectory;
}


// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
updateItem()
{
  if (!this->EntryPoint || !this->TargetPoint)
    {
    return;
    }

  if (!this->Trajectory)
    {
    // Points are set but trajectory doesn't exist yet. Create a new one.
    this->Trajectory = vtkMRMLAnnotationRulerNode::New();
    this->Trajectory->Initialize(qSlicerApplication::application()->mrmlScene());
    }

  // Set ruler points
  // Convention: Point1 -> Entry Point
  //             Point2 -> Target Point
  double targetPosition[4] = {0,0,0,0};
  double entryPosition[4] = {0,0,0,0};

  this->TargetPoint->GetFiducialWorldCoordinates(targetPosition);
  this->EntryPoint->GetFiducialWorldCoordinates(entryPosition);

  this->Trajectory->SetPositionWorldCoordinates1(entryPosition);
  this->Trajectory->SetPositionWorldCoordinates2(targetPosition);

  // Update table widget
  double itemRow = this->row();
  QTableWidget* tableWidget = this->tableWidget();
  if (!tableWidget)
    {
    return;
    }

  // -- Update ruler name
  QString itemName = tableWidget->item(itemRow,0)->text();
  QString rulerName(this->Trajectory->GetName());
  if (itemName.isEmpty())
    {
    // No name set. Use ruler name.
    tableWidget->item(itemRow,0)->setText(rulerName.toStdString().c_str());
    }
  else
    {
    if (itemName.compare(rulerName) != 0)
      {
      // Item name is different from ruler name
      // Update ruler name
      this->Trajectory->SetName(itemName.toStdString().c_str());
      }
    }

  // -- Update entry cell
  tableWidget->item(itemRow,1)->setText(this->EntryPoint->GetName());

  // -- Update target cell
  tableWidget->item(itemRow,2)->setText(this->TargetPoint->GetName());

  // Update fiducials when ruler is moved to keep them linked
  qvtkConnect(this->Trajectory, vtkCommand::ModifiedEvent,
              this, SLOT(trajectoryModified()));
  qvtkConnect(this->Trajectory, vtkMRMLDisplayableNode::DisplayModifiedEvent,
              this, SLOT(trajectoryDisplayModified()));
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
trajectoryModified()
{
  if (!this->EntryPoint || !this->TargetPoint ||
      !this->Trajectory)
    {
    return;
    }

  // Update fiducials
  double targetPos[4] = {0,0,0,0};
  double entryPos[4] = {0,0,0,0};
  this->Trajectory->GetPositionWorldCoordinates1(entryPos);
  this->Trajectory->GetPositionWorldCoordinates2(targetPos);

  this->TargetPoint->SetFiducialWorldCoordinates(targetPos);
  this->EntryPoint->SetFiducialWorldCoordinates(entryPos);
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
trajectoryDisplayModified()
{
  if(!this->Trajectory)
    {
    return;
    }

  QTableWidget* tableWidget = this->tableWidget();
  if (!tableWidget)
    {
    return;
    }

  // Update visibility
  if (this->Trajectory->GetDisplayVisibility())
    {
    tableWidget->item(this->row(), 3)->setCheckState(Qt::Checked);
    }
  else
    {
    tableWidget->item(this->row(), 3)->setCheckState(Qt::Unchecked);
    }

}

