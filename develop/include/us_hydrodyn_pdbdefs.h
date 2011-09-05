#ifndef US_HYDRODYN_PDBDEFS_H
#define US_HYDRODYN_PDBDEFS_H

#include <vector>
#include <map>
using namespace std;

// QT defs:
#include <qstring.h>

class point
{
 public:
   float axis[3];
   friend ostream& operator<<(ostream&, const point&);
};

struct matrix
{
   float element[3][3];
};

struct saxs
{
   QString saxs_name;            // name of atom, for example, CD+2
   float   a[4];                 // a coefficients
   float   b[4];                 // b coefficients
   float   c;                    // c coefficient
   float   volume;               // atomic volume
};

struct rotamer_atom
{
   QString name;
   point   coordinate;
   // float   occupancy;
   // QString tempFactor;
};

struct rotamer
{
   QString name;
   QString residue;
   QString extension;

   vector < float >                  dihedral_angles;
   vector < rotamer_atom >           side_chain;
   vector < rotamer_atom >           waters;
   vector < vector < QString > >     water_positioning_atoms;
   map    < QString, rotamer_atom >  atom_map;
};

struct PDB_atom
{
   unsigned int serial;
   QString name;
   QString orgName;
   QString altLoc;
   QString resName;
   QString chainID;
   QString resSeq;
   QString iCode;
   point coordinate;
   float occupancy;
   float tempFactor;
   QString element;
   QString charge;
   unsigned int accessibility;

   // assigned in surfracer
   bool active;
   float asa;           // maximum accessible surface area (A^2)
   struct residue *p_residue;        // NULL if not found
   struct atom *p_atom;              // NULL if not found
   float radius;

   // assigned after return from compute_asa()
   bool is_bead;                     // 0 = no, 1 = yes this is the bead info
   int bead_number;                  // sequence of bead #'s
   int bead_assignment;              // position in residue->r_bead[]
   int atom_assignment;              // position in residue->r_atom[]
   int visibility;                   // 0 = hidden, 1 = exposed
   int chain;                        // 0 = main, 1 = side
   int org_chain;                    // 0 = main, 1 = side
   unsigned int type;                // 0: amino acid, etc.
   float bead_asa;
   float ref_asa;                    // reference asa from the residue table
   int exposed_code;                 // 1 exposed, 6 side chain buried, 10 main chain buried
   bool bead_positioner;             // true if an atom had a bead positioner
   float mw;
   float bead_mw;
   float bead_recheck_asa;
   float bead_cog_mw;                // mw of those atoms contributing to the bead_cog
   point bead_position_coordinate;
   point bead_cog_coordinate;
   point bead_coordinate;
   bool normalized_ot_is_valid;      // true if the normalized ot is valid computed
   point normalized_ot;              // the ot
   float bead_hydration;
   unsigned int bead_color;
   float bead_ref_volume;            // this is taken from the bead structure+hydration
   float bead_ref_volume_unhydrated; // this is taken from the bead structure
   float bead_ref_mw;                // ditto
   float bead_computed_radius;       // from ref_volume
   float bead_actual_radius;         // used for radial reduction % computation, does not get reduced
   int placing_method;             // baric method (see struct bead->placing method, -1 undefined

  //        float vol_intersection            // temporary value used in popping
   vector <struct PDB_atom *> all_beads;  // this is used to keep track of beads that have been popped together
   QString residue_list;             // for loaded bead models
   int group;                        // used in surfracer for breaking up groups of atoms
   QString count_idx;                // used in us_hydrodyn for backtracking on bead/atom exceptions
   float atom_hydration;      // used for atob grid hydration
   saxs saxs_data;
   QString saxs_name;
   QString hybrid_name;
   int hydrogens;    
   float        saxs_excl_vol;   // SAXS excluded volume value
};

struct PDB_chain   // chain in PDB file
{
   vector <struct PDB_atom> atom;
   QString chainID;
   QString segID;
   float mw;                                // mw of chain
};

struct PDB_model
{
   vector <struct PDB_chain> molecule;
   vector <struct residue> residue;         // keep track of amino acid sequence in molecule for vbar calculation
   float vbar;
   float mw;                                // mw of model
   unsigned int model_id;
};

struct bead
{
   float        hydration;       // number of waters bound
   unsigned int color;           // color of bead
   unsigned int placing_method;  // baric method
                                 // 0: place the bead at the center of gravity of all atoms
                                 // 1: placing the bead at the position of the furthest atom
                                 // specified in the p3 file
                                 // 2: no positioning necessary (only one atom to place)
   unsigned int visibility;      // 0 = hidden
                                 // 1 = exposed
   unsigned int chain;           // 0 = main chain
                                 // 1 = side chain
   float        volume;          // anhydrous bead volume
   float        mw;              // bead mw
   bool         hydration_flag;  // false = use sum of atom's hydrations, true = bead hydration overrides
   float        atom_hydration;  // number of waters bound based upon sum of atoms' hydrations
};

struct saxs_options
{
   float   water_e_density;      // water electron density in e/A^3

