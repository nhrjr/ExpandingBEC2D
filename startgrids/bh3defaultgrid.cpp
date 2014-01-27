#include <bh3defaultgrid.h>
#include <complexgrid.h>
#include <gauss_random.h>
#include <bh3propagator.h>
#include <coordinate.h>
#include <stdlib.h>

using namespace std;

int mypow2(int x, int y)
{
	int z=1;
	
	for(int i=0; i<y; i++)
		z=x*z;
	
	return z;
}

inline complex<double> noise(GaussRandom &r)
{
    return r.gauss_random() * 0.5 ;
    //return 0.0;                                                                                                                                            
}


inline complex<double> noise2(GaussRandom &r)
{
    return r.gauss_random() * 7.5 ;
    //return 0.0;                                                                                                                                            
}

void phasefunction(int depth_o,int depth_u ,int Vy, int width_l,int width_r,int Vx,  const PathOptions &opt,ComplexGrid &m)
{
    double atan2(double y,double x);

    int  V_x[] = {Vx, Vx-2*width_l,    Vx+2*width_r, Vx+2*width_r,  Vx-2*width_l,            Vx,  Vx+2*width_r,            Vx,   Vx-2*width_l};
    int  V_y[] = {Vy,         Vy,  Vy+2*depth_o,           Vy,  Vy+2*depth_o,    Vy-2*depth_u,  Vy-2*depth_u,  Vy+2*depth_o,   Vy-2*depth_o};

    int u_limit=Vy+depth_o;
    int d_limit=Vy-depth_u;

    int l_limit=Vx-width_l;
    int r_limit=Vx+width_r;

    for(int i=0; i<9; i++)
    {
        for(int y=d_limit; y<=u_limit; y++)
        {
            for(int x=l_limit; x<=r_limit; x++)
            {
                m(0,x,y,0)+=(mypow2(-1,i))*(atan2(y-V_y[i],x-V_x[i]));
            }
        }
    }
}


