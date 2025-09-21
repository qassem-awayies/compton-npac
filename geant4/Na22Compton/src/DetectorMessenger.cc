#include "DetectorMessenger.hh"
#include "DetectorConstruction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

DetectorMessenger::DetectorMessenger(DetectorConstruction* det)
: fDetector(det)
{
    fDetDir = new G4UIdirectory("/det/");
    fDetDir->SetGuidance("Detector control commands");

    fDet2AngleCmd = new G4UIcmdWithADoubleAndUnit("/det/setDet2Angle", this);
    fDet2AngleCmd->SetGuidance("Set rotation angle of second detector around Y axis");
    fDet2AngleCmd->SetParameterName("angle", false);
    fDet2AngleCmd->SetUnitCategory("Angle");
    fDet2AngleCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

DetectorMessenger::~DetectorMessenger()
{
    delete fDet2AngleCmd;
    delete fDetDir;
}

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == fDet2AngleCmd) {
        G4double ang = fDet2AngleCmd->GetNewDoubleValue(newValue);
        fDetector->SetDet2Angle(ang);  // call the public setter
    }
}

