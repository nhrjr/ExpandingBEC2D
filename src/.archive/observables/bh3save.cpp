#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <utility>
#include <vector>

#include "complexgrid.h"
#include "realgrid.h"
#include "bh3binaryfile.h"
#include "bh3save.h"

using namespace std;

inline double my_abs2(int x, int y, int z, const vector<ComplexGrid> &field)
{
	return abs2(field[0].at(x,y,z)) + abs2(field[1].at(x,y,z));
}

void save_energies(const string &dirname, int p, const AverageClass<Bh3Evaluation::Averages> *av, int num_snapshots, bool dev = false)
{
	stringstream fs;
	if(p < 0)
		fs << dirname << "/Bh3AveragedEnergyData.txt";
	else
		fs << dirname << "/Bh3EnergyDataPath" << p << ".txt";
	ofstream pf;
	pf.open(fs.str().c_str(), ios_base::out | ios_base::trunc);
	stringstream str;
	str << "# time particles meanfield interaction pressure kinetick ckinetick ikinetick Ekin j compr_j incompr_j" << endl;
	for(int b=0; b < num_snapshots; b++)
	{
        Bh3Evaluation::Averages mean = av[b]->av();
        if(!dev)
		{
			str 	<< mean.time << " "
					<< mean.particle_count << " "
					<< mean.meanfield.real() << " " << mean.meanfield.imag() << " "
					<< mean.interaction_int << " "
					<< mean.pressure_int << " "
					<< mean.kinetick_int << " "
					<< mean.ckinetic_int << " "
					<< mean.ikinetic_int << " "
					<< mean.Ekin << " "
					<< mean.j_int << " "
					<< mean.cj_int << " "
					<< mean.ij_int << endl << endl;
		}
		else
		{
            Bh3Evaluation::Averages std_dev = av[b]->sd();
            str 	<< mean.time << " " << std_dev.time << " "
					<< mean.particle_count << " " << std_dev.particle_count << " "
					<< mean.meanfield.real() << " " << mean.meanfield.imag() << " " 
					<< std_dev.meanfield.real() << " " << std_dev.meanfield.imag() << " "
					<< mean.interaction_int << " " << std_dev.interaction_int << " "
					<< mean.pressure_int << " " << std_dev.pressure_int << " "
					<< mean.kinetick_int << " " << std_dev.kinetick_int << " "
					<< mean.ckinetic_int << " " << std_dev.ckinetic_int << " "
					<< mean.ikinetic_int << " " << std_dev.ikinetic_int << " "
					<< mean.Ekin << " " << std_dev.Ekin << " "
					<< mean.j_int << " " << std_dev.j_int << " "
					<< mean.cj_int << " " << std_dev.cj_int << " "
					<< mean.ij_int << " " << std_dev.ij_int << endl;
		}
	}
	pf << str.str();
	pf.close();
}