void phasedisturbance(const PathOptions &opt,ComplexGrid &m)
{
    GaussRandom r (get_seed());

    for(int x = 0; x < m.width();x++)
    {
        for(int y = 0; y < m.height(); y++)
        { 
            if(6.5 < sqrt(x*x+y*y) && sqrt(x*x+y*y) < 9.5)
            {

                m(0,x,y,0)=noise2(r);
                m(0,m.width()-1-x,y,0)=noise2(r);
                m(0,x,m.height()-1-y,0)=noise2(r);
                m(0,m.width()-1-x,m.height()-1-y,0)=noise2(r);
            }
            else
            {
                m(0,x,y,0) = complex<double> (0.0,0.0);
            }
        }
    }

	ComplexGrid::fft(m, m, false);

    for(int x = 0; x < m.width();x++)
    {
        for(int y = 0; y < m.height(); y++)
        { 
            m(0,x,y,0) = polar(1.0, real(m(0,x,y,0)));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////neus Startgitter fuer vortices2/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ComplexGrid *create_Vortex_start_Grid2(const PathOptions &opt,int Vortexnumber, int rows_y, int columns_x, int Q) 
{
    if(rows_y*columns_x!=Vortexnumber)
    {
        std::cout<< "Invalid Columnsize or Rowsize" <<std::endl;
        exit(1);
    }

    if((Vortexnumber % 2)!=0)
    {
        std::cout<< "!!!!!!!!!Be careful momentum is entering in the starting conditions!!!!!!!!!" <<std::endl;
    }

    ComplexGrid *g  = new ComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2], opt.grid[3]);

    double rho=opt.N/(opt.grid[1]*opt.grid[2]*opt.grid[3]);
  
    int V_dim_x[2*Vortexnumber];
    int V_dim_y[2*Vortexnumber];

    int V_y[Vortexnumber];
    int V_x[Vortexnumber]; 

    int Setting_x=(int)(opt.grid[1]/(columns_x*2));
    int Setting_y=(int)(opt.grid[2]/(rows_y*2));
      
    for(int i = 0; i < Vortexnumber; i++)
    {
        if(((i%columns_x)==0))
        {
            V_x[i]=Setting_x;
            V_dim_x[i]=Setting_x;
            V_dim_x[i+Vortexnumber]=Setting_x-1;
        }
        else if((i%(columns_x))==(columns_x-1))
        {
            V_x[i]=2*Setting_x+V_x[i-1];
            V_dim_x[i]=Setting_x;
            V_dim_x[i+Vortexnumber]=opt.grid[0]-V_x[i]-1;
        }
        else 
        {
            V_x[i]=2*Setting_x+V_x[i-1];
            V_dim_x[i]=Setting_x;
            V_dim_x[i+Vortexnumber]=Setting_x-1;
        }
             
    }
           
    for(int i = 0; i < Vortexnumber; i++)
    {
        if(i<columns_x)
        {
            V_y[i]=Setting_y;
            V_dim_y[i]=Setting_y;
            V_dim_y[i+Vortexnumber]=Setting_y-1;
        }
        else if(i>=(columns_x*(rows_y-1)))
        {
            V_y[i]=2*Setting_y+V_y[i-columns_x];
            V_dim_y[i]=Setting_y;
            V_dim_y[i+Vortexnumber]=opt.grid[1]-V_y[i]-1;
        }
        else 
        {
            V_y[i]=2*Setting_y+V_y[i-columns_x];
            V_dim_y[i]=Setting_y;
            V_dim_y[i+Vortexnumber]=Setting_y-1;
        }
    }

    
    for(int j = 0; j < opt.grid[0]; j++)
    {
        int r = 0; 
        for(int i = 0; i < Vortexnumber; i++)
        {
            if((columns_x % 2)==0 && (i % columns_x)==0)
            {    
                r++;
            }          
                  
            for(int y = 0; y < opt.grid[2]; y++)
            {
                for(int x = 0; x < opt.grid[1]; x++)
                {
                    if(i==0)
                    {
                        g->at(j,x,y,0)= polar(1.0,(Q*mypow2(-1,i+r))*(atan2(y-V_y[i],x-V_x[i])));
                    }
                    else if(i==Vortexnumber-1)
                    {
                        g->at(j,x,y,0)*= sqrt(rho)*polar(1.0,(Q*mypow2(-1,i+r))*(atan2(y-V_y[i],x-V_x[i])));
                    }
                    else
                    {
                        g->at(j,x,y,0)*= polar(1.0,(Q*mypow2(-1,i+r))*(atan2(y-V_y[i],x-V_x[i])));
                    }

                }
            }
        } 
    }
 
//truncated Wigner noise; 
/*    ComplexGrid::fft (*g, *g, false);

    GaussRandom rand (get_seed());

    for(int i = 0; i < opt.grid[0]; i++)
    {
        for(int x = 0; x < opt.grid[1]; x++)   
        {
            for (int y = 0; y < opt.grid[2]; y++)
            {
                for(int z = 0; z < opt.grid[3]; z++)
                {
                    if(!(z==0 && x==0 && y==0))
                        g->at(i,x,y,z) += noise(rand);
                }
            }
        }
    }
 
    ComplexGrid::fft (*g, *g, false);*/
    
    return g;  
    	                                                                     
}

//////////////neus Startgitter fuer vortices2//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ComplexGrid *create_Default_Start_Grid(const PathOptions &opt, int d) //use only in 2D!!
{
    GaussRandom r (get_seed());
     
	double RATIO = 0.5;
	ComplexGrid *g = new ComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2], opt.grid[3]);
	
    for(int i = 0; i < opt.grid[0]; i++)
    {
        for(int x=0; x < opt.grid[1]; x++)
        {
            for (int y=0; y < opt.grid[2]; y++)
            {
                for(int z=0; z < opt.grid[3]; z++)
                {
                    g->at(i,x,y,z) = noise(r);
                }
            }
        }
	
    
		if (d<0)
		{
         	d = -d;
            g->at(i,0,0,0) = complex<double>(sqrt(opt.N/4.0), sqrt(opt.N/4.0)) - noise(r);
            g->at(i,0,d,0) = complex<double>(sqrt(opt.N/8.0*RATIO), sqrt(opt.N/8.0*RATIO)) - noise(r);
            g->at(i,0,opt.grid[2] - d,0) = complex<double>(sqrt(opt.N/8.0*(1.0-RATIO)), sqrt(opt.N/8.0*(1.0-RATIO))) - noise(r);
       	
            g->at(i,opt.grid[1] - d ,d, 0) = complex<double>(sqrt(opt.N/8.0*RATIO), sqrt(opt.N/8.0*RATIO)) - noise(r);
            g->at(i,d,opt.grid[2] - d, 0) = complex<double>(sqrt(opt.N/8.0*(1.0-RATIO)), sqrt(opt.N/8.0*(1.0-RATIO))) - noise(r);
        }
	
        else if (d == 0)
        {
            g->at(i,0,0,0) = complex<double>(sqrt(opt.N/2.0), sqrt(opt.N/2.0)) - noise(r);
        }	
	
        else
        {  
            g->at(i,0,0,0) = complex<double>(sqrt(opt.N/4.0), sqrt(opt.N/4.0)) - noise(r);
            g->at(i,0,d,0) = complex<double>(sqrt(opt.N/8.0*RATIO), sqrt(opt.N/8.0*RATIO)) - noise(r);
            g->at(i,0, opt.grid[2] - d,0) = complex<double>(sqrt(opt.N/8.0*(1.0-RATIO)), sqrt(opt.N/8.0*(1.0-RATIO))) - noise(r);
		
            g->at(i,d,d,0) = complex<double>(sqrt(opt.N/8.0*RATIO), sqrt(opt.N/8.0*RATIO)) - noise(r);
            g->at(i,opt.grid[1] - d, opt.grid[2] - d,0) = complex<double>(sqrt(opt.N/8.0*(1.0-RATIO)), sqrt(opt.N/8.0*(1.0-RATIO))) - noise(r);
        }
    }
    
	ComplexGrid::fft(*g, *g, false);
	return g;
}

