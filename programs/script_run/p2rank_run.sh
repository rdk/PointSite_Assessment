#!/bin/bash

if [ $# -lt 5 ]
then
	echo "Usage: ./p2rank_run.sh <data> <indir> <outdir> <cpunum> <home> "
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

#----- run p2rank --------#
#-- run p2rank in a batch
$home/distro/prank predict $data -dataset_base_dir $indir -output_base_dir $outdir

#-- extract surface point
rm -rf $outdir/p2rank_$data
mv $outdir/predict_$data $outdir/p2rank_$data
for i in `cat $indir/$data`
do
	#--| get name
	fulnam=`basename $i`
	relnam=${fulnam%.*}
	#--| uncompress
	gunzip $outdir/p2rank_$data/visualizations/data/${i}_points.pdb.gz
	#--| extract
	num=`wc $outdir/p2rank_$data/${i}_predictions.csv | awk '{print $1-1}'`
	rm -f $outdir/p2rank_$data/${i}_surface.xyz
	for ((k=1;k<=num;k++))
	do
		#--> put predicted center as the first
		tail -n+2 $outdir/p2rank_$data/${i}_predictions.csv | head -n $k | tail -n1 | \
			awk -F"," '{printf "%d %8.3f %8.3f %8.3f\n",id,$6,$7,$8}' id=$k >> $outdir/p2rank_$data/${relnam}.surface
		#--> output others
		cat $outdir/p2rank_$data/visualizations/data/${i}_points.pdb | awk '{a=substr($0,7);print a}' | \
			awk '{if($5==id){x=substr($0,25,8);y=substr($0,33,8);z=substr($0,41,8); print id" "x" "y" "z }}' id=$k \
			>> $outdir/p2rank_$data/${relnam}.surface
	done
	#--| move results
	mkdir -p $outdir/p2rank_$data/${relnam}_out
	mv $outdir/p2rank_$data/${i}_predictions.csv $outdir/p2rank_$data/${relnam}.surface $outdir/p2rank_$data/${relnam}_out
done
#-- remove
rm -rf $outdir/p2rank_$data/visualizations
rm -f $outdir/p2rank_$data/run.log
rm -f $outdir/p2rank_$data/params.txt


#===== exit =====#
exit 0

