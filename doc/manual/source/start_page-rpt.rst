========================================
Reporting SV-Data
========================================

.. toctree:: 
  :maxdepth: 1

.. contents:: Index
  :local: 


Ultrascan-III Results from Sedimentation Velocity Data
===============================================================

Sedimentation velocity (SV) experiments in UltraScan-III produce a broad range of results that support data validation, simulation quality control, and model interpretation. The outputs range from basic run metadata and diagnostic residual analyses to visual, statistical, and confidence-based assessments of derived models. This document summarizes the principal result categories that an be obtained from sedimentation velocity data and identifies the modules used to access them.

1. Assessment of the Sedimentation Velocity Meta-data
-----------------------------------------------------

Modules responsible:
    * `Run Details <run_details.html>`_
    * `Time State Viewer <tmst_viewer.html>`_

The first level of result assessment in sedimentation velocity analysis is the verification and interpretation of experiment metadata. These values describe the conditions under which the experiment was acquired and are essential for judging whether the run is internally consistent and suitable for downstream analysis.

The metadata assessment includes the following items:

.. list-table::
  :widths: 20 50
  :header-rows: 0
  
  * - **Speed**
    - The measured rotor speed used during the experiment. This is one of the most important experimental descriptors because sedimentation behavior depends directly on angular velocity.
  * - **Temperature**
    - The recorded temperature during the run. Since solvent density, viscosity, and sedimentation behavior are temperature dependent, this value is critical for accurate physical  interpretation.
  * - **Run length**
    - The total duration of the experiment. The run length determines whether sufficient sedimentation progression was captured to support robust model fitting.
  * - **Time correction**
    - A corrected experimental time axis derived from the acquisition records. This is important when instrument timing and recorded scan timing differ, and it ensures that the analysis uses physically accurate elapsed times.
  * - **Run ID**
    - A unique identifier assigned to the experiment. The run ID links the data to a specific acquisition and is essential for traceability and reproducibility.
  * - **Directory of raw data**
    - The file system location of the raw experimental scans. This information supports provenance tracking and facilitates review or reprocessing of the original data.
  * - **Time interval**
    - The interval between scans. This value is useful for determining temporal resolution and whether the acquisition frequency is appropriate for the sedimentation process being observed.
  * - **Time**
    - The individual scan times or cumulative elapsed times used in the run. These values are required for precise reconstruction of the sedimentation process.
  * - **Raw speed and set-speed**
    - Both the measured rotor speed and the target rotor speed are reported. Comparing these values helps determine whether the rotor operated as intended and whether speed deviations could influence interpretation.
  * - :math:`\omega^2 t`
    - The integrated centrifugal field term, often expressed as :math:`\omega^2 t`. This quantity is fundamental in sedimentation analysis because it combines rotor speed and elapsed time into a single physically meaningful parameter.
  * - **Steps**
    - The sequence of acquisition or speed steps recorded during the run. These values help identify acceleration phases, speed transitions, or segmented un structures that may affect data interpretation.
  
Taken together, these metadata outputs provide the foundation for determining whether a sedimentation velocity experiment is complete, physically plausible, and ready for quantitative analysis.

2. Simulation Quality Assessment
--------------------------------

Modules responsible:
    * `Finite Element Model Viewer <fe_match/index.html>`_
    * `RMSD Model Query <rmsd_query.html>`_
    

Once metadata have been verified, the next level of assessment concerns the quality of the fit between the experimental data and the simulation or model. These outputs quantify how well the selected model reproduces the observed sedimentation process.

The simulation quality assessment includes the following results:

.. list-table::
  :widths: 20 50
  :header-rows: 0

  *  - **root-mean-square deviation**
     - The root-mean-square deviation (RMSD) is reported together with the edit profile, analysis method, and Triplicate designation. This allows the user to evaluate fit quality in the exact context in which the model was generated. RMSD is a primary indicator of how closely the simulation matches the experimental scans.
  *  - `Bitmap <fe_match/fe_match_simulate.html#residual-bit-map>`_
     - A bitmap representation of fit quality or residual structure may be generated to provide a rapid visual diagnostic of agreement between the model and the data. Such displays can help reveal systematic artifacts, scan-specific problems, or localized fitting errors.
  *  - `Time- and radially invariant noise profiles <fe_match/fe_match_simulate.html#noise-data-analysis>`_
     - Noise decomposition is a central element of SV analysis. UltraScan3 can report both time-invariant noise and radially invariant noise, allowing the user to determine whether systematic baseline distortions are contributing to the apparent signal. These noise profiles are important for judging whether the fit quality is limited by model inadequacy or by non-ideal data structure.
  * - **Residuals**
    - Residuals represent the difference between experimental data and simulated values. Inspection of residuals is essential for identifying systematic deviations, concentration-dependent effects, convection artifacts, optical distortions, or incomplete modeling assumptions. Ideally, residuals should be random and centered around zero.


