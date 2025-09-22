// PrimaryGeneratorAction.cc - Na-22 Source Implementation
#include "PrimaryGeneratorAction.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction()
: G4VUserPrimaryGeneratorAction(),
  fParticleGun(0)
{
  G4int n_particle = 1;
  fParticleGun = new G4ParticleGun(n_particle);

  // Default particle kinematics
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName;
  G4ParticleDefinition* particle = particleTable->FindParticle(particleName="gamma");
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticleEnergy(511.*keV);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  // Na-22 source position (from DetectorConstruction)
  G4ThreeVector sourcePos(-6*cm, 0, 0);
  
  // Add small random spread within source volume (2mm radius)
  G4double r = 2*mm * std::sqrt(G4UniformRand());
  G4double phi = 2*M_PI * G4UniformRand();
  G4double z = (2*G4UniformRand() - 1) * 2*mm; // ±2mm height
  
  G4ThreeVector randomOffset(r*std::cos(phi), r*std::sin(phi), z);
  G4ThreeVector actualPos = sourcePos + randomOffset;
  
  // Na-22 decay branching ratios:
  // 90.3% beta+ -> positron annihilation -> 2 × 511 keV back-to-back
  // 9.7% electron capture -> 1274 keV gamma
  
  G4double rand = G4UniformRand();
  
  if (rand < 0.903) {
    // Positron annihilation: Generate back-to-back 511 keV photons
    GenerateAnnihilationPhotons(actualPos, anEvent);
  } else {
    // Electron capture: Generate 1274 keV photon
    Generate1274keVPhoton(actualPos, anEvent);
  }
}

void PrimaryGeneratorAction::GenerateAnnihilationPhotons(const G4ThreeVector& pos, G4Event* anEvent)
{
  // Generate isotropic direction for first photon
  G4double cosTheta = 2*G4UniformRand() - 1;
  G4double sinTheta = std::sqrt(1 - cosTheta*cosTheta);
  G4double phi = 2*M_PI * G4UniformRand();
  
  G4ThreeVector dir1(sinTheta*std::cos(phi), sinTheta*std::sin(phi), cosTheta);
  G4ThreeVector dir2 = -dir1; // Back-to-back
  
  // First photon
  fParticleGun->SetParticlePosition(pos);
  fParticleGun->SetParticleMomentumDirection(dir1);
  fParticleGun->SetParticleEnergy(511.*keV);
  fParticleGun->GeneratePrimaryVertex(anEvent);
  
  // Second photon (back-to-back)
  fParticleGun->SetParticlePosition(pos);
  fParticleGun->SetParticleMomentumDirection(dir2);
  fParticleGun->SetParticleEnergy(511.*keV);
  fParticleGun->GeneratePrimaryVertex(anEvent);
}

void PrimaryGeneratorAction::Generate1274keVPhoton(const G4ThreeVector& pos, G4Event* anEvent)
{
  // Generate isotropic direction
  G4double cosTheta = 2*G4UniformRand() - 1;
  G4double sinTheta = std::sqrt(1 - cosTheta*cosTheta);
  G4double phi = 2*M_PI * G4UniformRand();
  
  G4ThreeVector dir(sinTheta*std::cos(phi), sinTheta*std::sin(phi), cosTheta);
  
  // Single 1274 keV photon
  fParticleGun->SetParticlePosition(pos);
  fParticleGun->SetParticleMomentumDirection(dir);
  fParticleGun->SetParticleEnergy(1274.*keV);
  fParticleGun->GeneratePrimaryVertex(anEvent);
}
