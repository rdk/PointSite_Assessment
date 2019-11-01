#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include "Fast_Sort.h"
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



//----------- vector sort ----------//
//-> UPorDOWN: UP for ascending order sort, DOWN for descending order sort
void Vector_Sort(vector <double> &input, vector <int> &order,int UPorDOWN=0)
{
	order.clear();
	//prepare Fast_Sort
	Fast_Sort <double> fast_sort;
	int size=(int)input.size();
	double *temp_score=new double[size];
	int *temp_index=new int[size];
	for(int i=0;i<size;i++)temp_score[i]=input[i];
	//Fast_Sort
	if(UPorDOWN==1)fast_sort.fast_sort_1up(temp_score,temp_index,size);
	else fast_sort.fast_sort_1(temp_score,temp_index,size);
	//output
	order.resize(size);
	for(int i=0;i<size;i++)order[i]=temp_index[i];
	//delete
	delete [] temp_score;
	delete [] temp_index;
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


//========= XYZ format for point-cloud ==========//
/*
    6 DOg  -22.389   29.406   56.176 0  58
    6 DOh  -23.720   31.160   56.319 0  58
    7 KNa  -22.679   27.076   60.272 0  55
    7 KCb  -21.836   26.578   61.340 0  55
    7 KCc  -20.356   26.635   60.950 0  55
    7 KOd  -19.746   25.631   60.582 0  55
    7 KCe  -22.307   25.189   61.783 0  55
    7 KCf  -23.751   25.237   62.298 0  55
    7 KCg  -24.043   24.207   63.371 0  55
    7 KCh  -25.357   24.538   64.095 0  55
    7 KNi  -25.610   23.600   65.237 0  55
    8 QNa  -19.785   27.829   61.073 0  24
    8 QCb  -18.384   28.110   60.732 0  24
    8 QCc  -17.293   27.383   61.502 0  24
    8 QOd  -16.145   27.388   61.063 0  24
*/
//----- check insert_code ------//
int Check_Ins(string &in)
{
	int i=(int)in.length()-1;
	if(in[i]>='0'&&in[i]<='9')return 0;
	else return 1;
}
//----- check chain_code ------//
int Check_Chain(string &in)
{
	int i=0;
	if(in[i]=='|')return 0;  //-> null chain
	else return 1;
}


//--------- load XYZ with label ----------//
void Load_XYZ_Label(string &fn,vector <vector <vector <double> > > &xyz,
	vector <vector <string> > &str, vector <vector <int> > &label,
	vector <vector <string> > &remain, vector <string> &resi)
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
	xyz.clear();
	str.clear();
	label.clear();
	remain.clear();
	resi.clear();
	vector <double> point(3);
	vector <vector <double> > xyz_tmp;
	vector <string> str_tmp;
	vector <int> lab_tmp;
	vector <string> remain_tmp;
	string prev="";
	string str_rec;
	int first=1;
	for(;;)
	{
		if(!getline(fin,buf,'\n'))break;
		istringstream www(buf);
		www>>temp;
		//check ins_code
		if(Check_Ins(temp)!=1)temp.push_back(' ');
		//check chain_code
		if(Check_Chain(temp)==0)temp=" "+temp;
		//record first
		if(first==1)
		{
			first=0;
			prev=temp;
		}
		if(temp!=prev)
		{
			xyz.push_back(xyz_tmp);
			str.push_back(str_tmp);
			label.push_back(lab_tmp);
			remain.push_back(remain_tmp);
			resi.push_back(prev);
			xyz_tmp.clear();
			str_tmp.clear();
			lab_tmp.clear();
			remain_tmp.clear();
			prev=temp;
		}
		www>>temp;
		str_tmp.push_back(temp);
		www>>point[0]>>point[1]>>point[2];
		xyz_tmp.push_back(point);
		//label and remain
		int remain_lab=0;
		string remain_rec="";
		int count=0;
		for(;;)
		{
			if(! (www>>temp) )break;
			if(count>0)remain_rec=remain_rec+temp+" ";
			else remain_lab=atoi(temp.c_str());
			count++;
		}
		lab_tmp.push_back(remain_lab);
		remain_tmp.push_back(remain_rec);
	}
	//termi
	if(first==0)
	{
		xyz.push_back(xyz_tmp);
		str.push_back(str_tmp);
		label.push_back(lab_tmp);
		remain.push_back(remain_tmp);
		resi.push_back(prev);
	}
}

