#include "plot_with_mgl.h"

#define COLOURBAR_MAX_VALUE 10000
#define IMAGE_SIZE 2500

using namespace std;


void plotSpectrum(string name,string title, Observables &ares){

	ofstream plotfile;
    vector<double> kval;
	vector<double> numberval;
	
	// plotfile.open(("runData/" + name + ".dat").c_str(), ios::out | ios::trunc);
    for (int r = 0; r < ares.number.size(); r++)             
	{	
		if(ares.k(r) != 0.0){
			// plotfile << r <<"\t"<< ares.k(r) <<"\t" << ares.number(r) <<"\t";
			// plotfile << endl;
			// if(r%2 == 0){ // reduce the number of k's plotted, because it gets cluttered.
				kval.push_back(ares.k(r));
				numberval.push_back(ares.number(r));
			// }
        }
	}
	// plotfile << endl << endl;	
	// plotfile.close();


	int n = kval.size();//-1; // don't plot the zero mode! (why? because it looks like shit)
	mglData k(n);
	mglData number(n);
	// mglData healing_length(2);
	// healing_length.a[0] = ares.healing_length;
	// healing_length.a[1] = 10000000;

	// cout << "mgl Reached" << endl;


	for(int i = 0; i < n; i++){
		k.a[i] = kval[i];
		number.a[i] = numberval[i];
	}

	// cout << "copied" << endl;  

	mglGraph gr;

	gr.SetMarkSize(0.7);
	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
	gr.SetFontSize(3.0);
	gr.SetQuality(3);
	gr.Title(title.c_str());
	gr.SetRange('x',k);
	gr.SetRange('y',1.0e-20,1.0e7);
	gr.SetCoor(11); // log-log-coordinates

	// gr.SubPlot(2,1,0);
	// gr.Axis();
	// gr.Plot(k,number);
	// gr.SubPlot(2,1,1);

	gr.Axis();

	// gr.SetFunc("lg(x)","lg(y)");
	gr.FPlot("x^(-2)");
	// gr.FPlot("x^(-4.66)");
	gr.FPlot("x^(-5)");

	// gr.Stem(healing_length);
	gr.Plot(k,number," .");

	name = name + ".png";

	gr.WritePNG(name.c_str(),"Spectrum",false);
}

void plotRadialDensity(string name,string title, Observables &ares){

	ofstream plotfile;
    vector<double> rval;
	vector<double> densityval;
	
	// plotfile.open(("runData/" + name + ".dat").c_str(), ios::out | ios::trunc);
    for (int r = 0; r < ares.radialDensity.size(); r++){	
		if((ares.r(r) != 0.0) && (ares.radialDensity(r) != 0.0)){
			// plotfile << r <<"\t"<< ares.r(r) <<"\t" << ares.radialDensity(r) <<"\t";
			// plotfile << endl;
			// if(r%10 == 0){ // reduce the number of k's plotted, because it gets cluttered.

				rval.push_back(ares.r(r));
				densityval.push_back(ares.radialDensity(r));
			// }
        }
	}
	// plotfile << endl << endl;	
	// plotfile.close();


	int n = rval.size();//-1; // don't plot the zero mode! (why? because it looks like shit)
	mglData r(n);
	mglData density(n);
	// mglData healing_length(2);
	// healing_length.a[0] = ares.healing_length;
	// healing_length.a[1] = 10000000;

	// cout << "mgl Reached" << endl;


	for(int i = 0; i < n; i++){
		r.a[i] = rval[i];
		density.a[i] = densityval[i];
		// cerr << r.a[i] << " - " << densityval[i] << endl;
	}

	// cout << "copied" << endl;

	mglGraph gr;

	gr.SetMarkSize(0.7);
	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
	gr.SetFontSize(3.0);
	gr.SetQuality(3);
	gr.Title(title.c_str());
	gr.SetCoor(11); // log-log-coordinates
	gr.SetRange('x',1.0e-4,2.0e4);
	gr.SetRange('y',1.0e-10,100);
	

	// gr.SubPlot(2,1,0);
	// gr.Axis();
	// gr.Plot(k,number);
	// gr.SubPlot(2,1,1);

	gr.Axis();

	// gr.SetFunc("lg(x)","lg(y)");
	gr.FPlot("x^(-2)");
	// gr.FPlot("x^(-4.66)");
	// gr.FPlot("x^(-5)");

	// gr.Stem(healing_length);
	gr.Plot(r,density," .");

	name = name + ".png";

	gr.WritePNG(name.c_str(),"Radial Density",false);
}

// void plotPairDistance(string name,string title,list<VortexData> vlist){

// 	ofstream plotfile;
//     vector<double> histogram;
// 	vector<double> distance;
	
// 	plotfile.open(("runData/" + name + ".dat").c_str(), ios::out | ios::trunc);
//     for (int r = 0; r < pres.distance.size(); r++)             
// 	{	
// 		if(pres.histogram[r] != 0.0){
// 			plotfile << r <<"\t"<< pres.histogram[r] <<"\t" << pres.distance[r] <<"\t";
// 			plotfile << endl;

