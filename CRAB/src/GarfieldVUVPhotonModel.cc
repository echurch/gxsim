#include "G4Electron.hh"
#include "G4SystemOfUnits.hh"
#include "GarfieldVUVPhotonModel.hh"
#include "G4Region.hh"
#include "G4ParticleDefinition.hh"
#include "G4UnitsTable.hh"
#include "G4Track.hh"
#include "Randomize.hh"
#include "G4UIcommand.hh"
#include <fstream>
#include "G4TransportationManager.hh"
#include "G4DynamicParticle.hh"
#include "G4RandomDirection.hh"
#include "globals.hh"
#include "MediumMagboltz.hh"
#include "GeometrySimple.hh"
#include "ComponentConstant.hh"
#include "ComponentUser.hh"
#include "Sensor.hh"
#include "AvalancheMicroscopic.hh"
#include "AvalancheMC.hh"
#include "Medium.hh"
#include "SolidTube.hh"
#include "G4OpticalPhoton.hh"
#include "GarfieldExcitationHit.hh"
#include "GasModelParameters.hh"
#include "DetectorConstruction.hh"
#include "GasBoxSD.hh"
#include "G4ProcessManager.hh"
#include "G4EventManager.hh"
#include "Analysis.hh"

#include "G4AutoLock.hh"
namespace{G4Mutex aMutex = G4MUTEX_INITIALIZER;}

const static G4double torr = 1. / 760. * atmosphere;

GarfieldVUVPhotonModel::GarfieldVUVPhotonModel(GasModelParameters* gmp, G4String modelName,G4Region* envelope,DetectorConstruction* dc,GasBoxSD* sd) :
		G4VFastSimulationModel(modelName, envelope),detCon(dc),fGasBoxSD(sd) {
	thermalE=gmp->GetThermalEnergy();
	InitialisePhysics();
}

G4bool GarfieldVUVPhotonModel::IsApplicable(const G4ParticleDefinition& particleType) {
  //  std::cout << "GarfieldVUVPhotonModel::IsApplicable() particleType is " << particleType.GetParticleName() << std::endl;
  if (particleType.GetParticleName()=="thermalelectron") // || particleType.GetParticleName()=="opticalphoton")
    return true;
  return false;
		
		
}

G4bool GarfieldVUVPhotonModel::ModelTrigger(const G4FastTrack& fastTrack){
  G4double ekin = fastTrack.GetPrimaryTrack()->GetKineticEnergy();
  //  std::cout << "GarfieldVUVPhotonModel::ModelTrigger() thermalE, ekin is " << thermalE << ",  "<< ekin << std::endl;
  //  counter[0]++; //maybe not thread safe.
  //  G4cout << "GarfieldVUV: candidate NEST thermales: " << counter[0] << G4endl;
  S1S2Fill(fastTrack);
  G4String particleName = fastTrack.GetPrimaryTrack()->GetParticleDefinition()->GetParticleName();
  if (ekin<thermalE && particleName=="thermalelectron")
    return true;
  return false;

} 
	
void GarfieldVUVPhotonModel::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep) 
{

  /* 
     This tracks all the ionization/conversion electrons created by Degrad in its simulation of the primary gamma's photoelectric effect on Xe.
     Each such electron is drifted in the E-field and avalanched, as appropriate. That creates excited Xe atoms. We put one ELM photon
     per excitation of 172 (7.2) nm (eV) onto the optical photon stack. Geant4 will track those in the normal way. 


     Note, the weirdness of userHandle, which seems to be called at each excitation by the AvalancheMicroscopic model and fills our
     GarfieldExcitationHitCollection. And which somehow we're allowed to grab here.

     EC, 2-Dec-2021.

   */
  
    //G4cout<<"HELLO Garfield"<<G4endl;
    ////The details of the Garfield model are implemented here
     fastStep.KillPrimaryTrack();//KILL NEST/DEGRAD/G4 TRACKS
     garfPos = fastTrack.GetPrimaryTrack()->GetVertexPosition();
     garfTime = fastTrack.GetPrimaryTrack()->GetGlobalTime();
     //G4cout<<"GLOBAL TIME "<<G4BestUnit(garfTime,"Time")<<" POSITION "<<G4BestUnit(garfPos,"Length")<<G4endl;
     counter[1]++; // maybe not threadsafe
     if (!(counter[1]%100)) G4cout << "GarfieldVUV: actual NEST thermales: " << counter[1] << G4endl;


     //     if (!(counter[1]%10)) 
       GenerateVUVPhotons(fastTrack,fastStep,garfPos,garfTime);

}

