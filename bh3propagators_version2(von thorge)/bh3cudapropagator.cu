/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and 
 * proprietary rights in and to this software and related documentation. 
 * Any use, reproduction, disclosure, or distribution of this software 
 * and related documentation without an express license agreement from
 * NVIDIA Corporation is strictly prohibited.
 *
 * Please refer to the applicable NVIDIA end user license agreement (EULA) 
 * associated with this source code for terms and conditions that govern 
 * your use of this NVIDIA software.
 * 
 */

/* Template project which demonstrates the basics on how to setup a project 
* example application.
* Host code.
*/

// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <gauss_random.h>
//#include <cuda_runtime.h>

// Includes, Kernels++//================================================Hier Kommen die kernels her : ==========================================
#include "bh3cudapropagator.h"
#include <complexgrid.h>
#include <wrapped_cuda_functions.h>

#define BLOCK_SIZE 8
#define BLOCK_1D_LENGTH 512

// Kernels for r- and k-propagation
#include <cuComplex.h>                    //////////////////////////////////////////////////////////////////////////////////////////////was ist denn dass hier????????????????????????////////

using namespace std;

typedef struct {
	double inv_grid[3];
	double klength[3];
	double grid[3];
  double fft_factor, timestepsize;
	double U;
	double u_12;
	double u_22;
        double omega_l;
} ConstKernelOptions;



inline complex<double> noise()
{
	return gauss_random() * 0.5;
}



__constant__ __device__ ConstKernelOptions dev_options;
//__constant__ __device__ double devOmega_t;

__device__ static __inline__ double cuCabs2 (cuDoubleComplex x)
{
	return cuCreal(x)*cuCreal(x) + cuCimag(x)*cuCimag(x);
	}

	
__global__ void init_kprop(cuDoubleComplex *kprop)
{
	// the coordinates for this thread
	const unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	const unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	const unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = x*dev_options.grid[2]*dev_options.grid[1] + y*dev_options.grid[2] + z;
	
	// calculate the propagator
	double T = 0.0;
	double k = 0.0;
	
	k = x * dev_options.inv_grid[0];/////////////////
	k = sin(k);
	k *= dev_options.klength[0];
	k *= k;
	T -= k;
	k = y * dev_options.inv_grid[1];
	k = sin(k);
	k *= dev_options.klength[1];
	k *= k;
	T -= k;
	k = z * dev_options.inv_grid[2];
	k = sin(k);
	k *= dev_options.klength[2];
	k *= k;
	T -= k;
	T *= dev_options.timestepsize;
	kprop[index] = make_cuDoubleComplex(cos(T) * dev_options.fft_factor,
										sin(T) * dev_options.fft_factor);
}


	
__global__ void init_kprop_imag(cuDoubleComplex *kprop_imag)
{
	// the coordinates for this thread
	const unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	const unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	const unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = x*dev_options.grid[2]*dev_options.grid[1] + y*dev_options.grid[2] + z;
	
	// calculate the propagator
	double T = 0.0;
	double k = 0.0;
	
	k = x * dev_options.inv_grid[0];/////////////////
	k = sin(k);
	k *= dev_options.klength[0];
	k *= k;
	T -= k;
	k = y * dev_options.inv_grid[1];
	k = sin(k);
	k *= dev_options.klength[1];
	k *= k;
	T -= k;
	k = z * dev_options.inv_grid[2];
	k = sin(k);
	k *= dev_options.klength[2];
	k *= k;
	T -= k;
	T *= 0.015;
	kprop_imag[index] = make_cuDoubleComplex(exp(T)*dev_options.fft_factor,0);
}

////////////////////////////////////////////////////////////////////////////////
//! propagator in k-space
//! @param g_idata  input data in global memory
//! @param g_odata  output data in global memory
////////////////////////////////////////////////////////////////////////////////
__global__ void kpropagate(cuDoubleComplex *grid, cuDoubleComplex *kprop) /////////////////////////////////////////grid muss hier in kgrid sien!!!!!!////////////////////////////////
{
	// the coordinates for this thread
	//const unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	//const unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	//const unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;//x*dev_options.opt.grid[2]*dev_options.opt.grid[1] + y*dev_options.opt.grid[2] + z;
	
	// propagate this k-point
	cuDoubleComplex value = grid[index];
	cuDoubleComplex p = kprop[index];
	cuDoubleComplex result = cuCmul(p, value);
	grid[index] = result;
}