// 			histogram.push_back(pres.histogram[r]);
// 			distance.push_back(pres.distance[r]);
			
//         }
// 	}
// 	plotfile << endl << endl;	
// 	plotfile.close();


// 	int n = histogram.size();//-1; // don't plot the zero mode! (why? because it looks like shit)
// 	mglData m_histogram(n);
// 	mglData m_distance(n);


// 	for(int i = 0; i < n; i++){
// 		m_histogram.a[i] = histogram[i];
// 		m_distance.a[i] = distance[i];
// 		// cout << "Histogram: " << m_histogram.a[i] << endl;
// 		// cout << "Distance: " << m_distance.a[i] << endl;
// 	}

// 	// cout << "copied" << endl;

// 	mglGraph gr;

// 	double maxrange = 10 * sqrt(2); // This is bad, but I have no access to opt.min_x etc.

// 	gr.SetMarkSize(0.7);
// 	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
// 	gr.SetFontSize(3.0);
// 	gr.SetQuality(3);
// 	gr.Title(title.c_str());
// 	gr.SetRange('x',0.0,maxrange);
// 	gr.SetRange('y',0.0,2);
// 	// gr.SetCoor(11); // log-log-coordinates

// 	// gr.SubPlot(2,1,0);
// 	// gr.Axis();
// 	// gr.Plot(k,m_distance);
// 	// gr.SubPlot(2,1,1);

// 	gr.Axis();

// 	// gr.SetFunc("lg(x)","lg(y)");
// 	// gr.FPlot("x^(-2)");
// 	// gr.FPlot("x^(-4.666)");
// 	// gr.FPlot("x^(-5)");

// 	// gr.Stem(healing_length);
// 	gr.Plot(m_distance,m_histogram," .");

// 	name = name + ".png";

// 	gr.WritePNG(name.c_str(),"Spectrum",false);
// }

// void plotVortexList(string name,string title,const RealGrid &phase,list<VortexData> &vlist,Options &opt){

// 	int32_t factor = (opt.grid[1] > 2048) ? opt.grid[1]/2048 : 1;
// 	int n = opt.grid[1]/factor;
// 	int m = opt.grid[2]/factor;

// 	int size = vlist.size();

// 	mglData phaseData(n,m);
// 	mglData v_x(size);
// 	mglData v_y(size);

// 	int l = 0;
// 	for(list<VortexData>::const_iterator it = vlist.begin(); it != vlist.end(); ++it){
// 		v_x.a[l] = it->c.x()/factor;
// 		v_y.a[l] = it->c.y()/factor;
// 		l++;
// 	}

// 	int i,j,k;

// 	for(i=0;i<n;i++) for(j=0;j<m;j++)
// 	{	
// 		k = i+n*j;
// 		phaseData.a[k] = phase.at(0,factor*i,factor*j,0);
// 	}

// 	mglGraph gr;

// 	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
// 	gr.SetFontSize(3.0);
// 	gr.SetQuality(3);
// 	gr.Title(title.c_str());

// 	// gr.SetRange('x',-opt.min_x,opt.min_x);
// 	// gr.SetRange('y',-opt.min_y,opt.min_y);
// 	// gr.SetRange('z',phaseData);
// 	// gr.SetRange('c',phaseData);
// 	gr.SetRange('x',0,opt.grid[1]/factor);
// 	gr.SetRange('y',0,opt.grid[2]/factor);

// 	gr.Axis();
// 	gr.Colorbar();
// 	gr.Dens(phaseData);
// 	gr.Plot(v_x,v_y," #xw");

// 	name = name + ".png";

// 	gr.WritePNG(name.c_str(),"Vortices",false);
// }

void plotContour(string name,  MatrixXcd &data, std::unordered_set<Coordinate<int32_t>> &contour, Options &opt){

	int n = data.rows();
	int m = data.cols();
	int size = contour.size();

	mglData densData(n,m);
	mglData v_x(size);
	mglData v_y(size);

	int l = 0;
	for(std::unordered_set<Coordinate<int32_t>>::const_iterator it = contour.begin(); it != contour.end(); ++it){
		v_x.a[l] = it->x();
		v_y.a[l] = it->y();
		l++;
	}

	int i,j,k;

	for(i=0;i<n;i++) for(j=0;j<m;j++)
	{	
		k = i+n*j;
		densData.a[k] = abs2(data(i,j));
	}

	mglGraph gr;

	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
	gr.SetFontSize(3.0);
	gr.SetQuality(3);
	// gr.Title(title.c_str());

	// gr.SetRange('x',-opt.min_x,opt.min_x);
	// gr.SetRange('y',-opt.min_y,opt.min_y);
	// gr.SetRange('z',densData);
	gr.SetRange('c',densData);
	gr.SetRange('x',0,data.rows());
	gr.SetRange('y',0,data.rows());

	gr.Axis();
	gr.Colorbar();
	gr.Dens(densData);
	gr.Plot(v_x,v_y," .w");

	name = name + ".png";

	gr.WritePNG(name.c_str(),"Contour",false);

}

