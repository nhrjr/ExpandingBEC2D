#define EIGEN_VECTORIZE

#include <EXP2D_itp.hpp>
#include <omp.h>

#define VORTICES_BUILD_TIME 10000

#define EIGEN_VECTORIZE
#define EIGEN_PARALLELIZE

using namespace std;
using namespace Eigen;

ITP::ITP()
{
  	// some constants used in computations to shorten stuff
	pi = M_PI;
	scaleFactor = 1;
}

ITP::ITP(MatrixXcd &wavedata,const Options &externaloptions)
{	
	// Both essential Variables
	// pPsi = c;
	wavefct = wavedata;
  	opt = externaloptions;
  	plot("ITP-INIT");


  	// some constants used in computations to shorten stuff
  	scaleFactor = 1;
	pi = M_PI;
 	zero=complex<double>(0,0);
 	half=complex<double>(0.5,0);
 	one=complex<double>(1,0);
 	two=complex<double>(2,0);
 	four=complex<double>(4,0);
 	six=complex<double>(6,0);
 	i_unit=complex<double>(0,1);


 	// setting up multithreading. Output to see what Eigen is doing.
	omp_set_num_threads(omp_get_max_threads());
	cout << "Max Number of Threads: " << omp_get_max_threads() << endl;	
	cout << "Eigenthreads: " << Eigen::nbThreads() << endl;

	// Using RunSetup to initialize the remaining variables.
	RunSetup();

}

ITP::~ITP(){

}

MatrixXcd ITP::result(){
	return wavefct;
}

void ITP::setOptions(Options &externaloptions){
	opt = externaloptions;

}

void ITP::RunSetup(){

	//Initialize and fill the Eigen Wavefunction Storage
	// wavefct = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);

	// the time-step sizes for Runge-Kutta integration for both schemes as complex valued variables
 	t_ITP = complex<double>(opt.ITP_step,0.0);

	// Grid Spacing variables
	h_x = complex<double>((2.*opt.min_x/opt.grid[1]),0.0);
  	h_y = complex<double>((2.*opt.min_y/opt.grid[2]),0.0);
  	cout << "Grid Spacing: " << h_x << " " << h_y << endl;  

  	// Coordinate vectors/arrays in different forms etc.
  	x_axis.resize(opt.grid[1]);
  	y_axis.resize(opt.grid[2]);
  	for(int i=0;i<opt.grid[1];i++){x_axis[i]=-opt.min_x+i*real(h_x);}
  	for(int j=0;j<opt.grid[2];j++){y_axis[j]=-opt.min_y+j*real(h_y);}

  	X = VectorXcd(opt.grid[1]); Y = VectorXcd(opt.grid[2]);
	for(int i = 0;i<opt.grid[1];i++){X(i) = complex<double>(x_axis[i],0.0);}
	for(int j = 0;j<opt.grid[2];j++){Y(j) = complex<double>(y_axis[j],0.0);}

   	// The laplacian and potential coefficients needed for the ITP scheme
   	// Precomputing

   	PotentialGrid = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);
   	for(int i = 0; i< opt.grid[1]; i++){
   		for(int j = 0; j < opt.grid[2]; j++){
   			// PotentialGrid(i,j) = complex<double>(rotatingPotential(i,j,45),0.0);
			PotentialGrid(i,j) = complex<double>(opt.potFactor,0.0) * /*two **/ (half * opt.omega_x * opt.omega_x * ( /*0.05 * X(i) * X(i) * X(i) * X(i) -*/ X(i) * X(i) ) +  half * opt.omega_y * opt.omega_y * Y(j) * Y(j) );
   		}
   	}

 //   	for(int i = 0; i< opt.grid[1]/2; i++){for(int j = 0; j < opt.grid[2]; j++){
	// PotentialGrid(i,j) = complex<double>(opt.potFactor,0.0) * /*two **/ (half * opt.omega_x * opt.omega_x * ( /*0.05 * X(i) * X(i) * X(i) * X(i) -*/ X(i+opt.grid[1]/4) * X(i+opt.grid[1]/4) ) +  half * opt.omega_y * opt.omega_y * Y(j) * Y(j) );}}

 //   	for(int i = opt.grid[1]/2; i< opt.grid[1]; i++){for(int j = 0; j < opt.grid[2]; j++){
	// PotentialGrid(i,j) = complex<double>(opt.potFactor,0.0) * /*two **/ (half * opt.omega_x * opt.omega_x * ( /*0.05 * X(i) * X(i) * X(i) * X(i) -*/ X(i-opt.grid[1]/4) * X(i-opt.grid[1]/4) ) +  half * opt.omega_y * opt.omega_y * Y(j) * Y(j) );}}

   	double factor =  1.0;// / (2);

	itp_laplacian_x = complex<double>(factor * 1.0,0.0) / (complex<double>(12.0,0.0) * h_x * h_x);
	itp_laplacian_y = complex<double>(factor * 1.0,0.0) / (complex<double>(12.0,0.0) * h_y * h_y);

	opt.stateInformation.resize(2);
	opt.stateInformation[0] = 1.0;
	opt.stateInformation[1] = 1.0;

}

