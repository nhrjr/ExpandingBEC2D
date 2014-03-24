#ifndef EXP_RK4_TOOLS_H__
#define EXP_RK4_TOOLS_H__

#include <iostream>
#include <complex>
#include <math.h>
#include <complexgrid.h>
#include <bh3binaryfile.h>
#include <vector>
#include <omp.h>
#include <string>
#include <iomanip>



using namespace std;

// Inherit PathOptions from bh3binaryfile.h with additional Options for RK4 and the Potential
typedef struct : PathOptions {
    complex<double> omega_x,omega_y; // Frequency of the harmonic trap
    double min_x,min_y; // Coordinate boundaries
    complex<double> scale_factor; //Scale factor
    complex<double> t_abs; //Absolute time 
    complex<double> exp_factor; //Expansion factor
    double g; // coupling constant
    double ITP_step, RTE_step; // stepsize for the timeiteration
    int n_it_ITP; // number of timesteps
    int n_it_ITP1; // number of timesteps
    int n_it_ITP2; // number of timesteps
    int n_it_RTE; // number of timesteps
    int n_save_RTE; // times, when to save the process 
    int n_save_ITP; // replace with snapshot_times
    int times; // naming of the datafile - time of the snapshot
    std::string name; // naming of the datafile
    std::string config; // name of the config file
    std::string workingdirectory;
    bool startgrid[3];
    int threads;
    //Vortex Positions and winding Number
    int Q;
    
} Options;


class RK4
{
  public:
    RK4();
    // RK4(Options &opt);
    RK4(ComplexGrid* &c,Options &opt);    
    ~RK4();
    
    // Propagatoren
    void itpToTime(Options &opt,bool plot);
    void rteToTime(Options &opt,bool plot);
    void ITP(ComplexGrid* & pPsi,Options &opt);
    void RTE(ComplexGrid* & pPsi,Options &opt);
    
    
    // Hilfsfunktionen 
    void cli_plot(Options &opt,string name,int counter_state, int counter_max, double start,bool plot);  
    double phase_save(ComplexGrid* & pPsi,int a,int b);
    
    // save the Grid to file
    void save_2D(ComplexGrid* & pPsi,Options &opt);
   
    // StorageObjects for the wavefunction and its phase
    ComplexGrid* pPsi;
    ComplexGrid* pPhase;
    
    // Coordinates
    vector<double> x_axis,y_axis; 

  private:

    // double gauss(double x,double y); //A simple Gaussian
   
    // Scaling of Wavefunction after every timestep in ITP and RTE
    void rescale(ComplexGrid* & pPsi, Options &opt);
    
    // Hilfsfunktionen fuer ITP
    void computeK_ITP(ComplexGrid* &pPsi, vector<ComplexGrid> &k,Options &opt,complex<double> &t_ITP);
    void computeK_RTE(ComplexGrid* &pPsi, vector<ComplexGrid> &k,Options &opt,complex<double> &t_RTE);
    void TimeStepRK4(ComplexGrid* &pPsi,vector<ComplexGrid> &k,Options &opt,complex<double> &t);
    void Neumann(ComplexGrid &k,ComplexGrid &PsiCopy,Options &opt);
    void NeumannRTE(ComplexGrid &k,ComplexGrid &wavefct,Options &opt);
    void Dirichlet(ComplexGrid* &pPsi,Options &opt);
    void DirichletK(ComplexGrid &pPsi,Options &opt);

    complex<double> T(ComplexGrid & pPsiCopy,int i, int j);
    complex<double> V(ComplexGrid & pPsicopy,int i, int j,Options &opt);   
    
    // Hilfsfunktionen fuer RTE
   
    complex<double> function_RTE(ComplexGrid &wavefct,int i, int j, Options &opt);
    // complex<double> function_RTE2(ComplexGrid &pPsiCopy,int i, int j,Options &opt);
    complex<double> interaction(complex<double> a,Options &opt);
    // complex<double> grad_x(complex<double> a, complex<double> b);
    // complex<double> grad_y(complex<double> a, complex<double> b);
    // complex<double> lambda_x(Options &opt);
    // complex<double> lambda_x_dot(Options &opt);
    // complex<double> lambda_y(Options &opt);
    // complex<double> lambda_y_dot(Options &opt);
    // complex<double> x_expand(complex<double> a,Options &opt);
    // complex<double> y_expand(complex<double> a,Options &opt);
    
