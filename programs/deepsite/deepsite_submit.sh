#!/bin/bash


# ----- usage ------ #
usage()
{
	echo "DeepSite_submit [Oct-16-2019] "
	echo "    Submit a PDB file to DeepSite webserver, and obtain the results automatically. "
	echo ""
	echo "USAGE:  ./deepsite_submit.sh <-i input_pdb> [-d chrome_driver] [-o out_root] [-c config_file] "
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
	echo "-o out_root       : Default output would the current directory. [default = './\${input_name}_DeepSite'] "
	echo ""
	echo "-c config_file    : The config file for user and password. [default = 'config.json' ] "
	echo ""
	echo "-s sleep_time     : The sleep time during selenium operation. [default = 2] "
	echo ""
	echo "-r refresh_time   : The refresh time during selenium operation. [default = 5] "
	echo ""
	echo "-w wait_time      : The wait time during selenium operation. [default = 20] "
	echo ""
	echo "-m maxwati_time   : The maximal wait time during selenium operation. [default = 60] "
	echo ""
	echo "***** CUBE_To_SURF arguments **"
	echo "-C occu_thres     : The threshold for occupancy in cube_file. [default = 0.1] "
	echo ""
	echo "-D surf_dist      : The distance for cube points spread around input_surf points. [default = 4.5] "
	echo ""
	echo "-T spread_thres   : The occupancy value for cube points spread around input_surf points. [default = 0.85] "
	echo ""
	echo "***** relevant directories *****"
	echo "-H home           : home directory of deepsite_submit.sh "
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
#-> config file
config_file=$home/config.json
#-> time during selenium
sleep_time=2
refresh_time=5
wait_time=20
maxwait_time=60
#-> CUBE_To_SURF parameters
occu_thres=0.1
surf_dist=4.5
spread_thres=0.85



#-> parse arguments
while getopts "i:d:o:c:s:w:m:r:C:D:T:H:" opt;
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
	c)
		config_file=$OPTARG
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
	#-> CUBE_To_SURF parameters
	C)
		occu_thres=$OPTARG
		;;
	D)
		surf_dist=$OPTARG
		;;
	T)
		spread_thres=$OPTARG
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
# ------- check config_file -------#
if [ ! -s "$config_file" ]
then
	echo "config_file $config_file not found !!" > $2
	exit 1
fi

# ------ check output directory -------- #
if [ "$out_root" == "" ]
then
	out_root=$curdir/${relnam}_DeepSite
fi
mkdir -p $out_root
out_root=`readlink -f $out_root`


#-----------------------------------------------------#
##### ===== Part 1: submit to DeepSite ====== #########
#-----------------------------------------------------#

#-> submit to DeepSite server
source activate selenium
python $home/deepsite_submit.py -i $input_pdb -o $out_root -d $chrome_driver -c $config_file \
	-s $sleep_time -r $refresh_time -w $wait_time -m $maxwait_time
source deactivate

#-> extract pivot surf points
grep "HOH" $out_root/$relnam.pdb | grep "XXX" | awk '{a=substr($0,7);print a}' | \
	awk '{id=$5;x=substr($0,25,8);y=substr($0,33,8);z=substr($0,41,8); print id" "x" "y" "z }' > $out_root/$relnam.surf_pivot

#-> generate final surf output
$home/util/CUBE_To_SURF $out_root/$relnam.cube $out_root/$relnam.surf_pivot $out_root/$relnam.surface \
	$occu_thres $surf_dist $spread_thres 0

# ========= exit 0 =========== #
exit 0


