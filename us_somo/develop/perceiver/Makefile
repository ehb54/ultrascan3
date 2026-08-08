# Standalone build for the SOMO perceiver prototype (Qt-free).
CXX      ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -g
SRC       = ../src/us_hydrodyn_perceive.cpp
INC       = -I../include
HDR       = ../include/us_hydrodyn_perceive.h ../include/us_hydrodyn_perceive_hybrid.h \
            ../include/us_hydrodyn_perceive_elements.h ../include/us_hydrodyn_perceive_saxs.h \
            pdb_lite.h residue_oracle.h tinytest.h

DEMOS = data/1HEL.pdb data/6LYZ.pdb data/2AAS.pdb data/8RAT.pdb \
        data/1AO6-compl_monA.pdb data/3CRO.pdb data/3GUT.pdb data/6LDH1.pdb

.PHONY: all unit regress emit clean
all: unit regress

bin/emit_residue: tests/emit_residue.cpp $(SRC) $(HDR)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INC) tests/emit_residue.cpp $(SRC) -o bin/emit_residue

emit: bin/emit_residue
	./bin/emit_residue $(PDB) $(RES) $(SEQ)

bin/unit: tests/tests_unit.cpp $(SRC) $(HDR)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INC) tests/tests_unit.cpp $(SRC) -o bin/unit

bin/regression: tests/regression.cpp $(SRC) $(HDR)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INC) tests/regression.cpp $(SRC) -o bin/regression

unit: bin/unit
	./bin/unit

regress: bin/regression
	./bin/regression $(DEMOS)

clean:
	rm -rf bin

bin/coverage: tests/coverage.cpp $(SRC) $(HDR)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INC) tests/coverage.cpp $(SRC) -o bin/coverage

coverage: bin/coverage
	./bin/coverage $(DEMOS)

bin/gridvol: tests/grid_volume.cpp ../src/us_hydrodyn_grid_volume.cpp ../include/us_hydrodyn_grid_volume.h tinytest.h
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INC) tests/grid_volume.cpp ../src/us_hydrodyn_grid_volume.cpp -o bin/gridvol

gridvol: bin/gridvol
	./bin/gridvol $(DATA)
