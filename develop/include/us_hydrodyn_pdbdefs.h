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

struct model
{
	vector <struct chain> molecule;
	unsigned int model_id;
};

struct chain	// chain in PDB file
{
   vector <unsigned int> serial;
   vector <QString> name;
   vector <QString> altLoc;
   vector <QString> resName;
   QString chainID;
   vector <unsigned int> resSeq;
   vector <QString> iCode;
   vector <struct point> coordinate;
   vector <float> occupancy;
   vector <float> tempFactor;
   QString segID;
   vector <QString> element;
   vector <QString> charge;
};

struct bead
{
	unsigned int hydration;			// number of waters bound
	unsigned int color;				// color of bead
	unsigned int placing_method;	// baric method
											// 0: place the bead at the center of gravity of all atoms  
											// 1: placing the bead at the position of the furthest atom specified in the p3 file
											// 2: no positioning necessary (only one atom to place)
	unsigned int visibility;		// 0 = hidden
											// 1 = exposed
	unsigned int chain;				// 0 = main chain
											// 1 = side chain							
//	vector <struct atom> atom;
};

struct atom
{
	QString name; // for example, Ca
	float mw;
	float radius;
	unsigned int bead_assignment;
	bool positioner;
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
	float molvol;						// the molecular volume of the residue
	float asa;							// maximum accessible surface area (A^2)
	vector <struct atom> r_atom;	// the atoms in the residue
	vector <struct bead> r_bead;	// the beads used to describe the residue
};

#endif