   float   h_scat_len;           // H scattering length (*10^-12 cm)
   float   d_scat_len;           // D scattering length (*10^-12 cm)
   float   h2o_scat_len_dens;    // H2O scattering length density (*10^-10 cm^2)
   float   d2o_scat_len_dens;    // D2O scattering length density (*10^-10 cm^2)
   float   d2o_conc;             // D2O concentration (0 to 1)
   float   frac_of_exch_pep;     // Fraction of exchanged peptide H (0 to 1)

   float   wavelength;           // scattering wavelengths
   float   start_angle;          // start angle
   float   end_angle;            // ending angle
   float   delta_angle;          // angle stepsize
   float   start_q;              // start q
   float   end_q;                // ending q
   float   delta_q;              // q stepsize
   float   max_size;             // maximum size (A)
   float   bin_size;             // Bin size (A)
   int     curve;                // 0 = raw, 1 = saxs, 2 = sans
   bool    hydrate_pdb;          // Hydrate the PDB model? (true/false)
   int     saxs_sans;            // 0 = saxs, 1 = sans

   // saved paths

   QString path_load_saxs_curve;
   QString path_load_gnom;
   QString path_load_prr;

   // guinier controls

   bool             guinier_csv;
   QString          guinier_csv_filename;

   double           qRgmax;
   double           qstart;
   double           qend;
   unsigned int     pointsmin;
   unsigned int     pointsmax;

   bool             normalize_by_mw;
   
   // options for saxs/sans iq curve computation

   bool    saxs_iq_native_debye;
   bool    saxs_iq_native_hybrid;
   bool    saxs_iq_native_hybrid2;
   bool    saxs_iq_native_hybrid3;
   bool    saxs_iq_native_fast;
   bool    saxs_iq_native_fast_compute_pr;
   bool    saxs_iq_foxs;
   bool    saxs_iq_crysol;

   bool    sans_iq_native_debye;
   bool    sans_iq_native_hybrid;
   bool    sans_iq_native_hybrid2;
   bool    sans_iq_native_hybrid3;
   bool    sans_iq_native_fast;
   bool    sans_iq_native_fast_compute_pr;
   bool    sans_iq_cryson;

   unsigned int hybrid2_q_points;

   bool    iq_ask;                // ask when "compute saxs curve" is pressed

   bool    iq_scale_ask;
   bool    iq_scale_angstrom;
   bool    iq_scale_nm;

   // crysol specific options
   unsigned int crysol_max_harmonics;
   unsigned int crysol_fibonacci_grid_order;
   float        crysol_hydration_shell_contrast;

   float   fast_bin_size;
   float   fast_modulation;

   bool    crysol_default_load_difference_intensity;
   bool    crysol_version_26;

   // bead model control
   bool    compute_saxs_coeff_for_bead_models;
   bool    compute_sans_coeff_for_bead_models;
   QString default_atom_filename;
   QString default_hybrid_filename;
   QString default_saxs_filename;
   QString default_rotamer_filename;

   double  steric_clash_distance;

   bool    disable_iq_scaling;
   bool    autocorrelate;
   bool    hybrid_radius_excl_vol;
   float   scale_excl_vol;
   bool    subtract_radius;
   float   iqq_scale_minq;
   float   iqq_scale_maxq;

   bool    iqq_scale_nnls;

   bool    iqq_scale_linear_offset;
   bool    iqq_scale_chi2_fitting;
   bool    iqq_expt_data_contains_variances;

   bool    iqq_ask_target_grid;
   bool    iqq_scale_play;

   float   swh_excl_vol;

   QString iqq_default_scaling_target;

   bool    saxs_iq_hybrid_adaptive;
   bool    sans_iq_hybrid_adaptive;

   bool    bead_model_rayleigh;
};

struct hybridization
{
   QString name;                 // for example, N4H3
   float   mw;                   // molecular weight of hybridization
   float   radius;               // radius of hybridization
   QString saxs_name;            // name for SAXS coefficients
   float   scat_len;             // Scattering length in H20 (*10^-12 cm)
   int     exch_prot;            // Number of exchangable protons
   int     num_elect;            // Number of electrons
   int     hydrogens;            // Number of hydrogens
};

struct atom
{
   QString name;                 // for example, CA
   hybridization hybrid;         // hybridization of atom
   unsigned int bead_assignment; // which bead this atom belongs to
   bool         positioner;      // does this atom control position? (yes=1, no=0)
   unsigned int serial_number;   // the serial number the atom occupies in the residue
   bool         tmp_flag;        // used for finding missing residues
   bool         tmp_used;        // used for avoiding duplicate usage
   float        saxs_excl_vol;   // SAXS excluded volume value
   float        hydration;       // atomic hydration / needed float for ABB
};

struct residue
{
   QString      name;
   QString      unique_name;
   QString      comment;
   QChar        onecharname;
   unsigned int type;            // 0: amino acid
                                 // 1: sugar
                                 // 2: nucleotide
                                 // 3: lipid
                                 // 4: detergent
                                 // 5: other
   float molvol;                 // the molecular volume of the residue
   float vbar;                   // the partial specific volume of the residue
   float asa;                    // maximum accessible surface area (A^2)
   vector <struct atom> r_atom;  // the atoms in the residue
   vector <struct bead> r_bead;  // the beads used to describe the residue
};

#endif

