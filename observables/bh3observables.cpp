#include "bh3observables.h"

#include "complexgrid.h" 
#include "realgrid.h"
#include "averageclass.h"

#include <vector>
#include <list>
#include <stack>
#include <math.h>
#include <time.h>
#include <fstream>
#include <omp.h>
#include <bench_time.h>

// little helpers
int mypow(int x, int y)
{
	int z=1;
	
	for(int i=0; i<y; i++)
		z=x*z;
	
	return z;
}

// Evaluation class implementation
Bh3Evaluation::Bh3Evaluation(const PathOptions &opt, Space s) : ares(4*opt.grid[1], opt.delta_t.size())
{
	initialize(opt);
	space = s;
}

Bh3Evaluation::Bh3Evaluation(const PathOptions &opt, const vector<ComplexGrid> &d, Space s) : 
					ares(4*opt.grid[1], opt.delta_t.size())
{
	initialize(opt);
	space = s;
	setData(d, s);
}

Bh3Evaluation::~Bh3Evaluation()
{
	if(phase)
		delete phase;
	if(zeros)
		delete zeros;
}

void Bh3Evaluation::initialize(const PathOptions &opt)
{
	options = opt;
	
	init_k_space();
	
	phase = NULL;
	zeros = NULL;
	data.resize(opt.delta_t.size() + 1);
}

void Bh3Evaluation::init_k_space()
{
	kspace.resize(3);
	kfspace.resize(3);
	kbspace.resize(3);
	for(int d = 0; d < 3; d++)
	{
		// set k-space
		kspace[d].resize(options.grid[d+1]);
		for (int i=0; i<options.grid[d+1]/2; i++)
			kspace[d][i] = options.klength[d]*sin( M_PI*((double)i)/((double)options.grid[d+1]) );

		for (int i=options.grid[d]/2; i<options.grid[d+1]; i++)
			kspace[d][i] = options.klength[d]*sin( M_PI*((double)(-options.grid[d+1]+i))/((double)options.grid[d+1]) );

		//Set k-forward
		kfspace[d].resize(options.grid[d+1]);
		for (int i=0; i<options.grid[d+1]; i++)
			kfspace[d][i] = - complex<double>(0,1) * (polar( 1.0, 2*M_PI*((double)i)/((double)options.grid[d+1]) ) - 1.0);
		//Set k-backward
		kbspace[d].resize(options.grid[d+1]);
		for (int i=0; i<options.grid[d+1]; i++)
			kbspace[d][i] = complex<double>(0,1) * (polar( 1.0, - 2*M_PI*((double)i)/((double)options.grid[d+1]) ) - 1.0);
	}
}

void Bh3Evaluation::setData(const vector<ComplexGrid> &d, Space s)
{
    if((d[0].width() == options.grid[1]) && (d[0].height() == options.grid[2]) && (d[0].depth() == options.grid[3]))
	{
		data = d;
		space = s;
    }
	else
		cout << "Warning: Gridsize of given grid and PathOptions do not fit! Ignoring data!" << endl;
}

void Bh3Evaluation::assert_space(Space s)
{
	if(space != s)
	{
		for(int i = 0; i < data.size(); i++)
			ComplexGrid::fft(data[i], data[i], s == KSpace);
		
		space = s;
	}
}

/*void averagemeanfield(ComplexGrid &meanfield, const ComplexGrid &data)
{
	for (int i=0; i<data.width(); i++)
		for (int j=0; j<data.height(); j++)
		{
			meanfield.at(i,j) = gsl_complex_add(meanfield.at(i,j), data.at(i,j));
		}
}


void calc_omega(Grid &omega, const ComplexGrid &data, const ComplexGrid &olddata)
{
	for (int i=0; i<data.width(); i++)
		for (int j=0; j<data.height(); j++)
		{
			omega.at(i,j) = gsl_complex_abs( gsl_complex_div( gsl_complex_sub(data.at(i,j), olddata.at(i,j)),
									  data.at(i,j) ) );
		}

}*/

void Bh3Evaluation::calc_fields()
{
	if(data.size() > 0)
	{
		assert_space(RSpace);
		if(phase == NULL)
			phase = new RealGrid(1,options.grid[1],options.grid[2],options.grid[3]);
		if(zeros == NULL)
			zeros = new RealGrid(1,options.grid[1],options.grid[2],options.grid[3]);
		
		calc_fields(data[0], phase, zeros);
	}
	else
		cout << "Error: calc_fields: No data set for evaluation!" << endl;
}

void Bh3Evaluation::calc_fields(const ComplexGrid &data, RealGrid *phase, RealGrid *zeros)
{
	assert_space(RSpace);
	double zero_threshold = options.N * 0.05 / data.width() / data.height() / data.depth();
	for(int x = 0; x < data.width(); x++)
	{
		for(int y = 0; y < data.height(); y++)
		{
			for(int z = 0; z < data.depth(); z++)
			{
				phase->at(0,x,y,z) = arg(data(0,x,y,z));
				if(abs2(data(0,x,y,z)) < zero_threshold)
					zeros->at(0,x,y,z) = 0.0;
				else
					zeros->at(0,x,y,z) = 1.0;
			}
		}
	}
}

Bh3Evaluation::Direction Bh3Evaluation::next_direction(Direction d) const
{
	switch(d)
	{
		case N:
			return NO;
		case NO:
			return O;
		case O:
			return SO;
		case SO:
			return S;
		case S:
			return SW;
		case SW:
			return W;
		case W:
			return NW;
		case NW:
			return N;
		default:
			cout << "Error: Internal bug!" << endl;
			return N;
	}
}

Bh3Evaluation::Direction Bh3Evaluation::prev_direction(Direction d) const
{
	switch(d)
	{
		case N:
			return NW;
		case NO:
			return N;
		case O:
			return NO;
		case SO:
			return O;
		case S:
			return SO;
		case SW:
			return S;
		case W:
			return SW;
		case NW:
			return W;
		default:
			cout << "Error: Internal bug!" << endl;
			return N;
	}
}

