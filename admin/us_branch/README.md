## Utility program for building and running ultrascan in $HOME/ultrascan3

 * This allows users to build and test various branches or local mods to ultrascan3
 * Make sure to also install the home-gui-build modulefile (see admin/modulefiles)

### typical install
 * `$ cp -rp admin/us_branch /opt/`

### run options
```
/opt/us_branch/us_branch.php --help

module is-avail ultrascan/home-gui-build
usage: /opt/us_branch/us_branch.php {options}

utility checkout, build and run ultrascan3 from a specified branch
leverages module ultrascan/home-gui-build

Options

--help                : print this information and exit

--branch branchname   : checkout branch
--pullrequest id      : checkout from pull request
--build               : build 
--run                 : run
```

### notes 
 * if it for some reason fails, make sure to double check the `branchname` is correct
 * remove or rename the ultrascan3 subdirectory in your home directory
