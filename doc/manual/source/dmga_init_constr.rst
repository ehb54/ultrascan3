=============================================
Discrete Model Genetic Algorithm Constraints
=============================================


.. toctree:: 
  :maxdepth: 3

.. contents:: Index
  :local: 

The Discrete Model GA Constraints Editor is a sub-module of the `DMGA Initialization <dmga_init.html>`_ module that 
allows the user to define constraints for each parameter of the selected model, and to determine which parameters
are held fixed and which are floated. 


.. image:: _static/images/dmga_constr.png
    :align: center

.. rst-class::
    :align: center

     **DMGA Contraints**

Parameter Functions:
====================

For each component defined in the model, the following parameters can be defined:

  * **Vbar at 20 (mL/g):**
  * **Molecular Wt.(MW)**
  * **Frictional ratio (f/f0)**
  * **Sedimentation Coeff. (s)**
  * **Diffusion Coeff. (D)**
  * **Friction coeff.(f)**
  * **Partial Concentration**

Multiple parameter combinations can be selected for fitting, but hydrodynamic constraints restrict
the total number of parameters that can be fitted simultaneously. Select three out of the 6 possible
parameters to define each component by selecting the checkbox in the front of the parameter name.
Parameter values highlighted in white are writeable and can be changed in the white field. Greyed-out
values are read-only and can be recomputed with the **Re-compute unselected component parameter values**.
Any parameters selected for fitting need to be floated by selecting the **Float?** checkbox. Floated 
parameters require a **Low** and **High** limit between which the parameter is allowed to float. Select
parameters can be set to a logarithmic scale by checking the **LogSc?** checkbox.


Component Parameters:
======================

.. list-table::
  :widths: 20 50
  :header-rows: 0 
  
  * - **Components**
    -
  * - **Parameters: (Value, Low, High, Float checkbox)**
    -
  * - **Vbar at 20 (mL/g):**
    -
  * - **Molecular Wt.(MW)**
    -
  * - **Frictional ratio (f/f0)**
    -
  * - **Sedimentation Coeff. (s)**
    -
  * - **Diffusion Coeff. (D)**
    -
  * - **Friction coeff.(f)**
    -
  * - **Partial Concentration**
    -
  * - **Extinction (OD/(cm*mol)).**
    -
  * - **Wavelength (nm):**
    -
  * - **Oligomer**
    -
  * - **is (Reactant or Product)**
    -
  * - **Concentration Dependency of s (σ)**
    -
  * - **Concentration Dependency of D (δ)**
    -
  * - **Re-Compute unselected component attribute values**
    -

Association parameters:
=======================

.. list-table::
  :widths: 20 50
  :header-rows: 0 
  
  * - **Association (reactions)**
    -
  * - **K_Dissociation**
    -
  * - **K_off rate**
    -

Window controls:
================

.. list-table::
  :widths: 20 50
  :header-rows: 0 

  * - **Help**
    -
  * - **Cancel**
    -
  * - **Accept**  
    - 