Vector<int32_t> Bh3Evaluation::dir_vector(Direction d) const
{
	switch(d)
	{
		case N:
			return Vector<int32_t>(0,1,0,options.grid[1],options.grid[2],options.grid[3]);
		case NO:
			return Vector<int32_t>(1,1,0,options.grid[1],options.grid[2],options.grid[3]);
		case O:
			return Vector<int32_t>(1,0,0,options.grid[1],options.grid[2],options.grid[3]);
		case SO:
			return Vector<int32_t>(1,-1,0,options.grid[1],options.grid[2],options.grid[3]);
		case S:
			return Vector<int32_t>(0,-1,0,options.grid[1],options.grid[2],options.grid[3]);
		case SW:
			return Vector<int32_t>(-1,-1,0,options.grid[1],options.grid[2],options.grid[3]);
		case W:
			return Vector<int32_t>(-1,0,0,options.grid[1],options.grid[2],options.grid[3]);
		case NW:
			return Vector<int32_t>(-1,1,0,options.grid[1],options.grid[2],options.grid[3]);
		default:
			cout << "Error: Internal bug!" << endl;
			return Vector<int32_t>(0,0,0,options.grid[1],options.grid[2],options.grid[3]);
	}
}

inline int Bh3Evaluation::get_phase_jump(const Coordinate<int32_t> &c, const Vector<int32_t> &v, const RealGrid *phase) const
{
	if(phase->at(0,c + v) + M_PI < phase->at(0,c))	// Phase ueberschreitet 0/2pi von unten
		return 1;
	else if(phase->at(0,c) + M_PI < phase->at(0,c + v))	// Phase ueberschreitet 0/2pi von oben
		return -1;
	else
		return 0;
}

int Bh3Evaluation::get_quantization(const Coordinate<int32_t> &start, const RealGrid *phase, const RealGrid *zeros) const
{
	Coordinate<int32_t> x = start;
	Vector<int32_t> down = phase->make_vector(0,-1,0);
	do 			// Bis zu einer Randstelle im Sueden gehen
	{
		x += down;
	} while ((x != start) && (zeros->at(0,x) == 0.0));
	if(x==start)
		return 0;
	
	// Quantisierung bestimmen (einmal die Phase um die Nullstelle herum analysieren)
	Coordinate<int32_t> pos = x;
	Direction olddir = S;
	int q = 0;
	do
	{
		Direction d = olddir - 2;
		Direction backwards = olddir + 4;
		while (zeros->at(0,pos + dir_vector(d)) == 0.0)			// Richtungen von links bis rechts durchprobieren
		{													// um die Richtung zu finden, in der keine Nullstelle ist
															// d.h. insgesamt im mathematisch positiven Sinne um den Vortex
															// herumgehen
			if(d == backwards)
				return 0;				// falls er hier ankommt (selbst zurueckgehen ist gescheitert),
										// ist er eingesperrt
			d = d + 2;
		}
		Vector<int32_t> v = dir_vector(d);
		q += get_phase_jump(pos, v, phase);
		pos = pos + v;
		olddir = d;
	} while(pos != x);
	
	return q;
}

// diese Funktion wird benutzt um die Nullstellen zu suchen. Wird eine gefunden, werden rekursiv alle Nullstellen desselben Vortex gesucht.
bool Bh3Evaluation::get_vortex(const Coordinate<int32_t> &c, const RealGrid *phase, const RealGrid *zeros, double &mass_zeros, vector< vector< vector<bool> > > &checked, VortexData &vortex) const
{
	if(!checked[c.x()][c.y()][c.z()])
	{
		checked[c.x()][c.y()][c.z()] = true;
		if ( zeros->at(0,c) == 0.0 )			// auf Nullstelle ueberpruefen
		{
			stack<fv_data> vortex_stack;
			vortex_stack.push(fv_data(c,N));
			fv_data *vs = & vortex_stack.top();
			++mass_zeros;								// bei einer Nullstelle Daten des Vortex
			vortex.points.clear();
			vortex.num_points = 1;
			vortex.points.push_back(vs->x);
			vortex.x = vs->x;		// ergaenzen (Anzahl der Nullstellen und Schwerpunkt)
			
			while(!vortex_stack.empty())
			{
				vs = & vortex_stack.top();
				if(vs->finished)
				{
					vortex_stack.pop();
				}
				else
				{
					fv_data fv(vs->x + dir_vector(vs->d), N);
					if(!checked[fv.x.x()][fv.x.y()][fv.x.z()])
					{
						checked[fv.x.x()][fv.x.y()][fv.x.z()] = true;
						if ( zeros->at(0,fv.x) == 0.0 )			// auf Nullstelle ueberpruefen
						{
							++mass_zeros;								// bei einer Nullstelle Daten des Vortex
							vortex.points.push_back(fv.x);
							vortex.num_points++;
							vortex.x += (((Coordinate<double>)fv.x) - vortex.x) / vortex.num_points;	// ergaenzen (Anzahl der Nullstellen und Schwerpunkt)
							vortex_stack.push(fv);
						}
					}
					++vs->d;
					if(vs->d == vs->startd)
						vs->finished = true;
				}
			}
			
			// get quantization
			vortex.n = get_quantization(vortex.points.front(), phase, zeros);	// und links um den Vortex herum
			
			return true;
		}
		else 
		{
			return false;
		}
	}
	else
		return false;
}

