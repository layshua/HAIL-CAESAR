//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// LSDRasterSpectral
// Land Surface Dynamics StatsTools
//
// An object within the University
//  of Edinburgh Land Surface Dynamics group topographic toolbox
//  for manipulating
//  and analysing raster data, with a particular focus on topography
//
// Developed by:
//  Simon M. Mudd
//  Martin D. Hurst
//  David T. Milodowski
//  Stuart W.D. Grieve
//  Declan A. Valters
//  Fiona Clubb
//
// Copyright (C) 2013 Simon M. Mudd 2013
//
// Developer can be contacted by simon.m.mudd _at_ ed.ac.uk
//
//    Simon Mudd
//    University of Edinburgh
//    School of GeoSciences
//    Drummond Street
//    Edinburgh, EH8 9XP
//    Scotland
//    United Kingdom
//
// This program is free software;
// you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation;
// either version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY;
// without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the
// GNU General Public License along with this program;
// if not, write to:
// Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301
// USA
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// LSDRaster.cpp
// cpp file for the LSDRaster object
// LSD stands for Land Surface Dynamics
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// This object is written by
// Simon M. Mudd, University of Edinburgh
// David T. Milodowski, University of Edinburgh
// Martin D. Hurst, British Geological Survey
// Fiona Clubb, University of Edinburgh
// Stuart Grieve, University of Edinburgh
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Version 1.0.0		16/07/2013
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// change log
// MASSIVE MERGE: Starting version 1.0.0 on 15/07/2013
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//-----------------------------------------------------------------
//DOCUMENTATION URL: http://www.geos.ed.ac.uk/~s0675405/LSD_Docs/
//-----------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <queue>
#include <math.h>
#include <string.h>
#include "TNT/tnt.h"
#include "TNT/jama_lu.h"
#include "TNT/jama_eig.h"
#include "LSDRaster.hpp"
#include "LSDStatsTools.hpp"
#include "LSDIndexRaster.hpp"
using namespace std;
using namespace TNT;
using namespace JAMA;

