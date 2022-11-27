/*
La classe rivelatore si occupa di fornire i due layer sensibili e la beam pipe
*/

#ifndef RIVELATORE_H
#define RIVELATORE_H

#include "TObject.h"
#include "Particella.h"
#include "Segnale.h"
#include "Punto.h"
#include "MyRandom.h"

class Rivelatore : public TObject {

	public:
		//Costruttore standard
		Rivelatore();
		
		//Costruttore di default
		Rivelatore(double r, double H, double Theta, double smear_z, double smear_rphi);
		
		//Copy
		Rivelatore(const Rivelatore& source);
		
		//Distruttore
		virtual ~Rivelatore();
		
		//Overloading dell'operatore =
		Rivelatore& operator=(const Rivelatore& source);
		
		//METODI
		Particella ZeroScattering(Particella *part) {return *part;};
		Particella MultiScattering(Particella *part);

		Segnale Smearing(Punto *P, int Num_part);
		Segnale NoSmearing(Punto *P, int Num_part);

		Punto Hit(Punto P, Particella *part);


		//GETTER
		double GetR() const {return dmR;};
		double GetH() const {return dmH;};
		double GetTheta() const {return dmTheta;};
		double GetSmear_z() const {return dmSmear_z;};
		double GetSmear_rphi() const {return dmSmear_rphi;};
		
		//SETTER
		void SetR(double r) {dmR=r;};
		void SetH(double H) {dmH=H;};
		void SetTheta(double Theta) {dmTheta=Theta;};
		void SetSmear_z(double smear_z) {dmSmear_z=smear_z;};
		void SetSmear_rphi(double smear_rphi) {dmSmear_rphi=smear_rphi;};
	
	private:
		double dmR; //Raggio
		double dmH; //Lunghezza 
		double dmTheta; //Angolo planare rms
		double dmSmear_z; //Deviazione standard smearing su Z
		double dmSmear_rphi; //Deviazione standard smearing su r*phi

ClassDef(Rivelatore,1)
};
#endif