/*void search_neighbours(const VortexData &v, list<VortexData> &vlist, list<VortexData> &point_list)
{
	list<VortexData>::iterator it = vlist.begin();
	while(it != vlist.end())
	{
		if((v.n == it->n) && ((it->x - v.x).norm() < 1.5))
		{
			VortexData nv = *it;
			point_list.push_back(nv);
			vlist.erase(it);
			search_neighbours(nv, vlist, point_list);
			it = vlist.begin();
		}
		else
			it++;
	}
}

VortexData combine_neighbours(const list<VortexData> &point_list)
{
	VortexData v;
	v.mass = point_list.size();
	v.n = point_list.front().n;
	v.x = point_list.front().x;
	for(list<VortexData>::const_iterator it = point_list.begin(); it != point_list.end(); it++)
	{
		v.x += (it->x - point_list.front().x) / point_list.size();
	}
	v.pair_distance = 0;
	return v;
}

void vortexCount2(const Grid &phase, list<VortexData> &vortexlist)
{
	int d = 10;
	
	vector<int> n(d, 0);
	list<VortexData> vlist;
	for(int x = 0; x < phase.width(); x++)
	{
		for(int y = 0; y < phase.height(); y++)
		{
			for(int i = 0; i < d; i++)
			{
				n[i] = 0;
				for(int k = 0; k < 2*i + 1; k++)
					n[i] += evaluate_phase_diff(phase,
												(x - i + k + 1 + phase.width()) % phase.width(),
												(y - i + phase.height()) % phase.height(),
												(x - i + k + phase.width()) % phase.width(),
												(y - i + phase.height()) % phase.height());
				for(int k = 0; k < 2*i + 1; k++)
					n[i] += evaluate_phase_diff(phase,
												(x + i + 1 + phase.width()) % phase.width(),
												(y - i + k + 1 + phase.height()) % phase.height(),
												(x + i + 1 + phase.width()) % phase.width(),
												(y - i + k + phase.height()) % phase.height());
				for(int k = 0; k < 2*i + 1; k++)
					n[i] += evaluate_phase_diff(phase,
												(x + i - k + phase.width()) % phase.width(),
												(y + i + 1 + phase.height()) % phase.height(),
												(x + i - k + 1 + phase.width()) % phase.width(),
												(y + i + 1 + phase.height()) % phase.height());
				for(int k = 0; k < 2*i + 1; k++)
					n[i] += evaluate_phase_diff(phase,
												(x - i + phase.width()) % phase.width(),
												(y + i - k + phase.height()) % phase.height(),
												(x - i + phase.width()) % phase.width(),
												(y + i - k + 1 + phase.height()) % phase.height());
			}
			
			vector<int> equal_count(d, 0);
			for(int i = 0; i < d; i++)
			{
				for(int j = i + 1; j < d; j++)
				{
					if(n[i] == n[j])
					{
						equal_count[i]++;
						equal_count[j]++;
					}
				}
			}
			
			int k;
			for(k = 0; (k < d) && (equal_count[k] < 0.5*d); k++) {}
			
			if((k < d) && (n[k] != 0))
			{
				VortexData v;
				v.n = n[k];
				v.x = phase.make_coord(x + 0.5, y + 0.5);
				v.mass = 1;
				
				vlist.push_back(v);
			}
		}
	}
	
	// mehrfach gezaehlte Vortices zu einem zusammenfassen
	while(vlist.size() != 0)
	{
		list<VortexData> point_list;
		VortexData v = vlist.front();
		point_list.push_back(v);
		vlist.pop_front();
		search_neighbours(v, vlist, point_list);
		v = combine_neighbours(point_list);
		vortexlist.push_back(v);
	}
}*/

void Bh3Evaluation::find_vortices(const RealGrid *phase, const RealGrid *zeros, list<VortexData> &vlist, double &mass_zeros) const
{
	// Nullstellen zaehlen
	vector< vector< vector<bool > > > checked(phase->width(), vector< vector<bool> >(phase->height(), vector<bool>(phase->depth(),false)));	// Welche felder schon ueberprueft wurden
	VortexData vortex;								// Charakteristika eines gefundenen Vortex
	vortex.n = 0;
	mass_zeros = 0;
	for (int z = 0; z < phase->depth(); z++)
	{
		for (int x = 0; x < phase->width(); x++)
		{
			for (int y = 0; y < phase->height(); y++)
			{
				Coordinate<int32_t> c = phase->make_coord(x,y,z);
				Vector<int32_t> down = phase->make_vector(0,-1,0);
				Vector<int32_t> right = phase->make_vector(1,0,0);
				Vector<int32_t> up = phase->make_vector(0,1,0);
				Vector<int32_t> left = phase->make_vector(-1,0,0);
				
				if(zeros && (zeros->at(0,x,y,z) == 0.0))
					mass_zeros++;
				
				int phase_winding = get_phase_jump(c, down, phase) + get_phase_jump(c+down, right, phase) + get_phase_jump(c+down+right, up, phase) + get_phase_jump(c+right, left, phase);
				
				if(phase_winding != 0)
				{
					vortex.n = phase_winding;
					vortex.x = c + phase->make_vector(0.5, -0.5, 0);
					vortex.points.clear();
					vortex.points.push_back(c);
					vortex.num_points = 1;
					vlist.push_back(vortex);
				}
				/*if(get_vortex(phase->make_coord(x,y,z), phase, zeros, mass_zeros, checked, vortex)) // prueft auf Vortices
				{
					vlist.push_back(vortex);					// und er wird in der Liste zurueckgegeben
				}*/
			}
		}
	}
}

inline void Bh3Evaluation::inc_pd_histogram(vector<float> &histogram, double distance) const
{
	double max_distance = sqrt(options.grid[1]*options.grid[1] + options.grid[2]*options.grid[2] + options.grid[3]*options.grid[3]) / 2.0;
	int index = (histogram.size() - 1) * distance / max_distance;
	histogram[index] += 0.5;
}

