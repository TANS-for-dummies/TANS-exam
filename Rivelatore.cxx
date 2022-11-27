#include "Rivelatore.h"
#include "TMath.h"

ClassImp(Rivelatore)

//Costruttore di default
Rivelatore::Rivelatore(): dmR(0.), dmH(0.), dmTheta(0.), dmSmear_z(0.), dmSmear_rphi(0.), TObject() {}

//Costruttore standard
Rivelatore::Rivelatore(double r, double H, double Theta, double smear_z, double smear_rphi): dmR(r), dmH(H), dmTheta(Theta), dmSmear_z(smear_z), dmSmear_rphi(smear_rphi), TObject() {}

//Copy
Rivelatore::Rivelatore(const Rivelatore& source) : dmR(source.dmR), dmH(source.dmH), dmTheta(source.dmTheta), dmSmear_z(source.dmSmear_z), dmSmear_rphi(source.dmSmear_rphi), TObject(source) {}

//Distruttore
Rivelatore::~Rivelatore() {}

//Operatore copia
Rivelatore& Rivelatore::operator=(const Rivelatore& source){
    if(this == &source) return *this;
    this->~Rivelatore();
    new(this) Rivelatore(source);
    return *this;
}

Particella Rivelatore::MultiScattering(Particella *part){

    //deviazione dalla direzione di entrata

    double ThetaP = 0.;
    do {ThetaP = gRandom -> Gaus(0,dmTheta);} while(ThetaP<0); //vogliamo theta >=0 (il Do serve per farlo eseguire almeno una volta)
    double PhiP = gRandom -> Rndm()*2.*TMath::Pi();

    double mr[3][3];
    mr[0][0] = - TMath::Sin(part->GetPhi()); 
    mr[1][0] = TMath::Cos(part->GetPhi()); 
    mr[2][0] = 0;
    mr[0][1] = - TMath::Cos(part->GetPhi())*TMath::Cos(part->GetTheta()); 
    mr[1][1] = - TMath::Cos(part->GetTheta())*TMath::Sin(part->GetPhi());
    mr[2][1] = TMath::Sin(part->GetTheta()); 
    mr[0][2] = TMath::Cos(part->GetPhi())*TMath::Sin(part->GetTheta());
    mr[1][2] = TMath::Sin(part->GetTheta())*TMath::Sin(part->GetPhi());
    mr[2][2] = TMath::Cos(part->GetTheta());

    double scat[3]; //coordinate cartesiane della deviazione (cioè della particella dopo il multiscattering, rispetto alla direzione iniziale)
    scat[0] = TMath::Sin(ThetaP)*TMath::Cos(PhiP);
    scat[1] = TMath::Sin(ThetaP)*TMath::Sin(PhiP);
    scat[2] = TMath::Cos(ThetaP);

    double final_dir[3];

    for (int i = 0; i < 3; i++){
        final_dir[i] = 0.;
        for (int j = 0; j < 3; j++){
            final_dir[i]+=mr[i][j]*scat[j];
        }
    }

    //Coordinate della particella dopo il multiscattering nel sistema di riferimento di partenza 
    double final_theta = TMath::ACos(final_dir[2]);
    double final_phi;
    
    //final_phi e' sempre ben definito poiche' se sin(theta) fosse 0 dovremmo avere un punto del tipo (0,0,z) ossia sull'asse del fascio -> non possibile 
    if(final_dir[1]>=0.) final_phi=TMath::ACos(final_dir[0]/(TMath::Sin(final_theta)));
    else final_phi=2.*TMath::Pi()-TMath::ACos(final_dir[0]/(TMath::Sin(final_theta)));
    return Particella(final_theta,final_phi);
}

Segnale Rivelatore:: Smearing(Punto *P, int Num_part){

    double z = P->GetZ() + gRandom->Gaus(0,dmSmear_z);
    double temp_phi = 0;
    
    if(P->GetY()>=0.) temp_phi = TMath::ACos(P->GetX()/P->GetRadiusXY());
    else temp_phi = 2.*TMath::Pi() - TMath::ACos(P->GetX()/P->GetRadiusXY());
    
    double phi = temp_phi + (gRandom->Gaus(0,dmSmear_rphi))/P->GetRadiusXY();

    //Controlliamo che Phi rimanga dentro l'intervallo giusto anche dopo lo smearing
    if(phi<0.) phi += 2.*TMath::Pi();
    else if(phi>2.*TMath::Pi()) phi -= 2.*TMath::Pi();

    Segnale temp(z,phi,Num_part);

    return temp; 
}

//Restituisce il segnale senza applicare smearing
Segnale Rivelatore::NoSmearing(Punto *P, int Num_part) {
    double z = P->GetZ();
    double phi = 0;
    if(P->GetY()>=0.) phi = TMath::ACos(P->GetX()/P->GetRadiusXY());
    else phi = 2.*TMath::Pi() - TMath::ACos(P->GetX()/P->GetRadiusXY());
    Segnale temp(z,phi,Num_part);
    return temp; 
}

Punto Rivelatore::Hit(Punto P, Particella *part){
    double x0 = P.GetX();
    double y0 = P.GetY();
    double z0 = P.GetZ();

    double c1 = TMath::Sin(part->GetTheta()) * TMath::Cos(part->GetPhi());
    double c2 = TMath::Sin(part->GetTheta()) * TMath::Sin(part->GetPhi());
    double c3 = TMath::Cos(part->GetTheta());

    double delta = (x0*c1 + y0*c2)*(x0*c1 + y0*c2) - (c1*c1 + c2*c2)*(x0*x0 + y0*y0 - dmR*dmR);
    double t1 = (-(x0*c1 + y0*c2) + TMath::Sqrt(delta))/(c1*c1 + c2*c2);
    double t2 = (-(x0*c1 + y0*c2) - TMath::Sqrt(delta))/(c1*c1 + c2*c2);
    double t;

    if(t1>0) t=t1;
    else t=t2;

    double x = x0 + c1*t;
    double y = y0 + c2*t;
    double z = z0 + c3*t;

    Punto hit(x,y,z);

    return hit;
}