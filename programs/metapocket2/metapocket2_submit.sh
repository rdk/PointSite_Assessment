#!/bin/bash


# ----- usage ------ #
usage()
{
	echo "MetaPocket2_submit [Oct-16-2019] "
	echo "    Submit a PDB file to MetaPocket2 webserver, and obtain the results automatically. "
	echo ""
	echo "USAGE:  ./metapocket2_submit.sh <-i input_pdb> [-d chrome_driver] [-o out_root] [-n ligand_number] "
	echo "                             [-s sleep_time] [-r refresh_time] [-w wait_time] [-m maxwati_time] "
	echo "                             [-C occu_thres] [-D surf_dist] [-T spread_thres] [-H home] "
	echo "Options:"
	echo ""
	echo "***** required arguments *****"
	echo "-i input_pdb      : Input protein in PDB format. "
	echo ""
	echo "***** optional arguments *****"
	echo "-d chrome_driver  : chrome driver [version: 77.0.3865.90]. [default = 'bin/chromedriver' ] "
	echo "" 
	echo "-o out_root       : Default output would the current directory. [default = './\${input_name}_MetaPocket2'] "
	echo ""
	echo "-n ligand_number  : The maximal number of ligand in MetaPocket2. [default = 10 ] "
	echo ""
	echo "-s sleep_time     : The sleep time during selenium operation. [default = 2] "
	echo ""
	echo "-r refresh_time   : The refresh time during selenium operation. [default = 5] "
	echo ""
	echo "-w wait_time      : The wait time during selenium operation. [default = 20] "
	echo ""
	echo "-m maxwati_time   : The maximal wait time during selenium operation. [default = 60] "
	echo ""
	echo "***** relevant directories *****"
	echo "-H home           : home directory of metapocket2_submit.sh "
	echo "                    [default = `dirname $0`]"
	echo ""
	exit 1
}

#-------------------------------------------------------------#
##### ===== get pwd and check HomeDirectory ====== #########
#-------------------------------------------------------------#

#------ current directory ------#
curdir="$(pwd)"


#-------- check usage -------#
if [ $# -lt 1 ];
then
	usage
fi



#---------------------------------------------------------#
##### ===== All arguments are defined here ====== #########
#---------------------------------------------------------#


# ----- get arguments ----- #
#-> required arguments
input_pdb=""
#-> optional arguments
out_root=""     #-> output to current directory
#-> home directory
home=`dirname $0`
#-> driver file
chrome_driver=$home/bin/chromedriver
#-> ligand number
ligand_number=10
#-> time during selenium
sleep_time=2
refresh_time=5
wait_time=20
maxwait_time=60


#-> parse arguments
while getopts "i:d:o:n:s:w:m:r:H:" opt;
do
	case $opt in
	#-> required arguments
	i)
		input_pdb=$OPTARG
		;;
	#-> optional arguments
	d)
		chrome_driver=$OPTARG
		;;
	o)
		out_root=$OPTARG
		;;
	n)
		ligand_number=$OPTARG
		;;
	#-> time for selenium
	s)
		sleep_time=$OPTARG
		;;
	r)
		refresh_time=$OPTARG
		;;
	w)
		wait_time=$OPTARG
		;;
	m)
		maxwait_time=$OPTARG
		;;
	#-> relevant directories
	H)
		home=$OPTARG
		;;
	#-> help
	\?)
		echo "Invalid option: -$OPTARG" >&2
		exit 1
		;;
	:)
		echo "Option -$OPTARG requires an argument." >&2
		exit 1
		;;
	esac
done


#---------------------------------------------------------#
##### ===== Part 0: initial argument check ====== #########
#---------------------------------------------------------#


# ------ check home directory ---------- #
if [ ! -d "$home" ]
then
	echo "home directory $home not exist " >&2
	exit 1
fi
home=`readlink -f $home`

#----------- check input_pdb  -----------#
if [ -z "$input_pdb" ]
then
	echo "input input_pdb is null !!" >&2
	exit 1
fi
if [ ! -s "$input_pdb" ]
then
	echo "input_pdb $input_pdb not found !!" >&2
	exit 1
fi
input_pdb=`readlink -f $input_pdb`
#-> get query_name
fulnam=`basename $input_pdb`
relnam=${fulnam%.*}

# ------- check chrome_driver -------#
if [ ! -s "$chrome_driver" ]
then
	echo "chrome_driver $chrome_driver not found !!" > $2
	exit 1
fi

# ------ check output directory -------- #
if [ "$out_root" == "" ]
then
	out_root=$curdir/${relnam}_MetaPocket2
fi
mkdir -p $out_root
out_root=`readlink -f $out_root`


#--------------------------------------------------------#
##### ===== Part 1: submit to MetaPocket2 ====== #########
#--------------------------------------------------------#

#-> submit to MetaPocket2 server
source activate selenium
python $home/metapocket2_submit.py -i $input_pdb -o $out_root -d $chrome_driver -n $ligand_number \
	-s $sleep_time -r $refresh_time -w $wait_time -m $maxwait_time
source deactivate

#-> extract pivot surf points
num=`grep "MPT" $out_root/$relnam.cent | awk '{print $5}' | sort | uniq | wc | awk '{print $1}'`
rm -f $out_root/$relnam.surface
for ((k=1;k<=num;k++))
do
	#--> put predicted center as the first
	grep "MPT" $out_root/$relnam.cent | \
		awk '{if($5==id){x=substr($0,31,8);y=substr($0,39,8);z=substr($0,47,8);print id" "x" "y" "z}}' id=$k >> \
		$out_root/$relnam.surface
	#--> output others
	grep "^ATOM" $out_root/$relnam.surf | \
		awk '{if($5==id){x=substr($0,31,8);y=substr($0,39,8);z=substr($0,47,8);print id" "x" "y" "z}}' id=$k >> \
		$out_root/$relnam.surface
done

# ========= exit 0 =========== #
exit 0


