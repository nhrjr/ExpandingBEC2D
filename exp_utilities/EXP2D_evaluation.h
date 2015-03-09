#ifndef EXP2D_EVALUATION_H__
#define EXP2D_EVALUATION_H__

#include <iostream>
#include <sys/stat.h>
#include <complex>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <complexgrid.h>
#include <realgrid.h>
#include <bh3binaryfile.h>
#include <coordinate.h>
#include <vector>
#include <unordered_set>
#include <omp.h>
#include <string>
#include <sstream>
#include <EXP2D_Contour.h>
#include <plot_with_mgl.h>
#include <EXP2D_tools.h>
#include <EXP2D_observables.h>
#include <eigen3/Eigen/Dense>

using namespace std;
using namespace Eigen;



class Eval{
public:
	Eval();
	~Eval();

	// wrapperfunctions 
	void saveData(vector<MatrixXcd> &wavefctVec,Options &external_opt,int external_snapshot_time,string external_runname); // If data comes as a vector of matrices (from statistics RTE)
	void saveData(MatrixXcd &wavefct,Options &external_opt,int external_snapshot_time,string external_runname); // If data comes only as a Matrix (from ITP)
	void saveData2DSlice(vector<ComplexGrid> &wavefctVec, Options & external_opt, int external_snapshot_time, string external_runname, int sliceNumber); // if data comes as a vector of ComplexGrids, just eval a sclice of the 3D data.
	void saveDataFromEval(Options &external_opt,int &external_snapshot_time,string &external_runname,vector<Eval> &extEval);
	void evaluateData(); // calculate the observables
	void evaluateDataITP();
	void plotData(); // plot Results
	bool checkResizeCondition();
	int getVortexNumber();
	void convertFromDimensionless();


	// Observables.h
	Observables totalResult;
	vector<PathResults> pres;
	vector<c_set> contour;

	


private:

	typedef struct{
		int32_t length;
		Coordinate<int32_t> start;
		Coordinate<int32_t> stop;
	} lineData;

	typedef struct{
		Coordinate<int32_t> c;
		double phi;
		double r;
	} contourData;

	// data savefiles

	RealGrid phase, zeros;
	string runname;
	vector<ComplexGrid> PsiVec;
	
	Options opt;
	int snapshot_time;
	vector<RealGrid> densityLocationMap;
	vector<vector<Coordinate<int32_t>>> densityCoordinates;
	vector<double> x_dist,y_dist,x_dist_grad,y_dist_grad;
	vector<int> densityCounter;

	void CombinedEval();
	void CombinedSpectrum();

	// doing functinos
	Observables calculator(ComplexGrid data,int sampleindex);
	Observables calculatorITP(ComplexGrid data,int sampleindex);
	void aspectRatio(Observables &obs, int &sampleindex);
	void getVortices(ComplexGrid &data, vector<Coordinate<int32_t>> &densityCoordinates,PathResults &pres);
	void getDensity(ComplexGrid &data, RealGrid &densityLocationMap, vector<Coordinate<int32_t>> &densityCoordinates,int &densityCounter);
	

	int get_phase_jump(const Coordinate<int32_t> &c, const Vector<int32_t> &v, const RealGrid &phase);
	void findVortices(vector<Coordinate<int32_t>> &densityCoordinates, list<VortexData> &vlist);

	inline double norm(Coordinate<double> &a, Coordinate<double> &b, double &h_x, double &h_y);
	inline void pairDistanceHistogram(PathResults &pres, double &distance, double &coordDistance);
	void getVortexDistance(PathResults &pres);
	void calc_fields(ComplexGrid &data, Options &opt);
	void checkEdges();

	// Contour Tracking Algorithm

	// c_set trackContour(const RealGrid &data);
	
	// inline Coordinate<int32_t> nextClockwise(Coordinate<int32_t> &s, int32_t &direction);
	// inline void setDirection(int32_t &direction);
	// void findInitialP(RealGrid &data,Coordinate<int32_t> &p,Coordinate<int32_t> &s, Coordinate<int32_t> *initial);
	// void findMostRightP(c_set &contour, Coordinate<int32_t> &p);


};





#endif // EXP2D_EVALUATION_H__