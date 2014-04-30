#ifndef EXP2D_TOOLS_H__
#define EXP2D_TOOLS_H__

#include <iostream>
#include <complex>
#include <math.h>
#include <complexgrid.h>
#include <bh3binaryfile.h>
#include <gauss_random.h>
#include <vector>
#include <string>
#include <iomanip>
#include <gauss_random.h>
#include <stdlib.h>
#include <eigen3/Eigen/Dense>

using namespace std;
using namespace Eigen;

typedef struct {
        // From bh3binaryfile
    double N; // Number of particles
    int32_t grid[4];  // gridsize
    double klength[3];
        // my own
    vector<double> stateInformation; // passing information to the observables atm FIXME : this is bad, but I don't know how to do it better atm
    complex<double> omega_x,omega_y; // Frequency of the harmonic trap
    complex<double> dispersion_x, dispersion_y; // dispersion relation for the expandion frame
    double min_x,min_y; // Coordinate boundaries
    complex<double> scale_factor; //Scale factor
    complex<double> t_abs; //Absolute time // remove from opt! put into the function, don't need it here
    complex<double> exp_factor; //Expansion factor
    double g; // coupling constant
    double ITP_step, RTE_step; // stepsize for the timeiteration
    // int n_it_ITP; // number of timesteps
    // int n_it_ITP1; // number of timesteps
    // int n_it_ITP2; // number of timesteps
    int n_it_RTE; // number of timesteps
    string name; // naming of the datafile      // think about that naming system remove it from here
    string config; // name of the config file 
    string workingdirectory;   // remove it from here, only needed in the program itself
    string workingfile;
    // bool startgrid[3];
    //Vortex Positions and winding Number
    // bool RTE_only;
    int samplesize;
    string runmode; // Use this to control the program flow: first char determines if the program is loading from a dataset or using ITP to generate the necessary datafile
                     // second char determines if expanding coordinates are used or not
                     // third char determines if potential is switch on for the differential equation
    
} Options;

class Observables {
        public:
        
        double Ekin, particle_count, healing_length;
        ArrayXd number;
        ArrayXd k;
        
        Observables() {};
        Observables(int avgrid);
        ~Observables() {};
    
        Observables operator+ (const Observables &a) const;
        Observables operator- (const Observables &a) const;
        Observables operator* (const Observables &a) const;
    
        Observables operator* (double d) const;
        Observables operator/ (double d) const;
    
        Observables &operator+= (const Observables &a);
        Observables &operator-= (const Observables &a);
        Observables &operator*= (const Observables &a);
        
        Observables &operator/= (double d);
    
        Observables &operator*= (double d);
};

void optToPath(Options &opt,PathOptions &pathopt);
void pathToOpt(PathOptions &pathopt,Options &opt);
void readDataFromHDF5(ComplexGrid* &g,Options &opt);
void saveDataToHDF5(ComplexGrid* &g, Options &opt);
void noiseTheGrid(ComplexGrid &g);



inline Observables::Observables(int avgrid) :
        number(avgrid),
        k(avgrid)
{
    Ekin = particle_count = healing_length = 0.0;
    number.setZero();
    k.setZero();
}

inline Observables Observables::operator+ (const Observables &a) const
{
    Observables ret(number.size());  

    ret.particle_count = particle_count + a.particle_count;
    ret.healing_length = healing_length + a.healing_length; 
    ret.Ekin = Ekin + a.Ekin;
    ret.number = number + a.number; 
    // ret.k = k + a.k;
    
    return ret;
}

inline Observables Observables::operator- (const Observables &a) const
{
    Observables ret(number.size());  

    ret.particle_count = particle_count - a.particle_count;
    ret.healing_length = healing_length - a.healing_length;     
    ret.Ekin = Ekin - a.Ekin;
    ret.number = number - a.number; 
    ret.k = k - a.k;
    
    return ret;
}

inline Observables Observables::operator* (const Observables &a) const
{
    Observables ret(number.size());  

    ret.particle_count = particle_count * a.particle_count;
    ret.healing_length = healing_length * a.healing_length;     
    ret.Ekin = Ekin * a.Ekin;
    ret.number = number * a.number; 
    ret.k = k * a.k;
    
    return ret;
}

inline Observables Observables::operator* (double d) const
{  
    Observables ret(number.size());

    ret.particle_count = particle_count * d;
    ret.healing_length = healing_length * d;    
    ret.Ekin = Ekin * d;
    ret.number = number * d;    
    ret.k = k * d;
}

inline Observables Observables::operator/ (double d) const
{  
    Observables ret(number.size());

    ret.particle_count = particle_count / d;
    ret.healing_length = healing_length / d;    
    ret.Ekin = Ekin / d;
    ret.number = number / d;    
    ret.k = k / d;
}

inline Observables & Observables::operator+= (const Observables &a)
{
    *this = *this + a;
    return *this;
}

inline Observables & Observables::operator-= (const Observables &a)
{
    *this = *this - a;
    return *this;
}

inline Observables & Observables::operator*= (const Observables &a)
{
    *this = *this * a;
    return *this;
}

inline Observables & Observables::operator/= (double d)
{
    *this = *this / d;
    return *this;
}

inline Observables & Observables::operator*= (double d)
{
    *this = *this * d;
    return *this;
}

#endif // EXP2D_TOOLS_H__