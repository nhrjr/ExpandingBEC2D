#ifndef EXP2D_STARTGRIDS_H__
#define EXP2D_STARTGRIDS_H__

#include "matrixdata.h"
#include "tools.h"
#include "plot_with_mgl.h"

void setGridToDoubleGaussian(MatrixData* &data, Options opt)
{
    double sigma_x = opt.min_x/4;
    double sigma_y = opt.min_y/4;
    double h_x = 2.*opt.min_x/opt.grid[1];
    double h_y = 2.*opt.min_y/opt.grid[2];
    vector<double> x(opt.grid[1]);
    vector<double> y(opt.grid[2]);

    for(int i=0;i<opt.grid[1];i++){x[i]=-opt.min_x+i*h_x;}
    for(int j=0;j<opt.grid[2];j++){y[j]=-opt.min_y+j*h_y;}

    complex<double> value;
    for(int i=0; i < opt.grid[1]; i++){
        for(int j=0; j < opt.grid[2]; j++){
            value =  complex<double>((opt.N/(4 * opt.min_x * opt.min_y)) * exp( -((x[i] - opt.min_x/2) * (x[i] - opt.min_x/2))/(2.*sigma_x*sigma_x) - (y[j] * y[j])/(2.*sigma_y*sigma_y) ), 0.0 );
            value += complex<double>((opt.N/(4 * opt.min_x * opt.min_y)) * exp( -((x[i] + opt.min_x/2) * (x[i] + opt.min_x/2))/(2.*sigma_x*sigma_x) - (y[j] * y[j])/(2.*sigma_y*sigma_y) ), 0.0 );
            for(int m = 0; m < data->wavefunction.size(); m++)
                data->wavefunction[m](i,j) = value;
        }
    }
};

void setGridToGaussian(MatrixData* &data, Options opt)
{
    double sigma_x = opt.min_x/8;
    double sigma_y = opt.min_y/8;
    double h_x = 2.*opt.min_x/opt.grid[1];
    double h_y = 2.*opt.min_y/opt.grid[2];
    vector<double> x(opt.grid[1]);
    vector<double> y(opt.grid[2]);

    for(int i=0;i<opt.grid[1];i++){x[i]=-opt.min_x+i*h_x;}
    for(int j=0;j<opt.grid[2];j++){y[j]=-opt.min_y+j*h_y;}

    complex<double> value;
    #pragma omp parallel for
    for(int i=0; i < opt.grid[1]; i++){
        for(int j=0; j < opt.grid[2]; j++){
            value = complex<double>(sqrt(opt.N) * exp( -(x[i] * x[i])/(2.*sigma_x*sigma_x) - (y[j] * y[j])/(2.*sigma_y*sigma_y) ), 0.0 );
                data->wavefunction[0](i,j) = value;
        }
    }
};

double density(double &x, double &y, double &Rx, double &Ry, double &N){

    double value = 2 * (N / M_PI) * (1 / (Rx * Ry)) * (1 - (x*x)/(Rx*Rx) - (y*y)/(Ry*Ry));
    if(value > 0){
        return sqrt(value);
    }else{
        return 0;
    }
};

void setGridToTF(shared_ptr<MatrixData> data, Options opt){

    double m = 87.0 * 1.66e-27;
    double hbar = 1.054e-34;

    double mu = sqrt(3.0  * opt.g * real(opt.omega_x) * real(opt.omega_y) * opt.N / 8.0);
    double Ry = sqrt(2.0 * mu / ( real(opt.omega_y)*real(opt.omega_y)));
    double Rx = sqrt(2.0 * mu / ( real(opt.omega_x)*real(opt.omega_x)));

    cout << "Thomas Fermi Radii set to Rx = " << Rx << " and Ry = " << Ry << endl;
    double n0 = 2 * (opt.N / M_PI) * (1 / (Rx * Ry));
    cout << "n_0 = " << n0 << endl;

    double h_x = 2.*opt.min_x/opt.grid[1];
    double h_y = 2.*opt.min_y/opt.grid[2];
    vector<double> x(opt.grid[1]);
    vector<double> y(opt.grid[2]);
    for(int i=0;i<opt.grid[1];i++){x[i]=-opt.min_x+i*h_x;}
    for(int j=0;j<opt.grid[2];j++){y[j]=-opt.min_y+j*h_y;}

    #pragma omp parallel for
    for(int i=0; i < opt.grid[1]; i++){
        for(int j=0; j < opt.grid[2]; j++){
            data->wavefunction[0](i,j) = complex<double>(density(x[i],y[j],Rx,Ry,opt.N),0.0);
        }
    }
};

void setGridToSinus(MatrixData* &data, Options opt){

    double h_x = 2.*opt.min_x/opt.grid[1];
    double h_y = 2.*opt.min_y/opt.grid[2];
    vector<double> x(opt.grid[1]);
    vector<double> y(opt.grid[2]);
    for(int i=0;i<opt.grid[1];i++){x[i]=-opt.min_x+i*h_x;}
    for(int j=0;j<opt.grid[2];j++){y[j]=-opt.min_y+j*h_y;}

    #pragma omp parallel for
    for(int i=0; i < opt.grid[1]; i++){
        for(int j=0; j < opt.grid[2]; j++){
            data->wavefunction[0](i,j) = complex<double>(i/*sin(2 * M_PI * i/opt.grid[1])*/,0.0);
        }
    }
};