//----------- load surface points --------//
//-> format
//[note]: we put the predicted pivot center to the 1st line !!
//        also, the ranking order shall be according to the predicted scores.
/*
1   27.380   37.576   48.347
1   27.991   36.587   46.747
1   27.991   38.565   46.747
1   28.980   40.165   49.336
1   28.980   40.165   47.358
1   31.773   35.116   42.723
1   30.504   36.704   43.093
1   29.923   36.968   43.700
1   28.273   37.598   44.720
1   30.610   36.987   44.224
1   26.223   38.693   47.322
1   27.158   37.673   46.165
1   27.295   36.958   44.515
1   26.430   36.322   46.176
2   55.692   37.585   40.618
2   59.405   40.771   42.033
2   58.526   43.058   42.607
2   58.391   39.936   42.111
2   55.621   39.452   43.084
2   55.753   38.769   41.509
...
*/

//--------- load SurfXYZ ----------//
void Load_SurfXYZ(string &fn,vector <vector <vector <double> > > &xyz,
	vector <string> &resi)
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
	xyz.clear();
	resi.clear();
	vector <double> point(3);
	vector <vector <double> > xyz_tmp;
	string prev="";
	string str_rec;
	int first=1;
	for(;;)
	{
		if(!getline(fin,buf,'\n'))break;
		istringstream www(buf);
		www>>temp;
		//record first
		if(first==1)
		{
			first=0;
			prev=temp;
		}
		if(temp!=prev)
		{
			xyz.push_back(xyz_tmp);
			resi.push_back(prev);
			xyz_tmp.clear();
			prev=temp;
		}
		www>>point[0]>>point[1]>>point[2];
		xyz_tmp.push_back(point);
	}
	//termi
	if(first==0)
	{
		xyz.push_back(xyz_tmp);
		resi.push_back(prev);
	}
}


//------------- generate pocket on proteins for each ligand ----------------//
//[note]: for one ligand, the corresponding protein pocket atoms might overlap with those calculate from other ligands !
void Gen_Pocket_on_Protein(double thres,
	vector <vector <vector <double> > > &lig,
	vector <vector <vector <double> > > &prot,
	vector <vector <vector <double> > > &pocketxyz)
{
	//init
	double thres2=thres*thres;
	int atom_num=0;
	int resi_num=0;
	for(int k=0;k<(int)prot.size();k++)
	{
		for(int l=0;l<(int)prot[k].size();l++)atom_num++;
		resi_num++;
	}
	//proc
	pocketxyz.clear();
	pocketxyz.resize((int)lig.size());
	//-> for each ligand
	for(int i=0;i<(int)lig.size();i++)
	{
		vector <int> atom_index(atom_num,0);
		vector <int> resi_index(resi_num,0);
		for(int j=0;j<(int)lig[i].size();j++)
		{
			//-> for each protein atom
			int atom_num=0;
			int resi_num=0;
			for(int k=0;k<(int)prot.size();k++)
			{
				for(int l=0;l<(int)prot[k].size();l++)
				{
					if(atom_index[atom_num]==0)
					{
						double dist2=Distance2(lig[i][j],prot[k][l]);
						if(dist2<thres2)
						{
							pocketxyz[i].push_back(prot[k][l]);
							atom_index[atom_num]=1;
							resi_index[resi_num]=1;
						}
					}
					atom_num++;
				}
				resi_num++;
			}
		}
	}
}



