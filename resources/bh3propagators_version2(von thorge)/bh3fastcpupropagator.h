#ifndef BH2CPUPROPAGATOR_H__
#define BH2CPUPROPAGATOR_H__

#include <math.h>
#include <complex>
#include "bh3propagator.h"

class Bh3FastCPUPropagator : public Bh3Propagator {
	public:
		Bh3FastCPUPropagator(const PathOptions &opt, const ComplexGrid &start);
		virtual ~Bh3FastCPUPropagator();
	protected:
		bool propagate1();
		void kprop(ComplexGrid *g1, ComplexGrid *g2);
		double bench;
};

#endif
 
