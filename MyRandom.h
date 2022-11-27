#ifndef MYRANDOM_H
#define MYRANDOM_H

#include "TRandom3.h"
#include "TH1D.h"
#include "TFile.h"

class MyRandom : public TRandom3 {

    public:

        MyRandom(); //Costruttore di default
        MyRandom(const char* input_file, double seed); //Costruttore standard
        MyRandom(const MyRandom& source); //Copy-Constructor
        virtual ~MyRandom(); //Distruttore
        MyRandom& operator=(const MyRandom& source); //Operatore =
        
        //Estraiamo dagli istogrammi valori casuali
        double RndTheta();
        int RndMolt(int) {return (int) dmMolt->GetRandom();};
        int RndMolt_unif(int N_max) {return (int) ( Rndm()*(N_max-1) ) + 1;};
        int RndMolt_fissa(int N){return N;};
        
        static bool GetFlag() {return sFlag;} //static significa che è costante per ogni istanza della classe (e occupa un solo posto in memoria)
        
    private:
    	static bool sFlag;
    	
    	TH1D* dmEta;
    	TH1D* dmMolt;
	
    ClassDef(MyRandom,1)
};

#endif