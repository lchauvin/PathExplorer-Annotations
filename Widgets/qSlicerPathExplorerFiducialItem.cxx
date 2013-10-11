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
#include "qSlicerPathExplorerFiducialItem.h"

// --------------------------------------------------------------------------
qSlicerPathExplorerFiducialItem
::qSlicerPathExplorerFiducialItem() : QTableWidgetItem()
{
  this->FiducialNode = NULL;
}

// --------------------------------------------------------------------------
qSlicerPathExplorerFiducialItem::
~qSlicerPathExplorerFiducialItem()
{
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerFiducialItem::
setFiducialNode(vtkMRMLAnnotationFiducialNode* fiducialNode)
{
  if (fiducialNode)
    {
    qvtkReconnect(this->FiducialNode, fiducialNode, vtkCommand::ModifiedEvent,
                  this, SLOT(updateItem()));
    this->FiducialNode = fiducialNode;
    this->updateItem();
    }
}

// --------------------------------------------------------------------------
vtkMRMLAnnotationFiducialNode* qSlicerPathExplorerFiducialItem::
getFiducialNode()
{
  return this->FiducialNode;
}

// --------------------------------------------------------------------------
void qSlicerPathExplorerFiducialItem::
updateItem()
{
  if (!this->FiducialNode)
    {
    return;
    }

  double targetPosition[4] = {0,0,0,0};
  this->FiducialNode->GetFiducialWorldCoordinates(targetPosition);

  double itemRow = this->row();
  QTableWidget* tableWidget = this->tableWidget();

  if (!tableWidget)
    {
    return;
    }

  if (!tableWidget->item(itemRow,0) ||
      !tableWidget->item(itemRow,1) ||
      !tableWidget->item(itemRow,2) ||
      !tableWidget->item(itemRow,3) ||
      !tableWidget->item(itemRow,4))
    {
    return;
    }

  // Update text
  std::stringstream fiduPos;
  fiduPos.precision(2);
  fiduPos.setf(std::ios::fixed);

  // Name
  QString pointName = tableWidget->item(itemRow,0)->text();
  QString fiducialName(this->FiducialNode->GetName());

  if (pointName.isEmpty())
    {
    // No name set. Use fiducial name.
    tableWidget->item(itemRow,0)->setText(fiducialName.toStdString().c_str());
    }
  else
    {
    if (pointName.compare(fiducialName) != 0)
      {
      // Item name is different from fiducial name
      // Update fiducial name
      this->FiducialNode->SetName(pointName.toStdString().c_str());
      }
    }

  // Temporary block signals to prevent loop
  // fiducial updating cells, cells updating fiducial position
  bool oldState = tableWidget->blockSignals(true);

  //   R
  fiduPos << targetPosition[0];
  tableWidget->item(itemRow, 1)->setText(fiduPos.str().c_str());
  fiduPos.str(std::string());

  //   A
  fiduPos << targetPosition[1];
  tableWidget->item(itemRow, 2)->setText(fiduPos.str().c_str());
  fiduPos.str(std::string());

  //   S
  fiduPos << targetPosition[2];
  tableWidget->item(itemRow, 3)->setText(fiduPos.str().c_str());
  fiduPos.str(std::string());

  // Time
  tableWidget->item(itemRow,4)->setText(QTime::currentTime().toString());

  // Restore signals
  tableWidget->blockSignals(oldState);
}

