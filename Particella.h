/*La classe particella descrive una particella tramite 2 angoli*/

#ifndef PARTICELLA_H
#define PARTICELLA_H

#include "TObject.h"

class Particella: public TObject{

    public:
        //Costruttore di default
        Particella();

        //Costruttore standard
        Particella(double Theta, double Phi);

        //Copy
        Particella(const Particella& source);

        //Distruttore: virtual poichè viene utilizzato il distruttore di TObject (e non alloca memoria)
        virtual ~Particella();

        //Overloading operatore =
        Particella& operator=(const Particella& source);

        //GETTER
        double GetTheta() const {return dmTheta;};
        double GetPhi() const {return dmPhi;};

        //SETTER
        void SetTheta(double Theta){ dmTheta = Theta;};
        void SetPhi(double Phi){ dmPhi = Phi;};

    private:
        //Data members
        double dmTheta;
        double dmPhi;

ClassDef(Particella,1)
};
#endif