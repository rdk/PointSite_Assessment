#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
using namespace std;


//-------- utility ------//
void getBaseName(string &in,string &out,char slash,char dot)
{
	int i,j;
	int len=(int)in.length();
	for(i=len-1;i>=0;i--)
	{
		if(in[i]==slash)break;
	}
	i++;
	for(j=len-1;j>=0;j--)
	{
		if(in[j]==dot)break;
	}
	if(j==-1)j=len;
	out=in.substr(i,j-i);
}
void getRootName(string &in,string &out,char slash)
{
	int i;
	int len=(int)in.length();
	for(i=len-1;i>=0;i--)
	{
		if(in[i]==slash)break;
	}
	if(i<=0)out=".";
	else out=in.substr(0,i);
}

//--------- parse string --------------//
int Parse_Str(string &in,vector <string> &out)
{
	istringstream www(in);
	out.clear();
	int count=0;
	for(;;)
	{
		string buf;
		if(! (www>>buf) )break;
		out.push_back(buf);
		count++;
	}
	return count;
}


//------ get distance -----//
double Distance2(vector <double> &in1,vector <double> &in2)
{
	double dist2=0;
	for(int i=0;i<3;i++)dist2+=(in1[i]-in2[i])*(in1[i]-in2[i]);
	return dist2;
}
double Distance(vector <double> &in1,vector <double> &in2)
{
	double dist=0;
	for(int i=0;i<3;i++)dist+=(in1[i]-in2[i])*(in1[i]-in2[i]);
	return sqrt(1.0*dist);
}

//--------- CUBE file format -------------//
//-> definition
/*
https://h5cube-spec.readthedocs.io/en/latest/cubeformat.html

Field Layout
{COMMENT1 (str)}
{COMMENT2 (str)}
{NATOMS (int)} {ORIGIN (3x float)} {NVAL (int)}
{XAXIS (int) (3x float)}
{YAXIS (int) (3x float)}
{ZAXIS (int) (3x float)}
{GEOM (int) (float) (3x float)}
      .
      .
{DSET_IDS (#x int)}
      .
      .
{DATA (#x scinot)}
      .
      .

Field Descriptions
{COMMENT1 (str)} and {COMMENT2 (str)}

Two lines of text at the head of the file. Per VMD [UIUC16], by convention {COMMENT1} is typically the title of the system and {COMMENT2} is a description of the property/content stored in the file, but they MAY be anything. For robustness, both of these fields SHOULD NOT be zero-length. As well, while there is no defined maximum length for either of these fields, both SHOULD NOT exceed 80 characters in length.
{NATOMS (int)}

This first field on the third line indicates the number of atoms present in the system. A negative value here indicates the CUBE file MUST contain the {DSET_IDS} line(s); a positive value indicates the file MUST NOT contain this/these lines.

The absolute value of {NATOMS} defines the number of rows of molecular geometry data that MUST be present in {GEOM}.

The CUBE specification is silent as to whether a zero value is permitted for {NATOMS}; most applications likely do not support CUBE files with no atoms.

{ORIGIN (3x float)}

This set of three fields defines the displacement vector from the geometric origin of the system (0,0,0) to the reference point (x0,y0,z0) for the spanning vectors defined in {XAXIS}, {YAXIS}, and {ZAXIS}.
{NVAL (int)}

If {NATOMS} is positive, this field indicates how many data values are recorded at each point in the voxel grid; it MAY be omitted, in which case a value of one is assumed.

If {NATOMS} is negative, this field MUST be either absent or have a value of one.

{XAXIS (int) (3x float)}

The first field on this line is an integer indicating the number of voxels NX present along the X-axis of the volumetric region represented by the CUBE file. This value SHOULD always be positive; whereas the input to the cubegen [Gau16] utility allows a negative value here as a flag for the units of the axis dimensions, in a CUBE file distance units MUST always be in Bohrs, and thus the ‘units flag’ function of a negative sign is superfluous. It is prudent to design applications to handle gracefully a negative value here, however.

The second through fourth values on this line are the components of the vector X?  defining the voxel X-axis. They SHOULD all be non-negative; proper loading/interpretation/calculation behavior is not guaranteed if negative values are supplied. As noted in the Gaussian documentation [Gau16], the voxel axes need neither be orthogonal nor aligned with the geometry axes. However, many tools only support voxel axes that are aligned with the geometry axes (and thus are also orthogonal). In this case, the first float value (Xx) will be positive and the other two (Xy and Xz) will be identically zero.

{YAXIS (int) (3x float)}

This line defines the Y-axis of the volumetric region of the CUBE file, in nearly identical fashion as for {XAXIS}. The key differences are: (1) the first integer field NY MUST always be positive; and (2) in the situation where the voxel axes aligned with the geometry axes, the second float field (Yy) will be positive and the first and third float fields (Yx and Yz) will be identically zero.
{ZAXIS (int) (3x float)}

This line defines the Z-axis of the volumetric region of the CUBE file, in nearly identical fashion as for {YAXIS}. The key difference is that in the situation where the voxel axes are aligned with the geometry axes, the third float field (Zz) will be positive and the first and second float fields (Zx and Zy) will be identically zero.
{GEOM (int) (float) (3x float)}

This field MUST have multiple rows, equal to the absolute value of {NATOMS}

Each row of this field provides atom identity and position information for an atom in the molecular system of the CUBE file:

(int) - Atomic number of atom a
(float) - Nuclear charge of atom a (will deviate from the atomic number when an ECP is used)
(3x float) - Position of the atom in the geometric frame of reference (xa,ya,za)


{DATA (#x scinot)}

This field encompasses the remainder of the CUBE file. Typical formatted CUBE output has up to six values on each line, in whitespace-separated scientific notation.

If {NATOMS} is positive, a total of NXNYNZ? {NVAL} values should be present, flattened as follows (in the below Python pseudocode the for-loop variables are iterated starting from zero):

for i in range(NX):
    for j in range(NY):
        for k in range(NZ):
            for l in range({NVAL}):

                write(data_array[i, j, k, l])
                if (k*{NVAL} + l) mod 6 == 5:
                    write('\n')

        write('\n')

*/

