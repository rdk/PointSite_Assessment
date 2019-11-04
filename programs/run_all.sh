#============== global variables defined here ========= # start
declare PointSite_HOME           #-> root directory
if [ -z "${PointSite_HOME}" ]
then
	#echo "PointSite_HOME not set. Use default value '~/GitBucket'"
	PointSite_HOME=~/GitBucket
fi


#============== Part I: run each method for a given dataset ==============#
#-> 0. initialization
#-- parameter setting
cur_root=`pwd`
cpunum=32
threadnum=8
#buildmod=/home/wangs0c/WS_Program/RaptorX-Threading/BuildModel_Package


#============= run simple test =====================#
#--- root input/output ----#
root_input=`pwd`/example
root_output=`pwd`/example

#-> get data name
orig_data=blind
proc_data=blind

#-> define input
suffix=${proc_data}_data
list=$root_input/${proc_data}_list
gt_dir=$root_input/${proc_data}
awk '{print $0"_xyz.pdb"}' $list > $gt_dir/$suffix

#-> define output
pred_dir=$root_output/${proc_data}_out
mkdir -p $pred_dir


#============= run all test sets ====================#
#[note]: we highly unrecommend the users to run ALL test sets in the following way.

#--- root input/output ---#
#root_input=$PointSite_HOME/PointSite_TestData/
#root_output=$PointSite_HOME/PointSite_Assessment/testset_result/

#--- for each dataset ----#
#for i in `cat dataset_list`
#do

#-> get data name
#orig_data=`echo $i | cut -d '|' -f 1`
#proc_data=`echo $i | cut -d '|' -f 2`
echo "#=================== data $orig_data ==================#"

#-> define input
#suffix=${proc_data}_data
#list=$root_input/${orig_data}_data/data_list
#gt_dir=$root_input/${orig_data}_data/data
#awk '{print $0"_xyz.pdb"}' $list > $gt_dir/$suffix

#-> define output
#pred_dir=$root_output/${proc_data}_out
#mkdir -p $pred_dir


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
#./deepsite_run.sh $suffix $gt_dir $pred_dir $threadnum $cur_root/deepsite

#-> 5. metapocket2 (not recommend to run, as it will submit the jobs to the website and need 300 to 500 seconds for each job!!)
#./metapocket2_run.sh $suffix $gt_dir $pred_dir $threadnum $cur_root/metapocket2

#-> TER. remove the created data file
rm -f $gt_dir/$suffix

#done