__global__ void kpropagate_imag(cuDoubleComplex *grid, cuDoubleComplex *kprop_imag) /////////////////////////////////////////grid muss hier in kgrid sien!!!!!!////////////////////////////////
{
	// the coordinates for this thread
	//const unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	//const unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	//const unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;//x*dev_options.opt.grid[2]*dev_options.opt.grid[1] + y*dev_options.opt.grid[2] + z;
	
	// propagate this k-point
	cuDoubleComplex value = grid[index];
	cuDoubleComplex p = kprop_imag[index];
	cuDoubleComplex result = cuCmul(p, value);
	grid[index] = result;
}

bool  Bh3CudaPropagator::Normfaktor()
{  
    double Result1=0.0;
    double Result2=0.0;


	   rgrid1[0] = *dev_rgrid1;
	   rgrid2[0] = *dev_rgrid2;

for(int x=0; x<options.grid[0] ; x++)
  {

   for(int y=0; y<options.grid[1] ; y++)
     {

      for(int z=0; z<options.grid[2] ; z++)
        {

	   Result1+=norm(rgrid1[0].at(x,y,z));
           Result2+=norm(rgrid2[0].at(x,y,z));

        }
    }
 }




for(int x=0; x<options.grid[0] ; x++)
  {

   for(int y=0; y<options.grid[1] ; y++)
     {

      for(int z=0; z<options.grid[2] ; z++)
        {

	  rgrid1[0].at(x,y,z)=rgrid1[0].at(x,y,z)*sqrt(options.N/Result1);
          rgrid2[0].at(x,y,z)=rgrid2[0].at(x,y,z)*sqrt(options.N/Result2);

        }
    }
 }


            *dev_rgrid1=rgrid1[0];
            *dev_rgrid2=rgrid2[0];

 return true;
}




bool Bh3CudaPropagator::noisefunction()
{
           *dev_rgrid1=rgrid1[0];
           *dev_rgrid2=rgrid2[0];

ComplexGrid::fft(rgrid1[0], kgrid1[0], true);
ComplexGrid::fft(rgrid2[0], kgrid2[0], true);

for(int x=0; x<options.grid[0] ; x++)
  {

   for(int y=0; y<options.grid[1] ; y++)
     {

      for(int z=0; z<options.grid[2] ; z++)
        {

	  kgrid1[0].at(x,y,z)=kgrid1[0].at(x,y,z)+noise();
          kgrid2[0].at(x,y,z)=kgrid2[0].at(x,y,z)+noise();

        }
    }
 }


ComplexGrid::fft(kgrid1[0], rgrid1[0], false);
ComplexGrid::fft(kgrid2[0], rgrid2[0], false);

        *dev_rgrid1=rgrid1[0];
        *dev_rgrid2=rgrid2[0];

 return true;
}




////////////////////////////////////////////////////////////////////////////////
//! propagator in r-space
//! @param g_idata  input data in global memory
//! @param g_odata  output data in global memory
////////////////////////////////////////////////////////////////////////////////
__global__ void rpropagate(cuDoubleComplex *grid1, cuDoubleComplex *grid2, double devOmega_t) 
{

  // std::cout<<"Warning rpropagate mix is not finished for complex U and the imag time Propagation!!!!!!!!!!!"<<std::endl;

	// the coordinates for this thread
	//const unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	//const unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	//const unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;//x*dev_options.opt.grid[2]*dev_options.opt.grid[1] + y*dev_options.opt.grid[2] + z;
	
	// calculate the propagator and propagate
	cuDoubleComplex value1 = grid1[index];
	cuDoubleComplex value2 = grid2[index];
	
   double sum = -dev_options.timestepsize*0.5*dev_options.U*((value1.x*value1.x + value1.y*value1.y) + dev_options.u_12*(value2.x*value2.x + value2.y*value2.y) + dev_options.u_12*(value1.x*value1.x + value1.y*value1.y) + dev_options.u_22*(value2.x*value2.x + value2.y*value2.y));
   double diff = dev_options.U*((value1.x*value1.x + value1.y*value1.y) + dev_options.u_12*(value2.x*value2.x + value2.y*value2.y) - dev_options.u_12*(value1.x*value1.x + value1.y*value1.y) - dev_options.u_22*(value2.x*value2.x + value2.y*value2.y));
	double delta = diff*diff + 4*devOmega_t*devOmega_t;
	delta = sqrt(delta);


	cuDoubleComplex prop;
	sincos(-0.5*dev_options.timestepsize*delta, &prop.y, &prop.x);
	
	cuDoubleComplex result1;
	cuDoubleComplex result2;
	result1 = make_cuDoubleComplex(delta*prop.x, diff*prop.y);
	result2 = make_cuDoubleComplex(0.0, -2.0*devOmega_t*prop.y);
	
	cuDoubleComplex final2 = cuCadd(cuCmul(result1, value1), cuCmul(result2, value2));
	
	result1 = make_cuDoubleComplex(delta*prop.x, -diff*prop.y);
	
	cuDoubleComplex final1 = cuCadd(cuCmul(result2, value1), cuCmul(result1, value2));
	
	sincos(sum, &prop.y, &prop.x);
	result1.x = prop.x/delta;
	result1.y = prop.y/delta;
	
	grid1[index] = cuCmul(result1, final1);
	grid2[index] = cuCmul(result1, final2);
}