//-> example (from DeepSite WebServer)
/*
CUBE FILE
OUTER LOOP: X, MIDDLE LOOP: Y, INNER LOOP: Z
    1  -106.013625   -80.893499   162.988866
   38     3.779452     0.000000     0.000000
   26     0.000000     3.779452     0.000000
   34     0.000000     0.000000     3.779452
    1     0.000000  -106.013625   -80.893499   162.988866
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38   1.4997e-38
   ....
*/

void Cube_File_Read(string &fn,
	vector <vector <double> > &cube_occupy, 
	vector <double> &cube_value,
	double thres)
{
	//init
	int nx,ny,nz;
	double cx,cy,cz;
	double ax,ay,az;
	cube_occupy.clear();
	cube_value.clear();
	//read
	ifstream fin;
	string buf,temp;
	fin.open(fn.c_str(), ios::in);
	if(fin.fail()!=0)
	{
		fprintf(stderr,"file %s not found!!\n",fn.c_str());
		exit(-1);
	}
	//ommit first two lines
	for(int i=0;i<2;i++)
	{
		if(!getline(fin,buf,'\n'))
		{
			fprintf(stderr,"file %s format bad!!\n",fn.c_str());
			exit(-1);
		}
	}
	//get center coordinate
	for(int i=0;i<1;i++)
	{
		if(!getline(fin,buf,'\n'))
		{
			fprintf(stderr,"file %s format bad!!\n",fn.c_str());
			exit(-1);
		}
		vector <string> tmprec;
		int retv=Parse_Str(buf,tmprec);
		if(retv!=4)
		{
			fprintf(stderr,"file %s format bad at %s!!\n",fn.c_str(),buf.c_str());
			exit(-1);
		}
		cx=atof(tmprec[1].c_str());
		cy=atof(tmprec[2].c_str());
		cz=atof(tmprec[3].c_str());
	}
	//get XYZ cute number
	for(int i=0;i<3;i++)
	{
		if(!getline(fin,buf,'\n'))
		{
			fprintf(stderr,"file %s format bad!!\n",fn.c_str());
			exit(-1);
		}
		vector <string> tmprec;
		int retv=Parse_Str(buf,tmprec);
		if(retv!=4)
		{
			fprintf(stderr,"file %s format bad at %s!!\n",fn.c_str(),buf.c_str());
			exit(-1);
		}
		istringstream www(buf);
		if(i==0)www>>nx>>ax>>temp>>temp;
		if(i==1)www>>ny>>temp>>ay>>temp;
		if(i==2)www>>nz>>temp>>temp>>az;
	}
	//skip one line
	for(int i=0;i<1;i++)
	{
		if(!getline(fin,buf,'\n'))
		{
			fprintf(stderr,"file %s format bad!!\n",fn.c_str());
			exit(-1);
		}
	}
	//------------ load data --------------//
	for(int i=0;i<nx;i++)
		for(int j=0;j<ny;j++)
			for(int k=0;k<nz;k++)
			{
				//read data
				double value;
				if(! (fin>>value) )
				{
					fprintf(stderr,"file %s format bad in data part!!\n",fn.c_str());
					exit(-1);
				}
				//check data to record
				if(value>thres)
				{
					//grid center
					double x=0.5*(i+0.0)*ax + 2.0*cx/ax;
					double y=0.5*(j+0.0)*ay + 2.0*cy/ay;
					double z=0.5*(k+0.0)*az + 2.0*cz/az;
					vector <double> tmp_xyz(3,0);
					//xyz coordinate
					tmp_xyz[0]=x;
					tmp_xyz[1]=y;
					tmp_xyz[2]=z;
					//push_back
					cube_occupy.push_back(tmp_xyz);
					cube_value.push_back(value);
				}
			}
}

