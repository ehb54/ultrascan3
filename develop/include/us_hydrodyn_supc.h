#ifndef US_HYDRODYN_SUPC_H
#define US_HYDRODYN_SUPC_H

#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>

struct dati1
{
    float x, y, z;		/* coordinates of the bead center    */
    float r;			/* hydrated radius of the bead       */
    float ru;			/* unhydrated radius of the bead     */
    int m;			/* mass of the bead                  */
    int col;			/* color of the bead                 */
    char *cor;			/* correspondence between beads and AA */
};
int us_hydro_supc_main();

#endif
