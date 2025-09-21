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

DetectorConstruction::DetectorConstruction()
 : fWorld(nullptr), fDet1Physical(nullptr), fDet2Physical(nullptr),
   fDet2Angle(0.), fDet2Radius(17.4*cm) {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume* DetectorConstruction::Construct() {
    auto nist = G4NistManager::Instance();

    // --- World ---
    auto worldMat = nist->FindOrBuildMaterial("G4_AIR");
    auto worldSize = 2.0*m;
    auto worldSolid = new G4Box("World", worldSize/2, worldSize/2, worldSize/2);
    auto worldLogical = new G4LogicalVolume(worldSolid, worldMat, "World");
    fWorld = new G4PVPlacement(0, {}, worldLogical, "World", 0, false, 0);
    worldLogical->SetVisAttributes(G4VisAttributes::GetInvisible());

    // --- Source (Na-22) ---
    auto Na22 = nist->FindOrBuildMaterial("G4_Galactic");
    auto sourceSolid = new G4Tubs("Source", 0., 2*mm, 2*mm, 0., 360.*deg);
    auto sourceLogical = new G4LogicalVolume(sourceSolid, Na22, "Source");
    new G4PVPlacement(0, {0,0,0}, sourceLogical, "Source", worldLogical, false, 0);
    sourceLogical->SetVisAttributes(new G4VisAttributes(G4Colour::Yellow()));

    // --- Lead Shield with Slit ---
    auto lead = nist->FindOrBuildMaterial("G4_Pb");
    auto shield_thick = 20*mm;
    auto shield_h = 40*mm;
    auto shield_w = 40*mm;
    auto slit_w = 2*mm;

    auto block1 = new G4Box("Block1", (shield_w-slit_w)/2, shield_thick/2, shield_h/2);
    auto block2 = new G4Box("Block2", (shield_w-slit_w)/2, shield_thick/2, shield_h/2);
    auto block1LV = new G4LogicalVolume(block1, lead, "Block1LV");
    auto block2LV = new G4LogicalVolume(block2, lead, "Block2LV");

    new G4PVPlacement(0, G4ThreeVector(-(slit_w+(shield_w-slit_w))/2,0,0),
                      block1LV,"Block1",worldLogical,false,0);
    new G4PVPlacement(0, G4ThreeVector((slit_w+(shield_w-slit_w))/2,0,0),
                      block2LV,"Block2",worldLogical,false,0);
    block1LV->SetVisAttributes(new G4VisAttributes(G4Colour::Gray()));
    block2LV->SetVisAttributes(new G4VisAttributes(G4Colour::Gray()));

    // --- First Detector (upright, along X axis) ---
    auto detMat = nist->FindOrBuildMaterial("G4_Si");
    auto detSolid = new G4Tubs("Det",0., 25*mm, 25*mm, 0., 360*deg);
    auto det1Logical = new G4LogicalVolume(detSolid, detMat, "Det1");
    auto rot1 = new G4RotationMatrix();
    rot1->rotateX(90*deg);  // upright
    fDet1Physical = new G4PVPlacement(rot1, G4ThreeVector(0,0,124*mm), det1Logical,
                                     "Det1", worldLogical, false, 0);
    det1Logical->SetVisAttributes(new G4VisAttributes(G4Colour::Blue()));

    // --- Second Detector (perpendicular, rotated along circle) ---
    auto det2Logical = new G4LogicalVolume(detSolid, detMat, "Det2");
    auto rot2 = new G4RotationMatrix();
    rot2->rotateY(90*deg);  // initial perpendicular orientation
    fDet2Physical = new G4PVPlacement(rot2, G4ThreeVector(fDet2Radius,0,0),
                                     det2Logical, "Det2", worldLogical, false, 0);
    det2Logical->SetVisAttributes(new G4VisAttributes(G4Colour::Red()));

    return fWorld;
}
void DetectorConstruction::SetDet2Angle(G4double angleDeg) {
    fDet2Angle = angleDeg;

    G4double rad = fDet2Radius;                // radius of circle
    G4double angleRad = fDet2Angle*deg;

    // Compute new position in X-Z plane (circle around Det1)
    G4double x = rad * std::cos(angleRad);
    G4double z = 124*mm + rad * std::sin(angleRad);  // Det1 at z=124 mm

    // Rotation to face Det1 (keep cylinder axis perpendicular to Det1)
    G4RotationMatrix* rot = new G4RotationMatrix();
    rot->rotateX(90*deg);            // make cylinder perpendicular
    rot->rotateY(-fDet2Angle*deg);   // rotate around Y to face Det1

    fDet2Physical->SetTranslation(G4ThreeVector(x, 0, z));
    fDet2Physical->SetRotation(rot);
}