ComplexGrid *create_no_noise_Start_Grid(const PathOptions &opt, int d)
{


	double RATIO = 0.5;
	ComplexGrid *g = new ComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2], opt.grid[3]);
	
    for (int i = 0; i < opt.grid[0]; i++)
    {
        if(d<0)
        {
            d = -d;
            g->at(i,0,0,0) = complex<double>(sqrt(opt.N/4.0), sqrt(opt.N/4.0));
            g->at(i,0,d,0) = complex<double>(sqrt(opt.N/8.0*RATIO), sqrt(opt.N/8.0*RATIO));
            g->at(i,0, opt.grid[2] - d,0) = complex<double>(sqrt(opt.N/8.0*(1.0-RATIO)), sqrt(opt.N/8.0*(1.0-RATIO)));
		
            g->at(i,d,d,0) = complex<double>(sqrt(opt.N/8.0*RATIO), sqrt(opt.N/8.0*RATIO));
            g->at(i,opt.grid[1] - d, opt.grid[2] - d,0) = complex<double>(sqrt(opt.N/8.0*(1.0-RATIO)), sqrt(opt.N/8.0*(1.0-RATIO)));
        }
        else if (d == 0)
        {
            g->at(i,0,0,0) = complex<double>(sqrt(opt.N/2.0), sqrt(opt.N/2.0));
        }	
	
        else
        {  
            g->at(i,0,0,0) = complex<double>(sqrt(opt.N/4.0), sqrt(opt.N/4.0));
            g->at(i,0,d,0) = complex<double>(sqrt(opt.N/8.0*RATIO), sqrt(opt.N/8.0*RATIO));
            g->at(i,0, opt.grid[2] - d,0) = complex<double>(sqrt(opt.N/8.0*(1.0-RATIO)), sqrt(opt.N/8.0*(1.0-RATIO)));
		
            g->at(i,opt.grid[1] - d,d,0) = complex<double>(sqrt(opt.N/8.0*RATIO), sqrt(opt.N/8.0*RATIO));
            g->at(i,d, opt.grid[2] - d,0) = complex<double>(sqrt(opt.N/8.0*(1.0-RATIO)), sqrt(opt.N/8.0*(1.0-RATIO)));
        }
    }
    
        
	ComplexGrid::fft(*g, *g, false);
	return g;
}