__global__ void rpropagate_no_mix(cuDoubleComplex *grid1, cuDoubleComplex *grid2) //=================================================geanuer erklaerung von de_grid, grid1...//
{

	// the coordinates for this thread
	//unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	//unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	//unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;
	//const unsigned int index = x*dev_options.grid[2]*dev_options.grid[1] + y*dev_options.grid[2] + z;
	
	// calculate the propagator and propagate
	cuDoubleComplex value1 = grid1[index];
	cuDoubleComplex value2 = grid2[index];
         
	
	
	double a = -dev_options.timestepsize*dev_options.U*((value1.x*value1.x + value1.y*value1.y) + dev_options.u_12*(value2.x*value2.x + value2.y*value2.y));
	double b = -dev_options.timestepsize*dev_options.U*(dev_options.u_12*(value1.x*value1.x + value1.y*value1.y) + dev_options.u_22*(value2.x*value2.x + value2.y*value2.y));
	
	cuDoubleComplex prop;
	
	sincos(a, &prop.y, &prop.x);//============================================haben x und y hier nichts mit dem index zu tun===========wie mit referenz=====??=========================//
	cuDoubleComplex result1 = cuCmul(prop, value1);
	sincos(b, &prop.y, &prop.x);
	cuDoubleComplex result2 = cuCmul(prop, value2);
	
	grid1[index] = result1;
	grid2[index] = result2; //==================================================================================================wohin woher genauer nachschauen????=================//
      
}


__global__ void rpropagate_no_mix_imag(cuDoubleComplex *grid1, cuDoubleComplex *grid2) //===================================================================geanuer erklaerung von de_grid, grid1...//
{

	// the coordinates for this thread
	//unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	//unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	//unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;
	//const unsigned int index = x*dev_options.grid[2]*dev_options.grid[1] + y*dev_options.grid[2] + z;
	
	// calculate the propagator and propagate
	cuDoubleComplex value1 = grid1[index];
	cuDoubleComplex value2 = grid2[index];
	
	double a = -0.015*dev_options.U*((value1.x*value1.x + value1.y*value1.y) + dev_options.u_12*(value2.x*value2.x + value2.y*value2.y));
	double b = -0.015*dev_options.U*(dev_options.u_12*(value1.x*value1.x + value1.y*value1.y) + dev_options.u_22*(value2.x*value2.x + value2.y*value2.y));
	
	cuDoubleComplex prop;
	
	prop.x=exp(a);//============================================haben x und y hier nichts mit dem index zu tun===========wie mit referenz=====??=========================//
	prop.y=0.0;
	cuDoubleComplex result1 = cuCmul(prop, value1);
	

        prop.x=exp(b);
	prop.y=0.0;

	cuDoubleComplex result2 = cuCmul(prop, value2);
	
	grid1[index] = result1;
	grid2[index] = result2; //==================================================================================================wohin woher genauer nachschauen????=================//
}


// Bh3CudaPropagator - Class implementation