void ITP::plot(const string name){
	// cout << "Plotting.." << endl;
		plotDataToPngEigen(name, wavefct,opt);
}

inline double ITP::rescale(MatrixXcd &wavefct){	

	// cout << "Rescale " << h_x << " " << h_y << endl;
	double Integral = 0.0;  
	// for(int i=0;i<opt.grid[1]-1;i++){
    	// for(int j=0;j<opt.grid[2]-1;j++){
    		Integral = real(h_x)*real(h_y)*wavefct.cwiseAbs2().sum();
      		// Integral += abs2(wavefct(i,j));      
    	// }
    // }
    // cout << "Integral" << Integral << endl;
    double value = opt.N/Integral;
    particleNumber = Integral;	
	// cout << "Integral : " << Integral << " scalefactor: " << scaleFactor << " " << sqrt(scaleFactor) << endl;
	wavefct.array() *= sqrt(value);
	return value;
}

void ITP::cli(string name,int counter_state, int counter_max, double start)
{
	if(counter_state%(counter_max/10)==0)
		{
			int seconds;
			int min;
			int hour;
			int total;

			total = omp_get_wtime() - start;
			hour = total / 3600;
			min = (total / 60) % 60;
			seconds = total % 60;

			double difference = (old_scaleFactor - scaleFactor);
			old_scaleFactor = scaleFactor;

			cout << "  " << name << " with " << VORTICES_BUILD_TIME << " Steps: "
				 << std::setw(2) << std::setfill('0') << hour << ":"
				 << std::setw(2) << std::setfill('0') << min << ":"
				 << std::setw(2) << std::setfill('0') << seconds  << "    "
				 << std::setw(3) << std::setfill('0') << (counter_state/(counter_max/100)) << "%"
				 << " Rescale Diff: " << std::setprecision (15) << difference
				 << " Particle Number: " << std::setprecision(15) << particleNumber
				 << "\r" << flush;
			plot("ITP-Vortices-" + to_string(counter_state));
		}
	if(counter_state == counter_max)
	{
		cout << endl;
	}
}

void ITP::CopyComplexGridToEigen(){
	for(int i = 0; i < opt.grid[1]; i++){for(int j = 0; j < opt.grid[2]; j++){ wavefct(i,j) = pPsi->at(0,i,j,0);}}
}

void ITP::CopyEigenToComplexGrid(){
	for(int i = 0; i < opt.grid[1]; i++){for(int j = 0; j < opt.grid[2]; j++){ pPsi->at(0,i,j,0) = wavefct(i,j);}}
}

void ITP::formVortices(string runname){
	double start;  // starttime of the run
	int state = 0;
	scaleFactor = 0;
	old_scaleFactor = 0;

	// CopyComplexGridToEigen();

	wavefctcp = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);
	k0 = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);
	k1 = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);
	k2 = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);
	k3 = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);

	start = omp_get_wtime();

	for(int m = 1; m <= VORTICES_BUILD_TIME; m++){

		ComputeDeltaPsi(wavefct, wavefctcp);

		state++;

		scaleFactor = rescale(wavefct);	

		cli(runname,m,VORTICES_BUILD_TIME,start);
	}

	// rescale(wavefct);
	plot(runname + "Vortices-Layout");
	cout << endl;
	// update the ComplexGrid* DATA object outside of this.
	// CopyEigenToComplexGrid();
}


