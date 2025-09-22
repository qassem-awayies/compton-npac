#pragma once

#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "globals.hh"

class DetectorSD;
class DetectorMessenger;

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
    DetectorConstruction();
    ~DetectorConstruction() override;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

    // Rotate second detector around first detector
    void SetDet2Angle(G4double angleDeg);

    // Getters for SDs
    DetectorSD* GetDet1SD() const { return fDet1SD; }
    DetectorSD* GetDet2SD() const { return fDet2SD; }

private:
    // World
    G4VPhysicalVolume* fWorld;
    G4LogicalVolume* fWorldLogical;

    // Detectors
    G4VPhysicalVolume* fDet1Physical;
    G4LogicalVolume* fDet1Logical;

    G4VPhysicalVolume* fDet2Physical;
    G4LogicalVolume* fDet2Logical;

    // Geometry parameters
    G4double fDet2Angle;   // in degrees
    G4double fDet2Radius;  // distance from Det1

    // Sensitive detectors
    DetectorSD* fDet1SD;
    DetectorSD* fDet2SD;

    // Messenger
    DetectorMessenger* fMessenger;
};
