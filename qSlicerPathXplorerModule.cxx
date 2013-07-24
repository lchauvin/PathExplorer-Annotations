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

// Qt includes
#include <QtPlugin>

// PathXplorer Logic includes
#include <vtkSlicerPathXplorerLogic.h>

// PathXplorer includes
#include "qSlicerPathXplorerModule.h"
#include "qSlicerPathXplorerModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerPathXplorerModule, qSlicerPathXplorerModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPathXplorerModulePrivate
{
public:
  qSlicerPathXplorerModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPathXplorerModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerPathXplorerModulePrivate
::qSlicerPathXplorerModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPathXplorerModule methods

//-----------------------------------------------------------------------------
qSlicerPathXplorerModule
::qSlicerPathXplorerModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerPathXplorerModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerPathXplorerModule::~qSlicerPathXplorerModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerPathXplorerModule::helpText()const
{
  return QString("PathXplorer is a module designed to facilitate the creation of trajectory");
}

//-----------------------------------------------------------------------------
QString qSlicerPathXplorerModule::acknowledgementText()const
{
  return QString("It is supported by grants 5P01CA067165, 5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377," 
  "5R42CA137886, 8P41EB015898");
}

//-----------------------------------------------------------------------------
QStringList qSlicerPathXplorerModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Laurent Chauvin (SNR), Atsushi Yamada PhD (SNR), Junichi Tokuda (SNR)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerPathXplorerModule::icon()const
{
  return QIcon(":/Icons/PathXplorer.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerPathXplorerModule::categories() const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPathXplorerModule::dependencies() const
{
  return QStringList() << "Annotations";
}

//-----------------------------------------------------------------------------
void qSlicerPathXplorerModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerPathXplorerModule
::createWidgetRepresentation()
{
  return new qSlicerPathXplorerModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerPathXplorerModule::createLogic()
{
  return vtkSlicerPathXplorerLogic::New();
}
