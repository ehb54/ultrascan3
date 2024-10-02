/*
*
* Template Numerical Toolkit (TNT): Linear Algebra Module
*
* Mathematical and Computational Sciences Division
* National Institute of Technology,
* Gaithersburg, MD USA
*
*
* This software was developed at the National Institute of Standards and
* Technology (NIST) by employees of the Federal Government in the course
* of their official duties. Pursuant to title 17 Section 105 of the
* United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility whatsoever for
* its use by other parties, and makes no guarantees, expressed or implied,
* about its quality, reliability, or any other characteristic.
*
*/

#ifndef TNT_H
#define TNT_H

//---------------------------------------------------------------------
// Define this macro if you want  TNT to track some of the out-of-bounds
// indexing. This can encur a small run-time overhead, but is recommended 
// while developing code.  It can be turned off for production runs.
// 
//       #define TNT_BOUNDS_CHECK
//---------------------------------------------------------------------
//

//#define TNT_BOUNDS_CHECK

#ifndef TNT_ARRAY1D_H
#define TNT_ARRAY1D_H

//#include <cstdlib>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <time.h>
#include <algorithm>

#ifdef TNT_BOUNDS_CHECK
#include <assert.h>
#endif

#ifndef TNT_I_REFVEC_H
#define TNT_I_REFVEC_H

#ifndef NULL
#define NULL 0
#endif

namespace TNT
{
/*
	Internal representation of ref-counted array.  The TNT
	arrays all use this building block.

	<p>
	If an array block is created by TNT, then every time 
	an assignment is made, the left-hand-side reference 
	is decreased by one, and the right-hand-side refernce
	count is increased by one.  If the array block was
	external to TNT, the refernce count is a NULL pointer
	regardless of how many references are made, since the 
	memory is not freed by TNT.


	
*/
template <class T>
class i_refvec
{


  private:
    T* data_;                  
    int *ref_count_;


  public:

			 i_refvec();
	explicit i_refvec(int n);
	inline	 i_refvec(T* data);
	inline	 i_refvec(const i_refvec &v);
	inline   T*		 begin();
	inline const T* begin() const;
	inline  T& operator[](int i);
	inline const T& operator[](int i) const;
	inline  i_refvec<T> & operator=(const i_refvec<T> &V);
		    void copy_(T* p, const T* q, const T* e); 
		    void set_(T* p, const T* b, const T* e); 
	inline 	int	 ref_count() const;
	inline  int is_null() const;
	inline  void destroy();
			 ~i_refvec();
			
};

template <class T>
void i_refvec<T>::copy_(T* p, const T* q, const T* e)
{
	for (T* t=p; q<e; t++, q++)
		*t= *q;
}

template <class T>
i_refvec<T>::i_refvec() : data_(NULL), ref_count_(NULL) {}

/**
	In case n is 0 or negative, it does NOT call new. 
*/
template <class T>
i_refvec<T>::i_refvec(int n) : data_(NULL), ref_count_(NULL)
{
	if (n >= 1)
	{
#ifdef TNT_DEBUG
		std::cout  << "new data storage.\n";
#endif
		data_ = new T[n];
		ref_count_ = new int;
		*ref_count_ = 1;
	}
}

template <class T>
inline	 i_refvec<T>::i_refvec(const i_refvec<T> &V): data_(V.data_),
	ref_count_(V.ref_count_)
{
	if (V.ref_count_ != NULL)
	    (*(V.ref_count_))++;
}


template <class T>
i_refvec<T>::i_refvec(T* data) : data_(data), ref_count_(NULL) {}

template <class T>
inline T* i_refvec<T>::begin()
{
	return data_;
}

template <class T>
inline const T& i_refvec<T>::operator[](int i) const
{
	return data_[i];
}

template <class T>
inline T& i_refvec<T>::operator[](int i)
{
	return data_[i];
}


template <class T>
inline const T* i_refvec<T>::begin() const
{
	return data_;
}



template <class T>
i_refvec<T> & i_refvec<T>::operator=(const i_refvec<T> &V)
{
	if (this == &V)
		return *this;


	if (ref_count_ != NULL)
	{
		(*ref_count_) --;
		if ((*ref_count_) == 0)
			destroy();
	}

	data_ = V.data_;
	ref_count_ = V.ref_count_;

	if (V.ref_count_ != NULL)
	    (*(V.ref_count_))++;

	return *this;
}

template <class T>
void i_refvec<T>::destroy()
{
	if (ref_count_ != NULL)
	{
#ifdef TNT_DEBUG
		std::cout << "destorying data... \n";
#endif
		delete ref_count_;

#ifdef TNT_DEBUG
		std::cout << "deleted ref_count_ ...\n";
#endif
		if (data_ != NULL)
			delete []data_;
#ifdef TNT_DEBUG
		std::cout << "deleted data_[] ...\n";
#endif
		data_ = NULL;
	}
}

/*
* return 1 is vector is empty, 0 otherwise
*
* if is_null() is false and ref_count() is 0, then
* 
*/
template<class T>
int i_refvec<T>::is_null() const
{
	return (data_ == NULL ? 1 : 0);
}

/*
*  returns -1 if data is external, 
*  returns 0 if a is NULL array,
*  otherwise returns the positive number of vectors sharing
*  		this data space.
*/
template <class T>
int i_refvec<T>::ref_count() const
{
	if (data_ == NULL)
		return 0;
	else
		return (ref_count_ != NULL ? *ref_count_ : -1) ; 
}

template <class T>
i_refvec<T>::~i_refvec()
{
	if (ref_count_ != NULL)
	{
		(*ref_count_)--;

		if (*ref_count_ == 0)
		destroy();
	}
}


} /* namespace TNT */





#endif
/* TNT_I_REFVEC_H */

namespace TNT
{

template <class T>
class Array1D 
{

  private:

	  /* ... */
    i_refvec<T> v_;
    int n_;
    T* data_;				/* this normally points to v_.begin(), but
                             * could also point to a portion (subvector)
							 * of v_.
                            */

    void copy_(T* p, const T*  q, int len) const;
    void set_(T* begin,  T* end, const T& val);
 

  public:

    typedef         T   value_type;


	         Array1D();
	explicit Array1D(int n);
	         Array1D(int n, const T &a);
	         Array1D(int n,  T *a);
    inline   Array1D(const Array1D &A);
	inline   operator T*();
	inline   operator const T*();
	inline   Array1D & operator=(const T &a);
	inline   Array1D & operator=(const Array1D &A);
	inline   Array1D & ref(const Array1D &A);
	         Array1D copy() const;
		     Array1D & inject(const Array1D & A);
	inline   T& operator[](int i);
	inline   const T& operator[](int i) const;
	inline 	 int dim1() const;
	inline   int dim() const;
              ~Array1D();


	/* ... extended interface ... */

	inline int ref_count() const;
	inline Array1D<T> subarray(int i0, int i1);

};

template <class T>
Array1D<T>::Array1D() : v_(), n_(0), data_(0) {}

template <class T>
Array1D<T>::Array1D(const Array1D<T> &A) : v_(A.v_),  n_(A.n_), 
		data_(A.data_)
{
#ifdef TNT_DEBUG
	std::cout << "Created Array1D(const Array1D<T> &A) \n";
#endif

}

template <class T>
Array1D<T>::Array1D(int n) : v_(n), n_(n), data_(v_.begin())
{
#ifdef TNT_DEBUG
	std::cout << "Created Array1D(int n) \n";
#endif
}

template <class T>
Array1D<T>::Array1D(int n, const T &val) : v_(n), n_(n), data_(v_.begin()) 
{
#ifdef TNT_DEBUG
	std::cout << "Created Array1D(int n, const T& val) \n";
#endif
	set_(data_, data_+ n, val);

}

template <class T>
Array1D<T>::Array1D(int n, T *a) : v_(a), n_(n) , data_(v_.begin())
{
#ifdef TNT_DEBUG
	std::cout << "Created Array1D(int n, T* a) \n";
#endif
}

template <class T>
inline Array1D<T>::operator T*()
{
	return &(v_[0]);
}


template <class T>
inline Array1D<T>::operator const T*()
{
	return &(v_[0]);
}



template <class T>
inline T& Array1D<T>::operator[](int i) 
{ 
#ifdef TNT_BOUNDS_CHECK
	assert(i>= 0);
	assert(i < n_);
#endif
	return data_[i]; 
}

template <class T>
inline const T& Array1D<T>::operator[](int i) const 
{ 
#ifdef TNT_BOUNDS_CHECK
	assert(i>= 0);
	assert(i < n_);
#endif
	return data_[i]; 
}


	

template <class T>
Array1D<T> & Array1D<T>::operator=(const T &a)
{
	set_(data_, data_+n_, a);
	return *this;
}

template <class T>
Array1D<T> Array1D<T>::copy() const
{
	Array1D A( n_);
	copy_(A.data_, data_, n_);

	return A;
}


template <class T>
Array1D<T> & Array1D<T>::inject(const Array1D &A)
{
	if (A.n_ == n_)
		copy_(data_, A.data_, n_);

	return *this;
}





template <class T>
Array1D<T> & Array1D<T>::ref(const Array1D<T> &A)
{
	if (this != &A)
	{
		v_ = A.v_;		/* operator= handles the reference counting. */
		n_ = A.n_;
		data_ = A.data_; 
		
	}
	return *this;
}

template <class T>
Array1D<T> & Array1D<T>::operator=(const Array1D<T> &A)
{
	return ref(A);
}

template <class T>
inline int Array1D<T>::dim1() const { return n_; }

template <class T>
inline int Array1D<T>::dim() const { return n_; }

template <class T>
Array1D<T>::~Array1D() {}


/* ............................ exented interface ......................*/

template <class T>
inline int Array1D<T>::ref_count() const
{
	return v_.ref_count();
}

template <class T>
inline Array1D<T> Array1D<T>::subarray(int i0, int i1)
{
	if ((i0 > 0) && (i1 < n_) || (i0 <= i1))
	{
		Array1D<T> X(*this);  /* create a new instance of this array. */
		X.n_ = i1-i0+1;
		X.data_ += i0;

		return X;
	}
	else
	{
		return Array1D<T>();
	}
}


/* private internal functions */


template <class T>
void Array1D<T>::set_(T* begin, T* end, const T& a)
{
	for (T* p=begin; p<end; p++)
		*p = a;

}

template <class T>
void Array1D<T>::copy_(T* p, const T* q, int len) const
{
	T *end = p + len;
	while (p<end )
		*p++ = *q++;

}


} /* namespace TNT */

#endif
/* TNT_ARRAY1D_H */

#ifndef TNT_ARRAY1D_UTILS_H
#define TNT_ARRAY1D_UTILS_H


namespace TNT
{


template <class T>
std::ostream& operator<<(std::ostream &s, const Array1D<T> &A)
{
    int N=A.dim1();

#ifdef TNT_DEBUG
	s << "addr: " << (void *) &A[0] << "\n";
#endif
    s << N << "\n";
    for (int j=0; j<N; j++)
    {
       s << A[j] << "\n";
    }
    s << "\n";

    return s;
}

template <class T>
std::istream& operator>>(std::istream &s, Array1D<T> &A)
{
	int N;
	s >> N;

	Array1D<T> B(N);
	for (int i=0; i<N; i++)
		s >> B[i];
	A = B;
	return s;
}



template <class T>
Array1D<T> operator+(const Array1D<T> &A, const Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() != n )
		return Array1D<T>();

	else
	{
		Array1D<T> C(n);

		for (int i=0; i<n; i++)
		{
			C[i] = A[i] + B[i];
		}
		return C;
	}
}

template <class T>
Array1D<T> operator-(const Array1D<T> &A, const Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() != n )
		return Array1D<T>();

	else
	{
		Array1D<T> C(n);

		for (int i=0; i<n; i++)
		{
			C[i] = A[i] - B[i];
		}
		return C;
	}
}


template <class T>
Array1D<T> operator*(const Array1D<T> &A, const Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() != n )
		return Array1D<T>();

	else
	{
		Array1D<T> C(n);

		for (int i=0; i<n; i++)
		{
			C[i] = A[i] * B[i];
		}
		return C;
	}
}