void ITP::findVortices(string runname)
{
	double start;  // starttime of the run
	bool finished = false;
	int counter_finished = 0;
	int state = 0;
	int old_Ekin = 0;

	// load external Data into wavefct
	// CopyComplexGridToEigen();

	Eval breakCondition;
	MatrixXcd wavefctcp = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);
	MatrixXcd k0 = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);
	MatrixXcd k1 = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);
	MatrixXcd k2 = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);
	MatrixXcd k3 = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);

	start = omp_get_wtime();

	//start loop here
	Eigen::initParallel();

	// plot("ITP-1");

	// do{
	// 	rescale(wavefct);
	// 	if(scaleFactor == 1){
	// 		finished = true;
	// 	}
	// } while (finished == false);

	// finished = false;

	// for(int m = 1; scaleFactor < 0.99 && scaleFactor > 1.01; m++){
	do {
		for(int m = 0; m < 50; m++){			

			// wavefct.row(0) = VectorXcd::Zero(opt.grid[1]);
			// wavefct.row(opt.grid[1]-1) = VectorXcd::Zero(opt.grid[1]);
			// wavefct.col(0) = VectorXcd::Zero(opt.grid[2]);
			// wavefct.col(opt.grid[2]-1) = VectorXcd::Zero(opt.grid[2]);			

			wavefctcp = wavefct;
	
			ITP_compute_k_parallel(k0,wavefctcp);
	
			wavefctcp = wavefct + half * t_ITP * k0;
			ITP_compute_k_parallel(k1,wavefctcp);
	
			wavefctcp = wavefct + half * t_ITP * k1;
			ITP_compute_k_parallel(k2,wavefctcp);
	
			wavefctcp = wavefct + t_ITP * k2;
			ITP_compute_k_parallel(k3,wavefctcp);
	
			wavefct += (t_ITP/six) * ( k0 + two * k1 + two * k2 + k3);			
	
			state++;

			// plot("ITP-Groundstate-"+to_string(state)+"-Before-Rescale");

			rescale(wavefct);			
				
		}
		

		breakCondition.saveData(wavefct,opt,state,runname);
		breakCondition.evaluateDataITP();

		cli_groundState(runname,start,state,breakCondition.totalResult);
		int difference = breakCondition.totalResult.Ekin - old_Ekin;
		if(difference == 0){
		// if(scaleFactor == 0){
			counter_finished++;
		}else{
			counter_finished = 0;
		}
		old_Ekin = breakCondition.totalResult.Ekin;
		if(counter_finished >= 2){
			finished = true;
		}
		// cli_plot(runname,m,runtime,start,plot);
	} while (finished == false);

	cout << endl;
	cout << "Groundstate found." << endl;

	// update the ComplexGrid* DATA object outside of this.
	// CopyEigenToComplexGrid();
}


void ITP::propagateToGroundState(string runname)
{
	double start;  // starttime of the run
	bool finished = false;
	int counter_finished = 0;
	int state = 0;
	old_scaleFactor = 0;
	scaleFactor = 0;

	// load external Data into wavefct
	// CopyComplexGridToEigen();

	Eval breakCondition;
	wavefctcp = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);
	k0 = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);
	k1 = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);
	k2 = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);
	k3 = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);

	start = omp_get_wtime();

	//start loop here
	// Eigen::initParallel();

	// plot("ITP-1");

	// do{
	// 	rescale(wavefct);
	// 	if(scaleFactor == 1){
	// 		finished = true;
	// 	}
	// } while (finished == false);

	// finished = false;

	// for(int m = 1; scaleFactor < 0.99 && scaleFactor > 1.01; m++){
	do {
		for(int m = 0; m < 300; m++){

			// wavefct.row(0) = VectorXcd::Zero(opt.grid[1]);
			// wavefct.row(opt.grid[1]-1) = VectorXcd::Zero(opt.grid[1]);
			// wavefct.col(0) = VectorXcd::Zero(opt.grid[2]);
			// wavefct.col(opt.grid[2]-1) = VectorXcd::Zero(opt.grid[2]);

			ComputeDeltaPsi(wavefct, wavefctcp);

			// wavefctcp = wavefct;
	
			// ITP_compute_k_parallel(k0,wavefctcp);
	
			// wavefctcp = wavefct + half * t_ITP * k0;
			// ITP_compute_k_parallel(k1,wavefctcp);
	
			// wavefctcp = wavefct + half * t_ITP * k1;
			// ITP_compute_k_parallel(k2,wavefctcp);
	
			// wavefctcp = wavefct + t_ITP * k2;
			// ITP_compute_k_parallel(k3,wavefctcp);
	
			// wavefct += (t_ITP/six) * ( k0 + two * k1 + two * k2 + k3);			
	
			state++;

			// plot("ITP-Groundstate-"+to_string(state)+"-Before-Rescale");

			scaleFactor = rescale(wavefct);			
				
		}
		

		// breakCondition.saveData(wavefct,opt,state,runname);
		// breakCondition.evaluateDataITP();
		

		// cli_groundState(runname,start,state,breakCondition.totalResult);
		plot(runname+"-"+to_string(state));
		// cout << endl << "breakC = " << breakCondition.totalResult.Ekin << " " << "Old Ekin " << old_Ekin;
		double difference = (old_scaleFactor - scaleFactor);
		cout << endl << "ITP Difference: " << std::setprecision (15) << difference << endl;
		if(fabs(difference) <= 1.0e-9){
		// if(scaleFactor == 0){
			counter_finished++;
		}else{
			counter_finished = 0;			
		}
		old_scaleFactor = scaleFactor;
		// old_Ekin = breakCondition.totalResult.Ekin;		
		if(counter_finished >= 2){
			finished = true;
		}
		// cli_plot(runname,m,runtime,start,plot);
	} while (finished == false);

	cout << endl;
	cout << "Groundstate found." << endl;

	// update the ComplexGrid* DATA object outside of this.
	// CopyEigenToComplexGrid();
}

