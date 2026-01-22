====================================
2-Dimensional Spectrum Analysis 
====================================

.. toctree:: 
  :maxdepth: 3

.. contents:: Index
  :local: 


The 2DSA method is used for composition analysis of sedimentation velocity experiments. It can generate sedimentation coefficient, diffusion coefficient, frictional coefficient, f/f :sub:`0` ratio, and molecular weight distributions. The distributions can be plotted as 3-dimensional plots (2 parameters from the above list against each other), with the third dimension representing the concentration of the solute found in the composition analysis. The set of all such final calculated solutes form a model which are used to generate a simulation via Lamm equations. The simulation is plotted overlaying a plot of experimental data.


Every 2DSA pass can be repeated for a specified number of refinement iterations. These iterations, in turn, can be repeated for a specified number of meniscus points or Monte Carlo iterations.


Each refinement iteration proceeds over a defined grid of s and f/f :sub:`0` values. That grid is divided into subgrids as defined by a number of grid refinements in each direction.

|

.. image:: ../_static/images/2dsa_main-o.png
    :align: center

.. rst-class::
    :align: center

    **2-Dimensional Spectrum Analysis**


2DSA Process
================

*  **Step 1:** First, load experimental velocity data. Click on `Load Data <../common_dialogs.html#data-loader>`_ to select an edited velocity data set from the database or from local disk.
*  **Step 2:** Secondly, open an analysis control window by clicking on `Fit Control <2dsa_analys.html>`_. Within that dialog, define the grids and iterations that comprise the analysis.
*  **Step 3:** Next, after having specified analysis parameters, begin the fit analysis by clicking "Start Fit".
*  **Step 4:** Display and Save Results: After simulation, a variety of options are available for displaying simulation results, residuals, and distributions. Report text files and graphics plot files can also be generated.

Once an analysis is complete, the main window will appear similar to this:

.. image:: ../_static/images/2dsa_main-c.png
    :align: center

.. rst-class::
    :align: center

    **Simulated 2DSA Model**

2DSA Main Window Functions
==========================

.. list-table::
  :widths: 20 50
  :header-rows: 0 
  
  * - **Load Experiment**
    - Click here and, in the resulting `Load Run Data Dialog <../common_dialogs.html#data-loader>`_, select an edited data set to load.
  * - **Run Details**
    - Pop up a dialog showing `Run Details Dialog <../run_details.html>`_.
  * - **Latest Data Edit**
    - Uncheck to allow choosing an edit other than the latest one for the raw experimental data.
  * - **Database**
    - Select to specify data input from the database.
  * - **Local Disk**
    - Select to specify data input from local disk.
  * - **View Data Report**  
    - Generate a report file and view it in a dialog. See the details of the report in `2DSA Process <2dsa_analys.html>`_.
  * - **Save Data**
    - Save models and noises, as well as report and plot images files.
  * - **Run ID / Edit ID:**
    - The Run identifier string and the Edit identifier (generally a time string) are displayed for loaded edit.
  * - **Avg Temperature:**
    - The average temperature over all the scans of the loaded data.
  * - **(experiment description)**
    - A text string is displayed giving a fairly detailed description of the experiment.
  * - **Cell / Channel / Wavelength**
    - One or more rows of data edit triples. If more than one, click on a row to select it as the data of interest.
  * - **Solution**
    - Click this button to open a `Solution Management dialog <../solution/index.html>`_ that allows changes to buffer and analyte characteristics of the data set. 
  * - **Density (20°C)**
    - Shows the density value for the loaded experiment. Click the Solution button to open a dialog in which density and other values may be changed.
  * - **Viscosity (20°C)**
    - Shows the viscosity value for these loaded experiment. Click the Solution button to open a dialog in which viscosity and other values may be changed.
  * - **Vbar (20°C)**
    - Shows the vbar value for the loaded experiment. Click the Solution button to open a dialog in which vbar and other values may be changed.
  * - **Skipped**
    - The count of experiment data scans skipped.
  * - **Variance:**
    - Variance value (square of RMSD) for residuals.
  * - **RMSD:**
    - Root-Mean-Square-of-Differences for residuals.
  * - **Scan focus from:**
    - Select a low scan of the range of values for exclusion from analysis. 
  * - **to:**
    - Select a high scan of the range of values for exclusion from analysis.
  * - **Exclude Scan Range**
    - Initiate exclusion of the scans given in the above controls. 
  * - **Reset Scan Range**
    - Reset to the full range of scans.
  * - **Fit Control**
    - Open a dialog to set analysis parameters and start a fit run. For details on the results of clicking this button, see `2DSA Analysis Control <2dsa_analys.html>`_.
  * - **3-D Plot**
    - After a 2DSA model is fitted, open a control window for a `3-Dimensional plot <../3d_plot.html>`_ of the final computed model. 
  * - **Residual Plot**
    - After a 2DSA model is fitted, open a plot dialog for a far more detailed set of result plots. See `Finite Element Viewer/Residual Plot Dialog <../fe_match/fe_match_simulate.html>`_ or `2-D Analysis Controls <2dsa_analys.html#residual-plot-dialog>`_ for further details.
  * - **Status Info:**
    - This text window displays continually updated summaries of computational activity and results.
  * - **(upper right side plot)**
    - Upon analysis completion, this plot is of the Residuals (Experimental minus Simulation minus any Noise).
  * - **(lower right side plot)**
    - Upon analysis completion, this plot is of an overlay of the Experimental and Simulation data.


**Window Controls**


.. list-table::
  :widths: 20 50
  :header-rows: 0 

  * - **Reset** 
    - Indicate that window is reset and the plots are initiated.
  * - **Help** 
    - Display this detailed 2-Dimensional Spectrum Analysis Functions help.
  * - **Close** 
    - Close all windows and exit.


Related
====================

`Fit Meniscus <../fit_meniscus.html>`_

`2-Dimensional Spectrum Analysis Process <2dsa_analys.html>`_

`Finite Element Model Viewer <../fe_match/index.html>`_
