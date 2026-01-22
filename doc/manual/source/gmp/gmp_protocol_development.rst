================================
Protocol Development
================================

.. toctree:: 
    :maxdepth: 1

.. contents:: Index:
    :local:

Allows for analysis profile optimization by re-attaching GMP runs at the **LIMS Import** stage with a modified **Aprofile**

Manage Optima Runs:
=======================

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_01.png
    :align: center

Select Completed Autoflow Run:
=================================

Provides a list of completed GMP runs in the database, as well as the current reporting stage (Report generation or collection of e-signatures).

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_02.png
    :align: center

A GMP run can be loaded from the database by highlighting it and clicking the **Select Item** button. This opens a modified **Data Acquisition Routine**, where the user can assign a new protocol name. However, the run name and experimental parameters cannot be changed, as the run is being reimported for editing and analysis.

Sections 1-9 can be accessed in any order using the navigation bar at the top of the window or sequentially using the **Previous Panel** and **Next Panel** buttons at the bottom of the window.


1: General
================

The protocol name is the only information that can be changed, since the source GMP run (Run Name, Label, Run Temperature and Temperature-Equilibration Delay) remains unchanged.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_03.png
    :align: center

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_04.png  
    :align: center
    
2: Lab/Rotor
===============

The user cannot change the laboratory, rotor, rotor calibration profile, instrument, and experiment types. The roles of Operator, Reviewer, Approver, and Subject Matter Expert (SME) can be re-assigned from the drop-down menus. See here for more information: `UltraScan GMP Help <../index.html>`_. 

3: Speeds
=============

None of the run parameters in this window can be modified.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_05.png 
    :align: center

4: Cells
===========

None of the run parameters in this window can be modified.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_06.png 
    :align: center

5: Solutions
=============

None of the run parameters in this window can be modified.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_07.png 
    :align: center

5b: Solution Details
---------------------

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_08.png 
    :align: center

6: Optics
============

None of the run parameters in this window can be modified.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_09.png 
    :align: center

7: Ranges
===========

None of the run parameters in this window can be modified.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_10.png 
    :align: center

7a: View Ranges
------------------

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_11.png 
    :align: center

8: AProfile (Analysis Profile)
=================================

Define edit profiles, analysis parameters and report settings for the experiment.

8.1: General
------------------

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_12.png 
    :align: center
    
Define numerical values for 'Loading Ratio', the 'Â± %Tolerance', 'Loading Volume (Î¼L)', the 'Â± %Tolerance', and 'Data End (cm)'. The user can then select checkboxes to specify whether the analysis should be 'Run', and included in the 'Run Report'.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_13.png 
    :align: center
    
The **Report General Settings** windows allow the user to select/deselect which of the protocol settings (Sections 1-8) will be presented in the report. The {down arrow} button can be used to collapse or expand each subsection.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_14.png 
    :align: center
    
The **Scan Exclusion Editor** allows the user to numerically define the number scans to be removed from the beginning and end of each channel, either by typing the value or using the arrows. The **Apply to All** button applies the values for the first channel to all channels.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_15.png 
    :align: center
    
The **Report** button summons the channel-specific report settings window. The user defines the 'Total Concentration', the 'Â± %Tolerance', 'RMSD (upper limit)', the 'Â± %Tolerance', and the wavelength to extract these metrics from. The user then lists the expected 'Experiment Duration', the 'Â± %Tolerance', and 'Minimum Intensity' at the selected wavelength from the xenon flash lamp that is deemed acceptable. In the 'Report Profile (Mask) Settings' the user can select the following: 'Total Concentration', 'Minimum Intensity', 'RMSD (upper limit)', 'Integration Results', 'Experiment Duration' and relevant plots (as well as the pseudo-3D plots for each parameter and analysis type) they wish to be included in the report.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_16.png 
    :align: center
    
**Global Parameters for Combined Plots** allows the user to define the plot minimum, plot maximum and Gaussian sigma for each plot type.

8.2: 2DSA
=============

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_17.png 
    :align: center
    
The user can define the 2-dimensional spectrum analysis (2DSA) controls for each channel. The grid size is defined by the minimum and maximum values for the sedimentation coefficient (<em>s</em>) and frictional ratio (<em>f/f0</em>), while the grid resolution is defined by the number of points for each of the parameters. A 2DSA **Custom Grid** (CG) can also be used if the user requires higher resolution in certain regions of the data, and only needs lower resolution in other regions. The user can also select to vary the partial specific volume or 'Vbar' parameter and hold the frictional ratio constant at a specified value. The **Apply to All** button can be used to apply these settings to all of the channels.