GarfieldExcitationHitsCollection *garfExcHitsCol;

void GarfieldVUVPhotonModel::GenerateVUVPhotons(const G4FastTrack& fastTrack, G4FastStep& fastStep,G4ThreeVector garfPos,G4double garfTime)
{

  // Drift in main region, then as LEM region is approached and traversed, avalanche multiplication and excitations  will occur.

  
  
	G4double x0=garfPos.getX()*0.1;//Garfield length units are in cm
	G4double y0=garfPos.getY()*0.1;
	G4double z0=garfPos.getZ()*0.1;
	G4double t0=garfTime;
	G4double e0=7.;// starting energy [eV]->I have chose 7 eV because is the energy cut in Degrad

	G4double ekin = fastTrack.GetPrimaryTrack()->GetKineticEnergy();
	G4ThreeVector dir = fastTrack.GetPrimaryTrack()->GetMomentumDirection();
	G4String particleName = fastTrack.GetPrimaryTrack()->GetParticleDefinition()->GetParticleName();
	if (particleName.find("thermalelectron")!=std::string::npos) particleName = "e-";

	garfExcHitsCol = new GarfieldExcitationHitsCollection();
	std::array<double, 3> ef{0,0,0};
	std::array<double, 3> bf{0,0,0};
	std::vector<double> vf{0,0,0};
	Garfield::Medium* medium = nullptr;
	int status(0);
	//	fSensor->ElectricField(x0,y0,z0, ef[0], ef[1], ef[2], medium, status);
	//	std::cout << "GVUVPM: E field in medium " << medium << " at " <<  x0<<","<<y0<<","<<z0 << "  is:  "  << ef[0]<<","<<ef[1]<<","<<ef[2] << std::endl;
	// bool bstatus;
	//medium->EnableDebugging(); // EC, 20-May-2022, 6pm MDT.
	//bstatus = medium->ElectronVelocity(ef[0],ef[1],ef[2],bf[0],bf[1],bf[2],vf[0],vf[1],vf[2]);
	//std::cout << "GVUVPM: Drift velocity status, velocity: " << bstatus << ", " << vf[0]<<","<<vf[1]<<","<<vf[2] << std::endl;
	//std::cout << "GVUVPM: Medium atomic number, mass, density: " <<  medium->GetAtomicNumber() << ", " << medium->GetAtomicWeight() << "," << medium->GetNumberDensity() << std::endl;

	// Need to get the AvalancheMC drift at the High-Field point in y, and then call fAvalanche-AvalancheElectron() to create excitations/VUVphotons.
	fAvalancheMC->DriftElectron(x0,y0,z0,t0);

	unsigned int n = fAvalancheMC->GetNumberOfDriftLinePoints();
	double xi,yi,zi,ti;
	std::cout << "Drift(): avalanchetracking, n DLTs is " << n << std::endl;

	for(unsigned int i=0;i<n;i++){
	  fAvalancheMC->GetDriftLinePoint(i,xi,yi,zi,ti);
	  //	  std::cout << "GVUVPM: ghb, yi is " << detCon->GetGasBoxH() << "," << yi << std::endl;
	  if (yi> (detCon->GetGasBoxH()/CLHEP::cm-0.5)) break;
	} // pts in driftline

	e0 = 1.13;
	std::cout << "GVUVPM: Avalanching in high field starting at: "  << xi<<"," <<yi<<","<<zi <<"," <<ti << std::endl;
	std::cout << "GVUVPM: fMediumMagboltz: "  << fMediumMagboltz << std::endl;
	fAvalanche->AvalancheElectron(xi,yi,zi,ti, e0, 0., 0., 0.);
	
	unsigned int nElastic, nIonising, nAttachment, nInelastic, nExcitation, nSuperelastic;
	fMediumMagboltz->GetNumberOfElectronCollisions(nElastic, nIonising, nAttachment, nInelastic, nExcitation, nSuperelastic);
	G4cout<<"NExcitation "<<nExcitation<<G4endl; // This quantity seems to be cumulative over (at least) the event. ... EC, 2-Dec-2021.
	G4int colHitsEntries=garfExcHitsCol->entries();
	G4cout<<"GarfExcHits entries "<<colHitsEntries<<G4endl; // This one is not cumulative.

	for (G4int i=0;i<colHitsEntries;i++){
	  GarfieldExcitationHit* newExcHit=new GarfieldExcitationHit();
	  newExcHit->SetPos((*garfExcHitsCol)[i]->GetPos());
	  newExcHit->SetTime((*garfExcHitsCol)[i]->GetTime());
	  fGasBoxSD->InsertGarfieldExcitationHit(newExcHit);
	  //	  fastStep.SetNumberOfSecondaryTracks(1);	//1 photon per excitation .... Causes weirdness w stack. EC, 12-Apr-2022.
	  
	  if (i % (colHitsEntries/colHitsEntries) == 0){ // 50. Need to uncomment this condition, along with one in degradmodel.cc. EC, 2-Dec-2021.
	    G4DynamicParticle VUVphoton(G4OpticalPhoton::OpticalPhotonDefinition(),G4RandomDirection(), 7.2*eV);
	    // Create photons track
	    G4Track *newTrack=fastStep.CreateSecondaryTrack(VUVphoton, (*garfExcHitsCol)[i]->GetPos(),(*garfExcHitsCol)[i]->GetTime(),false);
	    //	G4ProcessManager* pm= newTrack->GetDefinition()->GetProcessManager();
	    //	G4ProcessVectorfAtRestDoItVector = pm->GetAtRestProcessVector(typeDoIt);
	  }
	}

	const G4Track* pG4trk = fastTrack.GetPrimaryTrack();
	G4int pntid = pG4trk->GetParentID();
	G4int tid = pG4trk->GetTrackID();
	
	delete garfExcHitsCol;

	
}


