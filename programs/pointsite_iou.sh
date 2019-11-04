####### define the main_root of PointSite here !!!!! ########
#PointSite_HOME=/home/wangsheng/GitBucket/
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


#============== Part III: assess PointSite's IoU =========================#
#-- parameter setting
cur_root=`pwd`
cpunum=32
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

#==================== assess atom-IoU of PointSite ======================#
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

#------------ run PointSite_IoU for each method ----------#
rm -f ptassess_proc
#for method in `cat $method_list`;
for method in pointsite
do
	mkdir -p $outdir/${method}_${suffix}
	for i in `cat $list`;
	do
		echo "$cur_root/util/PointSite_IoU $gt_dir/${i}_atom.xyz $pt_dir/${i}_xyz_out/${i}_atom.xyz > $outdir/${method}_${suffix}/${i}.assess_iou" >> ptassess_proc
	done
done
$cur_root/util/distribute_bash.sh ptassess_proc $cpunum $cur_root
rm -f ptassess_proc


#------------ collect results ---------------#
#for method in `cat $method_list`;
for method in pointsite
do
	echo "#----------- method: $method ------------#"
	rm -f $outdir/${method}_${suffix}.assess_iou
	for i in `cat $list`;
	do
		reso=`head -n1 $outdir/${method}_${suffix}/${i}.assess_iou`
		if [ "$reso" != "" ]
		then
			echo "$i $reso" >> $outdir/${method}_${suffix}.assess_iou
		fi
	done
	awk 'BEGIN{a=0;b=0;c=0;}{if(NF==5){a+=$3;b+=$5;c++;}}END{print "atom-IoU "a/c" | protein_num "c}' $outdir/${method}_${suffix}.assess_iou
done

done


