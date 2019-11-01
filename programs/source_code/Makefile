

# make the subdir first
SHELL :=/bin/bash
CXX = g++
FLAG = -O3

all:
	# --- LigBind_Assess ---- #
	$(CXX) LigBind_Assess.cpp $(FLAG) -o LigBind_Assess
	$(CXX) PointSite_IoU.cpp $(FLAG) -o PointSite_IoU
	$(CXX) PointSite_Merge.cpp $(FLAG) -o PointSite_Merge
	mv LigBind_Assess PointSite_IoU PointSite_Merge ../util

# clean all the temp files
clean:

