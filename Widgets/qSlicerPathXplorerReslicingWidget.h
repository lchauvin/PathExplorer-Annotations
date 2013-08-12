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

#ifndef __qSlicerPathXplorerReslicingWidget_h
#define __qSlicerPathXplorerReslicingWidget_h

// VTK includes
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qSlicerPathXplorerModuleWidgetsExport.h"
#include "qSlicerWidget.h"

#include <qMRMLViewControllerBar.h>

class qSlicerPathXplorerReslicingWidgetPrivate;
class qSlicerPathXplorerTrajectoryItem;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLSliceNode;
class vtkMRMLAnnotationRulerNode;

class Q_SLICER_MODULE_PATHXPLORER_WIDGETS_EXPORT qSlicerPathXplorerReslicingWidget
: public qMRMLViewControllerBar
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLViewControllerBar Superclass;

  qSlicerPathXplorerReslicingWidget(vtkMRMLSliceNode* sliceNode, QWidget *parent=0);
  virtual ~qSlicerPathXplorerReslicingWidget();

public slots:
  void setTrajectoryItem(qSlicerPathXplorerTrajectoryItem* item);
  void onResliceToggled(bool buttonStatus);
  void onPerpendicularToggled(bool status);
  void onResliceValueChanged(int resliceValue);
  void resliceWithRuler(vtkMRMLAnnotationRulerNode* ruler,
			vtkMRMLSliceNode* viewer,
			bool perpendicular,
			double resliceValue);

private:
  Q_DECLARE_PRIVATE(qSlicerPathXplorerReslicingWidget);
  Q_DISABLE_COPY(qSlicerPathXplorerReslicingWidget);
};

#endif // __qSlicerPathXplorerReslicingWidget_h
