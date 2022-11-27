#include "Tracklet.h"
#include "TMath.h"

//classe importata da root
ClassImp(Tracklet)

//Costruttore di default
Tracklet::Tracklet() : dmR1(0.), dmR2(0.), dmZ1(0.), dmZ2(0.), TObject(){}

//Costruttore standard
Tracklet::Tracklet(double R1, double R2, double Z1, double Z2): dmR1(R1), dmR2(R2), dmZ1(Z1), dmZ2(Z2), TObject(){}

//Costruttore copia
Tracklet::Tracklet(const Tracklet& source) : TObject(source){
    dmR1 = source.dmR1;
    dmR2 = source.dmR2;
    dmZ1 = source.dmZ1;
    dmZ2 = source.dmZ2;
}

//Distruttore standard
Tracklet::~Tracklet(){}

//Operatore copia
Tracklet& Tracklet::operator=(const Tracklet& source){
    if(this == &source) return *this;
    this->~Tracklet();
    new(this) Tracklet(source);
    return *this;
}

//Funzione per trovare l'intersezione delle tracklets con l'asse del fascio
double Tracklet::Intersezione(){
    double m = (dmR2-dmR1)/(dmZ2-dmZ1);
    double z = -(dmR1 - m*dmZ1)/m;
    return z;
}