void save_vortices(const string &dirname, int p, const AverageClass<Bh3Evaluation::Averages> *av, int num_snapshots, bool dev = false)
{
	stringstream fs;
	if(p < 0)
		fs << dirname << "/Bh3AveragedVortexData.txt";
	else
		fs << dirname << "/Bh3VortexDataPath" << p << ".txt";
	ofstream pf;
	pf.open(fs.str().c_str(), ios_base::out | ios_base::trunc);
	stringstream str;
	str << "# time mass_zeros num_vortices n=-3 n=-2 n=-1 n=0 n=1 n=2 n=3 pair_distance_all pair_distance_nonzero max_pairdistance av_velocity";

	for(int b=0; b < av->size(); b++)
	{
        Bh3Evaluation::Averages mean = av[b]->av();
        if(b==0)
        {
            for(int i = 0; i < mean.av_vortex_velocity.size(); i++)
                str << "  av_vortex_velocity" << i;
            str << endl;
        }
        
		if(!dev)
		{
			str << mean.time << " " 
				<< mean.mass_zeros << " " 
				<< mean.num_vortices << " " 
				<< mean.nm3 << " " 
				<< mean.nm2 << " "
				<< mean.nm1 << " " 
				<< mean.n0 << " " 
				<< mean.n1 << " " 
				<< mean.n2 << " " 
				<< mean.n3 << " " 
				<< mean.pair_distance_all << " " 
				<< mean.pair_distance_nonzero << " " 
				<< mean.max_pair_distance << " " 
				<< mean.av_velocity;
			for(int i = 0; i < mean.av_vortex_velocity.size(); i++)
				str << " " << mean.av_vortex_velocity(i);
			str << endl;
		}
		else
		{
            Bh3Evaluation::Averages std_dev = av[b]->sd();
			str << mean.time << " " << std_dev.time << " "
				<< mean.mass_zeros << " " << std_dev.mass_zeros << " "
				<< mean.num_vortices << " " << std_dev.num_vortices << " "
				<< mean.nm3 << " " << std_dev.nm3 << " "
				<< mean.nm2 << " " << std_dev.nm2 << " "
				<< mean.nm1 << " "  << std_dev.nm1 << " "
				<< mean.n0 << " "  << std_dev.n0 << " "
				<< mean.n1 << " "  << std_dev.n1 << " "
				<< mean.n2 << " "  << std_dev.n2 << " "
				<< mean.n3 << " "  << std_dev.n3 << " "
				<< mean.pair_distance_all << " "  << std_dev.pair_distance_all << " "
				<< mean.pair_distance_nonzero << " "  << std_dev.pair_distance_nonzero << " "
				<< mean.max_pair_distance << " "  << std_dev.max_pair_distance << " "
				<< mean.av_velocity << " "  << std_dev.av_velocity;
			for(int i = 0; i < mean.av_vortex_velocity.size(); i++)
				str << " " << mean.av_vortex_velocity(i) << " "  << std_dev.av_vortex_velocity(i);
			str << endl;
		}
	}
	pf << str.str();
	pf.close();
}

