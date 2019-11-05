# PointSite: a point cloud segmentation tool for identification of protein ligand binding atoms

This is the assement part of [PointSite](https://github.com/PointSite/PointSite_Inference) on Python 3.6, Pytorch 1.1.0. The model generates binding atoms segmentation masks for each query protein. It's based on Submanifold Sparse Convolutional (SSC) based U-Net.![avatar](https://github.com/PointSite/PointSite_Assessment/tree/master/programs/sparseconv.png)

## Run and Assessment of Comparison Methods

It provides the details for the inference part of all comparison methods, which consists of four parts in `programs` folder:
* Run each method for a given dataset (not recommend to run, as ALL results have been generated in `testset_result` folder)
```sh
bash run_all.sh
```
* Run assessment for each method
```sh
bash assess_all.sh
```
* Assess PointSite's IoU
```sh
bash pointsite_iou.sh
```
* PointSite assisted merged results of each method
```sh
bash pointsite_merge.sh
```
Note that by default, all previous scripts will assess `blind` dataset. To assess the other six test datasets, simply type `export Assess_Switch=1` and re-run the corresponding script.

To change the root directory of `PointSite`, simply type `export PointSite_HOME=<the folder contains PointSite_Assessment and PointSite_TestData>`.



## Installation of Comparison Methods
### 1. [p2rank](https://github.com/rdk/p2rank)
To install and compile p2rank, we need to install [gradle](https://gradle.org/install/). Also, 
```sh 
export PATH=$PATH:/opt/gradle/gradle-5.6.2/bin
```

### 2. [fpocket](https://github.com/Discngine/fpocket)
To install fpocket, we may need the molfile plugin from VMD:
```sh 
sudo apt-get install libnetcdf-dev
```
or,
```sh 
sudo yum install netcdf-devel.x86_64
```
### 3. [Sitebound](http://scbx.mssm.edu/sitehound/sitehound-download/download.html)
To succesfully run sitehound, we need to install A LOT OF 32-bit libraries, such as libX11.so.6 and libSM.so.6 !!!
In order to install them, please first to install the 64-bit version libraries by the following command:
```sh
sudo yum install libSM libX11
```
Then, use yum provides to search for the required 32-bit libraries of those missing files, for example, `yum provides libX11.so.6`. It shall come with the required libraries as follows: `libX11-1.6.7-2.el7.i686`

Then, install this library by typing 
```sh
sudo yum install libX11.i686
```

[bugs]:
It is well known that sitehound will report a bug if the input PDB contains ANY missing atoms which is very common.
Thus, to handle such situations, we have to "reconstruct" the PDB with the following module:
```sh
BuildModel_Package/util/Complete_PDB.sh 5ow0A_xyz.pdb /tmp/5ow0A_xyz.pdb
```
### 4. Selenium
This is the package for command-line based Web Server submission for `deepsite` and `metapocket2`