The '2DSA Job Flow' panel allows the user to control which analysis steps are performed, including:


    * 2DSA
    * 2DSA with meniscus fit (FM), with a user-defined number of grid points, range (cm), and number of refinement iterations. The user can also choose to fit **only the bottom of the channel** or **both the meniscus and the bottom.**
    * FITMEN meniscus processing, with the option to 'Auto-pick' the meniscus position
    * 2DSA with iterative refinement (IT), with a user-defined number of iterations
    * 2DSA with Monte Carlo iterations (MC), with a user-defined number of Monte Carlo iterations.


8.3: PCSA
===========

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_18.png 
    :align: center
    

The user can select the option to not include the parametrically-constrained spectrum analysis (PCSA) in the submitted jobs. If the box is left unchecked, the user can define the PCSA controls for each channel.

The desired curve type is selected from the drop-down menu, and the user defines parameters for the x, y, and z axes.


    * The x and y axes are defined by minimum and maximum values
    * The z axis is assigned a value and a variation count
    * Numerical values are then assigned for the grid fit iterations and curve resolution points

Optional settings include:


    * Time- and radially-invariant noise fitting
    * Tikhonov regularization, with either an auto-picked or manually-selected alpha
    * Number of Monte Carlo iterations.


The 'Apply to All' button can be used to apply these parameters and settings to all of the channels.

9: Submit
==========

Submit the protocol development run for processing

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_19.png 
    :align: center
    
9a: View Experiment Details
---------------------------------

Summary of the information defined in sections 1-8.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_20.png 
    :align: center
    
9b: Submit the Run
-----------------------

A new protocol name and GMP e-signer roles must be assigned before the 'Submit the Run' button becomes active and allows for submission. Submission prompts the following window:

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_21.png 
    :align: center

The 'OK' button prompts the 'GMP Run Submitter Form', allowing the user to add a comment and prompting them for their master password for authentication.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_22.png 
    :align: center
    
The 'OK' button brings up the 'Run Reinitialization' window, which informs the user that 'Proceed' will delete existing edit profiles, models, and noises, and reinitialize at the EDIT stage.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_23.png 
    :align: center
    
The program moves to the 'IMPORT' stage, which displays the last scan of each triple from the intensity data. The user is prompted with the **Reference Scans: Automated Processing** message:

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_24.png 
    :align: center
    
Clicking **OK** displays the converted absorbance data following the automated reference scan processing. If the user is not satisfied with the automated reference scans processing, the **Undo Reference Scans** button allows for them to be defined manually using the **Define Reference Scans** button. **Drop Selected Triples** can be used to drop unwanted triples.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_25.png 
    :align: center
    
The **Save** button prompts the **GMP Run IMPORT Form**, allowing the user to add a comment and prompting them for their master password for authentication.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_26.png 
    :align: center
    
Once the data has been saved, the user is notified that the program is proceeding to the editing stage:

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_27.png 
    :align: center
    
The editing window presents the user with the data edited in accordance with 'Data End' value and 'Excluded Scan Ranges' defined in the **Aprofile**. The 'Meniscus' position and 'Data Start' value are defined through automated processing. If the user is not satisfied with the automated processing, the **Edit Currently Selected Profile Manually** button allows for the 'Meniscus' position and 'Data End' values to be defined manually. **Remove Spikes** removes some sharp, irregular intensity spikes from data. Each cell/channel/wavelength triple can be cycled through using the **Previous Triple** and **Next Triple** buttons.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_28.png
    :align: center
    
The **Save Edit Profiles** button prompts the **GMP Run EDIT Form**, allowing the user to add a comment and prompting them for their master password for authentication.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_29.png
    :align: center
    
Once the edited profiles have been saved, the user is notified that the program is proceeding to the analysis stage:

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_30.png
    :align: center
    
The analysis window displays the current status of the submitted jobs producing the models specified in the **Aprofile**. **Cancel** can be used to stop ongoing jobs. **View Fit** simulates completed model overlays and displays them. The **Expand All Triples** and **Collapse All Triples** buttons control all triples, while the {down arrow} button can be used to collapse or expand each individual triple.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_31.png
    :align: center
    
Once the analysis stage is complete, the models are simulated and the report is built. Which prompts the **Report Generation Complete** message box.

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_32.png
    :align: center
    
The **View Report** button opens the PDF file to be viewed:

.. image:: ../../_static/images/gmp/protocol_development/protocol_development_33.png
    :align: center
    