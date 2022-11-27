//Questa classe implementa oggetti di tipo punto con 3 coordinate (x,y,z)

/*
Notazione:
1) dmSomething è un data member
2) sSomething è uno static
2) cSomething è una const
3) i precedenti sono cumulabili, uno static const è un scSomething
4) le member function iniziano con la maiuscola

5) .h header file
6) .cxx file di implementazione di una classe
*/

#ifndef PUNTO_H
#define PUNTO_H

#include "TObject.h"

class Punto : public TObject {

	public:		
		
		//Costruttori e associati vari da inserire
		Punto();
		Punto(double x, double y, double z);
		Punto(const Punto& source);
		virtual ~Punto();
		Punto& operator=(const Punto& source);		
		
		/*
		I getter, fanno il return di un data member sono dei const 		
		siccome non devono poter alterare il punto
		*/
		double GetX() const {return dmX;};
		double GetY() const {return dmY;};
		double GetZ() const {return dmZ;};
		
		//I setter, impostano il valore di uno specifico data member
		void SetX(double x) {dmX=x;};
		void SetY(double y) {dmY=y;};
		void SetZ(double z) {dmZ=z;};
		
		//Restituisce XX+YY raggio delle coordinatte cilindriche
		double GetRadiusXY() const;
	
	private:
	
		//Data member
		double dmX;
		double dmY;
		double dmZ;
	
	//Definisce questa come versione 1  di Punto per ROOT
	ClassDef(Punto,1)
	
};

#endif