template <class T>
Array1D<T> operator/(const Array1D<T> &A, const Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() != n )
		return Array1D<T>();

	else
	{
		Array1D<T> C(n);

		for (int i=0; i<n; i++)
		{
			C[i] = A[i] / B[i];
		}
		return C;
	}
}

template <class T>
Array1D<T>&  operator+=(Array1D<T> &A, const Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() == n)
	{
		for (int i=0; i<n; i++)
		{
				A[i] += B[i];
		}
	}
	return A;
}

template <class T>
Array1D<T>&  operator-=(Array1D<T> &A, const Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() == n)
	{
		for (int i=0; i<n; i++)
		{
				A[i] -= B[i];
		}
	}
	return A;
}

template <class T>
Array1D<T>&  operator*=(Array1D<T> &A, const Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() == n)
	{
		for (int i=0; i<n; i++)
		{
				A[i] *= B[i];
		}
	}
	return A;
}

template <class T>
Array1D<T>&  operator/=(Array1D<T> &A, const Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() == n)
	{
		for (int i=0; i<n; i++)
		{
				A[i] /= B[i];
		}
	}
	return A;
}

} // namespace TNT

#endif

#ifndef TNT_ARRAY2D_H
#define TNT_ARRAY2D_H

namespace TNT
{

template <class T>
class Array2D 
{


  private:



  	Array1D<T> data_;
	Array1D<T*> v_;
	int m_;
    int n_;

  public:

    typedef         T   value_type;
	       Array2D();
	       Array2D(int m, int n);
	       Array2D(int m, int n,  T *a);
	       Array2D(int m, int n, const T &a);
    inline Array2D(const Array2D &A);
	inline operator T**();
	inline operator const T**();
	inline Array2D & operator=(const T &a);
	inline Array2D & operator=(const Array2D &A);
	inline Array2D & ref(const Array2D &A);
	       Array2D copy() const;
		   Array2D & inject(const Array2D & A);
	inline T* operator[](int i);
	inline const T* operator[](int i) const;
	inline int dim1() const;
	inline int dim2() const;
     ~Array2D();

	/* extended interface (not part of the standard) */


	inline int ref_count();
	inline int ref_count_data();
	inline int ref_count_dim1();
	Array2D subarray(int i0, int i1, int j0, int j1);

};


template <class T>
Array2D<T>::Array2D() : data_(), v_(), m_(0), n_(0) {} 

template <class T>
Array2D<T>::Array2D(const Array2D<T> &A) : data_(A.data_), v_(A.v_), 
	m_(A.m_), n_(A.n_) {}




template <class T>
Array2D<T>::Array2D(int m, int n) : data_(m*n), v_(m), m_(m), n_(n)
{
	if (m>0 && n>0)
	{
		T* p = &(data_[0]);
		for (int i=0; i<m; i++)
		{
			v_[i] = p;
			p += n;
		}
	}
}



template <class T>
Array2D<T>::Array2D(int m, int n, const T &val) : data_(m*n), v_(m), 
													m_(m), n_(n) 
{
  if (m>0 && n>0)
  {
	data_ = val;
	T* p  = &(data_[0]);
	for (int i=0; i<m; i++)
	{
			v_[i] = p;
			p += n;
	}
  }
}

template <class T>
Array2D<T>::Array2D(int m, int n, T *a) : data_(m*n, a), v_(m), m_(m), n_(n)
{
  if (m>0 && n>0)
  {
	T* p = &(data_[0]);
	
	for (int i=0; i<m; i++)
	{
			v_[i] = p;
			p += n;
	}
  }
}


template <class T>
inline T* Array2D<T>::operator[](int i) 
{ 
#ifdef TNT_BOUNDS_CHECK
	assert(i >= 0);
	assert(i < m_);
#endif

return v_[i]; 

}


template <class T>
inline const T* Array2D<T>::operator[](int i) const
{ 
#ifdef TNT_BOUNDS_CHECK
	assert(i >= 0);
	assert(i < m_);
#endif

return v_[i]; 

}

template <class T>
Array2D<T> & Array2D<T>::operator=(const T &a)
{
	/* non-optimzied, but will work with subarrays in future verions */

	for (int i=0; i<m_; i++)
		for (int j=0; j<n_; j++)
		v_[i][j] = a;
	return *this;
}




template <class T>
Array2D<T> Array2D<T>::copy() const
{
	Array2D A(m_, n_);

	for (int i=0; i<m_; i++)
		for (int j=0; j<n_; j++)
			A[i][j] = v_[i][j];


	return A;
}


template <class T>
Array2D<T> & Array2D<T>::inject(const Array2D &A)
{
	if (A.m_ == m_ &&  A.n_ == n_)
	{
		for (int i=0; i<m_; i++)
			for (int j=0; j<n_; j++)
				v_[i][j] = A[i][j];
	}
	return *this;
}




template <class T>
Array2D<T> & Array2D<T>::ref(const Array2D<T> &A)
{
	if (this != &A)
	{
		v_ = A.v_;
		data_ = A.data_;
		m_ = A.m_;
		n_ = A.n_;
		
	}
	return *this;
}



template <class T>
Array2D<T> & Array2D<T>::operator=(const Array2D<T> &A)
{
	return ref(A);
}

template <class T>
inline int Array2D<T>::dim1() const { return m_; }

template <class T>
inline int Array2D<T>::dim2() const { return n_; }


template <class T>
Array2D<T>::~Array2D() {}




template <class T>
inline Array2D<T>::operator T**()
{
	return &(v_[0]);
}
template <class T>
inline Array2D<T>::operator const T**()
{
	return &(v_[0]);
}

/* ............... extended interface ............... */
/**
	Create a new view to a subarray defined by the boundaries
	[i0][i0] and [i1][j1].  The size of the subarray is
	(i1-i0) by (j1-j0).  If either of these lengths are zero
	or negative, the subarray view is null.

*/
template <class T>
Array2D<T> Array2D<T>::subarray(int i0, int i1, int j0, int j1) 
{
	Array2D<T> A;
	int m = i1-i0+1;
	int n = j1-j0+1;

	/* if either length is zero or negative, this is an invalide
		subarray. return a null view.
	*/
	if (m<1 || n<1)
		return A;

	A.data_ = data_;
	A.m_ = m;
	A.n_ = n;
	A.v_ = Array1D<T*>(m);
	T* p = &(data_[0]) + i0 *  n_ + j0;
	for (int i=0; i<m; i++)
	{
		A.v_[i] = p + i*n_;

	}	
	return A;
}

template <class T>
inline int Array2D<T>::ref_count()
{
	return ref_count_data();
}



template <class T>
inline int Array2D<T>::ref_count_data()
{
	return data_.ref_count();
}

template <class T>
inline int Array2D<T>::ref_count_dim1()
{
	return v_.ref_count();
}




} /* namespace TNT */

#endif
/* TNT_ARRAY2D_H */

#ifndef TNT_ARRAY2D_UTILS_H
#define TNT_ARRAY2D_UTILS_H

namespace TNT
{


template <class T>
std::ostream& operator<<(std::ostream &s, const Array2D<T> &A)
{
    int M=A.dim1();
    int N=A.dim2();

    s << M << " " << N << "\n";

    for (int i=0; i<M; i++)
    {
        for (int j=0; j<N; j++)
        {
            s << A[i][j] << " ";
        }
        s << "\n";
    }


    return s;
}

template <class T>
std::istream& operator>>(std::istream &s, Array2D<T> &A)
{

    int M, N;

    s >> M >> N;

    Array2D<T> B(M,N);

    for (int i=0; i<M; i++)
       for (int j=0; j<N; j++)
       {
          s >>  B[i][j];
       }
    
    A = B;
    return s;
}


template <class T>
Array2D<T> operator+(const Array2D<T> &A, const Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() != m ||  B.dim2() != n )
		return Array2D<T>();

	else
	{
		Array2D<T> C(m,n);

		for (int i=0; i<m; i++)
		{
			for (int j=0; j<n; j++)
				C[i][j] = A[i][j] + B[i][j];
		}
		return C;
	}
}

template <class T>
Array2D<T> operator-(const Array2D<T> &A, const Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() != m ||  B.dim2() != n )
		return Array2D<T>();

	else
	{
		Array2D<T> C(m,n);

		for (int i=0; i<m; i++)
		{
			for (int j=0; j<n; j++)
				C[i][j] = A[i][j] - B[i][j];
		}
		return C;
	}
}


template <class T>
Array2D<T> operator*(const Array2D<T> &A, const Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() != m ||  B.dim2() != n )
		return Array2D<T>();

	else
	{
		Array2D<T> C(m,n);

		for (int i=0; i<m; i++)
		{
			for (int j=0; j<n; j++)
				C[i][j] = A[i][j] * B[i][j];
		}
		return C;
	}
}




template <class T>
Array2D<T> operator/(const Array2D<T> &A, const Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() != m ||  B.dim2() != n )
		return Array2D<T>();

	else
	{
		Array2D<T> C(m,n);

		for (int i=0; i<m; i++)
		{
			for (int j=0; j<n; j++)
				C[i][j] = A[i][j] / B[i][j];
		}
		return C;
	}
}





template <class T>
Array2D<T>&  operator+=(Array2D<T> &A, const Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() == m ||  B.dim2() == n )
	{
		for (int i=0; i<m; i++)
		{
			for (int j=0; j<n; j++)
				A[i][j] += B[i][j];
		}
	}
	return A;
}



template <class T>
Array2D<T>&  operator-=(Array2D<T> &A, const Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() == m ||  B.dim2() == n )
	{
		for (int i=0; i<m; i++)
		{
			for (int j=0; j<n; j++)
				A[i][j] -= B[i][j];
		}
	}
	return A;
}



template <class T>
Array2D<T>&  operator*=(Array2D<T> &A, const Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() == m ||  B.dim2() == n )
	{
		for (int i=0; i<m; i++)
		{
			for (int j=0; j<n; j++)
				A[i][j] *= B[i][j];
		}
	}
	return A;
}





template <class T>
Array2D<T>&  operator/=(Array2D<T> &A, const Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() == m ||  B.dim2() == n )
	{
		for (int i=0; i<m; i++)
		{
			for (int j=0; j<n; j++)
				A[i][j] /= B[i][j];
		}
	}
	return A;
}

/**
    Matrix Multiply:  compute C = A*B, where C[i][j]
    is the dot-product of row i of A and column j of B.


    @param A an (m x n) array
    @param B an (n x k) array
    @return the (m x k) array A*B, or a null array (0x0)
        if the matrices are non-conformant (i.e. the number
        of columns of A are different than the number of rows of B.)


*/
template <class T>
Array2D<T> matmult(const Array2D<T> &A, const Array2D<T> &B)
{
    if (A.dim2() != B.dim1())
        return Array2D<T>();

    int M = A.dim1();
    int N = A.dim2();
    int K = B.dim2();

    Array2D<T> C(M,K);

    for (int i=0; i<M; i++)
        for (int j=0; j<K; j++)
        {
            T sum = 0;

            for (int k=0; k<N; k++)
                sum += A[i][k] * B [k][j];

            C[i][j] = sum;
        }

    return C;

}

} // namespace TNT

#endif

#ifndef TNT_ARRAY3D_H
#define TNT_ARRAY3D_H

namespace TNT
{

template <class T>
class Array3D 
{


  private:
  	Array1D<T> data_;
	Array2D<T*> v_;
	int m_;
    int n_;
	int g_;


  public:

    typedef         T   value_type;

	       Array3D();
	       Array3D(int m, int n, int g);
	       Array3D(int m, int n, int g,  T val);
	       Array3D(int m, int n, int g, T *a);

	inline operator T***();
	inline operator const T***();
    inline Array3D(const Array3D &A);
	inline Array3D & operator=(const T &a);
	inline Array3D & operator=(const Array3D &A);
	inline Array3D & ref(const Array3D &A);
	       Array3D copy() const;
		   Array3D & inject(const Array3D & A);

	inline T** operator[](int i);
	inline const T* const * operator[](int i) const;
	inline int dim1() const;
	inline int dim2() const;
	inline int dim3() const;
               ~Array3D();

	/* extended interface */

