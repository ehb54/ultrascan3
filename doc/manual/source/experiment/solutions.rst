===============================================
Solutions: Define the Solutions per Channel
===============================================

.. toctree:: 
  :maxdepth: 3

.. contents:: Index
  :local: 

Panel Tab Options: 
===================

In each panel, tabs are visible at the top of the window to enable the user to move to another panel, to perform specific experiment subtasks. 

1. `General <general.html>`_: A panel whose primary purpose is to specify the experiment run ID (a description string), select a parent project name, and possibly change data source (database or local disk) or database investigator name. 
2. `Lab/Rotor <rotor.html>`_: A panel whose primary purpose is to select the Laboratory, Rotor, and Calibration values for the experiment. 
3. `Speeds <speeds.html>`_: A panel whose primary primary purpose is to specify one or more speed steps. For each step, speeds and durations may be given. 
4. `Cells <cells.html>`_: A panel whose primary purpose is to select the centerpieces (or counterbalance) for the cells, along with a quartz/sapphire windows selection. 
5. :ref:`Solutions <solutions>`: A panel whose primary purpose is to specify the solution to be used in each cell/channel. 
6. `Optics <optics.html>`_: A panel whose primary purpose is to select the optical system used to scan.  
7. `Range <range.html>`_: A panel whose primary purpose is to select the wavelength ranges and cell radius measured.  
8. `Submit <submit.html>`_: A panel whose primary purpose is save and upload the protocol to the Optima/Instrument and database.  


Solutions Panel:
===================

.. _solutions:

Using this panel, you can specify the solutions used in each channel. :ref:`Comments <comments>` can also be included and appended to the solution name. 
The `Solutions Management <../solutions/index.html>`_ module help page has detailed explanations of these items. 


.. image:: ../_static/images/experiment_solutions.png
    :align: center

.. rst-class::
    :align: center

    **Solutions Panel**

.. _comments:


.. image:: ../_static/images/experiment_solutions_comments.png
    :align: center

.. rst-class::
    :align: center

    **Solution Name**


Shared Panel Functions:
========================

A panel status box and multiple buttons are shared by all panels. The `General <general.html>`_ tab help page has detailed explanations of these items. 