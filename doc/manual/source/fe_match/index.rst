===========================================
Finite Element (FE) Model Viewer
===========================================

This module is used to display, and export FE solutions fitted to velocity experiments by various methods. 

.. toctree:: 
   :maxdepth: 1
   :caption: Table of Content:
   
   fe_match_main
   fe_match_simulate
   fe_match_adv

Steps to compare Experiment to FE models: 
=============================================

*  **Step 1:** *Load experimental velocity data.* Click on `Load Data <../common_dialogs.html#data-loader>`_ to select an edited velocity data set from the database or from local disk.

*  **Step 2:** *Select a model from database or disk.* Simply click on `Load Model <../common_dialogs.html#load-distribution-model>`_ and choose a model in the resulting dialog. If they exist, you will be given the choice of also loading noise vectors.

*  **Step 3:** *Simulate a model.* Simulate the loaded model with a finite element solution by clicking **Simulate Model**.

*  **Step 4:** *Examine the results.* After simulation, a variety of options are available for displaying `simulation results <fe_match_simulate.html>`_, residuals, and distributions. Report text files and graphics plot files can also be generated.


.. image:: ../_static/images/fe_match.png
   :align: center

.. rst-class:: center

   **Compare Experiment to Finite Element Solution**



