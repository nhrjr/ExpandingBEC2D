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
// #include <EXP2D_itp.hpp>
#include <EXP2D_binaryfile.h>
#include <EXP2D_splitstep.hpp>
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

int main( int argc, char** argv){	

try{

	StartUp startUp(argc,argv);	

	#if DEBUG_LOG
 		std::ofstream logstream("run.log");
 		redirecter redirectcout(logstream,std::cout); // redirects cout to logstream, until termination of this program. If DEBUG_LOG 1 is set, use cerr for output to console.
 		// std::ofstream errorstream("error.log");
 		// redirecter redirectcerr(errorstream,std::cerr);
 	#endif

 	startUp.printInitVar();

	Options tmpOpt = startUp.getOptions();
	MainControl mC = startUp.getControl();
	vector<ComplexGrid> data;
	int SLICE_NUMBER = 0;


	if(!startUp.restart()){
		MatrixData* startGrid = new MatrixData(1,tmpOpt.grid[1],tmpOpt.grid[2],0,0,tmpOpt.min_x,tmpOpt.min_y);
	
		// cout << "EigenThreads: " << Eigen::nbThreads() << endl;
		
		string startGridName = startUp.getStartingGridName(); // "StartGrid_2048x2048_N1000_alternatingVortices.h5";
	
		binaryFile* dataFile = new binaryFile(startGridName,binaryFile::in);
		dataFile->getSnapshot("StartGrid",0,startGrid,tmpOpt);
		delete dataFile;

		// addDrivingForce(startGrid,tmpOpt);

		
		data.resize(tmpOpt.samplesize);
		for(int i = 0; i < tmpOpt.samplesize; i++){
			data[i] = ComplexGrid(tmpOpt.grid[0],tmpOpt.grid[1],tmpOpt.grid[2],tmpOpt.grid[3]);
			#pragma omp parallel for
			for(int x = 0; x < tmpOpt.grid[1]; x++){
				for(int y = 0; y < tmpOpt.grid[2]; y++){
					for(int z = 0; z < tmpOpt.grid[3]; z++){					
						data[i](0,x,y,z) = startGrid->wavefunction[0](x,y);
					}
				}
			}
		}
		
		delete startGrid;
	
		string runName = startUp.getRunName();
		SplitStep* runExpanding = new SplitStep(data,startUp.getMeta(),startUp.getOptions(),SLICE_NUMBER);

		if(mC == SPLIT){
			cout << "splitToTime()" << endl;
			runExpanding->splitToTime(runName);
		}

		delete runExpanding;
	}
	
	if(startUp.restart()){
		string runName = startUp.getRunName();
		string filename = runName + "-LastGrid.h5";
		binaryFile* dataFile = new binaryFile(filename,binaryFile::in);
	
		vector<int> timeList = dataFile->getTimeList();
		MatrixData::MetaData tmpMeta;
		dataFile->getSnapshot(runName,timeList[0],data,tmpMeta,tmpOpt);
		delete dataFile;
		tmpOpt.initialRun = false;
		tmpOpt.n_it_RTE = startUp.getRunTime();
		tmpOpt.snapshots = startUp.getSnapShots();
		SplitStep* runExpanding = new SplitStep(data,tmpMeta,tmpOpt,SLICE_NUMBER);

		if(mC == SPLIT){
			cout << "splitToTime()" << endl;
			runExpanding->splitToTime(runName);
		}

		delete runExpanding;
	}
}


catch(const std::exception& e){ 
  	std::cerr << "Unhandled Exception reached the top of main: " 
    	      << e.what() << ", application will now exit" << std::endl; 
	return ERROR_UNHANDLED_EXCEPTION; 
}
catch(expException& e){
	e.printString();
	std::cerr << " Terminating now." << endl;
	return ERROR_UNHANDLED_EXCEPTION;
}
catch (const std::string& errorMessage){ 
	std::cerr << errorMessage.c_str(); 
	std::cerr << " Terminating now." << endl; 
	return SUCCESS; 
}
cerr << "[END]" << endl; 
return SUCCESS; 	
}



