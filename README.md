[![CodeQL](https://github.com/ehb54/ultrascan3/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/ehb54/ultrascan3/actions/workflows/codeql-analysis.yml)

This is the home of the UltraScan GUI software source code.
More information is available [here](https://ultrascan.aucsolutions.com)
Binaries are available [here]

### installation

The simplest way is to download and install a precompiled [binary](https://www.ultrascan3.aucsolutions.com/download.php)

### building
 
If you wish to compile from source, here are some notes to get you started.

You will need [qt5](https://doc.qt.io/qt-5/gettingstarted.html) development and [qwt](https://qwt.sourceforge.io/) installed.

We typically compile from source as in [ansible roles build-qt, build-qwt & build-us3](https://github.com/KJSain/us3lims-roles/tree/master/roles)

One could also take hints from this [Dockerfile](https://github.com/ehb54/ultrascan3/blob/master/admin/codeql/docker/docker/Dockerfile) which uses an Ubuntu 20.04 container and qt pacakges.

Please create an issue or [contact us](https://www.ultrascan3.aucsolutions.com/contacts.php) if you have further questions.
