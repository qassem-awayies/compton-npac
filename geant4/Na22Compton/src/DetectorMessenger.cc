#include "DetectorMessenger.hh"
#include "DetectorConstruction.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

DetectorMessenger::DetectorMessenger(DetectorConstruction* det)
 : fDetector(det) {
    fDet2AngleCmd = new G4UIcmdWithADoubleAndUnit("/detector/setDet2Angle", this);
    fDet2AngleCmd->SetGuidance("Set Detector 2 angle (deg, before run).");
    fDet2AngleCmd->SetParameterName("angle", false);
    fDet2AngleCmd->SetUnitCategory("Angle");
    fDet2AngleCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

DetectorMessenger::~DetectorMessenger() {
    delete fDet2AngleCmd;
}

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fDet2AngleCmd) {
        fDetector->SetDet2Angle(fDet2AngleCmd->GetNewDoubleValue(newValue));
    }
}
