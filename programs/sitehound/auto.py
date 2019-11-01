#!/usr/bin/python

################################################################################
# auto.py                                                                      #
# Author:  Dario Ghersi                                                        #
# Version: 011210                                                              #
# Goal:    The script carries out automatically all the steps for binding site #
#          identification (preparing the PDB file and calling 'EasyMIFs' and   #
#          'SiteHound')                                                        #
#                                                                              #
# Revisions:                                                                   #
# 011209    added the option for using compressed maps                         #
################################################################################

import getopt
import math
import sys
import os
import re
import shutil
import subprocess
import time

################################################################################
# CONSTANTS                                                                    #
################################################################################

winFlag = False # must be True if run on Windows

################################################################################
# CLASSES                                                                      #
################################################################################

class Parameters:
  def __init__(self, hashPars):
    ## force field
    self.ff = "G43b1" # GROMOS in vacuo force field

    ## name of the pdb file to process
    self.pdbName = hashPars["pdbName"]

    ## pdbStem
    pdbName = hashPars["pdbName"]
    if not (pdbName[-4:] == ".pdb" or pdbName[-4:] == ".ent"):
      print "The PDB file should have extension .pdb or .ent"
      sys.exit(1)
    pdbStem = ".".join(pdbName.split(".")[:-1])
    self.pdbStem = pdbStem
    
    ## probe type (EasyMIFs)
    self.probeType = hashPars["probeType"]

    ## compressed flag
    if hashPars.has_key("compressed"):
      self.compressed = True
    else:
      self.compressed = False

    ## remove the HETATMs from the PDB file
    if hashPars.has_key("stripHET"):
      self.stripHET = True
    else:
      self.stripHET = False

    ## spacing (resolution) of the grid (EasyMIFs)
    if hashPars.has_key("spacing"):
      self.spacing = hashPars["spacing"]
    else:
      self.spacing = '1.0' # default grid spacing

    ## grid center (EasyMIFs)
    if hashPars.has_key("center"):
      self.center = hashPars["center"]
    else:
      self.center = "default"

    ## grid dimensions (EasyMIFs)
    if hashPars.has_key("dimension"):
      self.dimension = hashPars["dimension"]
    else:
      self.dimension = "default"

    ## linkage (SiteHound)
    if hashPars.has_key("linkage"):
      self.linkage = hashPars["linkage"]
    else:
      self.linkage = "average"

    ## energy cutoff (SiteHound)
    if hashPars.has_key("energyCutoff"):
      self.energyCutoff = hashPars["energyCutoff"]
    else:
      if self.probeType == "CMET":
        self.energyCutoff = "-8.9"
      if self.probeType == "OP":
        self.energyCutoff = "-8.5"

    ## spatial cutoff (SiteHound)
    if hashPars.has_key("spatialCutoff"):
      self.spatialCutoff = hashPars["spatialCutoff"]
    else:
      if self.linkage == "single":
        self.spatialCutoff = "1.1"
      if self.linkage == "average":
        self.spatialCutoff = "7.8"

    ## log
    if hashPars.has_key("log"):
      self.log = True
    else:
      self.log = False

    ## pdb2gmx log
    if hashPars.has_key("pdb2gmx"):
      self.pdb2gmx = True
    else:
      self.pdb2gmx = False

################################################################################
# FUNCTIONS                                                                    #
################################################################################