void ePiecewise (const double x, const double y, const double z,
		 double& ex, double& ey, double& ez) {
  //  G4double dethz(detCon->GetGasBoxH()*0.5);
  G4double dethz = 10.0;  
  ex = ez = 0.;
  if (y<(dethz - 0.5/*1.0*/))
    ey = -1000.;
  else {
    ey = -3000.0;
    //    std::cout<<"ePiecewise: y, dethz, ey are [cm]: " << y << ", " << dethz << ", " << ey << std::endl;
  }

}



// Selection of Xenon exitations and ionizations
void GarfieldVUVPhotonModel::InitialisePhysics(){
	fMediumMagboltz = new Garfield::MediumMagboltz();
	double pressure = detCon->GetGasPressure()/torr;
	double temperature = detCon->GetTemperature()/kelvin;

	fMediumMagboltz->SetTemperature(temperature);
	fMediumMagboltz->SetPressure(pressure);
	fMediumMagboltz->SetComposition("Xe", 100.);
	fMediumMagboltz->Initialise(true);
	fMediumMagboltz->DisableDebugging();
	gasFile =  "Xenon.gas";
	ionMobFile = "IonMobility_Ar+_Ar.txt";
	G4cout << gasFile << G4endl;
	const std::string path = getenv("GARFIELD_HOME");
	G4AutoLock lock(&aMutex);
	if(ionMobFile!="")
	  fMediumMagboltz->LoadIonMobility(path + "/Data/" + ionMobFile);
	if(gasFile!=""){
	  const bool useLog = true; // to use Logarithmic spacing.
	  // fMediumMagboltz->SetFieldGrid(10.,200000.,20,useLog); // In 99:1 N2:O2 E/p = 3000000/3 causes Magboltz to crash. EC, 5-Nov-2021.`
	  //fMediumMagboltz->GenerateGasTable(5); // num of 1E7 collisions to do ... something.
	  //	  std::cout << "Writing gasfile." << std::endl;
	  // fMediumMagboltz->WriteGasFile(gasFile.c_str());      
	  //	  std::cout << "Wrote gasfile."<< std::endl;
	  fMediumMagboltz->LoadGasFile(gasFile.c_str());
	  std::cout << "Loaded gasfile." << std::endl;
	}

	
	Garfield::GeometrySimple* geo = new Garfield::GeometrySimple();
	// Make a box
	G4double detectorRadius=detCon->GetGasBoxR();//cm
	G4double detectorHalfZ=detCon->GetGasBoxH()*0.5;//cm

	Garfield::SolidTube* tube = new Garfield::SolidTube(0.0, detectorHalfZ/CLHEP::cm,0.,0.0, detectorRadius/CLHEP::cm, detectorHalfZ/CLHEP::cm,0.,1.,0.);//Tube oriented in Y'axis (0.,1.,0.,)

	// Add the solid to the geometry, together with the medium inside
	geo->AddSolid(tube, fMediumMagboltz);

	// Make a component with analytic electric field
	Garfield::ComponentUser* componentDriftLEM = new Garfield::ComponentUser();
	componentDriftLEM->SetGeometry(geo);
	componentDriftLEM->SetElectricField(ePiecewise);
	std::cout << " .....   now forcing density for the Garfield gas medium -- via P, T." << std::endl;
	geo->GetMedium(0.,0.,0.)->SetPressure(detCon->GetGasPressure()/torr);
	geo->GetMedium(0.,0.,0.)->SetTemperature(detCon->GetTemperature()/kelvin);
	// pull out the density to see if our pressure-setting in .mac file is in fact effected down in Garfield. EC, 28-Oct-2021.
	double ggeodens = geo->GetMedium(0.,0.,0.)->GetMassDensity();
	double ggeopress = geo->GetMedium(0.,0.,0.)->GetPressure();

	std::cout << "HeedModel::buildBox(): Garfield mass density [g/cm3], pressure [Torr] is: " << ggeodens << ", " << ggeopress << std::endl;

	
	fAvalanche = new Garfield::AvalancheMicroscopic();
	fSensor = new Garfield::Sensor();
	fAvalanche->SetUserHandleInelastic(userHandle);
	fSensor->AddComponent(componentDriftLEM);
	fAvalanche->SetSensor(fSensor);

	
	fAvalancheMC = new Garfield::AvalancheMC(); // drift, not avalanche, to be fair.
	fAvalancheMC->SetSensor(fSensor);
	//	fAvalancheMC->SetIons();
	fAvalancheMC->SetTimeSteps(0.05); // nsec, per example
	fAvalancheMC->SetDistanceSteps(2.e-2); // cm, 10x example
	//fAvalancheMC->EnableDebugging(); // way too much information. 
	fAvalancheMC->DisableDebugging();
	fAvalancheMC->EnableAttachment();
	//    fAvalancheMC->DisableAttachment();

	const double axis_x = detCon->GetGasBoxR()/CLHEP::cm;  
  // Y-width of drift simulation will cover between +/- axis_y
	const double axis_z = detCon->GetGasBoxR();  
	const double axis_y = detCon->GetGasBoxH()/CLHEP::cm;

	fSensor->SetArea(-axis_x, -0, -axis_z, axis_x, axis_y, axis_z);

	//std::array<double, 3> ef{0,0,0};
	//Garfield::Medium* medium = nullptr;
	//int status(0);
	//fSensor->ElectricField(0.1, 0.5, 0.1, ef[0], ef[1], ef[2], medium, status);
	//std::cout << "E field at 0.1,0.5,0.1 is " << ef[0]<<","<<ef[1]<<","<<ef[2] << std::endl;

	//	fSensor->AddElectrode(comp,"s2");


	
}

  
// Selection of Xenon exitations and ionizations
void userHandle(double x, double y, double z, double t, int type, int level,Garfield::Medium * m)
{
	G4ThreeVector Pos;

	if (level > 2 && level < 53){//XENON LEVELS
	
	GarfieldExcitationHit* newExcHit=new GarfieldExcitationHit();
	Pos.setX(x*10);//back to cm to GEANT4
	Pos.setY(y*10);//back to cm to GEANT4
	Pos.setZ(z*10);//back to cm to GEANT4
	newExcHit->SetPos(Pos);
	newExcHit->SetTime(t);
	garfExcHitsCol->insert(newExcHit);
	//If choose to draw change the visualizer from OGL to HepRep in vis.mac file
	//newExcHit->Draw();	
	}// if level
	
	
}// end userhandle	


void GarfieldVUVPhotonModel::S1S2Fill(const G4FastTrack& ftrk)
{

  
  const G4Track* track = ftrk.GetPrimaryTrack();
  G4int pntID = track->GetParentID();
  G4int pID = track->GetParticleDefinition()->GetPDGEncoding();
  G4ThreeVector tpos = track->GetVertexPosition();
  G4double time = track->GetGlobalTime();
  G4int id;

  
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  G4int  event = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();


  if (time/ns<10) id = 1; // S1 e's
    {
	  analysisManager->FillNtupleDColumn(id,0, event);
	  analysisManager->FillNtupleDColumn(id,1, pID);
	  analysisManager->FillNtupleDColumn(id,2, time/ns);
	  analysisManager->FillNtupleDColumn(id,3, tpos[0]/mm);
	  analysisManager->FillNtupleDColumn(id,4, tpos[1]/mm);
	  analysisManager->FillNtupleDColumn(id,5, tpos[2]/mm);
	  analysisManager->AddNtupleRow(id);
    }

}

void GarfieldVUVPhotonModel::Reset() {fSensor->ClearSignal();}
