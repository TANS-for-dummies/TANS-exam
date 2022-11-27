#include "Segnale.h"

//classe importata da root
ClassImp(Segnale)

//Costruttore di default
Segnale::Segnale() : dmZ(0.), dmPhi(0.), dmEtichetta(0), TObject(){};

//Costruttore standard
Segnale::Segnale(double Z, double Phi, int Etichetta): dmZ(Z), dmPhi(Phi), dmEtichetta(Etichetta), TObject(){};

//Copy
Segnale::Segnale(const Segnale& source): TObject(source){
    dmZ = source.dmZ;
    dmPhi = source.dmPhi;
    dmEtichetta = source.dmEtichetta;
};

//Overloading operatore =
Segnale& Segnale::operator=(const Segnale& source){
    if(this == &source) return *this;
    this->~Segnale();
    new(this) Segnale(source);
    return *this;
}