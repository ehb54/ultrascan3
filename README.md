[![CodeQL](https://github.com/ehb54/ultrascan3/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/ehb54/ultrascan3/actions/workflows/codeql-analysis.yml)

This is the home of the UltraScan GUI software source code.
More information is available [here](https://ultrascan.aucsolutions.com)

The UltraScan project is supported by NIH Funding GM120600

### Licensing

This project is licensed under the Lesser General Public License (LGPL). You can find the LGPL license in the LICENSE file in the root directory of this project.

This project uses the Eigen library, which is licensed under the Mozilla Public License 2.0 (MPL2). 
You can find the Eigen source code and license at [Eigen GitHub repository](https://gitlab.com/libeigen/eigen).

### Installation

The simplest way is to download and install a precompiled [binary](https://www.ultrascan3.aucsolutions.com/download.php)

### Building
 
If you wish to compile from source, here are some notes to get you started.

You will need [qt5](https://doc.qt.io/qt-5/gettingstarted.html) development and [qwt](https://qwt.sourceforge.io/) installed.

We typically compile from source as in [ansible roles build-qt, build-qwt & build-us3](https://github.com/KJSain/us3lims-roles/tree/master/roles)

One could also take hints from this [Dockerfile](https://github.com/ehb54/ultrascan3/blob/master/admin/codeql/docker/docker/Dockerfile) which uses an Ubuntu 20.04 container and qt packages.

Please create an issue or [contact us](https://www.ultrascan3.aucsolutions.com/contacts.php) if you have further questions.

### Container environment

Assuming you are experienced with containers...

A containerized development image is also available
```docker pull ehb1/us3comp```

The source is in `/ultrascan3`

Run and export the containers display in Linux 
```docker run -it --rm -e DISPLAY=${DISPLAY} -v /tmp/.X11-unix:/tmp/.X11-unix -v $HOME/.Xauthority:/home/xterm/.Xauthority --hostname $(hostname) ehb1/us3comp bash```

N.B. the `--rm` above will delete the container after you exit, so any changes will be lost after exiting.

Update the ultrascan3 code ```cd /ultrascan3 && git pull```

Compile both `us` and `us3_somo` ```cd /ultrascan3 && ./makeall.sh && ./makesomo.sh```

Run the compiled application ```LD_LIBRARY_PATH=/ultrascan3/lib:/qwt-6.1.5/lib /ultrascan3/bin/us```

Note - subprocesses do not currently run in the container, so you must run any compiled application individually, e.g. `us_fematch`

