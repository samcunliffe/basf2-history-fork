#include <TF1.h>
#include <ecl/digitization/OfflineFitFunction.h>
#include <TTree.h>
#include <TFile.h>
#include <TGraph.h>
#include <iostream>

//
/*
 See eclComputePulseTemplates_Step0.cc for README instructions.
 */
//
int main(int argc, char* argv[])
{
  //
  int LowCellIDLimit = atoi(argv[1]);
  int HighCellIDLimit = atoi(argv[2]);
  //
  double PhotonWaveformPar[11];
  TFile* PhotonParFile = new TFile("PhotonWaveformParameters.root");
  TTree* chain = (TTree*) PhotonParFile->Get("ParTree");
  chain->SetBranchAddress("PhotonPar", &PhotonWaveformPar);
  //
  TFile* f = new TFile(Form("PhotonShapes_Low%d_High%d.root", LowCellIDLimit, HighCellIDLimit), "RECREATE");
  TTree* mtree = new TTree("mtree", "");
  double PhotonWaveformArray[100000];
  mtree->Branch("PhotonArray", &PhotonWaveformArray, "PhotonWaveformArray[100000]/D");
  //
  for (Long64_t jentry = LowCellIDLimit; jentry < HighCellIDLimit; jentry++) {
    chain->GetEntry(jentry);
    TF1 PhotonShapeFunc = TF1(Form("photonShape_%lld", jentry), WaveFuncTwoComp, 0, 20, 26);;
    PhotonShapeFunc.SetNpx(1000);
    std::cout << PhotonWaveformPar[0] << std::endl;
    PhotonShapeFunc.SetParameter(0, 0);
    PhotonShapeFunc.SetParameter(1, 0);
    PhotonShapeFunc.SetParameter(2, 1);
    PhotonShapeFunc.SetParameter(3, 0);
    for (int k = 0; k < 10; k++) {
      PhotonShapeFunc.SetParameter(4 + k, PhotonWaveformPar[k + 1]);
      PhotonShapeFunc.SetParameter(10 + 4 + k, PhotonWaveformPar[k + 1]);
    }
    PhotonShapeFunc.SetParameter(24, PhotonWaveformPar[0]);
    PhotonShapeFunc.SetParameter(25, 1);
    //
    if (PhotonWaveformPar[0] > 0 && jentry <= 8737) {
      for (int k = 0; k < 100000; k++) PhotonWaveformArray[k] = PhotonShapeFunc.Eval(((double)k) * (1. / 1000.)) ;
    } else {
      for (int k = 0; k < 100000; k++) PhotonWaveformArray[k] = -999;
    }
    mtree->Fill();
    //
  }
  //
  f->cd();
  mtree->Write();
  f->Write();
  //
  return 0;
}
