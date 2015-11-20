#ifndef MAIN_H__
#define MAIN_H__

#include <libconfig.h++>
#include <boost/program_options.hpp>
#include <string>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <stdio.h>

#include <matrixdata.h>

#define SUCCESS 0 
#define ERROR_IN_COMMAND_LINE 1
#define ERROR_IN_CONFIG_FILE 2
#define ERROR_UNHANDLED_EXCEPTION 3

using namespace libconfig;

class redirecter 
{
public:
    redirecter(std::ostream & dst, std::ostream & src)
        : src(src), sbuf(src.rdbuf(dst.rdbuf())) {}
    ~redirecter() { src.rdbuf(sbuf); }
private:
    std::ostream & src;
    std::streambuf * const sbuf;
};

enum MainControl {
	SPLIT,
	EVAL,
	RK4,
	TRAP,
	EXP,
	ROT,
	RESUME,
	RESTART,
	NEW,
	PLOT,
	HYDRO,
	ITP
};



class InitMain {
public:
	InitMain(int argcTmp, char** argvTmp) /*: restartValue(false) */{
		argc = argcTmp;
		argv = argvTmp;
		readCli();
		readConfig();
		// setDirectory();		
	}
	inline void setIteration(int i);
	inline void printInitVar();
	inline void setDirectory();
	inline int readCli();
	inline int readConfig();
	inline void writeConfig(string filename);

	inline Options getOptions();
	inline void setOptions(Options &ext_opt);
	inline void setInitialRun(bool initialRun);
	inline void setVortexnumber(int number);
	inline void setRunMode(string runmode);
	inline void setRunTime(int runtime);
	inline void setOmegaW(double ow);
	inline void setWorkingDirectory(string dir);
	inline MatrixData::MetaData getMeta();
	inline void rotatePotential();
	
	// inline bool restart();
	inline string getStartingGridName();
	inline int getRunTime(){ return opt.n_it_RTE;};
	inline int getSnapShots(){ return opt.snapshots;};
	inline MainControl getRestart(){ return toMainControl(restartString);};
	inline MainControl getDgl(){ return toMainControl(dglString);};
	inline MainControl getAlgorithm(){return toMainControl(algorithmString);};
	inline string getRunName(){return runName;};


	inline void convertToDimensionless();
	inline void convertFromDimensionless();
	inline int getIterations();
private:
	MainControl toMainControl(const std::string& s);
	// bool restartValue;
	string startingGridName;
	MatrixData::MetaData meta;
	Options opt;
	int argc;
	char** argv;
	string algorithmString = "SPLIT";
	string dglString = "Empty String";
	string restartString = "Empty String";
	string runName = "default_runname";

	vector<double> omega_w_vector;
};

MainControl InitMain::toMainControl(const std::string& s)
{	
	// cerr << "to Maincontrol: " << s << endl;
    if (s == "SPLIT") return SPLIT;
    if (s == "EVAL") return EVAL;
    if (s == "RK4") return RK4;
    if (s == "TRAP") return TRAP;
    if (s == "EXP") return EXP;
    if (s == "ROT") return ROT;
    if (s == "RESUME") return RESUME;
    if (s == "RESTART") return RESTART;
    if (s == "NEW") return NEW;
    if (s == "PLOT") return PLOT;
    if (s == "HYDRO") return HYDRO;
    if (s == "ITP") return ITP;
    throw std::runtime_error("Invalid conversion from string to MainControl.");
}

// inline bool InitMain::restart(){
// 	return restartValue;
// }

inline int InitMain::getIterations(){
	return omega_w_vector.size();
}

inline string InitMain::getStartingGridName(){
	return startingGridName;
}

inline void InitMain::setIteration(int i){
	opt.omega_w = omega_w_vector[i];
	setWorkingDirectory("default");
 	setDirectory();
}

inline Options InitMain::getOptions(){
	Options tmpOptions = opt;
	toDimensionlessUnits(tmpOptions);
	return tmpOptions;
}

inline void InitMain::setOptions(Options &ext_opt){
	opt = ext_opt;
}

inline void InitMain::setInitialRun(bool initialRun){
	opt.initialRun = initialRun;
}

inline void InitMain::setOmegaW(double ow){
	opt.omega_w = ow;
	cout << endl << endl << "ow " << ow << " omega_w " << opt.omega_w << endl << endl;
}

inline void InitMain::setWorkingDirectory(string dir){
	opt.workingdirectory = dir;
}

// inline void InitMain::setRunMode(string runmode){
// 	// FIXME: Here should be checks for the sanity of runmode!
// 	opt.runmode = runmode;
// }

