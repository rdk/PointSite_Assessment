#!/bin/bash

if [ $# -lt 5 ]
then
	echo "Usage: ./deepsite_run.sh <data> <indir> <outdir> <cpunum> <home> "
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


#----- run deepsite --------#
#-> for deepsite (not recommend to run, as it will submit the jobs to the website and need 300 to 500 seconds for each job!!)
#                (also, the maximal length for deepsite shall below 1000 amino acids!!)
#-- run deepsite in a batch
tmpdir=/tmp/deepsite_tmpdir
mkdir -p $tmpdir
rm -f deepsite_proc
for i in `cat $indir/$data`
do
	#--| get name
	fulnam=`basename $i`
	relnam=${fulnam%.*}
	#--| submit to DeepSite website
	echo "$home/deepsite_submit.sh -i $indir/${relnam}.pdb -o $tmpdir/${relnam}_out" >> deepsite_proc
done

#-- single proc
#bash deepsite_proc   #-> we have to use at most 1 threads to run this submission job
#-- batch proc
threadnum=$cpunum     #-> we have to use at most 8 threads to run this API submission job
split -da 8 -n r/$threadnum deepsite_proc /tmp/deepsite_proc.
for ((i=0;i<$threadnum;i++));
do
	suffix=`echo $i | awk '{printf("%08d\n",$0)}'`
	line=$((i+1));
	script=`head -n $line $home/batch_id/account_list | tail -n1`;
	content="$home/batch_id/${script}";
	awk '{print $0" -c "c}' c=$content /tmp/deepsite_proc.$suffix > /tmp/deepsite_proc.$suffix.mod;
	awk '{print $3}' /tmp/deepsite_proc.$suffix.mod | awk -F"/" '{print $NF}' | cut -d '.' -f 1 > tmp_list;
	paste -d '|' /tmp/deepsite_proc.$suffix.mod tmp_list > tmp_list2;
	awk -F"|" '{print $1" 2> "tmpdir"/"$2".log"}' tmpdir=$tmpdir tmp_list2 > /tmp/deepsite_proc.$suffix.mod2;
	rm -f tmp_list tmp_list2 /tmp/deepsite_proc.$suffix /tmp/deepsite_proc.$suffix.mod;
	bash /tmp/deepsite_proc.$suffix.mod2 &
	sleep 5
done
wait
rm -f deepsite_proc
rm -f /tmp/deepsite_proc.*

#-- post process
outroot=$outdir/deepsite_$data
mkdir -p $outroot
for i in `cat $indir/$data`
do
	#--| get name
	fulnam=`basename $i`
	relnam=${fulnam%.*}
	#--| collect final result
	mkdir -p $outroot/${relnam}_out
	mv $tmpdir/${relnam}_out/$relnam.csv $outroot/${relnam}_out
	mv $tmpdir/${relnam}_out/$relnam.pdb $outroot/${relnam}_out
	mv $tmpdir/${relnam}_out/$relnam.cube $outroot/${relnam}_out
	mv $tmpdir/${relnam}_out/$relnam.surf_pivot $outroot/${relnam}_out
	mv $tmpdir/${relnam}_out/$relnam.surface $outroot/${relnam}_out
done
rm -rf $tmpdir


#===== exit =====#
exit 0

