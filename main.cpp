/**************************************************************************
Title: Simulating the Expansion of Turbulent Bose-Einstein Condensates (2D) 
Author: Simon Sailer (This work is based on the work of Bartholomew Andrews who made this as his master thesis.)
Last Update: 22/07/13
**************************************************************************/

#include <boost/program_options.hpp>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <cmath>
#include <complex>
#include <complexgrid.h>
#include <exp_RK4_tools.h>
#include <bh3defaultgrid.h>
#include <omp.h>
#include <main.h>
#include <plot_with_mgl.h>
// #include <typeinfo>
// #include <vortexcoordinates.h>


using namespace std;


namespace // namespace for program options
{ 
  const size_t ERROR_IN_COMMAND_LINE = 1; 
  const size_t SUCCESS = 0; 
  const size_t ERROR_UNHANDLED_EXCEPTION = 2; 
 
}

//>>>>>main program<<<<< 

int main( int argc, char** argv) 
{	
	Options opt;
	vector<double> snapshot_times;

	// Initialize all option variables

	init_bh3(argc, argv, opt, snapshot_times);
	

	// Beginning of the options block
try 
{ 
    /** Define and parse the program options 
     */ 
    namespace po = boost::program_options; 
    po::options_description desc("Options"); 
    desc.add_options() 
      ("help,h", "Print help messages.") 
      ("config,c",po::value<string>(&opt.config), "Name of the configfile");
      // ("xgrid,x",po::value<int>(&opt.grid[1]),"Gridsize in x direction.")
      // ("ygrid,y",po::value<int>(&opt.grid[2]),"Gridsize in y direction.")
      // ("gauss",po::value<bool>(&opt.startgrid[0]),"Initial Grid has gaussian form.")
      // ("vortices",po::value<bool>(&opt.startgrid[1]),"Add Vortices to the grid.")
      // ("itp",po::value<int>(&opt.n_it_ITP),"Total runtime of the ITP-Step.")
      // ("rte",po::value<int>(&opt.n_it_RTE),"Total runtime of the RTE-Step.")
      // ("number,N",po::value<double>(&opt.N),"Number of particles.")
      // ("expansion,e",po::value<complex<double> >(&opt.exp_factor),"Expansion Factor")
      // ("interaction,g",po::value<double> (&opt.g),"Interaction Constant");

	po::positional_options_description positionalOptions; 
	positionalOptions.add("config", 1);

    po::variables_map vm; 
    try 
    { 

		po::store(po::command_line_parser(argc, argv).options(desc)
					.positional(positionalOptions).run(), 
          			vm); 
 
      /** --help option 
       */ 
      if ( vm.count("help")  ) 
      { 
        std::cout << "This is a program to simulate the expansion of a BEC with Vortices in 2D" << endl
        		  << "after a harmonic trap has been turned off. The expansion is simulated by" << endl
        		  << "an expanding coordinate system. The implemented algorithm to solve the GPE" << endl
        		  << "is a 4-th order Runge-Kutta Integration." << endl << endl
                  << desc << endl; 
        return SUCCESS; 
      } 
 
      po::notify(vm); // throws on error, so do after help in case 
                      // there are any problems 
    } 
    catch(po::error& e) 
    { 
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
      std::cerr << desc << std::endl; 
      return ERROR_IN_COMMAND_LINE; 
    } 

// Beginning of the main program block

    // print the initial values of the run to the console

    printInitVar(opt); 
		
	// Initialize the needed grid object and run object

	ComplexGrid* startgrid = new ComplexGrid(opt.grid[0],opt.grid[1],opt.grid[2],opt.grid[3]);
	RK4* run = new RK4(startgrid,opt);

	// if the given value is true, initialize the startgrid with a gaussian distribution

	if(opt.startgrid[0]==true){
	double sigma_real[2];
	sigma_real[0] = opt.min_x/2;
	sigma_real[1] = opt.min_y/2;		
	run->pPsi = set_grid_to_gaussian(run->pPsi,opt,run->x_axis,run->y_axis,sigma_real[0],sigma_real[1]);
	}

	// set the datafile identifier name and save the initial grid

    opt.name = "INIT";
	run->save_2D(run->pPsi,opt);	
	plotdatatopng(run->pPsi,opt,false);


	//====> Imaginary Time Propagation (ITP)
	opt.name = "ITP1";
	opt.n_it_ITP = opt.n_it_ITP1;
	run->itpToTime(opt);

//////////// VORTICES ////////////////

	// if the given value is true, add vortices to the startgrid
	if(opt.startgrid[1]==true)
    {
    int sigma_grid[2];
	sigma_grid[0] = opt.grid[1]/4;
	sigma_grid[1] = opt.grid[2]/4;

    run->pPsi = add_vortex_to_grid(run->pPsi,opt,sigma_grid);
   	cout << "Vortices added." << endl;
   	opt.name = "VORT";
   	run->save_2D(run->pPsi,opt);
   	plotdatatopng(run->pPsi,opt,false);
   	}
////// END VORTICES //////////

   	//====> Imaginary Time Propagation (ITP)
    opt.name = "ITP2";
	opt.n_it_ITP = opt.n_it_ITP2;
	run->itpToTime(opt);
	plotdatatopng(run->pPsi,opt,false);


	//====> Real Time Expansion (RTE)
	opt.name = "RTE";
	run->rteToTime(opt);
	plotdatatopng(run->pPsi,opt,false);

	// Everything finished here, plots and cleanup remaining	

	cout << "Run finished." << endl;

    delete startgrid;
	delete run;
 
 
  } 
  // options menu exception catcher
  catch(std::exception& e) 
  { 
    std::cerr << "Unhandled Exception reached the top of main: " 
              << e.what() << ", application will now exit" << std::endl; 
    return ERROR_UNHANDLED_EXCEPTION; 
 
  } 
 
  return SUCCESS; 
	
}



