#include "Particella.h"
#include "TMath.h"

//classe importata da root
ClassImp(Particella)

//Costruttore di default
Particella::Particella() : dmTheta(0.), dmPhi(0.){}

//Costruttore standard
Particella::Particella(double Theta, double Phi): dmTheta(Theta), dmPhi(Phi), TObject(){}

//Costruttore d copia
Particella::Particella(const Particella& source) : TObject(source){
    dmTheta = source.dmTheta;
    dmPhi = source.dmPhi;
}

//Distruttore standard
Particella::~Particella(){}

//Operatore copia
Particella& Particella::operator=(const Particella& source){
    if(this == &source) return *this;
    this->~Particella();
    new(this) Particella(source);
    return *this;
}