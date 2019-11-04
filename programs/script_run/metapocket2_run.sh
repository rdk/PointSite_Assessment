#!/bin/bash

if [ $# -lt 5 ]
then
	echo "Usage: ./metapocket2_run.sh <data> <indir> <outdir> <cpunum> <home> "
	echo "[note]: <data> shall be list containing the input files, such as '.pdb'. "
	echo "        <indir> shall be the input directory that contains these input files. "
	echo "        <outdir> shall be the output directory for the result. "
	echo "        <cpunum> shall be the CPU number. "
	echo "        <home> shall be the home directory of the program. "
	exit
fi

#----- input arguments ---------#
data=$1
indir=$2
outdir=$3
cpunum=$4
home=$5


#----- run metapocket2 --------#
#-> for metapocket2 (not recommend to run, as it will submit the jobs to the website and need 300 to 500 seconds for each job!!)
#-- run metapocket2 in a batch
tmpdir=/tmp/metapocket2_tmpdir
mkdir -p $tmpdir
rm -f metapocket2_proc
for i in `cat $indir/$data`
do
	#--| get name
	fulnam=`basename $i`
	relnam=${fulnam%.*}
	#--| submit to MetaPocket2 website
	echo "$home/metapocket2_submit.sh -i $indir/${relnam}.pdb -o $tmpdir/${relnam}_out" >> metapocket2_proc
done

#-- single proc
#bash metapocket2_proc   #-> we have to use at most 1 threads to run this submission job
#-- batch proc
threadnum=$cpunum        #-> we have to use at most 8 threads to run this API submission job
split -da 8 -n r/$threadnum metapocket2_proc /tmp/metapocket2_proc.
for ((i=0;i<$threadnum;i++));
do
	suffix=`echo $i | awk '{printf("%08d\n",$0)}'`
	awk '{print $3}' /tmp/metapocket2_proc.$suffix | awk -F"/" '{print $NF}' | cut -d '.' -f 1 > tmp_list;
	paste -d '|' /tmp/metapocket2_proc.$suffix tmp_list > tmp_list2;
	awk -F"|" '{print $1" 2> "tmpdir"/"$2".log"}' tmpdir=$tmpdir tmp_list2 > /tmp/metapocket2_proc.$suffix.mod;
	rm -f tmp_list tmp_list2 /tmp/metapocket2_proc.$suffix;
	bash /tmp/metapocket2_proc.$suffix.mod &
	sleep 5
done
wait
rm -f metapocket2_proc
rm -f /tmp/metapocket2_proc.*

#-- post process
outroot=$outdir/metapocket2_$data
mkdir -p $outroot
for i in `cat $indir/$data`
do
	#--| get name
	fulnam=`basename $i`
	relnam=${fulnam%.*}
	#--| collect final result
	mkdir -p $outroot/${relnam}_out
	mv $tmpdir/${relnam}_out/$relnam.cent $outroot/${relnam}_out
	mv $tmpdir/${relnam}_out/$relnam.surf $outroot/${relnam}_out
	mv $tmpdir/${relnam}_out/$relnam.surface $outroot/${relnam}_out
done
rm -rf $tmpdir


#===== exit =====#
exit 0

