#ifndef BH3_OBSERVABLES_H__
#define BH3_OBSERVABLES_H__


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <list>
#include <vector>
#include <complex>

#include "coordinate.h"
#include "bh3propagator.h"

#include "complexgrid.h"
#include "realgrid.h"

#include <eigen3/Eigen/Core>

using namespace Eigen;

class RealGrid;
class ComplexGrid;

class Bh3Evaluation {
	public:						// Types
		typedef struct {
			int32_t n;
			Coordinate<double> x;
			vector<Vector<double> > velocity;
			list<Coordinate<int32_t> > points;
			int32_t num_points;
			double pair_distance;
		} VortexData;
		
		class Averages {
			public:
				double nm3, nm2, nm1, n0, n1, n2, n3, mass_zeros, num_vortices;
				complex<double> meanfield;
				ArrayXXf vortex_velocities_x, vortex_velocities_y, vortex_velocities_norm;
	
                ArrayXd velocities_x, velocities_y, velocities_z, velocities_norm;
				ArrayXd av_vortex_velocity;
				double av_velocity;
				double pair_distance_all, pair_distance_nonzero, max_pair_distance;
				ArrayXf /*pd_histogram_all, pd_histogram_closest,*/ g2, g2_av, g2_vv, g2_closest;
				ArrayXcd g1;
				double kinetick_int, pressure_int, interaction_int, particle_count, ckinetic_int, ikinetic_int,
						j_int, cj_int, ij_int, Ekin;
				ArrayXd number, omega, ikinetick, ckinetick, kinetick, pressure, j, cj, ij, pressure_kin_mixture, phase;
				ArrayXd ikinetick_wo_phase, ckinetick_wo_phase, kinetick_wo_phase, pressure_wo_phase;
				ArrayXd k;
				double time;
				
				Averages() {}
				Averages(int avgrid, int deltas);
				Averages operator+ (const Averages &a) const;
				Averages &operator+= (const Averages &a);
				Averages operator- (const Averages &a) const;
				Averages &operator-= (const Averages &a);
				Averages operator* (const Averages &a) const;
				Averages &operator*= (const Averages &a);
				Averages operator/ (double d) const;
				Averages &operator/= (double d);
				Averages operator* (double d) const;
				Averages &operator*= (double d);
		};
		
		struct PathResults {
			list<VortexData> vlist;
		};
		
		enum Direction { N = 0, NO = 1, O = 2, SO = 3, S = 4, SW = 5, W = 6, NW = 7 };
		enum Space { RSpace, KSpace };
		
		// Hilfsstruktur fuer get_vortex
		class fv_data{
			public:
				Coordinate<int32_t> x;
				Direction d, startd;
				bool finished;
				
				fv_data() { startd = d = N; finished = false; }
				
				fv_data(const Coordinate<int32_t> &c, Direction nd) { x = c; startd = d = nd; finished = false; }
		};
		
	protected:					// Properties
		PathOptions options;
		RealGrid *phase, *zeros;
		vector<ComplexGrid> data;
		vector<vector<double> > kspace;
		vector<vector<complex<double> > > kfspace, kbspace;
		PathResults pres;
		Averages ares;
		Space space;

	public:						// Public methods
		Bh3Evaluation(const PathOptions &opt, Space s = KSpace);
		Bh3Evaluation(const PathOptions &opt, const vector<ComplexGrid> &data, Space s = KSpace);
		
		~Bh3Evaluation();
		
		void setData(const vector<ComplexGrid> &d, Space s = KSpace);
		void setTime(double time) {ares.time = time;}
		void assert_space(Space s);
	
		// evaluation functions
		void calc_fields();
		void evaluate_vortices();		// FIXME: only 2D right now!!
		void calc_radial_averages();
		void calc_g1();
		
		inline const Averages &get_averageable_results() const {return ares;}
		inline const PathResults &get_path_results() const {return pres;}
		inline const RealGrid &get_phase_field() const {return *phase;}
		inline const RealGrid &get_zeros_field() const {return *zeros;}
	protected:					// Protected methods
		Bh3Evaluation() {}
		
