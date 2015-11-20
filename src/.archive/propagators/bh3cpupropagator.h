#ifndef BH2CPUPROPAGATOR_H__
#define BH2CPUPROPAGATOR_H__

#include <math.h>
#include <complex>
#include "bh3propagator.h"

class Bh3CPUPropagator : public Bh3Propagator {
	public:
		Bh3CPUPropagator(const PathOptions &opt, const ComplexGrid &start);
		virtual ~Bh3CPUPropagator();
	protected:
		bool propagate1();
		double bench_fft;
		double bench_kprop;
		double bench_rprop;
		double bench_fft_sys;
		double bench_kprop_sys;
		double bench_rprop_sys;
		ComplexGrid *kprop;
};

#endif
 