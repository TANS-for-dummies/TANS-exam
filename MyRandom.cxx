//Questa libreria implementa il generatore di numeri casuali che ci serve
#include "MyRandom.h"
#include "Riostream.h"
#include "TMath.h"

ClassImp(MyRandom)

bool MyRandom::sFlag = false; //Se True indica che il file in Input non è stato trovato

//Costruttore di default
MyRandom::MyRandom() : TRandom3() {
	dmEta = new TH1D();
	dmMolt = new TH1D();
}

//Costruttore standard
MyRandom::MyRandom(const char* input_file, double seed) : TRandom3(seed) {
	TFile F(input_file);
	if(F.IsZombie()) sFlag = true; //F.IsZombie è true se input_file punta ad una cella di memoria in cui non c'è il file di input

	else{	//prendiamo gli istogrammi dal file .root 
		TH1D* temp_eta = (TH1D*)F.Get("heta_cut");
		TH1D* temp_mol = (TH1D*)F.Get("hmul");
		
		//ci spostiamo nella directory corrente, così si può chiudere il file di input e lavorare sugli istogrammi
		temp_eta->SetDirectory(0);
		temp_mol->SetDirectory(0);
		F.Close();
		
		dmMolt = temp_mol;
		dmEta = temp_eta;
	}
}

//Copy-Constructor
MyRandom::MyRandom(const MyRandom& source) : TRandom3(source) {
	dmEta = new TH1D();
	dmMolt = new TH1D();
	*dmEta = *source.dmEta;
	*dmMolt = *source.dmMolt;
}

//Destructor Non dovrebbe servire deallocare i TH1D* siccome dovrebbe pensarci root
MyRandom::~MyRandom() {}

//Operatore =
MyRandom& MyRandom::operator=(const MyRandom& source) {
    if(this == &source) return *this;
    this->~MyRandom();
    new(this) MyRandom(source);
    return *this;
}

//RndTheta estrae secondo la distribuzione della pseudorapidità e poi calcola theta
double MyRandom::RndTheta() {
	double eta=dmEta->GetRandom();
	double temp=TMath::Exp(-eta);
	return 2.*TMath::ATan(temp);
}