Together, these outputs form the core quality-control layer for determining whether a given fit is acceptable and whether further refinement, editing, or model adjustment is required.

3. Model Assessment
-------------------

Model assessment focuses on the interpretation of the fitted sedimentation velocity solution. In UltraScan3, model assessment includes both visual and statistical outputs. These results help the user decide whether a model is physically meaningful, whether multiple species are resolved, and how stable the fitted parameters are across simulations.

A. 2-D and 3-D Plots
~~~~~~~~~~~~~~~~~~~~~

The visual assessment tools provide direct graphical insight into the fitted distributions and related spectral or structural properties.

Modules and outputs include:


`Combine Distribution Plot (vHw) <vhw_combine.html>`_
A visualization of the van Holde-Weischet-style interpretation or related transformed sedimentation boundaries, useful for examining heterogeneity and distributional spread.

`Combine Integral Distribution <idist_combine.html>`_
An integral representation of the distribution, emphasizing cumulative contributions across the modeled sedimentation range.

`Combine Discrete Distribution <ddist_combine.html>`_
A discrete-species representation in which resolved components are shown as separate populations with distinct parameter values.

`Combine Pseudo-3D Distribution <pseudo3d.html>`_
A pseudo-distribution view that provides an approximate or smoothed representation of model content for easier interpretation.

`Multi-Wavelength S Spectra Viewer <multi-wavelength/mwl_spectra.html>`_
This module supports inspection of multiwavelength-related results in a model context. It can help connect hydrodynamic outcomes to wavelength- dependent behavior, especially in experiments involving spectrally resolved data.

`3-D plot <3d_plot.html>`_
Three-dimensional plots provide a richer visualization of model structure, allowing distributions or signal relationships to be viewed across multiple axes. These plots are especially useful for complex systems, multi-parameter interpretations, and multiwavelength data exploration.

Visual outputs are often the first means by which a user identifies sample heterogeneity, discrete species, broad distributions, or model artifacts from improper grid ranges.

B. 2-D and 3-D Assessments
~~~~~~~~~~~~~~~~~~~~~~~~~~

Modules responsible:
    * `Finite Element Model Statistics and Metrics <us_modelmetrics.html>`_
    * `Initialize Genetic Algorithm <ga_initialize.html>`_

Beyond visual inspection, UltraScan3 provides higher-order statistical assessment of finite element and genetic algorithm-derived models. These tools support quantitative evaluation of model robustness and parameter certainty.

`Finite Element Model Statistics and Metrics <us_modelmetrics.html>`_
This module provides summary statistics for finite element-based models. The results support evaluation of model quality across parameters and may include descriptors of central tendency, spread, and agreement among simulated solutions.

`Initialize Genetic Algorithm <ga_initialize.html>`_
Genetic algorithm analysis provides population-based model exploration and permits statistical characterization of fitted attributes across many solutions. This is particularly useful for determining whether a derived solution is stable, unique, and supported by repeated simulations.

The principal results from these assessments include:

**95% confidence interval of every simulated attribute**
For each simulated parameter or attribute, UltraScan-III can estimate the range within which the true value is expected to lie with 95% confidence. These intervals are critical for distinguishing well-resolved parameters from poorly constrained ones.

**Standard deviation of every simulated attribute**
The standard deviation provides a direct measure of variability among the simulated solutions. Small standard deviations indicate stable and reproducible parameter estimates, while large standard deviations suggest uncertainty, parameter coupling, or insufficient information in the data.

These 2-D and 3-D statistical assessments are especially important when comparing alternative models, evaluating fit robustness, and reporting quantitative confidence in the derived sedimentation results.

Summary
-------

Sedimentation velocity data in UltraScan3 can yield results in three broad categories:

1. Metadata assessment, which verifies the integrity and physical context of the experiment.
2. Simulation quality assessment, which measures how well the model reproduces the experimental data.
3. Model assessment, which provides graphical and statistical interpretation of the derived solution.

Together, these outputs allow the user to move from simple run verification to deep physical interpretation of macromolecular behavior in solution. 
