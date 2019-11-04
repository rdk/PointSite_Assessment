#!/bin/bash

if [ $# -lt 7 ]
then
	echo "Usage: ./sitehound_run.sh <data> <indir> <outdir> <cpunum> <home> <buildmod> <distri> "
	echo "[note]: <data> shall be list containing the input files, such as '.pdb'. "
	echo "        <indir> shall be the input directory that contains these input files. "
	echo "        <outdir> shall be the output directory for the result. "
	echo "        <cpunum> shall be the CPU number. "
	echo "        <home> shall be the home directory of the program. "
	echo "        <buildmod> is the root for 'BuildModel_Package'. "
	echo "        <distri> is the root for 'distribute_bash.sh'. "
	exit
fi

#----- input arguments ---------#
data=$1
indir=$2
outdir=$3
cpunum=$4
home=$5
buildmod=$6
distri=$7

#----- run sitehound --------#
#-- preliminary
tmpdir=/tmp/sitehound_tmpdir
mkdir -p $tmpdir

#-- check BuildModel_Package
run_complete=1
if [ ! -s "$buildmod/util/Complete_PDB.sh" ]
then
	echo "$buildmod/util/Complete_PDB.sh file not found or null !"
	run_complete=0
fi


#-- run sitehound in a batch
rm -f sitehound_proc
for i in `cat $indir/$data`
do
	#--| get name
	fulnam=`basename $i`
	relnam=${fulnam%.*}
	#--| run sitehound
	if [ $run_complete -eq 1 ]
	then
		echo "$buildmod/util/Complete_PDB.sh $indir/${relnam}.pdb $tmpdir/${relnam}.pdb; $home/auto.py -i $tmpdir/${relnam}.pdb -p CMET;" \
			>> sitehound_proc
	else
		echo "cp $indir/${relnam}.pdb $tmpdir/${relnam}.pdb; home/auto.py -i $tmpdir/${relnam}.pdb -p CMET;" >> sitehound_proc
	fi
done
$distri/distribute_bash.sh sitehound_proc $cpunum ./
rm -f sitehound_proc
rm -f atom_types.txt ffG43b1nb.params

#-- post process
outroot=$outdir/sitehound_$data
mkdir -p $outroot
for i in `cat $indir/$data`
do
	#--| get name
	fulnam=`basename $i`
	relnam=${fulnam%.*}
	#--| move results
	mkdir -p $outroot/${relnam}_out
	mv $tmpdir/${relnam}_CMET* $tmpdir/${relnam}.easymifs $outroot/${relnam}_out
	#--| extract surface point
	num=`awk '{print $1}' $outroot/${relnam}_out/${relnam}_CMET_clusters.dat | sort | uniq | wc | awk '{print $1}'`
	rm -f $outroot/${relnam}_out/${relnam}.surface
	for ((k=1;k<=num;k++))
	do
		#--> put predicted center as the first
		grep "^$k" $outroot/${relnam}_out/${relnam}_CMET_summary.dat | head -n1 | awk '{printf "%d %8.3f %8.3f %8.3f\n",$1,$4,$5,$6}' >> \
			$outroot/${relnam}_out/${relnam}.surface
		#--> output others
		awk '{if($1==id){printf "%d %8.3f %8.3f %8.3f\n",id,$4,$5,$6}}' id=$k $outroot/${relnam}_out/${relnam}_CMET_clusters.dat >> \
			$outroot/${relnam}_out/${relnam}.surface
	done
	#--| remove unnecessary files
	mv $outroot/${relnam}_out/${relnam}_CMET_summary.dat $outroot/${relnam}_out/${relnam}.summary
	mv $outroot/${relnam}_out/${relnam}_CMET_predicted.dat $outroot/${relnam}_out/${relnam}.residue
	rm -f $outroot/${relnam}_out/${relnam}_CMET*
	rm -f $outroot/${relnam}_out/${relnam}.easymifs
	rm -f $outroot/${relnam}_out/${relnam}.top
done
rm -rf $tmpdir

#===== exit =====#
exit 0