// void plotContourSurround(string name, RealGrid &Psi, std::unordered_set<Coordinate<int32_t>> &contour, Options &opt){

// 	int size = contour.size();

// 	mglData v_x(size);
// 	mglData v_y(size);

// 	int l = 0;
// 	int x_max = 0; 
// 	int y_max = 0;
// 	int x_min = opt.grid[1];
// 	int y_min = opt.grid[2];
// 	for(std::unordered_set<Coordinate<int32_t>>::const_iterator it = contour.begin(); it != contour.end(); ++it){
// 		v_x.a[l] = it->x();
// 		v_y.a[l] = it->y();
// 		l++;
// 		x_max = (it->x() > x_max) ? it->x() : x_max;
// 		y_max = (it->y() > y_max) ? it->y() : y_max;
// 		x_min = (it->x() < x_min) ? it->x() : x_min;
// 		y_min = (it->y() < y_min) ? it->y() : y_min;
// 	}

// 	// int x_max = opt.grid[1]-5;
// 	// int x_min = 5;
// 	// int y_max = opt.grid[2]-5;
// 	// int y_min = 5;
// 	vector<int> dens_x;
// 	vector<int> dens_y;

// 	for(int i = x_min-5; i < x_max+5; i++){
// 	 	for(int j = y_min-5; j < y_max+5; j++){	
// 			if(Psi(0,i,j,0) > 0){
// 				dens_x.push_back(i);
// 				dens_y.push_back(j);
// 			}
// 		}
// 	}

// 	mglData densX(dens_x.size());
// 	mglData densY(dens_y.size());
// 	for(int i = 0; i < dens_x.size(); i++){
// 		densX.a[i] = dens_x[i];
// 		densY.a[i] = dens_y[i];
// 		// cout << "Density: " << densX.a[i] << " " << densY.a[i] << endl;
// 	}

// 	mglGraph gr;

// 	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
// 	gr.SetFontSize(3.0);
// 	gr.SetQuality(3);
// 	gr.Title(name.c_str());

// 	// gr.SetRange('x',-opt.min_x,opt.min_x);
// 	// gr.SetRange('y',-opt.min_y,opt.min_y);
// 	// gr.SetRange('z',densData);
// 	// gr.SetRange('c',densData);
// 	gr.SetRange('x',x_min-5,x_max+5);
// 	gr.SetRange('y',y_min-5,y_max+5);

// 	gr.Axis();
// 	// gr.Colorbar();
// 	// gr.Plot(densData," ");
// 	gr.Plot(densX,densY," #.k");
// 	gr.Plot(v_x,v_y," #.r");

// 	name = name + ".png";

// 	gr.WritePNG(name.c_str(),"ExpandingVortexGas2D",false);

// }

void plotDataToPng(string filename,MatrixXi &g,Options &opt){


	int n = opt.grid[1];
	int m = opt.grid[2];

	// mglComplex data(n,m);
	mglData density(n,m);

	int i,j,k;

	// data.Create(n,m);

	// complex<double> data1;

	for(i=0;i<n;i++) for(j=0;j<m;j++)
	{	
		k = i+n*j;
		density.a[k] = g(i,j);
		// if(density.a[k] > 1){
		// 	cout << "PLOTTER" << density.a[k] << " " << i << " " << j << endl;
		// }

		// data.a[k] = abs2(g(0,i,j,0));
	}

	mglGraph gr;

		
		// gr.Light(0,true);
		// gr.Alpha(true);

	filename = filename + ".png";

	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
	gr.SetFontSize(3.0);
	gr.SetQuality(3);
	// gr.Title(title.c_str());
	// gr.Alpha(true);

	gr.SetRange('x',0,opt.grid[1]);
	gr.SetRange('y',0,opt.grid[2]);
	gr.SetRange('z',density);
	gr.SetRange('c',density);

	gr.SubPlot(2,2,1);

	// gr.Light(true);
	gr.Rotate(40,40);
	gr.Box();
	gr.Axis();

	gr.Surf(density);

	gr.SubPlot(2,2,3);
	gr.Axis();
	gr.Colorbar("_");
	gr.Dens(density);

	gr.WritePNG(filename.c_str(),"ExpandingVortexGas2D",false);
}

// void plotDataToPng(string filename,string title,ComplexGrid* &g,Options &opt)
// {
	

// 	int n = opt.grid[1];
// 	int m = opt.grid[2];

// 	// mglComplex data(n,m);
// 	mglData density(n,m);
// 	mglData phase(n,m);

// 	int i,j,k;

// 	// data.Create(n,m);

// 	// complex<double> data1;

// 	for(i=0;i<n;i++) for(j=0;j<m;j++)
// 	{	
// 		k = i+n*j;
// 		density.a[k] = abs2(g->at(0,i,j,0));
// 		phase.a[k] = arg(g->at(0,i,j,0));

// 		// data.a[k] = abs2(g(0,i,j,0));
// 	}

// 	mglGraph gr;

		
// 		// gr.Light(0,true);
// 		// gr.Alpha(true);

// 	filename = filename + ".png";

