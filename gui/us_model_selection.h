//! \file us_modelselection.h
#ifndef US_MODEL_SELECTION_H
#define US_MODEL_SELECTION_H

// The only reason this is in the gui library is that it 
// instantiates a gui class.

#include "us_extern.h"
#include "us_femglobal.h"

class US_EXTERN US_ModelSelection
{
	public:

		static void selectModel( struct ModelSystem& );
		
   private:
      static void initAssoc1( struct ModelSystem&, 
                              unsigned int, unsigned int, unsigned int );
		
      static void initAssoc2( struct ModelSystem& );
};

#endif
