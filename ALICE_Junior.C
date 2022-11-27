//Librerie custom
#include "Rivelatore.h" //le altre classi (MyRandom, Particella, Segnale e Punto) sono incluse in Rivelatore

//Librerie
#include "Riostream.h"
#include "TMath.h"
#include "TStopwatch.h" //Monitora il tempo di CPU
#include "TSystem.h" //Monitora RAM
#include "TTree.h" //Output
#include "TBranch.h"
#include "TLeaf.h"
#include "TClonesArray.h"
#include <fstream>

void MonteCarlo(int N_esp = 1000000, int gen_z = 1, int gen_molt = 1, int N_false_hit = 0, bool scat = 1, bool smear = 1, const char* output_file = "MonteCarlo.root", bool verbose = 0, unsigned int seed = 125) {
    /*
    N_esp è il numero di esperimenti che si vuole simulare
    gen_z seleziona il tipo di generazione delle z (distribuzione gaussiana=1, uniforme=2, costante=3)
    gen_molt seleziona il tipo di generazione della molteplicità (distribuzione data=1, uniforme=2, costante=3)
    N_false_hit indica il numero di hit falsi che devono essere generati uniformemente sul rivelatore
    scat attiva (1) o disattiva (0) il multiscattering sui rivelatori
    smear attiva (1) o disattiva (0) lo smearing degli hit
    output_file è il nome del file di output che verrà generato dal programma, l'estensione deve sempre essere .root
    verbose se settato a 1 inserisce nel file di log un certo numero di eventi di controllo (1 ogni 1000)
    */

    //Regola quanti eventi vengono stampati nel log se verbosity è messa a 1 (numero_evento%VerbosityStep==0)
    int VerbosityStep = 1000;
	
    //Costanti
    double pi_greco = TMath::Pi();
    double Theta_Multi = 0.001/(TMath::Sqrt2()); //rad, deviazione standard della gaussiana che utilizziamo che estrarre il theta' del multiscattering
    
    //Settaggi input e output
    const char* input_file = "kinem_cut.root"; //file con le distribuzioni di molteplicità e pseudorapidità assegnate
  
    //Avviamo il timer	
    TStopwatch timer;
    timer.Start();
    
    //Apriamo il file di log
    std::ofstream ofs ("simulation_log.txt", std::ofstream::out);

    //Generatore di numeri random
    MyRandom *ptr = new MyRandom(input_file,seed);
    delete gRandom;
    gRandom = ptr;

    //Controlliamo che il file di input con le distribuzioni venga caricato correttamente
    if (ptr->GetFlag()) {
    	ofs << "File con le distribuzioni non trovato" << std::endl; 
    	return;
    }
	else ofs << "File con le distribuzioni trovato" << std::endl;

	//Creiamo una variabile per fissare la z nel caso di generazione secondo distribuzione costante
	double z_fissa = 0.;
	
	if(gen_z==3){
		std::cout << "Valore coordinata z del vertice:" << std::endl;    
		std::cin >> z_fissa;
	}

    TObject Generazione_Z;
    TObject Z_costante;
    TObject Generazione_molt;
	
	Generazione_Z.SetUniqueID(gen_z);
	Z_costante.SetUniqueID(z_fissa);		
	
    //Creazione del funtore per scegliere la molteplicita'
    int dim = 0;
    int N;
    int (MyRandom::*rndm_molt) (int); 
    if(gen_molt == 1) { //distribuzione estratta da grafico fornito
        rndm_molt = &MyRandom::RndMolt;
        dim = 36; //68.27% di 53 (massimo valore della molteplicità)
		Generazione_molt.SetUniqueID(0);
        }
    else if (gen_molt == 2) { //distribuzione uniforme
	    std::cout << "Numero massimo di particelle generabile con distribuzione uniforme:" << std::endl;    
	    std::cin >> N;   
        rndm_molt = &MyRandom::RndMolt_unif;
        dim = N/2 +1;
		Generazione_molt.SetUniqueID(-N);
        }
    else if (gen_molt == 3) { //distribuzione fissa
	std::cout << "Numero di particelle da generare:" << std::endl;    
	std::cin >> N; 
        rndm_molt = &MyRandom::RndMolt_fissa;
        dim = N;
		Generazione_molt.SetUniqueID(N);
        }
    else {ofs << "Scelta non valida. Impostato il settaggio di base: estrazione dall'istogramma" << std::endl;
        rndm_molt = &MyRandom::RndMolt;
        dim = 36;
		Generazione_molt.SetUniqueID(0); 
        }

    //----------------------------------------------------------------------------------------------------------------------------------------------

    //Creazione del funtore per MultiScattering e per lo smearing
    Particella (Rivelatore::*rndm_scatt) (Particella*);
    if (scat){rndm_scatt = &Rivelatore::MultiScattering;}
    else {rndm_scatt = &Rivelatore::ZeroScattering;}

    Segnale (Rivelatore::*smearing) (Punto*, int);
    if(smear){smearing = &Rivelatore::Smearing;}
    else {smearing = &Rivelatore::NoSmearing;}

    //Apertura file di output, e creazione di un TTree
    TFile Ofile(output_file, "RECREATE");
	Generazione_Z.Write("Generazione_Z");
    Z_costante.Write("Z_costante");
    Generazione_molt.Write("Generazione_molt");
	
    TTree *tree = new TTree("Tree","TTree con 3 branches"); //Vertice, layer1 e layer2

    TClonesArray *riv_1 = new TClonesArray("Segnale",dim);//Hit del rivelatore 1
    TClonesArray &hit1 = *riv_1;
    TClonesArray *riv_2 = new TClonesArray("Segnale",dim);//Hit del rivelatore 2
    TClonesArray &hit2 = *riv_2;
    
    //Rivelatori
    Rivelatore Beam_Pipe(3, 52, Theta_Multi, 0., 0.); //H=52 per contenere tutte le particelle generate con l'accettanza data
    Rivelatore Layer1(4, 27, Theta_Multi, 0.012, 0.003);
    Rivelatore Layer2(7, 27, Theta_Multi, 0.012, 0.003);

    //Definiamo una struct che contenga le caratteristiche del vertice (coordinate e molteplicita') 
    typedef struct{
        double x, y, z;
        int molt;
    } Vertice;
    
    static Vertice inizio;
    
    tree->Branch("VertMult", &inizio.x, "x/D:y:z:molt/I"); 
    tree->Branch("Hit1", &riv_1);
    tree->Branch("Hit2", &riv_2);

    tree->SetAutoSave(0); //Rimuove backup cycle

    //Creiamo una particella (fuori dal for sul numero degli esperimenti, cosi viene creata una sola volta)
    Particella* part = new Particella();

    //Creiamo un hit temporaneo
    Punto* hit = new Punto();

    //-----------------------------------------------------------GENERAZIONE DEL VERTICE-----------------------------------------------------------------------

    //for sul numero di esperimenti
    for(int k=0; k<N_esp; k++){
        //Iniziamo a generare il vertice, ci servono 3 coordinate e la molteplicita'
        //Unita di misura della lunghezza = cm
        inizio.x = ptr->Gaus(0.,0.01);
        inizio.y = ptr->Gaus(0.,0.01);
		
		switch(gen_z) {
			case 1:
				inizio.z = ptr->Gaus(0.,5.3);
				break;
			case 2:
				inizio.z = ptr->Rndm()*27. - 13.5;
				break;
			case 3:
				inizio.z = z_fissa;
				break;
			default:
				std::cout << "Scelta non valida. Impostato il settaggio di base: distribuzione gaussiana" << std::endl;
				gen_z = 1;
				inizio.z = ptr->Gaus(0.,5.3);
		}
        
        inizio.molt = (ptr->*rndm_molt)(N);
        
	    if (verbose && k%VerbosityStep == 0){ofs << std::endl << "Evento numero "  << k+1 << std::endl << "VERTICE (" << inizio.x << ", "<<inizio.y << ", "<<inizio.z << ") con molteplicita' " << inizio.molt << std::endl;};
	
        int pos1 = 0;
        int pos2 = 0;

	    //for sul numero di particelle
        for(int i=0; i<inizio.molt; i++) {
            //Generiamo i prodotti nel vertice
            part->SetTheta(ptr->RndTheta());
            part->SetPhi(ptr->Rndm()*2.*pi_greco);
            if (verbose && k%VerbosityStep == 0){ofs << std::endl << "Particella Numero " << i+1 << " : ( " << part->GetTheta() << " , " <<  part->GetPhi() << " )" << std::endl;} 
            
            //-------------------------------------Trasporto e multiscattering particella per particella-------------------------------------------
		
            //BEAM PIPE
            *hit = Beam_Pipe.Hit(Punto(inizio.x, inizio.y, 0), part);//facciamo tutto in z = 0 poi ritrasliamo alla fine
            *part = (Beam_Pipe.*rndm_scatt)(part);
            
            //LAYER 1
            *hit = Layer1.Hit(*hit, part);
		
	        //Controlliamo che la z del vertice sia all'interno del rivelatore
            if((hit -> GetZ()<=Layer1.GetH()/2.-inizio.z)&&(hit -> GetZ()>=-1.*Layer1.GetH()/2.-inizio.z)) {

                //Immagazziniamo lo smearing (coordinale cilindriche)
                Segnale temp1 = Segnale((Layer1.*smearing)(hit, i+1));
                temp1.SetZ(temp1.GetZ()+inizio.z);
                new(hit1[pos1]) Segnale(temp1);
                if (verbose && k%VerbosityStep == 0){ofs << "Segnale sul Rivelatore 1: ( " << temp1.GetZ() << ", " << temp1.GetPhi() << " )" << std::endl;}
		    
                //Multiscattering
                *part = (Layer1.*rndm_scatt)(part);
                
                //LAYER 2
                *hit = Layer2.Hit(*hit, part);
		    
	            //Controlliamo che la z del vertice sia all'interno del rivelatore
                if((hit -> GetZ()<=Layer2.GetH()/2.-inizio.z)&&(hit -> GetZ()>=-1.*Layer2.GetH()/2.-inizio.z)){
                    //Immagazziniamo lo smearing
                    Segnale temp2 = Segnale((Layer2.*smearing)(hit, i+1));
                    temp2.SetZ(temp2.GetZ()+inizio.z);
                    new(hit2[pos2]) Segnale(temp2);
                    if (verbose && k%VerbosityStep == 0){ofs << "Segnale sul Rivelatore 2: ( " << temp2.GetZ() << ", " << temp2.GetPhi() << " )"<< std::endl;}
                    pos2++;
                }
                pos1++;
            }
        } //chiusura del for sul numero di particelle
	    
        //Generazione dei false hit (uniformi in Z e phi) Z in cm
        for(int i=0; i<N_false_hit; i++) {
            new(hit1[pos1+i]) Segnale(-(Layer1.GetH())/2.+(ptr->Rndm())*(Layer1.GetH()),ptr->Rndm()*2*pi_greco, -(i+1));
            new(hit2[pos2+i]) Segnale(-(Layer2.GetH())/2.+(ptr->Rndm())*(Layer2.GetH()),ptr->Rndm()*2*pi_greco, -(i+1));
            if(verbose && k%VerbosityStep == 0){
                ofs << std::endl << "False hit numero " << i+1 << std::endl;
                Segnale *tst = (Segnale*)hit1[pos1+i];
                ofs << "Segnale sul rivelatore 1: ( " << tst->GetZ() << ", " << tst->GetPhi() << " )" << std::endl;
                tst = (Segnale*)hit2[pos2+i];
                ofs << "Segnale sul rivelatore 2: ( " << tst->GetZ() << ", " << tst->GetPhi() << " )" << std::endl;
                }
        }

        if(verbose && k%VerbosityStep == 0){ofs << std::endl << "----------------------------------FINE EVENTO----------------------------------" << std::endl;}   

        tree->Fill();

        riv_1->Clear();
        riv_2->Clear();
    }

    //Salviamo i dati sul file di output 
    Ofile.Write();
    
    //Chiudiamo il file di output
    Ofile.Close();
    
    timer.Stop();
    timer.Print();
	
    //Chiudiamo il file log	
    ofs.close();
	
    //Deallochiamo i puntatori
    delete part;
    delete hit;
}
