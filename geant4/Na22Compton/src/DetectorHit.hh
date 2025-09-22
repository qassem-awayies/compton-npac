// DetectorHit.hh
#ifndef DetectorHit_h
#define DetectorHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4Threading.hh"

/// Detector hit class
///
/// It defines data members to store the the energy deposit and track 
/// information in the sensitive detector

class DetectorHit : public G4VHit
{
public:
    DetectorHit();
    DetectorHit(const DetectorHit&);
    virtual ~DetectorHit();

    // operators
    const DetectorHit& operator=(const DetectorHit&);
    G4bool operator==(const DetectorHit&) const;

    inline void* operator new(size_t);
    inline void  operator delete(void*);

    // methods from base class
    virtual void Draw() {}
    virtual void Print();

    // methods to handle data
    void Add(G4double de, G4double dl);

    // get methods
    G4double GetEdep() const     { return fEdep; }
    G4double GetTrackLength() const { return fTrackLength; }
    G4ThreeVector GetPos() const { return fPos; }
    G4int GetTrackID() const     { return fTrackID; }
    G4double GetTime() const     { return fTime; }
    
    // set methods
    void SetEdep(G4double de)      { fEdep = de; }
    void SetTrackLength(G4double dl) { fTrackLength = dl; }
    void SetPos(G4ThreeVector xyz) { fPos = xyz; }
    void SetTrackID(G4int id)      { fTrackID = id; }
    void SetTime(G4double t)       { fTime = t; }

private:
    G4double      fEdep;        ///< Energy deposit in the sensitive volume
    G4double      fTrackLength; ///< Track length in the  sensitive volume
    G4ThreeVector fPos;         ///< Position of the hit
    G4int         fTrackID;     ///< Track ID
    G4double      fTime;        ///< Time of the hit
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

using DetectorHitsCollection = G4THitsCollection<DetectorHit>;

extern G4ThreadLocal G4Allocator<DetectorHit>* DetectorHitAllocator;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

inline void* DetectorHit::operator new(size_t)
{
    if(!DetectorHitAllocator)
        DetectorHitAllocator = new G4Allocator<DetectorHit>;
    void *hit;
    hit = (void *) DetectorHitAllocator->MallocSingle();
    return hit;
}

inline void DetectorHit::operator delete(void *hit)
{
    if(!DetectorHitAllocator)
        DetectorHitAllocator = new G4Allocator<DetectorHit>;
    DetectorHitAllocator->FreeSingle((DetectorHit*) hit);
}

inline void DetectorHit::Add(G4double de, G4double dl) {
    fEdep += de; 
    fTrackLength += dl;
}

#endif
