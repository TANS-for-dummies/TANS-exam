//La classe Segnale descrive un hit (coordinate z e phi) e il numero della particella che l'ha generato
#ifndef SEGNALE_H
#define SEGNALE_H

#include "TObject.h"

class Segnale: public TObject{

    public:
        //Costruttore di default
        Segnale();

        //Costruttore standard
        Segnale(double Z, double Phi, int Etichetta);

        //Copy
        Segnale(const Segnale& source);

        //Distruttore: virtual poichè viene utilizzato il distruttore di TObject (e non alloca memoria)
        virtual ~Segnale(){};

        //Overloading operatore =
        Segnale& operator=(const Segnale& source);

        //GETTER
        double GetZ() const {return dmZ;};
        double GetPhi() const {return dmPhi;};
        int GetEtichetta() const {return dmEtichetta;};

        //SETTER (bau)
        void SetZ(double Z){ dmZ = Z;};
        void SetPhi(double Phi){ dmPhi = Phi;};
        void SetEtichetta(int Etichetta){ dmEtichetta = Etichetta;};

    private:
        //Data members
        double dmZ;
        double dmPhi;
        int dmEtichetta;

ClassDef(Segnale,1)
};
#endif