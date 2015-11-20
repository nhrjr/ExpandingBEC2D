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
	InitMain initMain(argc,argv);	

	#if DEBUG_LOG
 		std::ofstream logstream("run.log");
 		redirecter redirectcout(logstream,std::cout); // redirects cout to logstream, until termination of this program. If DEBUG_LOG 1 is set, use cerr for output to console.
 		// std::ofstream errorstream("error.log");
 		// redirecter redirectcerr(errorstream,std::cerr);
 	#endif

 	initMain.printInitVar();
 	Options tmpOpt = initMain.getOptions();
 	MatrixData* startGrid = new MatrixData(1,tmpOpt.grid[1],tmpOpt.grid[2],0,0,tmpOpt.min_x,tmpOpt.min_y);

 	// omp_set_num_threads(12);/
	if(!initMain.restart()){
		// MatrixData* startGrid = new MatrixData(initMain.getMeta());
			
		setGridToTF(startGrid,initMain.getOptions());
		// // setGridToGaussian(startGrid,initMain.getOptions());
	
		ITP* groundStateITP = new ITP(startGrid->wavefunction[0],initMain.getOptions());
		string groundStateName = "ITP-Groundstate";
		groundStateITP->propagateToGroundState(groundStateName);
		startGrid->wavefunction[0] = groundStateITP->result();
		delete groundStateITP;
		
		string bfString = "StartGrid_2048_2048_NV_groundstate.h5";
		binaryFile* bF = new binaryFile(bfString,binaryFile::out);
		bF->appendSnapshot("StartGrid",0,startGrid,tmpOpt);
		delete bF;
	}

	if(initMain.restart()){	
		string startName = "StartGrid_2048_2048_NV_groundstate.h5";
		binaryFile* startFile = new binaryFile(startName,binaryFile::in);
		startFile->getSnapshot("StartGrid",0,startGrid,tmpOpt);
		delete startFile;
		tmpOpt = initMain.getOptions();

		int vnumber = 0;
		addVorticesAlternating(startGrid,initMain.getOptions(),vnumber);
		
		initMain.setVortexnumber(vnumber);
		cout << endl << "Set Vortices #: " << vnumber << endl;
	
		string itpname = "ITP-Vortices"+to_string(tmpOpt.vortexspacing);
		ITP* vorticesITP = new ITP(startGrid->wavefunction[0],initMain.getOptions());
		// vorticesITP->propagateToGroundState(itpname);
		vorticesITP->formVortices(itpname);
		// vorticesITP->findVortices(itpname);
		
		startGrid->wavefunction[0] = vorticesITP->result();
	
		delete vorticesITP;

		string startGridName = "StartGrid_2048_2048.h5";
		binaryFile* dataFile = new binaryFile(startGridName,binaryFile::out);
		dataFile->appendSnapshot("StartGrid",0,startGrid,tmpOpt);
		delete dataFile;
		delete startGrid;	
	}




	

}
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