def usage():
  """
  print a message with a brief description of each option
  """
  print "Usage: auto.py -i PDB -p PROBE [-r RESOLUTION -c X,Y,Z -d DX,DY,DX] [-e ENERGY_CUTOFF -l LINKAGE -s SPATIAL_CUTOFF]\n"
  print "-i --input\tThe pdb file"
  print "-p --probe\tThe probe type, either CMET (methyl) or OP (phosphate oxygen)"
  print "-k --clean\tRemove HETATM from the pdb"
  print "-r --resolution\tThe resolution of the grid in Angstrom (default: 1.0)"
  print "-c --center\tThe center of the grid (default: center of the protein)"
  print "-d --dimension\tThe dimensions of the box (default: whole protein)"
  print "-e --energy\t(default: -8.9 for CMET, -8.5 for OP)"
  print "-l --linkage\tEither 'single' or 'average' (default: 'average')"
  print "-s --spatial\tThe spatial cutoff (default: 1.1 for single linkage, 7.8 for average)"
  print "-x --pdb2gmx\tPrint the output of pdb2gmx"
  print "-o --log\tSend the output messages to a log file"
  print "-z --compressed\tUse compressed maps"
  print "-h --help\tShow this help\n"
  print "Example: auto.py --input=protein.pdb --probe=CMET --center=1.0,1.0,1.0\nor"
  print "         auto.py -i protein.pdb -p CMET -c 1.0,1.0,1.0\n"

################################################################################

def parseParameters(argv):
  """
  process the list of command-line arguments and returns an object 'Parameters'
  """
  pars = {} # the hash that will contain all the parameters
  try:
    opts, args = getopt.getopt(argv, "hzi:p:kr:c:d:e:l:s:xo", ["help", "compressed", "input=", "probe=", "clean", "resolution=", "center=", "dimension=", "energy=", "linkage=", "spatial=", "log", "pdb2gmx"])
  except getopt.GetoptError:
    usage()
    sys.exit(2)
  for opt, arg in opts:
    if opt in ("-h", "--help"): # help message
      usage()
      sys.exit()
    elif opt in ("-z", "--compressed"): # use compressed maps
      pars["compressed"] = True
    elif opt in ("-i", "--input"): # input file
      pars["pdbName"] = arg
    elif opt in ("-p", "--probe"): # probe type
      pars["probeType"] = arg
    elif opt in ("-k", "--clean"): # clean the PDB (remove HETATM)
      pars["stripHET"] = True
    elif opt in ("-r", "--resolution"): # resolution
      pars["spacing"] = arg
    elif opt in ("-c", "--center"): # center of the grid
      pars["center"] = arg
    elif opt in ("-d", "--dimension"): # dimension of the box
      pars["dimension"] = arg
    elif opt in ("-e", "--energy"): # energy cutoff
      pars["energyCutoff"] = arg
    elif opt in ("-l", "--linkage"): # linkage type
      pars["linkage"] = arg
    elif opt in ("-s", "--spatial"): # spatial cutoff
      pars["spatialCutoff"] = arg
    elif opt in ("-o", "--log"): # save the output messages to file
      pars["log"] = True
    elif opt in ("-x", "--pdb2gmx"): # print the output of pdb2gmx
      pars["pdb2gmx"] = True

  ## sanity checks
  if not pars.has_key("pdbName"): # pdb name must be supplied
    usage()
    sys.exit(2)

  if not pars.has_key("probeType"): # probe type must be supplied
    usage()
    sys.exit(2)
  if pars.has_key("probeType"): #
     if not pars["probeType"] in ("CMET", "OP"):
       print "Probe type must be either 'CMET' or 'OP'"
       sys.exit(2)

  if pars.has_key("spacing"): # grid resolution must be a number
    try:
      float(pars["spacing"])
    except:
      print "Grid resolution must be a number"
      sys.exit(2)

  if pars.has_key("center"): # grid center must be properly formatted
    try:
      center = map(float, pars["center"].split(","))
    except:
      print "Center must be specified as: X,Y,Z"
      sys.exit(2)
    if len(center) != 3:
      print "Center must be specified as: X,Y,Z"
      sys.exit(2)

  if pars.has_key("dimension"): # grid dimension must be properly formatted
    try:
      dimension = map(float, pars["dimension"].split(","))
    except:
      print "Dimensions must be specified as: NX,NY,NZ"
      sys.exit(2)
    if len(dimension) != 3:
      print "Dimensions must be specified as: NX,NY,NZ"
      sys.exit(2)

  if pars.has_key("energyCutoff"): # energy cutoff must be a negative number
    try:
      energy = float(pars["energyCutoff"])
    except:
      print "Energy cutoff must be a negative number"
      sys.exit(2)
    if energy > 0:
      print "Energy cutoff must be a negative number"
      sys.exit(2)
    
  if pars.has_key("linkage"): # linkage must be either 'single' or 'average'
    if not pars["linkage"] in ("single", "average"):
      print "Linkage must be either 'single' or 'average'"
      sys.exit(2)

  if pars.has_key("spatialCutoff"): # spatial cutoff must be a positive number
    try:
      spatial = float(pars["spatialCutoff"])
    except:
      print "Spatial cutoff must be a positive number"
      sys.exit(2)
    if spatial < 0:
      print "Spatial cutoff must be a positive number"
      sys.exit(2)

  ## create a parameters object
  params = Parameters(pars)
  pathname = os.path.dirname(sys.argv[0])
  ## set the gmxlib directory for GROMACS
  params.gmxlib = pathname + os.sep + "EasyMIFs" + os.sep + "pdb2gmx"

  return params


