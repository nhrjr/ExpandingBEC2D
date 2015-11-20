#include <sys/stat.h>
#include <sys/types.h>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <stdio.h>
#include <stdlib.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <averageclass.h>
#include <bh3cudapropagator.h>
#include <complexgrid.h>
#include <bh3defaultgrid.h>
#include <bh3binaryfile.h>
#include <gauss_random.h>
#include <bh3observables.h>
#include <omp.h>
#include <wrapped_cuda_functions.h>

#define FRAME_DURATION 10
#define WAIT_AFTER 200
#define NUM_SINGLE_PLOTS 999999
#define PNG_WIDTH 640 
#define PNG_HEIGHT 480
#define FIELD_WIDTH 6
#define PATHS 50
using namespace std;

void init_bh3(int argc, char** argv, PathOptions &opt, vector<double> &snapshot_times);
void plot(const string &dirname, const PathOptions& opt, vector<double> &snapshot_times, AverageClass<Bh3Evaluation::Averages> *av);

////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv) 
{
  
	
  
	PathOptions opt;
	vector<double> snapshot_times;
	
	string rm = "rm ";
	
	init_bh3(argc, argv, opt, snapshot_times);

	ComplexGrid::set_fft_planning_rigorosity(FFTW_MEASURE);

	init_random();

	stringstream dstr;
	dstr << "bh3cuda_TS"; /*<< opt.timestepsize
         << "_G" << opt.grid[0] << "_" << opt.grid[1] << "_" << opt.grid[2]
         << "_N" << opt.N
         << "_U" << opt.U
	     << "_KL" << opt.klength[0] << "_" << opt.klength[1] << "_" << opt.klength[2];*/
    
	string dirname = dstr.str();
    initialize_binary_dir(dirname, opt);
	mkdir((dirname + "/temp").c_str(), 0755);
	
	AverageClass<Bh3Evaluation::Averages> *av =  new AverageClass<Bh3Evaluation::Averages> [snapshot_times.size()];
	
	for(int k = 0; k < PATHS; k++)
	{
        if(init_cuda_device(0))
		{
            time_t timer = time(NULL);
			
			ComplexGrid *start;
			Bh3CPUPropagator *cp, *cp_imag;
			
                //start = create_Vortex_start_Grid2(opt,16,4,4,4);

            start = new ComplexGrid (opt.grid[0], opt.grid[1], opt.grid[2], opt.grid[3]);
            
            // opt.timestepsize = 0.015;
            // cp_imag = new Bh3CPUPropagator(opt, *start, Bh3CPUPropagator::imag);
            // cp_imag -> propagateToTime(opt.timestepsize*2000.);
            // cp_imag -> renoise();
        
            // *start = cp_imag -> getRGrid()[0];
        
            // delete cp_imag;
        

            // opt.timestepsize = 0.2;
            cp = new Bh3CPUPropagator(opt, *start, Bh3CPUPropagator::drivdiss);

            delete start;

            for(int j = 0; j < snapshot_times.size(); j++)
			{
                cout << "propagating...." << omp_get_thread_num() << endl;
                cp->propagateToTime(snapshot_times[j]);
							
                cout << "evaluating ..." << omp_get_thread_num() << endl;
                                                        
                Bh3Evaluation ev(opt);
								
                ev.setTime(snapshot_times[j]);
                ev.setData(cp->getRGrid(),Bh3Evaluation::RSpace);
                                                             
                ev.calc_radial_averages(); 
                
                av[j].average(ev.get_averageable_results());
			}

			delete cp;
			
			cout << "Path CUDA took " << time(NULL) - timer << "seconds" << endl;
		}		// if init_cuda_device
        cudaThreadExit();
                
		if(k%5 == 0)
		{
            plot(dirname, opt, snapshot_times, av);
            cout<< "run " << k << " done" << endl;
		}
	}
		 
    plot(dirname, opt, snapshot_times, av);
    delete [] av;
    
	stringstream rm_command;                     
	rm_command << "rm -r " << dirname << "/temp";
    system(rm_command.str().c_str());
	
	return 0;
}