	inline int ref_count(){ return data_.ref_count(); }
   Array3D subarray(int i0, int i1, int j0, int j1, 
		   		int k0, int k1);
};

template <class T>
Array3D<T>::Array3D() : data_(), v_(), m_(0), n_(0) {}

template <class T>
Array3D<T>::Array3D(const Array3D<T> &A) : data_(A.data_), 
	v_(A.v_), m_(A.m_), n_(A.n_), g_(A.g_)
{
}



template <class T>
Array3D<T>::Array3D(int m, int n, int g) : data_(m*n*g), v_(m,n),
	m_(m), n_(n), g_(g)
{

  if (m>0 && n>0 && g>0)
  {
	T* p = & (data_[0]);
	int ng = n_*g_;

	for (int i=0; i<m_; i++)
	{	
		T* ping = p+ i*ng;
		for (int j=0; j<n; j++)
			v_[i][j] = ping + j*g_;
	}
  }
}



template <class T>
Array3D<T>::Array3D(int m, int n, int g, T val) : data_(m*n*g, val), 
	v_(m,n), m_(m), n_(n), g_(g)
{
  if (m>0 && n>0 && g>0)
  {

	T* p = & (data_[0]);
	int ng = n_*g_;

	for (int i=0; i<m_; i++)
	{	
		T* ping = p+ i*ng;
		for (int j=0; j<n; j++)
			v_[i][j] = ping + j*g_;
	}
  }
}



template <class T>
Array3D<T>::Array3D(int m, int n, int g, T* a) : 
		data_(m*n*g, a), v_(m,n), m_(m), n_(n), g_(g)
{

  if (m>0 && n>0 && g>0)
  {
	T* p = & (data_[0]);
	int ng = n_*g_;

	for (int i=0; i<m_; i++)
	{	
		T* ping = p+ i*ng;
		for (int j=0; j<n; j++)
			v_[i][j] = ping + j*g_;
	}
  }
}



template <class T>
inline T** Array3D<T>::operator[](int i) 
{ 
#ifdef TNT_BOUNDS_CHECK
	assert(i >= 0);
	assert(i < m_);
#endif

return v_[i]; 

}

template <class T>
inline const T* const * Array3D<T>::operator[](int i) const 
{ return v_[i]; }

template <class T>
Array3D<T> & Array3D<T>::operator=(const T &a)
{
	for (int i=0; i<m_; i++)
		for (int j=0; j<n_; j++)
			for (int k=0; k<g_; k++)
				v_[i][j][k] = a;

	return *this;
}

template <class T>
Array3D<T> Array3D<T>::copy() const
{
	Array3D A(m_, n_, g_);
	for (int i=0; i<m_; i++)
		for (int j=0; j<n_; j++)
			for (int k=0; k<g_; k++)
				A.v_[i][j][k] = v_[i][j][k];

	return A;
}


template <class T>
Array3D<T> & Array3D<T>::inject(const Array3D &A)
{
	if (A.m_ == m_ &&  A.n_ == n_ && A.g_ == g_)

	for (int i=0; i<m_; i++)
		for (int j=0; j<n_; j++)
			for (int k=0; k<g_; k++)
				v_[i][j][k] = A.v_[i][j][k];

	return *this;
}



template <class T>
Array3D<T> & Array3D<T>::ref(const Array3D<T> &A)
{
	if (this != &A)
	{
		m_ = A.m_;
		n_ = A.n_;
		g_ = A.g_;
		v_ = A.v_;
		data_ = A.data_;
	}
	return *this;
}

template <class T>
Array3D<T> & Array3D<T>::operator=(const Array3D<T> &A)
{
	return ref(A);
}


template <class T>
inline int Array3D<T>::dim1() const { return m_; }

template <class T>
inline int Array3D<T>::dim2() const { return n_; }

template <class T>
inline int Array3D<T>::dim3() const { return g_; }



template <class T>
Array3D<T>::~Array3D() {}

template <class T>
inline Array3D<T>::operator T***()
{
	return v_;
}


template <class T>
inline Array3D<T>::operator const T***()
{
	return v_;
}

/* extended interface */
template <class T>
Array3D<T> Array3D<T>::subarray(int i0, int i1, int j0,
	int j1, int k0, int k1)
{

	/* check that ranges are valid. */
	if (!( 0 <= i0 && i0 <= i1 && i1 < m_ &&
	      0 <= j0 && j0 <= j1 && j1 < n_ &&
	      0 <= k0 && k0 <= k1 && k1 < g_))
		return Array3D<T>();  /* null array */


	Array3D<T> A;
	A.data_ = data_;
	A.m_ = i1-i0+1;
	A.n_ = j1-j0+1;
	A.g_ = k1-k0+1;
	A.v_ = Array2D<T*>(A.m_,A.n_);
	T* p = &(data_[0]) + i0*n_*g_ + j0*g_ + k0; 

	for (int i=0; i<A.m_; i++)
	{
		T* ping = p + i*n_*g_;
		for (int j=0; j<A.n_; j++)
			A.v_[i][j] = ping + j*g_ ;
	}

	return A;
}
	


} /* namespace TNT */

#endif
/* TNT_ARRAY3D_H */



#ifndef TNT_ARRAY3D_UTILS_H
#define TNT_ARRAY3D_UTILS_H

namespace TNT
{


template <class T>
std::ostream& operator<<(std::ostream &s, const Array3D<T> &A)
{
    int M=A.dim1();
    int N=A.dim2();
    int K=A.dim3();

    s << M << " " << N << " " << K << "\n";

    for (int i=0; i<M; i++)
    {
        for (int j=0; j<N; j++)
        {
			for (int k=0; k<K; k++)
            	s << A[i][j][k] << " ";
			s << "\n";
        }
        s << "\n";
    }


    return s;
}

template <class T>
std::istream& operator>>(std::istream &s, Array3D<T> &A)
{

    int M, N, K;

    s >> M >> N >> K;

    Array3D<T> B(M,N,K);

    for (int i=0; i<M; i++)
       for (int j=0; j<N; j++)
          for (int k=0; k<K; k++)
             s >>  B[i][j][k];

    A = B;
    return s;
}



template <class T>
Array3D<T> operator+(const Array3D<T> &A, const Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() != m ||  B.dim2() != n || B.dim3() != p )
		return Array3D<T>();

	else
	{
		Array3D<T> C(m,n,p);

		for (int i=0; i<m; i++)
			for (int j=0; j<n; j++)
				for (int k=0; k<p; k++)
				C[i][j][k] = A[i][j][k] + B[i][j][k];

		return C;
	}
}


template <class T>
Array3D<T> operator-(const Array3D<T> &A, const Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() != m ||  B.dim2() != n || B.dim3() != p )
		return Array3D<T>();

	else
	{
		Array3D<T> C(m,n,p);

		for (int i=0; i<m; i++)
			for (int j=0; j<n; j++)
				for (int k=0; k<p; k++)
				C[i][j][k] = A[i][j][k] - B[i][j][k];

		return C;
	}
}




template <class T>
Array3D<T> operator*(const Array3D<T> &A, const Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() != m ||  B.dim2() != n || B.dim3() != p )
		return Array3D<T>();

	else
	{
		Array3D<T> C(m,n,p);

		for (int i=0; i<m; i++)
			for (int j=0; j<n; j++)
				for (int k=0; k<p; k++)
				C[i][j][k] = A[i][j][k] * B[i][j][k];

		return C;
	}
}


template <class T>
Array3D<T> operator/(const Array3D<T> &A, const Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() != m ||  B.dim2() != n || B.dim3() != p )
		return Array3D<T>();

	else
	{
		Array3D<T> C(m,n,p);

		for (int i=0; i<m; i++)
			for (int j=0; j<n; j++)
				for (int k=0; k<p; k++)
				C[i][j][k] = A[i][j][k] / B[i][j][k];

		return C;
	}
}



template <class T>
Array3D<T>& operator+=(Array3D<T> &A, const Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() == m &&  B.dim2() == n && B.dim3() == p )
	{
		for (int i=0; i<m; i++)
			for (int j=0; j<n; j++)
				for (int k=0; k<p; k++)
					A[i][j][k] += B[i][j][k];
	}

	return A;
}

template <class T>
Array3D<T>& operator-=(Array3D<T> &A, const Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() == m &&  B.dim2() == n && B.dim3() == p )
	{
		for (int i=0; i<m; i++)
			for (int j=0; j<n; j++)
				for (int k=0; k<p; k++)
					A[i][j][k] -= B[i][j][k];
	}

	return A;
}

template <class T>
Array3D<T>& operator*=(Array3D<T> &A, const Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() == m &&  B.dim2() == n && B.dim3() == p )
	{
		for (int i=0; i<m; i++)
			for (int j=0; j<n; j++)
				for (int k=0; k<p; k++)
					A[i][j][k] *= B[i][j][k];
	}

	return A;
}


template <class T>
Array3D<T>& operator/=(Array3D<T> &A, const Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() == m &&  B.dim2() == n && B.dim3() == p )
	{
		for (int i=0; i<m; i++)
			for (int j=0; j<n; j++)
				for (int k=0; k<p; k++)
					A[i][j][k] /= B[i][j][k];
	}

	return A;
}





} // namespace TNT

#endif


// C compatible matrix: row-oriented, 0-based [i][j] and 1-based (i,j) indexing
//

#ifndef TNT_CMAT_H
#define TNT_CMAT_H

#ifndef TNT_SUBSCRPT_H
#define TNT_SUBSCRPT_H


//---------------------------------------------------------------------
// This definition describes the default TNT data type used for
// indexing into TNT matrices and vectors.  The data type should
// be wide enough to index into large arrays.  It defaults to an
// "int", but can be overriden at compile time redefining TNT_SUBSCRIPT_TYPE,
// e.g.
// 
//      c++ -DTNT_SUBSCRIPT_TYPE='unsigned int'  ...
//
//---------------------------------------------------------------------
//

#ifndef TNT_SUBSCRIPT_TYPE
#define TNT_SUBSCRIPT_TYPE int
#endif

namespace TNT
{
    typedef TNT_SUBSCRIPT_TYPE Subscript;
} /* namespace TNT */


// () indexing in TNT means 1-offset, i.e. x(1) and A(1,1) are the
// first elements.  This offset is left as a macro for future
// purposes, but should not be changed in the current release.
//
//
#define TNT_BASE_OFFSET (1)

#endif
#ifndef TNT_VEC_H
#define TNT_VEC_H

namespace TNT
{

/**
 <b>[Deprecatred]</b>  Value-based vector class from pre-1.0
 	TNT version.  Kept here for backward compatiblity, but should
	use the newer TNT::Array1D classes instead.

*/

template <class T>
class Vector 
{


  public:

    typedef Subscript   size_type;
    typedef         T   value_type;
    typedef         T   element_type;
    typedef         T*  pointer;
    typedef         T*  iterator;
    typedef         T&  reference;
    typedef const   T*  const_iterator;
    typedef const   T&  const_reference;

    Subscript lbound() const { return 1;}
 
  protected:
    T* v_;                  
    T* vm1_;        // pointer adjustment for optimzied 1-offset indexing
    Subscript n_;

    // internal helper function to create the array
    // of row pointers

    void initialize(Subscript N)
    {
        // adjust pointers so that they are 1-offset:
        // v_[] is the internal contiguous array, it is still 0-offset
        //
        assert(v_ == NULL);
        v_ = new T[N];
        assert(v_  != NULL);
        vm1_ = v_-1;
        n_ = N;
    }
   
    void copy(const T*  v)
    {
        Subscript N = n_;
        Subscript i;

#ifdef TNT_UNROLL_LOOPS
        Subscript Nmod4 = N & 3;
        Subscript N4 = N - Nmod4;

        for (i=0; i<N4; i+=4)
        {
            v_[i] = v[i];
            v_[i+1] = v[i+1];
            v_[i+2] = v[i+2];
            v_[i+3] = v[i+3];
        }

        for (i=N4; i< N; i++)
            v_[i] = v[i];
#else

        for (i=0; i< N; i++)
            v_[i] = v[i];
#endif      
    }

