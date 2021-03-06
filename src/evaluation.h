#ifndef EXP2D_EVALUATION_H__
#define EXP2D_EVALUATION_H__

#define EIGEN_FFTW_DEFAULT

#include <iostream>
#include <sys/stat.h>
#include <complex>
#include <cmath>
#include <numeric>
#include <stack>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <omp.h>
#include <string>
#include <sstream>
#include <gsl/gsl_sf_zeta.h>

#include "coordinate.h"
#include "contour.h"
#include "plot_with_mgl.h"
#include "tools.h"
#include "matrixdata.h"
#include "observables.h"
#include "lmfitter.h"

#include <eigen3/Eigen/Dense>



using namespace std;
using namespace Eigen;



class Eval{
public:
	Eval(shared_ptr<MatrixData> d,Options o);
	~Eval();

	void process();
	void save();
	bool checkResizeCondition();
	int getVortexNumber();
	bool checkResizeCondition(vector<int> &edges);

	Observables totalResult;
	vector<list<VortexData>> vlist;
	vector<c_set> contour;
	vector<MatrixXi> densityLocationMap;

	Ellipse ellipse;

	shared_ptr<MatrixData> data;
	Options opt;

	vector<double> punkte;

	// TEMPORARY
	double steigung, fehler/*, abschnitt*/;

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
	
	MatrixXd phase;
	MatrixXd density;

	int snapshot_time;

	vector<vector<Coordinate<int32_t>>> densityCoordinates;
	vector<double> x_dist,y_dist,x_dist_grad,y_dist_grad;
	vector<int> densityCounter;

	void CombinedEval();
	void CombinedSpectrum();

	Observables calculator(MatrixXcd DATA,int sampleindex);
	void aspectRatio(Observables &obs, int &sampleindex);
	void getVortices(MatrixXcd &DATA, vector<Coordinate<int32_t>> &densityCoordinates,list<VortexData> &vlist);
	void getDensity();

	void smooth(MatrixXd &dens);
	int checkSum(MatrixXi &d,int &i, int &j);
	void erosion(MatrixXi &d);
	void dilation(MatrixXi &d);

	void floodFill(MatrixXi &dens);
	void fillHoles(MatrixXi &dens);

	vector<double> polarDensity();
	vector<int> findMajorMinor();
	Ellipse fitEllipse(c_set &Data);

	c_set generateContour(Ellipse &ellipse);
	c_set generateContour(vector<double>& params_tf);
	vector<Coordinate<int32_t>> generate_density_coordinates(vector<double>& params_tf);

	vector<double> fitTF();
	

	int get_phase_jump(const Coordinate<int32_t> &c, const Vector<int32_t> &v);
	void findVortices(vector<Coordinate<int32_t>> &densityCoordinates, list<VortexData> &vlist);

	inline double norm(Coordinate<double> &a, Coordinate<double> &b, double &h_x, double &h_y);
	void calc_fields(MatrixXcd &DATA, Options &opt);

	// Contour Tracking Algorithm

	// c_set trackContour(const RealGrid &data);
	
	// inline Coordinate<int32_t> nextClockwise(Coordinate<int32_t> &s, int32_t &direction);
	// inline void setDirection(int32_t &direction);
	// void findInitialP(RealGrid &data,Coordinate<int32_t> &p,Coordinate<int32_t> &s, Coordinate<int32_t> *initial);
	// void findMostRightP(c_set &contour, Coordinate<int32_t> &p);


	// Helpermethods to access radial vectors:
	void checkNextAngles(vector<double> &r, int &i);
	void cyclicAssignment(vector<double> &r, int i, double rvalue);
	double cyclicReadout(vector<double> &r, int i);

};





#endif // EXP2D_EVALUATION_H__