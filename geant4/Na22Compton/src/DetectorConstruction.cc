// DetectorConstruction.cc (XY Plane Configuration)
#include "DetectorConstruction.hh"

#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4SDManager.hh"

#include "DetectorSD.hh"
#include "DetectorMessenger.hh"

#include <cmath>

DetectorConstruction::DetectorConstruction()
 : fWorld(nullptr), fWorldLogical(nullptr),
   fDet1Physical(nullptr), fDet1Logical(nullptr),
   fDet2Physical(nullptr), fDet2Logical(nullptr),
   fDet1SD(nullptr), fDet2SD(nullptr),
   fDet2Angle(0.), fDet2Radius(17.4*cm),
   fMessenger(nullptr) {
    
    // Create messenger
    fMessenger = new DetectorMessenger(this);
}

DetectorConstruction::~DetectorConstruction() {
    delete fMessenger;
    // Note: SDs are managed by G4SDManager, don't delete them here
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
    auto nist = G4NistManager::Instance();

    // --- World ---
    auto worldMat = nist->FindOrBuildMaterial("G4_AIR");
    auto worldSize = 2.0*m;
    auto worldSolid = new G4Box("World", worldSize/2, worldSize/2, worldSize/2);
    fWorldLogical = new G4LogicalVolume(worldSolid, worldMat, "World");
    fWorld = new G4PVPlacement(nullptr, G4ThreeVector(), fWorldLogical, "World", nullptr, false, 0);
    fWorldLogical->SetVisAttributes(G4VisAttributes::GetInvisible());

    // --- Source (Na-22) positioned between origin and Det1 ---
    auto Na22 = nist->FindOrBuildMaterial("G4_Galactic");
    auto sourceSolid = new G4Tubs("Source", 0., 2*mm, 2*mm, 0., 360.*deg);
    auto sourceLogical = new G4LogicalVolume(sourceSolid, Na22, "Source");
    // Source positioned closer to Det1, with cylinder axis along z
    new G4PVPlacement(nullptr, G4ThreeVector(-6*cm, 0, 0),
                      sourceLogical, "Source", fWorldLogical, false, 0);
    sourceLogical->SetVisAttributes(new G4VisAttributes(G4Colour::Yellow()));

    // --- Lead Shield with Slit positioned between source and Det1 ---
    auto lead = nist->FindOrBuildMaterial("G4_Pb");
    G4double shield_thick = 2*mm;
    G4double shield_h = 40*mm;
    G4double shield_w = 40*mm;
    G4double slit_w = 2*mm;

    // Shield oriented to create a slit along x-direction (beam travels in +x)
    auto block1 = new G4Box("Block1", shield_thick/2, (shield_w-slit_w)/2, shield_h/2);
    auto block2 = new G4Box("Block2", shield_thick/2, (shield_w-slit_w)/2, shield_h/2);
    auto block1LV = new G4LogicalVolume(block1, lead, "Block1LV");
    auto block2LV = new G4LogicalVolume(block2, lead, "Block2LV");

    // Place blocks to create horizontal slit (opening in y-direction)
    new G4PVPlacement(nullptr, G4ThreeVector(-3*cm, -(slit_w+(shield_w-slit_w))/2, 0),
                      block1LV, "Block1", fWorldLogical, false, 0);
    new G4PVPlacement(nullptr, G4ThreeVector(-3*cm, (slit_w+(shield_w-slit_w))/2, 0),
                      block2LV, "Block2", fWorldLogical, false, 0);
    block1LV->SetVisAttributes(new G4VisAttributes(G4Colour::Gray()));
    block2LV->SetVisAttributes(new G4VisAttributes(G4Colour::Gray()));

    // --- Detector Material: NaI (no Tl doping) ---
    auto elNa = nist->FindOrBuildElement("Na");
    auto elI  = nist->FindOrBuildElement("I");
    auto NaI = new G4Material("NaI", 3.67*g/cm3, 2);
    NaI->AddElement(elNa, 1);
    NaI->AddElement(elI, 1);

    // --- First Detector (at origin, lying flat in xy plane) ---
    auto detSolid = new G4Tubs("Det", 0., 25*mm, 25*mm, 0., 360*deg);
    fDet1Logical = new G4LogicalVolume(detSolid, NaI, "Det1");
    // No rotation needed - cylinder axis already along z
    auto rot1 = new G4RotationMatrix();
    fDet1Physical = new G4PVPlacement(rot1, G4ThreeVector(0,0,0),
                                      fDet1Logical, "Det1", fWorldLogical, false, 0);
    fDet1Logical->SetVisAttributes(new G4VisAttributes(G4Colour::Blue()));

    // --- Second Detector with perpendicular axis in xy plane ---
    G4double angleRad = fDet2Angle * CLHEP::deg;
    // Position in xy plane at specified radius and angle
    G4ThreeVector pos(fDet2Radius * std::cos(angleRad),
                      fDet2Radius * std::sin(angleRad),
                      0);  // z = 0 keeps it in xy plane

    // Make Det2 axis perpendicular to Det1 axis
    // Det1 has axis along z, so Det2 should have axis in xy plane
    // Point Det2 axis toward the center (Det1 position)
    G4ThreeVector det1Pos(0, 0, 0);
    G4ThreeVector towardCenter = (det1Pos - pos).unit();
    
    // Create rotation to orient cylinder axis toward center
    G4RotationMatrix* rot2 = new G4RotationMatrix();
    
    // Default cylinder axis is along z, we want it along towardCenter direction
    G4ThreeVector zAxis(0, 0, 1);
    G4ThreeVector rotationAxis = zAxis.cross(towardCenter);
    
    if (rotationAxis.mag() > 1e-6) {
        G4double rotationAngle = std::acos(zAxis.dot(towardCenter));
        rot2->rotate(rotationAngle, rotationAxis.unit());
    }
    
    fDet2Logical = new G4LogicalVolume(detSolid, NaI, "Det2");
    fDet2Physical = new G4PVPlacement(rot2, pos, fDet2Logical, "Det2", fWorldLogical, false, 0, true);
    fDet2Logical->SetVisAttributes(new G4VisAttributes(G4Colour::Red()));

    return fWorld;
}

