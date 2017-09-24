#if __cplusplus >= 201103L
#define USE_RAND_RNG
#define USE_SPHERE_CENTERS_MODEL 
#define USE_NANOFLANN_SORT_SCM

#include <future>
#include <chrono>
#include "../include/us_hydrodyn_zeno.h"
#undef R
#include "qprogressbar.h"
extern QProgressBar *zeno_progress;

#include <fstream>
class zeno_fout {
public:
   zeno_fout( const char * fname ) { ofs.open( fname ); };
   ~zeno_fout() { if ( ofs.is_open() ) { ofs.close(); } };
   std::ofstream ofs;
};
extern zeno_fout* zeno_cxx_fout;
#include "../include/us_zeno_cxx_nf.h"
// *** START *** PointModel/NanoFLANNPoint.cc ***
/// ================================================================
/// 
/// Disclaimer:  IMPORTANT:  This software was developed at the
/// National Institute of Standards and Technology by employees of the
/// Federal Government in the course of their official duties.
/// Pursuant to title 17 Section 105 of the United States Code this
/// software is not subject to copyright protection and is in the
/// public domain.  This is an experimental system.  NIST assumes no
/// responsibility whatsoever for its use by other parties, and makes
/// no guarantees, expressed or implied, about its quality,
/// reliability, or any other characteristic.  We would appreciate
/// acknowledgement if the software is used.  This software can be
/// redistributed and/or modified freely provided that any derivative
/// works bear some notice that they are derived from it, and any
/// modified versions bear some notice that they have been modified.
/// 
/// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Fri Aug 15 14:35:07 2014 EDT
//
// Time-stamp: <2015-08-31 17:31:21 dcj>
//
// ================================================================

#include <limits>
#include <cassert>
#include <iostream>

// inlined #include "../SpherePoint/QuasiRandomSpherePointsRegularCube.h"

// inlined #include "NanoFLANNPoint.h"

// ================================================================

NanoFLANNPoint::NanoFLANNPoint()
  : originalPoints(NULL),
    originalNormals(NULL),
    nanoFLANNInstance() {

}

NanoFLANNPoint::~NanoFLANNPoint() {
  delete nanoFLANNInstance.dataset;

  delete nanoFLANNInstance.kdTree;
}

void 
NanoFLANNPoint::preprocess(std::vector<Vector3<double> > const * points,
			   std::vector<Vector3<double> > const * normals,
			   double fracErrorBound) {

  originalPoints  = points;
  originalNormals = normals;

  //build kdTree

  DatasetAdaptorType * dataset = new DatasetAdaptorType(originalPoints);

  nanoFLANNInstance.dataset = dataset;

  nanoFLANNInstance.kdTree = new KDTreeType(3, *dataset);

  nanoFLANNInstance.kdTree->buildIndex();
}

void 
NanoFLANNPoint::findNearestPoint(Vector3<double> const * queryPoint,
				 double fracErrorBound,
				 Vector3<double> & nearestPoint,
				 Vector3<double> & nearestPointNormal) 
  const {

  double query[3];

  for (int i = 0; i < 3; i++) {
    query[i] = queryPoint->get(i);
  }

  //perform searches

  int resultIndex = -1;
  double distSqr  = -1;

  nanoflann::SearchParams params;
  params.eps = fracErrorBound;

  nanoflann::KNNResultSet<double, int> resultSet(1);
  resultSet.init(&resultIndex, &distSqr);

  nanoFLANNInstance.kdTree->findNeighbors(resultSet, query, 
					  params);

  nearestPoint       = originalPoints->at(resultIndex);
  nearestPointNormal = originalNormals->at(resultIndex);
}

void 
NanoFLANNPoint::printDataStructureStats() 
const {
  zeno_cxx_fout->ofs << "Used memory (MB): " << std::endl;

  zeno_cxx_fout->ofs << nanoFLANNInstance.kdTree->usedMemory() / 1000000.
	    << std::endl;

  zeno_cxx_fout->ofs << std::endl;
}

void 
NanoFLANNPoint::printSearchStats() 
const {

}

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** PointModel/NanoFLANNPoint.cc ***

#endif
