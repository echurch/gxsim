#include "TrackingAction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VTouchable.hh"

#include "DetectorConstruction.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"

#include "G4SDManager.hh"
#include "G4Run.hh"
#include "G4EventManager.hh"
#include "GasBoxSD.hh"


void TrackingAction::PreUserTrackingAction(const G4Track *aTrack) {
  auto const* evt = G4EventManager::GetEventManager()->GetConstCurrentEvent();
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  G4int  event = evt->GetEventID();
  const G4ThreeVector pos(aTrack->GetPosition());


  const G4ParticleDefinition* particle = aTrack->GetParticleDefinition();
  G4int pID       = particle->GetPDGEncoding();
  G4double time   = aTrack->GetGlobalTime();

  
  if (!(particle->GetParticleName().find("photon")!=std::string::npos))
    return;

  G4int id(12);
  if (pos[1]/mm<590.0) id = 1; // S1 optphotons
  if (pos[1]/mm>590.0) id = 2; // S2 optphotons

  /*
  G4ProcessManager* pmanager = particle->GetProcessManager();
  std::cout << "TrackingAction:: optphoton type S1/S2: " << id << " . processes observed: " << std::endl;
  for (short int ii = 0; ii<(short int)(pmanager->GetProcessList()->size());++ii)
    {
      std::cout << (*pmanager->GetProcessList())[ii]->GetProcessName() << std::endl;
    }
  */
  
  std::string startp("null");
  const G4VProcess* sprocess   = aTrack->GetCreatorProcess();
  G4String processName("null") ;
  if (sprocess)
    startp = sprocess->GetProcessName();

  // Weirdly, S1 is filled in two places. Here for optphotons and in GarfieldVUVPhotons::S1Fill() for thermale's..
  // S2 is only filled here.
  G4int row(0);
  analysisManager->FillNtupleDColumn(id,row, event); row++;

  analysisManager->FillNtupleDColumn(id,row, (G4double)pID); row++;
  analysisManager->FillNtupleDColumn(id,row, time/ns); row++;
  analysisManager->FillNtupleDColumn(id,row, pos[0]/mm); row++;
  analysisManager->FillNtupleDColumn(id,row, pos[1]/mm); row++;
  analysisManager->FillNtupleDColumn(id,row, pos[2]/mm); row++;
  analysisManager->FillNtupleSColumn(id,row, startp); row++;

  analysisManager->AddNtupleRow(id);


  
}



TrackingAction::TrackingAction():fPPID(0.0),fPKE(0.0), fFPKE(0.0)
{
}