void save_radial_averages(const string &dirname, int p, const PathOptions &options, const AverageClass<Bh3Evaluation::Averages> *av, int num_snapshots, bool dev = false)
{
	stringstream fs;
	if(p < 0)
		fs << dirname << "/Bh3AveragedRadialData.txt";
	else
		fs << dirname << "/Bh3RadialDataPath" << p << ".txt";
	ofstream pf;
	pf.open(fs.str().c_str(), ios_base::out | ios_base::trunc);
	stringstream str;
	vector<int> divisor(av->at(0).k.size(), 0);
	vector<vector<double> > kspace(3);
	double kwidth2[3];
	for(int d = 0; d < 3; d++)
	{
		// set k-space
		kspace[d].resize(options.grid[d]);
		for (int i=0; i<options.grid[d]/2; i++)
			kspace[d][i] = options.klength[d]*sin( M_PI*((double)i)/((double)options.grid[d]) );
		
		for (int i=options.grid[d]/2; i<options.grid[d]; i++)
			kspace[d][i] = options.klength[d]*sin( M_PI*((double)(-options.grid[d]+i))/((double)options.grid[d]) );
		
		kwidth2[d] = (options.grid[d] == 1) ? 0 : options.klength[d]*options.klength[d];
	}
	double index_factor = (av->at(0).k.size() - 1) / sqrt(kwidth2[0] + kwidth2[1] + kwidth2[2]);
	for(int x = 0; x < options.grid[0]; x++)
	{
		for(int y = 0; y < options.grid[1]; y++)
		{
			for(int z = 0; z < options.grid[2]; z++)
			{
				double k = sqrt(kspace[0][x]*kspace[0][x] + kspace[1][y]*kspace[1][y] + kspace[2][z]*kspace[2][z]);
				int index = index_factor * k;
				divisor[index]++;
			}
		}
	}
	for(int b = 0; b < av->size(); b++)
	{
		str << "# time: " << av->at(b).time << endl;
		str << "# k ikinetick ckinetick kinetick number omega pressure pk_mixture e^iphi ikinetick_w/o_phase ckinetick_w/o_phase kinetick_w/o_phase pressure_w/o_phase Q P1 P2 j compr_j incompr_j x g1 v velocities_x velocities_y velocities_z v velocities_norm" << endl;
		double Q, P1, P2, dQ, dP1, dP2;
		Q = P1 = P2 = 0.0;
		double interaction = av->at(b).particle_count * options.U / 
							(options.grid[0]*options.grid[1]*options.grid[2]);
		for(int l=0; l < av->at(b).number.size(); l++)
		{
			if(b > 0)
			{
				double flux = divisor[l] * (av->at(b).number[l] - av->at(b-1).number[l]) / (av->at(b).time - av->at(b-1).time);
				Q -= flux;
				P1 -= pow(av->at(b).k[l], 2.0) * flux;
				P2 -= interaction * flux;
				if(std_dev != NULL)
				{
					double dflux = divisor[l] * (std_dev->at(b).number[l] + std_dev->at(b-1).number[l]) / (av->at(b).time - av->at(b-1).time);
					dQ += dflux;
					dP1 += pow(av->at(b).k[l], 2.0) * dflux;
					dP2 += interaction * dflux;
				}
			}
			if(std_dev == NULL)
			{
				str << av->at(b).k[l] << " "
					<< av->at(b).ikinetick[l] << " "
					<< av->at(b).ckinetick[l] << " "
					<< av->at(b).kinetick[l] << " "
					<< av->at(b).number[l] << " "
					<< av->at(b).omega[l] << " "
					<< av->at(b).pressure[l] << " "
					<< av->at(b).pressure_kin_mixture[l] << " "
					<< av->at(b).phase[l] << " "
					<< av->at(b).ikinetick_wo_phase[l] << " "
					<< av->at(b).ckinetick_wo_phase[l] << " "
					<< av->at(b).kinetick_wo_phase[l] << " "
					<< av->at(b).pressure_wo_phase[l] << " "
					<< Q << " " << P1 << " " << P2 << " "
					<< av->at(b).j[l] << " "
					<< av->at(b).cj[l] << " "
					<< av->at(b).ij[l] << " "
					<< (double) l / 8.0 << " "
					<< abs(av->at(b).g1[l]) << " "
					<< ((double) l - (double) av->at(b).number.size() / 2) * 2.0 * M_PI / (double) av->at(b).number.size() << " "
					<< av->at(b).velocities_x[l] << " "
					<< av->at(b).velocities_y[l] << " "
					<< av->at(b).velocities_z[l] << " "
					<< l * M_PI / (double) av->at(b).number.size() << " "
					<< av->at(b).velocities_norm[l] << endl;
			}
			else
			{
				str << av->at(b).k[l] << " " << std_dev->at(b).k[l] << " "
					<< av->at(b).ikinetick[l] << " " << std_dev->at(b).ikinetick[l] << " "
					<< av->at(b).ckinetick[l] << " " << std_dev->at(b).ckinetick[l] << " "
					<< av->at(b).kinetick[l] << " " << std_dev->at(b).kinetick[l] << " "
					<< av->at(b).number[l] << " " << std_dev->at(b).number[l] << " "
					<< av->at(b).omega[l] << " " << std_dev->at(b).omega[l] << " "
					<< av->at(b).pressure[l] << " " << std_dev->at(b).pressure[l] << " "
					<< av->at(b).pressure_kin_mixture[l] << " " << std_dev->at(b).pressure_kin_mixture[l] << " "
					<< av->at(b).phase[l] << " " << std_dev->at(b).phase[l] << " "
					<< av->at(b).ikinetick_wo_phase[l] << " " << std_dev->at(b).ikinetick_wo_phase[l] << " "
					<< av->at(b).ckinetick_wo_phase[l] << " " << std_dev->at(b).ckinetick_wo_phase[l] << " "
					<< av->at(b).kinetick_wo_phase[l] << " " << std_dev->at(b).kinetick_wo_phase[l] << " "
					<< av->at(b).pressure_wo_phase[l] << " " << std_dev->at(b).pressure_wo_phase[l] << " "
					<< Q << " " << dQ << " " << P1 << " " << dP1 << " " << P2 << " " << dP2 << " "
					<< av->at(b).j[l] << " " << std_dev->at(b).j[l] << " "
					<< av->at(b).cj[l] << " " << std_dev->at(b).cj[l] << " "
					<< av->at(b).ij[l] << " " << std_dev->at(b).ij[l] << " "
					<< (double) l / 8.0 << " " << 0 << " "
					<< abs(av->at(b).g1[l]) << " " << abs(std_dev->at(b).g1[l]) << " "
					<< ((double) l - (double) av->at(b).number.size() / 2) * 2.0 * M_PI / (double) av->at(b).number.size() << " " << 0 << " "
					<< av->at(b).velocities_x[l] << " " << std_dev->at(b).velocities_x[l] << " "
					<< av->at(b).velocities_y[l] << " " << std_dev->at(b).velocities_y[l] << " "
					<< av->at(b).velocities_z[l] << " " << std_dev->at(b).velocities_z[l] << " "
					<< l * M_PI / (double) av->at(b).number.size() << " "
					<< av->at(b).velocities_norm[l] << " " << std_dev->at(b).velocities_norm[l] << endl;
			}
		}
		
		str << endl << endl;
	}
	pf << str.str();
	pf.close();
}