// 	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
// 	gr.SetFontSize(3.0);
// 	gr.SetQuality(3);
// 	gr.Title(title.c_str());
// 	// gr.Alpha(true);



// 	// data.use_abs=false;
// 	double xrange = opt.min_x*opt.stateInformation[0];
// 	double yrange = opt.min_y*opt.stateInformation[1];
// 	gr.SetRange('x',-xrange,xrange);
// 	gr.SetRange('y',-yrange,yrange);
// 	gr.SetRange('z',phase);
// 	gr.SetRange('c',phase);

// 	gr.SubPlot(2,2,0);

// 	gr.Rotate(40,40);
// 	gr.Box();
// 	gr.Axis();
// 	gr.Surf(phase);


// 	gr.SubPlot(2,2,2);
// 	gr.Axis();
// 	gr.Colorbar("_");
// 	gr.Dens(phase);


// 	// data.use_abs=true;
// 	gr.SetRange('x',-xrange,xrange);
// 	gr.SetRange('y',-yrange,yrange);
// 	gr.SetRange('z',density);
// 	gr.SetRange('c',density);

// 	gr.SubPlot(2,2,1);

// 	// gr.Light(true);
// 	gr.Rotate(40,40);
// 	gr.Box();
// 	gr.Axis();

// 	gr.Surf(density);

// 	gr.SubPlot(2,2,3);
// 	gr.Axis();
// 	gr.Colorbar("_");
// 	gr.Dens(density);

// 	gr.WritePNG(filename.c_str(),"ExpandingVortexGas2D",false);

// }

// void plotDataToPng(string filename,string title,ComplexGrid &g,Options &opt)
// {
	

// 	int n = opt.grid[1];
// 	int m = opt.grid[2];

// 	// mglComplex data(n,m);
// 	mglData density(n,m);
// 	mglData phase(n,m);

// 	int i,j,k;

// 	// data.Create(n,m);

// 	// complex<double> data1;

// 	for(i=0;i<n;i++) for(j=0;j<m;j++)
// 	{	
// 		k = i+n*j;
// 		density.a[k] = abs2(g(0,i,j,0)); // g(0,i,j,0).real(); // 
// 		phase.a[k] = arg(g(0,i,j,0)); // g(0,i,j,0).imag(); // 

// 		// data.a[k] = abs2(g(0,i,j,0));
// 	}

// 	mglGraph gr;

		
// 		// gr.Light(0,true);
// 		// gr.Alpha(true);

// 	filename = filename + ".png";

// 	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
// 	gr.SetFontSize(3.0);
// 	gr.SetQuality(3);
// 	gr.Title(title.c_str());
// 	// gr.Alpha(true);


// 	double xrange = opt.min_x*opt.stateInformation[0];
// 	double yrange = opt.min_y*opt.stateInformation[1];
// 	// data.use_abs=false;
// 	gr.SetRange('x',-xrange,xrange);
// 	gr.SetRange('y',-yrange,yrange);
// 	gr.SetRange('z',phase);
// 	gr.SetRange('c',phase);

// 	gr.SubPlot(2,2,0);

// 	gr.Rotate(40,40);
// 	gr.Box();
// 	gr.Axis();
// 	gr.Surf(phase);


// 	gr.SubPlot(2,2,2);
// 	gr.Axis();
// 	gr.Colorbar("_");
// 	gr.Dens(phase);


// 	// data.use_abs=true;
// 	gr.SetRange('x',-xrange,xrange);
// 	gr.SetRange('y',-yrange,yrange);
// 	gr.SetRange('z',density);
// 	gr.SetRange('c',density);

// 	gr.SubPlot(2,2,1);

// 	// gr.Light(true);
// 	gr.Rotate(40,40);
// 	gr.Box();
// 	gr.Axis();

// 	gr.Surf(density);

// 	gr.SubPlot(2,2,3);
// 	gr.Axis();
// 	gr.Colorbar("_");
// 	gr.Dens(density);

// 	gr.WritePNG(filename.c_str(),"ExpandingVortexGas2D",false);

// }

// void plotDataToPng(string filename,string title,RealGrid g,Options &opt){

// 	int n = opt.grid[1];
// 	int m = opt.grid[2];

// 	// mglComplex data(n,m);
// 	mglData grid(n,m);


// 	int i,j,k;

// 	// data.Create(n,m);

// 	// complex<double> data1;

// 	for(i=0;i<n;i++) for(j=0;j<m;j++)
// 	{	
// 		k = i+n*j;
// 		grid.a[k] = g(0,i,j,0);
// 		// data.a[k] = abs2(g(0,i,j,0));
// 	}

// 	mglGraph gr;

		
// 		// gr.Light(0,true);
// 		// gr.Alpha(true);

// 	filename = filename + ".png";

// 	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
// 	gr.SetFontSize(3.0);
// 	gr.SetQuality(3);
// 	gr.Title(title.c_str());
// 	// gr.Alpha(true);

// 	double xrange = opt.min_x*opt.stateInformation[0];
// 	double yrange = opt.min_y*opt.stateInformation[1];
// 	// data.use_abs=true;
// 	gr.SetRange('x',-xrange,xrange);
// 	gr.SetRange('y',-yrange,yrange);
// 	gr.SetRange('z',grid);
// 	gr.SetRange('c',grid);