################################################################################

def preparePdb(params):
  """
  call 'pdb2gmx' (part of the GROMACS suite) and process its output to prepare
  an input file for EasyMIFs
  """
  
  ## call the pdb2gmx program
  pdbName = params.pdbName
  if not os.path.isfile(pdbName):
    print "PDB %s not found" % pdbName
    if pars.log:
      logFile.write("PDB %s not found\n" % pdbName)
    sys.exit(2)
  pdbStem = params.pdbStem
  os.environ['GMXLIB'] = params.gmxlib
  pdb2gmx = params.gmxlib + os.sep + "pdb2gmx"
  ff = params.ff

  ## read the PDB file
  pdb = open(pdbName, "r")
  if params.stripHET: # strip the HETATMS
    strippedPdb = open(pdbStem + "_stripped.pdb", "w")
  for line in pdb:
    if line[:6] != "HETATM" and params.stripHET: 
        strippedPdb.write(line)
    elif line[:6] == "HETATM" and not params.stripHET:
      if params.log:
        logFile.write("HETATM record found in the PDB file...\nPlease consider using the --clean option or manually removing the HETATM records\n")
      else:
        print "HETATM record found in the PDB file...\nPlease consider using the --clean option or manually remove the HETATM records\n"
      pdb.close()
      sys.exit(2)
  
  if params.stripHET:
    strippedPdb.close()
    pdbName = pdbStem + "_stripped.pdb" # make pdb2gmx read this file
  pdb.close()
    
  ## launch pdb2gmx
  commandLine = "\"" + pdb2gmx + "\"" + " -ignh -merge -f " + pdbName + \
                " -ff " +  ff + " -o " + pdbStem + "_temp.pdb -p " + \
                pdbStem + ".top"
  
  if int(sys.version[0]) > 2 or (int(sys.version[0]) == 2 and int(sys.version[2]) >= 5):
    p = subprocess.Popen(commandLine, shell=True, env={'GMXLIB': params.gmxlib}, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    message = p[0] + "\n" + p[1]
  else:
    p = os.popen4(commandLine)
    message = p[1].readlines()
    message = "".join(message)
  message = message.replace("\r", " ")
  
  if params.pdb2gmx:
    if params.log:
      logFile.write(message)
    else:
      print message

  ## read both files storing the necessary information
  grofileName = pdbStem + "_temp.pdb"
  if not os.path.isfile(grofileName):
    if pars.log:
      logFile.write("Problems preparing the PDB file...please check if there are missing residues or missing atoms\n")
      logFile.close()
    else:
      print "Problems preparing the PDB file...please check if there are missing residues or missing atoms\n"
    sys.exit(2)
  grofile = open(grofileName, "r")
  data = grofile.readlines()
  grofile.close()
  grodata = []
  for line in data: # skip the first two lines of the file and the last one
    if line[:4] == "ATOM":
      grodata.append(line)

  topfileName = pdbStem + ".top"  
  topfile = open(topfileName, "r")
  data = topfile.readlines()
  topfile.close()
  topdata = []
  flag = 0 # flag to signal the beginning of the atom section
  for line in data:
    if flag and line == "\n": # end of the atom section
      break
    if flag: # atom section
      fields = line.split()
      atom_type, charge = fields[1], fields[6]
      topdata.append([atom_type, charge])
    if line.find("atom") != -1 and line.find("type") != -1 and \
       line.find("charge") != -1:
      flag = True
      continue

  ## merge the two files into a new one
  numOfAtoms = len(grodata)
  if numOfAtoms != len(topdata):
    print "GROFILE and TOPFILE contain a different number of atoms"
    sys.exit(1)
  outfileName = pdbStem + ".easymifs"
  outfile = open(outfileName, "w")
  for i in range(numOfAtoms):
    # prepare the line according to the PDB specification
    line = grodata[i][:12] + topdata[i][0] + " " * (4 - len(topdata[i][0])) + \
           grodata[i][16:55] + topdata[i][1]
    outfile.write(line + "\n") 
  outfile.close()

  ## remove temporary files
  os.remove(pdbStem + "_temp.pdb")
  os.remove("posre.itp")
  os.remove(pdbStem + ".top")
  if os.path.isfile(pdbStem + "_stripped.pdb"):
    os.remove(pdbStem + "_stripped.pdb")

################################################################################

def launchEasyMIFs(pars, root):
  """
  launch EasyMIFs with the specified parameters
  """
  infile = "-f=" + root + ".easymifs"
  probeType = "-p=" + pars.probeType
  resolution  = "-r=" + pars.spacing
  if pars.center == "default":
    center = ""
  else:
    center = "-c=" + pars.center
  if pars.dimension == "default":
    dimension = ""
  else:
    dimension = "-n=" + pars.dimension

  ## check if available (copy if not) 'atom_types.txt' and 'ffG43b1nb.params'  
  if not os.path.isfile("atom_types.txt"):
    rootDir = os.path.dirname(sys.argv[0])
    shutil.copy(rootDir + "/EasyMIFs/atom_types.txt", "atom_types.txt")
  if not os.path.isfile("ffG43b1nb.params"):
    shutil.copy(rootDir + "/EasyMIFs/ffG43b1nb.params", "ffG43b1nb.params")

  ## start the calculations with EasyMIFs
  if pars.log:
    logFile.write("Running EasyMIFs...\n")
  else:
    print "Running EasyMIFs..."
  pathname = os.path.dirname(sys.argv[0])
  if winFlag:
    commandLine = "\"" + pathname  + "/EasyMIFs/easyMIFs.exe \" " + \
                  infile + " " + probeType + " " + resolution + " " + \
                  center + " " + dimension
  else:
    commandLine = pathname  + "/EasyMIFs/easyMIFs " + \
                  infile + " " + probeType + " " + resolution + " " + \
                  center + " " + dimension
  if pars.compressed:
    commandLine += " -z"
  commandLine = os.path.normpath(commandLine)
  
  if int(sys.version[0]) > 2 or (int(sys.version[0]) == 2 and int(sys.version[2]) >= 5):
    p = subprocess.Popen(commandLine, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    message = p[0] + "\n" + p[1]
  else:
    p = os.popen4(commandLine)
    message = p[1].readlines()
    message = "".join(message)
  if pars.log:
    logFile.write(message)
  else:
    print message

################################################################################

def launchSiteHound(pars, root):
  """
  launch SiteHound with the specified parameters
  """
  pathname = os.path.dirname(sys.argv[0])
  if pars.compressed:
    infile = "-f=" + root + "_" + pars.probeType + ".cmp"
  else:
    infile = "-f=" + root + "_" + pars.probeType + ".dx"
  linkage = "-l=" + pars.linkage
  energy = "-e=" + pars.energyCutoff
  spatial = "-s=" + pars.spatialCutoff
  if winFlag:
    commandLine = "\"" + pathname + "/SiteHound/sitehound.exe \" " + infile + \
                  " -t=easymifs " + linkage + " " + spatial + " " + energy
  else:
    commandLine = pathname + "/SiteHound/sitehound " + infile + \
                  " -t=easymifs " + linkage + " " + spatial + " " + energy
  
  if pars.log:
    logFile.write("Running SiteHound...\n")
  else:
    print "Running SiteHound..."

  commandLine = os.path.normpath(commandLine)
  if int(sys.version[0]) > 2 or (int(sys.version[0]) == 2 and int(sys.version[2]) >= 5):
    p = subprocess.Popen(commandLine, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    message = p[0] + "\n" + p[1]
  else:
    p = os.popen4(commandLine)
    message = p[1].readlines()
    message = "".join(message)

  if pars.log:
    logFile.write(message)
  else:
    print message

################################################################################
# MAIN PROGRAM                                                                 #
################################################################################

## parse the parameters
pars = parseParameters(sys.argv[1:])

#print pars.ff, pars.pdbName, pars.probeType, pars.spacing, pars.center, pars.dimension, pars.linkage, pars.energyCutoff, pars.spatialCutoff, pars.gmxlib

## open log file if flag is true
if pars.log:
  logFile = open(pars.pdbStem + ".log", "w")

## print the header
header =  "**********************************************************\n"
header += "* Welcome to EasyMIFs_SiteHound                          *\n"
header += "* an automatic pipeline for binding site identification  *\n"
header += "* Version: 011209                                        *\n"
header += "**********************************************************"
if pars.log:
  logFile.write(header + "\n")
  logFile.write(time.strftime("%Y-%m-%d %H:%M:%S") + "\n\n")
else:
  print header + "\n"
  print time.strftime("%Y-%m-%d %H:%M:%S" + "\n")

## checkpoint
t1 = time.time()

## prepare the PDB file
preparePdb(pars)

## checkpoint
t2 = time.time()

## print timing
if pars.log:
  logFile.write("\npdb2gmx took %.1fs to finish\n\n" % float(t2 - t1))
else:
  print "pdb2gmx took %.1fs to finish\n" % float(t2 - t1)

## check if an 'easymifs' file has been created
root = ".".join(pars.pdbName.split(".")[:-1])
if not os.path.isfile(root + ".easymifs"):
  if pars.log:
    logFile.write("Problems preparing the PDB file...please check if there are missing residues or missing atoms\n")
    logFile.close()
  else:
    print "Problems preparing the PDB file...please check if there are missing residues or missing atoms\n"
  sys.exit(2)

## checkpoint
t1 = time.time()

## launch EasyMIFs
launchEasyMIFs(pars, root)

t2 = time.time()

## print timing
if pars.log:
  logFile.write("\nEasyMIFs took %.1fs to finish\n\n" % float(t2 - t1))
else:
  print "EasyMIFs took %.1fs to finish\n" % float(t2 - t1)

## check if the EasyMIFs yielded a map file
if pars.compressed:
  suffix = ".cmp"
else:
  suffix = ".dx"
if not os.path.isfile(root + "_" + pars.probeType + suffix):
  if pars.log:
    logFile.write("Problems running EasyMIFs...please check your parameters\n")
    logFile.close()
  else:
    print "Problems running EasyMIFs...please check your parameters"
  sys.exit(2)
else:
  if pars.log:
     logFile.write("* Interaction Energy Map successfully computed * \n\n")
  else:
    print "* Interaction Energy Map successfully computed * \n\n"

## checkpoint
t1 = time.time()

## launch SiteHound
launchSiteHound(pars, root)

## checkpoint
t2 = time.time()

## print timing
if pars.log:
  logFile.write("\nSiteHound took %.1fs to finish\n\n" % float(t2 - t1))
else:
  print "SiteHound took %.1fs to finish\n" % float(t2 - t1)

if pars.log:
  logFile.close()


