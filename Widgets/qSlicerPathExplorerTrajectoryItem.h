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

#ifndef __qSlicerPathExplorerTrajectoryItem_h
#define __qSlicerPathExplorerTrajectoryItem_h

#include "qSlicerPathExplorerModuleWidgetsExport.h"

// Standards
#include <sstream>

// Qt includes
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTime>

// VTK includes
#include <ctkVTKObject.h>

class vtkMRMLAnnotationFiducialNode;
class vtkMRMLAnnotationRulerNode;

class Q_SLICER_MODULE_PATHEXPLORER_WIDGETS_EXPORT qSlicerPathExplorerTrajectoryItem
: public QObject, public QTableWidgetItem
{
  Q_OBJECT
  QVTK_OBJECT

public:
    qSlicerPathExplorerTrajectoryItem();
  ~qSlicerPathExplorerTrajectoryItem();

  // Set/Get for Trajectory/Fiducials
  void setEntryPoint(vtkMRMLAnnotationFiducialNode* entryPoint);
  vtkMRMLAnnotationFiducialNode* entryPoint();
  void setTargetPoint(vtkMRMLAnnotationFiducialNode* targetPoint);
  vtkMRMLAnnotationFiducialNode* targetPoint();
  void setTrajectory(vtkMRMLAnnotationRulerNode* trajectory);
  vtkMRMLAnnotationRulerNode* trajectoryNode();

  // Set/Get for Reslicing
  inline void setResliceViewer(const char* viewer);
  inline void setReslicePerpendicular(bool perp);
  inline void setResliceAngle(double angle);
  inline void setReslicePosition(double position);
  inline const char* getResliceViewer();
  inline bool getReslicePerpendicular();
  inline double getResliceAngle();
  inline double getReslicePosition();

  // Set/Get for Visualization
  inline void setDisplayPath(bool display);
  inline void setDisplayEntry(bool display);
  inline void setDisplayTarget(bool display);
  inline bool getDisplayPath();
  inline bool getDisplayEntry();
  inline bool getDisplayTarget();

  // Set/Get for Projection
  inline void setProjectionPath(bool projection);
  inline void setProjectionEntry(bool projection);
  inline void setProjectionTarget(bool projection);
  inline bool getProjectionPath();
  inline bool getProjectionEntry();
  inline bool getProjectionTarget();

public slots:
    void updateItem();
    void trajectoryModified();
    void trajectoryDisplayModified();

private:
    // Trajectory/Fiducials
    vtkMRMLAnnotationFiducialNode* EntryPoint;
    vtkMRMLAnnotationFiducialNode* TargetPoint;
    vtkMRMLAnnotationRulerNode* Trajectory;

    // Reslicing
    std::string ResliceViewer;
    bool ReslicePerpendicular;
    double ResliceAngle;
    double ReslicePosition;
    
    // Visualization
    bool DisplayPath;
    bool DisplayEntry;
    bool DisplayTarget;

    // Projection
    bool ProjectionPath;
    bool ProjectionEntry;
    bool ProjectionTarget;

signals:
    void resliceModified();
    void displayModified();
    void projectionModified();
};

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
setResliceViewer(const char* viewer)
{
  if (viewer)
    {
    this->ResliceViewer.assign(viewer);
    //emit resliceModified();
    }
}

//----------------------------------------------------------------------------
const char* qSlicerPathExplorerTrajectoryItem::
getResliceViewer()
{
  return this->ResliceViewer.c_str();
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
setReslicePerpendicular(bool perp)
{
  this->ReslicePerpendicular = perp;
  emit resliceModified();
}

//----------------------------------------------------------------------------
bool qSlicerPathExplorerTrajectoryItem::
getReslicePerpendicular()
{
  return this->ReslicePerpendicular;
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
setResliceAngle(double angle)
{
  if (angle >= -180 && angle <= 180)
    {
    this->ResliceAngle = angle;
    }
  else
    {
    this->ResliceAngle = 0.0;
    }
  emit resliceModified();
}

//----------------------------------------------------------------------------
double qSlicerPathExplorerTrajectoryItem::
getResliceAngle()
{
  return this->ResliceAngle;
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
setReslicePosition(double position)
{
  if (position >= 0 && position <= 100)
    {
    this->ReslicePosition = position;
    }
  else
    {
    this->ReslicePosition = 0.0;
    }
  emit resliceModified();
}

//----------------------------------------------------------------------------
double qSlicerPathExplorerTrajectoryItem::
getReslicePosition()
{
  return this->ReslicePosition;
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
setDisplayPath(bool display)
{
  this->DisplayPath = display;
  emit displayModified();
}

//----------------------------------------------------------------------------
bool qSlicerPathExplorerTrajectoryItem::
getDisplayPath()
{
  return this->DisplayPath;
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
setDisplayEntry(bool display)
{
  this->DisplayEntry = display;
  emit displayModified();
}

//----------------------------------------------------------------------------
bool qSlicerPathExplorerTrajectoryItem::
getDisplayEntry()
{
  return this->DisplayEntry;
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
setDisplayTarget(bool display)
{
  this->DisplayTarget = display;
  emit displayModified();
}

//----------------------------------------------------------------------------
bool qSlicerPathExplorerTrajectoryItem::
getDisplayTarget()
{
  return this->DisplayTarget;
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
setProjectionPath(bool projection)
{
  this->ProjectionPath = projection;
  emit projectionModified();
}

//----------------------------------------------------------------------------
bool qSlicerPathExplorerTrajectoryItem::
getProjectionPath()
{
  return this->ProjectionPath;
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
setProjectionEntry(bool projection)
{
  this->ProjectionEntry = projection;
  emit projectionModified();
}

//----------------------------------------------------------------------------
bool qSlicerPathExplorerTrajectoryItem::
getProjectionEntry()
{
  return this->ProjectionEntry;
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryItem::
setProjectionTarget(bool projection)
{
  this->ProjectionTarget = projection;
  emit projectionModified();
}

//----------------------------------------------------------------------------
bool qSlicerPathExplorerTrajectoryItem::
getProjectionTarget()
{
  return this->ProjectionTarget;
}

#endif
