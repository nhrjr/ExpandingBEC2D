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
#include <cuComplex.h>  
#include <cuda_runtime.h>
#include <time.h>

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/transform_reduce.h>
#include <thrust/functional.h>

#include "bh3cudapropagator.h"
#include "complexgrid.h"
#include "wrapped_cuda_functions.h"
#include "gauss_random.h"

#define BLOCK_SIZE 8
#define KBLOCK_1D_LENGTH 512
#define RBLOCK_1D_LENGTH 256
#define GAMMA 1.

using namespace std;

typedef struct {
	double inv_grid[3];
	double klength[3];
	double grid[3];
    double fft_factor, timestepsize;
	double U;
    double U_imag;
    double N;
} ConstKernelOptions;

__constant__ __device__ ConstKernelOptions dev_options;

template <typename T>
struct square
{
    __host__ __device__
        T operator()(const T& x) const { 
            return x * x;
        }
};
	
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
       
	
	k = x * dev_options.inv_grid[0];
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

__global__ void init_kprop_drivdiss(cuDoubleComplex *kprop)
{
	// the coordinates for this thread
	const unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	const unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	const unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = x*dev_options.grid[2]*dev_options.grid[1] + y*dev_options.grid[2] + z;
	
	// calculate the propagator
	double T = 0.0;
	double k = 0.0;
       
	
	k = x * dev_options.inv_grid[0];
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

    k = exp(T*dev_options.U_imag/dev_options.U);

    cuDoubleComplex prop;
    sincos(T, &prop.y, &prop.x);
    
    prop.x *= k;
    prop.y *= k;
    prop.x *= dev_options.fft_factor;
    prop.y *= dev_options.fft_factor;
    
    kprop[index] = prop;
}

__global__ void imprint_corr(cuDoubleComplex *rand)
{
	// the coordinates for this thread
	const unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	const unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	const unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = x*dev_options.grid[2]*dev_options.grid[1] + y*dev_options.grid[2] + z;
	
	// calculate the propagator
	double k = 0.0;
       
    cuDoubleComplex r = rand[index];
    	
	k = x * dev_options.inv_grid[0];
	k = sin(k);
	k *= dev_options.klength[0];
	k *= k;
    k = y * dev_options.inv_grid[1];
	k = sin(k);
	k *= dev_options.klength[1];
	k *= k;
    k = z * dev_options.inv_grid[2];
	k = sin(k);
	k *= dev_options.klength[2];
	k *= k;
    
    k += 0.0001*dev_options.N*dev_options.fft_factor*dev_options.U;
    
    k /= 0.0001*dev_options.N*dev_options.fft_factor*dev_options.U;
    
    k = sqrt(k);
            
    r.x /= k;
    r.y /= k;

    k = sqrt(dev_options.fft_factor);

    r.x /= k;
    r.y /= k;
        
    rand[index] = r;
}

__global__ void init_kprop_imag(double *kprop_imag)
{
	// the coordinates for this thread
	const unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	const unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	const unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = x*dev_options.grid[2]*dev_options.grid[1] + y*dev_options.grid[2] + z;
	
	// calculate the propagator
	double T = 0.0;
	double k = 0.0;
	
	k = x * dev_options.inv_grid[0];
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
    k = exp(T);
    k *= dev_options.fft_factor;
    kprop_imag[index] = k;
}

__global__ void kpropagate(cuDoubleComplex *grid, cuDoubleComplex *kprop) 
{
	// the coordinates for this thread
	//const unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	//const unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	//const unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;
	
    cuDoubleComplex value = grid[index];
	cuDoubleComplex p = kprop[index];
	cuDoubleComplex result = cuCmul(p, value);
	grid[index] = result;
}

__global__ void kpropagate_drivdiss(cuDoubleComplex *grid, cuDoubleComplex *kprop, cuDoubleComplex *rand) 
{
	// the coordinates for this thread
	//const unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	//const unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	//const unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;
	
    cuDoubleComplex value = grid[index];
    cuDoubleComplex r = rand[index];
    cuDoubleComplex p = kprop[index];

    r.x *= dev_options.timestepsize;
    r.y *= dev_options.timestepsize;

    value.x += r.x;
    value.y += r.y;
    
	cuDoubleComplex result = cuCmul(p, value);
    
	grid[index] = result;
}