void save_vortex_velocities(const string &dirname, int p, const PathOptions &options, const vector<Bh3Evaluation::Averages> *av, const vector<Bh3Evaluation::Averages> *std_dev = NULL)
{
	stringstream fs;
	if(p < 0)
		fs << dirname << "/Bh3AveragedVortexVelocities.txt";
	else
		fs << dirname << "/Bh3VortexVelocitiesPath" << p << ".txt";
	ofstream pf;
	pf.open(fs.str().c_str(), ios_base::out | ios_base::trunc);
	stringstream str;
	for(int b = 0; b < av->size(); b++)
	{
		str << "# time: " << av->at(b).time << endl;
		str << "#";
		vector<double> k_factor_x(options.delta_t.size());
		vector<double> k_factor_y(options.delta_t.size());
		vector<double> k_factor_norm(options.delta_t.size());
		for(int i = 0; i < av->at(b).vortex_velocities_x.size(); i++)
		{
			str << " v" << options.delta_t[i] << " v_x" << options.delta_t[i] << " v_y" << options.delta_t[i] << " v_norm" << options.delta_t[i];
			k_factor_x[i] = 1.0 / (double) av->at(b).vortex_velocities_x[i].size() * options.grid[0] / options.delta_t[i];
			k_factor_y[i] = 1.0 / (double) av->at(b).vortex_velocities_y[i].size() * options.grid[1] / options.delta_t[i];
			k_factor_norm[i] = 1.0 / (double) av->at(b).vortex_velocities_norm[i].size() * 
									sqrt(options.grid[0]*options.grid[0] + options.grid[1]*options.grid[1]) / 2.0 / 
									options.delta_t[i];
		}
		str << endl;
		for(int k = 0; k < av->at(b).vortex_velocities_x[0].size(); k++)
		{
			if(std_dev == NULL)
			{
				for(int i = 0; i < av->at(b).vortex_velocities_x.size(); i++)
				{
					str << ((k - ((double) av->at(b).vortex_velocities_x[i].size()) / 2.0) * k_factor_x[i]) << " "
						<< ((k - ((double) av->at(b).vortex_velocities_y[i].size()) / 2.0) * k_factor_y[i]) << " "
						<< (k * k_factor_norm[i]) << " "
						<< av->at(b).vortex_velocities_x[i][k] << " "
						<< av->at(b).vortex_velocities_y[i][k] << " "
						<< av->at(b).vortex_velocities_norm[i][k] << " ";
				}
				str << endl;
			}
			else
			{
				for(int i = 0; i < av->at(b).vortex_velocities_x.size(); i++)
				{
					str << ((k - ((double) av->at(b).vortex_velocities_x[i].size()) / 2.0) * k_factor_x[i]) << " "
						<< ((k - ((double) av->at(b).vortex_velocities_y[i].size()) / 2.0) * k_factor_y[i]) << " "
						<< (k * k_factor_norm[i]) << " "
						<< av->at(b).vortex_velocities_x[i][k] << " " << std_dev->at(b).vortex_velocities_x[i][k] << " "
						<< av->at(b).vortex_velocities_y[i][k] << " " << std_dev->at(b).vortex_velocities_y[i][k] << " "
						<< av->at(b).vortex_velocities_norm[i][k] << " " << std_dev->at(b).vortex_velocities_norm[i][k] << " ";
				}
				str << endl;
			}
		}
		str << endl << endl;
	}
	pf << str.str();
	pf.close();
}