		// initialization
		void initialize(const PathOptions &opt);
		void init_k_space();
		
		// Direction helper functions
		Direction next_direction(Direction d) const;
		Direction prev_direction(Direction d) const;
		Vector<int32_t> dir_vector(Direction d) const;
		
		// vortex evaluation helper functions
		int get_phase_jump(const Coordinate<int32_t> &c, const Vector<int32_t> &v, const RealGrid *phase) const;
		int get_quantization(const Coordinate<int32_t> &start, const RealGrid *phase, const RealGrid *zeros) const;
		bool get_vortex(const Coordinate<int32_t> &c, const RealGrid *phase, const RealGrid *zeros, double &mass_zeros, vector< vector< vector<bool> > > &checked, VortexData &vortex) const;
		void find_vortices(const RealGrid *phase, const RealGrid *zeros, list<VortexData> &vlist, double &mass_zeros) const;
		void inc_pd_histogram(vector<float> &histogram, double distance) const;
		void calc_g2();
		void calc_vortex_distances();
		void calc_vortex_velocities(vector<list<VortexData> > &vlist);
		double get_phase_diff(const Coordinate<int32_t> &c1, const Coordinate<int32_t> &c2) const;
		complex<double> calc_compressible_part_x(int x, int y, int z, complex<double> kx, complex<double> ky, complex<double> kz) const;
		complex<double> calc_compressible_part_y(int x, int y, int z, complex<double> kx, complex<double> ky, complex<double> kz) const;
		complex<double> calc_compressible_part_z(int x, int y, int z, complex<double> kx, complex<double> ky, complex<double> kz) const;
		
		void calc_fields(const ComplexGrid &data, RealGrid *phase, RealGrid *zeros);
};

// some more Direction helper functions
inline Bh3Evaluation::Direction operator+ (Bh3Evaluation::Direction d, int o)
{
	return (Bh3Evaluation::Direction) ((((int)d) + o) % 8);
}

inline Bh3Evaluation::Direction operator- (Bh3Evaluation::Direction d, int o)
{
	return (Bh3Evaluation::Direction) ((((int)d) + 8 - o) % 8);
}

inline Bh3Evaluation::Direction &operator+= (Bh3Evaluation::Direction &d, int o)
{
	d = d + o;
	return d;
}

inline Bh3Evaluation::Direction &operator-= (Bh3Evaluation::Direction &d, int o)
{
	d = d - o;
	return d;
}

inline Bh3Evaluation::Direction operator++ (Bh3Evaluation::Direction &d)
{
	d = d + 1;
	return d;
}

inline Bh3Evaluation::Direction operator-- (Bh3Evaluation::Direction &d)
{
	d = d - 1;
	return d;
}

template <class T>
list<T> operator+ (const list<T> &l1, const list<T> &l2)
{
	list<T> ret(l1);
	ret.insert(ret.end(), l2.begin(), l2.end());
	return ret;
}

template <class T>
vector<T> operator+ (const vector<T> &v1, const vector<T> &v2)
{
	vector<T> ret(v1.size());
	for(int i = 0; i < ret.size(); i++)
	{
		ret[i] = v1[i] + v2[i];
	}
	return ret;
}

template <class T>
vector<T> operator+= (vector<T> &v1, const vector<T> &v2)
{
	v1 = v1 + v2;
	return v1;
}

template <class T>
vector<T> operator- (const vector<T> &v1, const vector<T> &v2)
{
	vector<T> ret(v1.size());
	for(int i = 0; i < ret.size(); i++)
	{
		ret[i] = v1[i] - v2[i];
	}
	return ret;
}

template <class T>
vector<T> operator/ (const vector<T> &v1, double n)
{
	vector<T> ret(v1.size(), v1[0]);
	for(int i = 0; i < ret.size(); i++)
	{
		ret[i] = v1[i] / n;
	}
	return ret;
}