void ITP::cli_groundState(string name, double start,int state,Observables totalResult){	

			int seconds;
			int min;
			int hour;
			int total;

			total = omp_get_wtime() - start;
			hour = total / 3600;
			min = (total / 60) % 60;
			seconds = total % 60;	

		cout << "  " << name << ":"
			<< std::setw(5) << std::setfill(' ') << state
			<< " Kinetic Energy: " << std::setw(5) << std::setfill(' ') << totalResult.Ekin << " "
			<< " Particles: " << std::setw(12) << std::setfill(' ') << totalResult.particle_count << " "
			<< " Volume: " << std::setw(12) << std::setfill(' ') << totalResult.volume << " "
			<< " Density: " << std::setw(12) << std::setfill(' ') << totalResult.density << " "
			<< " Scaling Factor: " << std::setw(12) << std::setfill(' ') << scaleFactor << " "
			<< std::setw(2) << std::setfill('0') << hour << ":"
			<< std::setw(2) << std::setfill('0') << min << ":"
			<< std::setw(2) << std::setfill('0') << seconds  << "\r" << flush;

			plot(name+"-"+to_string(state));


}

inline void ITP::ITP_compute_k(MatrixXcd &k,MatrixXcd &wavefctcp){
	// Matrix<std::complex<double>,Dynamic,Dynamic,ColMajor> wavefctcpX = Matrix<std::complex<double>,Dynamic,Dynamic,ColMajor>::Zero(opt.grid[1],opt.grid[2]);
	// Matrix<std::complex<double>,Dynamic,Dynamic,RowMajor> wavefctcpY = Matrix<std::complex<double>,Dynamic,Dynamic,RowMajor>::Zero(opt.grid[1],opt.grid[2]);
	
	int subx = opt.grid[1]-2;
	int suby = opt.grid[2]-2;
	MatrixXcd wavefctcpX(subx,suby);
	MatrixXcd wavefctcpY(subx,suby);
	k = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);

	// laplacian
	// #pragma omp parallel for
	// for(int j = 1;j<opt.grid[2]-1;j++){
	// 	for(int i = 1;i<opt.grid[1]-1;i++){
	// 		wavefctcpX(i,j) = wavefctcp(i-1,j) - two * wavefctcp(i,j) + wavefctcp(i+1,j);
	// 		wavefctcpY(i,j) = wavefctcp(i,j-1) - two * wavefctcp(i,j) + wavefctcp(i,j+1);
	// 	}
	// }
	// k.noalias() += wavefctcpX * itp_laplacian_x + wavefctcpY * itp_laplacian_y;

	k.block(1,1,subx,suby).noalias() += (wavefctcp.block(0,1,subx,suby) - two * wavefctcp.block(1,1,subx,suby) + wavefctcp.block(2,1,subx,suby)) * itp_laplacian_x
									  + (wavefctcp.block(1,0,subx,suby) - two * wavefctcp.block(1,1,subx,suby) + wavefctcp.block(1,2,subx,suby)) * itp_laplacian_y;

	// interaction + potential
	k.array() -= (PotentialGrid.array() + complex<double>(opt.g,0.0) * ( wavefctcp.conjugate().array() * wavefctcp.array() )) * wavefctcp.array();

}

