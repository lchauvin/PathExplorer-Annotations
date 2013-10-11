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

#ifndef __qSlicerPathExplorerTableWidget_h
#define __qSlicerPathExplorerTableWidget_h

// VTK includes
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qSlicerPathExplorerModuleWidgetsExport.h"
#include "qSlicerWidget.h"

// Qt includes
#include <QTableWidget>

class qSlicerPathExplorerTableWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLAnnotationHierarchyNode;
class vtkMRMLAnnotationFiducialNode;

class Q_SLICER_MODULE_PATHEXPLORER_WIDGETS_EXPORT qSlicerPathExplorerTableWidget
  : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerWidget Superclass;
  qSlicerPathExplorerTableWidget(QWidget *parent=0);
  virtual ~qSlicerPathExplorerTableWidget();

  QTableWidget* getTableWidget();
  void setSelectedHierarchyNode(vtkMRMLAnnotationHierarchyNode* selectedNode);
  vtkMRMLAnnotationHierarchyNode* selectedHierarchyNode();
  bool addButtonStatus;
  void setAddButtonState(bool state);

public slots:
  void onAddButtonToggled(bool pushed);
  void onDeleteButtonClicked();
  void onClearButtonClicked();
  void onSelectionChanged();
  void onCellChanged(int row, int column);

protected:
  QScopedPointer<qSlicerPathExplorerTableWidgetPrivate> d_ptr;
  
private:
  Q_DECLARE_PRIVATE(qSlicerPathExplorerTableWidget);
  Q_DISABLE_COPY(qSlicerPathExplorerTableWidget);

signals:
  void itemDeleted(vtkMRMLAnnotationFiducialNode*);
  void addButtonToggled(bool);
};

#endif // __qSlicerPathExplorerTableWidget_h
