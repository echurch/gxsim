#include "DetectorMessenger.hh"

#include "DetectorConstruction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWith3Vector.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorMessenger::DetectorMessenger(DetectorConstruction* HPGeDet)
    : detector(HPGeDet) {
    miniDir = new G4UIdirectory("/MWGPC/");
    miniDir->SetGuidance("MWGPC specific controls");

    ////////////////////
    geometryDir = new G4UIdirectory("/MWGPC/geometry/");
    geometryDir->SetGuidance("MWGPC geometry specific controls");


    ////////////////////
    setGasPressCmd =
      new G4UIcmdWithADoubleAndUnit("/MWGPC/geometry/SetGasPressure", this);

    setGasPressCmd->SetGuidance("Set gas pressure.");
    setGasPressCmd->SetUnitCategory("Pressure");
    setGasPressCmd->SetDefaultValue(0.3 * bar);
    setGasPressCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setNumHexesCmd =
      new G4UIcmdWithAnInteger("/MWGPC/geometry/SetNumHexes",this);

    //    setNumHexesCmd->SetGuidance("Set num hexes.");
    //   setNumHexesCmd->SetUnitCategory("NumHexes");
    setNumHexesCmd->SetDefaultValue(1);
    setNumHexesCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorMessenger::~DetectorMessenger() {
    delete miniDir;
    delete geometryDir;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
  if (command == setGasPressCmd)
    detector->SetGasPressure(setGasPressCmd->GetNewDoubleValue(newValues));
  if (command == setNumHexesCmd)
    detector->SetNumHexes(setNumHexesCmd->GetNewIntValue(newValues));
}
