#include "PrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction() {
    fParticleGun = new G4ParticleGun(1);

    auto particleTable = G4ParticleTable::GetParticleTable();
    auto gamma = particleTable->FindParticle("gamma");
    fParticleGun->SetParticleDefinition(gamma);
    fParticleGun->SetParticleEnergy(511*keV);
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0,0,1));
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
    fParticleGun->GeneratePrimaryVertex(event);
}

