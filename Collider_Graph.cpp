#include "TGLViewer.h"
#include "Segnale.h"
#include "Riostream.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TMath.h"
#include "TStopwatch.h"
#include "TClonesArray.h"
#include "TStyle.h"
#include "TPolyLine3D.h"
#include "TPolyMarker3D.h"
#include "TGeoVolume.h"
#include "TGeoMatrix.h"
#include "TGeoManager.h"
#include "Tracklet.h"

using namespace std;

void Collider_Graph(int event=0, const char* input = "MonteCarlo.root") {
	int dim = 36;

	TClonesArray *riv_1 = new TClonesArray("Segnale",dim);
	TClonesArray *riv_2 = new TClonesArray("Segnale",dim);
	TFile Input_file(input);
	
	if(Input_file.IsZombie()){
		cout << "File di input non trovato" << endl;
		return;
		}
	
	// definizione struct
	typedef struct {
		double x, y, z;
		int molt;
	} Vertice;

	static Vertice inizio;
	
	TTree *tree = (TTree*)Input_file.Get("Tree");
	TBranch *b1 = tree->GetBranch("VertMult");
	TBranch *b2 = tree->GetBranch("Hit1");
	TBranch *b3 = tree->GetBranch("Hit2");
	b1->SetAddress(&inizio.x);
	b2->SetAddress(&riv_1);
	b3->SetAddress(&riv_2); 

	TCanvas *c1 = new TCanvas();
	TGLViewer *view = (TGLViewer*)gPad->GetViewer3D(); //accede al pad del canvas
	TGeoManager *man = new TGeoManager(); //gestisce tutti i volumi presenti
	TGeoHMatrix *tras_rot = new TGeoHMatrix("trans_rot"); //gestisce le rotazioni e le traslazioni ( RotateX(deg) e SetDx(cm) )
	tras_rot->RotateY(0.);

	//Ora dobbiamo creare una BOX che definisca il volume del Master Reference Sistem
	TGeoVolume *top = man->MakeBox("BOX", NULL, 27., 27., 27.); //al posto di Null è possibile specificare il materiale dell'oggetto, le unità sono in cm
	TGeoVolume *beam_pipe = man->MakeTube("PIPE", NULL, 2.96, 3.04, 13.5); //al posto di Null potremmo creare un medium che registri automaticamente gli hit delle particelle....
	beam_pipe->SetLineColor(kGreen);
	TGeoVolume *riv_1_vol = man->MakeTube("RIV1", NULL, 3.99, 4.01, 13.5);
	riv_1_vol->SetLineColor(kOrange);
	TGeoVolume *riv_2_vol = man->MakeTube("RIV1", NULL, 6.99, 7.01, 13.5);
	riv_2_vol->SetLineColor(kBlue);

	man->SetTopVolume(top); //settiamo il volume madre (esterno)
	top->AddNode(beam_pipe, 0, tras_rot); //creiamo il layer figlio 0-esimo
	top->AddNode(riv_1_vol, 1, tras_rot);
	top->AddNode(riv_2_vol, 2, tras_rot);
	man->CloseGeometry(); //chiudere sempre la geometria creata se no non disegna
	
	if(event < 0 || tree->GetEntries() <= event){
		cout << "Evento non esistente" << endl;
		return;
	}
	
	tree->GetEvent(event);
	top->Draw("ogl"); //ogl crea l'effettivo oggetto con raytracing ecc, quindi può essere pesante
	const int data = riv_2->GetEntries();

	TPolyMarker3D* inizio_marker = new TPolyMarker3D(1,7);
	TPolyLine3D* tracks[data]; //facilmente utilizzabile per le particelle
	Tracklet* tr = new Tracklet(4., 7., 0., 0.);

	inizio_marker->SetPoint(0,inizio.x,inizio.y,inizio.z);
	inizio_marker->SetMarkerColor(kBlack);
	inizio_marker->Draw();

	//Crea i tracklets
	for (int j=0;j<riv_2->GetEntries();j++){
		tracks[j] = new TPolyLine3D(3);
		Segnale* riv_2_data = (Segnale*)riv_2->At(j);
		tr->SetZ2(riv_2_data->GetZ());
		Segnale* riv_1_data = new Segnale();

		for(int i=0;i<riv_1->GetEntries();i++) {
			Segnale* tst = (Segnale*)riv_1->At(i);        
			if(tst->GetEtichetta()==riv_2_data->GetEtichetta()) {
				riv_1_data->SetPhi(tst->GetPhi());
				riv_1_data->SetZ(tst->GetZ());
				riv_1_data->SetEtichetta(tst->GetEtichetta());
				tr->SetZ1(riv_1_data->GetZ());
			}
		}

		tracks[j]->SetPoint(0,7.*TMath::Cos(riv_2_data->GetPhi()),7.*TMath::Sin(riv_2_data->GetPhi()),riv_2_data->GetZ());
		tracks[j]->SetPoint(1,4.*TMath::Cos(riv_1_data->GetPhi()),4.*TMath::Sin(riv_1_data->GetPhi()),riv_1_data->GetZ());
		tracks[j]->SetPoint(2,0,0,tr->Intersezione());
		tracks[j]->SetLineColor(kRed);
		tracks[j]->Draw("same");
	}
}