Bh3CudaPropagator::Bh3CudaPropagator(const PathOptions &opt, const ComplexGrid &start1, const ComplexGrid &start2, const double t_q) :
			Bh3Propagator(opt, start1, start2)
{
	if(((opt.grid[0] > BLOCK_SIZE) && (opt.grid[0] % BLOCK_SIZE != 0)) ||
		((opt.grid[1] > BLOCK_SIZE) && (opt.grid[1] % BLOCK_SIZE != 0)) ||
		((opt.grid[2] > BLOCK_SIZE) && (opt.grid[2] % BLOCK_SIZE != 0)))
	{
		cout << "Warning: invalid grid-sizes: Must be <= BLOCK_SIZE or multiple of BLOCK_SIZE=" << BLOCK_SIZE  << " !" << endl;
	}
	
	dev_rgrid1 = new CudaComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2]);
	dev_kgrid1 = new CudaComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2]);
	dev_rgrid2 = new CudaComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2]);
	dev_kgrid2 = new CudaComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2]);
	dev_kprop = new CudaComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2]);
	dev_kprop_imag = new CudaComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2]);
	
	dim3 dimBlock(options.grid[0] > BLOCK_SIZE ? BLOCK_SIZE : options.grid[0],
				  options.grid[1] > BLOCK_SIZE ? BLOCK_SIZE : options.grid[1],
				  options.grid[2] > BLOCK_SIZE ? BLOCK_SIZE : options.grid[2]);
	dim3 dimGrid(options.grid[0] > BLOCK_SIZE ? options.grid[0]/BLOCK_SIZE : 1,
			   options.grid[1] > BLOCK_SIZE ? options.grid[1]/BLOCK_SIZE : 1,
			   options.grid[2] > BLOCK_SIZE ? options.grid[2]/BLOCK_SIZE : 1);
	
	double fft_factor = 1.0 / (double) (options.grid[0]*options.grid[1]*options.grid[2]);
	
	//initialize path options on constant device memory
	ConstKernelOptions o;
	
	o.fft_factor = fft_factor;//=======================================================================================================was genau ist der fft faktor=========================
	o.timestepsize = options.timestepsize;
	o.U = options.U;
	o.u_12 = options.u_12;
	o.u_22 = options.u_22;
	o.omega_l = options.omega_l;
	for(int i = 0; i < 3; i++)//====================================================================================================hier fuellen mit komischen Index========================
	{
		o.inv_grid[i] = M_PI / (double) options.grid[i];
		o.grid[i] = options.grid[i];
		o.klength[i] = options.klength[i];
	}
	
	memcpy_host_to_symbol("dev_options", &o, 
							sizeof(ConstKernelOptions), 0);


	
	//initialize control parameter Omega to 2*Omega_cr; set Omega = 0 for a run without annealing
		tau_q = t_q;
		Omega_cr = 2*options.N*fft_factor*options.U*(options.u_12*options.u_12 - options.u_22)/(1.0 + options.u_22 + 2*options.u_12);
	if (Omega_cr < 0)
		Omega_cr = - Omega_cr;

	Omega_t = 2*Omega_cr;
	//cout << Omega_cr << "\t" << Omega_t << endl;
							
	
	//initialize time propagation
  	     init_kprop<<<dimGrid, dimBlock>>>(dev_kprop->getDevicePointer());
	     init_kprop_imag<<<dimGrid, dimBlock>>>(dev_kprop_imag->getDevicePointer());
	

	    
	*dev_rgrid1 = rgrid1[0];
	*dev_rgrid2 = rgrid2[0];
	
	//	imag_Time_Prop();
         

}

Bh3CudaPropagator::~Bh3CudaPropagator()
{
	delete dev_rgrid1;
	delete dev_kgrid1;
	delete dev_rgrid2;
	delete dev_kgrid2;
	delete dev_kprop;
        delete dev_kprop_imag;
}