// 	gr.Axis();
// 	gr.Colorbar("_");
// 	gr.Dens(grid);

// 	gr.WritePNG(filename.c_str(),"ExpandingVortexGas2D",false);

// }

// void plotDataToPngExpanding(string filename,string title,ComplexGrid &g,Options &opt)
// {
	
// 	int32_t factor = (opt.grid[1] > 2048) ? opt.grid[1]/2048 : 1;
// 	int n = opt.grid[1]/factor;
// 	int m = opt.grid[2]/factor;

// 	// mglComplex data(n,m);
// 	mglData density(n,m);
// 	mglData xaxis(n);
// 	mglData yaxis(m);
// 	// mglData phase(n,m);

// 	int i,j,k;

// 	// data.Create(n,m);

// 	// complex<double> data1;

// 	for(i=0;i<n;i++) for(j=0;j<m;j++)
// 	{	
// 		k = i+n*j;
// 		density.a[k] = abs2(g(0,factor*i,factor*j,0));
// 		// phase.a[k] = arg(g(0,i,j,0));

// 		// data.a[k] = abs2(g(0,i,j,0));
// 	}

// 	mglGraph gr;

		
// 		// gr.Light(0,true);
// 		// gr.Alpha(true);

// 	filename = filename + ".png";

// 	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
// 	gr.SetFontSize(3.0);
// 	gr.SetQuality(3);
// 	gr.Title(title.c_str());
// 	// gr.Alpha(true);


// 	double xrange = opt.min_x*opt.stateInformation[0];
// 	double yrange = opt.min_y*opt.stateInformation[1];

// 	double range = (xrange > yrange) ? xrange : yrange;

// 	for( i = 0; i < n; i++){
// 		xaxis.a[i] = -xrange + factor*i * 2 * xrange / opt.grid[1];
// 	}
// 	for( j = 0; j < m; j++){
// 		yaxis.a[j] = -yrange + factor*j * 2 * yrange / opt.grid[2];
// 	}
// 	// data.use_abs=false;
// 	gr.SetRange('x',-range,range);
// 	gr.SetRange('y',-range,range);
// 	gr.SetRange('z',density);
// 	gr.SetRange('c',density);

// 	// gr.SubPlot(2,2,0);

// 	// gr.Rotate(40,40);
// 	// gr.Box();
// 	// gr.Axis();
// 	// gr.Surf(phase);


// 	// gr.SubPlot(2,2,2);
// 	// gr.Axis();
// 	// gr.Colorbar("_");
// 	// gr.Dens(phase);


// 	// data.use_abs=true;
// 	// gr.SetRange('x',-xrange,xrange);
// 	// gr.SetRange('y',-yrange,yrange);
// 	// gr.SetRange('z',density);
// 	// gr.SetRange('c',density);

// 	// gr.SubPlot(2,2,1);

// 	// gr.Light(true);
// 	// gr.Rotate(40,40);
// 	// gr.Box();
// 	// gr.Axis();

// 	// gr.Surf(density);

// 	// gr.SubPlot(2,2,3);
// 	gr.Axis();
// 	gr.Colorbar("_");
// 	gr.Dens(xaxis,yaxis,density);

// 	gr.WritePNG(filename.c_str(),"ExpandingVortexGas2D",false);

// }

void plotDataToPngEigen(string filename, Eigen::MatrixXcd& wavefct,Options opt)
{

	int32_t factor = (wavefct.cols() > 2048) ? wavefct.cols()/2048 : 1;
	int n = wavefct.rows()/factor;
	int m = wavefct.cols()/factor;

	// mglComplex data(n,m);
	mglData density(n,m);
	mglData phase(n,m);

	int i,j,k;

	// data.Create(n,m);

	// complex<double> data1;

	for(i=0;i<n;i++) for(j=0;j<m;j++)
	{	
		k = i+n*j;
		// data1 = g->at(0,i,j,0);
		density.a[k] = abs2(wavefct(factor*i,factor*j));
		phase.a[k] = arg(wavefct(factor*i,factor*j));
	}
	// cout << "arrays have size: k = " <<  k << endl;

	mglGraph gr;

		
		// gr.Light(0,true);
		// gr.Alpha(true);

	filename = filename + ".png";

	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
	gr.SetFontSize(3.0);
	gr.SetQuality(3);
	// gr.Title(title.c_str());
	// gr.Alpha(true);


	double xrange = opt.min_x*opt.stateInformation[0];
	double yrange = opt.min_y*opt.stateInformation[1];
	// data.use_abs=false;
	gr.SetRange('x',0,wavefct.rows());
	gr.SetRange('y',0,wavefct.cols());
	gr.SetRange('z',phase);
	gr.SetRange('c',phase);

	gr.SubPlot(2,2,0);

	gr.Rotate(40,40);
	gr.Box();
	gr.Axis();
	gr.Surf(phase);


	gr.SubPlot(2,2,2);
	gr.Axis();
	gr.Colorbar("_");
	gr.Dens(phase);


	// data.use_abs=true;
	gr.SetRange('x',0,wavefct.rows());
	gr.SetRange('y',0,wavefct.cols());
	gr.SetRange('z',density);
	gr.SetRange('c',density);

	gr.SubPlot(2,2,1);

	// gr.Light(true);
	gr.Rotate(40,40);
	gr.Box();
	gr.Axis();

	gr.Surf(density);

	gr.SubPlot(2,2,3);
	gr.Axis();
	gr.Colorbar("_");
	gr.Dens(density);

	gr.WritePNG(filename.c_str(),"ExpandingVortexGas2D",false);

}