template <class T>
vector<T> operator* (const vector<T> &v1, const vector<T> &v2)
{
	vector<T> ret(v1.size());
	for(int i = 0; i < ret.size(); i++)
	{
		ret[i] = v1[i] * v2[i];
	}
	return ret;
}

template <class T>
vector<T> operator* (const vector<T> &v1, double n)
{
	vector<T> ret(v1.size(), v1[0]);
	for(int i = 0; i < ret.size(); i++)
	{
		ret[i] = v1[i] * n;
	}
	return ret;
}

template <class T>
vector<T> sqrt (const vector<T> &v)
{
	vector<T> ret(v.size(), v[0]);
	for(int i = 0; i < ret.size(); i++)
	{
		ret[i] = sqrt(v[i]);
	}
	return ret;
}

inline Bh3Evaluation::Averages::Averages(int avgrid, int deltas) :
		vortex_velocities_x(deltas, 4*avgrid),
		vortex_velocities_y(deltas, 4*avgrid),
		vortex_velocities_norm(deltas, 4*avgrid),
		av_vortex_velocity(deltas),
		velocities_x(avgrid),
		velocities_y(avgrid),
		velocities_z(avgrid),
		velocities_norm(avgrid),
		g1(avgrid),
		g2(avgrid),
		g2_av(avgrid),
		g2_vv(avgrid),
		g2_closest(avgrid),
		number(avgrid),
		ikinetick(avgrid),
		ckinetick(avgrid),
		kinetick(avgrid),
		pressure(avgrid),
		pressure_kin_mixture(avgrid),
		j(avgrid),
		cj(avgrid),
		ij(avgrid),
		omega(avgrid),
		k(avgrid),
		ikinetick_wo_phase(avgrid),
		ckinetick_wo_phase(avgrid),
		kinetick_wo_phase(avgrid),
		pressure_wo_phase(avgrid),
		phase(avgrid)
{
	time = nm3 = nm2 = nm1 = n0 = n1 = n2 = n3 = mass_zeros = num_vortices = 0.0;
	pair_distance_all = pair_distance_nonzero = max_pair_distance = 0.0;
	Ekin = kinetick_int = pressure_int = interaction_int = particle_count = ckinetic_int = ikinetic_int = j_int = cj_int = ij_int = 0.0;
	meanfield = 0.0;
	av_velocity = 0.0;

    vortex_velocities_x.setZero();
    vortex_velocities_y.setZero();
    vortex_velocities_norm.setZero();
    av_vortex_velocity.setZero();
    velocities_x.setZero();
    velocities_y.setZero();
    velocities_z.setZero();
    velocities_norm.setZero();
    g1.setZero();
    g2.setZero();
    g2_av.setZero();
    g2_vv.setZero();
    g2_closest.setZero();
    number.setZero();
    ikinetick.setZero();
    ckinetick.setZero();
    kinetick.setZero();
    pressure.setZero();
    pressure_kin_mixture.setZero();
    j.setZero();
    cj.setZero();
    ij.setZero();
    omega.setZero();
    k.setZero();
    ikinetick_wo_phase.setZero();
    ckinetick_wo_phase.setZero();
    kinetick_wo_phase.setZero();
    pressure_wo_phase.setZero();
    phase.setZero();
}