bool Bh3CudaPropagator::propagate1()
{


	dim3 dimBlock(BLOCK_1D_LENGTH, 1, 1);
	dim3 dimGrid(options.grid[0]*options.grid[1]*options.grid[2] / BLOCK_1D_LENGTH, 1, 1);
	

//##################################### K propagation #######################################################################
	thread_synchronize("before r1->k1");
	if(!CudaComplexGrid::fft(*dev_rgrid1, *dev_kgrid1, CUFFT_FORWARD))
		return false;
	thread_synchronize("after r1->k1");
	if(!CudaComplexGrid::fft(*dev_rgrid2, *dev_kgrid2, CUFFT_FORWARD))
		return false;
	thread_synchronize("after r2->k2");
	kpropagate<<<dimGrid,dimBlock>>>(dev_kgrid1->getDevicePointer(), dev_kprop->getDevicePointer());
	kpropagate<<<dimGrid,dimBlock>>>(dev_kgrid2->getDevicePointer(), dev_kprop->getDevicePointer());

//####################################### R propagation #####################################################################
	thread_synchronize("after kprop");
	if(!CudaComplexGrid::fft(*dev_kgrid1, *dev_rgrid1, CUFFT_INVERSE))//====================================================wird hier in mehrere threads gegangen??===================//
		return false;
	thread_synchronize("after k1->r1");// ===================================================================warum dann threadsynchronize??==========================================//
	if(!CudaComplexGrid::fft(*dev_kgrid2, *dev_rgrid2, CUFFT_INVERSE))
		return false;
	thread_synchronize("after k2->r2");
	if(Omega_t == 0.0)
		rpropagate_no_mix<<<dimGrid,dimBlock>>>(dev_rgrid1->getDevicePointer(),dev_rgrid2->getDevicePointer());
	else
	{	
		rpropagate<<<dimGrid,dimBlock>>>(dev_rgrid1->getDevicePointer(),dev_rgrid2->getDevicePointer(), Omega_t);
	}
	thread_synchronize("after rprop");
	
	

//###################################### annealing of Omega #################################################################	
	if (Omega_t > 0.0 && annealing)
	{  
		Omega_t -= 2*Omega_cr*options.timestepsize/tau_q;
		if (Omega_t < 0.0)
			Omega_t = 0.0;
		
		//memcpy_host_to_symbol("devOmega_t", &Omega_t, sizeof(double), 0);
		//cout << Omega_t << endl;
	} // use this for linear ramping*/
	
	/*if (Omega_t > 0.0 && annealing)
	{  
		Omega_t *= (1.0 - options.timestepsize/tau_q);
		
		//memcpy_host_to_symbol("devOmega_t", &Omega_t, sizeof(double), 0);
		//cout << Omega_t << endl;
	} // use this for exponential ramping*/
	
	return true;	
}	






bool Bh3CudaPropagator::imag_Time_Prop()
{
  for(int i=0;i<100 /*5000*/;i++)
    {
	dim3 dimBlock(BLOCK_1D_LENGTH, 1, 1);
	dim3 dimGrid(options.grid[0]*options.grid[1]*options.grid[2] / BLOCK_1D_LENGTH, 1, 1);
	

//##################################### K propagation #######################################################################
	thread_synchronize("before r1->k1");

	if(!CudaComplexGrid::fft(*dev_rgrid1, *dev_kgrid1, CUFFT_FORWARD))
		return false;
	thread_synchronize("after r1->k1");

	if(!CudaComplexGrid::fft(*dev_rgrid2, *dev_kgrid2, CUFFT_FORWARD))
		return false;
	thread_synchronize("after r2->k2");

	kpropagate_imag<<<dimGrid,dimBlock>>>(dev_kgrid1->getDevicePointer(), dev_kprop_imag->getDevicePointer());
	kpropagate_imag<<<dimGrid,dimBlock>>>(dev_kgrid2->getDevicePointer(), dev_kprop_imag->getDevicePointer());

//####################################### R propagation #####################################################################
	thread_synchronize("after kprop");
	if(!CudaComplexGrid::fft(*dev_kgrid1, *dev_rgrid1, CUFFT_INVERSE))
		return false;
	thread_synchronize("after k1->r1");

	if(!CudaComplexGrid::fft(*dev_kgrid2, *dev_rgrid2, CUFFT_INVERSE))
		return false;
	thread_synchronize("after k2->r2");

	rpropagate_no_mix_imag<<<dimGrid,dimBlock>>>(dev_rgrid1->getDevicePointer(),dev_rgrid2->getDevicePointer());

	thread_synchronize("after rprop");

      Normfaktor();
    }
  
  noisefunction();
 
return true;
 
}

bool Bh3CudaPropagator::propagateN(int N)
{

  
  //std::cout<<"fehler"<<std::endl;
 
	int steps = N;
	for(int n = delta_N.size() - 1; n >= 0; n--)
	  {
		steps -= delta_N[n];
		if(steps < 0)
			steps = 0;
		for(int i = 0; i < steps; i++)
		{
			if(!propagate1())
				return false;
                         	
		}
		
		rgrid1[n] = *dev_rgrid1;//===============================================koennte auch weg?? wird ueber referenzen und gleich setzen soweiso erzwungen??==============//
		kgrid1[n] = *dev_kgrid1;
		rgrid2[n] = *dev_rgrid2;
	       	kgrid2[n] = *dev_kgrid2;
		
		N = steps = N - steps;
	
	}

       

 	return true;
}

