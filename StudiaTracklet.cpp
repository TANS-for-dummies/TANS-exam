//Librerie custom
#include "Segnale.h"
#include "Tracklet.h"
#include "RunningWindow.h"

//Librerie
#include "Riostream.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TMath.h"
#include "TStopwatch.h"
#include "TClonesArray.h"
#include "TH1D.h"
#include "TStyle.h"
#include "TGraphErrors.h"
#include "vector"
#include "algorithm"
#include "TF1.h"
#include <fstream>

void StudiaTracklet(const char* input = "MonteCarlo.root", int n_sigma = 3){ 
    //n_sigma: numero di deviazioni standard considerate per il taglio sulla Z
    //input: nome del file in input (solo .root)

    //Costanti
    double pi_greco = TMath::Pi();

    //Settaggi
    double r1 = 4.; //cm
    double r2 = 7.; //cm
    double delta_phi = 0.004; //ampiezza angolare in rad entro cui cercare corrispondenza hit
    double sigma_Z = 5.3; //cm deviazione standard della generazione delle Z

    //Avviamo il timer	
    TStopwatch timer;
    timer.Start();

    // definizione struct
    typedef struct {
        double x, y, z;
        int molt;
    } Vertice;

    static Vertice inizio;

    int dim = 36;

    // Dichiarazione TClonesArray
    TClonesArray *riv_1 = new TClonesArray("Segnale",dim);
    TClonesArray *riv_2 = new TClonesArray("Segnale",dim);

    //Apertura file di input
    TFile Input_file(input);
    if(Input_file.IsZombie()){
		cout << "File di input non trovato" << endl;
		return;
	}

    //----------------------------------------------------------------------------------------------------------------------------------------------------    
 
    //Lettura TTree e branch
    TTree *tree = (TTree*)Input_file.Get("Tree");
    TBranch *b1 = tree->GetBranch("VertMult");
    TBranch *b2 = tree->GetBranch("Hit1");
    TBranch *b3 = tree->GetBranch("Hit2");

    // Definizione degli indirizzi per la lettura dei dati su ttree
    b1->SetAddress(&inizio.x);
    b2->SetAddress(&riv_1);
    b3->SetAddress(&riv_2);

    //Creiamo un tracklet, con r1 ed r2 fissati
    Tracklet* tr = new Tracklet(r1, r2, 0., 0.); //Primo punto: layer 1; Secondo punto: layer 2
    
    //Creiamo gli istogrammi con cui analizzeremo la risoluzione del nostro algoritmo
    TH1D* deltaZ = new TH1D("deltaZ","Residui",200,-0.5,0.5);
    deltaZ->GetXaxis()->SetTitle("Zrec-Zvera [cm]");
    deltaZ->SetMarkerStyle(22);
    deltaZ->SetMarkerColor(52);
    deltaZ->SetLineColor(kBlack);

    TH1D* width_check = new TH1D("width_check","Width Check",100,0.,0.5);
    width_check->GetXaxis()->SetTitle("Width [cm]");
    width_check->SetMarkerStyle(22);
    width_check->SetMarkerColor(52);
    width_check->SetLineColor(kBlack);

    TH1D* mean_graph = new TH1D("mean","mean",100,-300,300);
    mean_graph->GetXaxis()->SetTitle("Mean - Z_MC [#mum]");
    mean_graph->SetMarkerStyle(22);
    mean_graph->SetMarkerColor(52);
    mean_graph->SetLineColor(kBlack);
                
    // loop sugli ingressi nel TTree
    for(int i=0; i<tree->GetEntries(); i++){
        tree->GetEvent(i);
        double Z_max = -100.;
        double Z_min = 100.;
        double mean = 10000;
        
        //Controlliamo che le Z generate siano entro n_sigma
        if(TMath::Abs(inizio.z) < (n_sigma*sigma_Z)){
            for (int j=0;j<inizio.molt;j++){
                for(int k=0; k<riv_1->GetEntries(); k++){
                    Segnale* interazione1 = (Segnale*)riv_1->At(k);
                    if(interazione1->GetEtichetta()==j) {
                        tr->SetZ1(interazione1->GetZ());
                        for(int l=0; l<riv_2->GetEntries(); l++){
                            Segnale* interazione2 = (Segnale*)riv_2->At(l);
                            if(interazione2->GetEtichetta()==j) {
                                tr->SetZ2(interazione2->GetZ());
                                double Z_tr = tr->Intersezione();
                                if(Z_min > Z_tr) Z_min = Z_tr;
                                if(Z_max < Z_tr) Z_max = Z_tr;
                                deltaZ->Fill(Z_tr-inizio.z);
                                mean = deltaZ->GetMean();
                                }
                        }
                    }
                }    
            }
            if(Z_max-Z_min != -200.){width_check->Fill(Z_max-Z_min);
            mean_graph->Fill(mean*10000);
            }
        }  //chiusura if sulle z
    } //chiusura del for sugli eventi

    TCanvas* c_residui = new TCanvas("c_residui","Residui",80,80,775,500);
    deltaZ->DrawCopy("pe");
    gStyle->SetOptFit(1111);
    TCanvas* c_residui_2 = new TCanvas("c_residui_2","Residui",80,80,775,500);
    width_check->DrawCopy("pe");
    TCanvas* c_residui_3 = new TCanvas("c_residui_3","Residui",80,80,775,500);
    mean_graph->DrawCopy("pe");

    timer.Stop();
    timer.Print();
}