inline Bh3Evaluation::Averages Bh3Evaluation::Averages::operator+ (const Averages &a) const
{
	Averages ret(number.size(), vortex_velocities_x.size());
	
	ret.time = time + a.time;
	ret.nm3 = nm3 + a.nm3;
	ret.nm2 = nm2 + a.nm2;
	ret.nm1 = nm1 + a.nm1;
	ret.n0 = n0 + a.n0;
	ret.n1 = n1 + a.n1;
	ret.n2 = n2 + a.n2;
	ret.n3 = n3 + a.n3;
	ret.mass_zeros = mass_zeros + a.mass_zeros;
	ret.num_vortices = num_vortices + a.num_vortices;
	ret.pair_distance_all = pair_distance_all + a.pair_distance_all;
	ret.pair_distance_nonzero = pair_distance_nonzero + a.pair_distance_nonzero;
	ret.max_pair_distance = max_pair_distance + a.max_pair_distance;
	ret.kinetick_int = kinetick_int + a.kinetick_int;
	ret.pressure_int = pressure_int + a.pressure_int;
	ret.interaction_int = interaction_int + a.interaction_int;
	ret.particle_count = particle_count + a.particle_count;
	ret.ckinetic_int = ckinetic_int + a.ckinetic_int;
	ret.ikinetic_int = ikinetic_int + a.ikinetic_int;
	ret.j_int = j_int + a.j_int;
	ret.cj_int = cj_int + a.cj_int;
	ret.ij_int = ij_int + a.ij_int;
	ret.Ekin = Ekin + a.Ekin;
	ret.meanfield = meanfield + a.meanfield;
	ret.av_vortex_velocity = av_vortex_velocity + a.av_vortex_velocity;
	ret.av_velocity = av_velocity + a.av_velocity;
	
	ret.vortex_velocities_x = vortex_velocities_x + a.vortex_velocities_x;
	ret.vortex_velocities_y = vortex_velocities_y + a.vortex_velocities_y;
	ret.vortex_velocities_norm = vortex_velocities_norm + a.vortex_velocities_norm;
	ret.velocities_x = velocities_x + a.velocities_x;
	ret.velocities_y = velocities_y + a.velocities_y;
	ret.velocities_z = velocities_z + a.velocities_z;
	ret.velocities_norm = velocities_norm + a.velocities_norm;
	ret.number = number + a.number;
	ret.ikinetick = ikinetick + a.ikinetick;
	ret.ckinetick = ckinetick + a.ckinetick;
	ret.kinetick = kinetick + a.kinetick;
	ret.pressure = pressure + a.pressure;
	ret.pressure_kin_mixture = pressure_kin_mixture + a.pressure_kin_mixture;
	ret.phase = phase + a.phase;
	ret.omega = omega + a.omega;
	ret.j = j + a.j;
	ret.ij = ij + a.ij;
	ret.cj = cj + a.cj;
	ret.g1 = g1 + a.g1;
	ret.g2 = g2 + a.g2;
	ret.g2_av = g2_av + a.g2_av;
	ret.g2_vv = g2_vv + a.g2_vv;
	ret.g2_closest = g2_closest + a.g2_closest;
	ret.k = k + a.k;
	ret.ikinetick_wo_phase = ikinetick_wo_phase + a.ikinetick_wo_phase;
	ret.ckinetick_wo_phase = ckinetick_wo_phase + a.ckinetick_wo_phase;
	ret.kinetick_wo_phase = kinetick_wo_phase + a.kinetick_wo_phase;
	ret.pressure_wo_phase = pressure_wo_phase + a.pressure_wo_phase;
	
	return ret;
}

inline Bh3Evaluation::Averages & Bh3Evaluation::Averages::operator+= (const Averages &a)
{
	*this = *this + a;
	return *this;
}