    void set(const T& val)
    {
        Subscript N = n_;
        Subscript i;

#ifdef TNT_UNROLL_LOOPS
        Subscript Nmod4 = N & 3;
        Subscript N4 = N - Nmod4;

        for (i=0; i<N4; i+=4)
        {
            v_[i] = val;
            v_[i+1] = val;
            v_[i+2] = val;
            v_[i+3] = val; 
        }

        for (i=N4; i< N; i++)
            v_[i] = val;
#else

        for (i=0; i< N; i++)
            v_[i] = val;
        
#endif      
    }
    


    void destroy()
    {     
        /* do nothing, if no memory has been previously allocated */
        if (v_ == NULL) return ;

        /* if we are here, then matrix was previously allocated */
        delete [] (v_);     

        v_ = NULL;
        vm1_ = NULL;
    }


  public:

    // access

    iterator begin() { return v_;}
    iterator end()   { return v_ + n_; }
    iterator begin() const { return v_;}
    iterator end() const  { return v_ + n_; }

    // destructor

    ~Vector() 
    {
        destroy();
    }

    // constructors

    Vector() : v_(0), vm1_(0), n_(0)  {};

    Vector(const Vector<T> &A) : v_(0), vm1_(0), n_(0)
    {
        initialize(A.n_);
        copy(A.v_);
    }

    Vector(Subscript N, const T& value = T()) :  v_(0), vm1_(0), n_(0)
    {
        initialize(N);
        set(value);
    }

    Vector(Subscript N, const T* v) :  v_(0), vm1_(0), n_(0)
    {
        initialize(N);
        copy(v);
    }

    Vector(Subscript N, char *s) :  v_(0), vm1_(0), n_(0)
    {
        initialize(N);
        std::istringstream ins(s);

        Subscript i;

        for (i=0; i<N; i++)
                ins >> v_[i];
    }


    // methods
    // 
    Vector<T>& newsize(Subscript N)
    {
        if (n_ == N) return *this;

        destroy();
        initialize(N);

        return *this;
    }


    // assignments
    //
    Vector<T>& operator=(const Vector<T> &A)
    {
        if (v_ == A.v_)
            return *this;

        if (n_ == A.n_)         // no need to re-alloc
            copy(A.v_);

        else
        {
            destroy();
            initialize(A.n_);
            copy(A.v_);
        }

        return *this;
    }
        
    Vector<T>& operator=(const T& scalar)
    { 
        set(scalar);  
        return *this;
    }

    inline Subscript dim() const 
    {
        return  n_; 
    }

    inline Subscript size() const 
    {
        return  n_; 
    }


    inline reference operator()(Subscript i)
    { 
#ifdef TNT_BOUNDS_CHECK
        assert(1<=i);
        assert(i <= n_) ;
#endif
        return vm1_[i]; 
    }

    inline const_reference operator() (Subscript i) const
    {
#ifdef TNT_BOUNDS_CHECK
        assert(1<=i);
        assert(i <= n_) ;
#endif
        return vm1_[i]; 
    }

    inline reference operator[](Subscript i)
    { 
#ifdef TNT_BOUNDS_CHECK
        assert(0<=i);
        assert(i < n_) ;
#endif
        return v_[i]; 
    }

    inline const_reference operator[](Subscript i) const
    {
#ifdef TNT_BOUNDS_CHECK
        assert(0<=i);






        assert(i < n_) ;
#endif
        return v_[i]; 
    }



};


/* ***************************  I/O  ********************************/

template <class T>
std::ostream& operator<<(std::ostream &s, const Vector<T> &A)
{
    Subscript N=A.dim();

    s <<  N << "\n";

    for (Subscript i=0; i<N; i++)
        s   << A[i] << " " << "\n";
    s << "\n";

    return s;
}

template <class T>
std::istream & operator>>(std::istream &s, Vector<T> &A)
{

    Subscript N;

    s >> N;

    if ( !(N == A.size() ))
    {
        A.newsize(N);
    }


    for (Subscript i=0; i<N; i++)
            s >>  A[i];


    return s;
}

// *******************[ basic matrix algorithms ]***************************


template <class T>
Vector<T> operator+(const Vector<T> &A, 
    const Vector<T> &B)
{
    Subscript N = A.dim();

    assert(N==B.dim());

    Vector<T> tmp(N);
    Subscript i;

    for (i=0; i<N; i++)
            tmp[i] = A[i] + B[i];

    return tmp;
}

template <class T>
Vector<T> operator-(const Vector<T> &A, 
    const Vector<T> &B)
{
    Subscript N = A.dim();

    assert(N==B.dim());

    Vector<T> tmp(N);
    Subscript i;

    for (i=0; i<N; i++)
            tmp[i] = A[i] - B[i];

    return tmp;
}

template <class T>
Vector<T> operator*(const Vector<T> &A, 
    const Vector<T> &B)
{
    Subscript N = A.dim();

    assert(N==B.dim());

    Vector<T> tmp(N);
    Subscript i;

    for (i=0; i<N; i++)
            tmp[i] = A[i] * B[i];

    return tmp;
}


template <class T>
T dot_prod(const Vector<T> &A, const Vector<T> &B)
{
    Subscript N = A.dim();
    assert(N == B.dim());

    Subscript i;
    T sum = 0;

    for (i=0; i<N; i++)
        sum += A[i] * B[i];

    return sum;
}

}   /* namespace TNT */

#endif
// TNT_VEC_H

namespace TNT
{


template <class T>
class Matrix 
{


  public:

    typedef Subscript   size_type;
    typedef         T   value_type;
    typedef         T   element_type;
    typedef         T*  pointer;
    typedef         T*  iterator;
    typedef         T&  reference;
    typedef const   T*  const_iterator;
    typedef const   T&  const_reference;

    Subscript lbound() const { return 1;}
 
  protected:
    Subscript m_;
    Subscript n_;
    Subscript mn_;      // total size
    T* v_;                  
    T** row_;           
    T* vm1_ ;       // these point to the same data, but are 1-based 
    T** rowm1_;

    // internal helper function to create the array
    // of row pointers

    void initialize(Subscript M, Subscript N)
    {
        mn_ = M*N;
        m_ = M;
        n_ = N;

        v_ = new T[mn_]; 
        row_ = new T*[M];
        rowm1_ = new T*[M];

        assert(v_  != NULL);
        assert(row_  != NULL);
        assert(rowm1_ != NULL);

        T* p = v_;              
        vm1_ = v_ - 1;
        for (Subscript i=0; i<M; i++)
        {
            row_[i] = p;
            rowm1_[i] = p-1;
            p += N ;
            
        }

        rowm1_ -- ;     // compensate for 1-based offset
    }
   
    void copy(const T*  v)
    {
        Subscript N = m_ * n_;
        Subscript i;

#ifdef TNT_UNROLL_LOOPS
        Subscript Nmod4 = N & 3;
        Subscript N4 = N - Nmod4;

        for (i=0; i<N4; i+=4)
        {
            v_[i] = v[i];
            v_[i+1] = v[i+1];
            v_[i+2] = v[i+2];
            v_[i+3] = v[i+3];
        }

        for (i=N4; i< N; i++)
            v_[i] = v[i];
#else

        for (i=0; i< N; i++)
            v_[i] = v[i];
#endif      
    }

    void set(const T& val)
    {
        Subscript N = m_ * n_;
        Subscript i;

#ifdef TNT_UNROLL_LOOPS
        Subscript Nmod4 = N & 3;
        Subscript N4 = N - Nmod4;

        for (i=0; i<N4; i+=4)
        {
            v_[i] = val;
            v_[i+1] = val;
            v_[i+2] = val;
            v_[i+3] = val; 
        }

        for (i=N4; i< N; i++)
            v_[i] = val;
#else

        for (i=0; i< N; i++)
            v_[i] = val;
        
#endif      
    }
    

    
    void destroy()
    {     
        /* do nothing, if no memory has been previously allocated */
        if (v_ == NULL) return ;

        /* if we are here, then matrix was previously allocated */
        if (v_ != NULL) delete [] (v_);     
        if (row_ != NULL) delete [] (row_);

        /* return rowm1_ back to original value */
        rowm1_ ++;
        if (rowm1_ != NULL ) delete [] (rowm1_);
    }


  public:

    operator T**(){ return  row_; }
    operator T**() const { return row_; }


    Subscript size() const { return mn_; }

    // constructors

    Matrix() : m_(0), n_(0), mn_(0), v_(0), row_(0), vm1_(0), rowm1_(0) {};

    Matrix(const Matrix<T> &A)
    {
        initialize(A.m_, A.n_);
        copy(A.v_);
    }

    Matrix(Subscript M, Subscript N, const T& value = T())
    {
        initialize(M,N);
        set(value);
    }

    Matrix(Subscript M, Subscript N, const T* v)
    {
        initialize(M,N);
        copy(v);
    }

    Matrix(Subscript M, Subscript N, const char *s)
    {
        initialize(M,N);
        //std::istrstream ins(s);
        std::istringstream ins(s);

        Subscript i, j;

        for (i=0; i<M; i++)
            for (j=0; j<N; j++)
                ins >> row_[i][j];
    }

    // destructor
    //
    ~Matrix()
    {
        destroy();
    }


    // reallocating
    //
    Matrix<T>& newsize(Subscript M, Subscript N)
    {
        if (num_rows() == M && num_cols() == N)
            return *this;

        destroy();
        initialize(M,N);
        
        return *this;
    }




    // assignments
    //
    Matrix<T>& operator=(const Matrix<T> &A)
    {
        if (v_ == A.v_)
            return *this;

        if (m_ == A.m_  && n_ == A.n_)      // no need to re-alloc
            copy(A.v_);

        else
        {
            destroy();
            initialize(A.m_, A.n_);
            copy(A.v_);
        }

        return *this;
    }
        
    Matrix<T>& operator=(const T& scalar)
    { 
        set(scalar); 
        return *this;
    }


    Subscript dim(Subscript d) const 
    {
#ifdef TNT_BOUNDS_CHECK
        assert( d >= 1);
        assert( d <= 2);
#endif
        return (d==1) ? m_ : ((d==2) ? n_ : 0); 
    }

    Subscript num_rows() const { return m_; }
    Subscript num_cols() const { return n_; }




    inline T* operator[](Subscript i)
    {
#ifdef TNT_BOUNDS_CHECK
        assert(0<=i);
        assert(i < m_) ;
#endif
        return row_[i];
    }

    inline const T* operator[](Subscript i) const
    {
#ifdef TNT_BOUNDS_CHECK
        assert(0<=i);
        assert(i < m_) ;
#endif
        return row_[i];
    }

    inline reference operator()(Subscript i)
    { 
#ifdef TNT_BOUNDS_CHECK
        assert(1<=i);
        assert(i <= mn_) ;
#endif
        return vm1_[i]; 
    }

    inline const_reference operator()(Subscript i) const
    { 
#ifdef TNT_BOUNDS_CHECK
        assert(1<=i);
        assert(i <= mn_) ;
#endif
        return vm1_[i]; 
    }