void Bh3Evaluation::calc_vortex_velocities(vector<list<VortexData> > &vlist)
{
	ares.vortex_velocities_x.setZero();
    ares.vortex_velocities_y.setZero();
    ares.vortex_velocities_norm.setZero();
    ares.av_vortex_velocity.setZero();
    	
	for(list<VortexData>::iterator it = vlist[0].begin(); it != vlist[0].end(); it++)
	{
		if(it->n != 0)
		{
			it->velocity.resize(vlist.size()-1);
			for(int b = 1; b < vlist.size(); b++)
			{
				double distance = 65536.0;
				list<VortexData>::const_iterator old_it;
				for(list<VortexData>::const_iterator ot = vlist[b].begin(); ot != vlist[b].end(); ot++)
				{
					if(ot->n == it->n)
					{
						double d = (it->x - ot->x).norm();
						if(d < distance)
						{
							distance = d;
							old_it = ot;
						}
					}
				}
				if(distance != 65536.0)
				{
					Vector<double> dx = it->x - old_it->x;
					it->velocity[b-1] = dx / options.delta_t[b-1];
					int index = (ares.vortex_velocities_x.cols() * (dx.x() + data[0].width() / 2)) / data[0].width();
					ares.vortex_velocities_x(b-1,index) += 1.0;
					index = (ares.vortex_velocities_y.cols() * (dx.y() + data[0].height() / 2)) / data[0].height();
					ares.vortex_velocities_y(b-1,index) += 1.0;
					double dx_max = sqrt(data[0].width()*data[0].width() + data[0].height()*data[0].height()) / 2.0;
					index = ((ares.vortex_velocities_norm.cols()-1) * dx.norm()) / dx_max;
					ares.vortex_velocities_norm(b-1,index) += 1.0;
					ares.av_vortex_velocity(b-1) += dx.norm() / options.delta_t[b-1];
				}
				else
				{
					it->velocity[b-1] = data[0].make_vector(0,0,0);
				}
			}
		}
		else
		{
			it->velocity.clear();
		}
	}
}

void Bh3Evaluation::calc_vortex_distances()
{
	AverageClass<double> av_pair_distance;
	for (list<VortexData>::iterator it = pres.vlist.begin(); it != pres.vlist.end(); it++)
	{
		if(it->n != 0)
		{
			double shortest_distance = 65536.0;
			for(list<VortexData>::iterator oit = pres.vlist.begin(); oit != pres.vlist.end(); oit++)
			{
				if((it != oit) && (oit->n!=0))
				{
					double distance = (it->x - oit->x).norm();
					if(distance < shortest_distance)
						shortest_distance = distance;
					//inc_pd_histogram(ares.pd_histogram_all, distance);
				}
			}
			if(shortest_distance != 65536.0)
			{
				it->pair_distance = shortest_distance;
				//inc_pd_histogram(ares.pd_histogram_closest, shortest_distance);
				av_pair_distance.average(it->pair_distance);
				if(it->pair_distance > ares.max_pair_distance)
					ares.max_pair_distance = it->pair_distance;
			}
			else
				it->pair_distance = 0.0;
		}
		else
			it->pair_distance = 0.0;
	}
	
	// mittlerer Abstand der Vortices
	ares.pair_distance_all = av_pair_distance.av();
	if(av_pair_distance.av() != 0)
		ares.pair_distance_nonzero = av_pair_distance.av();
}

void Bh3Evaluation::calc_g2()
{
	ares.g2_av.setZero();
    ares.g2_vv.setZero();
    ares.g2.setZero();
    ares.g2_closest.setZero();
    double index_factor = (ares.g2_av.size()-1) * 2 / sqrt(options.grid[1]*options.grid[1] + options.grid[2]*options.grid[2] + options.grid[3]*options.grid[3]);
		
	for(list<VortexData>::const_iterator it1 = pres.vlist.begin(); it1 != pres.vlist.end(); it1++)
	{
		if(it1->n != 0)
		{
			ares.g2_closest(it1->pair_distance * index_factor) += 1.0 / 2.0 / M_PI / it1->pair_distance;
		
			list<VortexData>::const_iterator it2 = it1;
			it2++;
			for(; it2 != pres.vlist.end(); it2++)
			{
				if(it2->n != 0)
				{
					double r = (it1->x - it2->x).norm();
					int index = r * index_factor;
					
					if(it2->n * it1->n == -1)
					{
						ares.g2_av(index) += 1.0 / 2.0 / M_PI / r;
					}
					else if(it2->n * it1->n == 1)
					{
						ares.g2_vv(index) += 1.0 / 2.0 / M_PI / r;
					}
					ares.g2(index) += 1.0 / 2.0 / M_PI / r;
                }
			}
		}
	}
}

void Bh3Evaluation::evaluate_vortices()
{
	assert_space(RSpace);
	
	if(phase == NULL)
		calc_fields();
	/*ComplexGrid *temp = new ComplexGrid(8192, 8192, 1);
	Grid *temp_phase = new Grid(8192, 8192, 1);
	Coordinate<int32_t> t_origin = temp->make_coord(0,0,0);
	Coordinate<int32_t> d_origin = data[0].make_coord(0,0,0);
	
	for(int x = 0; x < data[0].width(); x++)
	{
		for(int y = 0; y < data[0].height(); y++)
		{
			Coordinate<int32_t> c = data[0].make_coord(x,y,0);
			Vector<int32_t> v = c - d_origin;
			temp->at(t_origin + v) = data[0].at(c);
		}
	}
	ComplexGrid::fft(*temp, *temp, false);
	for(int x = 0; x < temp->width(); x++)
	{
		for(int y = 0; y < temp->height(); y++)
		{
			temp_phase->at(x,y,0) = arg(temp->at(x,y,0));
		}
	}*/
	
	pres.vlist.clear();
	find_vortices(/*temp_*/phase, zeros, pres.vlist, ares.mass_zeros);
	
	if(options.delta_t.size() > 0)
	{
		vector<list<VortexData> > vlists(options.delta_t.size() + 1);
		vlists[0] = pres.vlist;
		for(int i = 0; i < options.delta_t.size(); i++)
		{
			RealGrid *old_phase = new RealGrid(1, data[0].width(), data[0].height(), data[0].depth());
			RealGrid *old_zeros = new RealGrid(1, data[0].width(), data[0].height(), data[0].depth());
			//Grid *old_zeros = NULL;
			double old_mass_zeros = 0.0;
			list<VortexData> old_vlist;
			calc_fields(data[i+1],old_phase,old_zeros);
			/*for(int x = 0; x < data[i].width(); x++)
			{
				for(int y = 0; y < data[i].height(); y++)
				{
					Coordinate<int32_t> c = data[i].make_coord(x,y,0);
					Vector<int32_t> v = c - d_origin;
					temp->at(t_origin + v) = data[i].at(c);
				}
			}
			ComplexGrid::fft(*temp, *temp, false);
			for(int x = 0; x < temp->width(); x++)
			{
				for(int y = 0; y < temp->height(); y++)
				{
					temp_phase->at(x,y,0) = arg(temp->at(x,y,0));
				}
			}*/
			find_vortices(/*temp_*/old_phase, old_zeros, old_vlist, old_mass_zeros);
			vlists[i+1] = old_vlist;
			delete old_phase;
			delete old_zeros;
		}
		
		// Geschwindigkeiten der Vortices aus vlist und old_vlist bestimmen:
		calc_vortex_velocities(vlists);
	}
	//delete temp;
	//delete temp_phase;
	
	// Zaehlt wie viele Vortices mit der entsprechenden Quantisierung es gibt
	ares.n0 = 0; ares.n1 = 0; ares.n2 = 0; ares.n3 = 0; ares.nm1 = 0; ares.nm2 = 0; ares.nm3 = 0;
	for(list<VortexData>::const_iterator it = pres.vlist.begin(); it != pres.vlist.end(); ++it)
	{
		switch(it->n)
		{
			case -3:
				++ares.nm3;
				break;
			case -2:
				++ares.nm2;
				break;
			case -1:
				++ares.nm1;
				break;
			case 0:
				++ares.n0;
				break;
			case 1:
				++ares.n1;
				break;
			case 2:
				++ares.n2;
				break;
			case 3:
				++ares.n3;
				break;
			default:
				break;
		}
	}
	ares.num_vortices = pres.vlist.size();
	
	// Abstaende zwischen Nachbarn bestimmen
	calc_vortex_distances();
	
	// Vortex-Paar-Korrelationen
	calc_g2();
}