    // Hilfsvariablen
    complex<double> h_x, h_y;
    complex<double> integral(ComplexGrid* & pPsi,Options &opt);
    complex<double> Integral;
    complex<double> Integral_aux;
    
    
    // some used constants
    
    double pi; //acos(-1.0L);
    complex<double>  zero,half,one,two,four,six,i_unit;




    // inline functions for stuff

   inline complex<double> rte_kinetic(ComplexGrid &wavefct, int i, int j, Options &opt)
   {
   return i_unit * ( laplacian_x(wavefct,i,j,opt)/two /* /(two*lambda_x(opt)*lambda_x(opt) )*/  + laplacian_y(wavefct,i,j,opt)/two /*/(two*lambda_y(opt)*lambda_y(opt) )*/ );
   }

   inline complex<double> rte_potential(int i, int j, Options &opt)
   {
    complex<double> xvalue = complex<double>(x_axis[i],0);
    complex<double> yvalue = complex<double>(y_axis[j],0);
   return i_unit * (half * opt.omega_x * opt.omega_x * xvalue * xvalue + half * opt.omega_y * opt.omega_y * yvalue * yvalue);
   }

   inline complex<double> rte_interaction(ComplexGrid &wavefct,int i, int j, Options &opt)
   {
   return i_unit * complex<double>(opt.g,0) * norm(wavefct(0,i,j,0));
   }

   inline complex<double> rte_expandingframe(ComplexGrid &wavefct,int i, int j, Options &opt)
   {
    complex<double> xvalue = complex<double>(x_axis[i],0);
    complex<double> yvalue = complex<double>(y_axis[j],0);
   return (lambda_x_dot(opt)/lambda_x(opt)) * x_expand(xvalue,opt) * grad_x(wavefct,i,j) + (lambda_y_dot(opt)/lambda_y(opt)) * y_expand(yvalue,opt) * grad_y(wavefct,i,j);
   }

   inline complex<double> laplacian_x(ComplexGrid &wavefct,int i, int j, Options &opt)
   {
    return ( wavefct(0,i+1,j,0) - two * wavefct(0,i,j,0) + wavefct(0,i-1,j,0) ) / (h_x * h_x);
   }

   inline complex<double> laplacian_y(ComplexGrid &wavefct,int i, int j, Options &opt)
   {
   return ( wavefct(0,i,j+1,0) - two * wavefct(0,i,j,0) + wavefct(0,i,j-1,0) ) / (h_y * h_y);
   }

   inline complex<double> x_expand(complex<double> a,Options &opt)
   {
   return ((-complex<double>(opt.grid[1],0)/two+a)*h_x*lambda_x(opt));
   }

   inline complex<double> y_expand(complex<double> a,Options &opt)
   {
   return ((-complex<double>(opt.grid[2],0)/two+a)*h_y*lambda_y(opt));
   }

   inline complex<double> grad_x(ComplexGrid &wavefct,int i, int j)
   { //Central-difference x-grad approximation
   return (wavefct(0,i+1,j,0) + wavefct(0,i-1,j,0) ) / (two*h_x) ;
   }

   inline complex<double> grad_y(ComplexGrid &wavefct,int i, int j)
   { //Central-difference y-grad approximation
   return (wavefct(0,i,j+1,0) + wavefct(0,i,j-1,0) ) / (two*h_y) ;
   }

   inline complex<double> lambda_x(Options &opt)
   {
   return sqrt(one+opt.exp_factor*opt.omega_x*opt.omega_x*opt.t_abs*opt.t_abs);
   }

   inline complex<double> lambda_x_dot(Options &opt)
   {
   return (opt.exp_factor*opt.omega_x*opt.omega_x*opt.t_abs/sqrt(one+opt.exp_factor*opt.omega_x*opt.omega_x*opt.t_abs*opt.t_abs));
   }

   inline complex<double> lambda_y(Options &opt)
   {
   return sqrt(one+opt.exp_factor*opt.omega_y*opt.omega_y*opt.t_abs*opt.t_abs);
   }

   inline complex<double> lambda_y_dot(Options &opt)
   {
   return (opt.exp_factor*opt.omega_y*opt.omega_y*opt.t_abs/sqrt(one+opt.exp_factor*opt.omega_y*opt.omega_y*opt.t_abs*opt.t_abs));
   }
      

  
};

#endif // EXP_RK4_TOOLS_H__