void save_vortex_correlations(const string &dirname, int p, const PathOptions &options, const vector<Bh3Evaluation::Averages> *av, const vector<Bh3Evaluation::Averages> *std_dev = NULL)
{
	stringstream fs, fs2;
	if(p < 0)
	{
		fs << dirname << "/Bh3AveragedVortexCorrelations.txt";
		fs2 << dirname << "/Bh3AveragedVortexG3.txt";
	}
	else
	{
		fs << dirname << "/Bh3VortexCorrelationsPath" << p << ".txt";
		fs2 << dirname << "/Bh3VortexG3Path" << p << ".txt";
	}
	ofstream pf, pf2;
	pf.open(fs.str().c_str(), ios_base::out | ios_base::trunc);
	pf2.open(fs2.str().c_str(), ios_base::out | ios_base::trunc);
	stringstream str, str2;
	double area = options.grid[0] * options.grid[1] * options.grid[2];
	for(int b = 0; b < av->size(); b++)
	{
		double index_factor = (av->at(b).g2.size()-1) * 2 / sqrt(options.grid[0]*options.grid[0] + options.grid[1]*options.grid[1] + options.grid[2]*options.grid[2]);
		str << "# time: " << av->at(b).time << endl;
		str << "# r g2 g2_va g2_vv g2_closest n1 nm1" << endl;
		for(int i = 0; i < av->at(b).g2.size(); i++)
		{
			if(std_dev == NULL)
			{
				str << i / index_factor << " "
					<< av->at(b).g2[i] << " "
					<< av->at(b).g2_av[i] << " "
					<< av->at(b).g2_vv[i] << " "
					<< av->at(b).g2_closest[i] << " "
					<< (av->at(b).n1 / area) << " "
					<< (av->at(b).nm1 / area) << endl;
			}
			else
			{
				str << i / index_factor << " "
					<< av->at(b).g2[i] << " " << std_dev->at(b).g2[i] << " "
					<< av->at(b).g2_av[i] << " " << std_dev->at(b).g2_av[i] << " "
					<< av->at(b).g2_vv[i] << " " << std_dev->at(b).g2_vv[i] << " "
					<< av->at(b).g2_closest[i] << " " << std_dev->at(b).g2_closest[i] << " "
					<< (av->at(b).n1 / area) << " "
					<< (av->at(b).nm1 / area) << endl;
			}
		}
		
		str << endl << endl;
		
		double index_factor2 = (av->at(b).g3.size()-1) * 2 / sqrt(options.grid[0]*options.grid[0] + options.grid[1]*options.grid[1] + options.grid[2]*options.grid[2]);
		str2 << "# time: " << av->at(b).time << endl;
		str2 << "# r1 r2 g3 n1 nm1" << endl;
		for(int r1 = 0; r1 < av->at(b).g3.size(); r1++)
		{
			for(int r2 = 0; r2 < av->at(b).g3[r1].size(); r2++)
			{
				if(std_dev == NULL)
				{
					str2 << r1 / index_factor2 << " " 
						 << r2 / index_factor2 << " " 
						 << av->at(b).g3[r1][r2] << " "
						 << (av->at(b).n1 / area) << " "
						 << (av->at(b).nm1 / area) << endl;
				}
				else
				{
					str2 << r1 / index_factor2 << " "
						 << r2 / index_factor2 << " " 
						 << av->at(b).g3[r1][r2] << " " << std_dev->at(b).g3[r1][r2] << " "
						 << (av->at(b).n1 / area) << " "
						 << (av->at(b).nm1 / area) << endl;
				}
			}
		}
		
		str2 << endl << endl;
		
	}
	pf << str.str();
	pf.close();
	pf2 << str2.str();
	pf2.close();
}

void save_averages(const string & dirname, const PathOptions &opt, int p, const vector<Bh3Evaluation::Averages> &av)
{
	save_energies(dirname, p, &av);
	save_vortices(dirname, p, &av);
	save_radial_averages(dirname, p, opt, &av);
	
	if(opt.delta_t.size() > 0)
		save_vortex_velocities(dirname, p, opt, &av);
	
	save_vortex_correlations(dirname, p, opt, &av);
}

void save_averages(const string & dirname, const PathOptions &opt, int p, const vector<Bh3Evaluation::Averages> &av, const vector<Bh3Evaluation::Averages> &std_dev)
{
	save_energies(dirname, p, &av, &std_dev);
	save_vortices(dirname, p, &av, &std_dev);
	save_radial_averages(dirname, p, opt, &av, &std_dev);
	
	if(opt.delta_t.size() > 0)
		save_vortex_velocities(dirname, p, opt, &av, &std_dev);
	
	save_vortex_correlations(dirname, p, opt, &av, &std_dev);
}