// void plotWithExpandingFrame(string filename,string title, ComplexGrid &Psi,vector<double> &ranges, vector<double> &Xexpanding,vector<double> &Yexpanding,Options &opt)
// {
	
// 	int32_t factor = (opt.grid[1] > 2048) ? opt.grid[1]/2048 : 1;
// 	int n = opt.grid[1]/factor;
// 	int m = opt.grid[2]/factor;



// 	mglData data(n,m);
// 	mglData xaxis(n);
// 	mglData yaxis(m);

// 	int i,j,k;

// 	// data.Create(n,m);

// 	for(i=0;i<n;i++) for(j=0;j<m;j++)
// 	{	
// 		k = i+n*j;

// 		data.a[k] = abs2(Psi(0,i,j,0));		
// 	}

// 	for( i = 0; i < n; i++){ xaxis.a[factor*i] = Xexpanding[factor*i]; }
// 	for( j = 0; j < m; j++){ yaxis.a[factor*j] = Yexpanding[factor*j]; }



// 	mglGraph gr;

		
// 		// gr.Light(0,true);
// 		// gr.Alpha(true);
	
// 	double range = (ranges[0] >= ranges[1]) ? ranges[0] : ranges[1];
	

// 	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
// 	gr.SetFontSize(3.0);
// 	gr.SetQuality(3);
// 	gr.Title(title.c_str());
// 	gr.SetRange('x',-range,range);
// 	gr.SetRange('y',-range,range);
// 	// gr.Alpha(true);

// 	filename = filename + ".png";


// 	// data.use_abs=false;

// 	// gr.SetRange('z',data);
// 	// gr.SetRange('c',data);

// 	// gr.SubPlot(2,2,0);

// 	// gr.Rotate(40,40);
// 	// gr.Box();
// 	// gr.Axis();
// 	// gr.Surf(xaxis,yaxis,data);


// 	// gr.SubPlot(2,2,2);
// 	// gr.Axis();
// 	// gr.Colorbar("_");
// 	// gr.Dens(xaxis,yaxis,data);


// 	// data.use_abs=true;
// 	gr.SetRange('z',data);
// 	// gr.SetRange('c',data);
// 	gr.SetRange('c',data);

// 	// gr.SubPlot(2,2,1);

// 	// // gr.Light(true);
// 	// gr.Rotate(40,40);
// 	// gr.Box();
// 	// gr.Axis();

// 	// gr.Surf(xaxis,yaxis,data);

// 	// gr.SubPlot(2,2,3);
// 	gr.Axis();
// 	gr.Colorbar("_");
// 	gr.Dens(xaxis,yaxis,data,"w{B,0.05}bcyrR");

// 	gr.WritePNG(filename.c_str(),"ExpandingVortexGas2D",false);

// }

void plotDataToPngEigenExpanding(string filename, Eigen::MatrixXcd& mPsi,vector<double> &ranges,Eigen::VectorXd &Xexpanding,Eigen::VectorXd &Yexpanding,Options opt)
{
	

	int n = opt.grid[1];
	int m = opt.grid[2];



	mglData data(n,m);
	mglData xaxis(n);
	mglData yaxis(m);

	int i,j,k;

	// data.Create(n,m);

	for(i=0;i<n;i++) for(j=0;j<m;j++)
	{	
		k = i+n*j;

		data.a[k] = abs2(mPsi(i,j));		
	}

	for( i = 0; i < n; i++){ xaxis.a[i] = Xexpanding(i); }
	for( j = 0; j < m; j++){ yaxis.a[j] = Yexpanding(j); }



	mglGraph gr;

		
		// gr.Light(0,true);
		// gr.Alpha(true);

	filename = filename + "-Density-Expanding.png";

	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
	gr.SetFontSize(3.0);
	gr.SetQuality(3);
	gr.Title(filename.c_str());
	gr.SetRange('x',xaxis);
	gr.SetRange('y',yaxis);
	// gr.Alpha(true);


	// data.use_abs=false;

	// gr.SetRange('z',data);
	// gr.SetRange('c',data);

	// gr.SubPlot(2,2,0);

	// gr.Rotate(40,40);
	// gr.Box();
	// gr.Axis();
	// gr.Surf(xaxis,yaxis,data);


	// gr.SubPlot(2,2,2);
	// gr.Axis();
	// gr.Colorbar("_");
	// gr.Dens(xaxis,yaxis,data);


	// data.use_abs=true;
	gr.SetRange('z',data);
	// gr.SetRange('c',data);
	gr.SetRange('c',data);

	// gr.SubPlot(2,2,1);

	// // gr.Light(true);
	// gr.Rotate(40,40);
	// gr.Box();
	// gr.Axis();

	// gr.Surf(xaxis,yaxis,data);

	// gr.SubPlot(2,2,3);
	gr.Axis();
	gr.Colorbar("_");
	gr.Dens(xaxis,yaxis,data);

	gr.WritePNG(filename.c_str(),"ExpandingVortexGas2D",false);

}


