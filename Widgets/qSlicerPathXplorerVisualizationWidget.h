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

#ifndef __qSlicerPathXplorerVisualizationWidget_h
#define __qSlicerPathXplorerVisualizationWidget_h

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerPathXplorerModuleWidgetsExport.h"
#include "qSlicerWidget.h"

// MRML includes
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLLayoutLogic.h"
#include "vtkMRMLLayoutNode.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLSliceNode.h"

// VTK includes
#include <ctkVTKObject.h>
#include <vtkCollection.h>
#include <vtkMath.h>

// PathXplorer includes
#include "qSlicerPathXplorerTrajectoryItem.h"

class qSlicerPathXplorerVisualizationWidgetPrivate;

class Q_SLICER_MODULE_PATHXPLORER_WIDGETS_EXPORT qSlicerPathXplorerVisualizationWidget
  : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
    typedef qSlicerWidget Superclass;
    typedef std::map<std::string,vtkMRMLSliceNode*>::iterator SliceNodeIt;
    qSlicerPathXplorerVisualizationWidget(QWidget *parent=0);
    virtual ~qSlicerPathXplorerVisualizationWidget();

    void setTrajectoryItem(qSlicerPathXplorerTrajectoryItem* item);
    qSlicerPathXplorerTrajectoryItem* getTrajectoryItem();

    enum
    {
      TRANSVERSE,
      PERPENDICULAR
    };

protected:
    QScopedPointer<qSlicerPathXplorerVisualizationWidgetPrivate> d_ptr;

public slots:
  void onLayoutChanged(int newLayout);
  void onMRMLSceneChanged(vtkMRMLScene* newScene);
  
  void populateLayouts();
  void updateResliceMode(int mode);
  void updateReslice();
  void updateDisplay();
  void updateProjection();

  void onResliceChanged(QString newViewers);
  void onPerpendicularChanged(int state);
  void onAngleChanged(double angle);

  void onDisplayPathChanged(int state);
  void onDisplayEntryChanged(int state);
  void onDisplayTargetChanged(int state);

  void onProjectionPathChanged(int state);
  void onProjectionEntryChanged(int state);
  void onProjectionTargetChanged(int state);

  void updateReslicing();
  void resliceWithRuler(vtkMRMLAnnotationRulerNode* ruler,
			vtkMRMLSliceNode* viewer,
			double angleOrPosition,
			bool perpendicular);

signals:
  void entryDisplayModified(vtkMRMLAnnotationFiducialNode* node, bool displayValue);
  void targetDisplayModified(vtkMRMLAnnotationFiducialNode* node, bool displayValue);
  void entryProjectionModified(vtkMRMLAnnotationFiducialNode* node, bool projectionValue);
  void targetProjectionModified(vtkMRMLAnnotationFiducialNode* node, bool projectionValue);

private:
  Q_DECLARE_PRIVATE(qSlicerPathXplorerVisualizationWidget);
  Q_DISABLE_COPY(qSlicerPathXplorerVisualizationWidget);
};

#endif