inline Bh3Evaluation::Averages Bh3Evaluation::Averages::operator- (const Averages &a) const
{
	Averages ret(number.size(), vortex_velocities_x.size());
	
	ret.time = time - a.time;
	ret.nm3 = nm3 - a.nm3;
	ret.nm2 = nm2 - a.nm2;
	ret.nm1 = nm1 - a.nm1;
	ret.n0 = n0 - a.n0;
	ret.n1 = n1 - a.n1;
	ret.n2 = n2 - a.n2;
	ret.n3 = n3 - a.n3;
	ret.mass_zeros = mass_zeros - a.mass_zeros;
	ret.num_vortices = num_vortices - a.num_vortices;
	ret.pair_distance_all = pair_distance_all - a.pair_distance_all;
	ret.pair_distance_nonzero = pair_distance_nonzero - a.pair_distance_nonzero;
	ret.max_pair_distance = max_pair_distance - a.max_pair_distance;
	ret.kinetick_int = kinetick_int - a.kinetick_int;
	ret.pressure_int = pressure_int - a.pressure_int;
	ret.interaction_int = interaction_int - a.interaction_int;
	ret.particle_count = particle_count - a.particle_count;
	ret.ckinetic_int = ckinetic_int - a.ckinetic_int;
	ret.ikinetic_int = ikinetic_int - a.ikinetic_int;
	ret.j_int = j_int - a.j_int;
	ret.cj_int = cj_int - a.cj_int;
	ret.ij_int = ij_int - a.ij_int;
	ret.Ekin = Ekin - a.Ekin;
	ret.meanfield = meanfield - a.meanfield;
	ret.av_vortex_velocity = av_vortex_velocity - a.av_vortex_velocity;
	ret.av_velocity = av_velocity - a.av_velocity;
	
	ret.vortex_velocities_x = vortex_velocities_x - a.vortex_velocities_x;
	ret.vortex_velocities_y = vortex_velocities_y - a.vortex_velocities_y;
	ret.vortex_velocities_norm = vortex_velocities_norm - a.vortex_velocities_norm;
	ret.velocities_x = velocities_x - a.velocities_x;
	ret.velocities_y = velocities_y - a.velocities_y;
	ret.velocities_z = velocities_z - a.velocities_z;
	ret.velocities_norm = velocities_norm - a.velocities_norm;
	ret.number = number - a.number;
	ret.ikinetick = ikinetick - a.ikinetick;
	ret.ckinetick = ckinetick - a.ckinetick;
	ret.kinetick = kinetick - a.kinetick;
	ret.pressure = pressure - a.pressure;
	ret.pressure_kin_mixture = pressure_kin_mixture - a.pressure_kin_mixture;
	ret.phase = phase - a.phase;
	ret.omega = omega - a.omega;
	ret.j = j - a.j;
	ret.ij = ij - a.ij;
	ret.cj = cj - a.cj;
	ret.g1 = g1 - a.g1;
	ret.g2 = g2 - a.g2;
	ret.g2_av = g2_av - a.g2_av;
	ret.g2_vv = g2_vv - a.g2_vv;
	ret.g2_closest = g2_closest - a.g2_closest;
	ret.k = k - a.k;
	ret.ikinetick_wo_phase = ikinetick_wo_phase - a.ikinetick_wo_phase;
	ret.ckinetick_wo_phase = ckinetick_wo_phase - a.ckinetick_wo_phase;
	ret.kinetick_wo_phase = kinetick_wo_phase - a.kinetick_wo_phase;
	ret.pressure_wo_phase = pressure_wo_phase - a.pressure_wo_phase;
	
	return ret;
}

inline Bh3Evaluation::Averages & Bh3Evaluation::Averages::operator-= (const Averages &a)
{
	*this = *this - a;
	return *this;
}

