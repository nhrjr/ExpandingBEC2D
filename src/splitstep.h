#ifndef EXP2D_SPLITSTEP_H__
#define EXP2D_SPLITSTEP_H__

#include <iostream>
#include <complex>
#include <math.h>
#include <vector>
#include <omp.h>
#include <string>
#include <iomanip>

#include <eigen3/Eigen/Dense>

#include "tools.h"
#include "evaluation.h"
#include "binaryfile.h"
#include "constants.h"
#include "plot_with_mgl.h"
#include "matrixdata.h"

using namespace std;
using namespace Eigen;

class SplitStep
{
  public:
    SplitStep(Options &o);
    void assignMatrixData(shared_ptr<MatrixData> d);
    void setVariables();
    virtual void timeStep(double delta_t) = 0;

    void setOptions(const Options &externaloptions);
    void RunSetup();
    
    // Propagator
    void splitToTime(string runName);

    int samplesize;

    // Plotting and progress functions 
    void cli(string name,int &slowestthread, vector<int> threadinfo, vector<int> stateOfLoops, int counter_max, double start);
    void plot(const string name);
    
    shared_ptr<MatrixData> w;

    // internal RunOptions, use setOptions(Options) to update from the outside
    Options opt;
    vector<int> snapshot_times;

  protected:

    vector<vector<double>> kspace;
    vector<double> x_axis,y_axis;
    MatrixXcd kprop, kpropITP, kprop_x, kprop_y, Vgrid, PotentialGrid, kprop_x_strang, kprop_y_strang;

    MatrixData::MetaData meta;

    // Variables
    complex<double> h_x, h_y,h_z;

    int SLICE_NUMBER;
        
    // some used constants
    double pi;
    complex<double> t_RTE;
};

class SplitRot : public SplitStep {
public:
    SplitRot(Options &o) : SplitStep(o) {}
    virtual void timeStep(double delta_t);
};

class SplitTrap : public SplitStep {
public:
    SplitTrap(Options &o) : SplitStep(o) {}
    virtual void timeStep(double delta_t);
};

class SplitFree : public SplitStep {
public:
    SplitFree(Options &o) : SplitStep(o) {}
    virtual void timeStep(double delta_t);
};

class SplitRotStrang : public SplitStep {
public:
    SplitRotStrang(Options &o) : SplitStep(o) {}
    virtual void timeStep(double delta_t);
};

class SplitITP : public SplitStep {
public:
    SplitITP(Options &o) : SplitStep(o) {}
    virtual void timeStep(double delta_t);
};

#endif // EXP2D_SPLITSTEP_H__