//---------------- pocket-centric measurements --------------//
//-> distance between predict center to any of atom in (or, center of) ligand
int DCA_DCC_Calc(
	vector <vector <vector <double> > > &surfxyz,
	vector <vector <vector <double> > > &ligand, 
	double cutoff,int &dca,int &dcc)
{
	//init
	dca=0;
	dcc=0;
	double cutoff2=cutoff*cutoff;
	int suf_num=(int)surfxyz.size();
	int lig_num=(int)ligand.size();
	int rel_num=suf_num<lig_num?suf_num:lig_num;
	//-> calculate surfxyz center [note]: we define 1st line as the predicted center
	vector <vector <double> > surfxyz_center;
	surfxyz_center.resize(rel_num);
	for(int i=0;i<rel_num;i++)surfxyz_center[i]=surfxyz[i][0];
	//-> calculate ligand center
	vector <vector <double> > ligand_center;
	ligand_center.resize(lig_num);
	for(int i=0;i<lig_num;i++)
	{
		vector <double> point(3,0);
		int count=0;
		for(int j=0;j<(int)ligand[i].size();j++)
		{
			for(int k=0;k<3;k++)point[k]+=ligand[i][j][k];
			count++;
		}
		if(count!=0)
		{
			for(int k=0;k<3;k++)point[k]/=count;
		}
		ligand_center[i]=point;
	}
	//-> calculate DCA
	for(int i=0;i<lig_num;i++)
	{
		for(int j=0;j<(int)ligand[i].size();j++)
		{
			int found=0;
			for(int k=0;k<rel_num;k++)
			{
				double dist2=Distance2(ligand[i][j],surfxyz_center[k]);
				if(dist2<cutoff2)
				{
					found=1;
					break;
				}
			}
			if(found==1)
			{
				dca++;
				break;
			}
		}
	}
	//-> calculate DCC
	for(int i=0;i<lig_num;i++)
	{
		int found=0;
		for(int k=0;k<rel_num;k++)
		{
			double dist2=Distance2(ligand_center[i],surfxyz_center[k]);
			if(dist2<cutoff2)
			{
				found=1;
				break;
			}
		}
		if(found==1)
		{
			dcc++;
		}
	}
	//return
	return lig_num;
}


//---------------- protein-centric measurements --------------//
//-> IoU (intersectio over union) of atoms and residues, in consideration of all pockets
void Calculate_ResiAtom(double thres,int topk,
	vector <vector <vector <double> > > &lig,
	vector <vector <vector <double> > > &prot,
	vector <int> &atom_id, vector <int> &resi_str)
{
	//init
	double thres2=thres*thres;
	int atom_num=0;
	int resi_num=0;
	for(int k=0;k<(int)prot.size();k++)
	{
		for(int l=0;l<(int)prot[k].size();l++)atom_num++;
		resi_num++;
	}
	vector <int> atom_index(atom_num,0);
	vector <int> resi_index(resi_num,0);
	//proc
	int lig_num=(int)lig.size();
	int rel_num=topk<lig_num?topk:lig_num;
	for(int i=0;i<rel_num;i++)
	{
		for(int j=0;j<(int)lig[i].size();j++)
		{
			int atom_num=0;
			int resi_num=0;
			for(int k=0;k<(int)prot.size();k++)
			{
				for(int l=0;l<(int)prot[k].size();l++)
				{
					if(atom_index[atom_num]==0)
					{
						double dist2=Distance2(lig[i][j],prot[k][l]);
						if(dist2<thres2)
						{
							atom_index[atom_num]=1;
							resi_index[resi_num]=1;
						}
					}
					atom_num++;
				}
				resi_num++;
			}
		}
	}
	//final
	atom_id=atom_index;
	resi_str=resi_index;
}
void Calculate_ResiAtom_Label(
	vector <vector <int> > &label,
	vector <int> &atom_id, vector <int> &resi_str)
{
	//init
	int tot_atom_num=0;
	int tot_resi_num=0;
	for(int k=0;k<(int)label.size();k++)
	{
		for(int l=0;l<(int)label[k].size();l++)tot_atom_num++;
		tot_resi_num++;
	}
	vector <int> atom_index(tot_atom_num,0);
	vector <int> resi_index(tot_resi_num,0);
	//proc
	int atom_num=0;
	int resi_num=0;
	for(int k=0;k<(int)label.size();k++)
	{
		for(int l=0;l<(int)label[k].size();l++)
		{
			if(label[k][l]!=0)
			{
				atom_index[atom_num]=1;
				resi_index[resi_num]=1;
			}
			atom_num++;
		}
		resi_num++;
	}
	//final
	atom_id=atom_index;
	resi_str=resi_index;
}

