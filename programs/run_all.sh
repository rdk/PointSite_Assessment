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


#============== Part I: run each method for a given dataset ==============#
#-- parameter setting
cur_root=`pwd`
cpunum=32
threadnum=8
#buildmod=/home/wangs0c/WS_Program/RaptorX-Threading/BuildModel_Package
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


#============= run all test sets ====================#
#[note]: we highly unrecommend the users to run ALL test sets. 
#        Please just run blind dataset by setting Assess_Switch to 0

#--- for each dataset ----#
for i in `cat dataset_list`
do

#-> get data name
orig_data=`echo $i | cut -d '|' -f 1`
proc_data=`echo $i | cut -d '|' -f 2`
echo "#=================== data $orig_data ==================#"
#-> define input
suffix=${proc_data}_data
list=$root_input/${orig_data}_data/data_list
gt_dir=$root_input/${orig_data}_data/data
awk '{print $0"_xyz.pdb"}' $list > $gt_dir/$suffix
#-> define output
pred_dir=$root_output/${proc_data}_out
mkdir -p $pred_dir


#------------ run comparison methods ----------#
echo "#-> 1. p2rank"
./p2rank_run.sh $suffix $gt_dir $pred_dir $cpunum $cur_root/p2rank

echo "#-> 2. fpocket"
./fpocket_run.sh $suffix $gt_dir $pred_dir $cpunum $cur_root/fpocket $cur_root/util

echo "#-> 3. sitehound"
./sitehound_run.sh $suffix $gt_dir $pred_dir $cpunum $cur_root/sitehound $buildmod $cur_root/util
#[note]: neglect those "errors" as sitehound can't deal with non-standard amino acids.

#-> 4. deepsite (not recommend to run, as it will submit the jobs to the website and need 300 to 500 seconds for each job!!)
#               (also, the maximal length for deepsite shall below 1000 amino acids!!)
./deepsite_run.sh $suffix $gt_dir $pred_dir $threadnum $cur_root/deepsite

#-> 5. metapocket2 (not recommend to run, as it will submit the jobs to the website and need 300 to 500 seconds for each job!!)
./metapocket2_run.sh $suffix $gt_dir $pred_dir $threadnum $cur_root/metapocket2

#-> TER. remove the created data file
rm -f $gt_dir/$suffix

done