void ITP::ITP_compute_k_parallel(MatrixXcd &k, MatrixXcd &wavefctcp){
	int32_t threads = 16; //  omp_get_num_threads();
	// cerr << "threads" << threads << endl;
	int subx = opt.grid[1]-2;
	int suby = opt.grid[2]-2;
	vector<int32_t> frontx(threads);
	vector<int32_t> endx(threads);
	// vector<int32_t> fronty(threads);
	// vector<int32_t> endy(threads);
	int32_t partx = opt.grid[1] / threads;
	// int32_t party = opt.grid[2] / threads;

	// k = MatrixXcd::Zero(opt.grid[1],opt.grid[2]);

	for(int i = 0; i < threads; i++){
		if(i == 0){ frontx[i] = (i * partx) + 1;}
		else{ frontx[i] = (i *partx);}
		if((i == threads-1) || (i == 0)){ endx[i] = partx-1;}
		else{endx[i] = partx;}
	}


	// // #pragma omp parallel for
	// for (int i = 0; i < threads; ++i){
	// 	// cerr << "Thread# " << omp_get_thread_num() << endl;
	// 	cerr << "kblock = " << frontx[i] << "," << 1 << "," << endx[i] << "," << suby << endl;
	// 	cerr << frontx[i]-1 << "," << 1 << "," << endx[i] << "," << suby << endl;
	// 	cerr << frontx[i]   << "," << 1 << "," << endx[i] << "," << suby << endl;
	// 	cerr << frontx[i]+1 << "," << 1 << "," << endx[i] << "," << suby << endl;
	// 	cerr << frontx[i]   << "," << 0 << "," << endx[i] << "," << suby << endl;
	// 	cerr << frontx[i]   << "," << 1 << "," << endx[i] << "," << suby << endl;
	// 	cerr << frontx[i]   << "," << 2 << "," << endx[i] << "," << suby << endl;
	// }

	// NEUMANN BOUNDARY CONDITIONS
	k.row(0) = - (PotentialGrid.row(0).array() + complex<double>(opt.g,0.0) * ( wavefctcp.row(0).conjugate().array() * wavefctcp.row(0).array() )) * wavefctcp.row(0).array();
	k.row(opt.grid[1]-1) = - (PotentialGrid.row(opt.grid[1]-1).array() + complex<double>(opt.g,0.0) * ( wavefctcp.row(opt.grid[1]-1).conjugate().array() * wavefctcp.row(opt.grid[1]-1).array() )) * wavefctcp.row(opt.grid[1]-1).array();
	k.col(0) = - (PotentialGrid.col(0).array() + complex<double>(opt.g,0.0) * ( wavefctcp.col(0).conjugate().array() * wavefctcp.col(0).array() )) * wavefctcp.col(0).array();
	k.col(opt.grid[2]-1) = - (PotentialGrid.col(opt.grid[2]-1).array() + complex<double>(opt.g,0.0) * ( wavefctcp.col(opt.grid[2]-1).conjugate().array() * wavefctcp.col(opt.grid[2]-1).array() )) * wavefctcp.col(opt.grid[2]-1).array();
	// END CONDITIONS



	#pragma omp parallel for
	for (int i = 0; i < threads; ++i){
		k.block(frontx[i],1,endx[i],suby).noalias() =       (wavefctcp.block(frontx[i]-1,1,endx[i],suby)
													 - two * wavefctcp.block(frontx[i]  ,1,endx[i],suby)
													       + wavefctcp.block(frontx[i]+1,1,endx[i],suby)) * itp_laplacian_x
													      + (wavefctcp.block(frontx[i]  ,0,endx[i],suby)
													 - two * wavefctcp.block(frontx[i]  ,1,endx[i],suby)
													       + wavefctcp.block(frontx[i]  ,2,endx[i],suby)) * itp_laplacian_y;	
		k.block(frontx[i],1,endx[i],suby).array() -= (PotentialGrid.block(frontx[i],1,endx[i],suby).array() + complex<double>(opt.g,0.0) * ( wavefctcp.block(frontx[i],1,endx[i],suby).conjugate().array() * wavefctcp.block(frontx[i],1,endx[i],suby).array() )) * wavefctcp.block(frontx[i],1,endx[i],suby).array();
	}
}