#ifndef LSDRaster_CPP
#define LSDRaster_CPP

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// operators
// SMM, 2012
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster& LSDRaster::operator=(const LSDRaster& rhs)
 {
  if (&rhs != this)
   {
    create(rhs.get_NRows(),rhs.get_NCols(),rhs.get_XMinimum(),rhs.get_YMinimum(),
           rhs.get_DataResolution(),rhs.get_NoDataValue(),rhs.get_RasterData());
   }
  return *this;
 }

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// the create function. This is default and throws an error
// SMM 2012
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::create()
{
	cout << "LSDRaster line 64 Warning you have an empty LSDRaster!" << endl;
	//exit(EXIT_FAILURE);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// this creates a raster using an infile
// SMM 2012
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::create(string filename, string extension)
{
	read_raster(filename,extension);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// this creates a raster filled with no data values
// SMM 2012
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::create(int nrows, int ncols, double xmin, double ymin,
            double cellsize, double ndv, Array2D<double> data)
{
	NRows = nrows;
	NCols = ncols;
	XMinimum = xmin;
	YMinimum = ymin;
	DataResolution = cellsize;
	NoDataValue = ndv;

	RasterData = data.copy();

	if (RasterData.dim1() != NRows)
	{
		cout << "LSDRaster line 89 dimension of data is not the same as stated in NRows!" << endl;
		exit(EXIT_FAILURE);
	}
	if (RasterData.dim2() != NCols)
	{
		cout << "LSDRaster line 94 dimension of data is not the same as stated in NRows!" << endl;
		exit(EXIT_FAILURE);
	}

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// This function reads a DEM
// One has to provide both the filename and the extension
// the '.' between the filename and extension is not included
// for example, if the full filename is test.asc
// then
// filename = "test"
// and
// ext = "asc"
// The full filename could also be "test.01.asc"
// so filename would be "test.01"
// and ext would again be "asc"
//
// SMM 2012
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::read_raster(string filename, string extension)
{
	string string_filename;
	string dot = ".";
	string_filename = filename+dot+extension;
	cout << "The filename is " << string_filename << endl;


	if (extension == "asc")
	{
		// open the data file
		ifstream data_in(string_filename.c_str());

		//Read in raster data
		string str;			// a temporary string for discarding text

		// read the georeferencing data and metadata
		data_in >> str >> NCols >> str >> NRows
			    >> str >> XMinimum >> str >> YMinimum
		   		>> str >> DataResolution
			    >> str >> NoDataValue;

		cout << "Loading asc file; NCols: " << NCols << " NRows: " << NRows << endl
		     << "X minimum: " << XMinimum << " YMinimum: " << YMinimum << endl
		     << "Data Resolution: " << DataResolution << " and No Data Value: "
		     << NoDataValue << endl;

		// this is the array into which data is fed
		Array2D<double> data(NRows,NCols,NoDataValue);

		// read the data
		for (int i=0; i<NRows; ++i)
		{
			for (int j=0; j<NCols; ++j)
			{
				data_in >> data[i][j];
			}
		}
		data_in.close();

		// now update the objects raster data
		RasterData = data.copy();
	}
	else if (extension == "flt")
	{
		// float data (a binary format created by ArcMap) has a header file
		// this file must be opened first
		string header_filename;
		string header_extension = "hdr";
		header_filename = filename+dot+header_extension;

		ifstream ifs(header_filename.c_str());
		if( ifs.fail() )
		{
			cout << "\nFATAL ERROR: the header file \"" << header_filename
				 << "\" doesn't exist" << std::endl;
			exit(EXIT_FAILURE);
		}
		else
		{
			string str;
			ifs >> str >> NCols >> str >> NRows
				>> str >> XMinimum >> str >> YMinimum
				>> str >> DataResolution
				>> str >> NoDataValue;
		}
		ifs.close();

		cout << "Loading asc file; NCols: " << NCols << " NRows: " << NRows << endl
			 << "X minimum: " << XMinimum << " YMinimum: " << YMinimum << endl
		     << "Data Resolution: " << DataResolution << " and No Data Value: "
		     << NoDataValue << endl;

		// this is the array into which data is fed
		Array2D<double> data(NRows,NCols,NoDataValue);

		// now read the DEM, using the binary stream option
		ifstream ifs_data(string_filename.c_str(), ios::in | ios::binary);
		if( ifs_data.fail() )
		{
			cout << "\nFATAL ERROR: the data file \"" << string_filename
			     << "\" doesn't exist" << endl;
			exit(EXIT_FAILURE);
		}
		else
		{
			float temp;
			for (int i=0; i<NRows; ++i)
			{
				for (int j=0; j<NCols; ++j)
				{
					ifs_data.read(reinterpret_cast<char*>(&temp), sizeof(temp));
					data[i][j] = double(temp);
				}
			}
		}
		ifs_data.close();

		// now update the objects raster data
		RasterData = data.copy();
	}
	else
	{
		cout << "You did not enter and approprate extension!" << endl
				  << "You entered: " << extension << " options are .flt and .asc" << endl;
		exit(EXIT_FAILURE);
	}


}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// write_raster
// this function writes a raster. One has to give the filename and extension
// currently the options are for .asc and .flt files
//
// SMM 2012
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::write_raster(string filename, string extension)
{
	string string_filename;
	string dot = ".";
	string_filename = filename+dot+extension;
	cout << "The filename is " << string_filename << endl;

	// this first bit of logic is for the asc file.
	if (extension == "asc")
	{
		// open the data file
		ofstream data_out(string_filename.c_str());

		if( data_out.fail() )
		{
			cout << "\nFATAL ERROR: unable to write to " << string_filename << endl;
			exit(EXIT_FAILURE);
		}

		data_out <<  "ncols         " << NCols
				<< "\nnrows         " << NRows
				<< "\nxllcorner     " << setprecision(14) << XMinimum
				<< "\nyllcorner     " << setprecision(14) << YMinimum
				<< "\ncellsize      " << DataResolution
				<< "\nNODATA_value  " << NoDataValue << endl;


		for (int i=0; i<NRows; ++i)
		{
			for (int j=0; j<NCols; ++j)
			{
				data_out << setprecision(6) << RasterData[i][j] << " ";
			}
			if (i != NRows-1) data_out << endl;
		}
		data_out.close();

	}
	else if (extension == "flt")
	{
		// float data (a binary format created by ArcMap) has a header file
		// this file must be opened first
		string header_filename;
		string header_extension = "hdr";
		header_filename = filename+dot+header_extension;

		ofstream header_ofs(header_filename.c_str());
		string str;
		header_ofs <<  "ncols         " << NCols
			<< "\nnrows         " << NRows
			<< "\nxllcorner     " << setprecision(14) << XMinimum
			<< "\nyllcorner     " << setprecision(14) << YMinimum
			<< "\ncellsize      " << DataResolution
			<< "\nNODATA_value  " << NoDataValue
			<< "\nbyteorder     LSBFIRST" << endl;
		header_ofs.close();

		// now do the main data
		ofstream data_ofs(string_filename.c_str(), ios::out | ios::binary);
		float temp;
		for (int i=0; i<NRows; ++i)
		{
			for (int j=0; j<NCols; ++j)
			{
				temp = float(RasterData[i][j]);
				data_ofs.write(reinterpret_cast<char *>(&temp),sizeof(temp));
			}
		}
		data_ofs.close();
	}
	else
	{
		cout << "You did not enter and approprate extension!" << endl
				  << "You entered: " << extension << " options are .flt and .asc" << endl;
		exit(EXIT_FAILURE);
	}


}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Make LSDRaster object using a 'template' raster and an Array2D of data.
// SWDG 29/8/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::LSDRasterTemplate(Array2D<double> InputData){

  //do a dimensions check and exit on failure
  if (InputData.dim1() == NRows && InputData.dim2() == NCols){
    LSDRaster OutputRaster(NRows, NCols, XMinimum, YMinimum, DataResolution, NoDataValue, InputData);
    return OutputRaster;
  }
  else{
   	cout << "Array dimensions do not match template LSDRaster object" << endl;
		exit(EXIT_FAILURE);
  }

}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// This function generates a hillshade raster using the algorithm outlined in
// Burrough and McDonnell Principles of GIS 1990 and in the ArcMap web help
// http://edndoc.esri.com/arcobjects/9.2/net/shared/geoprocessing/
// spatial_analyst_tools/how_hillshade_works.htm
//
// Takes 3 doubles, representing the altitude of the illumination source in
// degrees, the azimuth of the illumination source in degrees and the z factor.
//
// Default values are altitude = 45, azimuth = 315, z_factor = 1
//
// Outputs an LSDRaster object.
//
// SWDG, February 2013
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::hillshade(double altitude, double azimuth, double z_factor)
{
    double PI = 3.14159265;

    //print parameters to screen
    cout << "Hillshading with altitude: " << altitude
    << ", azimuth: " << azimuth << " and z-factor: " << z_factor << endl;

    //create output array
    Array2D<double> hillshade(NRows,NCols,NoDataValue);

    //convert zenith and azimuth into radians for calculation
    double zenith_rad = (90 - altitude) * PI / 180.0;
    double azimuth_math = 360-azimuth + 90;
    if (azimuth_math >= 360.0) azimuth_math = azimuth_math - 360;
    double azimuth_rad = azimuth_math * PI /180.0;

    //calculate hillshade value for every non nodata value in the input raster
    for (int i = 1; i < NRows-1; ++i){
        for (int j = 1; j < NCols-1; ++j){
            double slope_rad = 0;
            double aspect_rad = 0;
            double dzdx = 0;
            double dzdy = 0;

            if (RasterData[i][j] != NoDataValue){
                dzdx = ((RasterData[i][j+1] + 2*RasterData[i+1][j] + RasterData[i+1][j+1]) -
                       (RasterData[i-1][j-1] + 2*RasterData[i-1][j] + RasterData[i-1][j+1]))
                        / (8 * DataResolution);
                dzdy = ((RasterData[i-1][j+1] + 2*RasterData[i][j+1] + RasterData[i+1][j+1]) -
                       (RasterData[i-1][j-1] + 2*RasterData[i][j-1] + RasterData[i+1][j-1]))
                       / (8 * DataResolution);

                slope_rad = atan(z_factor * sqrt((dzdx*dzdx) + (dzdy*dzdy)));

                if (dzdx != 0){
                    aspect_rad = atan2(dzdy, (dzdx*-1));
                    if (aspect_rad < 0) aspect_rad = 2*PI + aspect_rad;
                }
                else{
                    if (dzdy > 0) aspect_rad = PI/2;
                    else if (dzdy < 0) aspect_rad = 2 * PI - PI/2;
                    else aspect_rad = aspect_rad;
                }
                hillshade[i][j] = 255.0 * ((cos(zenith_rad) * cos(slope_rad)) +
                                  (sin(zenith_rad) * sin(slope_rad) *
                                  cos(azimuth_rad - aspect_rad)));

                if (hillshade[i][j] < 0) hillshade[i][j] = 0;
            }
        }
    }
    //create LSDRaster hillshade object
    LSDRaster hillshade_raster(NRows, NCols, XMinimum, YMinimum, DataResolution,
                               NoDataValue, hillshade);

    return hillshade_raster;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// This function generates a hillshade derivative raster using the algorithm outlined in
// Codilean (2006), identifying areas in shadow as 1 and all other values as 0.
//
// Is interfaced through LSDRaster::TopoShield and should not be called directly,
// to generate a hillshade use LSDRaster::hillshade instead.
//
// Takes 2 ints, representing theta, the zenith angle of the illumination source in
// degrees and the azimuth angle, phi, of the illumination source in degrees.
//
// Outputs an Array2D of doubles.
//
// SWDG, 11/4/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Array2D<double> LSDRaster::Shadow(int theta, int phi)
{
    double PI = 3.14159265;

    //create array of input data and an output array of same dimensions
    Array2D<double> data = RasterData;
    Array2D<double> hillshade(NRows,NCols,NoDataValue);

    //convert zenith and azimuth into radians for calculation
    double zenith_rad = (90 - theta) * PI / 180.0;
    double azimuth_math = 360-phi + 90;
    if (azimuth_math >= 360.0) azimuth_math = azimuth_math - 360;
    double azimuth_rad = azimuth_math * PI /180.0;

    //calculate hillshade value for every non nodata value in the input raster
    for (int i = 1; i < NRows-1; ++i){
        for (int j = 1; j < NCols-1; ++j){
            double slope_rad = 0;
            double aspect_rad = 0;
            double dzdx = 0;
            double dzdy = 0;

            if (data[i][j] != NoDataValue){
                dzdx = ((data[i][j+1] + 2*data[i+1][j] + data[i+1][j+1]) -
                       (data[i-1][j-1] + 2*data[i-1][j] + data[i-1][j+1]))
                        / (8 * DataResolution);
                dzdy = ((data[i-1][j+1] + 2*data[i][j+1] + data[i+1][j+1]) -
                       (data[i-1][j-1] + 2*data[i][j-1] + data[i+1][j-1]))
                       / (8 * DataResolution);

                slope_rad = atan(sqrt((dzdx*dzdx) + (dzdy*dzdy)));

                if (dzdx != 0){
                    aspect_rad = atan2(dzdy, (dzdx*-1));
                    if (aspect_rad < 0) aspect_rad = 2*PI + aspect_rad;
                }
                else{
                    if (dzdy > 0) aspect_rad = PI/2;
                    else if (dzdy < 0) aspect_rad = 2 * PI - PI/2;
                    else aspect_rad = aspect_rad;
                }

                hillshade[i][j] = acos(((sin(zenith_rad) * cos(slope_rad)) +
                                  (sin(zenith_rad) * sin(slope_rad) *
                                  cos(azimuth_rad - aspect_rad))))*180/PI;

                if (hillshade[i][j] > 90) hillshade[i][j] = 1;
                else hillshade[i][j] = 0;
            }

        }
    }

    //create LSDRaster hillshade object
    //LSDRaster hillshade_raster(NRows, NCols, XMinimum, YMinimum, DataResolution,
    //                           NoDataValue, hillshade);

    return hillshade;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// This function generates a topographic sheilding raster using the algorithm outlined in
// Codilean (2006), creating a raster of values between 0 and 1 whihc can be used as a
// scaling factor in Cosmo analysis.
//
// Goes further than the original algorithm allowing a theoretical theta, phi pair of
// 1,1 to be supplied and although this will increase the computatin time significantly,
// it is much faster than the original Avenue and VBScript implementations.
//
// Takes 2 ints, representing the theta, phi paring required. Codilean (2006) used 5,5
// as the standard values, but in reality values of 10,15 are often preferred to save
// processing time. **phi_step must be a factor of 360**
//
// Outputs an LSDRaster
//
// SWDG, 11/4/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::TopoShield(int theta_step, int phi_step){

  //calculate parameter of scaling calculation
  int phi_factor = 360/phi_step;

  double MaxFactor = 0;

  for(int theta = 10; theta < 90; theta += theta_step){
    MaxFactor += pow(sin(rad(theta)),3.3) * phi_factor;
  }

  //calculate maximum scaling factor and write it to an array
  MaxFactor += pow(sin(rad(90)),3.3);
  Array2D<double> MaxFactorArray(NRows,NCols,MaxFactor);

  //Calculate first shadow with theta value of 90 and scale it
  Array2D<double> Scaler90(NRows,NCols, pow(sin(rad(90)),3.3));
  Array2D<double> FinalArray = Shadow(0,90) * Scaler90;

  //loop through all the theta, phi pairs and increment the FinalArray with the scaled values
  for(int theta = 10; theta < 90; theta += theta_step){
    for(int phi = 0; phi < 360; phi += phi_step){

      cout << flush <<  "\tTheta = " << theta << ", Phi = " << phi << "           \r";

      Array2D<double> TempArray = Shadow(theta,phi);
      Array2D<double> Scaler(NRows, NCols, pow(sin(rad(theta)),3.3));
      FinalArray += TempArray * Scaler;
    }
  }

 //create array of ones needed in sheilding calculation
  Array2D<double> Ones(NRows,NCols,1);

  //Shielding factor calculation
  Array2D<double> ShFactor = Ones - (FinalArray/MaxFactorArray);

  //deal with nodata values on border of dem - more efficient than doing it in the hillshade function 10s of times.
  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      if (ShFactor[i][j] > 10){             //arbitrary number. But don't want to miss any values >1
        ShFactor[i][j] = NoDataValue;       //as they may indicate an error in the above calculations.
      }
    }
  }

   //flush output onto a new line
   cout << endl;

  //write LSDRaster
  LSDRaster Shielding(NRows, NCols, XMinimum, YMinimum, DataResolution, NoDataValue, ShFactor);
  return Shielding;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// this looks for isolated instances of nodata and fills them
//
// Not sure about author, I think MDH (SMM comment) 2012
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::check_isolated_nodata()
{
	for (int row=0; row<NRows; ++row)
	{
		for(int col=0; col<NCols; ++col)
		{
			if(RasterData[row][col] < 0)
			{
				cout << "LSDRaster::check_isolated_nodata stargine data point: row: "
				     << row << " col: " << col << " data: " << RasterData[row][col];
				RasterData[row][col] = NoDataValue;
			}

			if(RasterData[row][col] == NoDataValue)
			{
				cout << "LSDRaster::check_isolated_nodata found nodata: row: "
				     << row << " col: " << col << " data: " << RasterData[row][col];
			}

		}
	}
	cout << "Done!" << endl;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// calculate_polyfit_coefficient_matrices
//
// this function calcualtes 6 coefficient matrices that allow the user to
// then calcualte slope, curvature, aspect, a classification for finding saddles and peaks
// and other metrics
//
// The coefficient matrices are overwritten during the running of this member function
//
// DTM
//
// Updated 15/07/2013 to use a circular mask for surface fitting. DTM
// Updated 24/07/2013 to check window_radius size and correct values below data resolution. SWDG
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::calculate_polyfit_coefficient_matrices(double window_radius,
										Array2D<double>& a, Array2D<double>& b,
										Array2D<double>& c, Array2D<double>& d,
										Array2D<double>& e, Array2D<double>& f)
{


	// catch if the supplied window radius is less than the data resolution and set
	// it to equal the data resolution - SWDG
  if (window_radius < DataResolution){
    cout << "Supplied window radius: " << window_radius << " is less than the data resolution: " <<
    DataResolution << ".\nWindow radius has been set to data resolution." << endl;
    window_radius = DataResolution;
  }

  // this fits a polynomial surface over a kernel window. First, perpare the kernel
	int kr = int(ceil(window_radius/DataResolution));           // Set radius of kernel
	int kw=2*kr+1;                    						// width of kernel

	Array2D<double> data_kernel(kw,kw,NoDataValue);
	Array2D<double> x_kernel(kw,kw,NoDataValue);
	Array2D<double> y_kernel(kw,kw,NoDataValue);
	Array2D<int> mask(kw,kw,0);

	// reset the a,b,c,d,e and f matrices (the coefficient matrices)
	Array2D<double> temp_coef(NRows,NCols,0.0);
	a = temp_coef.copy();
	b = temp_coef.copy();
	c = temp_coef.copy();
	d = temp_coef.copy();
	e = temp_coef.copy();
	f = temp_coef.copy();

	// scale kernel window to resolution of DEM, and translate coordinates to be
	// centred on cell of interest (the centre cell)
	double x,y,zeta,radial_dist;
	for(int i=0;i<kw;++i)
	{
	    for(int j=0;j<kw;++j)
	    {
	      	x_kernel[i][j]=(i-kr)*DataResolution;
	      	y_kernel[i][j]=(j-kr)*DataResolution;

			// Build circular mask
			// distance from centre to this point.
			radial_dist = sqrt(y_kernel[i][j]*y_kernel[i][j] + x_kernel[i][j]*x_kernel[i][j]);

        	if (floor(radial_dist) <= window_radius)
        	{
				mask[i][j] = 1;
			}
      	}
	}

	// FIT POLYNOMIAL SURFACE BY LEAST SQUARES REGRESSION AND USE COEFFICIENTS TO
	// DETERMINE TOPOGRAPHIC METRICS

	// Have N simultaneous linear equations, and N unknowns.
	// => b = Ax, where x is a 1xN array containing the coefficients we need for
	// surface fitting.
	// A is constructed using different combinations of x and y, thus we only need
	// to compute this once, since the window size does not change.
	// For 2nd order surface fitting, there are 6 coefficients, therefore A is a
	// 6x6 matrix
	Array2D<double> A(6,6);
	for (int i=0; i<kw; ++i)
	{
		for (int j=0; j<kw; ++j)
		{
			if (mask[i][j] == 1)
      		{
        		x = x_kernel[i][j];
  				y = y_kernel[i][j];

  				// Generate matrix A
  				A[0][0] += pow(x,4);
  				A[0][1] += pow(x,2)*pow(y,2);
  				A[0][2] += pow(x,3)*y;
  				A[0][3] += pow(x,3);
  				A[0][4] += pow(x,2)*y;
  				A[0][5] += pow(x,2);
  				A[1][0] += pow(x,2)*pow(y,2);
  				A[1][1] += pow(y,4);
  				A[1][2] += x*pow(y,3);
  				A[1][3] += x*pow(y,2);
  				A[1][4] += pow(y,3);
  				A[1][5] += pow(y,2);
  				A[2][0] += pow(x,3)*y;
  				A[2][1] += x*pow(y,3);
  				A[2][2] += pow(x,2)*pow(y,2);
  				A[2][3] += pow(x,2)*y;
  				A[2][4] += x*pow(y,2);
  				A[2][5] += x*y;
  				A[3][0] += pow(x,3);
  				A[3][1] += x*pow(y,2);
  				A[3][2] += pow(x,2)*y;
  				A[3][3] += pow(x,2);
  				A[3][4] += x*y;
  				A[3][5] += x;
  				A[4][0] += pow(x,2)*y;
  				A[4][1] += pow(y,3);
  				A[4][2] += x*pow(y,2);
  				A[4][3] += x*y;
  				A[4][4] += pow(y,2);
  				A[4][5] += y;
  				A[5][0] += pow(x,2);
  				A[5][1] += pow(y,2);
  				A[5][2] += x*y;
  				A[5][3] += x;
  				A[5][4] += y;
  				A[5][5] += 1;
			}
		}
	}

	// Move window over DEM, fitting 2nd order polynomial surface to the
	// elevations within the window.
	cout << "\n\tRunning 2nd order polynomial fitting" << endl;
	cout << "\t\tDEM size = " << NRows << " x " << NCols << endl;
	int ndv_present = 0;

	for(int i=0;i<NRows;++i)
	{
		cout << "\tRow = " << i+1 << " / " << NRows << "    \r";
		for(int j=0;j<NCols;++j)
		{
			// Avoid edges
			if((i-kr < 0) || (i+kr+1 > NRows) || (j-kr < 0) || (j+kr+1 > NCols))
			{
				a[i][j] = NoDataValue;
				b[i][j] = NoDataValue;
				c[i][j] = NoDataValue;
				d[i][j] = NoDataValue;
				e[i][j] = NoDataValue;
				f[i][j] = NoDataValue;
			}
			// Avoid nodata values
			else if(RasterData[i][j]==NoDataValue)
			{
				a[i][j] = NoDataValue;
				b[i][j] = NoDataValue;
				c[i][j] = NoDataValue;
				d[i][j] = NoDataValue;
				e[i][j] = NoDataValue;
				f[i][j] = NoDataValue;
			}
			else
			{
				// clip DEM
				//zeta_sampler=zeta.copy();
				for(int i_kernel=0;i_kernel<kw;++i_kernel)
				{
			  		for(int j_kernel=0;j_kernel<kw;++j_kernel)
			  		{
						data_kernel[i_kernel][j_kernel] =
									RasterData[i-kr+i_kernel][j-kr+j_kernel];
						// check for nodata values nearby
						if(data_kernel[i_kernel][j_kernel]==NoDataValue)
						{
							ndv_present=1;
						}
			  		}
				}

				// Fit polynomial surface, avoiding nodata values
				if(ndv_present == 0)  // test for nodata values within the selection
				{
					Array1D<double> bb(6,0.0);
					Array1D<double> coeffs(6);
					for (int krow=0; krow<kw; ++krow)
					{
						for (int kcol=0; kcol<kw; ++kcol)
						{
							if (mask[krow][kcol] == 1)
              				{
                				x = x_kernel[krow][kcol];
					      		y = y_kernel[krow][kcol];
					      		zeta = data_kernel[krow][kcol];
					      		// Generate vector bb
					      		bb[0] += zeta*x*x;
					      		bb[1] += zeta*y*y;
					      		bb[2] += zeta*x*y;
					      		bb[3] += zeta*x;
					      		bb[4] += zeta*y;
					      		bb[5] += zeta;
					      	}		// end mask
            			}			// end kernal column
					}				// end kernal row
					// Solve matrix equations using LU decomposition using the TNT JAMA package:
					// A.coefs = b, where coefs is the coefficients vector.
					LU<double> sol_A(A);  // Create LU object
					coeffs = sol_A.solve(bb);

			  		a[i][j]=coeffs[0];
			  		b[i][j]=coeffs[1];
			  		c[i][j]=coeffs[2];
			  		d[i][j]=coeffs[3];
			  		e[i][j]=coeffs[4];
			  		f[i][j]=coeffs[5];
				}					// end if statement for no data value
				ndv_present = 0;
			}
		}
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// This function calculates the elevation based on a polynomial fit
// the window is determined by the calculate_polyfit_coefficient_matrices
// this function also calculates the a,b,c,d,e and f coefficient matrices
//
// added by FC 24/03/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::calculate_polyfit_elevation(Array2D<double>& f)
{
	// create the new elevation raster
	Array2D<double> elevation_data(NRows,NCols,NoDataValue);

	for (int row = 0; row<NRows; row++)
	{
		for(int col = 0; col<NCols; col++)
		{
			if (f[row][col] != NoDataValue)
			{
				elevation_data[row][col] = f[row][col];
			}
		}
	}


	LSDRaster elevation_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,NoDataValue,elevation_data);
	return elevation_raster;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// This function calculates the slope based on a polynomial fit
// the window is determined by the calculate_polyfit_coefficient_matrices
// this function also calculates the a,b,c,d,e and f coefficient matrices
//
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::calculate_polyfit_slope(Array2D<double>& d, Array2D<double>& e)
{
	// create the new slope raster
	Array2D<double> slope_data(NRows,NCols,NoDataValue);

	for (int row = 0; row<NRows; row++)
	{
		for(int col = 0; col<NCols; col++)
		{
			if (d[row][col] != NoDataValue)
			{
				slope_data[row][col] = sqrt(d[row][col]*d[row][col]+e[row][col]*e[row][col]);
			}
		}
	}


	LSDRaster slope_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,NoDataValue,slope_data);
	return slope_raster;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// This function calculates the aspect based on a polynomial fit
// the window is determined by the calculate_polyfit_coefficient_matrices
// this function also calculates the a,b,c,d,e and f coefficient matrices
// SMM modified from DTM standalone code 2012
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::calculate_polyfit_aspect(Array2D<double>& d, Array2D<double>& e)
{
	// create the new slope raster
	Array2D<double> aspect_data(NRows,NCols,NoDataValue);

	for (int row = 0; row<NRows; row++)
	{
		for(int col = 0; col<NCols; col++)
		{
			if (d[row][col] != NoDataValue)
			{
				if(d[row][col]==0 || e[row][col]==0)
				{
					aspect_data[row][col] = NoDataValue;
				}
				else
				{
					aspect_data[row][col] = 180 - 57.29578*atan(e[row][col]/d[row][col])
					                            + 90*(d[row][col]/abs(d[row][col]));
					if(aspect_data[row][col] < 180.0)
					{
						aspect_data[row][col] = 180.0 - aspect_data[row][col];
					}
					else
					{
						aspect_data[row][col] = 360.0 + (180 - aspect_data[row][col]);
					}
				}
			}
		}
	}

	LSDRaster aspect_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,
	                           NoDataValue,aspect_data);
	return aspect_raster;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// This function calculates the curvature based on a polynomial fit
// the window is determined by the calculate_polyfit_coefficient_matrices
// this function also calculates the a,b,c,d,e and f coefficient matrices
//
// SMM modified from DTM standalone code 2012
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::calculate_polyfit_curvature(Array2D<double>& a, Array2D<double>& b)
{
	// create the new slope raster
	Array2D<double> curvature_data(NRows,NCols,NoDataValue);

	for (int row = 0; row<NRows; row++)
	{
		for(int col = 0; col<NCols; col++)
		{
			if (a[row][col] != NoDataValue)
			{
				curvature_data[row][col] = 2*a[row][col]+2*b[row][col];
			}
		}
	}


	LSDRaster curvature_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,
	                           NoDataValue,curvature_data);
	return curvature_raster;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// This function calculates the planform curvature based on a polynomial fit
// the window is determined by the calculate_polyfit_coefficient_matrices
// this function also calculates the a,b,c,d,e and f coefficient matrices
// Code written by DM and FC 09/10/12
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::calculate_polyfit_planform_curvature(Array2D<double>& a, Array2D<double>& b,
                                                          Array2D<double>& c, Array2D<double>& d,
                                                          Array2D<double>& e)
{
	// create the new planform curvature raster
	Array2D<double> pl_curvature_data(NRows,NCols,NoDataValue);
  	double fx, fy, fxx, fyy, fxy, p, q;

	for (int row = 0; row<NRows; row++)
	{

		for(int col = 0; col<NCols; col++)
		{

			if (a[row][col] != NoDataValue)
			{
				fx = d[row][col];
			  	fy = e[row][col];
			  	fxx = 2*a[row][col];
			  	fyy = 2*b[row][col];
			  	fxy = c[row][col];
			  	p = fx*fx + fy*fy;
			  	q = p + 1;

			  	if (q > 0)
			  	{
					pl_curvature_data[row][col] = (fxx*fy*fy - 2*fxy*fx*fy + fyy*fx*fx)/(sqrt(q*q*q));
				}
				else
				{
					pl_curvature_data[row][col] = NoDataValue;
				}
			}
		}
	}


	LSDRaster planform_curvature_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,
	                           NoDataValue,pl_curvature_data);
	return planform_curvature_raster;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// This function calculates the profile curvature based on a polynomial fit
// the window is determined by the calculate_polyfit_coefficient_matrices
// this function also calculates the a,b,c,d,e and f coefficient matrices
// Code written by FC 09/10/12
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::calculate_polyfit_profile_curvature(Array2D<double>& a, Array2D<double>& b,
                                                          Array2D<double>& c, Array2D<double>& d,
                                                          Array2D<double>& e)
{
	// create the new profile curvature raster
	Array2D<double> profile_curvature_data(NRows,NCols,NoDataValue);
  	double fx, fy, fxx, fyy, fxy, p, q, qqq, denom;

	for (int row = 0; row<NRows; row++)
	{
		for(int col = 0; col<NCols; col++)
		{
			if (a[row][col] != NoDataValue)
			{
			  	fx = d[row][col];
			  	fy = e[row][col];
			  	fxx = 2*a[row][col];
			  	fyy = 2*b[row][col];
			  	fxy = c[row][col];
			  	p = fx*fx + fy*fy;
			  	q = p + 1;

			  	qqq = q*q*q;
			  	if( qqq>0)
			  	{
					denom = (p*sqrt(qqq));
					if( denom != 0)
					{
						profile_curvature_data[row][col] = (fxx*fx*fx + 2*fxy*fx*fy + fyy*fy*fy)/denom;
					}
					else
					{
						profile_curvature_data[row][col] = NoDataValue;
					}
				}
				else
				{
					profile_curvature_data[row][col] = NoDataValue;
				}



			}
		}
	}


	LSDRaster profile_curvature_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,
	                           NoDataValue,profile_curvature_data);
	return profile_curvature_raster;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// This function calculates the tangential curvature based on a polynomial fit
// the window is determined by the calculate_polyfit_coefficient_matrices
// this function also calculates the a,b,c,d,e and f coefficient matrices
// Code written by DM and FC 09/10/12
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::calculate_polyfit_tangential_curvature(Array2D<double>& a, Array2D<double>& b,
                                                          Array2D<double>& c, Array2D<double>& d,
                                                          Array2D<double>& e)
{
	// create the new planform curvature raster
	Array2D<double> ta_curvature_data(NRows,NCols,NoDataValue);
  	double fx, fy, fxx, fyy, fxy, p, q, denom;

	for (int row = 0; row<NRows; row++)
	{

		for(int col = 0; col<NCols; col++)
		{

			if (a[row][col] != NoDataValue)
			{
				fx = d[row][col];
			  	fy = e[row][col];
			  	fxx = 2*a[row][col];
			  	fyy = 2*b[row][col];
			  	fxy = c[row][col];
			  	p = fx*fx + fy*fy;
			  	q = p + 1;


			  	if( q>0)
			  	{
					denom = (p*sqrt(q));
					if( denom != 0)
					{
						ta_curvature_data[row][col] = (fxx*fy*fy - 2*fxy*fx*fy + fyy*fx*fx)/denom;
					}
					else
					{
						ta_curvature_data[row][col] = NoDataValue;
					}
				}
				else
				{
					ta_curvature_data[row][col] = NoDataValue;
				}

			}
		}
	}


	LSDRaster tangential_curvature_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,
	                           NoDataValue,ta_curvature_data);
	return tangential_curvature_raster;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// This function identifies approximate position of stationary points within
// discrete surface using a threshold slope. The nature of the stationary point
// is then determined to discriminate peaks, depressions and saddles.
// 0 = Non-stationary
// 1 = Peak
// 2 = Depression
// 3 = Saddle
//
// Added by DTM 17/09/2012
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDIndexRaster LSDRaster::calculate_polyfit_classification(Array2D<double>& a, Array2D<double>& b, Array2D<double>& c,
                                                           Array2D<double>& d, Array2D<double>& e)
{
	// create the new classification raster
	int intNoDataValue = int(NoDataValue);
	Array2D<int> classification(NRows,NCols,intNoDataValue);
	double d2z_dx2,d2z_dy2,d2z_dxdy,slope;
	for (int row = 0; row<NRows; row++)
	{
		for(int col = 0; col<NCols; col++)
		{
			if (d[row][col] != NoDataValue)
			{
				slope = sqrt(pow(d[row][col],2) + pow(e[row][col],2));
        		if (slope < 0.1) // Threshold for assessing whether point is close to a stationary point
        		{
					d2z_dx2 = 2*a[row][col];
          			d2z_dy2 = 2*b[row][col];
          			d2z_dxdy = c[row][col];
          			if (d2z_dx2 < 0 && d2z_dy2 < 0 && d2z_dxdy*d2z_dxdy < d2z_dx2*d2z_dy2)  // Conditions for peak
          			{
            			classification[row][col] = 1;
          			}
          			else if (d2z_dx2 > 0 && d2z_dy2 > 0 && d2z_dxdy*d2z_dxdy < d2z_dx2*d2z_dy2) // Conditions for a depression
          			{
            			classification[row][col] = 2;
          			}
          			else if (d2z_dx2*d2z_dy2 < 0 || d2z_dxdy*d2z_dxdy > d2z_dx2*d2z_dy2)  // Conditions for a saddle
          			{
           				classification[row][col] = 3;
          			}
          			else
          			{
            			classification = 0;
          			}
				}
			}
		}
	}

	LSDIndexRaster sp_class_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,intNoDataValue,classification);
	return sp_class_raster;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// this function takes the polyfit functions and requires a window radius and a vector telling the
// function which rasters to print to file. The function is data efficient since one does not
// need to recalucalte the polyfit coefficeint matrices
// it also takes a string which is the prename of the data files
// the file codes in the vector are:
// 0 slope
// 1 aspect
// 2 curvature
// 3 planform curvature
// 4 profile curvature
// 5 tangential curvature
// 6 classification
// SMM 18-Dec-2012
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::calculate_and_print_polyfit_rasters(double window_radius, string file_prefix, vector<int> file_code)
{
	// set up polyfit arrays
	Array2D<double> a;
	Array2D<double> b;
	Array2D<double> c;
	Array2D<double> d;
	Array2D<double> e;
	Array2D<double> f;

	int n_vec_entries = file_code.size();
	if ( n_vec_entries !=7)
	{
		cout << endl << "LSDRaster.calcualte_and_print_polyfit_rasters error" << endl;
		cout << "You have the wrong number of entries in the file code vector; taking no action!!!" << endl << endl;
	}
	else
	{
		int window_int = int(window_radius);
		double decimal = window_radius-double(window_int);
		double decimal_ten = decimal*10;
		int decimal_ten_str = int(decimal_ten);
		string window_number_str = itoa(window_int);
		string remainder_str = itoa(decimal_ten_str);
		string p_str = "p";
		string window_size_str = window_number_str+p_str+remainder_str;
		string DEM_flt_extension = "flt";
		string underscore = "_";


		// calcualte polyfit arrays
		calculate_polyfit_coefficient_matrices(window_radius,a, b,c, d, e, f);

		// now go through vector to see which files you want
		if (file_code[0] == 1)
		{
			LSDRaster PolySlope = calculate_polyfit_slope(d, e);
			string S_name = "_pslope_";
			S_name = file_prefix+S_name+window_size_str;
			PolySlope.write_raster(S_name,DEM_flt_extension);
		}
		if (file_code[1] == 1)
		{
			LSDRaster PolyAspect = calculate_polyfit_aspect(d,e);
			string A_name = "_paspect_";
			A_name = file_prefix+A_name+window_size_str;
			PolyAspect.write_raster(A_name,DEM_flt_extension);
		}
		if (file_code[2] == 1)
		{
			LSDRaster PolyCurv = calculate_polyfit_curvature(a,b);
			string C_name = "_pcurv_";
			C_name = file_prefix+C_name+window_size_str;
			PolyCurv.write_raster(C_name,DEM_flt_extension);
		}
		if (file_code[3] == 1)
		{
			LSDRaster PolyPlCurv = calculate_polyfit_planform_curvature(a,b,c,d,e);
			string CP_name = "_pplcurv_";
			CP_name = file_prefix+CP_name+window_size_str;
			PolyPlCurv.write_raster(CP_name,DEM_flt_extension);
		}
		if (file_code[4] == 1)
		{
			LSDRaster PolyPrCurv = calculate_polyfit_profile_curvature(a,b,c,d,e);
			string CPr_name = "_pprcurv_";
			CPr_name = file_prefix+CPr_name+window_size_str;
			PolyPrCurv.write_raster(CPr_name,DEM_flt_extension);
		}
		if (file_code[5] == 1)
		{
			LSDRaster PolyTaCurv = calculate_polyfit_tangential_curvature(a,b,c,d,e);
			string CTa_name = "_ptacurv_";
			CTa_name = file_prefix+CTa_name+window_size_str;
			PolyTaCurv.write_raster(CTa_name,DEM_flt_extension);
		}
		if (file_code[6] == 1)
		{
			LSDIndexRaster PolyClass = calculate_polyfit_classification(a,b,c,d,e);
			string CCl_name = "_pclass_";
			CCl_name = file_prefix+CCl_name+window_size_str;
			PolyClass.write_raster(CCl_name,DEM_flt_extension);
		}
	}

}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// this function combines the polyfit functions and the roughness function in one package that
// is data efficient becasue it only requires one calcualtion of the polyfit matrices.
// it takes the window radius of the polyfit and the window of the roughness calcualtion
// the file codes in the vector are:
// 0 slope
// 1 aspect
// 2 curvature
// 3 planform curvature
// 4 profile curvature
// 5 tangential curvature
// 6 classification
// 7 roughness s1
// 8 roughness s2
// 9 roughness s3
// SMM 19-12-2012 -> adapted by DTM 15/07/2013 since it really makes sense just to kick out
// s1,s2,s3 rather than derivatives for options 7-9
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::calculate_and_print_polyfit_and_roughness_rasters(double window_radius, double roughness_radius,
										string file_prefix, vector<int> file_code)
{
	// set up polyfit arrays
	Array2D<double> a;
	Array2D<double> b;
	Array2D<double> c;
	Array2D<double> d;
	Array2D<double> e;
	Array2D<double> f;

	int n_vec_entries = file_code.size();
	if ( n_vec_entries !=10)
	{
		cout << endl << "LSDRaster.calcualte_and_print_polyfit_rasters error" << endl;
		cout << "You have the wrong number of entries in the file code vector; taking no action!!!" << endl << endl;
	}
	else
	{
		int window_int = int(window_radius);
		double decimal = window_radius-double(window_int);
		double decimal_ten = decimal*10;
		int decimal_ten_str = int(decimal_ten);
		string window_number_str = itoa(window_int);
		string remainder_str = itoa(decimal_ten_str);
		string p_str = "p";
		string window_size_str = window_number_str+p_str+remainder_str;
		string DEM_flt_extension = "flt";
		string underscore = "_";

		int roughness_int = int(roughness_radius);
		double decimalroughness = roughness_radius-double(roughness_int);
		double decimalroughness_ten = decimalroughness*10;
		int decimalroughness_ten_str = int(decimalroughness_ten);
		string roughness_number_str = itoa(roughness_int);
		string remainderroughness_str = itoa(decimalroughness_ten_str);
		string polystring = "ply";
		string roughstring = "rgh";
		string roughness_size_str = roughness_number_str+p_str+remainderroughness_str;
		roughness_size_str = polystring+window_size_str+underscore+roughstring+roughness_size_str;

		// calcualte polyfit arrays
		calculate_polyfit_coefficient_matrices(window_radius,a, b,c, d, e, f);

		// now go through vector to see which files you want
		if (file_code[0] == 1)
		{
			LSDRaster PolySlope = calculate_polyfit_slope(d, e);
			string S_name = "_pslope_";
			S_name = file_prefix+S_name+window_size_str;
			PolySlope.write_raster(S_name,DEM_flt_extension);
		}
		if (file_code[1] == 1)
		{
			LSDRaster PolyAspect = calculate_polyfit_aspect(d,e);
			string A_name = "_paspect_";
			A_name = file_prefix+A_name+window_size_str;
			PolyAspect.write_raster(A_name,DEM_flt_extension);
		}
		if (file_code[2] == 1)
		{
			LSDRaster PolyCurv = calculate_polyfit_curvature(a,b);
			string C_name = "_pcurv_";
			C_name = file_prefix+C_name+window_size_str;
			PolyCurv.write_raster(C_name,DEM_flt_extension);
		}
		if (file_code[3] == 1)
		{
			LSDRaster PolyPlCurv = calculate_polyfit_planform_curvature(a,b,c,d,e);
			string CP_name = "_pplcurv_";
			CP_name = file_prefix+CP_name+window_size_str;
			PolyPlCurv.write_raster(CP_name,DEM_flt_extension);
		}
		if (file_code[4] == 1)
		{
			LSDRaster PolyPrCurv = calculate_polyfit_profile_curvature(a,b,c,d,e);
			string CPr_name = "_pprcurv_";
			CPr_name = file_prefix+CPr_name+window_size_str;
			PolyPrCurv.write_raster(CPr_name,DEM_flt_extension);
		}
		if (file_code[5] == 1)
		{
			LSDRaster PolyTaCurv = calculate_polyfit_tangential_curvature(a,b,c,d,e);
			string CTa_name = "_ptacurv_";
			CTa_name = file_prefix+CTa_name+window_size_str;
			PolyTaCurv.write_raster(CTa_name,DEM_flt_extension);
		}
		if (file_code[6] == 1)
		{
			LSDIndexRaster PolyClass = calculate_polyfit_classification(a,b,c,d,e);
			string CCl_name = "_pclass_";
			CCl_name = file_prefix+CCl_name+window_size_str;
			PolyClass.write_raster(CCl_name,DEM_flt_extension);
		}
		if (file_code[7] == 1|| file_code[8] == 1|| file_code[9] == 1)
		{
			Array2D<double> l;
			Array2D<double> m;
			Array2D<double> n;
			Array2D<double> s1;
			Array2D<double> s2;
			Array2D<double> s3;
			calculate_polyfit_directional_cosines(d, e, l, m, n);
			calculate_orientation_matrix_eigenvalues(roughness_radius,l,m,n,s1,s2,s3);

      		if (file_code[7] == 1)
		  	{
				LSDRaster s1_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,NoDataValue,s1);
        		string s1_name = "_s1_";
        		s1_name = file_prefix+s1_name+roughness_size_str;
        		s1_raster.write_raster(s1_name,DEM_flt_extension);
        	}
  			if (file_code[8] == 1)
  			{
				LSDRaster s2_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,NoDataValue,s2);
        		string s2_name = "_s2_";
  				s2_name = file_prefix+s2_name+roughness_size_str;
  				s2_raster.write_raster(s2_name,DEM_flt_extension);
  			}
  			if (file_code[9] == 1)
  			{
  				LSDRaster s3_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,NoDataValue,s3);
        		string s3_name = "_s3_";
  				s3_name = file_prefix+s3_name+roughness_size_str;
  				s3_raster.write_raster(s3_name,DEM_flt_extension);
  			}

		}		// end file codes 7,8,9

	}			// end test for file codes logic

}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// GET HILLTOP CURVATURE DTM 30/04/13
// Input rasters: curvature, hilltop network.
// Output raster: hilltop curvature
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::get_hilltop_curvature(LSDRaster& curvature, LSDRaster& Hilltops)
{
	// create the new planform curvature raster
	Array2D<double> hilltop_curvature(NRows,NCols,NoDataValue);

	for (int row = 0; row<NRows; row++)
	{
		for(int col = 0; col<NCols; col++)
		{
			if (Hilltops.get_data_element(row,col) != NoDataValue)
      		{
        		hilltop_curvature[row][col] = curvature.get_data_element(row,col);
        	}
    	}
  	}

  	LSDRaster hilltop_curvature_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,
	                           NoDataValue,hilltop_curvature);
	return hilltop_curvature_raster;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// RRRRR  EEEEEE IIIIII
// RR  RR EE       II
// RRRR   EEEE     II
// RR RR  EE       II
// RR  RR EEEEEE IIIIII
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=
// ROCK EXPOSURE INDEX
// DiBiase et al. (2012) developed the rock eposure index as a proxy for the
// degree of rock exposure within a basin as defined by the proportion of pixels
// with a local slope exceeding a critical value.  They calculate local slope by
// fitting a planar surface to a 9 cell moving window (window radius = 1).
// Algorithm written by DTM, 08/10/2012
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=
void LSDRaster::calculate_plane_coefficient_matrices(double window_radius,
										Array2D<double>& a_plane, Array2D<double>& b_plane,
										Array2D<double>& c_plane)
{
	// this fits a plane over a kernel window. First, perpare the kernel
	int kr = int(ceil(window_radius/DataResolution));           // Set radius of kernel
	int kw=2*kr+1;                    						// width of kernel
	Array2D<double> data_kernel(kw,kw,NoDataValue);
	Array2D<double> x_kernel(kw,kw,NoDataValue);
	Array2D<double> y_kernel(kw,kw,NoDataValue);
	// reset the a,b,c matrices (the coefficient matrices)
	Array2D<double> temp_coef(NRows,NCols,0.0);
	a_plane = temp_coef.copy();
	b_plane = temp_coef.copy();
	c_plane = temp_coef.copy();
	// scale kernel window to resolution of DEM, and translate coordinates to be
	// centred on cell of interest (the centre cell)
	double x,y,zeta;
	for(int i=0;i<kw;++i)
	{
	    for(int j=0;j<kw;++j)
	    {
	      	x_kernel[i][j]=(i-kr)*DataResolution;
	      	y_kernel[i][j]=(j-kr)*DataResolution;
	    }
	}
	// FIT PLANE BY LEAST SQUARES REGRESSION AND USE COEFFICIENTS TO DETERMINE
	// LOCAL SLOPE
	// Have N simultaneous linear equations, and N unknowns.
	// => b = Ax, where x is a 1xN array containing the coefficients we need for
	// surface fitting.
	// A is constructed using different combinations of x and y, thus we only need
	// to compute this once, since the window size does not change.
	// For 1st order surface fitting, there are 3 coefficients, therefore A is a
	// 3x3 matrix
	Array2D<double> A(3,3);
	for (int i=0; i<kw; ++i)
	{
		for (int j=0; j<kw; ++j)
		{
			x = x_kernel[i][j];
			y = y_kernel[i][j];
			// Generate matrix A
			A[0][0] += pow(x,2);
			A[0][1] += x*y;
			A[0][2] += x;
			A[1][0] += y*x;
			A[1][1] += pow(y,2);
			A[1][2] += y;
			A[2][0] += x;
			A[2][1] += y;
			A[2][2] += 1;
		}
	}
	// Move window over DEM, fitting planar surface to the elevations within the
  // window.
	cout << "\n\tRunning planar surface fitting" << endl;
	cout << "\t\tDEM size = " << NRows << " x " << NCols << endl;
	int ndv_present = 0;
	for(int i=0;i<NRows;++i)
	{
		cout << "\tRow = " << i+1 << " / " << NRows << "    \r";
		for(int j=0;j<NCols;++j)
		{
			// Avoid edges
			if(i-kr < 0 || i+kr+1 > NRows || j-kr < 0 || j+kr+1 > NCols)
			{
				a_plane[i][j] = NoDataValue;
				b_plane[i][j] = NoDataValue;
				c_plane[i][j] = NoDataValue;
			}
			// Avoid nodata values
			else if(RasterData[i][j]==NoDataValue)
			{
				a_plane[i][j] = NoDataValue;
				b_plane[i][j] = NoDataValue;
				c_plane[i][j] = NoDataValue;
			}
			else
			{
				// clip DEM
				//zeta_sampler=zeta.copy();
				for(int i_kernel=0;i_kernel<kw;++i_kernel)
				{
			  		for(int j_kernel=0;j_kernel<kw;++j_kernel)
			  		{
						data_kernel[i_kernel][j_kernel] =
						RasterData[i-kr+i_kernel][j-kr+j_kernel];
						// check for nodata values nearby
						if(data_kernel[i_kernel][j_kernel]==NoDataValue)
						{
							ndv_present=1;
						}
			  		}
				}
				// Fit best fitting plane, avoiding nodata values
				if(ndv_present == 0)  // test for nodata values within the selection
				{
					Array1D<double> bb(3,0.0);
					Array1D<double> coeffs(3);
					for (int krow=0; krow<kw; ++krow)
					{

					  	for (int kcol=0; kcol<kw; ++kcol)
					  	{
							x = x_kernel[krow][kcol];
					    	y = y_kernel[krow][kcol];
					    	zeta = data_kernel[krow][kcol];
					    	// Generate vector bb
					    	bb[0] += zeta*x;
					    	bb[1] += zeta*y;
					    	bb[2] += zeta;
					  	}
					}
					// Solve matrix equations using LU decomposition using the TNT JAMA package:
					// A.coefs = b, where coefs is the coefficients vector.
					LU<double> sol_A(A);  // Create LU object
					coeffs = sol_A.solve(bb);

			  		a_plane[i][j]=coeffs[0];
			  		b_plane[i][j]=coeffs[1];
			  		c_plane[i][j]=coeffs[2];
				}
				ndv_present = 0;
			}
		}
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDIndexRaster LSDRaster::calculate_REI(Array2D<double>& a_plane, Array2D<double>& b_plane, double CriticalSlope)
{
	// create the REI raster
	int intndv = int(NoDataValue);
	Array2D<int> REI_data(NRows,NCols,intndv);
  	double SlopeOfPlane;
	for (int row = 0; row<NRows; row++)
	{
		for(int col = 0; col<NCols; col++)
		{
			if (a_plane[row][col] != NoDataValue)
			{
				SlopeOfPlane = sqrt(a_plane[row][col]*a_plane[row][col]+b_plane[row][col]*b_plane[row][col]);
				// Create binary matrix 1 = rock, 0 = no rock
        		if (SlopeOfPlane > CriticalSlope)
				{
          			REI_data[row][col] = 1;
        		}
        		else
        		{
          			REI_data[row][col] = 0;
        		}
			}
		}
	}

	LSDIndexRaster REI_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,intndv,REI_data);
	return REI_raster;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// RRRRR    OOOO   UU   UU  GGGGG HH   HH NN    NN EEEEEE   SSSS    SSSS
// RR  RR  OO  OO  UU   UU GG     HH   HH NNN   NN EE     SS      SS
// RRRR   OO    OO UU   UU GG GGG HHHHHHH NN NN NN EEEE    SSSS    SSSS
// RR RR   OO  OO  UU   UU GG  GG HH   HH NN   NNN EE         SS      SS
// RR  RR   OOOO    UUUUU   GGGG  HH   HH NN    NN EEEEEE SSSSS   SSSSS
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Algorithm that assesses surface roughness based on a polynomial fit.  Runs a
// moving window across the DEM and assesses the variability of surface normals
// within that window.  Specifically the components of the normals are combined
// into an orientation matrix, which is then solved to find the eigenvalues s1,
// s2, s3 (Woodcock, 1977).
//
// Added by DTM 13/09/2012
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Get directional cosines
void LSDRaster::calculate_polyfit_directional_cosines(Array2D<double>& d, Array2D<double>& e,
                                    Array2D<double>& l, Array2D<double>& m, Array2D<double>& n)
{
	double pheta, phi;
	// reset the l, m and n matrices (the directional cosines matrices)
	Array2D<double> temp_coef(NRows,NCols,NoDataValue);

	l = temp_coef.copy();
	m = temp_coef.copy();
	n = temp_coef.copy();
	// get surface normals (in form of direction cosines) for each point in DEM
	for (int row = 0; row<NRows; row++)
	{
		for(int col = 0; col<NCols; col++)
		{
			if (d[row][col] != NoDataValue)
			{
				pheta = atan(sqrt(d[row][col]*d[row][col]+e[row][col]*e[row][col]));
				if (e[row][col] == 0 || d[row][col] == 0)
				{
					phi = NoDataValue;
				}
        		else
        		{
					phi = atan(e[row][col]/d[row][col]);
				}

        		// Directional cosines of normal vectors
        		if (pheta != 0)
        		{ // l and m are undefined if pheta = 0 because phi is undefined for a vertical surface normal
          			l[row][col]=sin(pheta)*cos(phi);
          			m[row][col]=sin(pheta)*sin(phi);
        		}

        		// Can define n for vertical surface normal, since pheta is always defined
        		n[row][col]=cos(pheta);
			}
		}
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Find eigenvalues for orientation matrix
//
// Added by DTM 13/09/2012
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::calculate_orientation_matrix_eigenvalues(double window_radius,
										Array2D<double>& l, Array2D<double>& m,
										Array2D<double>& n, Array2D<double>& s1,
                    					Array2D<double>& s2, Array2D<double>& s3)
{
	// Reset the eigenvalue matrices
	Array2D<double> temp_coef(NRows,NCols,0.0);

	//Array2D<double> temp_coef(NRows,NCols,NoDataValue);
	s1 = temp_coef.copy();
	s2 = temp_coef.copy();
	s3 = temp_coef.copy();

	// this fits a polynomial surface over a kernel window. First, perpare the kernel
	int kr = int(ceil(window_radius/DataResolution));       // Set radius of kernel => suggest = 1 cell
	int kw=2*kr+1;                    						// width of kernel
	double li,mi,ni;

	// Declare kernel arrays
	Array2D<double> l_kernel(kw,kw,NoDataValue);
	Array2D<double> m_kernel(kw,kw,NoDataValue);
	Array2D<double> n_kernel(kw,kw,NoDataValue);

	// Build circular mask
  	Array2D<int> mask(kw,kw,0);
  	double x_kernel,y_kernel, radial_dist;
  	for(int i=0;i<kw;++i)
	{
	    for(int j=0;j<kw;++j)
	    {
			x_kernel=(i-kr)*DataResolution;
	      	y_kernel=(j-kr)*DataResolution;
        	radial_dist = sqrt(y_kernel*y_kernel + x_kernel*x_kernel); // distance from centre to this point.
        	if (floor(radial_dist) <= window_radius)
        	{
				mask[i][j] = 1;
			}
      	}
	}

	// Loop over DEM, extacting the directional cosines for the data kernel
	for(int row=0;row<NRows;++row)
	{
		for(int col=0;col<NCols;++col)
	  	{
      		//Avoid edges
      		if(row-kr < 0 || row+kr+1 > NRows || col-kr < 0 || col+kr+1 > NCols)
      		{
				s1[row][col]=NoDataValue;
          		s2[row][col]=NoDataValue;
          		s3[row][col]=NoDataValue;
          	}
          	else
          	{
        		//Avoid NoDataValues
        		if(l[row][col] != NoDataValue)
        		{
          			// Construct orientation matrix and solve to retrieve eigenvalues for data window
          			// Build orientation matrix
          			Array2D<double> T(3,3,0.0);
          			Array2D<double> D(3,3);
          			int N=1;
          			for(int i=0;i<kw;++i)
          			{
            			for(int j=0;j<kw;++j)
            			{
							if (mask[i][j]==1)
            			  	{
								li=l[row-kr+i][col-kr+j];
              			  		mi=m[row-kr+i][col-kr+j];
              			  		ni=n[row-kr+i][col-kr+j];
						  	    T[0][0] += li*li;
						  	    T[0][1] += li*mi;
						  	    T[0][2] += li*ni;
						  	    T[1][0] += mi*li;
						  	    T[1][1] += mi*mi;
						  	    T[1][2] += mi*ni;
						  	    T[2][0] += ni*li;
						  	    T[2][1] += ni*mi;
						  	    T[2][2] += ni*ni;
						  	    ++N;
							}
            			}
          			}
          			// Find eigenvalues of the orientation matrix
          			Eigenvalue<double> eigenvalue_matrix(T);
          			eigenvalue_matrix.getD(D);
          			// Normalised eigenvalues (with respect to number of normals):
          			//cout << T << endl;
          			s1[row][col]=D[0][0]/N;
          			s2[row][col]=D[1][1]/N;
          			s3[row][col]=D[2][2]/N;
        		}
      		}
	  	}
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// This function is a wrapper to get the three roughness eigenvalues s1, s2 and
// s3.
//
//DTM 15/07/2013
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void LSDRaster::calculate_roughness_rasters(double window_radius, double roughness_radius,
										string file_prefix, vector<int> file_code)

{
	// set up polyfit arrays
	Array2D<double> a;
	Array2D<double> b;
	Array2D<double> c;
	Array2D<double> d;
	Array2D<double> e;
	Array2D<double> f;

	int n_vec_entries = file_code.size();
	if ( n_vec_entries !=3)
	{
		cout << endl << "LSDRaster.calcualte_and_print_polyfit_rasters error" << endl;
		cout << "You have the wrong number of entries in the file code vector; taking no action!!!" << endl << endl;
	}
	else
	{
		int window_int = int(window_radius);
		double decimal = window_radius-double(window_int);
		double decimal_ten = decimal*10;
		int decimal_ten_str = int(decimal_ten);
		string window_number_str = itoa(window_int);
		string remainder_str = itoa(decimal_ten_str);
		string p_str = "p";
		string window_size_str = window_number_str+p_str+remainder_str;
		string DEM_flt_extension = "flt";
		string underscore = "_";

		int roughness_int = int(roughness_radius);
		double decimalroughness = roughness_radius-double(roughness_int);
		double decimalroughness_ten = decimalroughness*10;
		int decimalroughness_ten_str = int(decimalroughness_ten);
		string roughness_number_str = itoa(roughness_int);
		string remainderroughness_str = itoa(decimalroughness_ten_str);
		string polystring = "ply";
		string roughstring = "rgh";
		string roughness_size_str = roughness_number_str+p_str+remainderroughness_str;
		roughness_size_str = polystring+window_size_str+underscore+roughstring+roughness_size_str;

		// calcualte polyfit arrays
		calculate_polyfit_coefficient_matrices(window_radius,a, b,c, d, e, f);
    	// analyse variability of normals
    	Array2D<double> l;
		Array2D<double> m;
		Array2D<double> n;
		Array2D<double> s1;
		Array2D<double> s2;
		Array2D<double> s3;
		calculate_polyfit_directional_cosines(d, e, l, m, n);
		calculate_orientation_matrix_eigenvalues(roughness_radius,l,m,n,s1,s2,s3);

		// now go through vector to see which files you want
		if (file_code[0] == 1)
		{
			LSDRaster s1_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,NoDataValue,s1);
      		string s1_name = "_s1_";
			s1_name = file_prefix+s1_name+roughness_size_str;
			s1_raster.write_raster(s1_name,DEM_flt_extension);
		}
		if (file_code[1] == 1)
		{
			LSDRaster s2_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,NoDataValue,s2);
      		string s2_name = "_s2_";
			s2_name = file_prefix+s2_name+roughness_size_str;
			s2_raster.write_raster(s2_name,DEM_flt_extension);
		}
		if (file_code[2] == 1)
		{
			LSDRaster s3_raster(NRows,NCols,XMinimum,YMinimum,DataResolution,NoDataValue,s3);
      		string s3_name = "_s3_";
			s3_name = file_prefix+s3_name+roughness_size_str;
			s3_raster.write_raster(s3_name,DEM_flt_extension);
		}
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// HH  HH YY   YY DDDD   RRRRR    OOOO   LL       OOOO    GGGGG  YY   YY
// HH  HH  YYYY   DD DD  RR  RR  OO  OO  LL      OO  OO  GG	      YYYY
// HHHHHH   YY    DD  DD RRRR   OO    OO LL     OO    OO GG GGG    YY
// HH  HH   YY    DD DD  RR RR   OO  OO  LL      OO  OO  GG  GG    YY
// HH  HH   YY    DDDD   RR  RR   OOOO   LLLLLL   OOOO    GGGGG    YY
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Fill
//---------------------------------------------------------------------------------
//
//This function fills pits/sinks in a DEM by incrementing elevations for cells with
//no downslope neighbour. The process is repeated adnausium until no cells require
//incrementing.
//
//Inputs required are a DEM file in ascii raster format as created by ARCMap
//and a file name to create a filled DEM grid.
//
//This code was built ontop of code made available by Jon D. Pelletier as part
//of his book:
//
//Pelletier,J.D.,'Quantitative Modelling of Landscapes' Cambridge University Press
//
//---------------------------------------------------------------------------------
//
// v1.3 reduced fill increment to 1mm  to avoid 'overfilling'
//
// Martin Hurst, October 2011
//
//---------------------------------------------------------------------------------
//
// v1.2 modified to read *.flt files
//
// Martin Hurst, November 2010
//
//---------------------------------------------------------------------------------
//
// v1.1 function incorporated to allow the tool to fill adjacent pixels immediately
// after filling a given pixel, should speed things up.
//
// Martin Hurst, October 2010
//
//---------------------------------------------------------------------------------
//
// v1.0 is slow as it requires many iterations through the dem
//
// Martin Hurst, June 2010
//
//---------------------------------------------------------------------------------
LSDRaster LSDRaster::fill()
{

	Array2D<double> FilledRasterData;
	FilledRasterData = RasterData.copy();
	cout << "N_rows is: " << NRows << " " << NCols << endl;
	cout << "Data rows: " << RasterData.dim1() << " cols: " << RasterData.dim2() << endl;
	for (int i=1; i<NRows-1; i++)
	{
		cout << "\rRow = " << i+1 << " / " << NRows << "    ";
		for (int j=1; j<NCols-1; j++)
		{
			//cout << "R: " << i << " C: " << j;
			//cout << " FDR: " << FilledRasterData[i][j];
			if (FilledRasterData[i][j] == NoDataValue || FilledRasterData[i-1][j-1] == NoDataValue
			         || FilledRasterData[i-1][j] == NoDataValue || FilledRasterData[i-1][j+1] == NoDataValue
			         || FilledRasterData[i][j+1] == NoDataValue || FilledRasterData[i+1][j+1] == NoDataValue
			         || FilledRasterData[i+1][j] == NoDataValue || FilledRasterData[i+1][j-1] == NoDataValue
			         || FilledRasterData[i][j-1] == NoDataValue)
			{ }
			else fill_iterator(FilledRasterData,i,j);

			//if (RasterData[i][j] == NoDataValue || RasterData[i-1][j-1] == NoDataValue
			//         || RasterData[i-1][j] == NoDataValue || RasterData[i-1][j+1] == NoDataValue
			//         || RasterData[i][j+1] == NoDataValue || RasterData[i+1][j+1] == NoDataValue
			//         || RasterData[i+1][j] == NoDataValue || RasterData[i+1][j-1] == NoDataValue
			//         || RasterData[i][j-1] == NoDataValue)
			//{ }
			//else fill_iterator(RasterData,i,j);
			//cout << " itercomplete" << endl;
		}
	}
	cout << endl;

	LSDRaster FilledDEM(NRows,NCols,XMinimum,YMinimum,DataResolution,NoDataValue,FilledRasterData);
	return FilledDEM;

}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// this is a recursive algorithm that is called by the fill function
//
// MDH, 2012
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::fill_iterator(Array2D<double>& fill_data, int i, int j)
{
	int a=i;
	int b=j;
	double fill_increment = 0.001;
	double min_zeta;
	double centre_zeta = fill_data[a][b];

	if (a==0 || b==0 || a == NRows-1 || b==NCols-1)
	{ }
	else if (fill_data[a][b] == NoDataValue || fill_data[a-1][b-1] == NoDataValue
			 || fill_data[a-1][b] == NoDataValue
	         || fill_data[a-1][b+1] == NoDataValue || fill_data[a][b+1] == NoDataValue
			 || fill_data[a+1][b+1] == NoDataValue || fill_data[a+1][b] == NoDataValue
			 || fill_data[a+1][b-1] == NoDataValue || fill_data[a][b-1] == NoDataValue)
	{}
	else
	{
		min_zeta = centre_zeta + 10;
		if (fill_data[a-1][b-1] < min_zeta) min_zeta = fill_data[a-1][b-1];
		if (fill_data[a-1][b] < min_zeta) min_zeta = fill_data[a-1][b];
		if (fill_data[a-1][b+1] < min_zeta) min_zeta = fill_data[a-1][b+1];
		if (fill_data[a][b+1] < min_zeta) min_zeta = fill_data[a][b+1];
		if (fill_data[a+1][b+1] < min_zeta) min_zeta = fill_data[a+1][b+1];
		if (fill_data[a+1][b] < min_zeta) min_zeta = fill_data[a+1][b];
		if (fill_data[a+1][b-1] < min_zeta) min_zeta = fill_data[a+1][b-1];
		if (fill_data[a][b-1] < min_zeta) min_zeta = fill_data[a][b-1];

		//increase elevation of centre cell if it is lower than or
		//equal in elevation compared to all adjacent cells
		if (centre_zeta <= min_zeta)
		{

      		// efficiency improvement by Dave Milodowski
      		double zeta_diff = min_zeta - centre_zeta;
      		fill_data[a][b] = fill_data[a][b] + zeta_diff + fill_increment;
			//fill adjacent pixels too
			fill_iterator(fill_data,a-1,b-1);
			fill_iterator(fill_data,a-1,b);
			fill_iterator(fill_data,a-1,b+1);
			fill_iterator(fill_data,a,b+1);
			fill_iterator(fill_data,a+1,b+1);
			fill_iterator(fill_data,a+1,b);
			fill_iterator(fill_data,a+1,b-1);
			fill_iterator(fill_data,a,b-1);
		}
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


//---------------------------------------------------------------------------------------
//
//	New fill function
//
//	This function fills pits/sinks in a DEM by checking for pits from lowest to highest
//	elevation, starting at the DEM boundary (raster edge or adjacent to NDVs). Utilises
//	a priority queue to progressively populate the stack and pop out the the lowest value
//	before checking that the neighbouring cells that are yet to be visited must be higher
//	in a hydrologically correct DEM. This method is substantially faster on datasets with
//	pits consisting of multiple cells since each cell only needs to be visited once.
//
//	Input argument required -> MinSlope the minimum slope between two Nodes once filled
//	If set to zero will create flats.
//
//	Method taken from Wang and Liu (2006), Int. J. of GIS. 20(2), 193-213
//
//	Martin Hurst, 12/3/13 */
//
//	Declare the node structure
///@brief Used in pit filling to store elevation data and row and colum indexes.
//	Method taken from Wang and Liu (2006), Int. J. of GIS. 20(2), 193-213
//	Method taken from Wang and Liu (2006), Int. J. of GIS. 20(2), 193-213
struct FillNode
{
  /// @brief Elevation data.
	double Zeta;
	/// @brief Row index value.
  int RowIndex;
	/// @brief Column index value.
  int ColIndex;
};

//Overload the less than and greater than operators to consider Zeta data only
//N.B. Fill only needs greater than but less than useful for mdflow routing
//(I've coded this but not yet added to LSDRaster, it's only faster than presorting
//when applied to pretty large datasets).
bool operator>( const FillNode& lhs, const FillNode& rhs )
{
	return lhs.Zeta > rhs.Zeta;
}
bool operator<( const FillNode& lhs, const FillNode& rhs )
{
	return lhs.Zeta < rhs.Zeta;
}

LSDRaster LSDRaster::fill(double& MinSlope)
{
	//cout << "Inside NewFill" << endl;

	//declare 1/root(2)
	double one_over_root2 = 0.707106781;

	//Declare the priority Queue with greater than comparison
	priority_queue< FillNode, vector<FillNode>, greater<FillNode> > PriorityQueue;
	//Declare a temporary FillNode structure which we populate before adding to the PQ
	//Declare a central node or node of interest
	FillNode TempFillNode, CentreFillNode;

	//declare vectors for slopes and row and col indices
	vector<double> slopes(8,NoDataValue);
	vector<int> row_kernal(8);
	vector<int> col_kernal(8);

	//Get Dimensions
	//int NRows = Zeta.dim1();
	//int NCols = Zeta.dim2();

	//Index array to track whether nodes are in queue or have been processed
	//-9999 = no_data, 0 = data but not processed or in queue,
	//1 = in queue but not processed, 2 = fully processed and removed from queue
	Array2D<int> FillIndex(NRows,NCols,NoDataValue);
	Array2D<double> FilledZeta;
	FilledZeta = RasterData.copy();

	//Collect boundary cells
	for (int i=0; i<NRows; ++i)
	{
		for (int j=0; j<NCols; ++j)
		{
			if (FilledZeta[i][j] != NoDataValue)
			{
				//If there is data the cell needs to be filled so
				//set fill index to zero (i.e. yet to be filled)
				FillIndex[i][j] = 0;

				//If we're at the edge or next to an NoDataValue then
				//put the cell into the priority queue
				if (i==0 || j==0 || i==NRows-1 || j==NCols-1 ||
					FilledZeta[i-1][j-1]==NoDataValue || FilledZeta[i-1][j]==NoDataValue ||
					FilledZeta[i-1][j+1]==NoDataValue || FilledZeta[i][j-1]==NoDataValue ||
					FilledZeta[i][j+1]==NoDataValue || FilledZeta[i+1][j-1]==NoDataValue ||
					FilledZeta[i+1][j]==NoDataValue || FilledZeta[i+1][j+1]==NoDataValue)
				{
					TempFillNode.Zeta = FilledZeta[i][j];
					TempFillNode.RowIndex = i;
					TempFillNode.ColIndex = j;
					PriorityQueue.push(TempFillNode);
					FillIndex[i][j] = 1;
				}
			}
		}
	}

	//Loop through the priority queue from lowest to highest elevations
	//filling as we go and adding unassessed neighbours to the priority queue
	while (!PriorityQueue.empty())
	{
		//first get the highest priority node and assign it before
		//removing it from the queue and declaring it processed
		CentreFillNode = PriorityQueue.top();
		int row=CentreFillNode.RowIndex, col=CentreFillNode.ColIndex;
		//cout << "Pop from Queue: Zeta = " << CentreFillNode.Zeta << endl;

		PriorityQueue.pop();
		FillIndex[row][col] = 2;

		//get neighbour indices
		//rows
		row_kernal[0] = row-1;
		row_kernal[1] = row-1;
		row_kernal[2] = row;
		row_kernal[3] = row+1;
		row_kernal[4] = row+1;
		row_kernal[5] = row+1;
		row_kernal[6] = row;
		row_kernal[7] = row-1;
		//cols
		col_kernal[0] = col;
		col_kernal[1] = col+1;
		col_kernal[2] = col+1;
		col_kernal[3] = col+1;
		col_kernal[4] = col;
		col_kernal[5] = col-1;
		col_kernal[6] = col-1;
		col_kernal[7] = col-1;

		//check if on array boundary and set kernal to NoDataValues to avoid
		//segmentation fault
		if (row == 0)
		{
			row_kernal[0] = NoDataValue;
			row_kernal[1] = NoDataValue;
			row_kernal[7] = NoDataValue;
		}
		else if (row==NRows-1)
		{
			row_kernal[3] = NoDataValue;
			row_kernal[4] = NoDataValue;
			row_kernal[5] = NoDataValue;
		}
		if (col == 0)
		{
			col_kernal[5] = NoDataValue;
			col_kernal[6] = NoDataValue;
			col_kernal[7] = NoDataValue;
		}
		else if (col == NCols-1)
		{
			col_kernal[1] = NoDataValue;
			col_kernal[2] = NoDataValue;
			col_kernal[3] = NoDataValue;
		}

		//loop through neighbours
		for (int Neighbour = 0; Neighbour<8; ++Neighbour)
		{
			//If the neighbour has data and is not already in the priority queue and has not been processed
			if (	row_kernal[Neighbour] == NoDataValue || col_kernal[Neighbour] == NoDataValue ||
					FillIndex[row_kernal[Neighbour]][col_kernal[Neighbour]] == 1 ||
					FillIndex[row_kernal[Neighbour]][col_kernal[Neighbour]] == 2 ||
					FillIndex[row_kernal[Neighbour]][col_kernal[Neighbour]] == NoDataValue ) {}
			else
			{
				//check if neighbour is equal/lower and therefore needs filling
				if (FilledZeta[row_kernal[Neighbour]][col_kernal[Neighbour]] <= CentreFillNode.Zeta)
				{
					//Modify neighbour's elevation
					if(Neighbour%2 == 0)
					{
						if (MinSlope > 0)
						{
							FilledZeta[row_kernal[Neighbour]][col_kernal[Neighbour]] = CentreFillNode.Zeta + MinSlope*DataResolution;
						}
						else
						{
							FilledZeta[row_kernal[Neighbour]][col_kernal[Neighbour]] = CentreFillNode.Zeta;
						}
					}
					else
					{
						if (MinSlope > 0)
						{
							FilledZeta[row_kernal[Neighbour]][col_kernal[Neighbour]] = CentreFillNode.Zeta
							                                     + MinSlope*DataResolution*one_over_root2;
						}
						else
						{
							FilledZeta[row_kernal[Neighbour]][col_kernal[Neighbour]] = CentreFillNode.Zeta;
						}
					}
				}
				//New neighbour needs to be added to the priority queue
				TempFillNode.Zeta = FilledZeta[row_kernal[Neighbour]][col_kernal[Neighbour]];
				TempFillNode.RowIndex = row_kernal[Neighbour];
				TempFillNode.ColIndex = col_kernal[Neighbour];
				PriorityQueue.push(TempFillNode);
				FillIndex[row_kernal[Neighbour]][col_kernal[Neighbour]] = 1;
				FillIndex[row][col] = 2;
			}
		}
	}
	LSDRaster FilledDEM(NRows,NCols,XMinimum,YMinimum,DataResolution,NoDataValue,FilledZeta);
	return FilledDEM;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=







//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//D-inf modules
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Main function for generating a D-infinity flow area raster after Tarboton (1997).
// Calls the recurisve D_infAccum function to get flow area for each pixel.
// Returns flow area in pixels.
//
// Code is ported and optimised from a Java implementation of the algorithm
// supplied under the GNU GPL licence through WhiteBox GAT:
// http://www.uoguelph.ca/~hydrogeo/Whitebox/ and provides identical results
// to the whitebox tool.
//
// SWDG - 26/07/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::D_inf_FlowArea(Array2D<double> FlowDir_array){

  // Arrays of indexes of neighbour cells wrt target cell and their
  //corresponding ranges of angles
  int dX[] = {1, 1, 1, 0, -1, -1, -1, 0};
  int dY[] = {-1, 0, 1, 1, 1, 0, -1, -1};
  double startFD[] = {180, 225, 270, 315, 0, 45, 90, 135};
  double endFD[] = {270, 315, 360, 45, 90, 135, 180, 225};

  Array2D<double> Flowarea_Raster(NRows,NCols,1);
  Array2D<double> CountGrid(NRows,NCols,NoDataValue); //array to hold no of inflowing neighbours

  int inflow_neighbours; //counter for number of inflowing neighbours
  double flowDir; //temp variable to store the flowdir of a neighbour

  // Calculate the number of inflowing neighbours to each cell.
  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      flowDir = FlowDir_array[i][j];
      if (flowDir != NoDataValue){
        inflow_neighbours = 0;

        for (int c = 0; c < 8; ++c){ //loop through the 8 neighbours of the target cell
          flowDir = FlowDir_array[i + dY[c]][j + dX[c]];
          if (flowDir >= 0 && flowDir <= 360){
            if (c != 3){  //handles the issue of 0,360 both pointing to North
              if (flowDir > startFD[c] && flowDir < endFD[c]){
                ++inflow_neighbours;
              }
            }
            else{
              if (flowDir > startFD[c] || flowDir < endFD[c]){
                ++inflow_neighbours;
              }
            }
          }
        }
        CountGrid[i][j] = inflow_neighbours;
      }
      else{
        Flowarea_Raster[i][j] = NoDataValue;
      }
    }
  }

  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      if (CountGrid[i][j] == 0){ //there are no inflowing neighbours
        //call the flowarea function and travel downstream from it
        D_infAccum(i, j, CountGrid, Flowarea_Raster, FlowDir_array);
      }
    }
  }

  LSDRaster FlowArea(NRows, NCols, XMinimum, YMinimum, DataResolution,
                          NoDataValue, Flowarea_Raster);

  return FlowArea;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Recursive function to calculate accumulating area for a given pixel. Called
// by the driver for every cell which has no contributing cells - eg the highest
// points on the landscape. Avoids the need to flatten and sort the DEM as
// required in the original Tarboton (1997) implementation. For more detail on the
// recursive algorithm following channels see Mark (1998) "Network Models in
// Geomorphology".
//
// Code is ported and optimised from a Java implementation of the algorithm
// supplied under the GNU GPL licence through WhiteBox GAT:
// http://www.uoguelph.ca/~hydrogeo/Whitebox/ and provides identical results
// to the whitebox tool.
//
// SWDG - 26/07/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::D_infAccum(int i, int j, Array2D<double> CountGrid, Array2D<double> Flowarea_Raster, Array2D<double> FlowDir){

  double flowAccumVal = Flowarea_Raster[i][j];
  double flowDir = FlowDir[i][j];

  //tables of angles and indexes used to rotate around each neighbour
  double FD_Low[] = {0, 45, 90, 135, 180, 225, 270, 315};
  double FD_High_361[] = {45, 90, 135, 180, 225, 270, 315, 361};  //this array ends with 361 to catch angles up to 360
  double FD_High[] = {45, 90, 135, 180, 225, 270, 315, 360};
  int Di1[] = {-1, -1, 0, 1, 1, 1, 0, -1};
  int Dj1[] = {0, 1, 1, 1, 0, -1, -1, -1};
  int Di2[] = {-1, 0, 1, 1, 1, 0, -1, -1};
  int Dj2[] = {1, 1, 1, 0, -1, -1, -1, 0};

  double proportion1 = 0; //proportion of flow to the lowest neighbour
  double proportion2 = 0; //proportion of flow to the second lowest neighbour

  // indexes to store the coordinates of the neighbours where flow is to be routed
  int a1 = 0;
  int b1 = 0;
  int a2 = 0;
  int b2 = 0;

  CountGrid[i][j] = -1; // flags a visted cell

  if (flowDir >= 0){  //avoids flagged pits

    // find which two cells receive flow and the proportion to each
    for (int q = 0; q < 8; ++q){
      if (flowDir >= FD_Low[q] && flowDir < FD_High_361[q]){
        proportion1 = (FD_High[q] - flowDir) / 45;
        a1 = i + Di1[q];
        b1 = j + Dj1[q];
        proportion2 = (flowDir - FD_Low[q]) / 45;
        a2 = i + Di2[q];
        b2 = j + Dj2[q];
        }
    }

      if (proportion1 > 0 && Flowarea_Raster[a1][b1] != NoDataValue){
        Flowarea_Raster[a1][b1] = Flowarea_Raster[a1][b1] + flowAccumVal * proportion1;
        CountGrid[a1][b1] = CountGrid[a1][b1] - 1;
        if (CountGrid[a1][b1] == 0){
          D_infAccum(a1, b1, CountGrid, Flowarea_Raster, FlowDir); //recursive call
        }
      }
      if (proportion2 > 0 && Flowarea_Raster[a2][b2] != NoDataValue){
        Flowarea_Raster[a2][b2] = Flowarea_Raster[a2][b2] + flowAccumVal * proportion2;
        CountGrid[a2][b2] = CountGrid[a2][b2] - 1;
        if (CountGrid[a2][b2] == 0){
          D_infAccum(a2, b2, CountGrid, Flowarea_Raster, FlowDir); //recursive call
      }
    }
  }
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// D-infinity flow direction algorithm after Tarboton (1997).
//
// Algorithm takes a filled DEM and for each cell calculates the steepest descent
// based on 8 triangular facets. Flow direction is assigned as an angle from 0-360
// degrees with -1 used to flag unresolved areas such as pits.
//
// Code is ported and optimised from a Java implementation of the algorithm
// supplied under the GNU GPL licence through WhiteBox GAT:
// http://www.uoguelph.ca/~hydrogeo/Whitebox/ and provides identical results
// to the whitebox tool.
//
// SWDG - 26/07/13
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Array2D<double> LSDRaster::D_inf_FlowDir(){

  Array2D<double> FlowDir_Array(NRows,NCols,NoDataValue);

  double maxSlope; //maxiumum slope
  double flowDir = 0; //temp variable to hold flowdirections while looping thru the 8 facets

  double pi = 3.14159265;

  //components of the triangular facets as outlined in Tarboton (1997) fig 3
  //& equations 1-5
  double e0;
  double e1;
  double e2;
  double s1;
  double s2;
  double r;
  double s;

  //Facet elevation factors from Tarboton (1997) Table 1
  int acVals[] = {0, 1, 1, 2, 2, 3, 3, 4};
  int afVals[] = {1, -1, 1, -1, 1, -1, 1, -1};
  int e1Col[] = {1, 0, 0, -1, -1, 0, 0, 1};
  int e1Row[] = {0, -1, -1, 0, 0, 1, 1, 0};
  int e2Col[] = {1, 1, -1, -1, -1, -1, 1, 1};
  int e2Row[] = {-1, -1, -1, -1, 1, 1, 1, 1};

  for (int i = 1; i < NRows - 1; ++i){
    for (int j = 1; j < NCols - 1; ++j){
      e0 = RasterData[i][j];
      if (e0 == NoDataValue){
        FlowDir_Array[i][j] = NoDataValue; //if there is no elevation data we cant have a flowdir
      }
      else{
        maxSlope = -9999999;  //set to a low value that != NDV so any slope will be bigger than it

        for (int a = 0; a < 8; ++a){  //loop through the 8 facets
          e1 = RasterData[i + e1Row[a]][j + e1Col[a]];
          e2 = RasterData[i + e2Row[a]][j + e2Col[a]];
          if (e1 != NoDataValue && e2 != NoDataValue){ //avoid facets lyng in no data
            if (e0 > e1 && e0 > e2){
              //calculate slopes (s1,s2,s) and bearings (r) along edges
              //of the facet when e0 is higher than e1 and e2
              s1 = (e0 - e1) / DataResolution;
              if (s1 == 0){
                s1 = 0.00001;
              }
              s2 = (e1 - e2) / DataResolution;
              r = atan(s2 / s1);
              s = sqrt(s1 * s1 + s2 * s2);

              if (s1 < 0 && s2 < 0){
                s = -1 * s;
              }
              if (s1 < 0 && s2 == 0){
                s = -1 * s;
              }
              if (s1 == 0 && s2 < 0){
                s = -1 * s;
              }
              if (s1 == 0.001 && s2 < 0){
                s = -1 * s;
              }
              if (r < 0 || r > atan(1)){
                if (r < 0){
                  r = 0;
                  s = s1;
                }
                else{
                  r = atan(1);
                  s = (e0 - e2) / (DataResolution * sqrt(2)); //diagonal cell length
                }
              }
              if (s >= maxSlope && s != 0.00001){
                maxSlope = s;
                flowDir = afVals[a] * r + acVals[a] * (pi / 2);
              }
            }
            //calculate slope (s) and bearing (r) along edges
            //of the facet when e0 is higher than e1 or e2
            else if (e0 > e1 || e0 > e2){
              if (e0 > e1){
                r = 0;
                s = (e0 - e1) / DataResolution;
              }
              else{
                r = atan(1);
                s = (e0 - e2) / (DataResolution * sqrt(2));
              }
              if (s >= maxSlope && s != 0.00001){
                maxSlope = s;
                flowDir = afVals[a] * r + acVals[a] * (pi / 2);
              }
            }
          }
        }

        if (maxSlope <= 0){
          FlowDir_Array[i][j] = -1;  //unresolved - Tarboton uses D8 to fill these pits - we have a better fill algorithm
        }
        else{
          flowDir = round((flowDir * (180 / pi)) * 10) / 10;
          flowDir = 360 - flowDir + 90;
          if (flowDir > 360){
            flowDir = flowDir - 360;
          }
          FlowDir_Array[i][j] = flowDir;
        }
      }
    }
  }

  return FlowDir_Array;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//Function to write the D-infinity flow directions to an LSDRaster.
//
//SWDG - 26/7/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::write_dinf_flowdir_to_LSDRaster(Array2D<double> dinflow){

  LSDRaster FlowDirection(NRows, NCols, XMinimum, YMinimum, DataResolution,
                          NoDataValue, dinflow);

  return FlowDirection;

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//Wrapper Function to create a D-infinity flow area raster with one function call.
//
//SWDG - 26/7/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::D_inf(){

  Array2D<double> Dinf_flow = D_inf_FlowDir();
  LSDRaster Dinf_area = D_inf_FlowArea(Dinf_flow);

  return Dinf_area;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//end of d-inf modules
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Driver function for the recursive hilltop flow routing routine. 
//
// Flow is routed from hilltop pixels down to the valley floor, based on the
// Lea (1991) "An aspect driven kinematic routing algorithm" paper, where the
// flow is routed across each cell from an inlet point (xi,yi) to an outlet
// (xo,yo) and the flow length is measured between these points. The benefit of 
// this is that flow is not constrained by the gridded nature of the data.
//
// Hilltop flow routing can be performed by calling this driver function with an 
// LSDRaster of hilltops, and Array2D of flowdirections, an Array2D of the stream
// network, an indexraster of the study basins and an output filename prefix.
//
// The basin array does not need to be supplied, as it is just used to code hilltops 
// to specific basins in the output. If this is not needed an LSDIndexRaster of NoDataValues 
// of the correct dimensions can be passed in instead.  
// 
// Returns a vector of Array2D<double> objects which are the hilltop network 
// coded with the hilltop metric values calculated for that pixel. This data is 
// also provided in the output text file written into the current path with the 
// filename <prefix>_HIlltopData.txt and the data within holds the format:
// "hilltop_i hilltop_j hilltop_easting hilltop_northing stream_i stream_j stream_easting stream_northing stream_id basin_id relief lh aspect slope"
//
// The structure of the returned vector< Array2D<double> > is as follows:
// [0] Hilltop Network coded with stream ID
// [1] Hillslope Lengths
// [2] Slope
// [3] Apect
// [4] Relief
// 
// SWDG 3/10/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
vector< Array2D<double> > LSDRaster::HFR_Driver(LSDRaster Hilltops, Array2D<double> FlowDir, LSDIndexRaster StreamNetwork, LSDIndexRaster Basins_Raster, string prefix){

  //Initialize all the data arrays for stroring input and output
  Array2D<double> HilltopArray = Hilltops.get_RasterData();
  Array2D<int> StreamNet = StreamNetwork.get_RasterData();
  Array2D<double> RoutedHilltops(NRows,NCols,NoDataValue);
  Array2D<int> Basins = Basins_Raster.get_RasterData();
  Array2D<double> HillslopeLength_Array(NRows,NCols,NoDataValue);
  Array2D<double> Slope_Array(NRows,NCols,NoDataValue);
  Array2D<double> Aspect_Array(NRows,NCols,NoDataValue);
  Array2D<double> Relief_Array(NRows,NCols,NoDataValue);

  //vector to store row of data for each hilltop pixel that is routed successfully to a stream 
  vector<string> HilltopData;
  
  //vector to store the output data arrays in one vector that can be returned
  vector< Array2D<double> > OutputArrays;

  cout << "\n\nPerforming Hilltop Flow Routing" << endl;

  //loop over every hilltop pixel and call the recursive hilltop cell function to start the trace
  for (int i = 1; i < NRows - 1; ++i){   
    cout << flush << "\t\tRow: " << i+1 << " of " << NRows-1 << "\r";
    for (int j = 1; j < NCols - 1; ++j){
      if (HilltopArray[i][j] == 1){ 
        Array2D<int> visited(NRows,NCols); //must reset the visited flags after every hilltop pixel trace - DO NOT INITIALIZE THE ARRAY VALUES IT IS A MASSIVE PERFORMANCE DRAIN    
        HilltopCell(i, j, visited, StreamNet, FlowDir, RasterData, i, j, RoutedHilltops, Basins, HillslopeLength_Array, Slope_Array, Aspect_Array, Relief_Array, HilltopData); 
      }
    }
  }
  
  //add the data arrays to the output vector
  OutputArrays.push_back(RoutedHilltops);
  OutputArrays.push_back(HillslopeLength_Array);
  OutputArrays.push_back(Slope_Array);
  OutputArrays.push_back(Aspect_Array);
  OutputArrays.push_back(Relief_Array);
  
  //create the output filename from the user supplied filename prefix
  stringstream ss_filename;
  ss_filename << prefix << "_HilltopData.txt";
  
  cout << "\nWriting Hilltop data to " << ss_filename.str()<< endl;
  
  ofstream HilltopDataWriter;  
  HilltopDataWriter.open(ss_filename.str().c_str());
  
  //write file headers
  HilltopDataWriter << "hilltop_i hilltop_j hilltop_easting hilltop_northing stream_i stream_j stream_easting stream_northing stream_id basin_id relief lh aspect slope" << endl;
    
  //write the hilltop pixel data to the opened file    
  for (int a = 0; a < int(HilltopData.size()); ++a){
    HilltopDataWriter << HilltopData[a] << endl;  
  }
    
  HilltopDataWriter.close();
  
  return OutputArrays;
   
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Fucnction to initialize hilltop flow routing, by starting in the centre
// of the hilltop cell and finding it's outlet point based on the flowdirection.
//
// Should not be called directly: is called by the HFR_Driver, as several variables
// must be correctly initalized for the trace to work.
// 
// SWDG 3/10/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void LSDRaster::HilltopCell(int i, int j, Array2D<int>& Visited, Array2D<int> StreamNet, Array2D<double> FlowDir, Array2D<double> Elevation, int old_i, int old_j, Array2D<double>& RoutedHilltops, Array2D<int> Basins, Array2D<double>& HillslopeLength_Array, Array2D<double>& Slope_Array, Array2D<double>& Aspect_Array, Array2D<double>& Relief_Array, vector<string>& HilltopData){

  //initialize the x and y inlet coordinates
  double xi = 0.5;
  double yi = 0.5;
  
  double theta = BearingToRad(FlowDir[i][j]);
  Visited[i][j] = 1; //mark hilltop as visited
  
  if (FlowDir[i][j] >= 0 && FlowDir[i][j] < 361 && (i != 0 || i != NRows - 1) && (j != 0 || j != NCols - 1)) {
  
    //Test the flow direction and update the i,j indices to go to the correct next cell 
    //and set the outlet points onto the correct face
    if (FlowDir[i][j] >= 45 && FlowDir[i][j] < 135){ //east
      xi = 0;
      yi = (1 + tan(theta)) / 2;
      ++j;
    }
    else if (FlowDir[i][j] >= 315 || FlowDir[i][j] < 45){ //north
      xi = (1 + (1/tan(theta)))/2;
      yi = 0;  
      --i;
    }
    else if (FlowDir[i][j] >= 225 && FlowDir[i][j] < 315){ //west
      xi = 1;
      yi = (1 - tan(theta))/2;
      --j;
    }  
    else if (FlowDir[i][j] >= 135 && FlowDir[i][j] < 225){ //south
      xi = (1 - (1/tan(theta)))/2;
      yi = 1; 
      ++i;
    }
        
    double TotalLength = sqrt(pow((xi - 0.5),2) + pow((yi - 0.5),2));
    
    //Call the HFR function to start the trace downslope  
    HFR(i, j, xi, yi, Visited, StreamNet, FlowDir, TotalLength, Elevation, old_i, old_j, RoutedHilltops, Basins, HillslopeLength_Array, Slope_Array, Aspect_Array, Relief_Array, HilltopData);
  
  }

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Main Hilltop flow routing function, once called by the hilltop cell function
// it will keep calling itself to find the nearest stream pixel.
//
// If no pixel can be found the trace will end gracefully and allow the next 
// hilltop trace to begin. Writes data to a vector which is written into a file
// by the HFR_Driver.  
// 
// SWDG 3/10/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void LSDRaster::HFR(int i, int j, double xi, double yi, Array2D<int>& Visited, Array2D<int> StreamNet, Array2D<double> FlowDir, double& TotalLength, Array2D<double> Elevation, int old_i, int old_j, Array2D<double>& RoutedHilltops, Array2D<int> Basins, Array2D<double>& HillslopeLength_Array, Array2D<double>& Slope_Array, Array2D<double>& Aspect_Array, Array2D<double>& Relief_Array, vector<string>& HilltopData){

  // x and y outlet coordinates
  double xo = 0;
  double yo = 0;
  
  if (FlowDir[i][j] >= 0 && FlowDir[i][j] < 361 && (i != 0 || i != NRows - 1) && (j != 0 || j != NCols - 1)) {

    double theta = BearingToRad(FlowDir[i][j]);
    
    if (xi == 1){ // east face
    
      if (FlowDir[i][j] > 180 && FlowDir[i][j] < 360){ //Flow meets another edge 
      
        xo = xi + (1 - yi) * (1/tan(theta));  //north edge
        yo = 1;  
  
        if (xo > 1 || xo < 0){  //west
          xo = 0;
          yo = yi - xi * tan(theta);      
        }

        if (yo > 1 || yo < 0){  //south
          xo = xi - yi * (1/tan(theta));
          yo = 0;      
        }
      }
      else{ //Flow does not not meet another edge -> Must go either N/S
   
        if (FlowDir[i][j] >= 90){ //go south
          xo = 0.99999;
          yo = 0;
        }
        if (FlowDir[i][j] < 90){ //go north
          xo = 0.99999;
          yo = 1;
        }   
      }   
    }
    else if (yi == 1){ // north face
    
      if (FlowDir[i][j] < 270 && FlowDir[i][j] > 90){   //Flow meets another edge
  
        xo = 0;  //west edge
        yo = yi - xi * tan(theta);  
  
        if (yo > 1 || yo < 0){  //south
          xo = xi - yi * (1/tan(theta));
          yo = 0;      
        }

        if (xo > 1 || xo < 0){ //east
          xo = 1;
          yo = yi + (1 - xi) * tan(theta);
        }
      }
      else{ //Flow does not not meet another edge -> Must go either E/W
   
        if (FlowDir[i][j] >= 270){ //go west
          xo = 0;
          yo = 0.99999;
        }
        if (FlowDir[i][j] <= 90){ //go east
          xo = 1;
          yo = 0.99999;
        }   
      }   
    }
    else if (xi == 0){ // west face
    
      if (FlowDir[i][j] > 0 && FlowDir[i][j] < 180){ //Flow meets another edge
  
        xo = xi - yi * (1/tan(theta)); //south
        yo = 0;    
  
        if (xo > 1 || xo < 0){  //east
          xo = 1;
          yo = yi + (1 - xi) * tan(theta);    
        }

        if (yo > 1 || yo < 0){ //north
        xo = xi + (1 - yi) * (1/tan(theta));
        yo = 1;  
        }
      }
      else{ //Flow does not not meet another edge -> Must go either N/S
   
        if (FlowDir[i][j] >= 270){ //go north
          xo = 0.00001;
          yo = 1;
        }
        if (FlowDir[i][j] < 270){ //go south
          xo = 0.00001;
          yo = 0;
        }   
      }   
    }    
    else if (yi == 0){ // south face
    
      if (FlowDir[i][j] > 270 || FlowDir[i][j] < 90){    //Flow meets another edge 
  
        xo = 1; // east
        yo = yi + (1 - xi) * tan(theta);    
  
        if (yo > 1 || yo < 0){ //north
          xo = xi + (1 - yi) * (1/tan(theta));
          yo = 1;  
        }
        
        if (xo > 1 || xo < 0){  //west
          xo = 0;
          yo = yi - xi * tan(theta);      
        }
      }
      else{ //Flow does not not meet another edge -> Must go either E/W
        if (FlowDir[i][j] >= 180){ //go west
          xo = 0;
          yo = 0.00001;
        }
        if (FlowDir[i][j] < 180){ //go east
          xo = 1;
          yo = 0.00001;
        }   
      }   
    } 
    
    //update trace length and flag the cell as visited    
    TotalLength += sqrt(pow((xi - xo),2) + pow((yi - yo),2));
    Visited[i][j] = 1;
     
    //get new i, j, and update xi, yi to reflect next cell to visit
    if (xo == 1){ //east
      ++j;
      xi = 0;
      yi = yo;
    }  
    else if (xo == 0){//west
      --j;
      xi = 1;
      yi = yo;
    }  
    else if (yo == 1){//north   
      --i;
      xi = xo;
      yi = 0;
    }  
    else if (yo == 0){//south
      ++i;
      xi = xo;
      yi = 1;
    }  
    
    if (StreamNet[i][j] > 0){ //will only find stream pixels, should ignore nodata values
      //Stream reached
      RoutedHilltops[old_i][old_j] = StreamNet[i][j]; //code hilltop px with stream order or unique segment ID depending on values in StreamNet
      
      //calculate metrics for this hilltop px
    
      double relief = Elevation[old_i][old_j] - Elevation[i][j];
      TotalLength = TotalLength * DataResolution;
      double slope = relief / TotalLength;
      double basin_id = Basins[old_i][old_j];
          
      // Determine aspect of flow routing from hilltop->channel - from original code by Martin Hurst
      double aspect = 0;
      double delta_j = old_j - j;
      double delta_i = old_i - i;
              
      if (old_i > i && old_j > j){ aspect = 180 - atan2(delta_i, delta_j);}               // SE
      else if (old_i > i && old_j < j){ aspect = -1 * atan2(delta_i, delta_j);}           // NE
      else if (old_i < i && old_j < j){ aspect = 360 - atan2(delta_i, delta_j);}          // NW
      else if (old_i < i && old_j > j){ aspect = 180 - atan2(delta_i, delta_j);}          // SW
      else if (old_i == i && old_j > j){ aspect = 90;}                                    // E
      else if (old_i == i && old_j < j){ aspect = 270;}                                   // W
      else if (old_i > i && old_j == j){ aspect = 180;}                                   // S
      else if (old_i < i && old_j == j){ aspect = 0;}                                     // N
      else{ aspect = NoDataValue;}
      if (aspect > 360){aspect -= 360;}

      //eastings
      double hilltop_easting = (old_j * DataResolution) + XMinimum;
      double stream_easting = (j * DataResolution) + XMinimum;
      
      //northings
      double hilltop_northing = ((old_i - NRows) * DataResolution) + YMinimum;
      double stream_northing = ((i - NRows) * DataResolution) + YMinimum;
  
      // update arrays with the current metrics 
      HillslopeLength_Array[old_i][old_j] = TotalLength;
      Slope_Array[old_i][old_j] = slope;
      Aspect_Array[old_i][old_j] = aspect;
      Relief_Array[old_i][old_j] = relief;  
  
      //concatenate string of this hilltop's data and concatenate string to the data vector 
      stringstream output_data;
      output_data <<  old_i << " " << old_j << " " << hilltop_easting << " " << hilltop_northing << " " << 
      i << " " << j << " " << stream_easting << " " << stream_northing << " " << StreamNet[i][j] << " " <<
      basin_id << " " << relief << " " << TotalLength << " " << aspect << " " << slope; 
      
      HilltopData.push_back(output_data.str());      
      
    }
    else if (Visited[i][j] != 1){
      //call recursive fn to continue downslope trace
      HFR(i, j, xi, yi, Visited, StreamNet, FlowDir, TotalLength, Elevation, old_i, old_j, RoutedHilltops, Basins, HillslopeLength_Array, Slope_Array, Aspect_Array, Relief_Array, HilltopData);
    }
  }  
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Generate data in two text files to create a boomerang plot as in Roering et al [2007].
// SWDG 27/8/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void LSDRaster::Boomerang(LSDRaster& Slope, LSDRaster& Dinf, string RasterFilename, double log_bin_width){

  Array2D<double> slope = Slope.get_RasterData();
  Array2D<double> area = Dinf.get_RasterData();

  //do some log binning
  vector<double> Mean_x_out;
  vector<double> Mean_y_out;
  vector<double> Midpoints_out;
  vector<double> STDDev_x_out;
  vector<double> STDDev_y_out;
  vector<double> STDErr_x_out;
  vector<double> STDErr_y_out;

  log_bin_data(area, slope, log_bin_width, Mean_x_out, Mean_y_out, Midpoints_out, STDDev_x_out, STDDev_y_out,STDErr_x_out,STDErr_y_out,NoDataValue);

  //set up a filestream object
  ofstream file;

  stringstream ss_bin;
  ss_bin << RasterFilename << "_boom_binned.txt";
  file.open(ss_bin.str().c_str());   //needs a null terminated character array, not a string. See pg 181 of accelerated c++


  for(int q = 0; q < int(Mean_x_out.size()); q++){
    file << Mean_x_out[q] << " " << Mean_y_out[q] << " " << STDDev_x_out[q] << " " << STDDev_y_out[q] << " " << STDErr_x_out[q] << " " << STDErr_y_out[q] << endl;
  }

  file.close();

  //data cloud
  ofstream cloud;

  stringstream ss_cloud;
  ss_cloud << RasterFilename << "_boom_cloud.txt";
  cloud.open(ss_cloud.str().c_str());     //needs a null terminated character array, not a string. See pg 181 of accelerated c++


  for (int i = 1; i < NRows-1; ++i){
    for (int j = 1; j < NCols-1; ++j){
      if(area[i][j] != NoDataValue && slope[i][j] != NoDataValue){
        cloud << area[i][j] << " " << slope[i][j] << endl;
      }
    }
  }

  cloud.close();

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Punch basins out of an LSDRaster to create DEMs of a single catchment.
//
// Writes files in the user supplied format (flt or asc) and returns a vector
// LSDRasters so they can be loaded into other functions.
// Updated 24/9/13 to return a vector of LSDRasters SWDG
// SWDG 27/8/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
vector<LSDRaster> LSDRaster::BasinPuncher(vector<int> basin_ids, LSDIndexRaster BasinArray){

  Array2D<int> BasinRaster = BasinArray.get_RasterData();

  vector<LSDRaster> BasinVector; //vector to contain individual basin LSDRasters

  for(string::size_type a = 0; a < basin_ids.size(); ++a){

    Array2D<double> BasinDEM(NRows, NCols, NoDataValue);
    bool Flag = false;

    for (int i=0; i<NRows; ++i){
		  for (int j=0; j<NCols; ++j){
		    if(BasinRaster[i][j] == basin_ids[a]){
		      Flag = true;
          BasinDEM[i][j] = RasterData[i][j];
        }
		  }
		}

    if (Flag == true){ //only write the raster if there is data to write
      LSDRaster Basin(NRows,NCols,XMinimum,YMinimum,DataResolution,NoDataValue,BasinDEM);
      LSDRaster TidyBasin = Basin.RasterTrimmer();
      BasinVector.push_back(TidyBasin);
    }
  }
  return BasinVector;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Collect all basin average metrics into a single file.
//
// File is written with the format:
// "basin_id slope elevation aspect area drainage_density hilltop_curvature hillslope_length mean_slope hilltop_relief hilltop_aspect E* R*"
// SWDG 27/8/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void LSDRaster::CollectBasinMetrics(LSDIndexRaster& Basins, LSDRaster& Slope, LSDRaster& Elevation, LSDRaster& Aspect,
                              LSDRaster& Area, LSDRaster& DrainageDensity, LSDRaster& Cht, LSDRaster& HillslopeLength,
                              LSDRaster& MeanSlope, LSDRaster& Relief, LSDRaster& MeanAspect, double CriticalSlope)
{

  vector<int> basin_index;
  Array2D<int> basin_ids = Basins.get_RasterData();


  //vectors to contain output data
  vector<int> BasinIDVector;
  vector<double> SlopeVector;
  vector<double> ElevationVector;
  vector<double> AspectVector;
  vector<double> AreaVector;
  vector<double> DrainageDensityVector;
  vector<double> ChtVector;
  vector<double> HillslopeLengthVector;
  vector<double> MeanSlopeVector;
  vector<double> ReliefVector;
  vector<double> MeanAspectVector;
  vector<double> EStarVector;
  vector<double> RStarVector;


  //make list of unique basins in each raster
  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      int id = basin_ids[i][j];
      if (id != NoDataValue){
        //check if next basin_id is unique
        if(find(basin_index.begin(), basin_index.end(), id) == basin_index.end()){
          basin_index.push_back(id);
        }
      }
    }
  }

  //loop through each basin
  for (vector<int>::iterator it = basin_index.begin(); it !=  basin_index.end(); ++it){

    int SlopeCounter = 0;
    double SlopeSum = 0;
    double ElevationSum = 0;
    int ElevationCounter = 0;
    double AspectSum = 0;
    int AspectCounter = 0;
    double AreaSum = 0;
    int AreaCounter = 0;
    double DrainageDensitySum = 0;
    int DrainageDensityCounter = 0;
    double ChtSum = 0;
    int ChtCounter = 0;
    double HillslopeLengthSum = 0;
    int HillslopeLengthCounter = 0;
    double MeanSlopeSum = 0;
    int MeanSlopeCounter = 0;
    double ReliefSum = 0;
    int ReliefCounter = 0;
    double MeanAspectSum = 0;
    int MeanAspectCounter = 0;

    for (int i = 0; i < NRows; ++i){
      for (int j = 0; j < NCols; ++j){

       if (Slope.get_data_element(i,j) != NoDataValue && basin_ids[i][j] == *it ){
         SlopeSum += Slope.get_data_element(i,j);
         ++SlopeCounter;
        }
       if (Elevation.get_data_element(i,j) != NoDataValue && basin_ids[i][j] == *it ){
         ElevationSum += Elevation.get_data_element(i,j);
         ++ElevationCounter;
        }
       if (Aspect.get_data_element(i,j) != NoDataValue && basin_ids[i][j] == *it ){
         AspectSum += Aspect.get_data_element(i,j);
         ++AspectCounter;
        }
       if (Area.get_data_element(i,j) != NoDataValue && basin_ids[i][j] == *it ){
         AreaSum += Area.get_data_element(i,j);
         ++AreaCounter;
        }
       if (DrainageDensity.get_data_element(i,j) != NoDataValue && basin_ids[i][j] == *it ){
         DrainageDensitySum += DrainageDensity.get_data_element(i,j);
         ++DrainageDensityCounter;
        }
       if (Cht.get_data_element(i,j) != NoDataValue && basin_ids[i][j] == *it ){
         ChtSum += Cht.get_data_element(i,j);
         ++ChtCounter;
        }
       if (HillslopeLength.get_data_element(i,j) != NoDataValue && basin_ids[i][j] == *it ){
         HillslopeLengthSum += HillslopeLength.get_data_element(i,j);
         ++HillslopeLengthCounter;
        }
       if (MeanSlope.get_data_element(i,j) != NoDataValue && basin_ids[i][j] == *it ){
         MeanSlopeSum += MeanSlope.get_data_element(i,j);
         ++MeanSlopeCounter;
        }
       if (Relief.get_data_element(i,j) != NoDataValue && basin_ids[i][j] == *it ){
         ReliefSum += Relief.get_data_element(i,j);
         ++ReliefCounter;
        }
       if (MeanAspect.get_data_element(i,j) != NoDataValue && basin_ids[i][j] == *it ){
         MeanAspectSum += MeanAspect.get_data_element(i,j);
         ++MeanAspectCounter;
        }

      }
    }

    //calculate means
    double AVGSlope = SlopeSum/SlopeCounter;
    double AVGElevation = ElevationSum/ElevationCounter;
    double AVGAspect = AspectSum/AspectCounter;
    double AVGArea = AreaSum/AreaCounter;
    double AVGDrainageDensity = DrainageDensitySum/DrainageDensityCounter;
    double AVGCht = ChtSum/ChtCounter;
    double AVGHillslopeLength = HillslopeLengthSum/HillslopeLengthCounter;
    double AVGMeanSlope = MeanSlopeSum/MeanSlopeCounter;
    double AVGRelief = ReliefSum/ReliefCounter;
    double AVGMeanAspect = MeanAspectSum/MeanAspectCounter;
    double EStar = (2 * (abs(AVGCht)) * AVGHillslopeLength) / CriticalSlope;
    double RStar = AVGRelief / (AVGHillslopeLength * CriticalSlope);

    //write means to vectors
    BasinIDVector.push_back(*it);
    SlopeVector.push_back(AVGSlope);
    ElevationVector.push_back(AVGElevation);
    AspectVector.push_back(AVGAspect);
    AreaVector.push_back(AVGArea);
    DrainageDensityVector.push_back(AVGDrainageDensity);
    ChtVector.push_back(AVGCht);
    HillslopeLengthVector.push_back(AVGHillslopeLength);
    MeanSlopeVector.push_back(AVGMeanSlope);
    ReliefVector.push_back(AVGRelief);
    MeanAspectVector.push_back(AVGMeanAspect);
    EStarVector.push_back(EStar);
    RStarVector.push_back(RStar);

  }


  ofstream file;
  file.open("Basin_metrics.txt");
  file << "basin_id slope elevation aspect area drainage_density hilltop_curvature hillslope_length mean_slope hilltop_relief hilltop_aspect E* R*" << endl;

  for(int q = 0; q < int(BasinIDVector.size()); q++){
    file << BasinIDVector[q] << " " << SlopeVector[q] << " " << ElevationVector[q] << " " << AspectVector[q] << " " << AreaVector[q] << " " << DrainageDensityVector[q] << " " << ChtVector[q] << " " << HillslopeLengthVector[q] << " " << MeanSlopeVector[q] <<  " " << ReliefVector[q] << " " << MeanAspectVector[q] << " " << EStarVector[q] << " " << RStarVector[q] << endl;
  }

  file.close();



}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Module to sample LSDRaster values running along a ridgetop network. Pass in
// a TNT array of doubles that denotes the ridge network. Ridge network is
// generated from LSDChannelNetwork::ExtractRidges
//
// Returns sampled LSDRaster object
//
// SWDG 04/2013
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::RidgeSample(Array2D<double>& Ridges){

  Array2D<double> Sample_data(NRows,NCols,NoDataValue);

  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      if (RasterData[i][j] != NoDataValue && Ridges[i][j] != NoDataValue ){
        Sample_data[i][j] = RasterData[i][j];
      }
    }
  }

  LSDRaster Sample(NRows,NCols, XMinimum, YMinimum, DataResolution, NoDataValue, Sample_data);
  return Sample;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Pass a smoothing window over a ridge LSDRaster object to calculate an average
// value running along the ridgetop.
//
// Pass in an optional integer smoothing window radius between 1 and 6.
// Default value is 2
//
// Returns LSDRaster object containing the averaged data.
//
// SWDG 04/2013
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::RidgeSmoother(int WindowRadius){

  //arbitrary upper bound to limit too large a radius
  if (WindowRadius < 1 || WindowRadius > 6){
    WindowRadius = 2; //
  }

  Array2D<double> Smoothed(NRows,NCols,NoDataValue);

  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      double sum = 0;
      int counter = 0;
      if (RasterData[i][j] != NoDataValue){
          for (int a = (-1*WindowRadius); a <= WindowRadius; ++a){
              for (int b = (-1*WindowRadius); b <= WindowRadius; ++b){
                if(a >= 0 && a <= NRows){
                  if(b >= 0 && b <= NCols){
                    if (RasterData[i+a][j+b] != NoDataValue){
                        sum += RasterData[i+a][j+b];
                        ++counter;
                    }
                  }
                }
             }
          }
      Smoothed[i][j] = sum/counter;
      }
    }
  }

  LSDRaster Smooth(NRows,NCols, XMinimum, YMinimum, DataResolution, NoDataValue, Smoothed);
  return Smooth;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Pass a buffer over a ridge LSDRaster object to increase sampling area.
//
// Pass in an optional integer buffer radius between 1 and 6.
// Default value is 2
//
// Returns LSDRaster object denoting the buffered ridge data.
//
// Buffers equally in all directions, so use with care to avoid sampling areas
// away from the axis of the original ridge line.
//
// SWDG 04/2013
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::RidgeBuffer(int BufferRadius){

  Array2D<double> HilltopBuffer(NRows, NCols, NoDataValue);

  //arbitrary upper bound to limit too large a buffer
  if (BufferRadius < 1 || BufferRadius > 6){
    BufferRadius = 2;
  }

  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      if (RasterData[i][j] != NoDataValue){
        for (int a = (-1 * BufferRadius); a <= BufferRadius; ++a){
          for (int b = (-1 * BufferRadius); b <= BufferRadius; ++b){
            if(i + a >= 0 && i + a <= NRows){
              if(j + b >= 0 && j + b <= NCols){
                HilltopBuffer[i + a][j + b] = RasterData[i][j];
              }
            }
          }
        }
      }
    }
  }

  LSDRaster Buffer(NRows, NCols, XMinimum, YMinimum, DataResolution, NoDataValue, HilltopBuffer);
  return Buffer;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Module assigns an average CHT (or other input LSDRaster value) to each basin,
// works by searching for every hilltop value that falls within a basin, summing
// these values and writing the final average to every cell identified as the
// basin in question.
//
// Pass in an LSDIndexRaster of Drainage basins, generated using
// ChannelNetwork::ExtractBasinsOrder
//
// Returns LSDRaster of average basin CHT for each identified basin.
//
// Very inefficent at present. Module loops through every cell in LSDRaster
// (2 * number of basins) + 1 times. Beware!
// Bug fixed in assignment of basin IDs - SWDG 2/9/13
// SWDG 04/2013
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::BasinAverager(LSDIndexRaster& Basins){

  vector<int> basin_index;
  Array2D<int> basin_ids = Basins.get_RasterData();

  Array2D<double> Averaged(NRows,NCols,NoDataValue);

  //make list of unique basins in each raster
  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      int id = basin_ids[i][j];
      if (id != NoDataValue){
        //check if next basin_id is unique
        if(find(basin_index.begin(), basin_index.end(), id) == basin_index.end()){
          basin_index.push_back(id);
        }
      }
    }
  }

  //loop through each basin
  for (vector<int>::iterator it = basin_index.begin(); it !=  basin_index.end(); ++it){
    int counter = 0;
    double sum = 0;

    for (int i = 0; i < NRows; ++i){
      for (int j = 0; j < NCols; ++j){

       if (RasterData[i][j] != NoDataValue && basin_ids[i][j] == *it ){
         sum += RasterData[i][j];
         ++counter;
        }
      }
    }

    for (int i = 0; i < NRows; ++i){
      for (int j = 0; j < NCols; ++j){
        if(basin_ids[i][j] == *it){
          Averaged[i][j] = sum/counter;
        }
      }
    }
  }

  LSDRaster Averaged_out(NRows,NCols, XMinimum, YMinimum, DataResolution, NoDataValue, Averaged);
  return Averaged_out;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=
// Write the area(in units of area) of each basin to the basin's pixels.
// SWDG 04/2013
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::BasinArea(LSDIndexRaster& Basins){

  vector<int> basin_index;
  Array2D<int> basin_ids = Basins.get_RasterData();

  Array2D<double> Areas(NRows,NCols,NoDataValue);

  //make list of unique basins in each raster
  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      int id = basin_ids[i][j];
      if (id != NoDataValue){
        //check if next basin_id is unique
        if(find(basin_index.begin(), basin_index.end(), id) == basin_index.end()){
          basin_index.push_back(id);
        }
      }
    }
  }

  //loop through each basin
  for (vector<int>::iterator it = basin_index.begin(); it !=  basin_index.end(); ++it){
    int counter = 0;

    for (int i = 0; i < NRows; ++i){
      for (int j = 0; j < NCols; ++j){

       if (basin_ids[i][j] == *it){
         ++counter;
        }
      }
    }

    for (int i = 0; i < NRows; ++i){
      for (int j = 0; j < NCols; ++j){
        if(basin_ids[i][j] == *it){
          Areas[i][j] = counter*(DataResolution*DataResolution);
        }
      }
    }
  }

  LSDRaster Area_out(NRows,NCols, XMinimum, YMinimum, DataResolution, NoDataValue, Areas);
  return Area_out;

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=
// Calulate drainage density of a set of input basins.
//
// Calculated as flow length/basin area.
//
// SWDG 04/2013
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::DrainageDensity(LSDIndexRaster& StreamNetwork, LSDIndexRaster& Basins, Array2D<int> FlowDir){

  vector<int> basin_index;
  Array2D<int> basin_ids = Basins.get_RasterData();
  double two_times_root2 = 2.828427;

  Array2D<double> Density(NRows,NCols,NoDataValue);

  //make list of unique basins in each raster
  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      int id = basin_ids[i][j];
      if (id != NoDataValue){
        //check if next basin_id is unique
        if(find(basin_index.begin(), basin_index.end(), id) == basin_index.end()){
          basin_index.push_back(id);
        }
      }
    }
  }

  //loop through each basin
  for (vector<int>::iterator it = basin_index.begin(); it !=  basin_index.end(); ++it){

    int stream_px = 0;
    int hillslope_px = 0;
    double stream_length = 0;

    for (int i = 0; i < NRows; ++i){
      for (int j = 0; j < NCols; ++j){
        if (RasterData[i][j] != NoDataValue && basin_ids[i][j] != *it ){
          if (StreamNetwork.get_data_element(i,j) != NoDataValue){

            if ((FlowDir[i][j] % 2) != 0 && (FlowDir[i][j] != -1 )){ //is odd but not -1
              stream_length += DataResolution * two_times_root2; //diagonal
              ++stream_px;
            }
            else if (FlowDir[i][j] % 2 == 0){  //is even
              stream_length += DataResolution;  //cardinal
              ++stream_px;
            }
          }
          else{
            ++hillslope_px;
          }
        }
      }
    }
    double density = (stream_length / ((hillslope_px+stream_px)*(DataResolution*DataResolution)));
    for (int i = 0; i < NRows; ++i){
      for (int j = 0; j < NCols; ++j){
        if(basin_ids[i][j] == *it){
          Density[i][j] = density;
        }
      }
    }

  }

  LSDRaster DrainageDensity(NRows, NCols, XMinimum, YMinimum, DataResolution, NoDataValue, Density);
  return DrainageDensity;

}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Function built around original c++ code by Martin Hurst to generate a flowarea
// raster.
//
// Computes the proportion of all downslope flows for each cell in the input
// DEM and routes the flow accordingly. Consequently the dem is sorted and indexed
// using LSDStatsTools.
//
// Can handle DEMs containing flats, but pits must be filled using the new
// LSDRaster fill.
//
// Outputs an LSDRaster
//
// SWDG, 18/4/13
//
// Updated 23/4/13 to allow periodic boundary condtitions - SWDG
// Needs to be able to handle the boundary cond vector and only reflect bondaries when needed.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::MDFlow(vector<string> BoundaryConditions){

  int Row2 = NRows + 2;
  int Col2 = NCols + 2;

  //create output array, populated with nodata
  Array2D<double> area_final(NRows, NCols, NoDataValue);

  Array2D<double> elev_pad(Row2, Col2, NoDataValue);
  Array2D<double> area(Row2, Col2, NoDataValue);

  //declare variables
  vector<double> flat;
  vector<double> sorted;
  vector<size_t> index_map;
  double one_ov_root_2 = 0.707106781187;


  //translate the data down and right by one cell to create a border
  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
        elev_pad[i+1][j+1] = RasterData[i][j];
    }
  }

  //reflect boundaries
  for (int i = 0; i < Row2; ++i){
    for (int j = 0; j < Col2; ++j){
      if(i==0 && j != 0 && j != Col2-1){
        //top row
        elev_pad[i][j] = elev_pad[Row2-2][j];
      }
      if(i==Row2-1 && j != 0 && j != Col2-1){
        //bottom row
        elev_pad[i][j] = elev_pad[1][j];
      }

      if(j==0 && i != 0 && i != Row2-1){
        //left column
        elev_pad[i][j] = elev_pad[i][Col2-2];
      }

      if(j==Col2-1 && i!=0 && i != Row2-1){
        //right column
        elev_pad[i][j] = elev_pad[i][1];
      }
    }
  }

  //loop through the dem cells creating a row major 1D vector, flat, and
  //setting the cell area to every non ndv cell
  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      flat.push_back(elev_pad[i][j]);
      if (elev_pad[i][j] != NoDataValue){
        area[i][j] = DataResolution*DataResolution;
      }
    }
  }

  //sort the 1D elevation vector and produce an index
  matlab_double_sort_descending(flat, sorted, index_map);

  for(int q = 0 ;q < int(flat.size()); ++q){

    if (sorted[q] != NoDataValue){

		  //use row major ordering to reconstruct each cell's i,j coordinates
  	  int i = index_map[q] / NCols;
   	  int j = index_map[q] % NCols;

      //skip edge cells
      if (i != 0 && j != 0 && i != NRows-1 && j != NCols-1){

        //reset variables on each loop
			  double total = 0;
			  double slope1 = 0;
        double slope2 = 0;
        double slope3 = 0;
        double slope4 = 0;
        double slope5 = 0;
        double slope6 = 0;
        double slope7 = 0;
        double slope8 = 0;

        //Get sum of magnitude of downslope flow, total, and store the magnitude of
        //each of the 8 downslope cells as slope1->8 *Avoids NDVs*
			  if (elev_pad[i][j] > elev_pad[i-1][j-1] && elev_pad[i-1][j-1] != NoDataValue){
          slope1 = (elev_pad[i][j] - elev_pad[i-1][j-1]) * one_ov_root_2;
          total += slope1;
        }
			  if (elev_pad[i][j] > elev_pad[i-1][j] && elev_pad[i-1][j] != NoDataValue){
          slope2 = elev_pad[i][j] - elev_pad[i-1][j];
          total += slope2;
			  }
		  	if (elev_pad[i][j] > elev_pad[i-1][j+1] && elev_pad[i-1][j+1] != NoDataValue){
          slope3 = (elev_pad[i][j] - elev_pad[i-1][j+1]) * one_ov_root_2;
          total += slope3;
		  	}
			  if (elev_pad[i][j] > elev_pad[i][j+1] && elev_pad[i][j+1] != NoDataValue){
          slope4 = elev_pad[i][j] - elev_pad[i][j+1];
          total += slope4;
        }
			  if (elev_pad[i][j] > elev_pad[i+1][j+1] && elev_pad[i+1][j+1] != NoDataValue){
          slope5 = (elev_pad[i][j] - elev_pad[i+1][j+1]) * one_ov_root_2;
          total += slope5;
		  	}
			  if (elev_pad[i][j] > elev_pad[i+1][j] && elev_pad[i+1][j] != NoDataValue){
          slope6 = elev_pad[i][j] - elev_pad[i+1][j];
          total += slope6;
        }
			  if (elev_pad[i][j] > elev_pad[i+1][j-1] && elev_pad[i+1][j-1] != NoDataValue){
          slope7 = (elev_pad[i][j] - elev_pad[i+1][j-1]) * one_ov_root_2;
          total += slope7;
			  }
			  if (elev_pad[i][j] > elev_pad[i][j-1] && elev_pad[i][j-1] != NoDataValue){
          slope8 = elev_pad[i][j] - elev_pad[i][j-1];
          total += slope8;
        }

			//if no slope is found (total == 0), pass flow to all equal elevation cells]
      if (total == 0) {
			  if(elev_pad[i-1][j-1] == elev_pad[i][j]){
          slope1 = 1;
          total += 1;
        }
				if(elev_pad[i-1][j] == elev_pad[i][j]){
				  slope2 = 1;
          total += 1;
        }
				if(elev_pad[i-1][j+1] == elev_pad[i][j]){
				  slope3 = 1;
          total += 1;
        }
				if(elev_pad[i][j+1] == elev_pad[i][j]){
				  slope4 = 1;
          total += 1;
        }
			  if(elev_pad[i+1][j+1] == elev_pad[i][j]){
				  slope5 = 1;
          total += 1;
        }
				if(elev_pad[i+1][j] == elev_pad[i][j]){
				  slope6 = 1;
          total += 1;
        }
				if(elev_pad[i+1][j-1] == elev_pad[i][j]){
          slope7 = 1;
          total += 1;
        }
				if(elev_pad[i][j-1] == elev_pad[i][j]){
          slope8 = 1;
          total += 1;
        }
			}

      //divide slope by total to get the proportion of flow directed to each cell
      //and increment the downslope cells. If no downslope flow to a node, 0 is
      //added, so no change is seen.
			area[i-1][j-1] += area[i][j] * (slope1/total);
			area[i-1][j] += area[i][j] * (slope2/total);
			area[i-1][j+1] += area[i][j] * (slope3/total);
			area[i][j+1] += area[i][j] * (slope4/total);
			area[i+1][j+1] += area[i][j] * (slope5/total);
			area[i+1][j] += area[i][j] * (slope6/total);
			area[i+1][j-1] += area[i][j] * (slope7/total);
			area[i][j-1] += area[i][j] * (slope8/total);
      }
    }
  }

  //translate the data back
  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
        area_final[i][j] = area[i+1][j+1];
    }
  }

  //write output LSDRaster object
  LSDRaster MultiFlow(NRows, NCols, XMinimum, YMinimum, DataResolution, NoDataValue, area_final);
  return MultiFlow;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Function built around original c++ code by Martin Hurst to generate a flowarea