// inline MainControl InitMain::getRunMode(){
// 	// FIXME: Here should be checks for the sanity of runmode!
// 	return toMainControl(opt.runmode);
// 	// return opt.runmode;
// }

inline void InitMain::setRunTime(int runtime){
	opt.n_it_RTE = runtime;
}

inline void InitMain::setVortexnumber(int number){
	opt.vortexnumber = number;
}

inline MatrixData::MetaData InitMain::getMeta(){

	Options tmpOptions = opt;
	toDimensionlessUnits(tmpOptions);

	meta.Ag = tmpOptions.Ag;
	meta.OmegaG = tmpOptions.OmegaG;
	meta.grid[0] = tmpOptions.grid[1];
	meta.grid[1] = tmpOptions.grid[2];
	meta.initCoord[0] = meta.coord[0] = tmpOptions.min_x;
	meta.initCoord[1] = meta.coord[1] = tmpOptions.min_y;
	meta.initSpacing[0] = meta.spacing[0] = tmpOptions.min_x * 2 / tmpOptions.grid[1];
	meta.initSpacing[1] = meta.spacing[1] = tmpOptions.min_y * 2 / tmpOptions.grid[2];
	meta.samplesize = tmpOptions.samplesize;
	meta.time = 0;
	meta.steps = 0;
	meta.isDimensionless = tmpOptions.isDimensionless;
	meta.dataToArray();

	cout << "DIMENSIONLESS UNITS: " << endl;
	cout << "h = " << meta.initSpacing[0] << endl;
	cout << "x_max = " << meta.initCoord[0] << endl; 
	double k = 2 * meta.initSpacing[0] * meta.initSpacing[0] / (M_PI * M_PI);
	cout << "Is delta T " << tmpOptions.RTE_step << " < k_threshold = " << k << " ";
	if(tmpOptions.RTE_step < k) cout << "Yes." << endl; else cout << "No." << endl;
	cout << endl;

	return meta;
}

inline void InitMain::rotatePotential(){
	complex<double> tmp = opt.omega_x;
	opt.omega_x = opt.omega_y;
	opt.omega_y = tmp;

	opt.dispersion_x = opt.omega_x;
	opt.dispersion_y = opt.omega_y;
}


inline void InitMain::printInitVar()
{
	std::cout.setf(std::ios::boolalpha);
	std::cout 	<< "Configfile: \"" << opt.config << "\"" << endl
				<< "Gridpoints in x-direction: " << opt.grid[1] << "\t" << "omega_x = " << opt.omega_x.real() << " dispersion_x = " << opt.dispersion_x.real() << endl
				<< "Gridpoints in y-direction: " << opt.grid[2] << "\t" << "omega_y = " << opt.omega_y.real() << " dispersion_y = " << opt.dispersion_y.real() << endl
				<< "omega_w = " << opt.omega_w.real() << endl
				// << "Coordinates in x-direction: " << meta.initCoord[0] << endl
				// << "Coordinates in y-direction: " << meta.initCoord[1] << endl
				<< "Expansionfactor: " << opt.exp_factor.real() << "\t" << "Number of particles: " << opt.N << "\t" << "Interaction constant g: " << opt.g << endl
				<< "Stepsize: " << opt.RTE_step
				<< " Runtime of the RTE: " << opt.n_it_RTE * opt.snapshots << " steps." << endl << endl;
}

inline void InitMain::setDirectory()
{	
	if(opt.workingdirectory == "default"){
		stringstream name;
		name << std::fixed << std::setprecision(0) << (int)opt.N << "_" << opt.grid[1] << "x" << opt.grid[2] << "_" << std::setprecision(3) << opt.g << "_" << std::setprecision(1) << real(opt.omega_w /*/ (2.0 * M_PI / opt.OmegaG)*/);
		opt.workingdirectory = name.str();
	}
	// cout << "Workingdirectory: " << "\"" << opt.workingdirectory << "\"" << endl;
	struct stat wd_stat;
	if(stat(opt.workingdirectory.c_str(),&wd_stat) == 0){
		if(chdir(opt.workingdirectory.c_str()) == 0){
			cerr << endl;
			cerr << "Using existing directory: " << "\"" << opt.workingdirectory << "\"." << endl;
		}
	}else{
		mkdir(opt.workingdirectory.c_str(),0755);
		cerr << "mkdir: " << "\"" << opt.workingdirectory << "\"" << endl;

		if(chdir(opt.workingdirectory.c_str()) == 0){
			cerr << "chdir:" << "\"" << opt.workingdirectory << "\"" << endl;
		}
		cerr << endl;
	}
}


