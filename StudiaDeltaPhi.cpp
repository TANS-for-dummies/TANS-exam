//Librerie custom
#include "Segnale.h"
#include "Tracklet.h"

//Librerie
#include "Riostream.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TMath.h"
#include "TClonesArray.h"
#include "TH1D.h"

//Questa funzione studia un file.root per cercare di ricavare una stima di delta phi max per la ricostruzione del vertice
void Studia_DeltaPhi(const char* input = "MonteCarlo.root"){

    //Creiamo un istogramma per i deltaPhi
    TH1D* histo_phi = new TH1D("histo_phi", "Istogramma delle phi", 100, -0.01, 0.01);

    // definizione struct
    typedef struct {
        double x, y, z;
        int molt;
    } Vertice;

    static Vertice inizio;

    // Dichiarazione TClonesArray
    TClonesArray *riv_1 = new TClonesArray("Segnale",300);
    TClonesArray *riv_2 = new TClonesArray("Segnale",300);
 
    //Apertura file di input
    TFile Input_file(input);
    if(Input_file.IsZombie()){
		cout << "File di input non trovato" << endl;
		return;
	}

    //Lettura TTree e branch
    TTree *tree = (TTree*)Input_file.Get("Tree");
    TBranch *b1 = tree->GetBranch("VertMult");
    TBranch *b2 = tree->GetBranch("Hit1");
    TBranch *b3 = tree->GetBranch("Hit2");

    // Definizione degli indirizzi per la lettura dei dati su ttree
    b1->SetAddress(&inizio.x);
    b2->SetAddress(&riv_1);
    b3->SetAddress(&riv_2);

    // loop sugli ingressi nel TTree
    for(int i=0; i<tree->GetEntries(); i++){
        tree->GetEvent(i);

        for(int j=0; j<riv_2->GetEntries(); j++){ 
            Segnale *tst_2=(Segnale*)riv_2->At(j);
            double etichetta = tst_2->GetEtichetta();
            for(int k=0;k<riv_1->GetEntries();k++){
                Segnale *tst_1=(Segnale*)riv_1->At(k);
                if(tst_1->GetEtichetta()==etichetta) histo_phi->Fill((tst_1->GetPhi())-(tst_2->GetPhi()));
            }
        }

        histo_phi->DrawCopy();
    }

    Input_file.Close();
    TFile output("DeltaPhiStudy.root","recreate");
	histo_phi->Write();
	output.Close();
}