// raster.
//
// Computes the proportion of all downslope flows for each cell in the input
// DEM, and weights them using the equation from Freeman et al 1991 and routes the
// flow accordingly.
//
// Paper link: http://www.sciencedirect.com/science/article/pii/009830049190048I
//
// Cardinal Weighting = (elevation_drop/total_elevation_drop)^1.1
// Diagonal Weighting = ((elevation_drop/total_elevation_drop)*(1/root(2)))^1.1
//
// Can *NOT* handle DEMs containing flats or pits -  must be filled using the new
// LSDRaster fill.
//
// Outputs an LSDRaster
//
// SWDG, 18/4/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::FreemanMDFlow(){

  //create output array, populated with nodata
  Array2D<double> area(NRows, NCols, NoDataValue);

  //declare variables
  vector<double> flat;
  vector<double> sorted;
  vector<size_t> index_map;
  double one_ov_root_2 = 0.707106781187;
  double p = 1.1; //value avoids preferential flow to diagonals

  //loop through the dem cells creating a row major 1D vector, flat, and
  //setting the cell area to every npn ndv cell
  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      flat.push_back(RasterData[i][j]);
      if (RasterData[i][j] != NoDataValue){
        area[i][j] = DataResolution*DataResolution;
      }
    }
  }

  //sort the 1D elevation vector and produce an index
  matlab_double_sort_descending(flat, sorted, index_map);

  for(int q = 0 ;q < int(flat.size()); ++q){

    if (sorted[q] != NoDataValue){

		  //use row major ordering to reconstruct each cell's i,j coordinates
  	  int i = index_map[q] / NCols;
   	  int j = index_map[q] % NCols;

      //skip edge cells
      if (i != 0 && j != 0 && i != NRows-1 && j != NCols-1){

        //reset variables on each loop
			  double total = 0;
			  double slope1 = 0;
        double slope2 = 0;
        double slope3 = 0;
        double slope4 = 0;
        double slope5 = 0;
        double slope6 = 0;
        double slope7 = 0;
        double slope8 = 0;

        //Get sum of magnitude of downslope flow, total, and store the magnitude of
        //each of the 8 downslope cells as slope1->8 *Avoids NDVs*
			  if (RasterData[i][j] > RasterData[i-1][j-1] && RasterData[i-1][j-1] != NoDataValue){
          slope1 = pow(((RasterData[i][j] - RasterData[i-1][j-1]) * one_ov_root_2),p);
          total += slope1;
        }
			  if (RasterData[i][j] > RasterData[i-1][j] && RasterData[i-1][j] != NoDataValue){
          slope2 = pow((RasterData[i][j] - RasterData[i-1][j]),p);
          total += slope2;
			  }
		  	if (RasterData[i][j] > RasterData[i-1][j+1] && RasterData[i-1][j+1] != NoDataValue){
          slope3 = pow(((RasterData[i][j] - RasterData[i-1][j+1]) * one_ov_root_2),p);
          total += slope3;
		  	}
			  if (RasterData[i][j] > RasterData[i][j+1] && RasterData[i][j+1] != NoDataValue){
          slope4 = pow((RasterData[i][j] - RasterData[i][j+1]),p);
          total += slope4;
        }
			  if (RasterData[i][j] > RasterData[i+1][j+1] && RasterData[i+1][j+1] != NoDataValue){
          slope5 = pow(((RasterData[i][j] - RasterData[i+1][j+1]) * one_ov_root_2),p);
          total += slope5;
		  	}
			  if (RasterData[i][j] > RasterData[i+1][j] && RasterData[i+1][j] != NoDataValue){
          slope6 = pow((RasterData[i][j] - RasterData[i+1][j]),p);
          total += slope6;
        }
			  if (RasterData[i][j] > RasterData[i+1][j-1] && RasterData[i+1][j-1] != NoDataValue){
          slope7 = pow(((RasterData[i][j] - RasterData[i+1][j-1]) * one_ov_root_2),p);
          total += slope7;
			  }
			  if (RasterData[i][j] > RasterData[i][j-1] && RasterData[i][j-1] != NoDataValue){
          slope8 = pow((RasterData[i][j] - RasterData[i][j-1]),p);
          total += slope8;
        }

      //divide slope by total to get the proportion of flow directed to each cell
      //and increment the downslope cells. If no downslope flow to a node, 0 is
      //added, so no change is seen.
			area[i-1][j-1] += area[i][j] * (slope1/total);
			area[i-1][j] += area[i][j] * (slope2/total);
			area[i-1][j+1] += area[i][j] * (slope3/total);
			area[i][j+1] += area[i][j] * (slope4/total);
			area[i+1][j+1] += area[i][j] * (slope5/total);
			area[i+1][j] += area[i][j] * (slope6/total);
			area[i+1][j-1] += area[i][j] * (slope7/total);
			area[i][j-1] += area[i][j] * (slope8/total);
      }
    }
  }
  //write output LSDRaster object
  LSDRaster FreemanMultiFlow(NRows, NCols, XMinimum, YMinimum, DataResolution, NoDataValue, area);
  return FreemanMultiFlow;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Function built around original c++ code by Martin Hurst to generate a flowarea