void append_fields(const string & dirname, const PathOptions &opt, int p, const Grid &phase, const Grid &zeros, const ComplexGrid &data)
{
	stringstream fs;
	fs << dirname << "/Bh3PhaseDataPath" << p << ".txt";
	ofstream pf;
	pf.open(fs.str().c_str(), ios_base::out | ios_base::app);
	stringstream str;
	str << "# x y z phase zeros data^2" << endl;
	for(int x = 0; x < data.width(); x++)
	{
		for(int y = 0; y < data.height(); y++)
		{
			for(int z = 0; z < data.depth(); z++)
			{
				str << x << " " << y << " " << z << " "
				<< phase(x,y,z) << " "
				<< zeros(x,y,z) << " "
				<< abs2(data(x,y,z)) << endl;
			}
		}
	}
	str << endl << endl;
	pf << str.str();
	pf.close();
}

void append_vortices(const string & dirname, const PathOptions &opt, int p, const Bh3Evaluation::PathResults &pres, const Bh3Evaluation::Averages &av, bool save0)
{
	stringstream fs, fs2;
	fs << dirname << "/Bh3VortexDetailsPath" << p << ".txt";
	fs2 << dirname << "/Bh3VorticesPath" << p << ".txt";
	ofstream pf;
	pf.open(fs.str().c_str(), ios_base::out | ios_base::app);
	stringstream str;
	ofstream pf2;
	pf2.open(fs2.str().c_str(), ios_base::out | ios_base::app);
	stringstream str2;
	str << "# time: " << av.time << endl;
	str << "# x y z n vortex_index" << endl;
	str2 << "# time: " << av.time << endl;
	str2 << "# x y z n pair_distance mass" << endl;
	int index = 0;
	for(list<Bh3Evaluation::VortexData>::const_iterator it = pres.vlist.begin(); it != pres.vlist.end(); ++it)
	{
		if(save0 || (it->n != 0))
		{
			for(list<Coordinate<int32_t> >::const_iterator pit = it->points.begin(); pit != it->points.end(); ++pit)
			{
				str << *pit << " " << it->n << " " << index << endl;
			}
			index++;
		}
		
		str2 << it->x << " " << it->n << " " << it->pair_distance << " " << it->points.size() << endl;
	}
	str << endl << endl;
	pf << str.str();
	pf.close();
	str2 << endl << endl;
	pf2 << str2.str();
	pf2.close();
}

bool initialize_evaluation_dir(string &dname, const string &dirsuffix, const list<string> &files)
{
	stringstream dirname;

	// Create directory for evaluations and return it in dirname
	string dirprefix = dname;
	if(dirprefix[dirprefix.length() - 1] == '/')
		dirprefix.erase(dirprefix.length() - 1);
	size_t slash = dirprefix.rfind('/');
	if(slash != string::npos)
	{
		dirprefix.erase(0, slash+1);
	}
	dirname << "results_" << dirprefix << dirsuffix;
	// Neuen Dateinamen garantieren
	DIR *dir = opendir(".");
	if(dir==NULL)
	{
		cout << "Couldn't open current directory!\n";
		return false;
	}
	dirent *entry;
	int run=0;
	char format[256];
	strcpy(format,dirname.str().c_str());
	strcat(format,"%d");
	while ( (entry=readdir(dir)) != NULL )
	{
		int temp;
		int result = sscanf(entry->d_name, format, &temp);
		if((result == 1) && (temp > run))
			run = temp;
	}
	closedir(dir);
	run++;
	dirname << run;
	
	if(mkdir(dirname.str().c_str(),0755) != 0)
	{
		cout << "Couldn't create directory " << dirname.str() << " !\n";
		return false;
	}
	dirname << "/";
	
	ofstream filelist;
	filelist.open((dirname.str() + "Path-Filelist.txt").c_str());
	int path = 0;
	for(list<string>::const_iterator it = files.begin(); it != files.end(); it++)
	{
		path++;
		filelist << "Path " << path << ": " << *it << endl;
	}
	filelist.close();
	dname = dirname.str();
	return true;
}

