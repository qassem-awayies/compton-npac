// EventAction.cc - Simplified coincidence detection compatible with existing DetectorSD interface
#include "EventAction.hh"
#include "DetectorSD.hh"

#include "G4Event.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "G4ios.hh"

EventAction::EventAction(DetectorSD* det1, DetectorSD* det2)
 : G4UserEventAction(),
   fDet1SD(det1), fDet2SD(det2),
   fCoincidenceCount(0),
   fTrueCoincidenceCount(0),
   fTotalEvents(0),
   fCoincidenceTimeWindow(10.*ns),  // 10 ns coincidence window
   fEnergyWindow(0.1)  // ±10% energy window
{}

EventAction::~EventAction()
{}

void EventAction::BeginOfEventAction(const G4Event*)
{
    // Clear detector energies for new event
    if (fDet1SD) fDet1SD->Clear();
    if (fDet2SD) fDet2SD->Clear();
}

void EventAction::EndOfEventAction(const G4Event* event)
{
    fTotalEvents++;
    
    // Get energies from detector SDs (using existing interface)
    G4double det1Energy = fDet1SD ? fDet1SD->GetTotalEnergyWithResolution() : 0.;
    G4double det2Energy = fDet2SD ? fDet2SD->GetTotalEnergyWithResolution() : 0.;
    
    // Check for hits in both detectors
    G4bool det1Hit = (det1Energy > 0.);
    G4bool det2Hit = (det2Energy > 0.);
    
    if (!det1Hit || !det2Hit) {
        // No coincidence - abort visualization
        G4EventManager::GetEventManager()->AbortCurrentEvent();
        
        // Clear detector energies for next event
        if (fDet1SD) fDet1SD->Clear();
        if (fDet2SD) fDet2SD->Clear();
        return;
    }

    // Basic coincidence found
    fCoincidenceCount++;

    // Check for 511 keV photopeaks (energy window analysis)
    G4double target511 = 511.*keV;
    G4bool det1_511keV = IsInEnergyWindow(det1Energy, target511, fEnergyWindow);
    G4bool det2_511keV = IsInEnergyWindow(det2Energy, target511, fEnergyWindow);
    
    G4bool isTrueCoincidence = det1_511keV && det2_511keV;
    
    if (isTrueCoincidence) {
        fTrueCoincidenceCount++;
    }

    // Print event information
    G4cout << "=== COINCIDENCE EVENT #" << event->GetEventID() 
           << " (Coin: " << fCoincidenceCount 
           << ", True: " << fTrueCoincidenceCount << ") ===" << G4endl;
    G4cout << "Det1 Energy: " << G4BestUnit(det1Energy, "Energy") 
           << (det1_511keV ? " [511keV Peak]" : "") << G4endl;
    G4cout << "Det2 Energy: " << G4BestUnit(det2Energy, "Energy") 
           << (det2_511keV ? " [511keV Peak]" : "") << G4endl;
    
    if (isTrueCoincidence) {
        G4cout << "*** TRUE ANNIHILATION COINCIDENCE ***" << G4endl;
    }
    
    // Keep trajectories for visualization
    G4EventManager::GetEventManager()->KeepTheCurrentEvent();
    
    // Clear detector energies for next event
    if (fDet1SD) fDet1SD->Clear();
    if (fDet2SD) fDet2SD->Clear();
}

G4bool EventAction::IsInEnergyWindow(G4double energy, G4double target, G4double window)
{
    G4double lowerBound = target * (1.0 - window);
    G4double upperBound = target * (1.0 + window);
    return (energy >= lowerBound && energy <= upperBound);
}

void EventAction::PrintStatistics() const
{
    G4cout << "\n=== COINCIDENCE STATISTICS ===" << G4endl;
    G4cout << "Total events processed: " << fTotalEvents << G4endl;
    G4cout << "Total coincidence events: " << fCoincidenceCount << G4endl;
    G4cout << "True 511keV coincidences: " << fTrueCoincidenceCount << G4endl;
    
    if (fTotalEvents > 0) {
        G4double coincidenceRate = (G4double)fCoincidenceCount / fTotalEvents * 100.0;
        G4double trueRate = (G4double)fTrueCoincidenceCount / fTotalEvents * 100.0;
        G4cout << "Coincidence rate: " << coincidenceRate << "%" << G4endl;
        G4cout << "True coincidence rate: " << trueRate << "%" << G4endl;
    }
    
    if (fCoincidenceCount > 0) {
        G4double trueRatio = (G4double)fTrueCoincidenceCount / fCoincidenceCount * 100.0;
        G4cout << "True/Total coincidence ratio: " << trueRatio << "%" << G4endl;
    }
}
