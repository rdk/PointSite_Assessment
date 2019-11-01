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


//---------------- protein-centric measurements --------------//
//-> IoU (intersectio over union) of atoms and residues, in consideration of all pockets
void Calculate_ResiAtom_Label(
	vector <vector <int> > &label,
	vector <int> &atom_id, vector <int> &resi_str)
{
	//init
	int atom_num=0;
	int resi_num=0;
	for(int k=0;k<(int)label.size();k++)
	{
		for(int l=0;l<(int)label[k].size();l++)atom_num++;
		resi_num++;
	}
	vector <int> atom_index(atom_num,0);
	vector <int> resi_index(resi_num,0);
	//proc
	atom_num=0;
	resi_num=0;
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

//--------------- calculate IoU ---------------//
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



//-------- main ---------//
int main(int argc,char **argv)
{
	//---- PointSite_IoU ----//
	{
		if(argc<3)
		{
			fprintf(stderr,"Version 1.00 \n");
			fprintf(stderr,"PointSite_IoU <gt_xyz_label> <pred_xyz_label> \n");
			exit(-1);
		}
		string gt_xyz_label=argv[1];
		string pred_xyz_label=argv[2];

		//=============== load data ============//
		//---- load protein -----//
		vector <vector <vector <double> > > gt_xyz;
		vector <vector <string> > str1;
		vector <vector <int> > lab1;
		vector <vector <string> > remain1;
		vector <string> resi1;
		Load_XYZ_Label(gt_xyz_label,gt_xyz,str1,lab1,remain1,resi1);
		//---- load ligand -----//
		vector <vector <vector <double> > > pred_xyz;
		vector <vector <string> > str2;
		vector <vector <int> > lab2;
		vector <vector <string> > remain2;
		vector <string> resi2;
		Load_XYZ_Label(pred_xyz_label,pred_xyz,str2,lab2,remain2,resi2);

		//================ assessment ==========//
		//----- protein-centric perspective ---//
		//-> ground-truth pocket residues and atoms for ALL ligands
		vector <int> gt_atom_id;
		vector <int> gt_resi_str;
		Calculate_ResiAtom_Label(lab1,gt_atom_id,gt_resi_str);
		//-> predicted pocket residues and atoms for ALL ligands
		vector <int> pred_atom_id;
		vector <int> pred_resi_str;
		Calculate_ResiAtom_Label(lab2,pred_atom_id,pred_resi_str);
		//-> check length
		if(gt_atom_id.size() != pred_atom_id.size())
		{
			fprintf(stderr,"gt_atom_id.size %d not equal to pred_atom_id.size %d\n",
				gt_atom_id.size(),pred_atom_id.size());
			exit(-1);
		}
		if(gt_resi_str.size() != pred_resi_str.size())
		{
			fprintf(stderr,"gt_resi_str.size %d not equal to pred_resi_str.size %d\n",
				gt_resi_str.size(),pred_resi_str.size());
			exit(-1);
		}		
		//-> calculate IoU on residue- and atom-level
		double atom_IoU=Calc_IoU(gt_atom_id,pred_atom_id);
		double resi_IoU=Calc_IoU(gt_resi_str,pred_resi_str);
		printf("atom_IoU: %lf resi_IoU: %lf \n",atom_IoU,resi_IoU);
	}
}