    inline reference operator()(Subscript i, Subscript j)
    { 
#ifdef TNT_BOUNDS_CHECK
        assert(1<=i);
        assert(i <= m_) ;
        assert(1<=j);
        assert(j <= n_);
#endif
        return  rowm1_[i][j]; 
    }


    
    inline const_reference operator() (Subscript i, Subscript j) const
    {
#ifdef TNT_BOUNDS_CHECK
        assert(1<=i);
        assert(i <= m_) ;
        assert(1<=j);
        assert(j <= n_);
#endif
        return rowm1_[i][j]; 
    }




};


/* ***************************  I/O  ********************************/

template <class T>
std::ostream& operator<<(std::ostream &s, const Matrix<T> &A)
{
    Subscript M=A.num_rows();
    Subscript N=A.num_cols();

    s << M << " " << N << "\n";

    for (Subscript i=0; i<M; i++)
    {
        for (Subscript j=0; j<N; j++)
        {
            s << A[i][j] << " ";
        }
        s << "\n";
    }


    return s;
}

template <class T>
std::istream& operator>>(std::istream &s, Matrix<T> &A)
{

    Subscript M, N;

    s >> M >> N;

    if ( !(M == A.num_rows() && N == A.num_cols() ))
    {
        A.newsize(M,N);
    }


    for (Subscript i=0; i<M; i++)
        for (Subscript j=0; j<N; j++)
        {
            s >>  A[i][j];
        }


    return s;
}

// *******************[ basic matrix algorithms ]***************************


template <class T>
Matrix<T> operator+(const Matrix<T> &A, 
    const Matrix<T> &B)
{
    Subscript M = A.num_rows();
    Subscript N = A.num_cols();

    assert(M==B.num_rows());
    assert(N==B.num_cols());

    Matrix<T> tmp(M,N);
    Subscript i,j;

    for (i=0; i<M; i++)
        for (j=0; j<N; j++)
            tmp[i][j] = A[i][j] + B[i][j];

    return tmp;
}

template <class T>
Matrix<T> operator-(const Matrix<T> &A, 
    const Matrix<T> &B)
{
    Subscript M = A.num_rows();
    Subscript N = A.num_cols();

    assert(M==B.num_rows());
    assert(N==B.num_cols());

    Matrix<T> tmp(M,N);
    Subscript i,j;

    for (i=0; i<M; i++)
        for (j=0; j<N; j++)
            tmp[i][j] = A[i][j] - B[i][j];

    return tmp;
}

template <class T>
Matrix<T> mult_element(const Matrix<T> &A, 
    const Matrix<T> &B)
{
    Subscript M = A.num_rows();
    Subscript N = A.num_cols();

    assert(M==B.num_rows());
    assert(N==B.num_cols());

    Matrix<T> tmp(M,N);
    Subscript i,j;

    for (i=0; i<M; i++)
        for (j=0; j<N; j++)
            tmp[i][j] = A[i][j] * B[i][j];

    return tmp;
}


template <class T>
Matrix<T> transpose(const Matrix<T> &A)
{
    Subscript M = A.num_rows();
    Subscript N = A.num_cols();

    Matrix<T> S(N,M);
    Subscript i, j;

    for (i=0; i<M; i++)
        for (j=0; j<N; j++)
            S[j][i] = A[i][j];

    return S;
}


    
template <class T>
inline Matrix<T> matmult(const Matrix<T>  &A, 
    const Matrix<T> &B)
{

#ifdef TNT_BOUNDS_CHECK
    assert(A.num_cols() == B.num_rows());
#endif

    Subscript M = A.num_rows();
    Subscript N = A.num_cols();
    Subscript K = B.num_cols();

    Matrix<T> tmp(M,K);
    T sum;

    for (Subscript i=0; i<M; i++)
    for (Subscript k=0; k<K; k++)
    {
        sum = 0;
        for (Subscript j=0; j<N; j++)
            sum = sum +  A[i][j] * B[j][k];

        tmp[i][k] = sum; 
    }

    return tmp;
}

template <class T>
inline Matrix<T> operator*(const Matrix<T>  &A, 
    const Matrix<T> &B)
{
    return matmult(A,B);
}

template <class T>
inline int matmult(Matrix<T>& C, const Matrix<T>  &A, 
    const Matrix<T> &B)
{

    assert(A.num_cols() == B.num_rows());

    Subscript M = A.num_rows();
    Subscript N = A.num_cols();
    Subscript K = B.num_cols();

    C.newsize(M,K);

    T sum;

    const T* row_i;
    const T* col_k;

    for (Subscript i=0; i<M; i++)
    for (Subscript k=0; k<K; k++)
    {
        row_i  = &(A[i][0]);
        col_k  = &(B[0][k]);
        sum = 0;
        for (Subscript j=0; j<N; j++)
        {
            sum  += *row_i * *col_k;
            row_i++;
            col_k += K;
        }
        C[i][k] = sum; 
    }

    return 0;
}


template <class T>
Vector<T> matmult(const Matrix<T>  &A, const Vector<T> &x)
{

#ifdef TNT_BOUNDS_CHECK
    assert(A.num_cols() == x.dim());
#endif

    Subscript M = A.num_rows();
    Subscript N = A.num_cols();

    Vector<T> tmp(M);
    T sum;

    for (Subscript i=0; i<M; i++)
    {
        sum = 0;
        const T* rowi = A[i];
        for (Subscript j=0; j<N; j++)
            sum = sum +  rowi[j] * x[j];

        tmp[i] = sum; 
    }

    return tmp;
}

template <class T>
inline Vector<T> operator*(const Matrix<T>  &A, const Vector<T> &x)
{
    return matmult(A,x);
}

} // namespace TNT

#endif
// CMAT_H

#ifndef TNT_FORTRAN_ARRAY1D_H
#define TNT_FORTRAN_ARRAY1D_H

namespace TNT
{

template <class T>
class Fortran_Array1D 
{

  private:

    i_refvec<T> v_;
    int n_;
    T* data_;				/* this normally points to v_.begin(), but
                             * could also point to a portion (subvector)
							 * of v_.
                            */

    void initialize_(int n);
    void copy_(T* p, const T*  q, int len) const;
    void set_(T* begin,  T* end, const T& val);
 

  public:

    typedef         T   value_type;


	         Fortran_Array1D();
	explicit Fortran_Array1D(int n);
	         Fortran_Array1D(int n, const T &a);
	         Fortran_Array1D(int n,  T *a);
    inline   Fortran_Array1D(const Fortran_Array1D &A);
	inline   Fortran_Array1D & operator=(const T &a);
	inline   Fortran_Array1D & operator=(const Fortran_Array1D &A);
	inline   Fortran_Array1D & ref(const Fortran_Array1D &A);
	         Fortran_Array1D copy() const;
		     Fortran_Array1D & inject(const Fortran_Array1D & A);
	inline   T& operator()(int i);
	inline   const T& operator()(int i) const;
	inline 	 int dim1() const;
	inline   int dim() const;
              ~Fortran_Array1D();


	/* ... extended interface ... */

	inline int ref_count() const;
	inline Fortran_Array1D<T> subarray(int i0, int i1);

};




template <class T>
Fortran_Array1D<T>::Fortran_Array1D() : v_(), n_(0), data_(0) {}

template <class T>
Fortran_Array1D<T>::Fortran_Array1D(const Fortran_Array1D<T> &A) : v_(A.v_),  n_(A.n_), 
		data_(A.data_)
{
#ifdef TNT_DEBUG
	std::cout << "Created Fortran_Array1D(const Fortran_Array1D<T> &A) \n";
#endif

}


template <class T>
Fortran_Array1D<T>::Fortran_Array1D(int n) : v_(n), n_(n), data_(v_.begin())
{
#ifdef TNT_DEBUG
	std::cout << "Created Fortran_Array1D(int n) \n";
#endif
}

template <class T>
Fortran_Array1D<T>::Fortran_Array1D(int n, const T &val) : v_(n), n_(n), data_(v_.begin()) 
{
#ifdef TNT_DEBUG
	std::cout << "Created Fortran_Array1D(int n, const T& val) \n";
#endif
	set_(data_, data_+ n, val);

}

template <class T>
Fortran_Array1D<T>::Fortran_Array1D(int n, T *a) : v_(a), n_(n) , data_(v_.begin())
{
#ifdef TNT_DEBUG
	std::cout << "Created Fortran_Array1D(int n, T* a) \n";
#endif
}

template <class T>
inline T& Fortran_Array1D<T>::operator()(int i) 
{ 
#ifdef TNT_BOUNDS_CHECK
	assert(i>= 1);
	assert(i <= n_);
#endif
	return data_[i-1]; 
}

template <class T>
inline const T& Fortran_Array1D<T>::operator()(int i) const 
{ 
#ifdef TNT_BOUNDS_CHECK
	assert(i>= 1);
	assert(i <= n_);
#endif
	return data_[i-1]; 
}


	

template <class T>
Fortran_Array1D<T> & Fortran_Array1D<T>::operator=(const T &a)
{
	set_(data_, data_+n_, a);
	return *this;
}

template <class T>
Fortran_Array1D<T> Fortran_Array1D<T>::copy() const
{
	Fortran_Array1D A( n_);
	copy_(A.data_, data_, n_);

	return A;
}


template <class T>
Fortran_Array1D<T> & Fortran_Array1D<T>::inject(const Fortran_Array1D &A)
{
	if (A.n_ == n_)
		copy_(data_, A.data_, n_);

	return *this;
}





template <class T>
Fortran_Array1D<T> & Fortran_Array1D<T>::ref(const Fortran_Array1D<T> &A)
{
	if (this != &A)
	{
		v_ = A.v_;		/* operator= handles the reference counting. */
		n_ = A.n_;
		data_ = A.data_; 
		
	}
	return *this;
}

template <class T>
Fortran_Array1D<T> & Fortran_Array1D<T>::operator=(const Fortran_Array1D<T> &A)
{
	return ref(A);
}

template <class T>
inline int Fortran_Array1D<T>::dim1() const { return n_; }

template <class T>
inline int Fortran_Array1D<T>::dim() const { return n_; }

template <class T>
Fortran_Array1D<T>::~Fortran_Array1D() {}


/* ............................ exented interface ......................*/

template <class T>
inline int Fortran_Array1D<T>::ref_count() const
{
	return v_.ref_count();
}

template <class T>
inline Fortran_Array1D<T> Fortran_Array1D<T>::subarray(int i0, int i1)
{
#ifdef TNT_DEBUG
		std::cout << "entered subarray. \n";
#endif
	if ((i0 > 0) && (i1 < n_) || (i0 <= i1))
	{
		Fortran_Array1D<T> X(*this);  /* create a new instance of this array. */
		X.n_ = i1-i0+1;
		X.data_ += i0;

		return X;
	}
	else
	{
#ifdef TNT_DEBUG
		std::cout << "subarray:  null return.\n";
#endif
		return Fortran_Array1D<T>();
	}
}


/* private internal functions */


template <class T>
void Fortran_Array1D<T>::set_(T* begin, T* end, const T& a)
{
	for (T* p=begin; p<end; p++)
		*p = a;

}

template <class T>
void Fortran_Array1D<T>::copy_(T* p, const T* q, int len) const
{
	T *end = p + len;
	while (p<end )
		*p++ = *q++;

}


} /* namespace TNT */

#endif
/* TNT_FORTRAN_ARRAY1D_H */

#ifndef TNT_FORTRAN_ARRAY1D_UTILS_H
#define TNT_FORTRAN_ARRAY1D_UTILS_H

namespace TNT
{


/**
	Write an array to a character outstream.  Output format is one that can
	be read back in via the in-stream operator: one integer
	denoting the array dimension (n), followed by n elements,
	one per line.  

*/
template <class T>
std::ostream& operator<<(std::ostream &s, const Fortran_Array1D<T> &A)
{
    int N=A.dim1();

    s << N << "\n";
    for (int j=1; j<=N; j++)
    {
       s << A(j) << "\n";
    }
    s << "\n";

    return s;
}

/**
	Read an array from a character stream.  Input format
	is one integer, denoting the dimension (n), followed
	by n whitespace-separated elments.  Newlines are ignored

	<p>
	Note: the array being read into references new memory
	storage. If the intent is to fill an existing conformant
	array, use <code> cin >> B;  A.inject(B) ); </code>
	instead or read the elements in one-a-time by hand.

	@param s the charater to read from (typically <code>std::in</code>)
	@param A the array to read into.
*/
template <class T>
std::istream& operator>>(std::istream &s, Fortran_Array1D<T> &A)
{
	int N;
	s >> N;

	Fortran_Array1D<T> B(N);
	for (int i=1; i<=N; i++)
		s >> B(i);
	A = B;
	return s;
}


template <class T>
Fortran_Array1D<T> operator+(const Fortran_Array1D<T> &A, const Fortran_Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() != n )
		return Fortran_Array1D<T>();

	else
	{
		Fortran_Array1D<T> C(n);

		for (int i=1; i<=n; i++)
		{
			C(i) = A(i) + B(i);
		}
		return C;
	}
}



template <class T>
Fortran_Array1D<T> operator-(const Fortran_Array1D<T> &A, const Fortran_Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() != n )
		return Fortran_Array1D<T>();

	else
	{
		Fortran_Array1D<T> C(n);

		for (int i=1; i<=n; i++)
		{
			C(i) = A(i) - B(i);
		}
		return C;
	}
}