void plotVector(string filename,vector<double> v,vector<double> w,Options &opt){

	int x = v.size();
	int y = w.size();

	mglData v_data(x);
	mglData w_data(y);

	for(int i=0;i < x;i++){	
		v_data.a[i] = v[i];
	}
	for(int j = 0; j < y; j++){
		w_data.a[j] = w[j];
	}

	mglGraph gr;
		
	filename = filename + ".png";

	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
	gr.SetFontSize(3.0);
	gr.SetQuality(3);
	// gr.Title(title.c_str());
	// gr.Alpha(true);

	gr.SubPlot(2,1,0);
	gr.SetRange('x',-opt.min_x,opt.min_x);
	gr.SetRange('y',v_data);
	gr.Axis();
	gr.Plot(v_data);

	gr.SubPlot(2,1,1);
	gr.SetRange('x',-opt.min_y,opt.min_y);
	gr.SetRange('y',w_data);
	gr.Axis();
	gr.Plot(w_data);

	gr.WritePNG(filename.c_str(),"ExpandingVortexGas2D",false);

}

void plotVector(string filename,string title,vector<double> v,Options &opt){

	int x = v.size();

	mglData data(x);

	for(int i=0;i < x;i++){	
		data.a[i] = v[i];
	}

	mglGraph gr;
		
	filename = filename + ".png";

	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
	gr.SetFontSize(3.0);
	gr.SetQuality(3);
	gr.Title(title.c_str());

	gr.SetRange('x',0,v.size());
	gr.SetRange('y',data);
	gr.Axis();
	gr.Plot(data);

	gr.WritePNG(filename.c_str(),"ExpandingVortexGas2D",false);

}

void plotVector(string filename,string title,ArrayXd v,ArrayXd w){

	int x = v.size();

	mglData data(x);
	mglData dens(x);

	for(int i=0;i < x;i++){	
		data.a[i] = v[i];
		dens.a[i] = w[i];
	}

	mglGraph gr;
		
	filename = filename + ".png";

	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
	gr.SetFontSize(3.0);
	gr.SetQuality(3);
	gr.Title(title.c_str());

	gr.SetRange('x',0,v.size());
	gr.SetRange('y',data);
	gr.Axis();
	
	gr.Plot(dens);
	gr.Plot(data);

	gr.WritePNG(filename.c_str(),"ExpandingVortexGas2D",false);

}

void plotTwoVectors(string filename,string title,ArrayXd v_x,ArrayXd w_x,ArrayXd v_y,ArrayXd w_y){

	int x = v_x.size();

	mglData data(x);
	mglData dens(x);

	for(int i=0;i < x;i++){	
		data.a[i] = v_x[i];
		dens.a[i] = w_x[i];
	}

	mglGraph gr;
		
	filename = filename + ".png";

	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
	gr.SetFontSize(3.0);
	gr.SetQuality(3);
	gr.Title(title.c_str());

	gr.SubPlot(2,1,0);

	gr.SetRange('x',0,v_x.size());
	gr.SetRange('y',data);
	gr.Axis();	
	gr.Plot(dens);
	gr.Plot(data);

	gr.SubPlot(2,1,1);

	for(int i=0;i < x;i++){	
		data.a[i] = v_y[i];
		dens.a[i] = w_y[i];
	}

	gr.SetRange('x',0,v_y.size());
	gr.SetRange('y',data);
	gr.Axis();	
	gr.Plot(dens);
	gr.Plot(data);

	gr.WritePNG(filename.c_str(),"ExpandingVortexGas2D",false);

}

void plotAngularDensity(string filename,vector<double> phi,vector<double> density,Options &opt){

	int x = phi.size();
	int y = density.size();

	mglData phi_data(x);
	mglData density_data(y);

	for(int i=0;i < x;i++){	
		phi_data.a[i] = phi[i];
	}
	for(int j = 0; j < y; j++){
		density_data.a[j] = density[j];
	}

	mglGraph gr;
		
	filename = filename + ".png";

	gr.SetSize(IMAGE_SIZE,IMAGE_SIZE);
	gr.SetFontSize(3.0);
	gr.SetQuality(3);
	gr.Title(filename.c_str());

	gr.SetRange('x',phi_data);
	gr.SetRange('y',density_data);
	gr.Axis();
	gr.Plot(phi_data,density_data);

	gr.WritePNG(filename.c_str(),"ExpandingVortexGas2D",false);

}