void write(ostream &o, const Bh3Evaluation::Averages &av)
{
	write(o, av.time);
	write(o, av.nm3);
	write(o, av.nm2);
	write(o, av.nm1);
	write(o, av.n0);
	write(o, av.n1);
	write(o, av.n2);
	write(o, av.n3);
	write(o, av.mass_zeros);
	write(o, av.num_vortices);
	write(o, av.meanfield);
	write(o, av.pair_distance_all);
	write(o, av.pair_distance_nonzero);
	write(o, av.max_pair_distance);
	write(o, av.kinetick_int);
	write(o, av.pressure_int);
	write(o, av.interaction_int);
	write(o, av.particle_count);
	write(o, av.ckinetic_int);
	write(o, av.ikinetic_int);
	write(o, av.j_int);
	write(o, av.cj_int);
	write(o, av.ij_int);
	write(o, av.Ekin);
	write(o, av.k);
	write(o, av.number);
	write(o, av.omega);
	write(o, av.ikinetick);
	write(o, av.ckinetick);
	write(o, av.kinetick);
	write(o, av.pressure);
	write(o, av.pressure_kin_mixture);
	write(o, av.phase);
	write(o, av.ikinetick_wo_phase);
	write(o, av.ckinetick_wo_phase);
	write(o, av.kinetick_wo_phase);
	write(o, av.pressure_wo_phase);
	write(o, av.j);
	write(o, av.cj);
	write(o, av.ij);
	write(o, av.vortex_velocities_x);
	write(o, av.vortex_velocities_y);
	write(o, av.vortex_velocities_norm);
	write(o, av.av_vortex_velocity);
	write(o, av.velocities_x);
	write(o, av.velocities_y);
	write(o, av.velocities_z);
	write(o, av.velocities_norm);
	write(o, av.av_velocity);
	write(o, av.g3);
	write(o, av.g2);
	write(o, av.g2_av);
	write(o, av.g2_vv);
	write(o, av.g2_closest);
	write(o, av.g1);
}

void read(istream &i, Bh3Evaluation::Averages &av)
{
	read(i, av.time);
	read(i, av.nm3);
	read(i, av.nm2);
	read(i, av.nm1);
	read(i, av.n0);
	read(i, av.n1);
	read(i, av.n2);
	read(i, av.n3);
	read(i, av.mass_zeros);
	read(i, av.num_vortices);
	read(i, av.meanfield);
	read(i, av.pair_distance_all);
	read(i, av.pair_distance_nonzero);
	read(i, av.max_pair_distance);
	read(i, av.kinetick_int);
	read(i, av.pressure_int);
	read(i, av.interaction_int);
	read(i, av.particle_count);
	read(i, av.ckinetic_int);
	read(i, av.ikinetic_int);
	read(i, av.j_int);
	read(i, av.cj_int);
	read(i, av.ij_int);
	read(i, av.Ekin);
	read(i, av.k);
	read(i, av.number);
	read(i, av.omega);
	read(i, av.ikinetick);
	read(i, av.ckinetick);
	read(i, av.kinetick);
	read(i, av.pressure);
	read(i, av.pressure_kin_mixture);
	read(i, av.phase);
	read(i, av.ikinetick_wo_phase);
	read(i, av.ckinetick_wo_phase);
	read(i, av.kinetick_wo_phase);
	read(i, av.pressure_wo_phase);
	read(i, av.j);
	read(i, av.cj);
	read(i, av.ij);
	read(i, av.vortex_velocities_x);
	read(i, av.vortex_velocities_y);
	read(i, av.vortex_velocities_norm);
	read(i, av.av_vortex_velocity);
	read(i, av.velocities_x);
	read(i, av.velocities_y);
	read(i, av.velocities_z);
	read(i, av.velocities_norm);
	read(i, av.av_velocity);
	read(i, av.g3);
	read(i, av.g2);
	read(i, av.g2_av);
	read(i, av.g2_vv);
	read(i, av.g2_closest);
	read(i, av.g1);
}

void write(ostream &o, const Bh3Evaluation::VortexData &v)
{
	write(o, v.n);
	write(o, v.x);
	write(o, v.velocity);
	write(o, v.num_points);
	write(o, v.points);
	write(o, v.pair_distance);
}

