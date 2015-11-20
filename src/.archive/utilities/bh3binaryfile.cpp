#include <sys/stat.h>
#include <sys/types.h>
#include <cstdlib>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include <bh3binaryfile.h>
#include <complexgrid.h>
#include <realgrid.h>
#include <hdf5.h>
#include <libconfig.h>

// Class implementation
Bh3BinaryFile::Bh3BinaryFile(const string &file, const PathOptions &opt, mode nm)
{
	filename = file;
	options = opt;
	m = nm;
	num_snapshots_c = 0;
	num_snapshots_r = 0;
	
	if(m == in)
	{
		struct stat buf;
		lstat(filename.c_str(), &buf);
		if(S_ISREG(buf.st_mode) && H5Fis_hdf5(filename.c_str())) // Nur normale Dateien ueberpruefen
		{
			h5_file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
			
			if(H5Aexists(h5_file, "PathOptions") > 0)
			{

				hid_t h5a_options;
				h5a_options = H5Aopen(h5_file, "PathOptions", H5P_DEFAULT);
				
				double temp_opt[10];
				
				H5Aread(h5a_options, H5T_IEEE_F64LE , temp_opt);

				//load PathOptions struct from file array. Don't forget to change appropriately when changing PathOptions struct 
				options.timestepsize = temp_opt[0];
				options.N = temp_opt[1];
				options.U = temp_opt[2];
				for(int i = 0; i<4;i++)
					options.grid[i] = (uint32_t)temp_opt[i+3];
				for(int i= 0; i < 3; i++)
					options.klength[i] = temp_opt[i+7];

				H5Aclose(h5a_options);
				
				//load g_matrix from file
				h5a_options = H5Aopen(h5_file, "g_matrix", H5P_DEFAULT);
				int size = (int)(H5Aget_storage_size(h5a_options)/sizeof(double));
				double temp_g[size];
				options.g.resize(size);
				
				H5Aread(h5a_options, H5T_IEEE_F64LE , temp_g);
				
				for (int i = 0; i < size; i++)
					options.g[i] = temp_g[i];
				
				H5Aclose(h5a_options);
				
				//load delta_t vector from file, if exists
				if(H5Aexists(h5_file, "delta_t") > 0)
				{
					h5a_options = H5Aopen(h5_file, "delta_t", H5P_DEFAULT);
					size = (int)(H5Aget_storage_size(h5a_options)/sizeof(double));
					double temp_delta_t[size];
					options.delta_t.resize(size);
					
					H5Aread(h5a_options, H5T_IEEE_F64LE , temp_g);
					
					for (int i = 0; i < size; i++)
						options.delta_t[i] = temp_delta_t[i];
					
					H5Aclose(h5a_options);
				}
				else
					options.delta_t.resize(0);
				
				//open data groups which contain grid data and check out number of snapshots
				h5_complexgriddata = H5Gopen(h5_file, "/ComplexGridData", H5P_DEFAULT);
				H5Gget_num_objs(h5_complexgriddata, (hsize_t *)&num_snapshots_c);
				
				h5_realgriddata = H5Gopen(h5_file, "/RealGridData", H5P_DEFAULT);
				H5Gget_num_objs(h5_realgriddata, (hsize_t *)&num_snapshots_r);				
			}
			else
			{
				cout << "error during opening of file "<< filename.c_str() << "occurred: No valid PathOptions attribute found in file." << endl;
				num_snapshots_c = 0;
				num_snapshots_r = 0;
			}
		}
		else
		{
			num_snapshots_c = 0;
			num_snapshots_r = 0;
			cout << "file "<< filename.c_str() <<" is either not regular or not in hdf5 storage format" << endl;
		}
	}
	else
	{

		h5_file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
		
		hid_t    h5a_options, dataspace;

		
		//load PathOptions struct to a reliable arrays. Don't forget to change appropriately when changing PathOptions struct 
		double temp_opt[10];
		double temp_g[options.g.size()];
		temp_opt[0] = options.timestepsize;
		temp_opt[1] = options.N;
		temp_opt[2] = options.U;
		for(int i = 0; i<4;i++)
			temp_opt[i+3] = (double)options.grid[i];
		for(int i= 0; i < 3; i++)
			temp_opt[i+7] = options.klength[i];
		for(int i = 0; i < options.g.size(); i++)
			temp_g[i] = options.g[i];
		
		//copy options to file
		hsize_t dimsf[1] = {10};
		dataspace = H5Screate_simple(1, dimsf, NULL); 
		
		h5a_options = H5Acreate(h5_file, "PathOptions", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
		H5Awrite (h5a_options, H5T_IEEE_F64LE, temp_opt);

		H5Aclose(h5a_options);
		H5Sclose(dataspace);
		
		//copy delta_t array to file if there is one
		if (options.delta_t.size()>0)
		{
			double temp_delta_t[options.delta_t.size()];
			
			for(int i = 0; i < options.delta_t.size(); i++)
				temp_delta_t[i] = options.delta_t[i];
			
			dimsf[0] = options.delta_t.size();
			dataspace = H5Screate_simple(1, dimsf, NULL);
			
			h5a_options = H5Acreate(h5_file, "delta_t", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
			H5Awrite (h5a_options, H5T_IEEE_F64LE, temp_delta_t);

			H5Aclose(h5a_options);
			H5Sclose(dataspace);
		}
		
		//copy g matrix to file
		dimsf[0] = options.g.size();
		dataspace = H5Screate_simple(1, dimsf, NULL);
		
		h5a_options = H5Acreate(h5_file, "g_matrix", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
		H5Awrite (h5a_options, H5T_IEEE_F64LE, temp_g);

		H5Aclose(h5a_options);
		H5Sclose(dataspace);
		
		//create groups for grid data
		h5_complexgriddata = H5Gcreate(h5_file, "/ComplexGridData", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		h5_realgriddata = H5Gcreate(h5_file, "/RealGridData", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		
	}
}

Bh3BinaryFile::~Bh3BinaryFile()
{
	close();
}

void Bh3BinaryFile::close()
{
	H5Fclose(h5_file);	
	H5Gclose(h5_complexgriddata);
	H5Gclose(h5_realgriddata);
    
	filename = "";
}

bool Bh3BinaryFile::append_snapshot(double time, const vector<ComplexGrid> &k, bool compression)
{
	if(m != out)
	{
		cout << "file "<< filename.c_str() << "is not in write mode" << endl;
		return false;
	}

    stringstream set_name;
 	set_name << num_snapshots_c;
	num_snapshots_c++;
	
	if (k.size() == 1)
	{
		
		hid_t    dataset, dataspace, dset_create_props, h5a_time;
		
		hsize_t dimsf[1] = {2*options.grid[0]*options.grid[1]*options.grid[2]*options.grid[3]}; //dimensions for whole grid
		dataspace = H5Screate_simple(1, dimsf, NULL); 
		
		dset_create_props = H5Pcreate (H5P_DATASET_CREATE); //create a default creation property list
		
		if (compression)
		{
			dimsf[0] = 2*options.grid[1]*options.grid[2]*options.grid[3]; //define single component as chunk block, maybe too large!!!FIXME
			H5Pset_chunk (dset_create_props, 1, dimsf);    //enable chunks for compression
			H5Pset_szip (dset_create_props, H5_SZIP_NN_OPTION_MASK, 8); //enable data compression
		}
		dataset = H5Dcreate(h5_complexgriddata, (set_name.str()).c_str(), H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, dset_create_props, H5P_DEFAULT); //create data block in file
		
		H5Dwrite (dataset, H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, (double *)k[0].get_address()); //write grid data

		//append grid time as attribute
		dimsf[0] = 1;
		dataspace = H5Screate_simple(1, dimsf, NULL);
		h5a_time = H5Acreate(dataset, "time", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
		H5Awrite(h5a_time, H5T_IEEE_F64LE, &time);
		
		//clean up
		H5Aclose(h5a_time);
		H5Dclose(dataset);
		H5Pclose(dset_create_props);
		H5Sclose(dataspace);
	}
	else
	{
		hid_t    dataset, dataspace, dset_create_props, h5a_time;
		

		hsize_t dimsf[1] = {2*k.size()*options.grid[0]*options.grid[1]*options.grid[2]*options.grid[3]};//for file dataspace};

		dset_create_props = H5Pcreate (H5P_DATASET_CREATE); //create a default creation property list
		dataspace = H5Screate_simple(1, dimsf, NULL); //define space in file
		
		
		if (compression)
		{
			dimsf[0] = 2*options.grid[1]*options.grid[2]*options.grid[3];//define single component as chunk block, maybe too large!!!FIXME
			H5Pset_chunk (dset_create_props, 1, dimsf);    //enable chunks for compression
			H5Pset_szip (dset_create_props, H5_SZIP_NN_OPTION_MASK, 8); //enable data compression
		}
			
		dataset = H5Dcreate(h5_complexgriddata, (set_name.str()).c_str(), H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, dset_create_props, H5P_DEFAULT); //create dataset with full space selection
		
		for (int i = 0; i < k.size(); i++)
		{
			hsize_t offset[1] = {i*2*options.grid[0]*options.grid[1]*options.grid[2]*options.grid[3]}; //prepare partial writing operation to dataset by defining an offset for destination
			hsize_t count[1] = {2*options.grid[0]*options.grid[1]*options.grid[2]*options.grid[3]}; // and the space which will be copied to
			hid_t memspace = H5Screate_simple(1, count, NULL); // define full space in memory which will be copied
			H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL); //select part in dataset which will be written to
			H5Dwrite (dataset, H5T_IEEE_F64LE, memspace, dataspace, H5P_DEFAULT, (double *)k[i].get_address()); //now write  
			H5Sclose(memspace);
		}
		
		//bind the time of snapshot as attribut to dataset
		dimsf[0] = 1;
		dataspace = H5Screate_simple(1, dimsf, NULL);
		h5a_time = H5Acreate(dataset, "time", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
		H5Awrite(h5a_time, H5T_IEEE_F64LE, &time);
		H5Aclose(h5a_time);
		
		//and a the following time intervalls 
		dimsf[0] = options.delta_t.size();
		dataspace = H5Screate_simple(1, dimsf, NULL);
		h5a_time = H5Acreate(dataset, "delta_t", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
		H5Awrite(h5a_time, H5T_IEEE_F64LE, &options.delta_t[0]);
		H5Aclose(h5a_time);
		
		//clean up
		H5Dclose(dataset); 
		H5Pclose(dset_create_props);
		H5Sclose(dataspace);
	}
	
	return true;
}

bool Bh3BinaryFile::append_snapshot(double time, const vector<RealGrid> &k, bool compression)
{
	if(m != out)
	{
		cout << "file "<< filename.c_str() << "is not in write mode" << endl;
		return false;
	}
  
       	stringstream set_name;
 	set_name << num_snapshots_r;
	num_snapshots_r++;

	
	if (k.size() == 1)
	{
		hid_t    dataset, dataspace, dset_create_props, h5a_time;
		
		hsize_t dimsf[1] = {options.grid[0]*k[0].fft_width()*k[0].fft_height()*k[0].fft_depth()}; //dimensions for whole grid
		dataspace = H5Screate_simple(1, dimsf, NULL); 
		
		dset_create_props = H5Pcreate (H5P_DATASET_CREATE); //create a default creation property list

		
		if (compression)
		{
			dimsf[0] = k[0].fft_width()*k[0].fft_height()*k[0].fft_depth(); //define single component as chunk block, maybe too large!!! FIXME
			H5Pset_chunk (dset_create_props, 1, dimsf);    //enable chunks for compression
			H5Pset_szip (dset_create_props, H5_SZIP_NN_OPTION_MASK, 8); //enable data compression
		}
			
		dataset = H5Dcreate(h5_realgriddata, (set_name.str()).c_str(), H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, dset_create_props, H5P_DEFAULT); //create data block in file
		
		H5Dwrite (dataset, H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, k[0].get_address()); //write grid data

		//append grid time as attribute
		dimsf[0] = 1;
		dataspace = H5Screate_simple(1, dimsf, NULL);
		h5a_time = H5Acreate(dataset, "time", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
		H5Awrite(h5a_time, H5T_IEEE_F64LE, &time);
		
		//clean up
		H5Aclose(h5a_time);
		H5Dclose(dataset);
		H5Pclose(dset_create_props);
		H5Sclose(dataspace);
	}
	else
	{
		hid_t    dataset, dataspace, dset_create_props, h5a_time;
		

		hsize_t dimsf[1] = {k.size()*options.grid[0]*k[0].fft_width()*k[0].fft_height()*k[0].fft_depth()};//for chunk definition

		dset_create_props = H5Pcreate (H5P_DATASET_CREATE); //create a default creation property list
		H5Pset_chunk (dset_create_props, 1, dimsf);    //enable chunks for compression
		
		dataspace = H5Screate_simple(1, dimsf, NULL); //define space in file
		
		if (compression)
		{
			dimsf[0] = k[0].fft_width()*k[0].fft_height()*k[0].fft_depth();//define single component as chunk block, maybe too large!!! FIXME
			H5Pset_chunk (dset_create_props, 1, dimsf);    //enable chunks for compression    
			H5Pset_szip (dset_create_props, H5_SZIP_NN_OPTION_MASK, 8); //enable data compression
		}
			

		dataset = H5Dcreate(h5_realgriddata, (set_name.str()).c_str(), H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, dset_create_props, H5P_DEFAULT); //create dataset with full space selection
		
		for (int i = 0; i < k.size(); i++)
		{
			hsize_t offset[1] = {i*options.grid[0]*k[0].fft_width()*k[0].fft_height()*k[0].fft_depth()}; //prepare partial writing operation to dataset by defining an offset for destination
			hsize_t count[1] = {options.grid[0]*k[0].fft_width()*k[0].fft_height()*k[0].fft_depth()}; // and the space which will be copied to
			hid_t memspace = H5Screate_simple(1, count, NULL); // define full space in memory which will be copied
			H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL); //select part in dataset which will be written to
			H5Dwrite (dataset, H5T_IEEE_F64LE, memspace, dataspace, H5P_DEFAULT, k[i].get_address()); //now write  
			H5Sclose(memspace);
		}
		
		//bind the time of snapshot as attribut to dataset
		dimsf[0] = 1;
		dataspace = H5Screate_simple(1, dimsf, NULL);
		h5a_time = H5Acreate(dataset, "time", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
		H5Awrite(h5a_time, H5T_IEEE_F64LE, &time);
		H5Aclose(h5a_time);
		
		//and a the following time intervalls 
		dimsf[0] = options.delta_t.size();
		dataspace = H5Screate_simple(1, dimsf, NULL);
		h5a_time = H5Acreate(dataset, "delta_t", H5T_IEEE_F64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
		H5Awrite(h5a_time, H5T_IEEE_F64LE, &options.delta_t[0]);
		H5Aclose(h5a_time);
		
		//clean up
		H5Dclose(dataset); 
		H5Pclose(dset_create_props);
		H5Sclose(dataspace);
	}
	
	return true;
}

bool Bh3BinaryFile::get_snapshot(double &time, vector<ComplexGrid> &k, int num)
{
	if(m != in)
	{
		cout << "file "<< filename.c_str() << "is not in read mode" << endl;
		return false;
	}

    k.assign(options.delta_t.size()+1, ComplexGrid(options.grid[0],options.grid[1],options.grid[2],options.grid[3]));
	
	stringstream set_name;
 	set_name << num;
    
	hid_t dataset, h5a_time;
	
	dataset = H5Dopen(h5_complexgriddata, (set_name.str()).c_str(), H5P_DEFAULT);
	
	if (dataset < 0)
	{
		cout << "error while opening complex grid snapshot "<< num << endl;
		return false;
	}
		
	if (k.size() == 1)
	{
		H5Dread(dataset,  H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, (double *)k[0].get_address());
    }
	else
	{
		for (int i = 0; i < k.size(); i++)
		{
			hsize_t offset[1] = {i*2*options.grid[0]*options.grid[1]*options.grid[2]*options.grid[3]}; //prepare partial reading operation from dataset by defining an offset for destination
			hsize_t count[1] = {2*options.grid[0]*options.grid[1]*options.grid[2]*options.grid[3]}; // and the space which will be copied to
			hid_t dataspace = H5Dget_space(dataset); //get dataspace in file 
			hid_t memspace = H5Screate_simple(1, count, NULL); // define full space in memory which will be copied to
			H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL); //select part in dataset which will be read from
			
			H5Dread (dataset, H5T_IEEE_F64LE, memspace, dataspace, H5P_DEFAULT, (double *)k[i].get_address()); //now read  

			H5Sclose(memspace);
			H5Sclose(dataspace);
		}	
	}
	h5a_time = H5Aopen(dataset, "time", H5P_DEFAULT);
	H5Aread(h5a_time, H5T_IEEE_F64LE, &time);

	H5Aclose(h5a_time);
	H5Dclose(dataset);
	
	return true;
}


bool Bh3BinaryFile::get_snapshot(double &time, vector<RealGrid> &k, int num)
{
	if(m != in)
	{
		cout << "file "<< filename.c_str() << "is not in read mode" << endl;
		return false;
	}

    k.assign(options.delta_t.size()+1, RealGrid(options.grid[0],options.grid[1],options.grid[2],options.grid[3]));
	
	stringstream set_name;
 	set_name << num;
    
	hid_t dataset, h5a_time;
	
	dataset = H5Dopen(h5_realgriddata, (set_name.str()).c_str(), H5P_DEFAULT);
	
	if (dataset < 0)
	{
		cout << "error while opening real grid snapshot "<< num << endl;
		return false;
	}
		
	if (k.size() == 1)
	{
		H5Dread(dataset,  H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, (double *)k[0].get_address());
	}
	else
	{
		for (int i = 0; i < k.size(); i++)
		{
			hsize_t offset[1] = {i*k[i].int_dim()*k[i].fft_width()*k[i].fft_height()*k[i].fft_depth()}; //prepare partial reading operation from dataset by defining an offset for destination
			hsize_t count[1] = {k[i].int_dim()*k[i].fft_width()*k[i].fft_height()*k[i].fft_depth()}; // and the space which will be copied to
			hid_t dataspace = H5Dget_space(dataset); //get dataspace in file 
			hid_t memspace = H5Screate_simple(1, count, NULL); // define full space in memory which will be copied to
			H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL); //select part in dataset which will be read from
			
			H5Dread (dataset, H5T_IEEE_F64LE, memspace, dataspace, H5P_DEFAULT, (double *)k[i].get_address()); //now read  

			H5Sclose(memspace);
			H5Sclose(dataspace);
		}	
	}
	h5a_time = H5Aopen(dataset, "time", H5P_DEFAULT);
	H5Aread(h5a_time, H5T_IEEE_F64LE, &time);

	H5Aclose(h5a_time);
	H5Dclose(dataset);
	
	return true;
}

bool initialize_binary_dir(string &dname, const PathOptions &opt)
{
	stringstream dirname;
	dirname << dname << "_" << "TS" << opt.timestepsize
	<< "_C" << opt.grid[0]
	<< "_G" << opt.grid[1] << "_" << opt.grid[2] << "_" << opt.grid[3]
	<< "_N" << opt.N
	<< "_U" << opt.U
	<< "_KL" << opt.klength[0] << "_" << opt.klength[1] << "_" << opt.klength[2];
	// Neuen Dateinamen garantieren

	DIR *dir = opendir(".");

	if(dir==NULL)
	{
		cout << "Couldn't open current directory!\n";
		return false;
	}

	dirent *entry;
	int run=0;
	char format[512];
	strcpy(format,dirname.str().c_str());
	strcat(format,"Run%d");
	while ( (entry=readdir(dir)) != NULL )
	{
		int temp;
		int result = sscanf(entry->d_name, format, &temp);
		if((result == 1) && (temp > run))
			run = temp;
	}
	run++;
	dirname << "Run" << run;
	closedir(dir);

	if(mkdir(dirname.str().c_str(),0755) != 0)
	{
		cout << "Couldn't create directory " << dirname.str() << " !\n";
		return false;
	}
	dirname << "/";
	ofstream description;
	try
	{
		description.open((dirname.str() + "Bh3Description.txt").c_str());
	}
	catch (exception &e)
	{
		cout << "Couldn't create one of the files!" << endl;
		return false;
	}

	stringstream ds;
	ds 		<< "Name			: Bh3-512" << endl
	<< "Time-StepSize		: " << opt.timestepsize << endl
	<< "Gridsize			: " << opt.grid[1] << "," << opt.grid[2] << "," << opt.grid[3] << endl
	<< "Number of particles		: " << opt.N << endl
	<< "Interaction strength (U)	: " << opt.U << endl
	<< "Comments			: Production run: vortex counting";
	description << ds.str();
	description.close();
	dname = dirname.str();
	return true;
}

bool initialize_path_options(int argc, char** argv, PathOptions &opt, vector<double> &snapshot_times, void *gen_times(vector<double> &snapshot_times), bool verbose)
{
	config_t conf;
	config_setting_t *setting;
	config_init(&conf);

	string config_file; 
	
	if (argc > 1)
		config_file = argv[1];
	else
		config_file = "/home/karl/myPyModules/bh3default.conf";
	
	
	if (config_read_file(&conf, config_file.c_str()) == CONFIG_FALSE)
	{
		cout << "error " << config_error_text(&conf) << " occurred during read process" << endl;
		config_destroy(&conf);
		return false;
	}
	
	setting = config_lookup(&conf, "PathOptions");
	
	if (setting == NULL)
	{
		cout << "error: Setting \"PathOptions\" not found in " << config_setting_source_file(setting) << endl;
		config_destroy(&conf);
		return false;
	}
	
	if (config_setting_lookup_float(setting, "timestepsize", &opt.timestepsize) == CONFIG_FALSE)
		cout << "setting \"timestepsize\" not found" << endl;
	
	if (config_setting_lookup_float(setting, "U", &opt.U)== CONFIG_FALSE)
		cout << "setting \"U\" not found" << endl ;	
	
	if (config_setting_lookup_float(setting, "N", &opt.N)== CONFIG_FALSE)
		cout << "setting \"N\" not found" << endl ;	
	
	if (config_setting_lookup_int(setting, "internal_dimension", &opt.grid[0])== CONFIG_FALSE)
		cout << "setting \"internal_dimension\" not found" << endl ;	
	
	setting = config_lookup(&conf, "PathOptions.grid_dims");
	if (setting == NULL)
	{
		cout << "error: Setting \"PathOptions.grid_dims\" not found in " << config_setting_source_file(setting)<< endl;
		config_destroy(&conf);
		return false;
	}
	for (int i = 0; i < 3; i++)
	{
		opt.grid[i+1] = config_setting_get_int_elem (setting, i);
	}
	
	setting = config_lookup(&conf, "PathOptions.klengths");
	if (setting == NULL)
	{
		cout << "error: Setting \"PathOptions.klength\" not found in " << config_setting_source_file(setting)<< endl;
		config_destroy(&conf);
		return false;
	}
	for (int i = 0; i < 3; i++)
		opt.klength[i] = config_setting_get_float_elem (setting, i);
	
	
	setting = config_lookup(&conf, "PathOptions.gmatrix");
	if (setting == NULL)
	{
		cout << "error: Setting \"PathOptions.gmatrix\" not found in " << config_setting_source_file(setting)<< endl;
		config_destroy(&conf);
		return false;
	}
	int index = config_setting_length(setting);
	if(index != opt.grid[0]*opt.grid[0])
		cout << "warning: dimension of gmatrix in file does not match the internal dimension" << endl;
	opt.g.resize(index);
	for (int i = 0; i<index; i++)
		opt.g[i] = config_setting_get_float_elem (setting, i);
	
	setting = config_lookup(&conf, "PathOptions.delta_t");
	if (setting != NULL)
	{
		index = config_setting_length(setting);
		opt.delta_t.resize(index);
		for (int i = 0; i<index; i++)
			opt.delta_t[i] = config_setting_get_float_elem (setting, i);
	}
	else
		opt.delta_t.resize(0);
	
	//print PathOptions if desired
	if(verbose)
		cout << opt << endl;
	
	//generate Snapshots either from given function gen_times
	if (gen_times != NULL)
		gen_times(snapshot_times);
	//or from config file
	else
	{
		setting = config_lookup(&conf, "SnapshotTimes");
		if (setting != NULL)
		{
			index = config_setting_length(setting);
			snapshot_times.resize(index);
			for (int i = 0; i<index; i++)
			{
				snapshot_times[i] = config_setting_get_float_elem (setting, i);
			}
		}
		//if no snapshots are provided, neither from config file nor from function, make snapshot from initial field 
		else
		{
			snapshot_times.resize(1);
			snapshot_times[0] = 0.;
		}
	}  
	
	config_destroy(&conf);
	return true;
	
}


list<string> get_file_list(const list<string> args)
{
	list<string> files;
	PathOptions options;
	
	// iterate the arguments and check them for valid Bh3-files
	for(list<string>::const_iterator bin = args.begin(); bin != args.end(); ++bin)
	{
		struct stat buffer;
		lstat((*bin).c_str(), &buffer);
		if(S_ISREG(buffer.st_mode))
		{
			Bh3BinaryFile b(*bin, options, Bh3BinaryFile::in);
			if(b.get_num_snapshots_c() > 0 || b.get_num_snapshots_r() > 0)
				files.push_back(*bin);
		}
		else
		{
			string bindir = *bin;
			if(bindir[bindir.length() - 1] != '/')
				bindir = bindir + "/";
			
			// Dateiname der Binaerdateien fuer Analyse bestimmen
			DIR *dir = opendir(bindir.c_str());
			if (dir == NULL)
			{
				cout << "Couldn't open directory with snapshots!" << endl;
				return list<string>();
			}
			dirent *entry;
			while( (entry = readdir(dir)) != NULL)
			{
				string filename = bindir + entry->d_name;
				Bh3BinaryFile b(filename, options, Bh3BinaryFile::in);
				if(b.get_num_snapshots_c() > 0 || b.get_num_snapshots_r() > 0)
					files.push_back(filename);
			}
			closedir(dir);
		}
	}
	return files;
}

void convert_bin_file(const string &filename, bool forward, bool compression)
{
	PathOptions options;
	Bh3BinaryFile in(filename, options, Bh3BinaryFile::in);
	options = in.get_options();
	int num_snapshots_c = in.get_num_snapshots_c();
	int num_snapshots_r = in.get_num_snapshots_r();
	
	Bh3BinaryFile* out = new Bh3BinaryFile((filename + ".tmp").c_str(), options, Bh3BinaryFile::out);
	double time;
	vector<ComplexGrid> c(options.delta_t.size()+1);
	vector<RealGrid> r(options.delta_t.size()+1);
	
	for(int t = 0; t < num_snapshots_r; t++)
	{
		in.get_snapshot(time, r, t);
		for(int i = 0; i < r.size(); i++)
			RealGrid::fft(r[i], r[i], forward);
		out->append_snapshot(time, r, compression);
	}

	for(int t = 0; t < num_snapshots_c; t++)
	{
		in.get_snapshot(time, c, t);
		for(int i = 0; i < c.size(); i++)
			ComplexGrid::fft(c[i], c[i], forward);
		out->append_snapshot(time, c, compression);
	}

	out->close();
	
	stringstream command1, command2;
	command1 << "rm " << filename;
	system(command1.str().c_str());
	command2 << "mv " << filename << ".tmp " << filename;
	system(command2.str().c_str());
}