void init_bh3(int argc, char** argv, PathOptions &opt, vector<double> &snapshot_times)
{
        // Parameter setzen
	opt.timestepsize = 0.2;
	opt.delta_t.resize(0);             
        //opt.delta_t[0]=0.2;
        //opt.delta_t[1]=0.4;

	opt.N = 3.2e9;  //normed for 512*512 N=64*50000
	
    opt.grid[0] = 1;
    opt.grid[1] = 1024;
	opt.grid[2] = 1024;
	opt.grid[3] = 1;
	opt.U = 3e-5;
	
    opt.g.resize(1);
    opt.g[0] = 1./4.;
    	
	opt.klength[0] = 2.0;
	opt.klength[1] = 2.0;
	opt.klength[2] = 2.0;
	
    snapshot_times.resize(5);
    snapshot_times[0] =5000;
    snapshot_times[1] =10000;
    snapshot_times[2] =15000;
    snapshot_times[3] =20000;
    snapshot_times[4] =25000;
          // snapshot_times[6] =17000;
          // snapshot_times[7] =20000;
          // snapshot_times[8] =30000;
          // snapshot_times[9] =50000;
          // snapshot_times[10]=70000;
          // snapshot_times[11]=100000;
          // snapshot_times[12]=150000;
          // snapshot_times[13]=180000;
          // snapshot_times[14]=220000;
          // snapshot_times[15]=250000;
          // snapshot_times[16]=300000;
          // snapshot_times[17]=450000;
          // snapshot_times[18]=500000;
          // snapshot_times[19]=550000;
          // snapshot_times[20]=600000;
          // snapshot_times[21]=600000;
          // snapshot_times[22]=600000;
          // snapshot_times[23]=650000;
          // snapshot_times[24]=700000;
          // snapshot_times[25]=750000;
          // snapshot_times[26]=800000;
          // snapshot_times[27]=840000;
          // snapshot_times[28]=870000;
          // snapshot_times[29]=900000;
          // snapshot_times[30]=930000;
          // snapshot_times[31]=960000;
          // snapshot_times[32]=1000000;
          // snapshot_times[33]=1100000;
          // snapshot_times[34]=1200000;
          // snapshot_times[35]=1300000;
          // snapshot_times[36]=1400000;
          // snapshot_times[37]=1500000;
          // snapshot_times[38]=1600000;
          // snapshot_times[39]=1700000;
          // snapshot_times[40]=1800000;
          // snapshot_times[41]=1900000;*/
      
		

        /*		for(int i = 0; i < snapshot_times.size(); i++)///////////////////////was passiert hier/????????////====warum die snapshot zeit veraendern???========?????
                {
                int time_res = 1000;
                snapshot_times[i] = (int) (500.*pow(10,((double)i/10.)));
                    //cout<<snapshot_times[i]<<endl;
                        //snapshot_times[i] = i*time_res + 25000;
                        }*/
	
}


void plot(const string &dirname, const PathOptions& opt, vector<double> &snapshot_times, AverageClass<Bh3Evaluation::Averages> *av)
{  
	stringstream d;
	d << dirname << "/" << "spectrum" << "/";
	string dir = d.str();
	system((string("mkdir -p ") + dir).c_str());
	
    ofstream plotfile;
	
	plotfile.open((dir + string("radial_avgs.dat")).c_str(), ios::out | ios::trunc);
	
	for (int i = 0; i < snapshot_times.size(); i++)
	{
        Bh3Evaluation::Averages means = av[i].av();
        
        for (int r = 0; r < means.number.size(); r++)             
		{
            plotfile << r <<"\t"<< means.k(r) <<"\t" << means.number(r) <<"\t";
            plotfile << means.ikinetick(r) << "\t" << means.ckinetick(r) << "\t";
            plotfile << means.kinetick(r) << "\t" << means.pressure(r) << "\t";
            plotfile << means.ikinetick_wo_phase(r) <<  "\t" ;
            plotfile << means.ckinetick_wo_phase(r) << "\t";
            plotfile << means.kinetick_wo_phase(r) << "\t" ;
            plotfile << means.pressure_wo_phase(r) <<"\t";
            plotfile << endl;
		}
		plotfile << endl << endl;
	}
	
	plotfile.close();
}







  























