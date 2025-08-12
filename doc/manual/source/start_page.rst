=========================================================
Flowchart for the Analysis of Sedimentation Velocity Data 
=========================================================


.. toctree:: 
  :maxdepth: 3

.. contents:: Index
  :local: 


Step 1 (required for data from XLA or XLI, this step is performed automatically in the Optima AUC): Import Experimental data into UltraScan-III OpenAUC format

    Utilities:Import Experimental Data
    Confirm Investigator setting and local/database selection
    Import Experimental Data from HD - sedimentation velocity data should not be measured in absorbance mode, use intensity mode instead.
    Edit Run Information, Select Lab/Rotor/Calibration
    Enter a Label (verbose description for the run)
    Select the corresponding project by clicking on the Project button. If there is no project, create a new project. Projects can als be entered through the LIMS
    Confirm experiment type and optical detection system
    Enter any comments, if applicable
    Select instrument, rotor, rotor calibration and operator
    Click on Accept.
    Edit the Description field if necessary
    Navigate to the first channel and select the centerpiece type.
    Select the proper solution, make sure that the solution contains at least one analyte and a buffer
    If you have more than one triple, you can click on Apply to All but verify centerpiece and solution for each triple first. Also, check the Description field again to make sure the appropriate information is saved.
    If data were collected in intensity mode, you will need to Define Reference Scans by selecting a short region from the air-to-air interface portion of the data.
    Failed triples or empty triples can be excluded from the run by clicking on Drop Selected Triples.
    For equilibrium data from 6-channel centerpieces you should separate each channel with the Define Subsets/Process Subsets functions.
    When everything has been set you can Save the scans to database or disk.




Step 2 (required): Edit experimental data (see UltraScan Manual for details).





Step 3 (required): Perform a Time Derivative analysis and find the limits of the s-value range.

    Select Velocity:Time Derivative and Load Experiment
    Set Data Smoothing to ~ 10
    Set the Boundary Pos (%) to zero
    Exclude scans that do not exhibit a stable upper plateau.
    Select Average S to plot the dc/dt S-value distribution (default setting).
    Adjust S-value cutoff as needed.
    The correct S limits to choose are the left and right limits of the S-value distribution where all signal returns to baseline. The minimum S-value allowed is 0.2 S.




Step 4 (required): Submit a 2DSA analysis request in USLIMS.

    Locate the dataset and set the s-value limits to values obtained in step 3, and set f/f0 limits to 1-4 or adjust the upper limit based on prior knowledge of the sample (in case of DNA for example).
    Set the resolution for S and f/f0 to the desired value. 64 is the default for both S and f/f0, the resolution is the number of points into which this variable will be discretized. For example, if the range is selected from 1-5, and the resolution is set to 40, there will be 10 grid points/S-value, resulting in an increment of 0.1 S.
    Set the grid resolution. The default resolution of 64 will be sufficient for most situations. When you have a very polydisperse or heterogeneous sample you may have to go higher. Larger values will increase compute time. A good strategy is to check if there are noticeable jumps in the fit. If there are, increase the resolution. If you are trying to fit a bi- or tri-modal system, you may be able to get a higher resolution by using the custom grid method, and use the appropriate resolution for each grid.
    Turn on time invariant noise and leave all other settings at default values.
    Submit the job to your desired cluster or calculate locally using the 2DSA GUI method
    Check queue viewer for job completion
    Confirm results with Velocity:FE Model Viewer, load newly generated time invariant noise file. If random residuals are obtained, proceed with step 5, but if residuals do not look random, and a strong diagonal line in the residual bitmap is apparent, investigate range settings for S and f/f0 settings and repeat 2DSA with improved ranges.
    Note: Do not set the lower s-value limit too low, as this could create artificially low-s species signal if a baseline or slowly changing baseline exists. These artifacts are better handled in the time invariant noise. 




Step 5 (required): Perform Meniscus fit.

    repeat submission by also fitting the meniscus over 0.03 cm with 10 points, turning on both time invariant and radially invariant noise. Make sure to load the previously generated time invariant noise when loading the dataset. Use the same range settings as in Step 4.
    Check queue or e-mail and once results are available, load Utilities:Fit Meniscus. Click on Scan Database if using the database, and check the Status line for new results. Go to File:Load to load the desired meniscus fit (check the UltraScan manual for details).
    After updating the meniscus, confirm the deletion of the scans that resulted in non-optimal RMSD values.



Step 6 (required): Perform a final 2DSA refinement

    Refine the analysis by re-fitting the data with the improved meniscus value by repeating the analysis with identical range settings as before fitted in Step 5, except this time selecting the time- and radially invariant noise that was generated in Step 5, and also do not select meniscus fitting. Instead, select Iterative Refinement and set the refinement level to 10 iterations. Also refit time and radially invariant noise.
    Visualize the final results in Velocity:FE Model Viewer and save results to database.
    All subsequent analyses methods should now be based on the model generated in this final 2DSA refinement step.