// raster.
//
// Computes the proportion of all downslope flows for each cell in the input
// DEM, and weights them using the equation from Quinn et al 1991 and routes the
// flow accordingly.
//
// Paper link: http://onlinelibrary.wiley.com/doi/10.1002/hyp.3360050106/abstract
//
// Cardinal Weighting = (elevation_drop/total_elevation_drop)*DataResolution/2
// Diagonal Weighting =
//      ((elevation_drop/total_elevation_drop)*(1/root(2)))* DataResolution*0.354
//
// Can *NOT* handle DEMs containing flats or pits -  must be filled using the new
// LSDRaster fill.
//
// Outputs an LSDRaster
//
// SWDG, 18/4/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::QuinnMDFlow(){

  //create output array, populated with nodata
  Array2D<double> area(NRows, NCols, NoDataValue);

  //declare variables
  vector<double> flat;
  vector<double> sorted;
  vector<size_t> index_map;
  double one_ov_root_2 = 0.707106781187;
  double Lc = DataResolution/2; //cardinal scaling factor
  double Ld = DataResolution * 0.354; //diagonal scaling factor


  //loop through the dem cells creating a row major 1D vector, flat, and
  //setting the cell area to every npn ndv cell
  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      flat.push_back(RasterData[i][j]);
      if (RasterData[i][j] != NoDataValue){
        area[i][j] = DataResolution*DataResolution;
      }
    }
  }

  //sort the 1D elevation vector and produce an index
  matlab_double_sort_descending(flat, sorted, index_map);

  for(int q = 0 ;q < int(flat.size()); ++q){

    if (sorted[q] != NoDataValue){

		  //use row major ordering to reconstruct each cell's i,j coordinates
  	  int i = index_map[q] / NCols;
   	  int j = index_map[q] % NCols;

      //skip edge cells
      if (i != 0 && j != 0 && i != NRows-1 && j != NCols-1){

        //reset variables on each loop
			  double total = 0;
			  double slope1 = 0;
        double slope2 = 0;
        double slope3 = 0;
        double slope4 = 0;
        double slope5 = 0;
        double slope6 = 0;
        double slope7 = 0;
        double slope8 = 0;

        //Get sum of magnitude of downslope flow, total, and store the magnitude of
        //each of the 8 downslope cells as slope1->8 *Avoids NDVs*
			  if (RasterData[i][j] > RasterData[i-1][j-1] && RasterData[i-1][j-1] != NoDataValue){
          slope1 = ((RasterData[i][j] - RasterData[i-1][j-1]) * one_ov_root_2) * Ld;
          total += slope1;
        }
			  if (RasterData[i][j] > RasterData[i-1][j] && RasterData[i-1][j] != NoDataValue){
          slope2 = (RasterData[i][j] - RasterData[i-1][j]) * Lc;
          total += slope2;
			  }
		  	if (RasterData[i][j] > RasterData[i-1][j+1] && RasterData[i-1][j+1] != NoDataValue){
          slope3 = ((RasterData[i][j] - RasterData[i-1][j+1]) * one_ov_root_2) * Ld;
          total += slope3;
		  	}
			  if (RasterData[i][j] > RasterData[i][j+1] && RasterData[i][j+1] != NoDataValue){
          slope4 = (RasterData[i][j] - RasterData[i][j+1]) * Lc;
          total += slope4;
        }
			  if (RasterData[i][j] > RasterData[i+1][j+1] && RasterData[i+1][j+1] != NoDataValue){
          slope5 = ((RasterData[i][j] - RasterData[i+1][j+1]) * one_ov_root_2) * Ld;
          total += slope5;
		  	}
			  if (RasterData[i][j] > RasterData[i+1][j] && RasterData[i+1][j] != NoDataValue){
          slope6 = (RasterData[i][j] - RasterData[i+1][j]) * Lc;
          total += slope6;
        }
			  if (RasterData[i][j] > RasterData[i+1][j-1] && RasterData[i+1][j-1] != NoDataValue){
          slope7 = ((RasterData[i][j] - RasterData[i+1][j-1]) * one_ov_root_2) * Ld;
          total += slope7;
			  }
			  if (RasterData[i][j] > RasterData[i][j-1] && RasterData[i][j-1] != NoDataValue){
          slope8 = (RasterData[i][j] - RasterData[i][j-1]) * Lc;
          total += slope8;
        }

      //divide slope by total to get the proportion of flow directed to each cell
      //and increment the downslope cells. If no downslope flow to a node, 0 is
      //added, so no change is seen.
			area[i-1][j-1] += area[i][j] * (slope1/total);
			area[i-1][j] += area[i][j] * (slope2/total);
			area[i-1][j+1] += area[i][j] * (slope3/total);
			area[i][j+1] += area[i][j] * (slope4/total);
			area[i+1][j+1] += area[i][j] * (slope5/total);
			area[i+1][j] += area[i][j] * (slope6/total);
			area[i+1][j-1] += area[i][j] * (slope7/total);
			area[i][j-1] += area[i][j] * (slope8/total);
      }
    }
  }
  //write output LSDRaster object
  LSDRaster QuinnMultiFlow(NRows, NCols, XMinimum, YMinimum, DataResolution, NoDataValue, area);
  return QuinnMultiFlow;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Function built around original c++ code by Martin Hurst to generate a flowarea
