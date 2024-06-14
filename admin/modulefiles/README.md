## Modulefile for building and running ultrascan in $HOME/ultrascan3

 * This allows users to build and test various branches or local mods to ultrascan3
 * It was originally developed for linux for uslims.uleth.ca, but can work (with possibile modifications) on other systems
 * N.B. this file has qt & qwt version specfic information
   * typically placed in /etc/modulefiles/ultrascan/home-gui-build

### once installed in a typical enviornment
 * `$ module swap ultrascan/home-gui-build`
### restore original `/opt/ultrascan3` paths
 * `$ module swap ultrascan/gui`
