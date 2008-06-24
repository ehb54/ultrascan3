#ifndef US_HYDRODYN_PDBDEFS_H
#define US_HYDRODYN_PDBDEFS_H

#include <vector>
using namespace std;

// QT defs:
#include <qstring.h>

struct point
{
   float axis[3];
};

struct matrix
{
   float element[3][3];
};

struct PDB_atom
{
	unsigned int serial;
	QString name;
	QString altLoc;
	QString resName;
	unsigned int resSeq;
	QString iCode;
	struct point coordinate;
	float occupancy;
	float tempFactor;
	QString element;
	QString charge;
	unsigned int accessibility;

        // assigned in surfracer
        bool active;
        float asa;			  // maximum accessible surface area (A^2)
        struct residue *p_residue;        // NULL if not found
        struct atom *p_atom;              // NULL if not found
        float radius;

        // assigned after return from compute_asa()
        bool is_bead;                     // 0 = no, 1 = yes this is the bead info
        int bead_number;                  // sequence of bead #'s
        int bead_assignment;              // number in residue->r_bead[]
        int visibility;                   // 0 = hidden, 1 = exposed
        int chain;                        // 0 = main, 1 = side
        float bead_asa;
        int exposed_code;                 // 1 exposed, 6 side chain buried, 10 main chain buried
        bool bead_positioner;             // true if an atom had a bead positioner
        float mw;
        float bead_mw;
	point bead_position_coordinate;
	point bead_cog_coordinate;
	point bead_coordinate;
	int placing_method;	          // baric method (see struct bead->placing method, -1 undefined
};

struct PDB_chain	// chain in PDB file
{
   vector <struct PDB_atom> atom;
   QString chainID;
   QString segID;
};

struct PDB_model
{
	vector <struct PDB_chain> molecule;
	unsigned int model_id;
};


struct bead
{
	unsigned int hydration;			// number of waters bound
	unsigned int color;				// color of bead
	unsigned int placing_method;	// baric method
											// 0: place the bead at the center of gravity of all atoms
											// 1: placing the bead at the position of the furthest atom
											// specified in the p3 file
											// 2: no positioning necessary (only one atom to place)
	unsigned int visibility;		// 0 = hidden
											// 1 = exposed
	unsigned int chain;				// 0 = main chain
											// 1 = side chain
	float	       volume;				// anhydrous bead volume
	float	       mw;					// bead mw
};

struct hybridization
{
	QString name; 						// for example, N4H3
	float mw;						// molecular weight of hybridization
	float radius;						// radius of hybridization
};

struct atom
{
	QString name; 						// for example, CA
	hybridization hybrid;			// hybridization of atom
	unsigned int bead_assignment;	// which bead this atom belongs to
	bool positioner;					// does this atom control position? (yes=1, no=0)
	unsigned int serial_number;	// the serial number the atom occupies in the residue
	unsigned int chain;				// 0 = main chain
											// 1 = side chain
											// 2 = prosthetic group
											// 3 = undefined
											// 4 = other
};

struct residue
{
	QString name;
	unsigned int type;				// 0: amino acid
											// 1: sugar
											// 2: nucleotide
											// 3: lipid
											// 4: detergent
											// 5: other
	float molvol;						// the molecular volume of the residue
	float asa;							// maximum accessible surface area (A^2)
	vector <struct atom> r_atom;	// the atoms in the residue
	vector <struct bead> r_bead;	// the beads used to describe the residue
};

#endif

