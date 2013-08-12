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

#ifndef __qSlicerPathXplorerModuleWidget_h
#define __qSlicerPathXplorerModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerPathXplorerModuleExport.h"

// VTK includes
#include <ctkVTKObject.h>

// Qt includes
#include <QTableWidget>

class qSlicerPathXplorerModuleWidgetPrivate;
class vtkMRMLAnnotationFiducialNode;
class vtkMRMLNode;
class vtkMRMLSliceNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_PATHXPLORER_EXPORT qSlicerPathXplorerModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerPathXplorerModuleWidget(QWidget *parent=0);
  virtual ~qSlicerPathXplorerModuleWidget();

public slots:
  void onEntryListNodeChanged(vtkMRMLNode* newList);
  void onTargetListNodeChanged(vtkMRMLNode* newList);
  void onItemChanged(QTableWidgetItem *item);
  void refreshEntryView();
  void refreshTargetView();
  void onAddButtonClicked();
  void onDeleteButtonClicked();
  void onUpdateButtonClicked();
  void onClearButtonClicked();
  void onTrajectoryListNodeChanged(vtkMRMLNode* newList);
  void onTrajectorySelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void onMRMLSceneChanged(vtkMRMLScene* newScene);
  void addNewReslicer(vtkMRMLSliceNode* sliceNode);
  void onTargetSelectionChanged();
  void onEntrySelectionChanged();
  void onTrajectoryCellChanged(int row, int column);
  void onEntryPointDeleted(vtkMRMLAnnotationFiducialNode* itemDeleted);
  void onTargetPointDeleted(vtkMRMLAnnotationFiducialNode* itemDeleted);
  void deleteTrajectory(int trajectoryRow);
  void onEntryDisplayModified(vtkMRMLAnnotationFiducialNode* modifiedNode, bool visibility);
  void onTargetDisplayModified(vtkMRMLAnnotationFiducialNode* modifiedNode, bool visibility);
  void onEntryProjectionModified(vtkMRMLAnnotationFiducialNode* modifiedNode, bool projection);
  void onTargetProjectionModified(vtkMRMLAnnotationFiducialNode* modifiedNode, bool projection);
  void onEntryTableWidgetAddButtonToggled(bool state);
  void onTargetTableWidgetAddButtonToggled(bool state);

protected:
  QScopedPointer<qSlicerPathXplorerModuleWidgetPrivate> d_ptr;
  
  virtual void setup();
  void addNewFiducialItem(QTableWidget* tableWidget, vtkMRMLAnnotationFiducialNode* fiducialNode);
  void addNewRulerItem(vtkMRMLAnnotationFiducialNode* entryPoint, vtkMRMLAnnotationFiducialNode* targetPoint);

private:
  Q_DECLARE_PRIVATE(qSlicerPathXplorerModuleWidget);
  Q_DISABLE_COPY(qSlicerPathXplorerModuleWidget);
};

#endif