double Bh3Evaluation::get_phase_diff(const Coordinate<int32_t> &c1, const Coordinate<int32_t> &c2) const
{
	double res = phase->at(0,c1) - phase->at(0,c2);
	
	while(res >= M_PI)
	{
		res -= 2*M_PI;
	}
	while(res < -M_PI)
	{
		res += 2*M_PI;
	}
	
	return res;
}

complex<double> Bh3Evaluation::calc_compressible_part_x(int x, int y, int z, complex<double> kx, complex<double> ky, complex<double> kz) const
{
	return (kx*kfspace[0][x] + ky*kfspace[1][y] + kz*kfspace[2][z]) * kbspace[0][x] /
			(kbspace[0][x]*kfspace[0][x] + kbspace[1][y]*kfspace[1][y] + kbspace[2][z]*kfspace[2][z]);
}

complex<double> Bh3Evaluation::calc_compressible_part_y(int x, int y, int z, complex<double> kx, complex<double> ky, complex<double> kz) const
{
	return (kx*kfspace[0][x] + ky*kfspace[1][y] + kz*kfspace[2][z]) * kbspace[1][y] /
			(kbspace[0][x]*kfspace[0][x] + kbspace[1][y]*kfspace[1][y] + kbspace[2][z]*kfspace[2][z]);
}

complex<double> Bh3Evaluation::calc_compressible_part_z(int x, int y, int z, complex<double> kx, complex<double> ky, complex<double> kz) const
{
	return (kx*kfspace[0][x] + ky*kfspace[1][y] + kz*kfspace[2][z]) * kbspace[2][z] /
			(kbspace[0][x]*kfspace[0][x] + kbspace[1][y]*kfspace[1][y] + kbspace[2][z]*kfspace[2][z]);
}

