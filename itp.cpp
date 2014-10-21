/**************************************************************************
Title: Simulating the Expansion of Turbulent Bose-Einstein Condensates (2D) 
Author: Simon Sailer (This work is based on the work of Bartholomew Andrews who made this as his master thesis.)
Last Update: 22/07/13
**************************************************************************/

#include <boost/program_options.hpp>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
// #include <cstring>
#include <string>
#include <cmath>
#include <complex>
#include <omp.h>
#include <sys/stat.h>
#include <dirent.h>

#include <complexgrid.h>
#include <bh3defaultgrid.h>
#include <averageclass.h>
#include <bh3observables.h>

#include <EXP2D_MatrixData.h>
#include <main.h>
#include <EXP2D_tools.h>
#include <EXP2D_itp.hpp>
// #include <EXP2D_rte.hpp>
#include <EXP2D_binaryfile.h>
#include <EXP2D_evaluation.h>
#include <plot_with_mgl.h>
#include <EXP2D_startgrids.h>

// #include <typeinfo>

#define SUCCESS 0
#define ERROR_IN_COMMAND_LINE 1
#define ERROR_IN_CONFIG_FILE 2
#define ERROR_UNHANDLED_EXCEPTION 3
#define DEBUG_LOG 1

using namespace std;

int main( int argc, char** argv)
{	
try{
	cout << "EigenThreads: " << Eigen::nbThreads() << endl;
	StartUp startUp(argc,argv);	

	#if DEBUG_LOG
 		std::ofstream logstream("run.log");
 		redirecter redirectcout(logstream,std::cout); // redirects cout to logstream, until termination of this program. If DEBUG_LOG 1 is set, use cerr for output to console.
 		// std::ofstream errorstream("error.log");
 		// redirecter redirectcerr(errorstream,std::cerr);
 	#endif

 	startUp.printInitVar();

 	omp_set_num_threads(6);
	
	// MatrixData* startGrid = new MatrixData(startUp.getMeta());
	Options tmpOpt = startUp.getOptions();
	MatrixData* startGrid = new MatrixData(1,tmpOpt.grid[1],tmpOpt.grid[2],0,0,tmpOpt.min_x,tmpOpt.min_y);

	setGridToGaussian(startGrid,startUp.getOptions());

	// string groundStateName = "StartGrid_2048x2048_N1000_groundState.h5";
	// binaryFile* groundStateFile = new binaryFile(groundStateName,binaryFile::in);
	// groundStateFile->getSnapshot("StartGrid",0,startGrid,tmpOpt);
	// delete groundStateFile;

	ITP* groundStateITP = new ITP(startGrid->wavefunction[0],startUp.getOptions());
	string groundStateName = "ITP-Groundstate"+to_string(tmpOpt.vortexspacing);
	groundStateITP->propagateToGroundState(groundStateName);
	startGrid->wavefunction[0] = groundStateITP->result();
	delete groundStateITP;

		int vnumber = 0;
		addVorticesAlternating(startGrid,startUp.getOptions(),vnumber);
		
		startUp.setVortexnumber(vnumber);
		cout << endl << "Set Vortices #: " << vnumber << endl;
	
		string itpname = "ITP-Vortices"+to_string(tmpOpt.vortexspacing);
		ITP* vorticesITP = new ITP(startGrid->wavefunction[0],startUp.getOptions());
		vorticesITP->formVortices(itpname);
		// vorticesITP->findVortices(itpname);
		
		startGrid->wavefunction[0] = vorticesITP->result();
	
		delete vorticesITP;


	string startGridName = "StartGrid_2048x2048_N1000_sV_WN1_40_53_"+to_string(tmpOpt.vortexspacing)+".h5";
	binaryFile* dataFile = new binaryFile(startGridName,binaryFile::out);
	dataFile->appendSnapshot("StartGrid",0,startGrid,tmpOpt);
	delete dataFile;
	delete startGrid;		

}  // exceptions catcher


catch(const std::exception& e) 
{ 
  	std::cerr << "Unhandled Exception reached the top of main: " 
    	      << e.what() << ", application will now exit" << std::endl; 
	return ERROR_UNHANDLED_EXCEPTION; 
}
catch(expException& e){
	e.printString();
	std::cerr << " Terminating now." << endl;
	return ERROR_UNHANDLED_EXCEPTION;
}
catch (const std::string& errorMessage) 
{ 
	std::cerr << errorMessage.c_str(); 
	std::cerr << " Terminating now." << endl; 
	return SUCCESS; 
// the code could be different depending on the exception message 
}
cerr << "Run complete. Terminating successfully." << endl; 
return SUCCESS; 	
}



