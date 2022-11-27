#include "RunningWindow.h"

#include "TMath.h"
#include "iostream"

using std::vector;

ClassImp(RunningWindow)

double media(vector<double> V,double limite_inf, double limite_sup) {
//media i valori degli elementi di un vector che si trovano in un certo range di valori 
    int vec_dim = V.size();
    double temp = 0;
    int count = 0;
    for(int i=0; i<vec_dim && V.at(i)<=limite_sup; i++) {
        if(V.at(i)>=limite_inf){
            temp+=V.at(i);
            count++;
        }
    }
    return temp/(double)count;

}

//Copy-Constructor
RunningWindow::RunningWindow(const RunningWindow& source) : TObject(source) {
	dmSize=source.dmSize;
	dmStep=source.dmStep;
}

//Operatore =
RunningWindow& RunningWindow::operator=(const RunningWindow& source) {
	//Utilizzo quello che sfrutta il copy constructor per non ripetere tutto
	if(this == &source) return *this;
	this->~RunningWindow();
	new(this) RunningWindow(source);
	return *this;
}

double RunningWindow::running_window(vector<double> vec,bool &stato_rec) {

    stato_rec = 1; //segna se il vertice e' stato ricostruito o meno
    
    int c_max = 0; //conteggio massimo
    double Z_max = 0; //media delle Zrec nella window con conteggio massimo
    double k_start = 0; //lower bound della finestra
    int j_max = 0; //numero della finestra con conteggio massimo

    if(vec.empty()) {
        stato_rec = 0;
        return 0;
    }
    else {
        double z_0 = vec.at(0) - 0.1; //cm //Punto di partenza della prima finestra
        int vec_dim = vec.size();
        for(int j=0; vec.at(vec_dim-1) > z_0 + j*dmStep; j++){ //controlliamo che la finestra non superi l'ultimo elemento del vector

            bool inside = 1; //segna se siamo dentro la finestra
            bool start_window = 1; //ci serve per salvare l'indice del primo elemento del vector che entra nella finestra

            int c = 0; //conteggio
            int k = k_start; //salva l'inizio della finestra

            while((k < vec_dim) && (inside)){
                if((vec.at(k) >= z_0 + j*dmStep) && (vec.at(k) <= z_0 + j*dmStep + dmSize)){
                    if(start_window) {
                        k_start=k; //ci salva da dove partire a scorrere sul vector per la prossima finestra
                        start_window = 0;
                    }
                    c++;
                }
                else if(vec.at(k) > z_0 + j*dmStep + dmSize) inside = 0;
                k++;
            }

            if(c>c_max) {
                c_max = c;
                stato_rec = 1;
                j_max = j;
            }

            else if(c==c_max && j-j_max!=1) stato_rec = 0; //la seconda condizione tiene conto che le finestre adiacenti hanno un'intersezione 
        }

        int conteggi_prima = 0; //Conteggi delle finestre adiacenti a quella col picco
        int conteggi_dopo = 0;

        for(int i = 0;(i<vec_dim) && (vec.at(i)<z_0+(j_max+1)*dmStep+dmSize);i++) {
            //la seconda condizione del for ferma il loop sul vector dopo che usciamo dalla regione di interesse
            if((vec.at(i)>=z_0+(j_max-1)*dmStep) && (vec.at(i)<=z_0+j_max*dmStep)) conteggi_prima++;
            if((vec.at(i)>=z_0+j_max*dmStep+dmSize) && (vec.at(i)<=z_0+(j_max+1)*dmStep+dmSize)) conteggi_dopo++;
        }
        
        if (conteggi_prima>=conteggi_dopo) Z_max = media(vec, z_0 + (j_max-1)*dmStep, z_0 + (j_max)*dmStep + dmSize);
        else if (conteggi_prima<conteggi_dopo) Z_max = media(vec, z_0+j_max*dmStep, z_0+(j_max+1)*dmStep+dmSize);
    }

    return Z_max;
}