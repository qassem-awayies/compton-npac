// EventAction.hh - Simplified header compatible with existing DetectorSD interface
#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class DetectorSD;

/// Enhanced Event action class for coincidence detection
///
/// This class implements coincidence logic with:
/// - Energy resolution simulation
/// - Energy window analysis for photopeak identification
/// - Statistical analysis

class EventAction : public G4UserEventAction
{
public:
    EventAction(DetectorSD* det1 = nullptr, DetectorSD* det2 = nullptr);
    virtual ~EventAction();
    
    virtual void BeginOfEventAction(const G4Event* event);
    virtual void EndOfEventAction(const G4Event* event);
    
    // Getters
    G4int GetCoincidenceCount() const { return fCoincidenceCount; }
    G4int GetTrueCoincidenceCount() const { return fTrueCoincidenceCount; }
    G4int GetTotalEvents() const { return fTotalEvents; }
    
    // Setters for analysis parameters
    void SetCoincidenceTimeWindow(G4double window) { fCoincidenceTimeWindow = window; }
    void SetEnergyWindow(G4double window) { fEnergyWindow = window; }
    
    // Analysis methods
    void PrintStatistics() const;
    
private:
    // Detector references for energy analysis
    DetectorSD* fDet1SD;
    DetectorSD* fDet2SD;
    
    // Statistics
    G4int fCoincidenceCount;      // Any coincidence (both detectors hit)
    G4int fTrueCoincidenceCount;  // 511keV photopeaks in both detectors
    G4int fTotalEvents;
    
    // Analysis parameters
    G4double fCoincidenceTimeWindow;  // Time window for coincidence (ns)
    G4double fEnergyWindow;          // Fractional energy window (e.g., 0.1 = ±10%)
    
    // Helper methods
    G4bool IsInEnergyWindow(G4double energy, G4double target, G4double window);
};

#endif