inline int InitMain::readCli()
{
	// Beginning of the options block

    // Define and parse the program options

    namespace po = boost::program_options; 
    po::options_description desc("Options"); 
    desc.add_options() 
      ("help,h", "Print help messages.") 
      ("config,c",po::value<string>(&opt.config), "Name of the configfile.")      
      ("directory,d",po::value<string>(&opt.workingdirectory), "Name of the directory this run saves its data.")
      // ("takeup,t","Start the run from last Grid, with time set to 0")
      // ("resume,r","Resume the run from the last saved Grid.")
      ("restart",po::value<string>(&restartString), "NEW RESUME RESTART PLOT HYDRO")      
      ("dgl",po::value<string>(&dglString), "EXP ROT TRAP")
      ("algo",po::value<string>(&algorithmString)->default_value("SPLIT"), "SPLIT RK4")
      ("name,n",po::value<string>(&runName), "Name of the run.");

      // ("xgrid,x",po::value<int>(&opt.grid[1]),"Gridsize in x direction.")
      // ("ygrid,y",po::value<int>(&opt.grid[2]),"Gridsize in y direction.")
      // ("gauss",po::value<bool>(&opt.startgrid[0]),"Initial Grid has gaussian form.")
      // ("vortices",po::value<bool>(&opt.startgrid[1]),"Add Vortices to the grid.")
      // ("itp",po::value<int>(&opt.n_it_ITP),"Total runtime of the ITP-Step.")
      // ("rte",po::value<int>(&opt.n_it_RTE),"Total runtime of the RTE-Step.")
      // ("number,N",po::value<double>(&opt.N),"Number of particles.")
      // ("expansion,e",po::value<double>(&exp_factor),"Expansion Factor")
      // ("interaction,g",po::value<double> (&opt.g),"Interaction Constant");

    




	po::positional_options_description positionalOptions; 
	positionalOptions.add("config", 1);	
	positionalOptions.add("directory",1);
	positionalOptions.add("restart",1);
	positionalOptions.add("dgl",1);
	positionalOptions.add("algo", 1);	
	// positionalOptions.add("name",1);

    po::variables_map vm; 

    try 
    { 

		po::store(po::command_line_parser(argc, argv).options(desc)
					.positional(positionalOptions).run(), 
          			vm); 
 
      /** --help option 
       */ 
      if ( vm.count("help")  ){ 
        std::cout << "This is a program to simulate the expansion of a BEC with Vortices in 2D" << endl
        		  << "after a harmonic trap has been turned off. The expansion is simulated by" << endl
        		  << "an expanding coordinate system. The implemented algorithm to solve the GPE" << endl
        		  << "is a 4-th order Runge-Kutta Integration." << endl << endl
                  << desc << endl;
        string e = "End of Help.";
        throw e;
      } 
 
      po::notify(vm); // throws on error, so do after help in case 
                      // there are any problems 

  //   	if(vm.count("resume"))
  //   		restartString = "RESUME";
		// else if(vm.count("takeup"))
		// 	restartString = "RESTART";
		// else 
		// 	restartString = "NEW";

    } 
    catch(po::error& e) 
    { 
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
      std::cerr << desc << std::endl; 
      return ERROR_IN_COMMAND_LINE; 
    } 

    return SUCCESS;

}