__global__ void kpropagate_imag(cuDoubleComplex *grid, double *kprop_imag, double scale) 
{
	// the coordinates for this thread
	//const unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	//const unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	//const unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;
	
	// propagate this k-point
	cuDoubleComplex value = grid[index];
	double p = kprop_imag[index];

    value.x *= scale;
    value.y *= scale;
        
    value.x *= p;
    value.y *= p;
    
	grid[index] = value;
}

__global__ void rpropagate(cuDoubleComplex *grid) 
{

	// the coordinates for this thread
	//unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	//unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	//unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;
		
	// calculate the propagator and propagate
	cuDoubleComplex value = grid[index];
		
	double a = -dev_options.timestepsize*dev_options.U*(value.x*value.x + value.y*value.y - dev_options.N*dev_options.fft_factor);
    
	cuDoubleComplex prop;
 
	sincos(a, &prop.y, &prop.x);
        
	cuDoubleComplex result = cuCmul(prop, value);
    	
	grid[index] = result;
}


__global__ void rpropagate_diss(cuDoubleComplex *grid) 
{

	// the coordinates for this thread
	//unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	//unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	//unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;
		
	// calculate the propagator and propagate
	cuDoubleComplex value = grid[index];
		
	double a = -dev_options.timestepsize*dev_options.U*(value.x*value.x + value.y*value.y);
    
	cuDoubleComplex prop;
 
	sincos(a, &prop.y, &prop.x);
        
	cuDoubleComplex result = cuCmul(prop, value);
    	
    a = -dev_options.timestepsize*dev_options.U_imag*(value.x*value.x + value.y*value.y);
    prop.x = exp(a);
    
    result.x *= prop.x;
    result.y *= prop.x;
    
	grid[index] = result;
}

__global__ void rpropagate_drivdiss(cuDoubleComplex *grid) 
{

	// the coordinates for this thread
	//unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	//unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	//unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;
		
	// calculate the propagator and propagate
	cuDoubleComplex value = grid[index];
    
    double a = -dev_options.timestepsize*dev_options.U*(value.x*value.x + value.y*value.y - dev_options.N*dev_options.fft_factor);
    
	cuDoubleComplex prop;
 
	sincos(a, &prop.y, &prop.x);
        
	cuDoubleComplex result = cuCmul(prop, value);
    	
    a = -dev_options.timestepsize*dev_options.U_imag*(value.x*value.x + value.y*value.y - dev_options.N*dev_options.fft_factor);
    prop.x = exp(a);
    
    result.x *= prop.x;
    result.y *= prop.x;
    
	grid[index] = result;
}

__global__ void rpropagate_imag(cuDoubleComplex *grid) 
{

	// the coordinates for this thread
	//unsigned int x = blockDim.x * blockIdx.x + threadIdx.x;
	//unsigned int y = blockDim.y * blockIdx.y + threadIdx.y;
	//unsigned int z = blockDim.z * blockIdx.z + threadIdx.z;
	const unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;
	
	// calculate the propagator and propagate
	cuDoubleComplex value = grid[index];
        
	double a = -dev_options.timestepsize*dev_options.U*(value.x*value.x + value.y*value.y);
	
	double prop;
	
	prop = exp(a*GAMMA);

    value.x *= prop;
    value.y *= prop;
    
    grid[index] = value;
}

// Bh3CudaPropagator - Class implementation