//-> calculate IoU
double Calc_IoU(vector <int> &in1, vector <int> &in2)
{
	//-> check length
	int len1=(int)in1.size();
	int len2=(int)in2.size();
	if(len1!=len2)
	{
		fprintf(stderr,"len1 %d not equal to len2 %d \n",
			len1,len2);
		exit(-1);
	}
	//-> calculate total
	int len=len1;
	vector <int> total(len,0);
	for(int i=0;i<len;i++)
	{
		if(in1[i]>0)total[i]++;
		if(in2[i]>0)total[i]++;
	}
	//-> calculate IoU
	int intersect=0;
	int unionsect=0;
	for(int i=0;i<len;i++)
	{
		if(total[i]>0)unionsect++;
		if(total[i]>1)intersect++;
	}
	if(unionsect>0)return 1.0*intersect/unionsect;
}


//===================== PointSite SavePlan ======================//
void PointSite_SavePlan(
	double dist_thres, double ratio_thres,
	vector <int> &pred_atom_idx, vector <int> &pred_resi_idx,
	vector <vector <vector <double> > > &lig,
	vector <vector <vector <double> > > &prot,
	vector <vector <vector <double> > > &surfacexyz_,
	vector <vector <vector <double> > > &pocketxyz_,
	vector <double> &prob_score_)
{
	//init
	double thres2=dist_thres*dist_thres;
	int tot_atom_num=0;
	int tot_resi_num=0;
	for(int k=0;k<(int)prot.size();k++)
	{
		for(int l=0;l<(int)prot[k].size();l++)tot_atom_num++;
		tot_resi_num++;
	}
	//proc
	vector <vector <vector <double> > > surfacexyz;
	vector <vector <vector <double> > > pocketxyz;
	vector <double> prob_score;
	pocketxyz.clear();
	surfacexyz.clear();
	prob_score.clear();
	int valid_num=0;
	//-> for each ligand
	for(int i=0;i<(int)lig.size();i++)
	{
		//----- hit ligand -----//
		vector <int> atom_index(tot_atom_num,0);
		vector <int> resi_index(tot_resi_num,0);
		for(int j=0;j<(int)lig[i].size();j++)
		{
			//-> for each protein atom
			int atom_num=0;
			int resi_num=0;
			for(int k=0;k<(int)prot.size();k++)
			{
				for(int l=0;l<(int)prot[k].size();l++)
				{
					if(atom_index[atom_num]==0)
					{
						double dist2=Distance2(lig[i][j],prot[k][l]);
						if(dist2<thres2)
						{
							atom_index[atom_num]=1;
							resi_index[resi_num]=1;
						}
					}
					atom_num++;
				}
				resi_num++;
			}
		}
		//----- check result ------//
		int common=0;
		int hitnum=0;
		for(int k=0;k<tot_atom_num;k++)
		{
			if(atom_index[k]!=0 && pred_atom_idx[k]!=0)common++;
			if(atom_index[k]!=0) hitnum++;
		}
		if(hitnum==0)continue;
		if(1.0*common/hitnum<ratio_thres)continue;
		//----- add result --------//
		valid_num++;
		prob_score.push_back(1.0*common);
		//--|| we have two options here: (i) directly add the surface_center result, (ii) add the common_atom result
		//-> part (i) add the surface_center result
		vector <vector <double> > tmp_surf;
		for(int k=0;k<(int)lig[i].size();k++)tmp_surf.push_back(lig[i][k]);
		surfacexyz.push_back(tmp_surf);
		//-> part (ii) add the common_atom result
		vector <vector <double> > tmp_rec;
		vector <double> point(3,0);
		int znum=0;
		//--| for each common protein atom
		int atom_num=0;
		int resi_num=0;
		for(int k=0;k<(int)prot.size();k++)
		{
			for(int l=0;l<(int)prot[k].size();l++)
			{
				if(atom_index[atom_num]!=0 && pred_atom_idx[atom_num]!=0)
				{
					tmp_rec.push_back(prot[k][l]);
					for(int z=0;z<3;z++)point[z]+=prot[k][l][z];
					znum++;
				}
				atom_num++;
			}
			resi_num++;
		}
		//--| push_front and push_back
		if(znum!=0)for(int z=0;z<3;z++)point[z]/=znum;
		tmp_rec.insert(tmp_rec.begin(),point);
		pocketxyz.push_back(tmp_rec);
	}
	//---- sort result -----//
	vector <int> order;
	Vector_Sort(prob_score, order);
	pocketxyz_.clear();
	surfacexyz_.clear();
	prob_score_.clear();
	for(int i=0;i<valid_num;i++)
	{
		int index=order[i];
		pocketxyz_.push_back(pocketxyz[index]);
		surfacexyz_.push_back(surfacexyz[index]);
		prob_score_.push_back(prob_score[index]);
	}
}




