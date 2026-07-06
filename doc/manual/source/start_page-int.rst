=======================================
Analysis of Interacting Solutes
=======================================


.. toctree:: 
  :maxdepth: 1

.. contents:: Index
  :local: 
  


Step 1: 2DSA refinement 
------------------------

#. Follow steps 1-6 in the :doc:`2DSA Flowchart <start_page>`  to perform
a 2DSA analysis and fit the boundary conditions and the associated time-
and radially-invariant noises using iterative refinements.

Step 2: Define the model
-------------------------

#. Open the Discrete Model Genetic Algorithm (DMGA) function from the velocity menu.
#. Select **Define Base Model** to create a new model to be fitted.
#. The Model Editor window will open an existing model, or allow the user to create a new model.
#. To create a new model, select **Create New Model** and change the name in the **Model Description** text box.
#. Select **Manage Components** to open the Component window and to define the analytes in the model.
#. Click on **New Analyte** to select a previously entered analyte from the analyte management window and **Accept**.
#. **For non-interacting models** - Update the signal concentration to the **partial concentration** of each solute from a :ref:`GA statistical report <ga-stats-report>`. 
#. **For interacting models** - Update the signal concentration of the reactants to the **total concentration** from the :ref:`2DSA-IT results <fe-match-report>`. The product concentration should remain 0 and allow the DMGA fit to simulate the concentration based on the kD and Ka ranges to be provided. 

.. note::
  During DMGA fitting, the concentration of each analyte is simulated based on the user-defined ranges of kD and Ka. For best results, initial fitting should be performed using broad log-scale ranges for both parameters, enabling the algorithm to approximate the correct values efficiently. These ranges should then be progressively refined through iterative fitting until the optimal kD and Ka values are determined.

#. Review the parameters carefully and once satisfied, click **Accept**. 
#. The Component dialog will close. Click **Manage Associations** to define the models. 
#. The analytes will be listed in the **Association** Text box. Drag each analyte into the Analyte and Product positions. 
#. For each reaction, add the approximate K_dissociation (kD) and K_off Rate (Ka). 
#. Multiple model reactions can be made. Models can also be deleted by clicking the **D** button next to the row number. 
#. Review the reactions carefully and once satisfied, click **Accept**. 
#. Once both Components and Associations have been defined, click **Accept** in the :doc:`Model Editor <model_editor>` window. 
#. Click **Save Base Model** in the DMGA main window and edit the name of the model and save.  


Step 3: Define the constraints
-------------------------------

#. If a constraints has previously been defined, click **Load DMGA constraints** and select the model from the :ref:`Load Distribution Model <fe-model-loader>` window and click **Accept**.  
#. Click **Define Constraints** in the DMGA main window and the :doc:`DMGA Constraints Editor <dmga_init_constr>` will open. 

.. note::
  For interacting models, only for reactant analyte parameters can be floated.  

#. Select three Parameters that will be floated by check the **Float?** checkbox next to each parameter. 
#. Enter the Low and High limits for each selected parameter. 
#. Review the constraints carefully and once satisfied, click **Accept** and **Save Constraints** in the DMGA main window and edit the name of the constraints model and save.  

Step 4: Perform DMGA Monte Carlo Analysis
-----------------------------------------------

#. Log in and select the run and triples in `USLIMS <https://uslims.aucsolutions.com/index.php>`_.
#. Locate the dataset and select **Setup Discrete Model GA Control**. 
#. Select 8-10 Monte Carlo iterations and submit data to DMGA analysis, but do not refit time or radially invariant noise.

Step 5: Review parameter results
---------------------------------

#. Visualize results by using the Velocity: :doc:`FE Model Viewer <fe_match/fe_match_simulate>`.
#. Review the statistical results by clicking :doc:`Advanced DMGA-MC Controls <fe_match/fe_dmga_adv>`. 


Step 6: Refine constraints
---------------------------

#. Edit the Constraints parameters to improve simulation fit and repeat steps 4-5. 