void plotPair(std::vector<std::pair<dlib::matrix<double,2,1>, double> > data_samples)
{
	int n = data_samples.size();
	mglData x(n), y(n), z(n);
	for(int i=0;i<n;i++)
  	{
    x.a[i] = data_samples[i].first(0);
    y.a[i] = data_samples[i].first(1);
    z.a[i] = data_samples[i].second;
  }
  	mglGraph gr;
    gr.SubPlot(1,1,0);
    gr.Title("Dots sample");
    gr.SetRange('x',x);
	gr.SetRange('y',y);
    gr.Rotate(50,60);
  	gr.Box();
	gr.Dots(x,y,z);
	gr.WritePNG("plotInitialGaussian.png","ExpandingVortexGas2D",false);
}

void plotGauss(dlib::matrix<double,4,1> params,int n, double coordinate_axis)
{	
	double delta_x = 2 * coordinate_axis / n;
	mglData z(n,n);
	cerr << "NNN " << n << endl;
	int k = 0;
	int test = n * n;
	int counter = 0;
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < n; ++j){
			k = i+n*j;
  			double i0 = - coordinate_axis + delta_x * i;
  			double i1 = - coordinate_axis + delta_x * j;
    		z.a[k] = params(0) * exp(- params(1) * i0 * i0 - params(2) * i0 * i1 - params(3) * i1 * i1);
    		
    		if(counter >= test){
    			 cerr << "ACHTUNG" << endl;
    			 cerr << "counter " << counter << endl;
    		}
    		counter++;
    	}
    }
    mglGraph gr;
    gr.SubPlot(1,1,0);
    gr.Title("Dots sample");
    gr.SetRange('x',-coordinate_axis,coordinate_axis);
	gr.SetRange('y',-coordinate_axis,coordinate_axis);
	gr.SetRange('z',z);
    gr.Rotate(50,60);
  	gr.Surf(z);
  	gr.WritePNG("plotFittedGaussian.png","ExpandingVortexGas2D",false);
}

void plotPairAndGauss(std::vector<std::pair<dlib::matrix<double,2,1>, double> > data_samples, dlib::matrix<double,4,1> params,int n, double coordinate_axis){
	int m = data_samples.size();
	mglData x(m), y(m), z(m);
	mglData diff(m);
	for(int i=0;i<m;i++)
  	{
    	x.a[i] = data_samples[i].first(0);
    	y.a[i] = data_samples[i].first(1);
    	z.a[i] = data_samples[i].second;
    	// diff.a[i] = params(0) * exp(- params(1) * x.a[i] * x.a[i] - params(2) * x.a[i] * y.a[i] - params(3) * y.a[i] * y.a[i]) - z.a[i];
    	double temp = /*2 * (params(0) / M_PI) * (1 / (params(1) * params(3)))*/ params(0) * (1 - (x.a[i]*x.a[i])/(params(1)*params(1)) - (y.a[i]*y.a[i])/(params(3)*params(3)) - params(2) * x.a[i] * y.a[i]);
    	if(temp < 0) temp = 0;
    	diff.a[i] = temp - z.a[i];
  	}

  	double delta_x = 2 * coordinate_axis / n;
	mglData data(n,n);
	int k = 0;
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < n; ++j){
			k = i+n*j;
  			double i0 = - coordinate_axis + delta_x * i;
  			double i1 = - coordinate_axis + delta_x * j;
    		// data.a[k] = params(0) * exp(- params(1) * i0 * i0 - params(2) * i0 * i1 - params(3) * i1 * i1);
    		double temp = /*2 * (params(0) / M_PI) * (1 / (params(1) * params(3)))*/ params(0) * (1 - (i0*i0)/(params(1)*params(1)) - (i1*i1)/(params(3)*params(3)) - params(2) * i0 * i1);
    		if(temp < 0) temp = 0;
    		data.a[k] = temp;
    		
    	}
    }
    mglGraph gr;

    gr.SetSize(IMAGE_SIZE*2,IMAGE_SIZE);
	gr.SetFontSize(3.0);
	gr.SetQuality(3);
    gr.Light(true);    gr.Alpha(true);
    
    gr.Title("Dots sample");

    gr.SubPlot(3,1,0);
    gr.SetRange('x',-coordinate_axis,coordinate_axis);
	gr.SetRange('y',-coordinate_axis,coordinate_axis);
	gr.SetRange('z',z);	
    gr.Rotate(50,60);
    gr.Axis();
    gr.Dots(x,y,z);    

    gr.SubPlot(3,1,1);
    gr.SetRange('x',-coordinate_axis,coordinate_axis);
	gr.SetRange('y',-coordinate_axis,coordinate_axis);
	gr.SetRange('z',data);
    gr.Rotate(50,60);
    gr.Axis();
    gr.Surf(data);

    gr.SubPlot(3,1,2);
    gr.SetRange('x',-coordinate_axis,coordinate_axis);
	gr.SetRange('y',-coordinate_axis,coordinate_axis);
	gr.SetRange('z',data);
    gr.Rotate(50,60);
    gr.Axis();
    gr.Dots(x,y,diff);
    
  	
  	gr.WritePNG("plotCombinedGaussian.png","ExpandingVortexGas2D",false);
}