ComplexGrid *create_noise_Start_Grid(const PathOptions &opt)
{
    GaussRandom r (get_seed());
	
    double RATIO = 0.5;
	
    ComplexGrid *g = new ComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2], opt.grid[3]);
	
    for(int i = 0; i < opt.grid[0]; i++)
    {
        for(int x=0; x < opt.grid[1]; x++)
        {
            for (int y=0; y < opt.grid[2]; y++)
            {
                for(int z=0; z < opt.grid[3]; z++)
                {
                    g->at(i,x,y,z) = noise(r);
                }
            }
        }
	}
    


	ComplexGrid::fft(*g, *g, false);
	return g;
}

ComplexGrid *create_Energy_Start_Grid(const PathOptions &opt, int d)
{
    GaussRandom r (get_seed());

	ComplexGrid *g = new ComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2], opt.grid[3]);

    for(int i = 0; i < opt.grid[0]; i++)
    {
        for(int x = 0; x < opt.grid[1]; x++)
        {
            for (int y = 0; y < opt.grid[2]; y++)
            {
                for(int z = 0; z < opt.grid[3]; z++)
                {
                    g->at(i,x,y,z) = noise(r);
                }
            }
        }
	}
    
	if(d<1)
		d = 2;
	
    for(int i = 0; i < opt.grid[0]; i++)
    {
        g->at(i,0,0,0) = complex<double>(sqrt(opt.N*d/d/d/2.0), sqrt(opt.N*d/d/d/2.0)) - noise(r);

        for(int j = 1; j < d; j++)
        {
            g->at(i,0,j,0) = complex<double>(sqrt(opt.N*(d-i)/d/d/4.0), sqrt(opt.N*(d-i)/d/d/4.0)) - noise(r);
            g->at(i,0, opt.grid[2] - j,0) = complex<double>(sqrt(opt.N*(d-i)/d/d/4.0), sqrt(opt.N*(d-i)/d/d/4.0)) - noise(r);
		
            g->at(i,j,j,0) = complex<double>(sqrt(opt.N*(d-i)/d/d/4.0), sqrt(opt.N*(d-i)/d/d/4.0)) - noise(r);
            g->at(i,opt.grid[1] - j, opt.grid[2] - j,0) = complex<double>(sqrt(opt.N*(d-i)/d/d/4.0), sqrt(opt.N*(d-i)/d/d/4.0)) - noise(r);
        }
    }
    
	ComplexGrid::fft(*g, *g, false);
	return g;
}

ComplexGrid *create_vortex(const PathOptions &opt, int d)
{
	Coordinate<int> origin(0,0,0, opt.grid[1], opt.grid[2], opt.grid[3]);
	
	ComplexGrid *g = new ComplexGrid(opt.grid[0], opt.grid[1], opt.grid[2], opt.grid[3]);
	
	double rho = opt.N/(opt.grid[1]*opt.grid[2]*opt.grid[3]); 
	double xi  = 1./sqrt(opt.U*rho);
	
    for(int i = 0; i < opt.grid[0]; i++)
    {
        for(int x = 0; x < opt.grid[1]; x++)
        {
            for (int y = 0; y < opt.grid[2]; y++)
            {
                for(int z = 0; z < opt.grid[3]; z++)
                {
                    Coordinate<int> c(x,y,z, opt.grid[1], opt.grid[2], opt.grid[3]);
                    Vector<int> vec = c - origin;
                    Coordinate<int> c_new(vec.x(),vec.y(),vec.z(), opt.grid[1], opt.grid[2], opt.grid[3]);

                    g->at(i,c_new) = (rho/xi)*vec.norm()/sqrt(2+vec.norm()*vec.norm()/(xi*xi));
                }
            }
        }
	}
    
	return g;
}


ComplexGrid *create_inverse_Start_Grid(const PathOptions &opt, const ComplexGrid &start)
{   
    GaussRandom r (get_seed());

	ComplexGrid *g = new ComplexGrid(start.int_dim(), start.width(), start.height(), start.depth());
	
    for(int i = 0; i < start.int_dim(); i++)
    {
        for(int x = 0; x < start.width(); x++)
        {
            for (int y = 0; y < start.height(); y++)
            {
                for(int z = 0; z < start.depth(); z++)
                {
                    complex <double> total = noise(r) + sqrt(2.*opt.N/(start.width()*start.height()*start.depth()));
                    g->at(i,x,y,z) = total - start(i,x,y,z);
                }
            }
        }
	}
    
	return g;
}



 
