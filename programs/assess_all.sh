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


#============== Part II: run assessment for each method ===============#
#-- parameter setting
cur_root=`pwd`
cpunum=32
cutoff_truth=6.5
cutoff_pred=6.5
method_list=`pwd`/method_list
#--- output directory ----#
outdir=/tmp/tmp_assess
mkdir -p $outdir
#---- use SIMPLE or ALL switch -----#
if [ $Assess_Switch -eq 1 ]
then
	dataset_list_wrapper=dataset_list
	root_input=$PointSite_HOME/PointSite_TestData/
	root_output=$PointSite_HOME/PointSite_Assessment/testset_result/
else
	orig_data=blind
	echo "$orig_data|$orig_data" > blind_dataset
	dataset_list_wrapper=blind_dataset
	root_input=$PointSite_HOME/PointSite_Assessment/programs/example/
	root_output=$PointSite_HOME/PointSite_Assessment/programs/example/
fi


#=================================== run assessment ======================#
#--- for each cutoff_pred
for cutoff_pred in 4.5 5.5 6.5
do
echo "#++++++++++++++++++++ cutoff_pred $cutoff_pred +++++++++++++++#"

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


#------------ run assessment for each method ----------#
rm -f assess_proc
for method in `cat $method_list`;
do
	mkdir -p $outdir/${method}_${suffix}
	#for param in 4.0 4.5 5.0 5.5 6.0 6.5 7.0 7.5 8.0 8.5 9.0 9.5 10.0
	for param in 4.0
	do
		for i in `cat $list`;
		do
			echo "$cur_root/util/LigBind_Assess $gt_dir/${i}_atom.xyz $gt_dir/${i}_lig.xyz $pred_dir/${method}_${suffix}/${i}_xyz_out/${i}_xyz.surface $cutoff_truth $cutoff_pred $param > $outdir/${method}_${suffix}/${i}.assess_$param" >> assess_proc
		done
	done
done
$cur_root/util/distribute_bash.sh assess_proc $cpunum $cur_root
rm -f assess_proc


#------------ collect results ---------------#
for method in `cat $method_list`;
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
		awk 'BEGIN{a=0;b=0;c=0;d=0;e=0;f=0;g=0;h=0;z=0;y=0;}{if(NF==26){a+=$4;b+=$6;c+=$8;d+=$12;e+=$14;f+=$19;g+=$21;z+=$24;y+=$26;h++}}END{print "DCA "a/c" | atom-IoU "f/h}' $outdir/${method}_${suffix}.assess_$param
	done
done

done

done

#--- remove temp data ----#
if [ $Assess_Switch -eq 0 ]
then
	rm -f blind_dataset
fi


