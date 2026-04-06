===========================================================================
Getting Started 
===========================================================================


.. toctree:: 
  :maxdepth: 3

.. contents:: Analysis Steps:
  :local: 


Overview of Ultrascan-III
===========================

UltraScan is a comprehensive data analysis software package for hydrodynamic data from analytical ultracentrifugation experiments. The software features an integrated data editing and analysis environment with a portable graphical user interface. UltraScan provides the highest possible resolution for sedimentation velocity experiments using high-performance computing modules for 2-dimensional spectrum analysis, genetic algorithm, and for Monte Carlo analysis. A MySQL database backend is integrated for data management, and the UltraScan LIMS system provides web-based access to the database and supercomputing functionality. Supercomputing resources are provided through the NSF-funded UltraScan Science Gateway, which currently is supported by the NSF-ACCESS infrastructure, which provides cloud storage and high performance supercomputing resources to the UltraScan project. Special care has been taken in the design of the software to guarantee an optimum in flexibility, user-friendliness, stability and convenience. The software is written in C++.

The software features popular sedimentation velocity and equilibrium analysis methods as well as simulation modules, utilities, and editing modules with support for velocity and equilibrium experiments, single and multi-channel centerpieces, absorbance and interference optics. All analysis methods generate ASCII formatted data suitable for import into third-party plotting programs, and contains a built-in postscript printer engine to generate publication - quality graphical representations of analysis data.

     

The LIMS component of UltraScan, coupled with the high-performance computing backend dramatically improves analysis speed and throughput and allows simultaneous analysis of a virtually unlimited number of experiments. Any institution, academic or for profit can obtain free data storage and supercomputing analysis resources through an NSF TeraGrid allocation grant issued to Borries Demeler. Please contact us for assistance with setting up a secure database and supercomputer access. 

UltraScan-III Installation:
============================

`Download the software <https://ultrascan3.aucsolutions.com/download.php>`_. UltraScan-III is available for free download in binary or source-code format for three major operating systems: Windows, Macintosh, and Linux.

The UltraScan is multi-platform and can be installed on Windows, Macintosh, and Linux computers. For Linux, several popular distributions are supported. Each operating system has operating system specific instructions:

    * Microsoft Windows: Download the installation file and execute it on your computer. When prompted for an installation directory, please choose C:\Program Files\UltraScan, this is the expected path. If your international version of Windows has a different path, please create this subdirectory first before installation. On Windows 7, also install in C:\Program Files\UltraScan, not in C:\Program Files (x86)\UltraScan. You may have to run the program as Administrator.

    * Mac OS-X: Download and Double-click the installation package. If you have not previously installed XQuartz, you will be requested to do so. XQuartz can be downloaded from https://www.xquartz.org.

    * Linux, Unix/X11: To install the tar.xz package for any distribution, please follow these steps:

After downloading the tar.xz package, you need to uncompress and extract it in a location of your choice, for example, /usr/lib or /usr/local or /usr/local/lib. For the purpose of this document, it is assumed you are going to install UltraScan in /usr/lib and /path is the path to your downloaded file.

    * cd /usr/lib
    * tar Jxf /path/ultrascan-version.tar.xz

    At this point, a new directory will be created (you can rename it without issue) Under that directory will be a bin subdirectory. In the bin subdirectory are two scripts:

    * us.sh
    * us_somo.sh

    You can run these to start the UltraScan GUI and the US-SOMO GUI respectively. 

`Register the software <register.html>`_. UltraScan development is funded by NIH and your registration helps us document usage of the software to NIH. Only operating system, and the number of active users for any given year are shared with the NIH. Registrations are free, and valid for one year. After that they have to be renewed (also free).

