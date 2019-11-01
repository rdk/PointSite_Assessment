#!/bin/bash

#------- usage ------------#
if [ $# -lt 3 ]
then
	echo "Usage: ./distribute_bash.sh <proc> <cpu_num> <out_root> "
	echo "       <proc> is the running command for process "
	echo "       <cpu_num> is the thread number. Set -1 for using ALL available CPUs "
	echo "       <out_root> is the output directory "
	exit 1
fi

#------ get arguments ----------#
proc=$1
cpu_num=$2
out_root=$3

#------ detect CPU numbers -----#
if [ $cpu_num -le 0 ]
then
	cpu_num=`nproc`
fi

#------ get input name -----#
fulnam=`basename $proc`

#------ create tmporary folder ----#
DATE=`date '+%Y_%m_%d_%H_%M_%S'`
relnam=${fulnam}_${RANDOM}_${DATE}
tmp_root="/tmp/TMP_DISTRI_${relnam}"
mkdir -p $tmp_root

#------ run BASH distribute -----#
split -da 8 -n r/$cpu_num $proc $tmp_root/${relnam}.
for ((i=0;i<$cpu_num;i++))
do
	suffix=`echo $i | awk '{printf("%08d\n",$0)}'`
	mkdir -p $tmp_root/${relnam}_${suffix}
	mv $tmp_root/${relnam}.${suffix} $tmp_root/${relnam}_${suffix}
	screen -S ${relnam}_$i -d -m bash -c "cd $tmp_root/${relnam}_${suffix};bash ${relnam}.${suffix}"
done

#------ check termination -----#
while true
do
	remains=`screen -ls | grep ${relnam} | wc | awk '{print $1}'`
	echo "remains: $remains/$cpu_num"
	if [ $remains -eq 0 ]
	then
		break
	fi
	sleep 10s
done

#------ collect output files ----#
mkdir -p $out_root
for ((i=0;i<$cpu_num;i++))
do
	suffix=`echo $i | awk '{printf("%08d\n",$0)}'`
	rm -f $tmp_root/${relnam}_${suffix}/${relnam}.${suffix}
	mv $tmp_root/${relnam}_${suffix}/* $out_root 2>/dev/null
done

#========== exit =============#
rm -rf $tmp_root
exit 0

