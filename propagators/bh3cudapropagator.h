#ifndef BH3CUDAPROPAGATOR_H__
#define BH3CUDAPROPAGATOR_H__

#include <curand.h>

#include "cudacomplexgrid.h"
#include "cudarealgrid.h"
#include "bh3propagator.h"


class Bh3CudaPropagator : public Bh3Propagator {
    
  public:
    enum runmode {imag, normal, diss, drivdiss};
     
  protected:
    CudaComplexGrid *dev_rgrid;
    CudaComplexGrid *rand_grid;
    CudaComplexGrid *dev_kprop;
    CudaRealGrid *dev_kprop_imag;
    curandGenerator_t gen;

		
  public:
    Bh3CudaPropagator(const PathOptions &opt, const ComplexGrid &start, const runmode rm);
    virtual ~Bh3CudaPropagator();
    
    bool renoise();
    bool renoise_phase();
    
  protected:
    runmode mode;
    bool propagateN(int N);
    bool propagate1();
};



#endif
 