void Bh3Evaluation::calc_radial_averages()
{
	assert_space(RSpace);
	
	double kwidth2[3];
	for(int i = 0; i < 3; i++)
		kwidth2[i] = (options.grid[i+1] == 1) ? 0 : options.klength[i]*options.klength[i];
	double index_factor = (ares.number.size() - 1) / sqrt(kwidth2[0] + kwidth2[1] + kwidth2[2]);
	
	ArrayXd divisor(ares.number.size());
    divisor.setZero();
    
	// Auf Null setzen
    ares.number.setZero();
    ares.omega.setZero();
    ares.ikinetick.setZero();
    ares.ckinetick.setZero();
    ares.kinetick.setZero();
    ares.pressure.setZero();
    ares.pressure_kin_mixture.setZero();
    ares.ikinetick_wo_phase.setZero();
    ares.ckinetick_wo_phase.setZero();
    ares.kinetick_wo_phase.setZero();
    ares.pressure_wo_phase.setZero();
    ares.phase.setZero();
    ares.j.setZero();
    ares.cj.setZero();
    ares.ij.setZero();
    ares.velocities_x.setZero();
    ares.velocities_y.setZero();
    ares.velocities_z.setZero();
    ares.velocities_norm.setZero();
	
	ares.meanfield = 0.0;
	ares.av_velocity = 0.0;
	
	if(phase == NULL)
		calc_fields();

	
    

	// temporaere Variable
	vector<ComplexGrid> field(3, ComplexGrid(1,options.grid[1], options.grid[2], options.grid[3]));
	//Vector<int32_t> up = data->make_vector(0,1,0);
	Vector<int32_t> down = data[0].make_vector(0,-1,0);
	//Vector<int32_t> right = data->make_vector(1,0,0);
	Vector<int32_t> left = data[0].make_vector(-1,0,0);
	//Vector<int32_t> forward = data->make_vector(0,0,1);
	Vector<int32_t> back = data[0].make_vector(0,0,-1);
	
	ares.interaction_int = 0.0;
	
	// Geschwindigkeits-Statistiken
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				
				ares.interaction_int += abs2(data[0](0,c)) * abs2(data[0](0,c)) * options.U / 2.0;
				
				double velocity[3] = {	get_phase_diff(c,c+left),
				get_phase_diff(c,c+down),
				get_phase_diff(c,c+back)};
				double v = sqrt(velocity[0]*velocity[0] + velocity[1]*velocity[1] + velocity[2]*velocity[2]);
				int v_index = v/ M_PI * ares.velocities_norm.size();
				int v_x = (velocity[0] + M_PI)/ (2*M_PI) * ares.velocities_x.size();
				int v_y = (velocity[1] + M_PI)/ (2*M_PI) * ares.velocities_y.size();
				int v_z = (velocity[2] + M_PI)/ (2*M_PI) * ares.velocities_z.size();
				
				ares.velocities_x(v_x) += 1.0;
				ares.velocities_y(v_y) += 1.0;
				ares.velocities_z(v_z) += 1.0;
				ares.av_velocity += v;
				if(v_index < ares.velocities_norm.size())
				{
					ares.velocities_norm(v_index) += 1.0;
				}
				ares.meanfield += data[0](0,c);
			}
		}
	}
	ares.meanfield /= (double) data[0].width() * data[0].height() * data[0].depth();
	ares.av_velocity /= (double) data[0].width() * data[0].height() * data[0].depth();
	
	// Summen initialisieren
	ares.kinetick_int     = 0.0;
	ares.pressure_int    = 0.0;
	ares.particle_count   = 0.0;
	ares.ckinetic_int = 0.0;
	ares.ikinetic_int = 0.0;
	ares.j_int = 0.0;
	ares.cj_int = 0.0;
	ares.ij_int = 0.0;
	ares.Ekin = 0.0;

	// Druckfeld berechnen
	#pragma omp parallel for schedule(guided, 1)
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				
				double sqrt_n_back[3] = {abs(data[0](0,c+left)),
                                         abs(data[0](0,c+down)),
                                         abs(data[0](0,c+back))};
				double sqrt_n = abs(data[0](0,c));
				complex<double> exp_i_phi_back[3] = {data[0](0,c+left) / sqrt_n_back[0],
                                                     data[0](0,c+down) / sqrt_n_back[1],
                                                     data[0](0,c+back) / sqrt_n_back[2]};
				complex<double> exp_i_phi = data[0](0,c) / sqrt_n;
				
				field[0](0,c) = (sqrt_n - sqrt_n_back[0]) * (exp_i_phi_back[0] + exp_i_phi) / 2.0 / exp_i_phi;
				field[1](0,c) = (sqrt_n - sqrt_n_back[1]) * (exp_i_phi_back[1] + exp_i_phi) / 2.0 / exp_i_phi;
				field[2](0,c) = (sqrt_n - sqrt_n_back[2]) * (exp_i_phi_back[2] + exp_i_phi) / 2.0 / exp_i_phi;
			}
		}
	}
	ComplexGrid::fft(field[0],field[0]);
	ComplexGrid::fft(field[1],field[1]);
	ComplexGrid::fft(field[2],field[2]);

	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				double k = sqrt(kspace[0][x]*kspace[0][x] + kspace[1][y]*kspace[1][y] + kspace[2][z]*kspace[2][z]);
				int index = index_factor * k;
                
                ares.k(index) += k;
				divisor(index)++;
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				
				// pressure energy:
				double pressure = abs2(field[0](0,c)) + abs2(field[1](0,c)) + abs2(field[2](0,c));
				
				ares.pressure_wo_phase(index) += pressure;
				ares.pressure_int += pressure;
			}
		}
	}
	ComplexGrid::fft(field[0],field[0], false);
	ComplexGrid::fft(field[1],field[1], false);
	ComplexGrid::fft(field[2],field[2], false);
    
	#pragma omp parallel for schedule(guided,1)
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				
				complex<double> phase = data[0](0,c) / abs(data[0](0,c));
				
				field[0](0,c) *= phase;
				field[1](0,c) *= phase;
				field[2](0,c) *= phase;
			}
		}
	}

	ComplexGrid::fft(field[0],field[0]);
	ComplexGrid::fft(field[1],field[1]);
	ComplexGrid::fft(field[2],field[2]);
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				double k = sqrt(kspace[0][x]*kspace[0][x] + kspace[1][y]*kspace[1][y] + kspace[2][z]*kspace[2][z]);
				int index = index_factor * k;
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
                ares.pressure(index) += abs2(field[0](0,c)) + abs2(field[1](0,c)) + abs2(field[2](0,c));
			}
		}
	}
	// Druck fertig
    
	// Stromfeld berechnen
	#pragma omp parallel for schedule(guided, 1)
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				double sqrt_n_back[3] = {abs(data[0](0,c+left)),
                                         abs(data[0](0,c+down)),
                                         abs(data[0](0,c+back))};
				double sqrt_n = abs(data[0](0,c));
				complex<double> exp_i_phi_back[3] = {data[0](0,c+left) / sqrt_n_back[0],
                                                     data[0](0,c+down) / sqrt_n_back[1],
                                                     data[0](0,c+back) / sqrt_n_back[2]};
				complex<double> exp_i_phi = data[0](0,c) / sqrt_n;
				
				field[0](0,c) = abs2(data[0](0,c)) * (exp_i_phi - exp_i_phi_back[0]) / exp_i_phi;
				field[1](0,c) = abs2(data[0](0,c)) * (exp_i_phi - exp_i_phi_back[1]) / exp_i_phi;
				field[2](0,c) = abs2(data[0](0,c)) * (exp_i_phi - exp_i_phi_back[2]) / exp_i_phi;
			}
		}
	}
	ComplexGrid::fft(field[0],field[0]);
	ComplexGrid::fft(field[1],field[1]);
	ComplexGrid::fft(field[2],field[2]);
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				double k = sqrt(kspace[0][x]*kspace[0][x] + kspace[1][y]*kspace[1][y] + kspace[2][z]*kspace[2][z]);
				int index = index_factor * k;
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				complex<double> cj[3];
				if((x==0)&&(y==0)&&(z==0))
				{
					cj[0] = field[0](0,c);
					cj[1] = field[1](0,c);
					cj[2] = field[2](0,c);
				}
				else
				{
					cj[0] = calc_compressible_part_x(x,y,z,field[0].at(0,c),field[1].at(0,c), field[2].at(0,c));
					cj[1] = calc_compressible_part_y(x,y,z,field[0].at(0,c),field[1].at(0,c), field[2].at(0,c));
					cj[2] = calc_compressible_part_z(x,y,z,field[0].at(0,c),field[1].at(0,c), field[2].at(0,c));
				}
				complex<double> ij[3] = {field[0](0,c) - cj[0],
                                         field[1](0,c) - cj[1],
                                         field[2](0,c) - cj[2]};
				
				double j2 = abs2(field[0](0,c)) + abs2(field[1](0,c)) + abs2(field[2](0,c));
				double cj2 = abs2(cj[0]) + abs2(cj[1]) + abs2(cj[2]);
				double ij2 = abs2(ij[0]) + abs2(ij[1]) + abs2(ij[2]);
				
				ares.j(index) += j2;
				ares.cj(index) += cj2;
				ares.ij(index) += ij2;
				
				ares.j_int += j2;
				ares.cj_int += cj2;
				ares.ij_int += ij2;
			}
		}
	}
	// Strom fertig
	
	// Im Ortsraum Geschwindigkeitsfeld berechnen
	#pragma omp parallel for schedule(guided, 1)
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				
				double sqrt_n_back[3] = {abs(data[0](0,c+left)),
										 abs(data[0](0,c+down)),
										 abs(data[0](0,c+back))};
				double sqrt_n = abs(data[0](0,c));
				complex<double> exp_i_phi_back[3] = {data[0](0,c+left) / sqrt_n_back[0],
                                                     data[0](0,c+down) / sqrt_n_back[1],
                                                     data[0](0,c+back) / sqrt_n_back[2]};
				complex<double> exp_i_phi = data[0](0,c) / sqrt_n;
				
				field[0](0,c) = (sqrt_n_back[0] + sqrt_n) / 2.0 * (exp_i_phi - exp_i_phi_back[0]) / exp_i_phi;
				field[1](0,c) = (sqrt_n_back[1] + sqrt_n) / 2.0 * (exp_i_phi - exp_i_phi_back[1]) / exp_i_phi;
				field[2](0,c) = (sqrt_n_back[2] + sqrt_n) / 2.0 * (exp_i_phi - exp_i_phi_back[2]) / exp_i_phi;
			}
		}
	}
	
	ComplexGrid::fft(field[0],field[0]);
	ComplexGrid::fft(field[1],field[1]);
	ComplexGrid::fft(field[2],field[2]);
	
	assert_space(KSpace);
	
	// Im Impulsraum Werte aufaddieren
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				double k = sqrt(kspace[0][x]*kspace[0][x] + kspace[1][y]*kspace[1][y] + kspace[2][z]*kspace[2][z]);
				int index = index_factor * k;
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				
				// occupation number
				double number = abs2(data[0](0,c));
				ares.number(index) += number;
				ares.particle_count += number;
				ares.Ekin += number * k * k;
				
				if(data.size() > 1)
					ares.omega(index) += abs((data[0](0,c) - data[1](0,c)) / data[0](0,c));
				
				// Calculating the incompressible:
				// compressible and incompressible velocity:
				complex<double> cvelofield[3];
				if((x==0)&&(y==0)&&(z==0))
				{
					cvelofield[0] = field[0](0,0,0,0);
					cvelofield[1] = field[1](0,0,0,0);
					cvelofield[2] = field[2](0,0,0,0);
				}
				else
				{
					cvelofield[0] = calc_compressible_part_x(x,y,z,field[0](0,c),field[1](0,c), field[2](0,c));
					cvelofield[1] = calc_compressible_part_y(x,y,z,field[0](0,c),field[1](0,c), field[2](0,c));
					cvelofield[2] = calc_compressible_part_z(x,y,z,field[0](0,c),field[1](0,c), field[2](0,c));
				}
				
				double ckinetick = abs2(cvelofield[0]) + abs2(cvelofield[1]) + abs2(cvelofield[2]);
				double kinetick = abs2(field[0](0,c)) + abs2(field[1](0,c)) + abs2(field[2](0,c));
				double ikinetick = kinetick - ckinetick;
				
				ares.ikinetick_wo_phase(index) += ikinetick;
				ares.ckinetick_wo_phase(index) += ckinetick;
				ares.kinetick_wo_phase(index) += kinetick;
				
				ares.ikinetic_int += ikinetick;
				ares.ckinetic_int += ckinetick;
				ares.kinetick_int += kinetick;
				
				field[0](0,c) = cvelofield[0];
				field[1](0,c) = cvelofield[1];
				field[2](0,c) = cvelofield[2];
			}
		}
	}
	ComplexGrid::fft(field[0],field[0], false);
	ComplexGrid::fft(field[1],field[1], false);
	ComplexGrid::fft(field[2],field[2], false);
	assert_space(RSpace);
	
	#pragma omp parallel for schedule(guided,1)
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				
				complex<double> phase = data[0](0,c) / abs(data[0](0,c));
				
				field[0](0,c) *= phase;
				field[1](0,c) *= phase;
				field[2](0,c) *= phase;
			}
		}
	}
	ComplexGrid::fft(field[0],field[0]);
	ComplexGrid::fft(field[1],field[1]);
	ComplexGrid::fft(field[2],field[2]);
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				double k = sqrt(kspace[0][x]*kspace[0][x] + kspace[1][y]*kspace[1][y] + kspace[2][z]*kspace[2][z]);
				int index = index_factor * k;
				
				ares.ckinetick(index) += abs2(field[0](0,c)) + abs2(field[1](0,c)) + abs2(field[2](0,c));
			}
		}
	}
	
	#pragma omp parallel for schedule(guided, 1)
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				
				double sqrt_n_back[3] = {abs(data[0](0,c+left)),
                                         abs(data[0](0,c+down)),
                                         abs(data[0](0,c+back))};
				double sqrt_n = abs(data[0](0,c));
				complex<double> exp_i_phi_back[3] = {data[0](0,c+left) / sqrt_n_back[0],
                                                     data[0](0,c+down) / sqrt_n_back[1],
                                                     data[0](0,c+back) / sqrt_n_back[2]};
				complex<double> exp_i_phi = data[0](0,c) / sqrt_n;
				
				field[0](0,c) = (sqrt_n_back[0] + sqrt_n) / 2.0 * (exp_i_phi - exp_i_phi_back[0]);
				field[1](0,c) = (sqrt_n_back[1] + sqrt_n) / 2.0 * (exp_i_phi - exp_i_phi_back[1]);
				field[2](0,c) = (sqrt_n_back[2] + sqrt_n) / 2.0 * (exp_i_phi - exp_i_phi_back[2]);
			}
		}
	}
	ComplexGrid::fft(field[0],field[0]);
	ComplexGrid::fft(field[1],field[1]);
	ComplexGrid::fft(field[2],field[2]);
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				double k = sqrt(kspace[0][x]*kspace[0][x] + kspace[1][y]*kspace[1][y] + kspace[2][z]*kspace[2][z]);
				int index = index_factor * k;
				
				ares.kinetick(index) += abs2(field[0](0,c)) + abs2(field[1](0,c)) + abs2(field[2](0,c));
			}
		}
	}

    ares.ikinetick = ares.kinetick - ares.ckinetick;
	
	#pragma omp parallel for schedule(guided, 1)
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				
				field[0](0,c) = data[0](0,c) / abs(data[0](0,c));
			}
		}
	}
	ComplexGrid::fft(field[0],field[0]);
	for(int x = 0; x < data[0].width(); x++)
	{
		for (int y = 0; y < data[0].height(); y++)
		{
			for (int z = 0; z < data[0].depth(); z++)
			{
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				double k = sqrt(kspace[0][x]*kspace[0][x] + kspace[1][y]*kspace[1][y] + kspace[2][z]*kspace[2][z]);
				int index = index_factor * k;
				
				ares.phase(index) += abs2(field[0](0,c));
			}
		}
	}
	
	// Geschwindigkeiten fertig

	// Pressure-Kin-Mixture
	for(int d = 0; d < 3; d++)
	{
		for(int x = 0; x < data[0].width(); x++)
		{
			for (int y = 0; y < data[0].height(); y++)
			{
				for (int z = 0; z < data[0].depth(); z++)
				{
					Coordinate<int32_t> c = data[0].make_coord(x,y,z);
					
					double sqrt_n_back[3] = {abs(data[0](0,c+left)),
                                             abs(data[0](0,c+down)),
                                             abs(data[0](0,c+back))};
					double sqrt_n = abs(data[0](0,c));
					complex<double> exp_i_phi_back[3] = {data[0](0,c+left) / sqrt_n_back[0],
                                                         data[0](0,c+down) / sqrt_n_back[1],
                                                         data[0](0,c+back) / sqrt_n_back[2]};
					complex<double> exp_i_phi = data[0](0,c) / sqrt_n;
					
					field[0](0,c) = (sqrt_n_back[d] + sqrt_n) / 2.0 * (exp_i_phi - exp_i_phi_back[d]);
					field[1](0,c) = (sqrt_n - sqrt_n_back[d]) * (exp_i_phi_back[d] + exp_i_phi) / 2.0;
				}
			}
		}
		ComplexGrid::fft(field[0],field[0]);
		ComplexGrid::fft(field[1],field[1]);
		
		for(int x = 0; x < data[0].width(); x++)
		{
			for (int y = 0; y < data[0].height(); y++)
			{
				for (int z = 0; z < data[0].depth(); z++)
				{
					Coordinate<int32_t> c = data[0].make_coord(x,y,z);
					double k = sqrt(kspace[0][x]*kspace[0][x] + kspace[1][y]*kspace[1][y] + kspace[2][z]*kspace[2][z]);
					int index = index_factor * k;
					
					double pressure_kin_mixture = 2.0 * real(field[1](0,c)*conj(field[0](0,c)));
					ares.pressure_kin_mixture(index) += pressure_kin_mixture;
				}
			}
		}
	}
	// Pressure-Kin-Mixture fertig
	
	#pragma omp parallel for schedule(guided,1)
	for(int l = 0; l < ares.number.size(); l++)
	{
		if(divisor[l] == 0)
			divisor[l] = 1;
	}
        // durch Anzahl der Gridpunkte teilen
    ares.number /= divisor;
    ares.omega /= divisor;
    ares.ikinetick /= divisor;
    ares.ckinetick /= divisor;
    ares.kinetick /= divisor;
    ares.pressure /= divisor;
    ares.pressure_kin_mixture /= divisor;
    ares.ikinetick_wo_phase /= divisor;
    ares.ckinetick_wo_phase /= divisor;
    ares.kinetick_wo_phase /= divisor;
    ares.pressure_wo_phase /= divisor;
    ares.j /= divisor;
    ares.cj /= divisor;
    ares.ij /= divisor;
    ares.k /= divisor;
}

