#ifndef US_SURFRACER_H
#define US_SURFRACER_H

int surfracer_main(float probe_radius, 
		    vector <residue> residue_list,
		    vector <PDB_model> *model_vector);

#define US_SURFRACER_ERR_MISSING_RESIDUE     1
#define US_SURFRACER_ERR_MISSING_ATOM        2
#define US_SURFRACER_ERR_MEMORY_ALLOC        3


// uncomment next line to include MSA and cavity code
// #define US_SURFRACER_COMPUTE_EXTRAS  

#endif