template <class T>
Fortran_Array1D<T> operator*(const Fortran_Array1D<T> &A, const Fortran_Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() != n )
		return Fortran_Array1D<T>();

	else
	{
		Fortran_Array1D<T> C(n);

		for (int i=1; i<=n; i++)
		{
			C(i) = A(i) * B(i);
		}
		return C;
	}
}


template <class T>
Fortran_Array1D<T> operator/(const Fortran_Array1D<T> &A, const Fortran_Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() != n )
		return Fortran_Array1D<T>();

	else
	{
		Fortran_Array1D<T> C(n);

		for (int i=1; i<=n; i++)
		{
			C(i) = A(i) / B(i);
		}
		return C;
	}
}









template <class T>
Fortran_Array1D<T>&  operator+=(Fortran_Array1D<T> &A, const Fortran_Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() == n)
	{
		for (int i=1; i<=n; i++)
		{
				A(i) += B(i);
		}
	}
	return A;
}




template <class T>
Fortran_Array1D<T>&  operator-=(Fortran_Array1D<T> &A, const Fortran_Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() == n)
	{
		for (int i=1; i<=n; i++)
		{
				A(i) -= B(i);
		}
	}
	return A;
}



template <class T>
Fortran_Array1D<T>&  operator*=(Fortran_Array1D<T> &A, const Fortran_Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() == n)
	{
		for (int i=1; i<=n; i++)
		{
				A(i) *= B(i);
		}
	}
	return A;
}




template <class T>
Fortran_Array1D<T>&  operator/=(Fortran_Array1D<T> &A, const Fortran_Array1D<T> &B)
{
	int n = A.dim1();

	if (B.dim1() == n)
	{
		for (int i=1; i<=n; i++)
		{
				A(i) /= B(i);
		}
	}
	return A;
}


} // namespace TNT

#endif

#ifndef TNT_FORTRAN_ARRAY2D_H
#define TNT_FORTRAN_ARRAY2D_H

namespace TNT
{

template <class T>
class Fortran_Array2D 
{


  private: 
  		i_refvec<T> v_;
		int m_;
		int n_;
		T* data_;


    	void initialize_(int n);
    	void copy_(T* p, const T*  q, int len);
    	void set_(T* begin,  T* end, const T& val);
 
  public:

    typedef         T   value_type;

	       Fortran_Array2D();
	       Fortran_Array2D(int m, int n);
	       Fortran_Array2D(int m, int n,  T *a);
	       Fortran_Array2D(int m, int n, const T &a);
    inline Fortran_Array2D(const Fortran_Array2D &A);
	inline Fortran_Array2D & operator=(const T &a);
	inline Fortran_Array2D & operator=(const Fortran_Array2D &A);
	inline Fortran_Array2D & ref(const Fortran_Array2D &A);
	       Fortran_Array2D copy() const;
		   Fortran_Array2D & inject(const Fortran_Array2D & A);
	inline T& operator()(int i, int j);
	inline const T& operator()(int i, int j) const ;
	inline int dim1() const;
	inline int dim2() const;
               ~Fortran_Array2D();

	/* extended interface */

	inline int ref_count() const;

};

template <class T>
Fortran_Array2D<T>::Fortran_Array2D() : v_(), m_(0), n_(0), data_(0) {}


template <class T>
Fortran_Array2D<T>::Fortran_Array2D(const Fortran_Array2D<T> &A) : v_(A.v_),
		m_(A.m_), n_(A.n_), data_(A.data_) {}



template <class T>
Fortran_Array2D<T>::Fortran_Array2D(int m, int n) : v_(m*n), m_(m), n_(n),
	data_(v_.begin()) {}

template <class T>
Fortran_Array2D<T>::Fortran_Array2D(int m, int n, const T &val) : 
	v_(m*n), m_(m), n_(n), data_(v_.begin())
{
	set_(data_, data_+m*n, val);
}


template <class T>
Fortran_Array2D<T>::Fortran_Array2D(int m, int n, T *a) : v_(a),
	m_(m), n_(n), data_(v_.begin()) {}




template <class T>
inline T& Fortran_Array2D<T>::operator()(int i, int j) 
{ 
#ifdef TNT_BOUNDS_CHECK
	assert(i >= 1);
	assert(i <= m_);
	assert(j >= 1);
	assert(j <= n_);
#endif

	return v_[ (j-1)*m_ + (i-1) ];

}

template <class T>
inline const T& Fortran_Array2D<T>::operator()(int i, int j) const
{ 
#ifdef TNT_BOUNDS_CHECK
	assert(i >= 1);
	assert(i <= m_);
	assert(j >= 1);
	assert(j <= n_);
#endif

	return v_[ (j-1)*m_ + (i-1) ];

}


template <class T>
Fortran_Array2D<T> & Fortran_Array2D<T>::operator=(const T &a)
{
 	set_(data_, data_+m_*n_, a);
	return *this;
}

template <class T>
Fortran_Array2D<T> Fortran_Array2D<T>::copy() const
{

	Fortran_Array2D B(m_,n_);
	
	B.inject(*this);
	return B;
}


template <class T>
Fortran_Array2D<T> & Fortran_Array2D<T>::inject(const Fortran_Array2D &A)
{
	if (m_ == A.m_ && n_ == A.n_)
		copy_(data_, A.data_, m_*n_);

	return *this;
}



template <class T>
Fortran_Array2D<T> & Fortran_Array2D<T>::ref(const Fortran_Array2D<T> &A)
{
	if (this != &A)
	{
		v_ = A.v_;
		m_ = A.m_;
		n_ = A.n_;
		data_ = A.data_;
	}
	return *this;
}

template <class T>
Fortran_Array2D<T> & Fortran_Array2D<T>::operator=(const Fortran_Array2D<T> &A)
{
	return ref(A);
}

template <class T>
inline int Fortran_Array2D<T>::dim1() const { return m_; }

template <class T>
inline int Fortran_Array2D<T>::dim2() const { return n_; }


template <class T>
Fortran_Array2D<T>::~Fortran_Array2D()
{
}

template <class T>
inline int Fortran_Array2D<T>::ref_count() const { return v_.ref_count(); }




template <class T>
void Fortran_Array2D<T>::set_(T* begin, T* end, const T& a)
{
	for (T* p=begin; p<end; p++)
		*p = a;

}

template <class T>
void Fortran_Array2D<T>::copy_(T* p, const T* q, int len) 
{
	T *end = p + len;
	while (p<end )
		*p++ = *q++;

}


} /* namespace TNT */

#endif
/* TNT_FORTRAN_ARRAY2D_H */

#ifndef TNT_FORTRAN_ARRAY2D_UTILS_H
#define TNT_FORTRAN_ARRAY2D_UTILS_H

namespace TNT
{


template <class T>
std::ostream& operator<<(std::ostream &s, const Fortran_Array2D<T> &A)
{
    int M=A.dim1();
    int N=A.dim2();

    s << M << " " << N << "\n";

    for (int i=1; i<=M; i++)
    {
        for (int j=1; j<=N; j++)
        {
            s << A(i,j) << " ";
        }
        s << "\n";
    }


    return s;
}

template <class T>
std::istream& operator>>(std::istream &s, Fortran_Array2D<T> &A)
{

    int M, N;

    s >> M >> N;

    Fortran_Array2D<T> B(M,N);

    for (int i=1; i<=M; i++)
       for (int j=1; j<=N; j++)
       {
          s >>  B(i,j);
       }

    A = B;
    return s;
}




template <class T>
Fortran_Array2D<T> operator+(const Fortran_Array2D<T> &A, const Fortran_Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() != m ||  B.dim2() != n )
		return Fortran_Array2D<T>();

	else
	{
		Fortran_Array2D<T> C(m,n);

		for (int i=1; i<=m; i++)
		{
			for (int j=1; j<=n; j++)
				C(i,j) = A(i,j) + B(i,j);
		}
		return C;
	}
}

template <class T>
Fortran_Array2D<T> operator-(const Fortran_Array2D<T> &A, const Fortran_Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() != m ||  B.dim2() != n )
		return Fortran_Array2D<T>();

	else
	{
		Fortran_Array2D<T> C(m,n);

		for (int i=1; i<=m; i++)
		{
			for (int j=1; j<=n; j++)
				C(i,j) = A(i,j) - B(i,j);
		}
		return C;
	}
}


template <class T>
Fortran_Array2D<T> operator*(const Fortran_Array2D<T> &A, const Fortran_Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() != m ||  B.dim2() != n )
		return Fortran_Array2D<T>();

	else
	{
		Fortran_Array2D<T> C(m,n);

		for (int i=1; i<=m; i++)
		{
			for (int j=1; j<=n; j++)
				C(i,j) = A(i,j) * B(i,j);
		}
		return C;
	}
}


template <class T>
Fortran_Array2D<T> operator/(const Fortran_Array2D<T> &A, const Fortran_Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() != m ||  B.dim2() != n )
		return Fortran_Array2D<T>();

	else
	{
		Fortran_Array2D<T> C(m,n);

		for (int i=1; i<=m; i++)
		{
			for (int j=1; j<=n; j++)
				C(i,j) = A(i,j) / B(i,j);
		}
		return C;
	}
}



template <class T>
Fortran_Array2D<T>&  operator+=(Fortran_Array2D<T> &A, const Fortran_Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() == m ||  B.dim2() == n )
	{
		for (int i=1; i<=m; i++)
		{
			for (int j=1; j<=n; j++)
				A(i,j) += B(i,j);
		}
	}
	return A;
}

template <class T>
Fortran_Array2D<T>&  operator-=(Fortran_Array2D<T> &A, const Fortran_Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() == m ||  B.dim2() == n )
	{
		for (int i=1; i<=m; i++)
		{
			for (int j=1; j<=n; j++)
				A(i,j) -= B(i,j);
		}
	}
	return A;
}

template <class T>
Fortran_Array2D<T>&  operator*=(Fortran_Array2D<T> &A, const Fortran_Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() == m ||  B.dim2() == n )
	{
		for (int i=1; i<=m; i++)
		{
			for (int j=1; j<=n; j++)
				A(i,j) *= B(i,j);
		}
	}
	return A;
}

template <class T>
Fortran_Array2D<T>&  operator/=(Fortran_Array2D<T> &A, const Fortran_Array2D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();

	if (B.dim1() == m ||  B.dim2() == n )
	{
		for (int i=1; i<=m; i++)
		{
			for (int j=1; j<=n; j++)
				A(i,j) /= B(i,j);
		}
	}
	return A;
}

} // namespace TNT

#endif

#ifndef TNT_FORTRAN_ARRAY3D_H
#define TNT_FORTRAN_ARRAY3D_H

namespace TNT
{

template <class T>
class Fortran_Array3D 
{


  private: 


		i_refvec<T> v_;
		int m_;
		int n_;
		int k_;
		T* data_;

  public:

    typedef         T   value_type;