inline Bh3Evaluation::Averages Bh3Evaluation::Averages::operator* (const Averages &a) const
{
	Averages ret(number.size(), vortex_velocities_x.size());
	
	ret.time = time * a.time;
	ret.nm3 = nm3 * a.nm3;
	ret.nm2 = nm2 * a.nm2;
	ret.nm1 = nm1 * a.nm1;
	ret.n0 = n0 * a.n0;
	ret.n1 = n1 * a.n1;
	ret.n2 = n2 * a.n2;
	ret.n3 = n3 * a.n3;
	ret.mass_zeros = mass_zeros * a.mass_zeros;
	ret.num_vortices = num_vortices * a.num_vortices;
	ret.pair_distance_all = pair_distance_all * a.pair_distance_all;
	ret.pair_distance_nonzero = pair_distance_nonzero * a.pair_distance_nonzero;
	ret.max_pair_distance = max_pair_distance * a.max_pair_distance;
	ret.kinetick_int = kinetick_int * a.kinetick_int;
	ret.pressure_int = pressure_int * a.pressure_int;
	ret.interaction_int = interaction_int * a.interaction_int;
	ret.particle_count = particle_count * a.particle_count;
	ret.ckinetic_int = ckinetic_int * a.ckinetic_int;
	ret.ikinetic_int = ikinetic_int * a.ikinetic_int;
	ret.j_int = j_int * a.j_int;
	ret.cj_int = cj_int * a.cj_int;
	ret.ij_int = ij_int * a.ij_int;
	ret.Ekin = Ekin * a.Ekin;
	ret.meanfield = meanfield * a.meanfield;
	ret.av_vortex_velocity = av_vortex_velocity * a.av_vortex_velocity;
	ret.av_velocity = av_velocity * a.av_velocity;
	
	ret.vortex_velocities_x = vortex_velocities_x * a.vortex_velocities_x;
	ret.vortex_velocities_y = vortex_velocities_y * a.vortex_velocities_y;
	ret.vortex_velocities_norm = vortex_velocities_norm * a.vortex_velocities_norm;
	ret.velocities_x = velocities_x * a.velocities_x;
	ret.velocities_y = velocities_y * a.velocities_y;
	ret.velocities_z = velocities_z * a.velocities_z;
	ret.velocities_norm = velocities_norm * a.velocities_norm;
	ret.number = number * a.number;
	ret.ikinetick = ikinetick * a.ikinetick;
	ret.ckinetick = ckinetick * a.ckinetick;
	ret.kinetick = kinetick * a.kinetick;
	ret.pressure = pressure * a.pressure;
	ret.pressure_kin_mixture = pressure_kin_mixture * a.pressure_kin_mixture;
	ret.phase = phase * a.phase;
	ret.omega = omega * a.omega;
	ret.j = j * a.j;
	ret.ij = ij * a.ij;
	ret.cj = cj * a.cj;
	ret.g1 = g1 * a.g1;
	ret.g2 = g2 * a.g2;
	ret.g2_av = g2_av * a.g2_av;
	ret.g2_vv = g2_vv * a.g2_vv;
	ret.g2_closest = g2_closest * a.g2_closest;
	ret.k = k * a.k;
	ret.ikinetick_wo_phase = ikinetick_wo_phase * a.ikinetick_wo_phase;
	ret.ckinetick_wo_phase = ckinetick_wo_phase * a.ckinetick_wo_phase;
	ret.kinetick_wo_phase = kinetick_wo_phase * a.kinetick_wo_phase;
	ret.pressure_wo_phase = pressure_wo_phase * a.pressure_wo_phase;
	
	return ret;
}

inline Bh3Evaluation::Averages & Bh3Evaluation::Averages::operator*= (const Averages &a)
{
	*this = *this * a;
	return *this;
}

inline Bh3Evaluation::Averages Bh3Evaluation::Averages::operator/ (double d) const
{                                                                             
	Averages ret(number.size(), vortex_velocities_x.size());
	
	ret.time = time / d;
	ret.nm3 = nm3 / d;
	ret.nm2 = nm2 / d;
	ret.nm1 = nm1 / d;
	ret.n0 = n0 / d;
	ret.n1 = n1 / d;
	ret.n2 = n2 / d;
	ret.n3 = n3 / d;
	ret.mass_zeros = mass_zeros / d;
	ret.num_vortices = num_vortices / d;
	ret.pair_distance_all = pair_distance_all / d;
	ret.pair_distance_nonzero = pair_distance_nonzero / d;
	ret.max_pair_distance = max_pair_distance / d;
	ret.kinetick_int = kinetick_int  / d;
	ret.pressure_int = pressure_int / d;
	ret.interaction_int = interaction_int / d;
	ret.particle_count = particle_count / d;
	ret.ckinetic_int = ckinetic_int / d;
	ret.ikinetic_int = ikinetic_int / d;
	ret.j_int = j_int / d;
	ret.cj_int = cj_int  / d;
	ret.ij_int = ij_int / d;
	ret.Ekin = Ekin / d;
	ret.meanfield = meanfield / d;
	ret.av_vortex_velocity = av_vortex_velocity / d;
	ret.av_velocity = av_velocity / d;
	
	ret.vortex_velocities_x = vortex_velocities_x / d;
	ret.vortex_velocities_y = vortex_velocities_y / d;
	ret.vortex_velocities_norm = vortex_velocities_norm / d;
	ret.velocities_x = velocities_x / d;
	ret.velocities_y = velocities_y / d;
	ret.velocities_z = velocities_z / d;
	ret.velocities_norm = velocities_norm / d;
	ret.number = number  / d;
	ret.ikinetick = ikinetick / d;
	ret.ckinetick = ckinetick / d;
	ret.kinetick = kinetick / d;
	ret.pressure = pressure / d;
	ret.pressure_kin_mixture = pressure_kin_mixture / d;
	ret.phase = phase / d;
	ret.omega = omega / d;
	ret.j = j / d;
	ret.ij = ij / d;
	ret.cj = cj / d;
	ret.g1 = g1 / d;
	ret.g2 = g2 / d;
	ret.g2_av = g2_av / d;
	ret.g2_vv = g2_vv / d;
	ret.g2_closest = g2_closest / d;
	ret.k = k / d;
	ret.ikinetick_wo_phase = ikinetick_wo_phase / d;
	ret.ckinetick_wo_phase = ckinetick_wo_phase / d;
	ret.kinetick_wo_phase = kinetick_wo_phase / d;
	ret.pressure_wo_phase = pressure_wo_phase / d;
	
	return ret;
}