inline int InitMain::readConfig()
{
	libconfig::Config cfg;

	string sConfig = opt.config;

	  // Read the file. If there is an error, report it and exit.
	  try
	  {
	    cfg.readFile(sConfig.c_str());
	  }
	  catch(const libconfig::FileIOException &fioex)
	  {
	    std::cerr << "I/O error while reading file." << std::endl;
	  }
	  catch(const libconfig::ParseException &pex)
	  {
	    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
	              << " - " << pex.getError() << std::endl;
	  }


	const libconfig::Setting & root = cfg.getRoot();

	try
	{

	opt.N                    = root["RunOptions"]["N"];
	opt.min_x                = root["RunOptions"]["min_x"]; 					
	opt.min_y                = root["RunOptions"]["min_y"];
		
	opt.grid[1]              = root["RunOptions"]["grid_x"];				
	opt.grid[2]              = root["RunOptions"]["grid_y"];	   			
	opt.g                    = root["RunOptions"]["g"]; 						
	opt.n_it_RTE             = root["RunOptions"]["sizeOfSnapshots"]; 				
	opt.snapshots            = root["RunOptions"]["numberOfSnapshots"];
	opt.ITP_step             = root["RunOptions"]["ITP_step"]; 				
	opt.RTE_step             = root["RunOptions"]["RTE_step"];
	opt.samplesize			 = root["RunOptions"]["samplesize"];
	opt.potFactor			 = root["RunOptions"]["potentialFactor"];
	opt.vortexspacing		 = root["RunOptions"]["vortexspacing"];
	opt.runmode = dglString;

	double exp_factor        = root["RunOptions"]["exp_factor"];
	double omega_x_realValue = root["RunOptions"]["omega_x"];  // cfg.lookup("RunOptions.omega_x");
	double omega_y_realValue = root["RunOptions"]["omega_y"];  // cfg.lookup("RunOptions.omega_y");

	omega_w_vector.resize(root["RunOptions"]["omega_w"].getLength());
	for(int i = 0; i < root["RunOptions"]["omega_w"].getLength(); ++i){
		omega_w_vector[i] = root["RunOptions"]["omega_w"][i];
	}
	opt.omega_w = omega_w_vector[0];

	double dispersion_x_realValue = root["RunOptions"]["dispersion_x"]; 
	double dispersion_y_realValue = root["RunOptions"]["dispersion_y"]; 


	opt.exp_factor           = complex<double>(exp_factor,0); //Expansion factor
	opt.omega_x              = complex<double>(omega_x_realValue,0);
	opt.omega_y              = complex<double>(omega_y_realValue,0);
	opt.dispersion_x		 = complex<double>(dispersion_x_realValue,0);
	opt.dispersion_y 		 = complex<double>(dispersion_y_realValue,0);

	


	}
	catch(const SettingNotFoundException &nfex)
	{
	cerr << endl <<  "Something is wrong here with your config." << endl << endl;

	return ERROR_IN_CONFIG_FILE;

	}

	// runspecific Values, just initilized here
	// opt.t_abs = complex<double>(0,0); //Absolute time 
	opt.stateInformation.resize(2);
	opt.stateInformation[0] = 1;
	opt.stateInformation[1] = 1;
	opt.vortexnumber = 0;

    // Set Parameters manually (default values)
	//opt.timestepsize = 0.2;
	//opt.delta_t.resize(0);   
    //opt.delta_t[0]=0.2;
    //opt.delta_t[1]=0.4;
	// 	opt.klength[0] = 2.0;
	// 	opt.klength[1] = 2.0;
	// 	opt.klength[2] = 2.0;

     return SUCCESS;
	
}

inline void InitMain::writeConfig(string filename){
	// string filename = "sim.cfg";
	ofstream datafile;
  		datafile.open(filename.c_str(), ios::out);
  		datafile << "RunOptions =\n" << "{\n"
  				 << "N = " << opt.N << "\n"
				 << "g = " << opt.g << "\n"
				 << "min_x = " << opt.min_x << "\n"
				 << "min_y = " << opt.min_y << "\n"
				 << "grid_x = " << opt.grid[1] << "\n"
				 << "grid_y = " << opt.grid[2] << "\n"
				 << "sizeOfSnapshots = " << opt.n_it_RTE << "\n"
				 << "numberOfSnapshots = " << opt.snapshots << "\n"
				 << "ITP_step = " << opt.ITP_step << "\n"
				 << "RTE_step = " << opt.RTE_step << "\n"
				 << "exp_factor = " << opt.exp_factor.real() << "\n"
				 << "potentialFactor = " << opt.potFactor << "\n"
				 << "vortexspacing = " << opt.vortexspacing << "\n"
				 << "omega_x = " << opt.omega_x.real() << "\n"
				 << "omega_y = " << opt.omega_y.real() << "\n"
				 << "dispersion_x = " << opt.dispersion_x.real() << "\n"
				 << "dispersion_y = " << opt.dispersion_y.real() << "\n"
				 << "samplesize = " << opt.samplesize << "\n"
				 << "}";
	datafile.close();
}

// inline void InitMain::convertToDimensionless(){

// 	const double m = 87 * 1.66 * 1.0e-27;
// 	const double hbar = 1.054 * 1.0e-22;	
// 	opt.Ag = 2 * opt.min_x / opt.grid[1];
// 	opt.OmegaG = hbar / ( m * opt.Ag * opt.Ag);

// 	opt.min_x /= opt.Ag;
// 	opt.min_y /= opt.Ag;
// 	opt.ITP_step *= opt.OmegaG;
// 	opt.RTE_step *= opt.OmegaG;
// 	opt.omega_x *= 2.0 * M_PI / opt.OmegaG;
// 	opt.omega_y *= 2.0 * M_PI / opt.OmegaG;
// 	opt.omega_w *= 2.0 * M_PI / opt.OmegaG;
// 	opt.dispersion_x *= 2.0 * M_PI / opt.OmegaG;
// 	opt.dispersion_y *= 2.0 * M_PI / opt.OmegaG;
// }

inline void InitMain::convertFromDimensionless(){
	cout << "WATCH OUT: InitMain::convertFromDimensionless() is not yet implemented" << endl;
}






#endif // MAIN_H__