	       Fortran_Array3D();
	       Fortran_Array3D(int m, int n, int k);
	       Fortran_Array3D(int m, int n, int k,  T *a);
	       Fortran_Array3D(int m, int n, int k, const T &a);
    inline Fortran_Array3D(const Fortran_Array3D &A);
	inline Fortran_Array3D & operator=(const T &a);
	inline Fortran_Array3D & operator=(const Fortran_Array3D &A);
	inline Fortran_Array3D & ref(const Fortran_Array3D &A);
	       Fortran_Array3D copy() const;
		   Fortran_Array3D & inject(const Fortran_Array3D & A);
	inline T& operator()(int i, int j, int k);
	inline const T& operator()(int i, int j, int k) const ;
	inline int dim1() const;
	inline int dim2() const;
	inline int dim3() const;
	inline int ref_count() const;
               ~Fortran_Array3D();


};

template <class T>
Fortran_Array3D<T>::Fortran_Array3D() :  v_(), m_(0), n_(0), k_(0), data_(0) {}


template <class T>
Fortran_Array3D<T>::Fortran_Array3D(const Fortran_Array3D<T> &A) : 
	v_(A.v_), m_(A.m_), n_(A.n_), k_(A.k_), data_(A.data_) {}



template <class T>
Fortran_Array3D<T>::Fortran_Array3D(int m, int n, int k) : 
	v_(m*n*k), m_(m), n_(n), k_(k), data_(v_.begin()) {}



template <class T>
Fortran_Array3D<T>::Fortran_Array3D(int m, int n, int k, const T &val) : 
	v_(m*n*k), m_(m), n_(n), k_(k), data_(v_.begin())
{
	for (T* p = data_; p < data_ + m*n*k; p++)
		*p = val;
}

template <class T>
Fortran_Array3D<T>::Fortran_Array3D(int m, int n, int k, T *a) : 
	v_(a), m_(m), n_(n), k_(k), data_(v_.begin()) {}




template <class T>
inline T& Fortran_Array3D<T>::operator()(int i, int j, int k) 
{ 
#ifdef TNT_BOUNDS_CHECK
	assert(i >= 1);
	assert(i <= m_);
	assert(j >= 1);
	assert(j <= n_);
	assert(k >= 1);
	assert(k <= k_);
#endif

	return data_[(k-1)*m_*n_ + (j-1) * m_ + i-1];

}

template <class T>
inline const T& Fortran_Array3D<T>::operator()(int i, int j, int k)  const
{ 
#ifdef TNT_BOUNDS_CHECK
	assert(i >= 1);
	assert(i <= m_);
	assert(j >= 1);
	assert(j <= n_);
	assert(k >= 1);
	assert(k <= k_);
#endif

	return data_[(k-1)*m_*n_ + (j-1) * m_ + i-1];
}


template <class T>
Fortran_Array3D<T> & Fortran_Array3D<T>::operator=(const T &a)
{

	T *end = data_ + m_*n_*k_;

	for (T *p=data_; p != end; *p++ = a) {};

	return *this;
}

template <class T>
Fortran_Array3D<T> Fortran_Array3D<T>::copy() const
{

	Fortran_Array3D B(m_, n_, k_);
	B.inject(*this);
	return B;
	
}


template <class T>
Fortran_Array3D<T> & Fortran_Array3D<T>::inject(const Fortran_Array3D &A)
{

	if (m_ == A.m_ && n_ == A.n_ && k_ == A.k_)
	{
		T *p = data_;
		T *end = data_ + m_*n_*k_;
		const T* q = A.data_;
		for (; p < end; *p++ =  *q++) {};
	}
	return *this;
}




template <class T>
Fortran_Array3D<T> & Fortran_Array3D<T>::ref(const Fortran_Array3D<T> &A)
{

	if (this != &A)
	{
		v_ = A.v_;
		m_ = A.m_;
		n_ = A.n_;
		k_ = A.k_;
		data_ = A.data_;
	}
	return *this;
}

template <class T>
Fortran_Array3D<T> & Fortran_Array3D<T>::operator=(const Fortran_Array3D<T> &A)
{
	return ref(A);
}

template <class T>
inline int Fortran_Array3D<T>::dim1() const { return m_; }

template <class T>
inline int Fortran_Array3D<T>::dim2() const { return n_; }

template <class T>
inline int Fortran_Array3D<T>::dim3() const { return k_; }


template <class T>
inline int Fortran_Array3D<T>::ref_count() const 
{ 
	return v_.ref_count(); 
}

template <class T>
Fortran_Array3D<T>::~Fortran_Array3D()
{
}


} /* namespace TNT */

#endif
/* TNT_FORTRAN_ARRAY3D_H */

#ifndef TNT_FORTRAN_ARRAY3D_UTILS_H
#define TNT_FORTRAN_ARRAY3D_UTILS_H

namespace TNT
{


template <class T>
std::ostream& operator<<(std::ostream &s, const Fortran_Array3D<T> &A)
{
    int M=A.dim1();
    int N=A.dim2();
    int K=A.dim3();

    s << M << " " << N << " " << K << "\n";

    for (int i=1; i<=M; i++)
    {
        for (int j=1; j<=N; j++)
        {
			for (int k=1; k<=K; k++)
            	s << A(i,j,k) << " ";
			s << "\n";
        }
        s << "\n";
    }


    return s;
}

template <class T>
std::istream& operator>>(std::istream &s, Fortran_Array3D<T> &A)
{

    int M, N, K;

    s >> M >> N >> K;

    Fortran_Array3D<T> B(M,N,K);

    for (int i=1; i<=M; i++)
       for (int j=1; j<=N; j++)
          for (int k=1; k<=K; k++)
             s >>  B(i,j,k);

    A = B;
    return s;
}


template <class T>
Fortran_Array3D<T> operator+(const Fortran_Array3D<T> &A, const Fortran_Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() != m ||  B.dim2() != n || B.dim3() != p )
		return Fortran_Array3D<T>();

	else
	{
		Fortran_Array3D<T> C(m,n,p);

		for (int i=1; i<=m; i++)
			for (int j=1; j<=n; j++)
				for (int k=1; k<=p; k++)
				C(i,j,k) = A(i,j,k)+ B(i,j,k);

		return C;
	}
}


template <class T>
Fortran_Array3D<T> operator-(const Fortran_Array3D<T> &A, const Fortran_Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() != m ||  B.dim2() != n || B.dim3() != p )
		return Fortran_Array3D<T>();

	else
	{
		Fortran_Array3D<T> C(m,n,p);

		for (int i=1; i<=m; i++)
			for (int j=1; j<=n; j++)
				for (int k=1; k<=p; k++)
				C(i,j,k) = A(i,j,k)- B(i,j,k);

		return C;
	}
}


template <class T>
Fortran_Array3D<T> operator*(const Fortran_Array3D<T> &A, const Fortran_Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() != m ||  B.dim2() != n || B.dim3() != p )
		return Fortran_Array3D<T>();

	else
	{
		Fortran_Array3D<T> C(m,n,p);

		for (int i=1; i<=m; i++)
			for (int j=1; j<=n; j++)
				for (int k=1; k<=p; k++)
				C(i,j,k) = A(i,j,k)* B(i,j,k);

		return C;
	}
}


template <class T>
Fortran_Array3D<T> operator/(const Fortran_Array3D<T> &A, const Fortran_Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() != m ||  B.dim2() != n || B.dim3() != p )
		return Fortran_Array3D<T>();

	else
	{
		Fortran_Array3D<T> C(m,n,p);

		for (int i=1; i<=m; i++)
			for (int j=1; j<=n; j++)
				for (int k=1; k<=p; k++)
				C(i,j,k) = A(i,j,k)/ B(i,j,k);

		return C;
	}
}


template <class T>
Fortran_Array3D<T>& operator+=(Fortran_Array3D<T> &A, const Fortran_Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() == m &&  B.dim2() == n && B.dim3() == p )
	{
		for (int i=1; i<=m; i++)
			for (int j=1; j<=n; j++)
				for (int k=1; k<=p; k++)
					A(i,j,k) += B(i,j,k);
	}

	return A;
}


template <class T>
Fortran_Array3D<T>& operator-=(Fortran_Array3D<T> &A, const Fortran_Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() == m &&  B.dim2() == n && B.dim3() == p )
	{
		for (int i=1; i<=m; i++)
			for (int j=1; j<=n; j++)
				for (int k=1; k<=p; k++)
					A(i,j,k) -= B(i,j,k);
	}

	return A;
}


template <class T>
Fortran_Array3D<T>& operator*=(Fortran_Array3D<T> &A, const Fortran_Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() == m &&  B.dim2() == n && B.dim3() == p )
	{
		for (int i=1; i<=m; i++)
			for (int j=1; j<=n; j++)
				for (int k=1; k<=p; k++)
					A(i,j,k) *= B(i,j,k);
	}

	return A;
}


template <class T>
Fortran_Array3D<T>& operator/=(Fortran_Array3D<T> &A, const Fortran_Array3D<T> &B)
{
	int m = A.dim1();
	int n = A.dim2();
	int p = A.dim3();

	if (B.dim1() == m &&  B.dim2() == n && B.dim3() == p )
	{
		for (int i=1; i<=m; i++)
			for (int j=1; j<=n; j++)
				for (int k=1; k<=p; k++)
					A(i,j,k) /= B(i,j,k);
	}

	return A;
}


} // namespace TNT

#endif


#ifndef MATH_UTILS_H
#define MATH_UTILS_H

/* needed for fabs, sqrt() below */



namespace TNT
{
/**
	@returns hypotenuse of real (non-complex) scalars a and b by 
	avoiding underflow/overflow
	using (a * sqrt( 1 + (b/a) * (b/a))), rather than
	sqrt(a*a + b*b).
*/
template <class Real>
Real hypot(const Real &a, const Real &b)
{
	
	if (a== 0)
		return abs(b);
	else
	{
		Real c = b/a;
		return fabs(a) * sqrt(1 + c*c);
	}
}
} /* TNT namespace */


#endif
/* MATH_UTILS_H */

    
#endif


#ifndef TNT_VERSION_H
#define TNT_VERSION_H


//---------------------------------------------------------------------
//  current version 
//---------------------------------------------------------------------


#define TNT_MAJOR_VERSION    '1'
#define TNT_MINOR_VERSION    '2'
#define TNT_SUBMINOR_VERSION '6'
#define TNT_VERSION_STRING "1.2.6"





#endif
// TNT_VERSION_H

#ifndef JAMA_SVD_H
#define JAMA_SVD_H

using namespace TNT;
using namespace std;

namespace JAMA
{
#define TNT_JAMA_MAX_SVD_ITER 200000
   /** Singular Value Decomposition.
   <P>
   For an m-by-n matrix A with m >= n, the singular value decomposition is
   an m-by-n orthogonal matrix U, an n-by-n diagonal matrix S, and
   an n-by-n orthogonal matrix V so that A = U*S*V'.
   <P>
   The singular values, sigma[k] = S[k][k], are ordered so that
   sigma[0] >= sigma[1] >= ... >= sigma[n-1].
   <P>
   The singular value decompostion always exists, so the constructor will
   never fail.  The matrix condition number and the effective numerical
   rank can be computed from this decomposition.

   <p>
	(Adapted from JAMA, a Java Matrix Library, developed by jointly 
	by the Mathworks and NIST; see  http://math.nist.gov/javanumerics/jama).
   */
template <class Real>
class SVD 
{


	Array2D<Real> U, V;
	Array1D<Real> s;
	int m, n;

  public:

        bool over_iter_limit;