inline Bh3Evaluation::Averages & Bh3Evaluation::Averages::operator/= (double d)
{
	*this = *this / d;
	return *this;
}

inline Bh3Evaluation::Averages Bh3Evaluation::Averages::operator* (double d) const
{                                                                             
	Averages ret(number.size(), vortex_velocities_x.size());
	
	ret.time = time * d;
	ret.nm3 = nm3 * d;
	ret.nm2 = nm2 * d;
	ret.nm1 = nm1 * d;
	ret.n0 = n0 * d;
	ret.n1 = n1 * d;
	ret.n2 = n2 * d;
	ret.n3 = n3 * d;
	ret.mass_zeros = mass_zeros * d;
	ret.num_vortices = num_vortices * d;
	ret.pair_distance_all = pair_distance_all * d;
	ret.pair_distance_nonzero = pair_distance_nonzero * d;
	ret.max_pair_distance = max_pair_distance * d;
	ret.kinetick_int = kinetick_int * d;
	ret.pressure_int = pressure_int * d;
	ret.interaction_int = interaction_int * d;
	ret.particle_count = particle_count * d;
	ret.ckinetic_int = ckinetic_int * d;
	ret.ikinetic_int = ikinetic_int * d;
	ret.j_int = j_int * d;
	ret.cj_int = cj_int * d;
	ret.ij_int = ij_int * d;
	ret.Ekin = Ekin * d;
	ret.meanfield = meanfield * d;
	ret.av_vortex_velocity = av_vortex_velocity * d;
	ret.av_velocity = av_velocity * d;
	
	ret.vortex_velocities_x = vortex_velocities_x * d;
	ret.vortex_velocities_y = vortex_velocities_y * d;
	ret.vortex_velocities_norm = vortex_velocities_norm * d;
	ret.velocities_x = velocities_x * d;
	ret.velocities_y = velocities_y * d;
	ret.velocities_z = velocities_z * d;
	ret.velocities_norm = velocities_norm * d;
	ret.number = number  * d;
	ret.ikinetick = ikinetick * d;
	ret.ckinetick = ckinetick * d;
	ret.kinetick = kinetick * d;
	ret.pressure = pressure * d;
	ret.pressure_kin_mixture = pressure_kin_mixture * d;
	ret.phase = phase * d;
	ret.omega = omega * d;
	ret.j = j * d;
	ret.ij = ij * d;
	ret.cj = cj * d;
	ret.g1 = g1 * d;
	ret.g2 = g2 * d;
	ret.g2_av = g2_av * d;
	ret.g2_vv = g2_vv * d;
	ret.g2_closest = g2_closest * d;
	ret.k = k * d;
	ret.ikinetick_wo_phase = ikinetick_wo_phase * d;
	ret.ckinetick_wo_phase = ckinetick_wo_phase * d;
	ret.kinetick_wo_phase = kinetick_wo_phase * d;
	ret.pressure_wo_phase = pressure_wo_phase * d;
	
	return ret;
}

