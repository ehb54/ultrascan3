# Standalone build for the SOMO perceiver prototype (Qt-free).
CXX      ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -g
SRC       = ../src/us_hydrodyn_perceive.cpp
INC       = -I../include
HDR       = ../include/us_hydrodyn_perceive.h ../include/us_hydrodyn_perceive_hybrid.h \
            ../include/us_hydrodyn_perceive_elements.h ../include/us_hydrodyn_perceive_saxs.h \
            pdb_lite.h residue_oracle.h tinytest.h

BUILDER_SRC = $(SRC) ../src/us_hydrodyn_psv.cpp ../src/us_hydrodyn_hydration.cpp \
              ../src/us_hydrodyn_grid_volume.cpp ../src/us_hydrodyn_residue_builder.cpp

DEMOS = data/1HEL.pdb data/6LYZ.pdb data/2AAS_mod1.pdb data/8RAT.pdb \
        data/1AO6-compl_monA.pdb data/3CRO.pdb data/3GUT.pdb data/6LDH1.pdb

.PHONY: all unit regress regress-ci emit clean
all: unit regress

bin/emit_residue: tests/emit_residue.cpp $(BUILDER_SRC) $(HDR)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INC) tests/emit_residue.cpp $(BUILDER_SRC) -o bin/emit_residue

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

# What CI runs. The point is that the regression ACTUALLY RUNS and its numbers appear in the
# log; the pass/fail conditions are deliberately slack, because the perceiver is under active
# development and a gate pinned to today's figures would fail on every legitimate change to it.
#
#   --require-all-inputs  every fixture must yield something to score. This is the check that
#                         earns its place: it catches a fixture missing, unreadable or
#                         truncated -- the failure that would otherwise leave the percentages
#                         looking healthy while measuring less -- and it is orthogonal to
#                         perception quality, so tuning the perceiver cannot trip it.
#   --min-geometric 95    a collapse detector, nothing finer. Current value is 99.833%, so
#                         there is ~5 points of room; only a real breakage reaches it.
#
# Deliberately NOT set: --min-atoms. A floor on the atom count IS sensitive to perceiver work
# (parsing and which atoms are scorable), so pinning it would break CI for anyone changing the
# module. Tighten these once the perceiver settles.
REGRESS_MIN_GEOMETRIC = 95

regress-ci: bin/regression
	./bin/regression --require-all-inputs --min-geometric $(REGRESS_MIN_GEOMETRIC) $(DEMOS)

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

bin/sssr: tests/sssr.cpp $(SRC) $(HDR)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INC) tests/sssr.cpp $(SRC) -o bin/sssr

sssr: bin/sssr
	./bin/sssr

bin/sssrreal: tests/sssr_real.cpp $(SRC) $(HDR)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INC) tests/sssr_real.cpp $(SRC) -o bin/sssrreal

sssrreal: bin/sssrreal
	./bin/sssrreal $(DEMOS)

bin/psv: tests/psv.cpp $(SRC) ../src/us_hydrodyn_psv.cpp $(HDR) ../include/us_hydrodyn_psv.h
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INC) tests/psv.cpp $(SRC) ../src/us_hydrodyn_psv.cpp -o bin/psv

psv: bin/psv
	./bin/psv

bin/hydration: tests/hydration.cpp ../src/us_hydrodyn_hydration.cpp ../include/us_hydrodyn_hydration.h
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INC) tests/hydration.cpp ../src/us_hydrodyn_hydration.cpp -o bin/hydration

hydration: bin/hydration
	./bin/hydration


bin/builder: tests/builder.cpp $(BUILDER_SRC) $(HDR)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INC) tests/builder.cpp $(BUILDER_SRC) -o bin/builder

builder: bin/builder
	./bin/builder

bin/protein_psv: tests/protein_psv.cpp $(SRC) ../src/us_hydrodyn_psv.cpp $(HDR) ../include/us_hydrodyn_psv.h
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INC) tests/protein_psv.cpp $(SRC) ../src/us_hydrodyn_psv.cpp -o bin/protein_psv

PSV_PROTEINS = data/1HEL.pdb data/8RAT.pdb data/psv/1BEB.pdb data/psv/2CGA.pdb \
               data/1AO6-compl_monA.pdb data/6LDH1.pdb data/1ADO_noSO4.pdb

protein_psv: bin/protein_psv
	./bin/protein_psv data/psv_measured.txt $(if $(PDBS),$(PDBS),$(PSV_PROTEINS))