void read(istream &i, Bh3Evaluation::VortexData &v)
{
	read(i, v.n);
	read(i, v.x);
	read(i, v.velocity);
	read(i, v.num_points);
	read(i, v.points);
	read(i, v.pair_distance);
}

void write(ostream &o, const Bh3Evaluation::PathResults &pres)
{
	write(o, pres.vlist);
}

void read(istream &i, Bh3Evaluation::PathResults &pres)
{
	read(i, pres.vlist);
}

void save_binary_averages(const string & dirname, const PathOptions &options, int p, const vector<Bh3Evaluation::Averages> &av, const vector<Bh3Evaluation::PathResults> &pres)
{
	stringstream fs;
	fs << dirname << "/Bh3BinaryAveragesPath" << p << ".bin";
	ofstream pf;
	pf.open(fs.str().c_str(), ios_base::out | ios_base::trunc | ios_base::binary);
	pf.write("Bh3BinaryAverage", 16);
	write(pf, (int32_t) 2);		// version number
	write(pf, options);
	write(pf, (int32_t) av.size());
	for(int i = 0; i < av.size(); i++)
	{
		write(pf, av[i]);
		write(pf, pres[i]);
	}
	pf.close();
}

string start_binary_averages(const string & dirname, const PathOptions &options, int p, int32_t size)
{
	stringstream fs;
	fs << dirname << "/Bh3BinaryAveragesPath" << p << ".bin";
	ofstream pf;
	pf.open(fs.str().c_str(), ios_base::out | ios_base::trunc | ios_base::binary);
	pf.write("Bh3BinaryAverage", 16);
	write(pf, (int32_t) 2);		// version number
	write(pf, options);
	write(pf, size);
	pf.close();
	return fs.str();
}

void append_binary_averages(const string & filename, const Bh3Evaluation::Averages &av, const Bh3Evaluation::PathResults &pres)
{
	ofstream pf;
	pf.open(filename.c_str(), ios_base::out | ios_base::app | ios_base::binary);
	write(pf, av);
	write(pf, pres);
	pf.close();
}

void load_binary_averages(const string & filename, PathOptions &options, vector<Bh3Evaluation::Averages> &av, vector<Bh3Evaluation::PathResults> &pres)
{
	ifstream pf;
	pf.open(filename.c_str(), ios_base::in | ios_base::binary);
	char header[17];
	memset(header, 0, 17);
	pf.read(header, 16);
	if(strncmp(header, "Bh3BinaryAverage", 16) == 0)
	{
		int32_t version;
		read(pf, version);		// version number
		if(version == 2)
		{
			read(pf, options);
			int32_t size;
			read(pf, size);
			av.resize(size);
			pres.resize(size);
			for(int i = 0; i < size; i++)
			{
				read(pf, av[i]);
				read(pf, pres[i]);
			}
		}
	}
	else
	{
		cout << "File " << filename << " has a wrong format!" << endl;
	}
	pf.close();
}

bool is_binary_average_file(const string &filename)
{
	ifstream pf;
	pf.open(filename.c_str(), ios_base::in | ios_base::binary);
	char header[17];
	memset(header, 0, 17);
	pf.read(header, 16);
	pf.close();
	return (strncmp(header, "Bh3BinaryAverage", 16) == 0);
}

list<string> get_average_file_list(const list<string> args)
{
	list<string> files;
	
	// iterate the arguments and check them for valid Bh3BinaryAverage-files
	for(list<string>::const_iterator bin = args.begin(); bin != args.end(); ++bin)
	{
		struct stat buffer;
		lstat((*bin).c_str(), &buffer);
		if(S_ISREG(buffer.st_mode))
		{
			if(is_binary_average_file(*bin))
				files.push_back(*bin);
		}
		else
		{
			string bindir = *bin;
			if(bindir[bindir.length() - 1] != '/')
				bindir = bindir + "/";
			
			// Dateiname der Binaerdateien fuer Analyse bestimmen
			DIR *dir = opendir(bindir.c_str());
			if (dir == NULL)
			{
				cout << "Couldn't open directory with averages!" << endl;
				return list<string>();
			}
			dirent *entry;
			while( (entry = readdir(dir)) != NULL)
			{
				string filename = bindir + entry->d_name;
				if(is_binary_average_file(filename))
					files.push_back(filename);
			}
			closedir(dir);
		}
	}
	return files;
}
