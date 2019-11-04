####### define the main_root of PointSite here !!!!! ########
#PointSite_ROOT=/home/wangsheng/GitBucket/
#============== global variables defined here ========= # start
declare PointSite_HOME           #-> root directory
if [ -z "${PointSite_HOME}" ]
then
	#echo "PointSite_HOME not set. Use default value '~/GitBucket'"
	PointSite_HOME=~/GitBucket
fi
#--- assess switch: run simple or all ----#
declare Assess_Switch
if [ -z "${Assess_Switch}" ]
then
	Assess_Switch=0          #-> by default, we only assess 'blind'
fi


#============== Part IV: use PointSite to help each method ===============#
#-- parameter setting
cur_root=`pwd`
cpunum=32
dist_thres=6.5
ratio_thres=0.1
method_list=`pwd`/method_list
#--- output directory ----#
outdir=/tmp/tmp_pointsite
mkdir -p $outdir
#---- use SIMPLE or ALL switch -----#
if [ $Assess_Switch -eq 1 ]         #-> ALL test datasets
then
	dataset_list_wrapper=dataset_list
	root_input=$PointSite_HOME/PointSite_TestData/
	root_output=$PointSite_HOME/PointSite_Assessment/testset_result/
else                                #-> blind dataset
	dataset_list_wrapper=blind_dataset
	root_input=$PointSite_HOME/PointSite_Assessment/programs/example/
	root_output=$PointSite_HOME/PointSite_Assessment/programs/example/
fi


#=================== PointSite assisted merged results ======================#
#--- for dist_thres ----#
for dist_thres in 4.5 5.5 6.5
do
echo "#++++++++++++++++++++ dist_thres $dist_thres +++++++++++++++#"

for ratio_thres in 0.1 0.2 0.3
do
echo "#||||||||||||||||||||| ratio_thres $ratio_thres |||||||||||||#"


#--- for each dataset ----#
for i in `cat $dataset_list_wrapper`
do

#-> get data name
orig_data=`echo $i | cut -d '|' -f 1`
proc_data=`echo $i | cut -d '|' -f 2`
echo "#=================== data $orig_data ==================#"
#-> define input
suffix=${proc_data}_data
list=$root_input/${orig_data}_data/data_list
gt_dir=$root_input/${orig_data}_data/data
pred_dir=$root_output/${proc_data}_out
pt_dir=$pred_dir/pointsite_$suffix


#------------ run PointSite_Merge for each method ----------#
rm -f ptsave_proc
for method in `cat $method_list`; 
#for method in sitehound
do 
	mkdir -p $outdir/${method}_${suffix}
	#for param in 4.0 4.5 5.0 5.5 6.0 6.5 7.0 7.5 8.0 8.5 9.0 9.5 10.0
	for param in 4.0
	do
		for i in `cat $list`; 
		do 
			echo "$cur_root/util/PointSite_Merge $pt_dir/${i}_xyz_out/${i}_atom.xyz $gt_dir/${i}_lig.xyz $pred_dir/${method}_${suffix}/${i}_xyz_out/${i}_xyz.surface $dist_thres $ratio_thres $param 1> $outdir/${method}_${suffix}/${i}.assess_$param 2> $outdir/${method}_${suffix}/${i}.result_$param " >> ptsave_proc
		done
	done
done
$cur_root/util/distribute_bash.sh ptsave_proc $cpunum $cur_root
rm -f ptsave_proc


#------------ collect results ---------------#
for method in `cat $method_list`;
#for method in sitehound
do
	echo "#----------- method: $method ------------#"
	#for param in 4.0 4.5 5.0 5.5 6.0 6.5 7.0 7.5 8.0 8.5 9.0 9.5 10.0
	for param in 4.0
	do
		echo "#-> param: $param"
		rm -f $outdir/${method}_${suffix}.assess_$param
		for i in `cat $list`;
		do
			reso=`head -n1 $outdir/${method}_${suffix}/${i}.assess_$param`
			if [ "$reso" != "" ]
			then
				echo "$i $reso" >> $outdir/${method}_${suffix}.assess_$param
			fi
		done
		#awk 'BEGIN{a=0;b=0;c=0;d=0;e=0;f=0;g=0;h=0;z=0;y=0;}{if(NF==26){a+=$4;b+=$6;c+=$8;d+=$12;e+=$14;f+=$19;g+=$21;z+=$24;y+=$26;h++}}END{print a/c" "b/c" "d/c" "e/c" "f/h" "g/h" "c" "h" "z/h" "y/h}' $outdir/${method}_${suffix}.assess_$param
		awk 'BEGIN{a=0;b=0;c=0;d=0;e=0;f=0;g=0;h=0;z=0;y=0;}{if(NF==26){a+=$4;b+=$6;c+=$8;d+=$12;e+=$14;f+=$19;g+=$21;z+=$24;y+=$26;h++}}END{print "DCA "a/c" | atom-IoU "f/h" | ligand_num "c" | protein_num "h}' $outdir/${method}_${suffix}.assess_$param
	done
done

done

done

done