//------- load surf_file ----------//
//-> example
/*
1  -26.715   25.640    3.466
2  -26.715   25.640    5.466
*/
int Surf_File_Read(string &fn,
	vector <vector <vector <double> > > &xyz, vector <string> &resi)
{
	ifstream fin;
	string buf,temp;
	//read
	fin.open(fn.c_str(), ios::in);
	if(fin.fail()!=0)
	{
		fprintf(stderr,"list %s not found!!\n",fn.c_str());
		exit(-1);
	}
	resi.clear();
	xyz.clear();
	vector <double> point(3);
	vector <vector <double> > xyz_tmp;
	string prev="";
	string str_rec;
	int first=1;
	int count=0;
	for(;;)
	{
		if(!getline(fin,buf,'\n'))break;
		//-> get input
		vector <string> tmp_rec;
		int retv=Parse_Str(buf,tmp_rec);
		if(retv!=4)
		{
			fprintf(stderr,"xyz %s format bad!!\n",fn.c_str());
			exit(-1);
		}
		temp=tmp_rec[0];
		//record first
		if(first==1)
		{
			first=0;
			prev=temp;
		}
		if(temp!=prev)
		{
			resi.push_back(prev);
			xyz.push_back(xyz_tmp);
			xyz_tmp.clear();
			prev=temp;
			count++;
		}
		point[0]=atof(tmp_rec[1].c_str());
		point[1]=atof(tmp_rec[2].c_str());
		point[2]=atof(tmp_rec[3].c_str());
		xyz_tmp.push_back(point);
	}
	//termi
	if(first==0)
	{
		resi.push_back(prev);
		xyz.push_back(xyz_tmp);
		count++;
	}
	//return
	return count;
}


//-------- assign cube to surf ---------//
//-> note that we only choose the first point in surf as pivot
void Assign_Cube_To_Surf(
	vector <vector <double> > &cube,
	vector <vector <vector <double> > > &surf,
	vector <vector <int> > &surf_index,
	double surf_dist)
{
	//init surf_index
	int number=(int)surf.size();
	surf_index.resize(number);
	for(int j=0;j<number;j++)
	{
		//-> check size
		if(surf[j].size()!=1)
		{
			fprintf(stderr,"surf[%d].size() %d not equal to 1 \n",
				j,surf[j].size());
			exit(-1);
		}
		surf_index[j].push_back(-1);
	}
	//proc
	for(int i=0;i<(int)cube.size();i++)
	{
		//calculate minimal distance to surf points
		double minimal_dist=999999.0*999999.0;
		int minimal_surf=-1;
		for(int j=0;j<number;j++)
		{
			double dist=Distance2(cube[i],surf[j][0]);
			if(dist<minimal_dist)
			{
				minimal_dist=dist;
				minimal_surf=j;
			}
		}
		//add this cube point to surf
		if(minimal_dist<surf_dist*surf_dist)
		{
			surf[minimal_surf].push_back(cube[i]);
			surf_index[minimal_surf].push_back(i);
		}
	}
}

//-------- filter surf points -----------//
//-> we first find the maximal value that resides in the surf point closest to the pivot,
//   then filter out those surf points that are less than X% to this value.
void Filter_Surf_Points(
	vector <double> &cube_value,
	vector <vector <vector <double> > > &surf_in,
	vector <vector <int> > &surf_index,
	vector <vector <vector <double> > > &surf_out,
	double surf_filter)
{
	//init surf_out
	int number=(int)surf_in.size();
	surf_out.resize(number);
	//proc
	for(int i=0;i<number;i++)
	{
		//-> add pivot
		surf_out[i].push_back(surf_in[i][0]);
		//-> find the aximal value that resides in the surf point closest to the pivot
		//calculate minimal distance to surf points
		double minimal_dist=999999.0*999999.0;
		int minimal_surf=-1;
		for(int j=1;j<(int)surf_in[i].size();j++)
		{
			double dist=Distance2(surf_in[i][j],surf_in[i][0]);
			if(dist<minimal_dist)
			{
				minimal_dist=dist;
				minimal_surf=j;
			}
		}
		if(minimal_surf==-1)
		{
			fprintf(stderr,"WARNING!! surf_in[%d] contains no surf points \n",i);
			continue;
		}
		//-> filter out those surf points that are less than X% to this value
		double maximal_value=cube_value[surf_index[i][minimal_surf]];
		for(int j=1;j<(int)surf_in[i].size();j++)
		{
			if(cube_value[surf_index[i][j]]>maximal_value*surf_filter)
				surf_out[i].push_back(surf_in[i][j]);
		}
	}
}




