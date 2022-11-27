//Questo file rielabora kinem.root in modo da avere entrambi i grafici già a posto
#include "TFile.h"
#include "TH1D.h"
#include "TAxis.h"
#include "iostream"

void Cutter(){

	TFile F("kinem.root");
	if(F.IsZombie()){
		cout << "File di input non trovato" << endl;
		return;
	}
    //prendiamo gli istogrammi dal file .root 
	TH1D* temp_eta = (TH1D*)F.Get("heta");
	TH1D* temp_mol = (TH1D*)F.Get("hmul");
	
	//ci spostiamo nella directory corrente, così si può chiudere il file di input e lavorare sugli istogrammi
	temp_eta->SetDirectory(0);
	temp_mol->SetDirectory(0);
	F.Close();
	//restringiamo la distribuzione della pseudorapidità al range (-2;2)
	TAxis *xa=temp_eta->GetXaxis();
	Int_t b1=xa->FindBin(-2.);
	Int_t b2=xa->FindBin(2.);
	Double_t xlow=xa->GetBinLowEdge(b1);
	Double_t xhig=xa->GetBinUpEdge(b2);
	Int_t nobins=b2-b1+1;
	TH1D* eta_cut = new TH1D("heta_cut","heta_cut",nobins,xlow,xhig);
	Int_t j=1;
	for(Int_t i=b1;i<=b2;i++)eta_cut->SetBinContent(j++,temp_eta->GetBinContent(i));
    TFile Ofile("kinem_cut.root","RECREATE");
    temp_mol->Write();
    eta_cut->Write();
    Ofile.Close();
}