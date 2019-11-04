# PointSite: a point cloud segmentation tool for identification of protein ligand binding atoms

This is the assement part of [PointSite](XXX) on Python 3.6, Pytorch 1.1.0. The model generates binding atoms segmentation masks for each query protein. It's based on Submanifold Sparse Convolutional (SSC) based U-Net.

# Installation of Comparison Methods
### 1. [p2rank](https://github.com/rdk/p2rank)
To install p2rank, we need install [gradle](https://gradle.org/install/)
Also, 
```sh 
export PATH=$PATH:/opt/gradle/gradle-5.6.2/bin
```

### 2. [fpocket](https://github.com/Discngine/fpocket)
To install fpocket, we need the molfile plugin from VMD:
```sh 
sudo apt-get install libnetcdf-dev
```
or,
```sh 
sudo yum install netcdf-devel.x86_64
```
### 3. [Sitebound](http://scbx.mssm.edu/sitehound/sitehound-download/download.html)
To succesfully run sitehound, we need install A LOT OF 32-bit libraries, such as libX11.so.6 and libSM.so.6 !!!
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


# Run of Comparison Methods

It provides the details for the inference part of all comparison methods, which consists of five parts.
* Run each method for a given dataset
* Run assessment for each method
* Assess PointSite's IoU
* PointSite assisted merged results of each method

