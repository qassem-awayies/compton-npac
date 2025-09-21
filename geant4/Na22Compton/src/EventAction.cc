#include "EventAction.hh"
#include "G4Event.hh"
#include "CLHEP/Units/SystemOfUnits.h"
#include <fstream>

EventAction::EventAction() {}

EventAction::~EventAction() {}

void EventAction::EndOfEventAction(const G4Event* /*event*/) {
    // Example placeholders: replace with actual energy deposition
    G4double edep1 = 0.0;
    G4double edep2 = 0.0;

    std::ofstream out("edep.csv", std::ios::app);
    out << edep1/CLHEP::keV << "," << edep2/CLHEP::keV << "\n";
}