void ITP::ComputeDeltaPsi(MatrixXcd &wavefct, MatrixXcd &wavefctcp){

	int32_t threads = 16;
	int32_t subx = opt.grid[1]-4;
	int32_t suby = opt.grid[2]-4;
	vector<int32_t> frontx(threads);
	vector<int32_t> endx(threads);
	int32_t partx = opt.grid[1] / threads;

	for(int i = 0; i < threads; i++){
		if(i == 0){ frontx[i] = 2;}
		else{ frontx[i] = (i *partx);}
		if((i == threads-1) || (i == 0)){ endx[i] = partx-2;}
		else{endx[i] = partx;}
	}

	#pragma omp parallel
	{	
		#pragma omp for
		for(int i = 0; i < threads; ++i){
			wavefctcp.block(i * partx,0,partx,opt.grid[2]) = wavefct.block(i * partx,0,partx,opt.grid[2]);
		}

		#pragma omp for
		for(int i = 0; i < threads; ++i){
			singleK(k0,wavefctcp,frontx[i],endx[i],subx,suby);
		}

		#pragma omp for
		for(int i = 0; i < threads; ++i){
			wavefctcp.block(i * partx,0,partx,opt.grid[2]) = wavefct.block(i * partx,0,partx,opt.grid[2]) + half * t_ITP * k0.block(i * partx,0,partx,opt.grid[2]);
		}

		#pragma omp for
		for(int i = 0; i < threads; ++i){
			singleK(k1,wavefctcp,frontx[i],endx[i],subx,suby);
		}
		#pragma omp for
		for(int i = 0; i < threads; ++i){
			wavefctcp.block(i * partx,0,partx,opt.grid[2]) = wavefct.block(i * partx,0,partx,opt.grid[2]) + half * t_ITP * k1.block(i * partx,0,partx,opt.grid[2]);
		}
	
		#pragma omp for
		for(int i = 0; i < threads; ++i){
			singleK(k2,wavefctcp,frontx[i],endx[i],subx,suby);
		}
		#pragma omp for
		for(int i = 0; i < threads; ++i){
			wavefctcp.block(i * partx,0,partx,opt.grid[2]) = wavefct.block(i * partx,0,partx,opt.grid[2]) + t_ITP * k2.block(i * partx,0,partx,opt.grid[2]);
		}	
		
		#pragma omp for
		for(int i = 0; i < threads; ++i){
			singleK(k3,wavefctcp,frontx[i],endx[i],subx,suby);
		}
	
		#pragma omp for
		for(int i = 0; i < threads; ++i){
			wavefctcp.block(i * partx,0,partx,opt.grid[2]) = (one/six) * ( k0.block(i * partx,0,partx,opt.grid[2]) + two * k1.block(i * partx,0,partx,opt.grid[2]) + two * k2.block(i * partx,0,partx,opt.grid[2]) + k3.block(i * partx,0,partx,opt.grid[2]));
		}

		#pragma omp barrier
		#pragma omp single
		{
			// NEUMANN BOUNDARY CONDITIONS
			wavefctcp.row(0) = - (PotentialGrid.row(0).array() + complex<double>(opt.g,0.0) * ( wavefctcp.row(0).conjugate().array() * wavefctcp.row(0).array() )) * wavefctcp.row(0).array();
			wavefctcp.row(opt.grid[1]-1) = - (PotentialGrid.row(opt.grid[1]-1).array() + complex<double>(opt.g,0.0) * ( wavefctcp.row(opt.grid[1]-1).conjugate().array() * wavefctcp.row(opt.grid[1]-1).array() )) * wavefctcp.row(opt.grid[1]-1).array();
			wavefctcp.col(0) = - (PotentialGrid.col(0).array() + complex<double>(opt.g,0.0) * ( wavefctcp.col(0).conjugate().array() * wavefctcp.col(0).array() )) * wavefctcp.col(0).array();
			wavefctcp.col(opt.grid[2]-1) = - (PotentialGrid.col(opt.grid[2]-1).array() + complex<double>(opt.g,0.0) * ( wavefctcp.col(opt.grid[2]-1).conjugate().array() * wavefctcp.col(opt.grid[2]-1).array() )) * wavefctcp.col(opt.grid[2]-1).array();

			wavefctcp.row(1) = - (PotentialGrid.row(1).array() + complex<double>(opt.g,0.0) * ( wavefctcp.row(1).conjugate().array() * wavefctcp.row(1).array() )) * wavefctcp.row(1).array();
			wavefctcp.row(opt.grid[1]-2) = - (PotentialGrid.row(opt.grid[1]-2).array() + complex<double>(opt.g,0.0) * ( wavefctcp.row(opt.grid[1]-2).conjugate().array() * wavefctcp.row(opt.grid[1]-2).array() )) * wavefctcp.row(opt.grid[1]-2).array();
			wavefctcp.col(1) = - (PotentialGrid.col(1).array() + complex<double>(opt.g,0.0) * ( wavefctcp.col(1).conjugate().array() * wavefctcp.col(1).array() )) * wavefctcp.col(1).array();
			wavefctcp.col(opt.grid[2]-2) = - (PotentialGrid.col(opt.grid[2]-2).array() + complex<double>(opt.g,0.0) * ( wavefctcp.col(opt.grid[2]-2).conjugate().array() * wavefctcp.col(opt.grid[2]-2).array() )) * wavefctcp.col(opt.grid[2]-2).array();
			// END CONDITIONS
		}

		#pragma omp for
		for(int i = 0; i < threads; ++i){
			wavefct.block(i * partx,0,partx,opt.grid[2]) += t_ITP * wavefctcp.block(i * partx,0,partx,opt.grid[2]);
		}
	}	

}

