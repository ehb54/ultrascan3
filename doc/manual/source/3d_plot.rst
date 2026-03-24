==============================================
3-Dimensional Plot Controls
==============================================


.. toctree:: 
  :maxdepth: 3

.. contents:: Index
  :local: 



Upon completion of FE Match simulation, a pair of enhanced plot controls dialogs appears. The **3D Plot** button in the first of these launches a 3-dimensional plot window whose primary control resides in this dialog. There are size, orientation, and other controls that are in the 3D dialog itself. A second dialog displays a set of data and residuals plots that are more refined and controllable than those in the main FE Match window.


.. image:: /_static/images/fe_match_sim5.png
    :align: center

.. rst-class:: 
    :align: center

    **3-D Plotting Controls**

3D Plot Control Functions:
-----------------------------

.. list-table::
  :widths: 20 50
  :header-rows: 0 

  * - **Dimensions:**
    -  The five pairs of check boxes allow the user a way to set the 3D plot X and Y dimension value types: MW; s :sub:`20,w`; D :sub:`20,w`; f; and f/f :sub:`0`.
  * - **Z-Scaling Factor:**  
    -  Set the scale factor for 3D Z values, to increase/decrease peak extents relative to X and Y.
  * - **Grid Resolution:**
    -  Set the number of pixels in X and Y for which Z raster values are calculated.
  * - **Peak Smoothing:**
    -  Set the smoothing factor applied to peaks.
  * - **Peak Width:**
    -  Set the factor that governs the spread of peaks.
  * - **3D Plot**
    -  Click this button initially to launch the 3D-plot window. Thereafter, when dimension value or other settings are changed, click this button for a re-draw of the 3D plot. 


**Window Controls**


.. list-table::
  :widths: 20 50
  :header-rows: 0 

  * - **Help** 
    - Display this detailed Fit Meniscus help.
  * - **Close** 
    - Close all windows and exit.

Model Solute 3-Dimensional Viewer
---------------------------------------

.. image:: /_static/images/fe_match_3dplot.png
    :align: center

.. rst-class:: 
    :align: center

    **Model Solute 3-Dimensional Viewer**

**Keys to control view:**
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* **Shft + Mouse scroll wheel** - expand the z dimension (Concentration in the 3-D Viewer)
* **Alt + hold right arrow key** - expand X axis (sedimentation - s*1e+12)
* **Alt + hold left arrow key** - contract X axis
* **Alt + hold up arrow key** - expand Y axis (Fricitional ratio - f/f0)
* **Alt + hold down arrow key** - expand Y axis
* **Mouse scroll wheel** - zoom in on all Dimensions
* **Hold left mouse key and move up/down** - rotate X axis 
* **Hold left mouse key and move left/right** - rotate along Y axis