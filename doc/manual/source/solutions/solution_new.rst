=======================================
Enter New Solution Tab
=======================================

.. toctree:: 
  :maxdepth: 3

.. contents:: Index
  :local: 


**Panel Tab Options:**

* `Select Solution <solution_select.html>`_ - A panel whose primary purpose is to select a Solution to return to the caller.
* :ref:`Enter New Solution <enter-new-solution>` - A panel whose primary purpose is to enter a brand new Solution, defined mostly by specifying components and each one's concentration.
* `Edit Existing Solution <solution_edit.html>`_ - A panel whose primary purpose is to change non-hydrodynamic characteristics of an already existing Solution.
* `Settings <solution_settings.html>`_ - A panel whose primary purpose is to set Database-or-Disk input, the investigator; or to synchronize the local Solution components file from the database.

Enter New Solution Panel
=========================

.. _enter-new-solution: 

Using this panel, you can create a new solution in the current database or on the local disk. Most commonly, you select or create analytes in the solution using the `Analyte Management <analyte/index.html>`_. As each is selected, you enter the molar ratio of each analyte, then select or create a buffer condition from the set of buffers in the `Buffer Management <buffer/index.html>`_. After providing a descriptive title, you add a spectrum of the solution by clicking `Manage Spectrum <analyte/analyte_new.html#add-analyte-spectrum>`_ or you can click on the Accept button to upload the solution to the DB or to local disk and to return to the Select Solution panel.

As with all panels, a set of tabs allows you to navigate to other panels in order to perform specialized subtasks relating to solution management. Links to and summaries of the panels are given in the final section of this page.

.. image:: ../_static/images/solution_new.png
    :align: center

.. rst-class:: 
    :align: center

    **Enter New solution Panel**
  
Functions:
-----------

.. list-table::
  :widths: 20 50

  * - **Click on project to select**
    - After clicking on the Query Projects button, this widget will contain a list of projects available in the current location (database or local disk). Click on a project in the widget to select.
  * - **Use Database**
    - Check to select read or write of the project definition to or from the database.
  * - **Use Local Disk**
    - Check to select read or write of the project definition to or from the hard disk.
  * - **Query Projects**
    - This button initiates a read of project definitions and population of the project list widget.
  * - **Save Project**
    - Saves the current project definition to the hard disk or database.
  * - **New Project**
    - Creates a new project by returning all values to their defaults. Work through each tab to change the current contents as appropriate, and then click on the Save Project button.
  * - **Delete Project**
    - Deletes the current project from the database or hard disk.
  * - **Project Name**
    - The name of the project currently selected in the Click on project to select list widget, or a new name when creating a new project definition in the database or on the hard disk.
  * - **Global Identifier**
    - Read-only global identifier of the project.
  * - **Reset**
    - Reset all project values to default setting.
  * - **Help**
    - Show this documentation.
  * - **Close**
    - Close the dialog and possibly return project selections to the caller.