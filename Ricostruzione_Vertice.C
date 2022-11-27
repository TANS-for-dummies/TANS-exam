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
#include "TGraphAsymmErrors.h"
#include "vector"
#include "algorithm"
#include "TF1.h"
#include <fstream>
#include "TEfficiency.h"

using std::vector;

void Ricostruzione_Vertice(double window_size = 0.35, double window_step = 0.175, int n_sigma = 3, const char* input = "MonteCarlo.root", const char* output = "Analisi.root"){ 
    //window e step in cm
    //n_sigma: numero di deviazioni standard considerate per il taglio sulla Z
    //input: nome del file in input (solo .root)
    //output: file con tutti i grafici e gli istogrammi utilizzati

    if(window_size<=0 || window_step<=0 || window_step>window_size) {
        //con settaggi inappropriati di queste due variabili il programma rischia di entrare in un loop infinito, quindi preveniamo il problema
        std::cout << "Settaggi della finestra non validi, verranno impostati ai valori standard" << std::endl;
        std::cout << "window_size = 0.35" << std::endl << "window_step = 0.175" << std::endl; 
        window_size = 0.35;
        window_step = 0.175;
    }

    RunningWindow *window = new RunningWindow(window_size,window_step);

    //Costanti
    double pi_greco = TMath::Pi();    

    //Settaggi
    double r1 = 4.; //cm
    double r2 = 7.; //cm
    double delta_phi = 0.004; //ampiezza angolare in rad entro cui cercare corrispondenza hit, impostato con gli esiti di StudiaDeltaPhi
    double sigma_Z = 5.3; //cm (deviazione standard della generazione delle Z)

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

    //Apertura file di input e output
    TFile Input_file(input);
	
	if(Input_file.IsZombie()){
		cout << "File di input non trovato" << endl;
		return;
	}
	
    TFile Output_file(output,"recreate");


    //------------------------------------------------------------Lettura "Generazione"------------------------------------------------------------------

    //Nel file vengono salvati alcuni parametri di generazione degli eventi per migliorare la rappresentazione dei risultati    
    TObject* Generazione_Z = (TObject*)Input_file.Get("Generazione_Z");
    TObject* Z_costante = (TObject*)Input_file.Get("Z_costante");
    TObject* Generazione_molt = (TObject*)Input_file.Get("Generazione_molt");
    
    int gen_z = Generazione_Z->GetUniqueID();
    int z_fissa = Z_costante->GetUniqueID();
    int N_molt = Generazione_molt->GetUniqueID();
    
    vector<double> molteplicita_studiate; //è dichiarato double per fare il grafico dopo
    double molteplicita_studiate_standard[10] = {3,5,7,9,11,15,20,30,40,50};
    double sigma_molt_studiate = 0.5;
    
    //distribuzione data
    if(N_molt == 0){        
        for(int i = 0; i < 10; i++){
            molteplicita_studiate.push_back(molteplicita_studiate_standard[i]); //tiene conto delle molteplicita' medie che vogliamo analizzare con i grafici
        }
    }
    
    //molteplicità fissata
    else if(N_molt > 0){
        molteplicita_studiate.push_back(N_molt);
        dim = N_molt;
    }
    
    //molteplicità uniforme
    else{
        for(int i = 0; i < 10; i++){
            if(molteplicita_studiate_standard[i] < -1.*N_molt-1){
                molteplicita_studiate.push_back(molteplicita_studiate_standard[i]);
            }
        }
        molteplicita_studiate.push_back(-1*N_molt-1);
    }

    const int dim_molt = molteplicita_studiate.size();

    //costruzione degli istogrammi in intervalli di Z
    vector<double> Z_studiate; //è dichiarato double per fare il grafico dopo
    double Z_studiate_standard[15] = {-12.6, -10.8, -9., -7.2, -5.4, -3.6, -1.8, 0., 1.8, 3.6, 5.4, 7.2, 9., 10.8, 12.6};
    double sigma_Z_studiate = 0.9;

    if(gen_z==3) Z_studiate.push_back(z_fissa);
    else{
        for (int i = 0;i < 15; i++) Z_studiate.push_back(Z_studiate_standard[i]);

    }

    const int dim_Z = Z_studiate.size();
    
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
    
    //Creiamo un vector per le z
    vector<double> vec_z; 
    
    //Creiamo gli istogrammi con cui analizzeremo la risoluzione del nostro algoritmo
    TH1D* deltaZ = new TH1D("deltaZ","Residui",200,-1000,1000);
    deltaZ->GetXaxis()->SetTitle("Zrec-Zvera [#mum]");
    deltaZ->SetMarkerStyle(22);
    deltaZ->SetMarkerColor(52);
    deltaZ->SetLineColor(kBlack);
    
    //creiamo un array di istogrammi e uno di canvas sia per molteplicità che per Z
    TH1D *histo_molt[dim_molt];

    TH1D *histo_Z[dim_Z];

    char nome[30];
    char titolo[80];
    //loop sull'array di molteplicita' studiate per creare gli istogrammi di deltaZ per
    //singole molteplicita' ed inizializzare le gaussiane, lo stesso viene fatto per le Z studiate
    for (int i=0;i<dim_molt; i++) {
        //Istogrammi sulle molteplicita'
        sprintf(nome, "fixed molt center %f", molteplicita_studiate.at(i));
        sprintf(titolo,"Residui - molteplicita' fissata da %f a %f", molteplicita_studiate.at(i) - 0.5, molteplicita_studiate.at(i) + 0.5);
        histo_molt[i] = new TH1D(nome, titolo, 400, -1000, 1000);
        histo_molt[i] -> GetXaxis() -> SetTitle("Zrec-Zvera [#mum]");
        histo_molt[i] -> SetMarkerStyle(33);
    }
    for (int i=0;i<dim_Z; i++) {
        //Istogrammi sulle Z
        sprintf(nome, "fixed Z center %f", Z_studiate.at(i));
        sprintf(titolo,"Residui - Z fissata da %f a %f", Z_studiate.at(i) - 0.5, Z_studiate.at(i) + 0.5);
        histo_Z[i] = new TH1D(nome, titolo, 400, -1000, 1000);
        histo_Z[i] -> GetXaxis() -> SetTitle("Zrec-Zvera [#mum]");
        histo_Z[i] -> SetMarkerStyle(33);
    }
    

    //Creiamo il grafico dell'efficienza e della risoluzione in funzione della molteplicita'
    vector<double> s_molt(dim_molt,0.); //array di errori per la molteplicita
    vector<double> ris_molt(dim_molt,0.); //array per le risoluzioni prese dai fit
    vector<double> s_ris_molt(dim_molt,0.); //array di errori sulla risoluzione
    TGraphErrors *risoluzione_molt;
    TEfficiency* efficiency_molt = new TEfficiency("eff_molt","Efficienza vs Molteplicita';molt;#epsilon",30,0,60);

    //Creiamo il grafico dell'efficienza e della risoluzione in funzione di Z
    vector<double> s_Z(dim_Z,0.);
    vector<double> ris_Z(dim_Z,0.); //array per le risoluzioni prese dai fit
    vector<double> s_ris_Z(dim_Z,0.); //array di errori sulla risoluzione 
    TGraphErrors *risoluzione_Z;
    TEfficiency* efficiency_Z = new TEfficiency("eff_Z","Efficienza vs Z;Z;#epsilon",16,-16,16);

    //----------------------------------------------INIZIO ANALISI-------------------------------------------------------

    //loop sugli ingressi nel TTree
    for(int i=0; i<tree->GetEntries(); i++){
        tree->GetEvent(i);
  
        //Controlliamo che le Z generate siano entro n_sigma
        if(TMath::Abs(inizio.z) < (n_sigma*sigma_Z)){
        
           for(int j=0; j<riv_1->GetEntries(); j++){ //for sul layer 1
                Segnale* interazione1 = (Segnale*)riv_1->At(j);
                tr->SetZ1(interazione1->GetZ()); //inseriamo la Z1 dentro tracklet

                for(int k=0; k<riv_2->GetEntries(); k++){  //for sul layer 2
                    Segnale* interazione2 = (Segnale*)riv_2->At(k); 

                    if( TMath::Abs(interazione1->GetPhi() - interazione2->GetPhi()) <= delta_phi ){
                        tr->SetZ2(interazione2->GetZ()); //inseriamo la Z2 dentro tracklet
                        vec_z.push_back(tr->Intersezione()); //riempiamo il vector di intersezioni dei tracklet con l'asse Z
                    }
               }
           } //fine del loop sul TClonesArray


            sort(vec_z.begin(), vec_z.end()); //riordiniamo il vector in ordine crescente, per poter utilizzare la running window

            bool Rec = 1; //indica che riusciamo a ricostruire il vertice, running window sarà in grado di modificarlo successivamente
            double Z_rec = window->running_window(vec_z, Rec); //Ricostruzione con metodo della running window

            if(Rec == 0) {
                window->SetStep(2*window_step);
                window->SetSize(2*window_size);
                Z_rec = window->running_window(vec_z, Rec);
            }

            efficiency_molt->Fill(Rec,inizio.molt);
            efficiency_Z->Fill(Rec,inizio.z);
            
            if(Rec) {
                deltaZ -> Fill((Z_rec-inizio.z)*10000); //residuo in micrometri
                
                //loop sui vari istogrammi al variare di Z e molteplicità
                for(int j=0; j<dim_molt; j++){ 
                    if((inizio.molt>molteplicita_studiate.at(j)-sigma_molt_studiate) && (inizio.molt<molteplicita_studiate.at(j)+sigma_molt_studiate)) histo_molt[j]->Fill((Z_rec-inizio.z)*10000); //residuo in micrometri
                }
                for(int j=0;j<dim_Z;j++){
                    if((inizio.z>(Z_studiate.at(j)-sigma_Z_studiate)) && (inizio.z<(Z_studiate.at(j)+sigma_Z_studiate))) histo_Z[j]->Fill((Z_rec-inizio.z)*10000); //residuo in micrometri
                }
            }
        
           //Reset della window e clear del vector
           vec_z.clear(); 
           window->SetStep(window_step);
           window->SetSize(window_size);
        }  //chiusura if sulle z
    } //chiusura del for sugli eventi

    //Loop sulle molteplicità studiate
    for(int i=0; i<dim_molt; i++) {
        s_molt[i]=sigma_molt_studiate;
        //Fit delle gaussiane
        if(histo_molt[i] -> GetEntries()!=0){
            histo_molt[i] -> Fit("gaus");
            TF1 *Gauss_molt = histo_molt[i] -> GetFunction("gaus");
            
            //Calcolo della risoluzione
            ris_molt[i] = Gauss_molt -> GetParameter(2);
            s_ris_molt[i] = Gauss_molt -> GetParError(2);
        }
    }    
    //Loop sulle Z studiate
    for(int i=0; i<dim_Z; i++) {
        s_Z[i]=sigma_Z_studiate;
        //Fit delle gaussiane
        if(histo_Z[i]->GetEntries()!=0) {
            histo_Z[i] -> Fit("gaus");
            TF1 *Gauss_Z = histo_Z[i] -> GetFunction("gaus");
            
            //Calcolo della risoluzione
            ris_Z[i] = Gauss_Z -> GetParameter(2);
            s_ris_Z[i] = Gauss_Z -> GetParError(2);
        }
    }

    //Efficienza in funzione della molteplicità
    TCanvas* c1 = new TCanvas("c1","c1",80,80,775,500);
    efficiency_molt->SetMarkerStyle(33);
    efficiency_molt->SetMarkerColor(77);
    efficiency_molt->Draw("APC");
    
    //Efficienza in funzione di Z
    TCanvas* c2 = new TCanvas("c2","c2",80,80,775,500);
    efficiency_Z->SetMarkerStyle(33);
    efficiency_Z->SetMarkerColor(77);
    efficiency_Z->Draw("APC");
    c2->Update();
    efficiency_Z->GetPaintedGraph()->SetMinimum(0.3);
    efficiency_Z->GetPaintedGraph()->SetMaximum(1.1);
    efficiency_Z->Draw("APC");
    
    //Risoluzione in funzione della molteplicità
    TCanvas* c3 = new TCanvas("c3","c3",80,80,775,500);
    risoluzione_molt = new TGraphErrors(dim_molt,&(molteplicita_studiate[0]),&(ris_molt[0]),&(s_molt[0]),&(s_ris_molt[0]));
    risoluzione_molt->SetTitle("Risoluzione vs Molteplicita'");
    risoluzione_molt->GetXaxis()->SetTitle("Molteplicita'");
    risoluzione_molt->GetYaxis()->SetTitle("Risoluzione (#mum)");
    risoluzione_molt->SetMarkerStyle(33);
    risoluzione_molt->SetMarkerColor(77);
    risoluzione_molt ->Draw ("APC");

    //Risoluzione in funzione di Z
    TCanvas* c4 = new TCanvas("c4","c4",80,80,775,500);
    risoluzione_Z = new TGraphErrors(dim_Z,&(Z_studiate[0]),&(ris_Z[0]),&(s_Z[0]),&(s_ris_Z[0]));
    risoluzione_Z->SetTitle("Risoluzione vs Z");
    risoluzione_Z->GetXaxis()->SetTitle("Z (cm)");
    risoluzione_Z->GetYaxis()->SetTitle("Risoluzione (#mum)");
    risoluzione_Z->SetMarkerStyle(33);
    risoluzione_Z->SetMarkerColor(77);
    risoluzione_Z ->Draw ("APC");

    //Scrittura di tutti i grafici in un file apposito
    Output_file.cd();
    Output_file.Write();
    efficiency_molt->Write("efficienza molt");
    efficiency_Z->Write("efficienza Z");
    risoluzione_molt->Write("risoluzione molt");
    risoluzione_Z->Write("risoluzione Z");
	Output_file.Close();

    delete window;

    timer.Stop();
    timer.Print();
}