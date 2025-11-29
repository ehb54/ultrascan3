==========================================
Manage Data in Database and on Local Disk
==========================================

.. toctree:: 
  :maxdepth: 3

.. contents:: Index
  :local: 

This module is used to display the four main types of data that may exist for the user in the database and/or on local disk. They are: (1) Raw experiment data; (2) Edited experiment data; (3) Model; and (4) Noise. Besides presenting a tree view of the user's data and their relationships, this module provides a simple means of performing processing on data: (1) upload to DB; (2) download to local disk; (3) remove from DB or local. 
The primary end of any processing of the data is to achieve a syncronizing of data in the database and on local disk. Particularly when preparing for off-network work or when returning to the network after such work, it is desirable to insure that any new data produced has a presence on local disk for off-network work and in the database for normal day-to-day processing. Instances of any data need to be in sync on the two media. This module provides a means to accomplish that.   

Process:
========

    * **Database Password:** Upon opening, US_ManageData requires you to sign on to the database with your DB password. 
    * **Investigator:** For database work, you must specify your investigator name so you are limited to data that you have permission to access and modify. 
    * **Scan Data:** Initiate a scan of all your data on the database and local disk. You will then be presented a full tree view of that data. 
    * **Navigate and Process:** Once you have a tree view of your data, you may navigate it using expand/collapse buttons or the normal **+** and **-** mechanism for specific branch expand and collapse. Context menus at each row allow upload/download/remove/details. 


Initially you are presented with a small window to enter your database password. Then you will see the main window with a sample data tree that shows the kinds of states that data may be in (see tree help below).

.. image:: _static/images/
    :align: center

.. rst-class:: 
    :align: center

    **Data Main Window**