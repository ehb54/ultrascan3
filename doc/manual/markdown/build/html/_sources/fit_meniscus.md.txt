# Fit Meniscus

This module enables you to modify an Edited data set’s meniscus value based on meniscus-fit models. A polynomial fit to a set of meniscus-RMSD points is plotted and its low point presented as the default new meniscus value. That value - or a manually modified one - can then replace the value in the associated Edited data set. The model with closest meniscus value in the set of fit-meniscus models that were obtained by a front-end or back-end meniscus fit 2DSA run can be preserved, while all other models in the set (and any associated noises) are removed.

&nbsp;

![fit_meniscus.png](:../../../../images/fit_meniscus.png)

## Functions

| Function | usage |
| --- | --- |
| **(Meniscus-RMSD points)** | The main left-side text box displays a list of meniscus-RMSD values from a selected fit output by 2DSA. The fit to display is chosen by a *File->Load* selection in a [**FitMen Load Dialog**](../../../manual/html/fit_men-load.img.html) that allows navigation to a ./results/runID directory and choice of a fitmen.dat file. |
| **(right side plot)** | This plot shows a yellow curve connecting all fitmen radius-RMSD points; a red fitted polynomial curve; and a vertical bar with annotation delineating the meniscus value at the low point of the polynomial curve. |
| **Status:** | A text box here displays the status of database scan or data load. |
| **Fit Order** | The counter here may be modified to select the order of the polynomial to fit to meniscus-radius points. |