   SVD (const Array2D<Real> &Arg) {

      m = Arg.dim1();
      n = Arg.dim2();
      int nu = min(m,n);
      s = Array1D<Real>(min(m+1,n)); 
      U = Array2D<Real>(m, nu, Real(0));
      V = Array2D<Real>(n,n);
      Array1D<Real> e(n);
      Array1D<Real> work(m);
      Array2D<Real> A(Arg.copy());
      int wantu = 1;  					/* boolean */
      int wantv = 1;  					/* boolean */
      int i=0, j=0, k=0;

      // Reduce A to bidiagonal form, storing the diagonal elements
      // in s and the super-diagonal elements in e.

      int nct = min(m-1,n);
      int nrt = max(0,min(n-2,m));
      for (k = 0; k < max(nct,nrt); k++) {
         if (k < nct) {

            // Compute the transformation for the k-th column and
            // place the k-th diagonal in s[k].
            // Compute 2-norm of k-th column without under/overflow.
            s[k] = 0;
            for (i = k; i < m; i++) {
               s[k] = hypot(s[k],A[i][k]);
            }
            if (s[k] != 0.0) {
               if (A[k][k] < 0.0) {
                  s[k] = -s[k];
               }
               for (i = k; i < m; i++) {
                  A[i][k] /= s[k];
               }
               A[k][k] += 1.0;
            }
            s[k] = -s[k];
         }
         for (j = k+1; j < n; j++) {
            if ((k < nct) && (s[k] != 0.0))  {

            // Apply the transformation.

               Real t(0.0);
               for (i = k; i < m; i++) {
                  t += A[i][k]*A[i][j];
               }
               t = -t/A[k][k];
               for (i = k; i < m; i++) {
                  A[i][j] += t*A[i][k];
               }
            }

            // Place the k-th row of A into e for the
            // subsequent calculation of the row transformation.

            e[j] = A[k][j];
         }
         if (wantu & (k < nct)) {

            // Place the transformation in U for subsequent back
            // multiplication.

            for (i = k; i < m; i++) {
               U[i][k] = A[i][k];
            }
         }
         if (k < nrt) {

            // Compute the k-th row transformation and place the
            // k-th super-diagonal in e[k].
            // Compute 2-norm without under/overflow.
            e[k] = 0;
            for (i = k+1; i < n; i++) {
               e[k] = hypot(e[k],e[i]);
            }
            if (e[k] != 0.0) {
               if (e[k+1] < 0.0) {
                  e[k] = -e[k];
               }
               for (i = k+1; i < n; i++) {
                  e[i] /= e[k];
               }
               e[k+1] += 1.0;
            }
            e[k] = -e[k];
            if ((k+1 < m) & (e[k] != 0.0)) {

            // Apply the transformation.

               for (i = k+1; i < m; i++) {
                  work[i] = 0.0;
               }
               for (j = k+1; j < n; j++) {
                  for (i = k+1; i < m; i++) {
                     work[i] += e[j]*A[i][j];
                  }
               }
               for (j = k+1; j < n; j++) {
                  Real t(-e[j]/e[k+1]);
                  for (i = k+1; i < m; i++) {
                     A[i][j] += t*work[i];
                  }
               }
            }
            if (wantv) {

            // Place the transformation in V for subsequent
            // back multiplication.

               for (i = k+1; i < n; i++) {
                  V[i][k] = e[i];
               }
            }
         }
      }

      // Set up the final bidiagonal matrix or order p.

      int p = min(n,m+1);
      if (nct < n) {
         s[nct] = A[nct][nct];
      }
      if (m < p) {
         s[p-1] = 0.0;
      }
      if (nrt+1 < p) {
         e[nrt] = A[nrt][p-1];
      }
      e[p-1] = 0.0;

      // If required, generate U.

      if (wantu) {
         for (j = nct; j < nu; j++) {
            for (i = 0; i < m; i++) {
               U[i][j] = 0.0;
            }
            U[j][j] = 1.0;
         }
         for (k = nct-1; k >= 0; k--) {
            if (s[k] != 0.0) {
               for (j = k+1; j < nu; j++) {
                  Real t(0.0);
                  for (i = k; i < m; i++) {
                     t += U[i][k]*U[i][j];
                  }
                  t = -t/U[k][k];
                  for (i = k; i < m; i++) {
                     U[i][j] += t*U[i][k];
                  }
               }
               for (i = k; i < m; i++ ) {
                  U[i][k] = -U[i][k];
               }
               U[k][k] = 1.0 + U[k][k];
               for (i = 0; i < k-1; i++) {
                  U[i][k] = 0.0;
               }
            } else {
               for (i = 0; i < m; i++) {
                  U[i][k] = 0.0;
               }
               U[k][k] = 1.0;
            }
         }
      }

      // If required, generate V.

      if (wantv) {
         for (k = n-1; k >= 0; k--) {
            if ((k < nrt) & (e[k] != 0.0)) {
               for (j = k+1; j < nu; j++) {
                  Real t(0.0);
                  for (i = k+1; i < n; i++) {
                     t += V[i][k]*V[i][j];
                  }
                  t = -t/V[k+1][k];
                  for (i = k+1; i < n; i++) {
                     V[i][j] += t*V[i][k];
                  }
               }
            }
            for (i = 0; i < n; i++) {
               V[i][k] = 0.0;
            }
            V[k][k] = 1.0;
         }
      }

      // Main iteration loop for the singular values.

      int pp = p-1;
      int iter = 0;
      Real eps(pow(2.0,-52.0));
      over_iter_limit = false;
      while (p > 0) {
         int k=0;
         int kase=0;

         // Here is where a test for too many iterations would go.

         if ( iter > TNT_JAMA_MAX_SVD_ITER )
         {
            over_iter_limit = true;
         }

         // This section of the program inspects for
         // negligible elements in the s and e arrays.  On
         // completion the variables kase and k are set as follows.

         // kase = 1     if s(p) and e[k-1] are negligible and k<p
         // kase = 2     if s(k) is negligible and k<p
         // kase = 3     if e[k-1] is negligible, k<p, and
         //              s(k), ..., s(p) are not negligible (qr step).
         // kase = 4     if e(p-1) is negligible (convergence).

         for (k = p-2; k >= -1; k--) {
            if (k == -1) {
               break;
            }
            if (abs(e[k]) <= eps*(abs(s[k]) + abs(s[k+1]))) {
               e[k] = 0.0;
               break;
            }
         }
         if ( k == p-2 ) {
            kase = 4;
         } else {
            int ks;
            for (ks = p-1; ks >= k; ks--) {
               if (ks == k) {
                  break;
               }
               Real t( (ks != p ? abs(e[ks]) : 0.) + 
                          (ks != k+1 ? abs(e[ks-1]) : 0.));
               if (abs(s[ks]) <= eps*t)  {
                  s[ks] = 0.0;
                  break;
               }
            }
            if (ks == k) {
               kase = 3;
            } else if (ks == p-1) {
               kase = 1;
            } else {
               kase = 2;
               k = ks;
            }
         }
         k++;

         // Perform the task indicated by kase.

         if ( over_iter_limit )
         {
            kase = 4;
         }

         switch (kase) {

            // Deflate negligible s(p).

            case 1: {
               Real f(e[p-2]);
               e[p-2] = 0.0;
               for (j = p-2; j >= k; j--) {
                  Real t( hypot(s[j],f));
                  Real cs(s[j]/t);
                  Real sn(f/t);
                  s[j] = t;
                  if (j != k) {
                     f = -sn*e[j-1];
                     e[j-1] = cs*e[j-1];
                  }
                  if (wantv) {
                     for (i = 0; i < n; i++) {
                        t = cs*V[i][j] + sn*V[i][p-1];
                        V[i][p-1] = -sn*V[i][j] + cs*V[i][p-1];
                        V[i][j] = t;
                     }
                  }
               }
            }
            break;

            // Split at negligible s(k).

            case 2: {
               Real f(e[k-1]);
               e[k-1] = 0.0;
               for (j = k; j < p; j++) {
                  Real t(hypot(s[j],f));
                  Real cs( s[j]/t);
                  Real sn(f/t);
                  s[j] = t;
                  f = -sn*e[j];
                  e[j] = cs*e[j];
                  if (wantu) {
                     for (i = 0; i < m; i++) {
                        t = cs*U[i][j] + sn*U[i][k-1];
                        U[i][k-1] = -sn*U[i][j] + cs*U[i][k-1];
                        U[i][j] = t;
                     }
                  }
               }
            }
            break;

            // Perform one qr step.

            case 3: {

               // Calculate the shift.
   
               Real scale = max(max(max(max(
                       abs(s[p-1]),abs(s[p-2])),abs(e[p-2])), 
                       abs(s[k])),abs(e[k]));
               Real sp = s[p-1]/scale;
               Real spm1 = s[p-2]/scale;
               Real epm1 = e[p-2]/scale;
               Real sk = s[k]/scale;
               Real ek = e[k]/scale;
               Real b = ((spm1 + sp)*(spm1 - sp) + epm1*epm1)/2.0;
               Real c = (sp*epm1)*(sp*epm1);
               Real shift = 0.0;
               if ((b != 0.0) || (c != 0.0)) {
                  shift = sqrt(b*b + c);
                  if (b < 0.0) {
                     shift = -shift;
                  }
                  shift = c/(b + shift);
               }
               Real f = (sk + sp)*(sk - sp) + shift;
               Real g = sk*ek;
   
               // Chase zeros.
   
               for (j = k; j < p-1; j++) {
                  Real t = hypot(f,g);
                  Real cs = f/t;
                  Real sn = g/t;
                  if (j != k) {
                     e[j-1] = t;
                  }
                  f = cs*s[j] + sn*e[j];
                  e[j] = cs*e[j] - sn*s[j];
                  g = sn*s[j+1];
                  s[j+1] = cs*s[j+1];
                  if (wantv) {
                     for (i = 0; i < n; i++) {
                        t = cs*V[i][j] + sn*V[i][j+1];
                        V[i][j+1] = -sn*V[i][j] + cs*V[i][j+1];
                        V[i][j] = t;
                     }
                  }
                  t = hypot(f,g);
                  cs = f/t;
                  sn = g/t;
                  s[j] = t;
                  f = cs*e[j] + sn*s[j+1];
                  s[j+1] = -sn*e[j] + cs*s[j+1];
                  g = sn*e[j+1];
                  e[j+1] = cs*e[j+1];
                  if (wantu && (j < m-1)) {
                     for (i = 0; i < m; i++) {
                        t = cs*U[i][j] + sn*U[i][j+1];
                        U[i][j+1] = -sn*U[i][j] + cs*U[i][j+1];
                        U[i][j] = t;
                     }
                  }
               }
               e[p-2] = f;
               iter = iter + 1;
            }
            break;

            // Convergence.

            case 4: {

               // Make the singular values positive.
   
               if (s[k] <= 0.0) {
                  s[k] = (s[k] < 0.0 ? -s[k] : 0.0);
                  if (wantv) {
                     for (i = 0; i <= pp; i++) {
                        V[i][k] = -V[i][k];
                     }
                  }
               }
   
               // Order the singular values.
   
               while (k < pp) {
                  if (s[k] >= s[k+1]) {
                     break;
                  }
                  Real t = s[k];
                  s[k] = s[k+1];
                  s[k+1] = t;
                  if (wantv && (k < n-1)) {
                     for (i = 0; i < n; i++) {
                        t = V[i][k+1]; V[i][k+1] = V[i][k]; V[i][k] = t;
                     }
                  }
                  if (wantu && (k < m-1)) {
                     for (i = 0; i < m; i++) {
                        t = U[i][k+1]; U[i][k+1] = U[i][k]; U[i][k] = t;
                     }
                  }
                  k++;
               }
               iter = 0;
               p--;
            }
            break;
         }
      }
   }

   void getU (Array2D<Real> &A) 
   {
   	  int minm = min(m+1,n);

	  A = Array2D<Real>(m, minm);

	  for (int i=0; i<m; i++)
	  	for (int j=0; j<minm; j++)
			A[i][j] = U[i][j];
   	
   }

   /* Return the right singular vectors */

   void getV (Array2D<Real> &A) 
   {
   	  A = V;
   }

   /** Return the one-dimensional array of singular values */

   void getSingularValues (Array1D<Real> &x) 
   {
      x = s;
   }

   /** Return the diagonal matrix of singular values
   @return     S
   */

   void getS (Array2D<Real> &A) {
   	  A = Array2D<Real>(n,n);
      for (int i = 0; i < n; i++) {
         for (int j = 0; j < n; j++) {
            A[i][j] = 0.0;
         }
         A[i][i] = s[i];
      }
   }

   /** Two norm  (max(S)) */

   Real norm2 () {
      return s[0];
   }

   /** Two norm of condition number (max(S)/min(S)) */

   Real cond () {
      return s[0]/s[min(m,n)-1];
   }

   /** Effective numerical matrix rank
   @return     Number of nonnegligible singular values.
   */

   int rank () 
   {
      Real eps = pow(2.0,-52.0);
      Real tol = max(m,n)*s[0]*eps;
      int r = 0;
      for (int i = 0; i < s.dim(); i++) {
         if (s[i] > tol) {
            r++;
         }
      }
      return r;
   }
};

}

template <class T>
T det(const TNT::Array2D<T>& m) {
   assert(m.dim1() == 3);
   assert(m.dim2() == 3);
   
   return (m[0][0]*(m[1][1]*m[2][2] - m[1][2]*m[2][1]) -
           m[0][1]*(m[1][0]*m[2][2] - m[1][2]*m[2][0]) +
           m[0][2]*(m[1][0]*m[2][1] - m[1][1]*m[2][0]));
}
   
// Transpose a matrix
template <class T>
TNT::Array2D<T> transpose(const TNT::Array2D<T>& m) {
   TNT::Array2D<T> mt(m.dim2(), m.dim1());
   for (int i = 0; i < m.dim1(); i++) {
      for (int j = 0; j < m.dim2(); j++) {
         mt[j][i] = m[i][j];
      }
   }
   return mt;
}

#endif
// JAMA_SVD_H
