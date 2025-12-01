==========================================
Simulation Module
==========================================

.. toctree:: 
  :maxdepth: 3

.. contents:: Index
  :local: 

This module is used to simulate an experiment using Finite Element (ASTFEM) or Finite Volume (ASTFVM) methods. A model is loaded from the database or from local disk. Simulation parameters are specified. A simulation is then calculated and displayed. The simulation may be saved as a synthetic version of raw experimental data. 
If model and simulation parameters indicate ideal conditions, the default method is Adaptive Space Time Finite Element Method (ASTFEM). For non-ideal cases - such as concentration dependence, co-sedimenting, or compressibility, the default is Adaptive Space Time Finite Volume Method (ASTFVM). 

Process: 
==========

1. Model Control: First, load a model on which the simulation will be based. 
2. Simulation Parameters: Secondly, open a dialog to specify parameters governing the simulation. 
3. Define Buffer: Optionally (and rarely), you may wish to specify buffer conditions. This may be necessary if your intention is to simulate and save an artificial version of a specific experimental data set. Normally, the buffer conditions are those of water at 20 degrees Centigrade. 
4. Start Simulation: Initiate calculation of the specified simulation. 
5. Display and/or Save Results: After simulation, a variety of options are available for displaying simulation results and for optionally saving them to disk. 