//-------- main ---------//
int main(int argc,char **argv)
{
	//---- PointSite_Save ----//
	{
		if(argc<7)
		{
			fprintf(stderr,"Version 1.00 \n");
			fprintf(stderr,"PointSite_Save <protein_xyz> <ligand_xyz> <surf_pred> \n");
			fprintf(stderr,"               <dist_thres> <ratio_thres> <DCA_DIST> \n");
			fprintf(stderr,"[note]: <dist_thres> is the distance cutoff to define the predicted pocket atoms. (e.g., 6.5A)\n");
			fprintf(stderr,"        <ratio_thres> is the ratio cutoff to select the common biding atoms (e.g., 0.85) \n");
			fprintf(stderr,"        <DCA_DIST> is the distance to calculate DCC and DCA. (e.g., 4.0A) \n");
			exit(-1);
		}
		string protein_xyz=argv[1];
		string ligand_xyz=argv[2];
		string surf_pred=argv[3];
		double dist_thres=atof(argv[4]);
		double ratio_thres=atof(argv[5]);
		double DCA_DIST=atof(argv[6]);
		//--- fixed parameter --//
		double cutoff_truth=6.5;

		//=============== load data ============//
		//---- load protein -----//
		vector <vector <vector <double> > > prot;
		vector <vector <string> > str1;
		vector <vector <int> > lab1;
		vector <vector <string> > remain1;
		vector <string> resi1;
		Load_XYZ_Label(protein_xyz,prot,str1,lab1,remain1,resi1);
		//---- load ligand -----//
		vector <vector <vector <double> > > lig;
		vector <vector <string> > str2;
		vector <vector <int> > lab2;
		vector <vector <string> > remain2;
		vector <string> resi2;
		Load_XYZ_Label(ligand_xyz,lig,str2,lab2,remain2,resi2);
		//---- load surface prediction ----//
		vector <vector <vector <double> > > surf_pred_xyz;
		vector <string> surf_pred_resi;
		Load_SurfXYZ(surf_pred,surf_pred_xyz,surf_pred_resi);

		//================ save pointsite ======//
		//-> calculate pointsite predicted atoms
		vector <int> def_atom_id;
		vector <int> def_resi_str;
		Calculate_ResiAtom_Label(lab1,def_atom_id,def_resi_str);
		//-> save pointsite
		vector <vector <vector <double> > > surfacexyz;
		vector <vector <vector <double> > > pocketxyz;
		vector <double> prob_score;
		PointSite_SavePlan(dist_thres,ratio_thres,
			def_atom_id,def_resi_str,surf_pred_xyz,prot,surfacexyz,pocketxyz,prob_score);


//---- output these two results ---------//

{
	fprintf(stderr,"#---------- ground_truth output ------------# \n");
	for(int k=0;k<(int)lig.size();k++)
	{
		vector <double> point(3,0);
		int count=0;
		for(int l=0;l<(int)lig[k].size();l++)
		{
			for(int z=0;z<3;z++)point[z]+=lig[k][l][z];
			count++;
		}
		if(count!=0)
		{
			for(int z=0;z<3;z++)point[z]/=count;
		}
		fprintf(stderr,"%d %8.3f %8.3f %8.3f\n",
			k+1,point[0],point[1],point[2]);
		for(int l=0;l<(int)lig[k].size();l++)
		{
			fprintf(stderr,"%d %8.3f %8.3f %8.3f\n",
				k+1,lig[k][l][0],lig[k][l][1],lig[k][l][2]);
		}
	}
	fprintf(stderr,"#---------- prob_score output --------------# \n");
	for(int k=0;k<(int)prob_score.size();k++)
	{
		fprintf(stderr,"%d %lf\n",k+1,prob_score[k]);
	}
	fprintf(stderr,"#---------- surfacexyz output --------------# \n");
	for(int k=0;k<(int)surfacexyz.size();k++)
	{
		for(int l=0;l<(int)surfacexyz[k].size();l++)
		{
			fprintf(stderr,"%d %8.3f %8.3f %8.3f\n",
				k+1,surfacexyz[k][l][0],surfacexyz[k][l][1],surfacexyz[k][l][2]);
		}
	}
	fprintf(stderr,"#---------- pocketxyz output --------------# \n");
	for(int k=0;k<(int)pocketxyz.size();k++)
	{
		for(int l=0;l<(int)pocketxyz[k].size();l++)
		{
			fprintf(stderr,"%d %8.3f %8.3f %8.3f\n",
				k+1,pocketxyz[k][l][0],pocketxyz[k][l][1],pocketxyz[k][l][2]);
		}
	}
}


		//================ assessment ==========//
		//----- pocket-centric perspective ---//
		//-> DCA and DCC (ligand-base)
		int dca_lig,dcc_lig;
		int lignum_lig=DCA_DCC_Calc(surfacexyz,lig,DCA_DIST,
			dca_lig,dcc_lig);
		printf("ligand-base DCA: %d DCC: %d LIGNUM: %d | ",
			dca_lig,dcc_lig,lignum_lig);
		//-> DCA and DCC (protein-base)
		int dca_prot,dcc_prot;
		int lignum_prot=DCA_DCC_Calc(pocketxyz,lig,DCA_DIST,
			dca_prot,dcc_prot);
		printf("protein-base DCA: %d DCC: %d LIGNUM: %d | ",
			dca_prot,dcc_prot,lignum_lig);

		//----- protein-centric perspective ---//
		//-> ground-truth pocket residues and atoms for ALL ligands (maximal number is lig_size)
		vector <int> gt_atom_id;
		vector <int> gt_resi_str;
		Calculate_ResiAtom(cutoff_truth,lig.size(),lig,prot,gt_atom_id,gt_resi_str);
		//-> predicted pocket residues and atoms for ALL ligands (maximal number is lig_size)
		vector <int> pred_atom_id;
		vector <int> pred_resi_str;
		Calculate_ResiAtom(dist_thres,lig.size(),surfacexyz,prot,pred_atom_id,pred_resi_str);
		//-> calculate IoU on residue- and atom-level (merged result of IoU)
		double atom_IoU=Calc_IoU(gt_atom_id,pred_atom_id);
		double resi_IoU=Calc_IoU(gt_resi_str,pred_resi_str);
		printf("atom_IoU: %lf resi_IoU: %lf | ",atom_IoU,resi_IoU);
		//-> calculate IoU on residue- and atom-level (original IoU from PointSite)
		double atom_IoU_orig=Calc_IoU(gt_atom_id,def_atom_id);
		double resi_IoU_orig=Calc_IoU(gt_resi_str,def_resi_str);
		printf("atom_IoU_orig: %lf resi_IoU_orig: %lf \n",atom_IoU_orig,resi_IoU_orig);
	}
}


