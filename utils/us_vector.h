//! \file us_vector.h
#ifndef US_VECTOR_H
#define US_VECTOR_H

#include <QtCore>
#include "us_extern.h"

//! \brief A collecion of vector routines for doubles.
class US_UTIL_EXTERN US_Vector : public QVector< double >
{
   public:
      //! \brief Create a vector of doubles
      //! \param size The length of the vector
      //! \param value The initial values of the vector  
      US_Vector      ( int, double = 0.0 );

      //! \brief Add two vectors
      //! \brief other The vector to be added
      void   add     ( const US_Vector& );

      //! \brief Add a constant to every element of a vector
      //! \brief value The constant to add to the vector elements
      void   add     ( double );

      //! \brief Calculate the dot product of two vectors
      //! \param other The second vector of the calculation
      double dot     ( const US_Vector& );

      //! \brief Multiply the elements of two vectors, element by element
      //! \param other The second vector of the calculation
      void   mult    ( const US_Vector& );

      //! \brief Scale a vector by a constant value
      //! \param value The constant to scale each element of the vector
      void   scale   ( double );

      //! \brief Calculate the Euclidean distance between two vectors
      //! \param other The second vector of the calculation
      double distance( const US_Vector& );

      //! \brief Calculate the L2 norm of the vector
      double L2norm( void );

      //! \brief Return the number of elements in the vector
      int    size( void ) const { return v.size(); };

      // Note: Trying to use [] as an lvalue gets really complicated
      // so it's not implemented here. See Stroustrup, 3rd Edition, 
      // Section 11.12

      //! \brief Assign a value to a vector element
      //! \param i The element to change
      //! \param d The value to assign to the element
      void   assign( int i, double d ){ v[ i ] = d; };

      //! \brief Return an element using the [] operator
      //! \param i The element to return
      double operator[]( int i ) const { return v[ i ]; };

   private:
      QVector< double > v;

};
#endif

