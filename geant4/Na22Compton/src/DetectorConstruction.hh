#pragma once
#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
    DetectorConstruction();
    ~DetectorConstruction() override;

    G4VPhysicalVolume* Construct() override;

    // Rotate second detector along a circle around Det1
    void SetDet2Angle(G4double angleDeg);

private:
    G4VPhysicalVolume* fWorld;
    G4VPhysicalVolume* fDet1Physical;
    G4VPhysicalVolume* fDet2Physical;

    G4double fDet2Angle;  // in degrees
    G4double fDet2Radius; // fixed radius from Det1
};