Bh3CudaPropagator::Bh3CudaPropagator(const PathOptions &opt, const ComplexGrid &start, const runmode rm) :
			Bh3Propagator(opt, start)
{
    mode = rm;
  
    if(mode == drivdiss)
    {
#ifdef CULEGACY
        curandCreateGenerator(&gen, CURAND_RNG_PSEUDO_XORWOW);
#else
        curandCreateGenerator(&gen, CURAND_RNG_PSEUDO_MTGP32);
#endif
        curandSetPseudoRandomGeneratorSeed(gen, time(NULL));
    }
    
	if(((opt.grid[1] > BLOCK_SIZE) && (opt.grid[2] % BLOCK_SIZE != 0)) ||
		((opt.grid[1] > BLOCK_SIZE) && (opt.grid[2] % BLOCK_SIZE != 0)) ||
		((opt.grid[3] > BLOCK_SIZE) && (opt.grid[3] % BLOCK_SIZE != 0)))
	{
		cout << "Warning: invalid grid-sizes: Must be <= BLOCK_SIZE or multiple of BLOCK_SIZE=" << BLOCK_SIZE  << " !" << endl;
	}

    if(opt.grid[0] > 1)
    {
        cout << "Warning: internal grid dimension > 1. I hope you know what you do." << endl;
    }
    	
	dev_rgrid = new CudaComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2], opt.grid[3]);

    if(mode == normal || mode == diss)
    {
        dev_kprop = new CudaComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2], opt.grid[3]);
        dev_kprop_imag = NULL;
        rand_grid = NULL;
    }
    else if(mode == drivdiss)
    {
        dev_kprop = new CudaComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2], opt.grid[3]);
        rand_grid = new CudaComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2], opt.grid[3]);
        dev_kprop_imag = NULL;
    }
    else if (mode == imag)
    {
        dev_kprop_imag = new CudaRealGrid(opt.grid[0], opt.grid[1], opt.grid[2], opt.grid[3]);
        dev_kprop = NULL;
        rand_grid = NULL;
    }
    else
        cout << "runmode not implemeted" << endl;
    
    
	dim3 dimBlock(options.grid[1] > BLOCK_SIZE ? BLOCK_SIZE : options.grid[1],
				  options.grid[2] > BLOCK_SIZE ? BLOCK_SIZE : options.grid[2],
				  options.grid[3] > BLOCK_SIZE ? BLOCK_SIZE : options.grid[3]);
	dim3 dimGrid(options.grid[1] > BLOCK_SIZE ? options.grid[1]/BLOCK_SIZE : 1,
			   options.grid[2] > BLOCK_SIZE ? options.grid[2]/BLOCK_SIZE : 1,
			   options.grid[3] > BLOCK_SIZE ? options.grid[3]/BLOCK_SIZE : 1);
	

	double fft_factor = 1.0 / (double) (options.grid[1]*options.grid[2]*options.grid[3]);

	//initialize path options on constant device memory
	ConstKernelOptions o;
		
	o.fft_factor = fft_factor;//=======================================================================================================was genau ist der fft faktor=========================
	o.N = (double) options.N;
	o.timestepsize = options.timestepsize;
	o.U = options.U;
    o.U_imag = options.U*options.g[0];
    for(int i = 0; i < 3; i++)//====================================================================================================hier fuellen mit komischen Index========================
	{
		o.inv_grid[i] = M_PI / (double) options.grid[i+1];
		o.grid[i] = options.grid[i+1];
		o.klength[i] = options.klength[i];
	}
	
	memcpy_host_to_symbol("dev_options", &o, 
							sizeof(ConstKernelOptions), 0);

	//initialize time propagation
    if (mode == imag)
    {
        init_kprop_imag<<<dimGrid, dimBlock>>>((double *)dev_kprop_imag->getDevicePointer());
    }
    else if (mode == drivdiss)
    {
        init_kprop_drivdiss<<<dimGrid, dimBlock>>>(dev_kprop->getDevicePointer());
    }
    else
    {
        init_kprop<<<dimGrid, dimBlock>>>(dev_kprop->getDevicePointer());
    }
    
    	    
	*dev_rgrid = rgrid[0];
}

Bh3CudaPropagator::~Bh3CudaPropagator()
{
    if(dev_rgrid)
        delete dev_rgrid;
    if(dev_kprop)
        delete dev_kprop;
    if(dev_kprop_imag)
        delete dev_kprop_imag;
    if(rand_grid)
        delete rand_grid;
    if(mode == drivdiss)
        curandDestroyGenerator(gen);
}