//----------- output XYZ in PDB format --------------//
//-> example
/*
ATOM      1  CA  GLU A   1      19.375   4.600  31.639  1.00 25.28      A    C
ATOM      2  CA  ASN A   2      20.501   7.608  33.670  1.00 10.54      A    C
ATOM      3  CA  ILE A   3      18.924   9.098  36.780  1.00 10.65      A    C
ATOM      5 1H   GLY     1       7.371  -4.742  27.839  1.00  4.58           H

....
*/
void Output_XYZ(vector <vector <vector <double> > > &xyz,FILE *fp)
{
	int atom_num=1;
	for(int i=0;i<(int)xyz.size();i++)
		for(int j=0;j<(int)xyz[i].size();j++)
		{
			fprintf(fp,"ATOM  %5d 1H   ALA Z%4d    %8.3f%8.3f%8.3f  1.00  0.00           H\n",
				atom_num,i+1,xyz[i][j][0],xyz[i][j][1],xyz[i][j][2]);
			atom_num++;
		}
}

//----------- output XYZ in SURF format --------------//
//-> example
/*
1   44.464   35.361   18.387
1   44.354   35.691   17.718
1   45.223   35.773   16.440
1   44.856   34.503   15.810
1   43.740   35.522   18.737
1   44.076   35.501   18.756
1   43.040   35.622   18.898
1   40.297   37.214   16.964
2   51.209   19.860   35.202
2   51.316   20.147   35.378
2   51.637   20.249   35.396
....
*/
void Output_SURF(vector <vector <vector <double> > > &xyz,FILE *fp)
{
	for(int i=0;i<(int)xyz.size();i++)
		for(int j=0;j<(int)xyz[i].size();j++)
		{
			fprintf(fp,"%d %8.3f %8.3f %8.3f\n",
				i+1,xyz[i][j][0],xyz[i][j][1],xyz[i][j][2]);
		}
}


//-------- main ---------//
int main(int argc,char **argv)
{
	//---- Cube_To_Surf ----//
	{
		if(argc<8)
		{
			fprintf(stderr,"Cube_To_Surf <cube_file> <input_surf> <output_surf> <occu_thres> <surf_dist> <spread_thres> <XYZorPDB> \n");
			fprintf(stderr,"[note]: <occu_thres> is the threshold for occupancy in cube_file [0.1], \n");
			fprintf(stderr,"        <surf_dist> is the distance for cube spread around input_surf points [4.5]. \n");
			fprintf(stderr,"        <spread_thres> is the occupancy value for cube spread around input_surf points [0.85]. \n");
			fprintf(stderr,"        <XYZorPDB> to specify the output format to be XYZ [0] or PDB [1]. \n");
			exit(-1);
		}
		string cube_file=argv[1];
		string input_surf=argv[2];
		string output_surf=argv[3];
		double occu_thres=atof(argv[4]);
		double surf_dist=atof(argv[5]);
		double spread_thres=atof(argv[6]);
		int XYZorPDB=atoi(argv[7]);
		//load cube file
		vector <vector <double> > cube_occupy;
		vector <double> cube_value;
		Cube_File_Read(cube_file,cube_occupy,cube_value,occu_thres);
		//load surf file
		vector <vector <vector <double> > > surf;
		vector <string> surf_resi;
		int surf_number=Surf_File_Read(input_surf,surf,surf_resi);
		//assign cube to surf
		vector <vector <int> > surf_index;
		Assign_Cube_To_Surf(cube_occupy,surf,surf_index,surf_dist);
		//filter surf points
		vector <vector <vector <double> > > surf_out;
		Filter_Surf_Points(cube_value,surf,surf_index,surf_out,spread_thres);
		//output SURF file
		FILE *fp=fopen(output_surf.c_str(),"wb");
		if(XYZorPDB==0)  //-> output XYZ format
		{
			Output_SURF(surf_out,fp);
		}
		else             //-> output PDB format
		{
			Output_XYZ(surf_out,fp);
		}
		fclose(fp);
		//exit
		exit(0);
	}
}