void addDrivingForce(MatrixData* &data, Options &opt){
    #pragma omp parallel for
    for(int i = 0; i < opt.grid[1]; i++){
        double phase = abs(4*M_PI/opt.grid[1]*i - 2 * M_PI);
        for(int j = 0; j < opt.grid[1]; j++){
            data->wavefunction[0](i,j) *= exp(complex<double>(0,phase));
        }
    }
};

// void setGridToParaboloid(MatrixData* &data, Options opt){
//     complex<double> value;
//     #pragma omp parallel for
//     for(int i=0; i < opt.grid[1]; i++){
//         for(int j=0; j < opt.grid[2]; j++){
//             value = complex<double>((opt.N/(4 * opt.min_x * opt.min_y )) * exp( -(x[i] * x[i])/(2.*sigma_x*sigma_x) - (y[j] * y[j])/(2.*sigma_y*sigma_y) ), 0.0 );
//                 data->wavefunction[0](i,j) = value;
//         }
//     }
// };

// void addVorticesAlternating(MatrixData* &data, Options opt, int &vnumber){

// double maximum = 0;
// for(int i = 0; i < opt.grid[1]; ++i){
//     for(int j = 0; j < opt.grid[2]; ++j){
//         double value = abs2(data->wavefunction[0](i,j));
//         maximum = ( value > maximum) ? value : maximum;
//     }
// }

// double LOWER_THRESHOLD = maximum * 0.35; // opt.N / (4. * opt.min_x  * opt.min_y );

// int x_jump = opt.vortexspacing; // opt.grid[1] / 5;
// int y_jump = opt.vortexspacing; // opt.grid[2] / 5;
// int windingnumber = 1;

// // ComplexGrid grid(opt.grid[0],opt.grid[1],opt.grid[2],opt.grid[3]);

// vector<Coordinate<int32_t>> c;

// for(int y = y_jump; y < opt.grid[2]; y += y_jump*2){
//     for(int x = x_jump; x < opt.grid[1]; x += x_jump){
//         if(abs2(data->wavefunction[0](x,y)) >= LOWER_THRESHOLD){
//             c.push_back(grid.make_coord(x,y,0));
//         }
//     }
// }
// for(int y = y_jump*2; y < opt.grid[2]; y += y_jump*2){
//     for(int x = x_jump/2; x < opt.grid[1]; x += x_jump){
//         if(abs2(data->wavefunction[0](x,y)) >= LOWER_THRESHOLD){
//             c.push_back(grid.make_coord(x,y,0));
//         }
//     }
// }

// MatrixData test(1,opt.grid[1],opt.grid[2],0,0,opt.min_x,opt.min_y);
// for(int i = 0; i < c.size(); i++){
//     #pragma omp parallel for
//     for(int y = 0; y < opt.grid[2]; y++){
//         for(int x = 0; x < opt.grid[1]; x++){   
//             data->wavefunction[0](x,y) *= polar(1.0, (windingnumber  /** mypow2(-1,i+1)*/ ) * vortex( y,c[i].y(),x,c[i].x() )) ;
//         }
//     }
//     // g->wavefunction[0](c) complex<double>(0.0,0.0);
// }
// vnumber += c.size() * windingnumber;
//     // return opt.vortexnumber;
// }

// void addVorticesRegular(MatrixData* &data, Options opt, int &vnumber){

// cout << "Adding Vortices with a spacing of " << opt.vortexspacing << "." << endl;

// int x_jump = opt.vortexspacing; // opt.grid[1] / 5;
// int y_jump = opt.vortexspacing; // opt.grid[2] / 5;
// int windingnumber = 1;

// // ComplexGrid grid(opt.grid[0],opt.grid[1],opt.grid[2],opt.grid[3]);

// vector<Coordinate<int32_t>> c;

// for(int y = y_jump; y < opt.grid[2]; y += y_jump){
//     for(int x = x_jump; x < opt.grid[1]; x += x_jump){
//         if(abs2(data->wavefunction[0](x,y)) >= 10){
//             c.push_back(grid.make_coord(x,y,0));
//         }
//     }
// }
// // for(int y = y_jump*2; y < opt.grid[2]; y += y_jump*2){
// //     for(int x = x_jump/2; x < opt.grid[1]; x += x_jump){
// //         if(abs2(data->wavefunction[0](x,y)) >= 10){
// //             c.push_back(grid.make_coord(x,y,0));
// //         }
// //     }
// // }
// for(int i = 0; i < c.size(); i++){
//     #pragma omp parallel for
//     for(int y = 0; y < opt.grid[2]; y++){
//         for(int x = 0; x < opt.grid[1]; x++){   
//             data->wavefunction[0](x,y) *= polar(1.0, (windingnumber  /* * mypow2(-1,i+1) */) *vortex( y,c[i].y(),x,c[i].x() )) ;
//         }
//     }
//     // g->wavefunction[0](c) complex<double>(0.0,0.0);
// }




// vnumber += c.size() * windingnumber;
//     // return opt.vortexnumber;
// }

#endif // EXP2D_STARTGRIDS_H__