bool Bh3CudaPropagator::propagate1()
{
	dim3 dimRBlock(RBLOCK_1D_LENGTH, 1, 1);
	dim3 dimRGrid(options.grid[1]*options.grid[2]*options.grid[3] / RBLOCK_1D_LENGTH, 1, 1);

	dim3 dimKBlock(KBLOCK_1D_LENGTH, 1, 1);
	dim3 dimKGrid(options.grid[1]*options.grid[2]*options.grid[3] / KBLOCK_1D_LENGTH, 1, 1);
	


    if(mode == normal)
    {
//##################################### K propagation #######################################################################
        thread_synchronize("before r->k");
        if(!CudaComplexGrid::fft(*dev_rgrid, *dev_rgrid, CUFFT_FORWARD))
            return false;
        thread_synchronize("after r->k");

     	kpropagate<<<dimKGrid,dimKBlock>>>(dev_rgrid->getDevicePointer(), dev_kprop->getDevicePointer());

//####################################### R propagation #####################################################################
        thread_synchronize("after kprop");
        if(!CudaComplexGrid::fft(*dev_rgrid, *dev_rgrid, CUFFT_INVERSE))//====================================================wird hier in mehrere threads gegangen?? Ist ein Aufruf einer CUDA Methode, also ja===================//
            return false;

        thread_synchronize("after k->r");

        rpropagate<<<dimRGrid,dimRBlock>>>(dev_rgrid->getDevicePointer());
    }
    else if(mode == diss)
    {
//##################################### K propagation #######################################################################
        thread_synchronize("before r->k");
        if(!CudaComplexGrid::fft(*dev_rgrid, *dev_rgrid, CUFFT_FORWARD))
            return false;
        thread_synchronize("after r->k");


     	kpropagate<<<dimKGrid,dimKBlock>>>(dev_rgrid->getDevicePointer(), dev_kprop->getDevicePointer());

//####################################### R propagation #####################################################################
        thread_synchronize("after kprop");
        if(!CudaComplexGrid::fft(*dev_rgrid, *dev_rgrid, CUFFT_INVERSE))
            return false;

        thread_synchronize("after k->r");

        rpropagate_diss<<<dimRGrid,dimRBlock>>>(dev_rgrid->getDevicePointer());
    }
    else if(mode == drivdiss)
    {
//##################################### K propagation #######################################################################
        curandGenerateNormalDouble(gen, (double *)rand_grid->getDevicePointer(), 2*options.grid[1]*options.grid[2]*options.grid[3], 0., sqrt(GAMMA));

        thread_synchronize("before r->k");

        dim3 dimBlock_K (options.grid[1] > BLOCK_SIZE ? BLOCK_SIZE : options.grid[1],
                      options.grid[2] > BLOCK_SIZE ? BLOCK_SIZE : options.grid[2],
                      options.grid[3] > BLOCK_SIZE ? BLOCK_SIZE : options.grid[3]);
        dim3 dimGrid_K (options.grid[1] > BLOCK_SIZE ? options.grid[1]/BLOCK_SIZE : 1,
                     options.grid[2] > BLOCK_SIZE ? options.grid[2]/BLOCK_SIZE : 1,
                     options.grid[3] > BLOCK_SIZE ? options.grid[3]/BLOCK_SIZE : 1);

        imprint_corr<<<dimGrid_K, dimBlock_K>>>(rand_grid->getDevicePointer());
        
        if(!CudaComplexGrid::fft(*dev_rgrid, *dev_rgrid, CUFFT_FORWARD))
            return false;

        thread_synchronize("after r->k");

     	kpropagate_drivdiss<<<dimKGrid,dimKBlock>>>(dev_rgrid->getDevicePointer(), dev_kprop->getDevicePointer(), rand_grid->getDevicePointer());

//####################################### R propagation #####################################################################
        thread_synchronize("after kprop");
        if(!CudaComplexGrid::fft(*dev_rgrid, *dev_rgrid, CUFFT_INVERSE))
            return false;

        thread_synchronize("after k->r");

        rpropagate_drivdiss<<<dimRGrid,dimRBlock>>>(dev_rgrid->getDevicePointer());
    }
    else if(mode == imag)
    {
        thread_synchronize("before norm computation");

        thrust::device_ptr<double> thrust_rgrid_ptr((double *)dev_rgrid->getDevicePointer());
        thrust::device_vector<double> thrust_rgrid(thrust_rgrid_ptr, thrust_rgrid_ptr + 2*options.grid[1]*options.grid[2]*options.grid[3]);
        
            // setup arguments
        square<double>        unary_op;
        thrust::plus<double> binary_op;
        double init = 0;

            // compute norm
        double scale =  (double)options.N/thrust::transform_reduce(thrust_rgrid.begin(), thrust_rgrid.end(), unary_op, init, binary_op);
                
//##################################### K propagation #######################################################################
        thread_synchronize("before r->k");
        if(!CudaComplexGrid::fft(*dev_rgrid, *dev_rgrid, CUFFT_FORWARD))
            return false;
        thread_synchronize("after r->k");

     	kpropagate_imag<<<dimKGrid,dimKBlock>>>(dev_rgrid->getDevicePointer(), (double *)dev_kprop_imag->getDevicePointer(), scale);

//####################################### R propagation #####################################################################
        thread_synchronize("after kprop");
        if(!CudaComplexGrid::fft(*dev_rgrid, *dev_rgrid, CUFFT_INVERSE))
            return false;

        thread_synchronize("after k->r");
        
        rpropagate_imag<<<dimRGrid,dimRBlock>>>(dev_rgrid->getDevicePointer());
    }
    
	thread_synchronize("after rprop");
	
    current_time += options.timestepsize;
    
	return true;	
}
//call to curand, for future use	
//curandGenerateNormalDouble(gen, (double *)randGrid->getDevicePointer(), 2*options.grid[1]*options.grid[2]*options.grid[3], 0., sqrt(GAMMA*0.5));

