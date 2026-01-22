=====================================================
Density Matching
=====================================================


.. toctree:: 
    :maxdepth: 3

.. contents:: Index
    :local: 

Density Matching experiments are used to determine the partial specific volume of colloidal particles by repeating the velocity experiment in a range of buffers with a different density, but identical chemical properties. Here it is important that the ionic strength and pH stay constant. This is most easily accomplished by using D2O (heavy water) which has mostly the same chemical properties as light water. The change in buoyancy incurred when changing the density of the solvent causes a change in sedimentation speed, which can be measured. The change is proportional to the density of the analyte. Density matching experiments exploit this difference.

It is recommended that users of this technique measure sedimentation velocity experiments in 4-6 different D2O concentrations, including one buffer that does not have any D2O in it (just light water). Finite element models from these experiments are imported into this program to elucidate the partial specific volume distribution for the entire solution mixture. 

.. image:: /_static/images/densmatch_main.png
    :align: center

.. rst-class::
    :align: center

    **Density Matching Window**

Functions:
==============

**Model Selection Controls**

.. list-table::
  :widths: 20 50
  :header-rows: 0 


  * - **Database**
    - Check this radio button to indicate that distribution data to be selected is in the database.
  * - **Local Disk**
    - Check this radio button to indicate that distribution data to be selected is on local disk file systems.
  * - **(prefilter text):**
    - Brief information about PreFilter selection(s) made. 
  * - **Select PreFilter:**
    - Open a `Models Pre-Filter dialog <common_dialogs.html#data-loader>`_ to select Run IDs on which to pre-filter lists of models for loading. 
  * - **Load Distribution(s):**
    - Load model distribution data to use in density matching, as specified through a `Model Loader dialog <common_dialogs.html#load-distribution-model>`_.
  * - **Remove Distribution(s):**
    - Open a `Remove Models dialog <common_dialogs.html#remove-model-distributions>`_ to pare down the list of selected models. 
  * - **Refresh Plot:**
    - Force a re-plot of the currently selected plot type. 
  * - **Set Model Parameters:**
    - Open a Model Parameters dialog to associate D20_Percent, Density, Label values with each model. 
  * - **Save:** 
    - Generate CSV spreadsheet files and PNG plot files for all plot types and save them locally to a runID subdirectory of the */ultrascan/reports directory.*
  * - **Plot X:**
    - Select the plot to currently be displayed by selecting its X axis: 
  * - * **s** Sedimentation coefficient.
      * **D** Diffusion coefficient.
      * **vbar** Partial specific volume.
      * **m.mass** Molar Mass. 
      * **f/f0** Frictional ratio. 
      * **Rh** Hydrodynamic radius.
    -
  * - **Diffusion Coefficient Averaging:**
    - Select the means of averaging diffusion coefficients at each boundary fraction, across models:
  * - * **none** No averaging (maintain individual model D values). 
      * **simple** Simple averaging. 
      * **weighted** Weighted averaging, using concentrations. 
    -
  * - **(Distribution Information text box)**
    - Information about Run and Model Distributions currently being used. 
  * - **Divisions:)**
    - Set the number of boundary fraction divisions to use. 
  * - **% of Boundary::)**
    - Set the extent in boundary percent over which plot values are computed and plotted. 
  * - **Boundary Position (%)::)**
    - Set the starting boundary percent. 
  * - **Data Smoothing::)**
    - Set the number of smoothing points to use in curves displayed. 
  

**Window Controls**


.. list-table::
  :widths: 20 50
  :header-rows: 0 

  * - **Reset Plot** 
    - Clear PreFilter and Distribution selection data. 
  * - **Help** 
    - Display this detailed Density Matching help.
  * - **Close** 
    - Close all windows and exit.


