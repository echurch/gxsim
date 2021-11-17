#ifndef DetectorConstruction_hh
#define DetectorConstruction_hh 1

#include "G4VUserDetectorConstruction.hh"
#include "HeedDeltaElectronModel.hh"
#include "G4SystemOfUnits.hh"
#include "DetectorMessenger.hh"
#include "G4UserLimits.hh"
#include "DetectorMessenger.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4RunManager.hh"
#include "G4FieldManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4UniformMagField.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4Polycone.hh"
#include "G4Polyhedra.hh"
#include "G4UnionSolid.hh"
#include "G4Region.hh"
#include "G4Orb.hh"
#include "GasModelParameters.hh"



class G4VSolid;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4UniformMagField;


using namespace std;
/*! \class  DetectorConstruction*/
/*! \brief class derived from G4VUserDetectorConstruction*/

class DetectorConstruction : public G4VUserDetectorConstruction {
 public:
  DetectorConstruction(GasModelParameters*);
  virtual ~DetectorConstruction();

  // Mandatory methods
  virtual G4VPhysicalVolume* Construct();
  virtual void ConstructSDandField();

  //Setters for the dimensions and environment variables of the setup
  inline void CheckOverlaps(G4bool co){checkOverlaps=co;};
  inline void SetWorldHalfLength(G4double d){worldHalfLength=d;};
  inline void SetGasBoxR(G4double d){gasboxR=d;};
  inline void SetGasBoxH(G4double d){gasboxH=d;};
  inline void SetWallThickness(G4double d){wallThickness=d;};

  inline void SetNumHexes(G4int n){numHexes=n;};
  inline void SetGasPressure(G4double d){gasPressure=d;};
  inline void SetTemperature(G4double d){temperature=d;};
  //Getters for the dimensions and environment variables of the setup
  inline G4double GetWorldHalfLength(){return worldHalfLength;};
  inline G4double GetGasBoxR(){return gasboxR;};
  inline G4double GetGasBoxH(){return gasboxH;};
  inline G4double GetWallThickness(){return wallThickness;};

  inline G4double GetGasPressure(){return gasPressure;};
  inline G4double GetNumHexes(){return numHexes;};
  inline G4double GetTemperature(){return temperature;};
  inline G4double GetArgonPercentage(){return argonPercentage;};
  inline G4double GetCH4Percentage(){return ch4Percentage;};

  HeedDeltaElectronModel* fHDEt;
  HeedDeltaElectronModel* GetHDETracks() { std::cout << "DetCon::GetHDETracks()" << std::endl; return fHDEt;};

  G4double fBinSz;
  G4double fNumBins;
  G4double GetBinsz() {return fBinSz;};
  G4int GetMaxBin() {return fNumBins;};
  
 private:
  DetectorMessenger* detectorMessenger;
  G4LogicalVolume* logicGasBox;

  GasModelParameters* fGasModelParameters;
  G4bool checkOverlaps; // Check overlaps in the detector geometry if true
  G4double worldHalfLength; //World volume is a cube with side length = 2m;
  G4double gasboxR; // radius of tube filled with gas
  G4double gasboxH; // length of the tube
  G4double wallThickness; //thickness of the copper wall
  G4int    numHexes;
  G4double gasPressure; // pressure in the gas
  G4double temperature; // temperature of the gas
  G4double admixturePercentage;
  G4double argonPercentage;
  G4double ch4Percentage;
  
  
};
#endif