bool Bh3CudaPropagator::propagateN(int N)
{
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

        rgrid[n] = *dev_rgrid;

        if(mode == imag)
        {
            thread_synchronize("before norm computation");

            thrust::device_ptr<double> thrust_rgrid_ptr((double *)dev_rgrid->getDevicePointer());
            thrust::device_vector<double> thrust_rgrid(thrust_rgrid_ptr, thrust_rgrid_ptr + 2*options.grid[1]*options.grid[2]*options.grid[3]);
        
                // setup arguments
            square<double>        unary_op;
            thrust::plus<double> binary_op;
            double init = 0;

                // compute norm
            double scale =  (double)options.N/thrust::transform_reduce(thrust_rgrid.begin(), thrust_rgrid.end(), unary_op, init, binary_op);
		
            for(int i = 0; i < rgrid[n].int_dim(); i++)
            {
                
                for(int x = 0; x < rgrid[n].width(); x++)
                {
                    for(int y = 0; y < rgrid[n].height(); y++)
                    {
                        for(int z = 0; z < rgrid[n].depth(); z++)
                        {
                            rgrid[n](i,x,y,z) *= scale;
                        }
                    }
                } 
            }
        }
			
		N = steps = N - steps;
	}

 	return true;
}


bool Bh3CudaPropagator::renoise()
{
    GaussRandom r (get_seed());

    for (int n = 0; n < rgrid.size(); n++)
    {
        ComplexGrid::fft(rgrid[n], rgrid[n], true);
   
        for(int i = 0; i < rgrid[n].int_dim(); i++)
        {
            for(int x = 0; x < rgrid[n].width(); x++)
            {
                for(int y = 0; y < rgrid[n].height(); y++)
                {
                    for(int z = 0; z < rgrid[n].depth(); z++)
                    {
                        if (!(x==0 && y==0 && z==0))
                            rgrid[n](i,x,y,z) += r.gauss_random()*0.5;
                    }
                }
            }
        }
    

        ComplexGrid::fft(rgrid[n], rgrid[n], false);
    }
    
    return true;
}


bool Bh3CudaPropagator::renoise_phase()
{
    GaussRandom r (get_seed());

    ComplexGrid *phase = new ComplexGrid(1, options.grid[1], options.grid[2], options.grid[3]);

    for(int x = 0; x < options.grid[1]/2 ; x++)
    {
        for(int y = 0; y < options.grid[2]/2 ; y++)
        {
            for(int z = 0; z < options.grid[3] ; z++)
            {
                if(x*x+y*y <= 60000 /*sqrt(options.grid[1]*options.grid[1] + options.grid[2]*options.grid[2]))/2./2.*/ )
                {
                    phase->at(0,x,y,z) = r.gauss_random();
                    phase->at(0,options.grid[1]-1-x,y,z) = r.gauss_random();
                    phase->at(0,x,options.grid[2]-1-y,z) = r.gauss_random();
                    phase->at(0,options.grid[1]-1-x,options.grid[2]-1-y,z) = r.gauss_random();
                }
            }
        }
    }
    phase->at(0,0,0,0) = complex<double>(0.,0.);
        
    ComplexGrid::fft_unnormalized(*phase, *phase, false);
    
    for (int n = 0; n < rgrid.size(); n++)
    {
        for(int i = 0; i < rgrid[n].int_dim(); i++)
        {
            for(int x=0; x<options.grid[0] ; x++)
            {
                for(int y=0; y<options.grid[1] ; y++)
                {
                    for(int z=0; z<options.grid[2] ; z++)
                    {
                        rgrid[n](i,x,y,z) *= polar(1.,real(phase->at(0,x,y,z)));
                    }
                }
            }
        }
    }
    
    delete phase;
    
    return true;
}