void Bh3Evaluation::calc_g1()
{
	assert_space(KSpace);
	
	ComplexGrid g1(1, data[0].width(), data[0].height(), data[0].depth());
	vector<AverageClass<complex<double> > > temp(ares.g1.size());
	Coordinate<double> origin = data[0].make_coord(0,0,0);
	
	double index_factor = ((double)temp.size()-1.0) * 2.0 / sqrt(data[0].width()*data[0].width() + data[0].height()*data[0].height() + data[0].depth()*data[0].depth());
	double norm_factor = 1.0 / (options.N / (data[0].height() * data[0].width() * data[0].depth()));
	
	for(int x = 0; x < data[0].width(); x++)
	{
		for(int y = 0; y < data[0].height(); y++)
		{
			for(int z = 0; z < data[0].depth(); z++)
			{
				Coordinate<int32_t> c = data[0].make_coord(x,y,z);
				
				g1(0,c) = abs2(data[0](0,c));
			}
		}
	}
	
	ComplexGrid::fft(g1, g1, false);
	
	for(int x = 0; x < data[0].width(); x++)
	{
		for(int y = 0; y < data[0].height(); y++)
		{
			for(int z = 0; z < data[0].depth(); z++)
			{
				Coordinate<double> c = data[0].make_coord(x,y,z);
				
				double r = (c - origin).norm();
				int index = r * index_factor;
				
				temp[index].average(g1(0,c) * norm_factor);
			}
		}
	}
	
	for(int i = 0; i < temp.size(); i++)
	{
		ares.g1(i) = temp[i].av();
	}
}