inline Bh3Evaluation::Averages & Bh3Evaluation::Averages::operator*= (double d)
{
	*this = *this * d;
	return *this;
}

inline Bh3Evaluation::Averages sqrt (const Bh3Evaluation::Averages &a)
{                                                                             
	Bh3Evaluation::Averages ret(a.number.size(), a.vortex_velocities_x.size());
	
	ret.time = sqrt(a.time);
	ret.nm3 = sqrt(a.nm3);
	ret.nm2 = sqrt(a.nm2);
	ret.nm1 = sqrt(a.nm1);
	ret.n0 = sqrt(a.n0);
	ret.n1 = sqrt(a.n1);
	ret.n2 = sqrt(a.n2);
	ret.n3 = sqrt(a.n3);
	ret.mass_zeros = sqrt(a.mass_zeros);
	ret.num_vortices = sqrt(a.num_vortices);
	ret.pair_distance_all = sqrt(a.pair_distance_all);
	ret.pair_distance_nonzero = sqrt(a.pair_distance_nonzero);
	ret.max_pair_distance = sqrt(a.max_pair_distance);
	ret.kinetick_int = sqrt(a.kinetick_int);
	ret.pressure_int = sqrt(a.pressure_int);
	ret.interaction_int = sqrt(a.interaction_int);
	ret.particle_count = sqrt(a.particle_count);
	ret.ckinetic_int = sqrt(a.ckinetic_int);
	ret.ikinetic_int = sqrt(a.ikinetic_int);
	ret.j_int = sqrt(a.j_int);
	ret.cj_int = sqrt(a.cj_int);
	ret.ij_int = sqrt(a.ij_int);
	ret.Ekin = sqrt(a.Ekin);
	ret.meanfield = sqrt(a.meanfield);
	ret.av_vortex_velocity = sqrt(a.av_vortex_velocity);
	ret.av_velocity = sqrt(a.av_velocity);
	
	ret.vortex_velocities_x = sqrt(a.vortex_velocities_x);
	ret.vortex_velocities_y = sqrt(a.vortex_velocities_y);
	ret.vortex_velocities_norm = sqrt(a.vortex_velocities_norm);
	ret.velocities_x = sqrt(a.velocities_x);
	ret.velocities_y = sqrt(a.velocities_y);
	ret.velocities_z = sqrt(a.velocities_z);
	ret.velocities_norm = sqrt(a.velocities_norm);
	ret.number = sqrt(a.number);
	ret.ikinetick = sqrt(a.ikinetick);
	ret.ckinetick = sqrt(a.ckinetick);
	ret.kinetick = sqrt(a.kinetick);
	ret.pressure = sqrt(a.pressure);
	ret.pressure_kin_mixture = sqrt(a.pressure_kin_mixture);
	ret.phase = sqrt(a.phase);
	ret.omega = sqrt(a.omega);
	ret.j = sqrt(a.j);
	ret.ij = sqrt(a.ij);
	ret.cj = sqrt(a.cj);
	ret.g1 = sqrt(a.g1);
	ret.g2 = sqrt(a.g2);
	ret.g2_av = sqrt(a.g2_av);
	ret.g2_vv = sqrt(a.g2_vv);
	ret.g2_closest = sqrt(a.g2_closest);
	ret.k = sqrt(a.k);
	ret.ikinetick_wo_phase = sqrt(a.ikinetick_wo_phase);
	ret.ckinetick_wo_phase = sqrt(a.ckinetick_wo_phase);
	ret.kinetick_wo_phase = sqrt(a.kinetick_wo_phase);
	ret.pressure_wo_phase = sqrt(a.pressure_wo_phase);
	
	return ret;
}

#endif 