void ITP::singleK(MatrixXcd &k, MatrixXcd &wavefctcp, int32_t &front, int32_t &end,int32_t &subx,int32_t & suby){
	// k.block(front,1,end,suby).noalias() =  (wavefctcp.block(front-1,1,end,suby)	- two * wavefctcp.block(front  ,1,end,suby) + wavefctcp.block(front+1,1,end,suby)) * itp_laplacian_x
	// 									 + (wavefctcp.block(front  ,0,end,suby) - two * wavefctcp.block(front  ,1,end,suby) + wavefctcp.block(front  ,2,end,suby)) * itp_laplacian_y;

	// k.block(front,1,end,suby).array() -= ( PotentialGrid.block(front,1,end,suby).array() + complex<double>(opt.g,0.0) * ( wavefctcp.block(front,1,end,suby).conjugate().array() * wavefctcp.block(front,1,end,suby).array() )) * wavefctcp.block(front,1,end,suby).array();

	k.block(front,2,end,suby).noalias() =  (-wavefctcp.block(front-2,2,end,suby) + complex<double>(16.0,0) * wavefctcp.block(front-1,2,end,suby) - complex<double>(30.0,0.0) * wavefctcp.block(front  ,2,end,suby) + complex<double>(16.0,0) * wavefctcp.block(front+1,2,end,suby) - wavefctcp.block(front+2,2,end,suby)) * itp_laplacian_x
										 + (-wavefctcp.block(front  ,0,end,suby) + complex<double>(16.0,0) * wavefctcp.block(front  ,1,end,suby) - complex<double>(30.0,0.0) * wavefctcp.block(front  ,2,end,suby) + complex<double>(16.0,0) * wavefctcp.block(front  ,3,end,suby) - wavefctcp.block(front  ,4,end,suby)) * itp_laplacian_y;

	k.block(front,2,end,suby).array() -= ( PotentialGrid.block(front,2,end,suby).array() + (complex<double>(opt.g,0.0) * ( wavefctcp.block(front,2,end,suby).conjugate().array() * wavefctcp.block(front,2,end,suby).array() ))) * wavefctcp.block(front,2,end,suby).array();

}

inline double ITP::rotatingPotential(int i, int j, int angle){
	double potential;
		double alpha = 2 * M_PI / 360;
		double x = X(i).real() * cos(alpha * angle) + Y(j).real() * sin(alpha * angle);
		double y = - X(i).real() * sin(alpha * angle) + Y(j).real() * cos(alpha * angle);
		potential = 0.5 * opt.omega_x.real() * opt.omega_x.real() * x * x + 0.5 * opt.omega_y.real() * opt.omega_y.real() * y * y;
	return potential;
}
