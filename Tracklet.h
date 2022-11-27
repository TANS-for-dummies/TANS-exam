/*La classe tracklet descrive una retta tramite due suoi punti (hit sui layer) nel piano r-z*/
#ifndef TRACKLET_H
#define TRACKLET_H

#include "TObject.h"

class Tracklet: public TObject{

    public:
        //Costruttore di default
        Tracklet();

        //Costruttore standard
        Tracklet(double R1, double R2, double Z1, double Z2);

        //Copy
        Tracklet(const Tracklet& source);

        //Distruttore: virtual poichè viene utilizzato il distruttore di TObject (e non alloca memoria)
        virtual ~Tracklet();

        //Overloading operatore =
        Tracklet& operator=(const Tracklet& source);

        //Funzione per trovare l'intersezione delle tracklets con l'asse del fascio
        double Intersezione();

        //GETTER
        double GetR1() const {return dmR1;};
        double GetR2() const {return dmR2;};
        double GetZ1() const {return dmZ1;};
        double GetZ2() const {return dmZ2;};
        
        //SETTER
        void SetR1(double R1){ dmR1 = R1;};
        void SetR2(double R2){ dmR2 = R2;};
        void SetZ1(double Z1){ dmZ1 = Z1;};
        void SetZ2(double Z2){ dmZ2 = Z2;};

    private:
        //Data members
        double dmR1;
        double dmR2;
        double dmZ1;
        double dmZ2;

ClassDef(Tracklet,1)
};
#endif