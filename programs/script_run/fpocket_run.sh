#!/bin/bash

if [ $# -lt 6 ]
then
	echo "Usage: ./fpocket_run.sh <data> <indir> <outdir> <cpunum> <home> <distri>"
	echo "[note]: <data> shall be list containing the input files, such as '.pdb'. "
	echo "        <indir> shall be the input directory that contains these input files. "
	echo "        <outdir> shall be the output directory for the result. "
	echo "        <cpunum> shall be the CPU number. "
	echo "        <home> shall be the home directory of the program. "
	echo "        <distri> is the root for 'distribute_bash.sh'. "
	exit
fi

#----- input arguments ---------#
data=$1
indir=$2
outdir=$3
cpunum=$4
home=$5
distri=$6

#----- run fpocket --------#
#-- run fpocket in a batch
rm -f fpocket_proc
for i in `cat $indir/$data`
do
	#--| get name
	fulnam=`basename $i`
	relnam=${fulnam%.*}
	#--| run fpocket
	echo "$home/bin/fpocket -f $indir/${relnam}.pdb" >> fpocket_proc
done
$distri/distribute_bash.sh fpocket_proc $cpunum ./
rm -f fpocket_proc

#-- post process
outroot=$outdir/fpocket_$data
mkdir -p $outroot
for i in `cat $indir/$data`
do
	#--| get name
	fulnam=`basename $i`
	relnam=${fulnam%.*}
	#--| move results
	rm -rf $outroot/${relnam}_out
	mv $indir/${relnam}_out $outroot
	#--| extract surface point
	num=`grep "STP" $outroot/${relnam}_out/${relnam}_pockets.pqr | awk '{print $5}' | sort | uniq | wc | awk '{print $1}'`
	rm -f $outroot/${relnam}_out/${relnam}.surface
	for ((k=1;k<=num;k++))
	do
		#--> put predicted center as the first
		grep "STP" $outroot/${relnam}_out/${relnam}_pockets.pqr | \
			awk '{if($5==id){x=substr($0,31,8);y=substr($0,39,8);z=substr($0,47,8);print id" "x" "y" "z}}' id=$k | \
			awk 'BEGIN{x=0;y=0;z=0;c=0}{x+=$2;y+=$3;z+=$4;c++}END{printf "%d %8.3f %8.3f %8.3f\n",id,x/c,y/c,z/c}' id=$k >> \
			$outroot/${relnam}_out/${relnam}.surface
		#--> output others
		grep "STP" $outroot/${relnam}_out/${relnam}_pockets.pqr | \
			awk '{if($5==id){x=substr($0,31,8);y=substr($0,39,8);z=substr($0,47,8);print id" "x" "y" "z}}' id=$k >> \
			$outroot/${relnam}_out/${relnam}.surface
	done
	#--| remove unnecessary files
	rm -rf $outroot/${relnam}_out/pockets
	rm -f $outroot/${relnam}_out/${relnam}_*
	rm -f $outroot/${relnam}_out/${relnam}.tcl $outroot/${relnam}_out/${relnam}.pml
done

#===== exit =====#
exit 0