void DetectorConstruction::ConstructSDandField() {
    // Create sensitive detectors
    fDet1SD = new DetectorSD("Det1SD");
    fDet2SD = new DetectorSD("Det2SD");

    // Register SDs with SDManager
    G4SDManager::GetSDMpointer()->AddNewDetector(fDet1SD);
    G4SDManager::GetSDMpointer()->AddNewDetector(fDet2SD);

    // Attach SDs to logical volumes
    fDet1Logical->SetSensitiveDetector(fDet1SD);
    fDet2Logical->SetSensitiveDetector(fDet2SD);
}

// Helper: remove and delete a PV safely
static void SafeDeletePV(G4VPhysicalVolume* & pv) {
    if(!pv) return;
    if (pv->GetMotherLogical()) {
        pv->GetMotherLogical()->RemoveDaughter(pv);
    }
    delete pv;
    pv = nullptr;
}

void DetectorConstruction::SetDet2Angle(G4double angleDeg)
{
    fDet2Angle = angleDeg;

    if (!fDet1Physical || !fDet2Logical || !fWorldLogical) return;

    G4ThreeVector det1Pos = fDet1Physical->GetTranslation();

    // Calculate new position in xy plane
    G4double angleRad = fDet2Angle * CLHEP::deg;
    G4ThreeVector pos(
        det1Pos.x() + fDet2Radius * std::cos(angleRad),
        det1Pos.y() + fDet2Radius * std::sin(angleRad),
        det1Pos.z()  // Keep same z as Det1 (should be 0)
    );

    // Orient Det2 axis perpendicular to Det1 (toward center)
    G4ThreeVector towardCenter = (det1Pos - pos).unit();
    
    G4RotationMatrix* rot = new G4RotationMatrix();
    
    // Default cylinder axis is along z, rotate to point toward center
    G4ThreeVector zAxis(0, 0, 1);
    G4ThreeVector rotationAxis = zAxis.cross(towardCenter);
    
    if (rotationAxis.mag() > 1e-6) {
        G4double rotationAngle = std::acos(zAxis.dot(towardCenter));
        rot->rotate(rotationAngle, rotationAxis.unit());
    }

    SafeDeletePV(fDet2Physical);
    fDet2Physical = new G4PVPlacement(rot, pos, fDet2Logical, "Det2", fWorldLogical, false, 0, true);

    G4RunManager::GetRunManager()->GeometryHasBeenModified();
    G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/update");
}