// raster.
//
// Computes the proportion of all downslope flows for each cell in the input
// DEM. Finds the cell of the steepest descent and then checks the two
// neighbouring cells slopes. If either is also downslope proportion flow
// between the steepest cell and the steepest neighbour. If neither neighbour
// is downslope 100% of flow follows the steepest path.
//
// Can *NOT* handle DEMs containing flats or pits -  must be filled using the new
// LSDRaster fill.
//
// Outputs an LSDRaster
//
// SWDG - 02/08/2013
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::M2DFlow(){

  //create output array, populated with nodata
  Array2D<double> area(NRows, NCols, NoDataValue);

  //declare variables
  vector<double> flat;
  vector<double> sorted;
  vector<size_t> index_map;
  double one_ov_root_2 = 0.707106781187;

  //loop through the dem cells creating a row major 1D vector, flat, and
  //setting the cell area to every npn ndv cell
  for (int i = 0; i < NRows; ++i){
    for (int j = 0; j < NCols; ++j){
      flat.push_back(RasterData[i][j]);
      if (RasterData[i][j] != NoDataValue){
        area[i][j] = DataResolution*DataResolution;
      }
    }
  }

  //sort the 1D elevation vector and produce an index
  matlab_double_sort_descending(flat, sorted, index_map);

  for(int q = 0 ;q < int(flat.size()); ++q){

    if (sorted[q] != NoDataValue){

		  //use row major ordering to reconstruct each cell's i,j coordinates
  	  int i = index_map[q] / NCols;
   	  int j = index_map[q] % NCols;

      //skip edge cells
      if (i != 0 && j != 0 && i != NRows-1 && j != NCols-1){

        //reset variables on each loop
			  double slope0 = 0;
        double slope1 = 0;
        double slope2 = 0;
        double slope3 = 0;
        double slope4 = 0;
        double slope5 = 0;
        double slope6 = 0;
        double slope7 = 0;
        vector<double> slopes;

        double p1 = 0;
        double p2 = 0;
        int second_slope = -1; //initialized using value outside of range.

        //Get magnitude of downslope flow slope0->7 *Avoids NDVs*
			  if (RasterData[i][j] > RasterData[i-1][j-1] && RasterData[i-1][j-1] != NoDataValue){
          slope0 = ((RasterData[i][j] - RasterData[i-1][j-1]) * one_ov_root_2);
          slopes.push_back(slope0);
        }
        else {
          slopes.push_back(0);
        }

			  if (RasterData[i][j] > RasterData[i-1][j] && RasterData[i-1][j] != NoDataValue){
          slope1 = (RasterData[i][j] - RasterData[i-1][j]);
          slopes.push_back(slope1);
			  }
        else {
          slopes.push_back(0);
        }

		  	if (RasterData[i][j] > RasterData[i-1][j+1] && RasterData[i-1][j+1] != NoDataValue){
          slope2 = ((RasterData[i][j] - RasterData[i-1][j+1]) * one_ov_root_2);
          slopes.push_back(slope2);
		  	}
        else {
          slopes.push_back(0);
        }

			  if (RasterData[i][j] > RasterData[i][j+1] && RasterData[i][j+1] != NoDataValue){
          slope3 = (RasterData[i][j] - RasterData[i][j+1]);
          slopes.push_back(slope3);
        }
        else {
          slopes.push_back(0);
        }

			  if (RasterData[i][j] > RasterData[i+1][j+1] && RasterData[i+1][j+1] != NoDataValue){
          slope4 = ((RasterData[i][j] - RasterData[i+1][j+1]) * one_ov_root_2);
          slopes.push_back(slope4);
		  	}
        else {
          slopes.push_back(0);
        }

			  if (RasterData[i][j] > RasterData[i+1][j] && RasterData[i+1][j] != NoDataValue){
          slope5 = (RasterData[i][j] - RasterData[i+1][j]);
          slopes.push_back(slope5);
        }
        else {
          slopes.push_back(0);
        }

			  if (RasterData[i][j] > RasterData[i+1][j-1] && RasterData[i+1][j-1] != NoDataValue){
          slope6 = ((RasterData[i][j] - RasterData[i+1][j-1]) * one_ov_root_2);
          slopes.push_back(slope6);
			  }
        else {
          slopes.push_back(0);
        }

			  if (RasterData[i][j] > RasterData[i][j-1] && RasterData[i][j-1] != NoDataValue){
          slope7 = (RasterData[i][j] - RasterData[i][j-1]);
          slopes.push_back(slope7);
        }
        else {
          slopes.push_back(0);
        }

        if (int(slopes.size()) > 0 ){   //catch outlets with no neighbours to drain to

          //find maximum slope & its index location in the slopes vector
          double S_max = *max_element(slopes.begin(), slopes.end());
          int S_max_index = find(slopes.begin(), slopes.end(), S_max) - slopes.begin();

          //find steepest neighbour
          if (S_max_index == 0){
            if (slope7 > 0 && slope1 == 0){
              second_slope = 7;
            }
            if (slope7 == 0 && slope1 > 0){
              second_slope = 1;
            }
            if (slope7 > 0 && slope1 > 0){
              if (slope7 > slope1){
                second_slope = 7;
              }
              else{
                second_slope = 1;
              }
            }
            if (slope7 == slope1){
              second_slope = 0;
            }
          }

          if (S_max_index == 1){
            if (slope0 > 0 && slope2 == 0){
              second_slope = 7;
            }
            if (slope0 == 0 && slope2 > 0){
              second_slope = 1;
            }
            if (slope0 > 0 && slope2 > 0){
              if (slope0 > slope2){
                second_slope = 7;
              }
              else{
                second_slope = 1;
              }
            }
            if (slope0 == slope2){
              second_slope = 0;
            }
          }

          if (S_max_index == 2){
            if (slope1 > 0 && slope3 == 0){
              second_slope = 7;
            }
            if (slope1 == 0 && slope3 > 0){
              second_slope = 1;
            }
            if (slope1 > 0 && slope3 > 0){
              if (slope1 > slope3){
                second_slope = 7;
              }
              else{
                second_slope = 1;
              }
            }
            if (slope1 == slope3){
              second_slope = 0;
            }
          }

          if (S_max_index == 3){
            if (slope2 > 0 && slope4 == 0){
              second_slope = 7;
            }
            if (slope2 == 0 && slope4 > 0){
              second_slope = 1;
            }
            if (slope2 > 0 && slope4 > 0){
              if (slope2 > slope4){
                second_slope = 7;
              }
              else{
                second_slope = 1;
              }
            }
            if (slope2 == slope4){
              second_slope = 0;
            }
          }

          if (S_max_index == 4){
            if (slope3 > 0 && slope5 == 0){
              second_slope = 7;
            }
            if (slope3 == 0 && slope5 > 0){
              second_slope = 1;
            }
            if (slope3 > 0 && slope5 > 0){
              if (slope3 > slope5){
                second_slope = 7;
              }
              else{
                second_slope = 1;
              }
            }
            if (slope3 == slope5){
              second_slope = 0;
            }
          }

          if (S_max_index == 5){
            if (slope4 > 0 && slope6 == 0){
              second_slope = 7;
            }
            if (slope4 == 0 && slope6 > 0){
              second_slope = 1;
            }
            if (slope4 > 0 && slope6 > 0){
              if (slope4 > slope6){
                second_slope = 7;
              }
              else{
                second_slope = 1;
              }
            }
            if (slope4 == slope6){
              second_slope = 0;
            }
          }


          if (S_max_index == 6){
            if (slope5 > 0 && slope7 == 0){
              second_slope = 7;
            }
            if (slope5 == 0 && slope7 > 0){
              second_slope = 1;
            }
            if (slope5 > 0 && slope7 > 0){
              if (slope5 > slope7){
                second_slope = 7;
              }
              else{
                second_slope = 1;
              }
            }
            if (slope5 == slope7){
              second_slope = 0;
            }
          }

          if (S_max_index == 7){
            if (slope6 > 0 && slope0 == 0){
              second_slope = 7;
            }
            if (slope6 == 0 && slope0 > 0){
              second_slope = 1;
            }
            if (slope6 > 0 && slope0 > 0){
              if (slope6 > slope0){
                second_slope = 7;
              }
              else{
                second_slope = 1;
              }
            }
            if (slope6 == slope0){
              second_slope = 0;
            }
          }

          //get proportions p1 and p2
          if (second_slope != S_max_index){
            p1 = S_max/(S_max + slopes[second_slope]);
            p2 = slopes[second_slope]/(S_max + slopes[second_slope]);
          }
          else{ //flow only in 1 direction
            p1 = 1;
            p2 = 0;
          }

          //partition flow following the steepest slope and it's steepest neighbour
          if (S_max_index == 0 && area[i-1][j-1] != NoDataValue){
            area[i-1][j-1] += area[i][j] * p1;
            if (second_slope == 1){
              area[i-1][j] += area[i][j] * p2;
            }
            if (second_slope == 7){
              area[i][j-1] += area[i][j] * p2;
            }
          }

          if (S_max_index == 1 && area[i-1][j] != NoDataValue){
            area[i-1][j] += area[i][j] * p1;
            if (second_slope == 2){
              area[i-1][j+1] += area[i][j] * p2;
            }
            if (second_slope == 0){
              area[i-1][j-1] += area[i][j] * p2;
            }
          }

          if (S_max_index == 2 && area[i-1][j+1] != NoDataValue){
            area[i-1][j+1] += area[i][j] * p1;
            if (second_slope == 3){
              area[i][j+1] += area[i][j] * p2;
            }
            if (second_slope == 1){
              area[i-1][j] += area[i][j] * p2;
            }
          }

          if (S_max_index == 3 && area[i][j+1] != NoDataValue){
            area[i][j+1] += area[i][j] * p1;
            if (second_slope == 4){
              area[i+1][j+1] += area[i][j] * p2;
            }
            if (second_slope == 2){
              area[i-1][j+1] += area[i][j] * p2;
            }
          }

          if (S_max_index == 4 && area[i+1][j+1] != NoDataValue){
            area[i+1][j+1] += area[i][j] * p1;
            if (second_slope == 5){
              area[i+1][j] += area[i][j] * p2;
            }
            if (second_slope == 3){
              area[i][j+1] += area[i][j] * p2;
            }
          }

          if (S_max_index == 5 && area[i+1][j] != NoDataValue){
            area[i+1][j] += area[i][j] * p1;
            if (second_slope == 6){
              area[i+1][j-1] += area[i][j] * p2;
            }
            if (second_slope == 4){
              area[i+1][j+1] += area[i][j] * p2;
            }
          }

          if (S_max_index == 6 && area[i+1][j-1] != NoDataValue){
            area[i+1][j-1] += area[i][j] * p1;
            if (second_slope == 7){
              area[i][j-1] += area[i][j] * p2;
            }
            if (second_slope == 5){
              area[i+1][j] += area[i][j] * p2;
            }
          }

          if (S_max_index == 7 && area[i][j-1] != NoDataValue){
            area[i][j-1] += area[i][j] * p1;
            if (second_slope == 0){
              area[i-1][j] += area[i][j] * p2;
            }
            if (second_slope == 6){
              area[i-1][j-1] += area[i][j] * p2;
            }
          }
        }
      }
    }
  }

  //write output LSDRaster object
  LSDRaster Multi2Flow(NRows, NCols, XMinimum, YMinimum, DataResolution, NoDataValue, area);
  return Multi2Flow;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// PREDICTING CHANNEL HEADS USING TANGENTIAL CURVATURE
//
// This function is used to predict channel head locations based on the method proposed by
// Pelletier (2013).  It creates a contour curvature map and identifies channel heads as pixels greater
// than a user defined contour curvature threshold value, set by default at 0.1.  The threshold curvature
// can also be defined as a multiple of the standard deviation of the curvature.  Before this function is called
// the DEM must be filtered using the wiener filter in the LSDRasterSpectral object in order to remove high frequency
// noise.
//
// Reference: Pelletier (2013) A robust, two-parameter method for the extraction of drainage
// networks from high-resolution digital elevation models (DEMs): Evaluation using synthetic and real-world
// DEMs, Water Resources Research 49: 1-15
//
// added by FC 16/07/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

LSDRaster LSDRaster::calculate_pelletier_channel_heads(int NRows,int NCols,int XMinimum,int YMinimum,double DataResolution,
                                                          int NoDataValue,double window_radius,
                                                          double tan_curv_threshold,Array2D<double>& tan_curv_array)
{
	Array2D<double> chan_head_locations(NRows,NCols,NoDataValue);
	double total_curv = 0;
	int n_observations = 0;

  //get the mean of the tangential curvature
  for (int row = 0; row < NRows; row++)
	{
    for(int col = 0; col < NCols; col++)
    {
      if (tan_curv_array[row][col] != NoDataValue)
      {
        total_curv = total_curv + tan_curv_array[row][col];
        ++n_observations;
      }
    }
  }

  double mean_curv = total_curv/n_observations;
  double total_st_dev = 0;

  // get the standard deviation of the curvature and use 3*st dev as the threshold value
  for (int row = 0; row < NRows; row++)
	{
    for(int col = 0; col < NCols; col++)
    {
      if (tan_curv_array[row][col] != NoDataValue)
      {
        total_st_dev = ((tan_curv_array[row][col] - mean_curv)*(tan_curv_array[row][col] - mean_curv)) + total_st_dev;
      }
    }
  }

  //double st_dev = sqrt(total_st_dev/n_observations);
 // double tan_curv_threshold = 3*st_dev;

  // Get all the locations where the tan curvature is greater than the user defined threshold
  for (int row = 0; row < NRows; row++)
	{
    for(int col = 0; col < NCols; col++)
    {
      if (tan_curv_array[row][col] > tan_curv_threshold)
      {
        chan_head_locations[row][col] = tan_curv_array[row][col];
      }
      else
      {
        chan_head_locations[row][col] = 0;
      }
    }
  }

  // Write raster of predicted channel head locations
  LSDRaster channel_heads(NRows,NCols,XMinimum,YMinimum,DataResolution,
	                           NoDataValue,chan_head_locations);
  return channel_heads;

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Calculate the minimum bounding rectangle for an LSDRaster Object and crop out
// all the surrounding NoDataValues to reduce the size and load times of output
// rasters.
//
// Ideal for use with chi analysis tools which output basin and chi m value rasters
// which can be predominantly no data. As an example, a 253 Mb file can be reduced to
// ~5 Mb with no loss or resampling of data.
//
// Returns A trimmed LSDRaster object.
//
// SWDG 22/08/13
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::RasterTrimmer(){

  //minimum index value in a column
  int a = 0;
  int min_col = 100000; //a big number

  for (int row = 0; row < NRows; ++row){
    a = 0;
    while (RasterData[row][a] == NoDataValue && a < NCols-1){
      ++a;
    }
    if (min_col > a){
      min_col = a;
    }
  }

  //maximum index value in a column
  a = NCols - 1;
  int max_col = 0; //a small number

  for (int row = 0; row < NRows; ++row){
    a = NCols - 1;
    while (RasterData[row][a] == NoDataValue && a > 0){
      --a;
    }
    if (max_col < a){
      max_col = a;
    }
  }


  //minimum index value in a row
  a = 0;
  int min_row = 100000; //a big number

  for (int col = 0; col < NCols; ++col){
    a = 0;
    while (RasterData[a][col] == NoDataValue && a < NRows - 1){
      ++a;
    }
    if (min_row > a){
      min_row = a;
    }
  }

  //maximum index value in a row
  a = NRows - 1;
  int max_row = 0; //a small number

  for (int col = 0; col < NCols; ++col){
    a = NRows - 1;
    while (RasterData[a][col] == NoDataValue && a > 0){
      --a;
    }
    if (max_row < a){
      max_row = a;
    }
  }

  // create new row and col sizes taking account of zero indexing
  int new_row_dimension = (max_row-min_row) + 1;
  int new_col_dimension = (max_col-min_col) + 1;

  Array2D<double>TrimmedData(new_row_dimension, new_col_dimension, NoDataValue);

  //loop over min bounding rectangle and store it in new array of shape new_row_dimension x new_col_dimension
  int TrimmedRow = 0;
  int TrimmedCol = 0;
  for (int row = min_row - 1; row < max_row; ++row){
    for(int col = min_col - 1; col < max_col; ++col){
      TrimmedData[TrimmedRow][TrimmedCol] = RasterData[row][col];
      ++TrimmedCol;
    }
    ++TrimmedRow;
    TrimmedCol = 0;
  }

  //calculate lower left corner coordinates of new array
  double new_XLL = ((min_col - 1) * DataResolution) + XMinimum;
  double new_YLL = YMinimum + ((NRows - (max_row + 0)) * DataResolution);

  LSDRaster TrimmedRaster(new_row_dimension, new_col_dimension, new_XLL,
                          new_YLL, DataResolution, NoDataValue, TrimmedData);

  return TrimmedRaster;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//		 sSSSs MM   MM  oOOo   oOOo  TTTTTT HH  HH IIII NN   NN  gGGGG
//		SS     M M M M oO  Oo oO  Oo   TT   HH  HH  II  NNN  NN GG
//		 sSSs  M  M  M OO  OO OO  OO   TT   HHHHHH  II  NN N NN GG GGG
//		    SS M     M oO  Oo oO  Oo   TT   HH  HH  II  NN  NNN GG  GG
//		sSSSs  M     M  oOOo   oOOo    TT   HH  HH IIII NN   NN  GGGGG
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Perform Non-local means filtering on a DEM following Baude et al. [2005]
//	Smoothes non-gaussian noise.
//
//	Inputs required:
//		the search window radius,
//		the similarity window radius and
//		the degree of filtering
//
//	Martin Hurst, February, 2012
//	Modified by David Milodowski, May 2012- generates grid of recording filtered noise
//
//	WindowRadius has to be <= SimilarityRadius ?
//
//	Adapted from a matlab script by:
//	Author: Jose Vicente Manjon Herrera & Antoni Buades
//	Date: 09-03-2006
//
//	Implementation of the Non local filter proposed for A. Buades, B. Coll and J.M. Morel in
//	"A non-local algorithm for image denoising"
//
//	Added soft threshold optimal correction - David Milodowski, 05/2012
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Martin Hurst, February, 2012
//	Modified by David Milodowski, May 2012- generates grid of recording filtered noise
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LSDRaster LSDRaster::NonLocalMeansFilter(int WindowRadius, int SimilarityRadius, int DegreeFiltering, double Sigma)
{



	//Declare Arrays to hold Filtered Data and Noise
	Array2D<double> FilteredRasterData(NRows,NCols);
	//Array2D<double> FilteredNoise(NRows,NCols);

	//Declare Array to hold a padded copy of the raster with padding values taken
	//as reflected values from the edge of RasterData
	Array2D<double> PaddedRasterData(NRows+2*SimilarityRadius, NCols+2*SimilarityRadius,0.0);
	PadRasterSymmetric(PaddedRasterData, SimilarityRadius);

	//initiate the local gaussian kernel and populate
	int KernelDimension = 2*SimilarityRadius+1;
	Array2D<double> Kernel(KernelDimension,KernelDimension,0.0);
  	MakeGaussianKernel(Kernel, Sigma, SimilarityRadius);

	//initiate temporary arrays
	Array2D<double> W1(KernelDimension,KernelDimension);
	Array2D<double> W2(KernelDimension,KernelDimension);

	//initiate temp variables
	float w, wmax, average, sweight, d;

	//loop through DEM
	int i1, j1, rowmin, rowmax, colmin, colmax;

  	for (int i=0; i<NRows; ++i)
	{
		i1 = i+SimilarityRadius;
		for (int j=0; j<NCols; ++j)
		{
			j1 = j+SimilarityRadius;
      		//Get DEM sample  with size SimilarityRadius, centred on cell of interest
      		for (int a=0; a<(KernelDimension); ++a)
			{
  		  		for (int b=0; b<(KernelDimension); ++b)
				{
          			W1[a][b] = PaddedRasterData[i1-SimilarityRadius+a][j1-SimilarityRadius+b];
				}
      		}

      		wmax=0;
      		average=0;
  			sweight=0;

			//get bounding conditions
			rowmin = max(i1-WindowRadius,SimilarityRadius);
			rowmax = min(i1+WindowRadius,NRows+SimilarityRadius-1);
			colmin = max(j1-WindowRadius,SimilarityRadius);
			colmax = min(j1+WindowRadius,NCols+SimilarityRadius-1);

			//loop to calculate weigths for each cell
      		for (int row=rowmin; row<rowmax+1; ++row)
			{
			     for (int col=colmin; col<colmax+1; ++col)
				{
			          d=0;

					//If centre cell do nothing
			          if (row!=i1 || col!=j1)
					//Otherwise do the calculations
			          {
						//Extract DEM centred around each point in kernel
						for (int a=0; a<(KernelDimension); ++a)
						{
							for (int b=0; b<(KernelDimension); ++b)
							{
								W2[a][b] = PaddedRasterData[row+a-SimilarityRadius][col+b-SimilarityRadius];
								d += Kernel[a][b]*(W1[a][b]-W2[a][b])*(W1[a][b]-W2[a][b]);
							}
						}

						w = exp(-d/(DegreeFiltering*DegreeFiltering));
            				if (w>wmax) wmax=w;
           				sweight += w;
            				average += w*PaddedRasterData[row][col];
          			}
				}
      		}

			average += wmax*PaddedRasterData[i1][j1];
			sweight += wmax;

      		if (sweight > 0) FilteredRasterData[i][j] = average/sweight;
			else FilteredRasterData[i][j] = RasterData[i][j];

      		// Also extract a record of the noise
      		//FilteredNoise[i][j]=RasterData[i][j]-FilteredRasterData[i][j];
		}
	}

	LSDRaster NLFilteredDEM(NRows,NCols,XMinimum,YMinimum,DataResolution,NoDataValue,FilteredRasterData);
	//LSDRaster NLFilteredNoise(NRows,NCols,XMinimum,YMinimum,DataResolution,NoDataValue,FilteredNoise);
	return NLFilteredDEM; //, NLFilteredNoise;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Creates a buffer around an array (of size SimilarityRadius) and gives the new border
//	mirror symmetric values of the original array reflected across the boundary.
//	SimilarityRadius should be the size of the window if filtering
//
//	New array has size nrows + 2*SimilarityRadius by ncols + 2*SimilarityRadius
//
//	Martin Hurst, Feb 2012
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::PadRasterSymmetric(Array2D<double>& PaddedRasterData, int& SimilarityRadius)
{


	int PaddedNRows = NRows + 2*SimilarityRadius;
	int PaddedNCols = NCols + 2*SimilarityRadius;

	int minus_i;
	int minus_j;

	for (int i=0; i<PaddedNRows; ++i)
	{
		for (int j=0; j<PaddedNCols; ++j)
		{
			//reverse of i and j
			minus_i = PaddedNRows-1-i;
			minus_j = PaddedNCols-1-j;

			//north boundary
			if (i<SimilarityRadius) {
				if (j<SimilarityRadius) {
					PaddedRasterData[i][j] = RasterData[SimilarityRadius-i][SimilarityRadius-j];
				}
				else if (j>(NCols-1+SimilarityRadius)) {
					PaddedRasterData[i][j] = RasterData[SimilarityRadius-i][j-SimilarityRadius-2*(SimilarityRadius-minus_j)];

				}
				else {
					PaddedRasterData[i][j] = RasterData[SimilarityRadius-i][j-SimilarityRadius];
				}
			}
			//south boundary
			else if (i>NRows-1+SimilarityRadius) {
				if (j<SimilarityRadius) {
					PaddedRasterData[i][j] = RasterData[i-SimilarityRadius-2*(SimilarityRadius-minus_i)][SimilarityRadius-j];
				}
				else if (j>NCols+SimilarityRadius) {
					PaddedRasterData[i][j] = RasterData[i-SimilarityRadius-2*(SimilarityRadius-minus_i)][j-SimilarityRadius-2*(SimilarityRadius-minus_j)];
				}
				else {
					PaddedRasterData[i][j] = RasterData[i-SimilarityRadius-2*(SimilarityRadius-minus_i)][j-SimilarityRadius];
				}
			}
			//west boundary
			else if (j<SimilarityRadius) {
				PaddedRasterData[i][j] = RasterData[i-SimilarityRadius][SimilarityRadius-j];
			}
			//east boundary
			else if (j>NCols-1+SimilarityRadius) {
				PaddedRasterData[i][j] = RasterData[i-SimilarityRadius][j-SimilarityRadius-2*(SimilarityRadius-minus_j)];
			}
			//copy rest of RasterData
			else {
				PaddedRasterData[i][j] = RasterData[i-SimilarityRadius][j-SimilarityRadius];
			}
		}
	}

}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Generate gaussian weighted kernel
//	kernel array must be predeclared of size SimilarityRadius and consist of zeros:
//	Array2D<double> Kernel(SimilarityRadius,SimilarityRadius,0.0);
//
//	Kernel generated using:
//	G(x,y) = (1/2*pi*sigma^2) exp ((-x^2+y^2)/(2*sigma^2))
//
//	Martin Hurst, Feb 2012
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void LSDRaster::MakeGaussianKernel(Array2D<double>& Kernel, double sigma, int SimilarityRadius)
{


	double pi = 3.1415926536;
	double left_side = 1/(2*pi*sigma*sigma);
	double twosigma2 = 2.0*sigma*sigma;
	double right_side;
	double wgt = 0;
	double value;

	//calculate kernel values
	for (int i=0;i<2*SimilarityRadius+1;++i)
	{
		for (int j=0;j<2*SimilarityRadius+1;++j)
		{
			right_side = -(((j-SimilarityRadius)*(j-SimilarityRadius) + (i-SimilarityRadius)*(i-SimilarityRadius))/twosigma2);
			right_side = exp(right_side);
			value = left_side*right_side;
			Kernel[i][j] = value;
			wgt += value;
		}
	}

	//scale to sum to 1
	for (int i=0;i<2*SimilarityRadius+1;++i)
	{
		for (int j=0;j<2*SimilarityRadius+1;++j)
		{
			Kernel[i][j] = Kernel[i][j]/wgt;
		}
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


#endif