At this point, multiple analysis options exist depending on the properties of the analyte distribution. If a paucidisperse solution is obtained, parsimonious regularization with the genetic algorithm method is appropriate. Otherwise, the data should be analyzed only by the 2DSA analysis in conjunction with a 50-iteration Monte Carlo analysis. Both options are explained below.




Step 7 (optional): Genetic Algorithm analysis.

    If the refined 2DSA data are appropriate for genetic algorithm analysis, select Velocity:Initialize Genetic Algorithm and load the model from Step 6 into the initialization program. See the corresponding UltraScan Manual section for details.
    Assign initialization and save to disk
    Log into USLIMS and submit data to Genetic Algorithm analysis, but do not refit time or radially invariant noise. Make sure to select time and radially invariant noise generated in Step 6, which is set to occur by default unless overridden by the user.
    Select the gadistro file from the UltraScan/results/run-id directory for the correct triple.
    Visualize results by using the Velocity:FE Model Viewer





Step 8 (recommended): Perform 2DSA Monte Carlo analysis.

    Use the same limits as before, but do not refit time or radially invariant noise, instead, load the noise corrections from Step 6 (which will be done by default in the LIMS and GUI). Select 50 Monte Carlo iterations.
    If the 2DSA distribution appears to be a sparse solute situation, and not a smooth continuous distribution of many species, you can further refine the data with a parsimonious regularization using the GA analysis.
    When using 2DSA Monte Carlo distributions for the GA initialization, make sure to use the manual GA initalization method in Velocity:Initialize Genetic Algorithm.
    Proceed as described in Step 7.

Step 9 (optional): Perform Monte Carlo GA analysis

    Using the results from Step 7, initialize the genetic algorithm - Monte Carlo analysis as described in Step 7.
    In USLIMS, load the data, using the noise files generated in Step 6 (will be done by default if settings are not changed).
    Select a multiple of 8 Monte Carlo iterations (48, 56 or 64 are good choices)
    select parallel processing with 8 program groups.
    repeat Step 7 by loading the gadistro file generated from the GA distribution model.
    Submit to desired cluster and visualize results by using the Velocity:FE Model Viewer




Step 10 (recommended): Perform van Holde - Weischet analysis

    Open Velocity:Enhanced van Holde - Weischet
    Load the desired experiment, applying the noise files from Step 6 (the latest model).
    Check Plateaus from 2DSA and Use Enhanced vHW
    Adjust Beck Diffusion Tolerance, Divisions, Data Smoothing, % of Boundary, and Boundary Position to desired values.
    If appropriate, delete early scans to improve resolution and reduce noise. Only keep scans and boundary portions that contribute to well correlated line fits in the linear extrapolations.
    Select groups, if appropriate, to generate weight averaged s-values for discrete species
    Display Distribution Plot and histogram.
    Save Data and distributions
    Refer to the van Holde - Weischet manual page for additional details.




Step 11 (recommended): Overlay combined distributions

    All van Holde - Weischet distributions and finite element models can be combined into a single plot for easy comparison.
    Use Velocity:Combine Distribution Plots (vHW) for van Holde - Weischet plots
    Use Velocity:Combine discrete Distributions for all finite element models (2DSA, GA, Monte Carlo)





Step 12 (optional): Perform Parametrically Constrained Spectrum Analysis (PCSA)

    Inspect the results from the 2DSA-Monte Carlo analysis to determine an appropriate parameterization constraint for the PCSA analysis. Select a parametrization (straight line, increasing sigmoid, decreasing sigmoid, power law or horizontal line (equivalent to C(s), not recommended) that is consistent with the pattern displayed in the pseudo-3D plot of the 2DSA-Monte Carlo analysis, i.e., a line shape that best describes the distribution of most solute points. You can select the PCSA either from the desktop GUI (Velocity:PCSA) or from the corresponding LIMS option after loading the data.
    Decide on a parameter combination to be fitted and set appropriate limits. If fitting s and f/f0, use the same limits as were used in the 2DSA.
    Optionally, regularize PCSA result either by Monte Carlo or Tikhonov regularization. Tikhonov regularization can be performed with a specified regularization level, or by asking the program to auto-compute the most appropriate regularization level based on the L-curve criterion, see:
    Gorbet G., T. Devlin, B. Hernandez Uribe, A. K. Demeler, Z. Lindsey, S. Ganji, S. Breton, L. Weise-Cross, E.M. Lafer, E.H. Brookes, B. Demeler. A parametrically constrained optimization method for fitting sedimentation velocity experiments. Biophys. J. (2014) vol 106, 1741-50.
