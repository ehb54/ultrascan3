#ifndef US_ZENO_CXX_H
#define US_ZENO_CXX_H
// *** START *** /root/zeno/nanoflann-master/include/nanoflann.hpp ***
/***********************************************************************
 * Software License Agreement (BSD License)
 *
 * Copyright 2008-2009  Marius Muja (mariusm@cs.ubc.ca). All rights reserved.
 * Copyright 2008-2009  David G. Lowe (lowe@cs.ubc.ca). All rights reserved.
 * Copyright 2011-2014  Jose Luis Blanco (joseluisblancoc@gmail.com).
 *   All rights reserved.
 *
 * THE BSD LICENSE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************/

/** \mainpage nanoflann C++ API documentation 
  *  nanoflann is a C++ header-only library for building KD-Trees, mostly 
  *  optimized for 2D or 3D point clouds. 
  *  
  *  nanoflann does not require compiling or installing, just an 
  *  // #include <nanoflann.hpp> in your code.  
  *  
  *  See: 
  *   - <a href="modules.html" >C++ API organized by modules</a>
  *   - <a href="https://github.com/jlblancoc/nanoflann" >Online README</a>
  */

#ifndef  NANOFLANN_HPP_
#define  NANOFLANN_HPP_

#include <vector>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <cstdio>  // for fwrite()
#include <cmath>   // for fabs(),...
#include <limits>

// Avoid conflicting declaration of min/max macros in windows headers
#if !defined(NOMINMAX) && (defined(_WIN32) || defined(_WIN32_)  || defined(WIN32) || defined(_WIN64))
# define NOMINMAX
# ifdef max
#  undef   max
#  undef   min
# endif
#endif

namespace nanoflann
{
/** @addtogroup nanoflann_grp nanoflann C++ library for ANN
  *  @{ */

  	/** Library version: 0xMmP (M=Major,m=minor,P=patch) */
	#define NANOFLANN_VERSION 0x119

	/** @addtogroup result_sets_grp Result set classes
	  *  @{ */
	template <typename DistanceType, typename IndexType = size_t, typename CountType = size_t>
	class KNNResultSet
	{
		IndexType * indices;
		DistanceType* dists;
		CountType capacity;
		CountType count;

	public:
		inline KNNResultSet(CountType capacity_) : indices(0), dists(0), capacity(capacity_), count(0)
		{
		}

		inline void init(IndexType* indices_, DistanceType* dists_)
		{
			indices = indices_;
			dists = dists_;
			count = 0;
            if (capacity)
                dists[capacity-1] = (std::numeric_limits<DistanceType>::max)();
		}

		inline CountType size() const
		{
			return count;
		}

		inline bool full() const
		{
			return count == capacity;
		}


		inline void addPoint(DistanceType dist, IndexType index)
		{
			CountType i;
			for (i=count; i>0; --i) {
#ifdef NANOFLANN_FIRST_MATCH   // If defined and two points have the same distance, the one with the lowest-index will be returned first.
				if ( (dists[i-1]>dist) || ((dist==dists[i-1])&&(indices[i-1]>index)) ) {
#else
				if (dists[i-1]>dist) {
#endif
					if (i<capacity) {
						dists[i] = dists[i-1];
						indices[i] = indices[i-1];
					}
				}
				else break;
			}
			if (i<capacity) {
				dists[i] = dist;
				indices[i] = index;
			}
			if (count<capacity) count++;
		}

		inline DistanceType worstDist() const
		{
			return dists[capacity-1];
		}
	};


	/**
	 * A result-set class used when performing a radius based search.
	 */
	template <typename DistanceType, typename IndexType = size_t>
	class RadiusResultSet
	{
	public:
		const DistanceType radius;

		std::vector<std::pair<IndexType,DistanceType> >& m_indices_dists;

		inline RadiusResultSet(DistanceType radius_, std::vector<std::pair<IndexType,DistanceType> >& indices_dists) : radius(radius_), m_indices_dists(indices_dists)
		{
			init();
		}

		inline ~RadiusResultSet() { }

		inline void init( ) { clear( ); }
		inline void clear( ) { m_indices_dists.clear( ); }

		inline size_t size() const { return m_indices_dists.size(); }

		inline bool full() const { return true; }

		inline void addPoint(DistanceType dist, IndexType index)
		{
			if (dist<radius)
				m_indices_dists.push_back(std::make_pair(index,dist));
		}

		inline DistanceType worstDist() const { return radius; }

		/** Clears the result set and adjusts the search radius. */
		inline void set_radius_and_clear( const DistanceType r )
		{
			radius = r;
			clear( );
		}

		/**
		 * Find the worst result (furtherest neighbor) without copying or sorting
		 * Pre-conditions: size() > 0
		 */
		std::pair<IndexType,DistanceType> worst_item() const
		{
		   if (m_indices_dists.empty()) throw std::runtime_error("Cannot invoke RadiusResultSet::worst_item() on an empty list of results.");
		   typedef typename std::vector<std::pair<IndexType,DistanceType> >::const_iterator DistIt;
		   DistIt it = std::max_element(m_indices_dists.begin(), m_indices_dists.end());
		   return *it;
		}
	};

	/** operator "<" for std::sort() */
	struct IndexDist_Sorter
	{
		/** PairType will be typically: std::pair<IndexType,DistanceType> */
		template <typename PairType>
		inline bool operator()(const PairType &p1, const PairType &p2) const {
			return p1.second < p2.second;
		}
	};

	/** @} */


	/** @addtogroup loadsave_grp Load/save auxiliary functions
	  * @{ */
	template<typename T>
	void save_value(FILE* stream, const T& value, size_t count = 1)
	{
		fwrite(&value, sizeof(value),count, stream);
	}

	template<typename T>
	void save_value(FILE* stream, const std::vector<T>& value)
	{
		size_t size = value.size();
		fwrite(&size, sizeof(size_t), 1, stream);
		fwrite(&value[0], sizeof(T), size, stream);
	}

	template<typename T>
	void load_value(FILE* stream, T& value, size_t count = 1)
	{
		size_t read_cnt = fread(&value, sizeof(value), count, stream);
		if (read_cnt != count) {
			throw std::runtime_error("Cannot read from file");
		}
	}


	template<typename T>
	void load_value(FILE* stream, std::vector<T>& value)
	{
		size_t size;
		size_t read_cnt = fread(&size, sizeof(size_t), 1, stream);
		if (read_cnt!=1) {
			throw std::runtime_error("Cannot read from file");
		}
		value.resize(size);
		read_cnt = fread(&value[0], sizeof(T), size, stream);
		if (read_cnt!=size) {
			throw std::runtime_error("Cannot read from file");
		}
	}
	/** @} */


	/** @addtogroup metric_grp Metric (distance) classes
	  * @{ */

	template<typename T> inline T abs(T x) { return (x<0) ? -x : x; }
	template<> inline int abs<int>(int x) { return ::abs(x); }
	template<> inline float abs<float>(float x) { return fabsf(x); }
	template<> inline double abs<double>(double x) { return fabs(x); }
	template<> inline long double abs<long double>(long double x) { return fabsl(x); }

	/** Manhattan distance functor (generic version, optimized for high-dimensionality data sets).
	  *  Corresponding distance traits: nanoflann::metric_L1
	  * \tparam T Type of the elements (e.g. double, float, uint8_t)
	  * \tparam _DistanceType Type of distance variables (must be signed) (e.g. float, double, int64_t)
	  */
	template<class T, class DataSource, typename _DistanceType = T>
	struct L1_Adaptor
	{
		typedef T ElementType;
		typedef _DistanceType DistanceType;

		const DataSource &data_source;

		L1_Adaptor(const DataSource &_data_source) : data_source(_data_source) { }

		inline DistanceType operator()(const T* a, const size_t b_idx, size_t size, DistanceType worst_dist = -1) const
		{
			DistanceType result = DistanceType();
			const T* last = a + size;
			const T* lastgroup = last - 3;
			size_t d = 0;

			/* Process 4 items with each loop for efficiency. */
			while (a < lastgroup) {
				const DistanceType diff0 = nanoflann::abs(a[0] - data_source.kdtree_get_pt(b_idx,d++));
				const DistanceType diff1 = nanoflann::abs(a[1] - data_source.kdtree_get_pt(b_idx,d++));
				const DistanceType diff2 = nanoflann::abs(a[2] - data_source.kdtree_get_pt(b_idx,d++));
				const DistanceType diff3 = nanoflann::abs(a[3] - data_source.kdtree_get_pt(b_idx,d++));
				result += diff0 + diff1 + diff2 + diff3;
				a += 4;
				if ((worst_dist>0)&&(result>worst_dist)) {
					return result;
				}
			}
			/* Process last 0-3 components.  Not needed for standard vector lengths. */
			while (a < last) {
				result += nanoflann::abs( *a++ - data_source.kdtree_get_pt(b_idx,d++) );
			}
			return result;
		}

		template <typename U, typename V>
		inline DistanceType accum_dist(const U a, const V b, int ) const
		{
			return nanoflann::abs(a-b);
		}
	};

	/** Squared Euclidean distance functor (generic version, optimized for high-dimensionality data sets).
	  *  Corresponding distance traits: nanoflann::metric_L2
	  * \tparam T Type of the elements (e.g. double, float, uint8_t)
	  * \tparam _DistanceType Type of distance variables (must be signed) (e.g. float, double, int64_t)
	  */
	template<class T, class DataSource, typename _DistanceType = T>
	struct L2_Adaptor
	{
		typedef T ElementType;
		typedef _DistanceType DistanceType;

		const DataSource &data_source;

		L2_Adaptor(const DataSource &_data_source) : data_source(_data_source) { }

		inline DistanceType operator()(const T* a, const size_t b_idx, size_t size, DistanceType worst_dist = -1) const
		{
			DistanceType result = DistanceType();
			const T* last = a + size;
			const T* lastgroup = last - 3;
			size_t d = 0;

			/* Process 4 items with each loop for efficiency. */
			while (a < lastgroup) {
				const DistanceType diff0 = a[0] - data_source.kdtree_get_pt(b_idx,d++);
				const DistanceType diff1 = a[1] - data_source.kdtree_get_pt(b_idx,d++);
				const DistanceType diff2 = a[2] - data_source.kdtree_get_pt(b_idx,d++);
				const DistanceType diff3 = a[3] - data_source.kdtree_get_pt(b_idx,d++);
				result += diff0 * diff0 + diff1 * diff1 + diff2 * diff2 + diff3 * diff3;
				a += 4;
				if ((worst_dist>0)&&(result>worst_dist)) {
					return result;
				}
			}
			/* Process last 0-3 components.  Not needed for standard vector lengths. */
			while (a < last) {
				const DistanceType diff0 = *a++ - data_source.kdtree_get_pt(b_idx,d++);
				result += diff0 * diff0;
			}
			return result;
		}

		template <typename U, typename V>
		inline DistanceType accum_dist(const U a, const V b, int ) const
		{
			return (a-b)*(a-b);
		}
	};

	/** Squared Euclidean (L2) distance functor (suitable for low-dimensionality datasets, like 2D or 3D point clouds)
	  *  Corresponding distance traits: nanoflann::metric_L2_Simple
	  * \tparam T Type of the elements (e.g. double, float, uint8_t)
	  * \tparam _DistanceType Type of distance variables (must be signed) (e.g. float, double, int64_t)
	  */
	template<class T, class DataSource, typename _DistanceType = T>
	struct L2_Simple_Adaptor
	{
		typedef T ElementType;
		typedef _DistanceType DistanceType;

		const DataSource &data_source;

		L2_Simple_Adaptor(const DataSource &_data_source) : data_source(_data_source) { }

		inline DistanceType operator()(const T* a, const size_t b_idx, size_t size) const {
			return data_source.kdtree_distance(a,b_idx,size);
		}

		template <typename U, typename V>
		inline DistanceType accum_dist(const U a, const V b, int ) const
		{
			return (a-b)*(a-b);
		}
	};

	/** Metaprogramming helper traits class for the L1 (Manhattan) metric */
	struct metric_L1 {
		template<class T, class DataSource>
		struct traits {
			typedef L1_Adaptor<T,DataSource> distance_t;
		};
	};
	/** Metaprogramming helper traits class for the L2 (Euclidean) metric */
	struct metric_L2 {
		template<class T, class DataSource>
		struct traits {
			typedef L2_Adaptor<T,DataSource> distance_t;
		};
	};
	/** Metaprogramming helper traits class for the L2_simple (Euclidean) metric */
	struct metric_L2_Simple {
		template<class T, class DataSource>
		struct traits {
			typedef L2_Simple_Adaptor<T,DataSource> distance_t;
		};
	};

	/** @} */

	/** @addtogroup param_grp Parameter structs
	  * @{ */

	/**  Parameters (see README.md) */
	struct KDTreeSingleIndexAdaptorParams
	{
		KDTreeSingleIndexAdaptorParams(size_t _leaf_max_size = 10) :
			leaf_max_size(_leaf_max_size)
		{}

		size_t leaf_max_size;
	};

	/** Search options for KDTreeSingleIndexAdaptor::findNeighbors() */
	struct SearchParams
	{
		/** Note: The first argument (checks_IGNORED_) is ignored, but kept for compatibility with the FLANN interface */
		SearchParams(int checks_IGNORED_ = 32, float eps_ = 0, bool sorted_ = true ) :
			checks(checks_IGNORED_), eps(eps_), sorted(sorted_) {}

		int   checks;  //!< Ignored parameter (Kept for compatibility with the FLANN interface).
		float eps;  //!< search for eps-approximate neighbours (default: 0)
		bool sorted; //!< only for radius search, require neighbours sorted by distance (default: true)
	};
	/** @} */


	/** @addtogroup memalloc_grp Memory allocation
	  * @{ */

	/**
	 * Allocates (using C's malloc) a generic type T.
	 *
	 * Params:
	 *     count = number of instances to allocate.
	 * Returns: pointer (of type T*) to memory buffer
	 */
	template <typename T>
	inline T* allocate(size_t count = 1)
	{
		T* mem = static_cast<T*>( ::malloc(sizeof(T)*count));
		return mem;
	}


	/**
	 * Pooled storage allocator
	 *
	 * The following routines allow for the efficient allocation of storage in
	 * small chunks from a specified pool.  Rather than allowing each structure
	 * to be freed individually, an entire pool of storage is freed at once.
	 * This method has two advantages over just using malloc() and free().  First,
	 * it is far more efficient for allocating small objects, as there is
	 * no overhead for remembering all the information needed to free each
	 * object or consolidating fragmented memory.  Second, the decision about
	 * how long to keep an object is made at the time of allocation, and there
	 * is no need to track down all the objects to free them.
	 *
	 */

	const size_t     WORDSIZE=16;
	const size_t     BLOCKSIZE=8192;

	class PooledAllocator
	{
		/* We maintain memory alignment to word boundaries by requiring that all
		    allocations be in multiples of the machine wordsize.  */
		/* Size of machine word in bytes.  Must be power of 2. */
		/* Minimum number of bytes requested at a time from	the system.  Must be multiple of WORDSIZE. */


		size_t  remaining;  /* Number of bytes left in current block of storage. */
		void*   base;     /* Pointer to base of current block of storage. */
		void*   loc;      /* Current location in block to next allocate memory. */

		void internal_init()
		{
			remaining = 0;
			base = NULL;
			usedMemory = 0;
			wastedMemory = 0;
		}

	public:
		size_t  usedMemory;
		size_t  wastedMemory;

		/**
		    Default constructor. Initializes a new pool.
		 */
		PooledAllocator() {
			internal_init();
		}

		/**
		 * Destructor. Frees all the memory allocated in this pool.
		 */
		~PooledAllocator() {
			free_all();
		}

		/** Frees all allocated memory chunks */
		void free_all()
		{
			while (base != NULL) {
				void *prev = *(static_cast<void**>( base)); /* Get pointer to prev block. */
				::free(base);
				base = prev;
			}
			internal_init();
		}

		/**
		 * Returns a pointer to a piece of new memory of the given size in bytes
		 * allocated from the pool.
		 */
		void* malloc(const size_t req_size)
		{
			/* Round size up to a multiple of wordsize.  The following expression
			    only works for WORDSIZE that is a power of 2, by masking last bits of
			    incremented size to zero.
			 */
			const size_t size = (req_size + (WORDSIZE - 1)) & ~(WORDSIZE - 1);

			/* Check whether a new block must be allocated.  Note that the first word
			    of a block is reserved for a pointer to the previous block.
			 */
			if (size > remaining) {

				wastedMemory += remaining;

				/* Allocate new storage. */
				const size_t blocksize = (size + sizeof(void*) + (WORDSIZE-1) > BLOCKSIZE) ?
							size + sizeof(void*) + (WORDSIZE-1) : BLOCKSIZE;

				// use the standard C malloc to allocate memory
				void* m = ::malloc(blocksize);
				if (!m) {
					fprintf(stderr,"Failed to allocate memory.\n");
					return NULL;
				}

				/* Fill first word of new block with pointer to previous block. */
				static_cast<void**>(m)[0] = base;
				base = m;

				size_t shift = 0;
				//int size_t = (WORDSIZE - ( (((size_t)m) + sizeof(void*)) & (WORDSIZE-1))) & (WORDSIZE-1);

				remaining = blocksize - sizeof(void*) - shift;
				loc = (static_cast<char*>(m) + sizeof(void*) + shift);
			}
			void* rloc = loc;
			loc = static_cast<char*>(loc) + size;
			remaining -= size;

			usedMemory += size;

			return rloc;
		}

		/**
		 * Allocates (using this pool) a generic type T.
		 *
		 * Params:
		 *     count = number of instances to allocate.
		 * Returns: pointer (of type T*) to memory buffer
		 */
		template <typename T>
		T* allocate(const size_t count = 1)
		{
			T* mem = static_cast<T*>(this->malloc(sizeof(T)*count));
			return mem;
		}

	};
	/** @} */

	/** @addtogroup nanoflann_metaprog_grp Auxiliary metaprogramming stuff
	  * @{ */

	// ----------------  CArray -------------------------
	/** A STL container (as wrapper) for arrays of constant size defined at compile time (class imported from the MRPT project)
	 * This code is an adapted version from Boost, modifed for its integration
	 *	within MRPT (JLBC, Dec/2009) (Renamed array -> CArray to avoid possible potential conflicts).
	 * See
	 *      http://www.josuttis.com/cppcode
	 * for details and the latest version.
	 * See
	 *      http://www.boost.org/libs/array for Documentation.
	 * for documentation.
	 *
	 * (C) Copyright Nicolai M. Josuttis 2001.
	 * Permission to copy, use, modify, sell and distribute this software
	 * is granted provided this copyright notice appears in all copies.
	 * This software is provided "as is" without express or implied
	 * warranty, and with no claim as to its suitability for any purpose.
	 *
	 * 29 Jan 2004 - minor fixes (Nico Josuttis)
	 * 04 Dec 2003 - update to synch with library TR1 (Alisdair Meredith)
	 * 23 Aug 2002 - fix for Non-MSVC compilers combined with MSVC libraries.
	 * 05 Aug 2001 - minor update (Nico Josuttis)
	 * 20 Jan 2001 - STLport fix (Beman Dawes)
	 * 29 Sep 2000 - Initial Revision (Nico Josuttis)
	 *
	 * Jan 30, 2004
	 */
    template <typename T, std::size_t N>
    class CArray {
      public:
        T elems[N];    // fixed-size array of elements of type T

      public:
        // type definitions
        typedef T              value_type;
        typedef T*             iterator;
        typedef const T*       const_iterator;
        typedef T&             reference;
        typedef const T&       const_reference;
        typedef std::size_t    size_type;
        typedef std::ptrdiff_t difference_type;

        // iterator support
        inline iterator begin() { return elems; }
        inline const_iterator begin() const { return elems; }
        inline iterator end() { return elems+N; }
        inline const_iterator end() const { return elems+N; }

        // reverse iterator support
#if !defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION) && !defined(BOOST_MSVC_STD_ITERATOR) && !defined(BOOST_NO_STD_ITERATOR_TRAITS)
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
#elif defined(_MSC_VER) && (_MSC_VER == 1300) && defined(BOOST_DINKUMWARE_STDLIB) && (BOOST_DINKUMWARE_STDLIB == 310)
        // workaround for broken reverse_iterator in VC7
        typedef std::reverse_iterator<std::_Ptrit<value_type, difference_type, iterator,
                                      reference, iterator, reference> > reverse_iterator;
        typedef std::reverse_iterator<std::_Ptrit<value_type, difference_type, const_iterator,
                                      const_reference, iterator, reference> > const_reverse_iterator;
#else
        // workaround for broken reverse_iterator implementations
        typedef std::reverse_iterator<iterator,T> reverse_iterator;
        typedef std::reverse_iterator<const_iterator,T> const_reverse_iterator;
#endif

        reverse_iterator rbegin() { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
        reverse_iterator rend() { return reverse_iterator(begin()); }
        const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
        // operator[]
        inline reference operator[](size_type i) { return elems[i]; }
        inline const_reference operator[](size_type i) const { return elems[i]; }
        // at() with range check
        reference at(size_type i) { rangecheck(i); return elems[i]; }
        const_reference at(size_type i) const { rangecheck(i); return elems[i]; }
        // front() and back()
        reference front() { return elems[0]; }
        const_reference front() const { return elems[0]; }
        reference back() { return elems[N-1]; }
        const_reference back() const { return elems[N-1]; }
        // size is constant
        static inline size_type size() { return N; }
        static bool empty() { return false; }
        static size_type max_size() { return N; }
        enum { static_size = N };
		/** This method has no effects in this class, but raises an exception if the expected size does not match */
		inline void resize(const size_t nElements) { if (nElements!=N) throw std::logic_error("Try to change the size of a CArray."); }
        // swap (note: linear complexity in N, constant for given instantiation)
        void swap (CArray<T,N>& y) { std::swap_ranges(begin(),end(),y.begin()); }
        // direct access to data (read-only)
        const T* data() const { return elems; }
        // use array as C array (direct read/write access to data)
        T* data() { return elems; }
        // assignment with type conversion
        template <typename T2> CArray<T,N>& operator= (const CArray<T2,N>& rhs) {
            std::copy(rhs.begin(),rhs.end(), begin());
            return *this;
        }
        // assign one value to all elements
        inline void assign (const T& value) { for (size_t i=0;i<N;i++) elems[i]=value; }
        // assign (compatible with std::vector's one) (by JLBC for MRPT)
        void assign (const size_t n, const T& value) { assert(N==n); for (size_t i=0;i<N;i++) elems[i]=value; }
      private:
        // check range (may be private because it is static)
        static void rangecheck (size_type i) { if (i >= size()) { throw std::out_of_range("CArray<>: index out of range"); } }
    }; // end of CArray

	/** Used to declare fixed-size arrays when DIM>0, dynamically-allocated vectors when DIM=-1.
	  * Fixed size version for a generic DIM:
	  */
	template <int DIM, typename T>
	struct array_or_vector_selector
	{
		typedef CArray<T,DIM> container_t;
	};
	/** Dynamic size version */
	template <typename T>
	struct array_or_vector_selector<-1,T> {
		typedef std::vector<T> container_t;
	};
	/** @} */

	/** @addtogroup kdtrees_grp KD-tree classes and adaptors
	  * @{ */

	/** kd-tree index
	 *
	 * Contains the k-d trees and other information for indexing a set of points
	 * for nearest-neighbor matching.
	 *
	 *  The class "DatasetAdaptor" must provide the following interface (can be non-virtual, inlined methods):
	 *
	 *  \code
	 *   // Must return the number of data poins
	 *   inline size_t kdtree_get_point_count() const { ... }
	 *
	 *   // [Only if using the metric_L2_Simple type] Must return the Euclidean (L2) distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
	 *   inline DistanceType kdtree_distance(const T *p1, const size_t idx_p2,size_t size) const { ... }
	 *
	 *   // Must return the dim'th component of the idx'th point in the class:
	 *   inline T kdtree_get_pt(const size_t idx, int dim) const { ... }
	 *
	 *   // Optional bounding-box computation: return false to default to a standard bbox computation loop.
	 *   //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
	 *   //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
	 *   template <class BBOX>
	 *   bool kdtree_get_bbox(BBOX &bb) const
	 *   {
	 *      bb[0].low = ...; bb[0].high = ...;  // 0th dimension limits
	 *      bb[1].low = ...; bb[1].high = ...;  // 1st dimension limits
	 *      ...
	 *      return true;
	 *   }
	 *
	 *  \endcode
	 * 
	 * \tparam DatasetAdaptor The user-provided adaptor (see comments above).
	 * \tparam Distance The distance metric to use: nanoflann::metric_L1, nanoflann::metric_L2, nanoflann::metric_L2_Simple, etc.
	 * \tparam DIM Dimensionality of data points (e.g. 3 for 3D points)
	 * \tparam IndexType Will be typically size_t or int
	 */
	template <typename Distance, class DatasetAdaptor,int DIM = -1, typename IndexType = size_t>
	class KDTreeSingleIndexAdaptor
	{
	private:
		/** Hidden copy constructor, to disallow copying indices (Not implemented) */
		KDTreeSingleIndexAdaptor(const KDTreeSingleIndexAdaptor<Distance,DatasetAdaptor,DIM,IndexType>&);
	public:
		typedef typename Distance::ElementType  ElementType;
		typedef typename Distance::DistanceType DistanceType;
	protected:

		/**
		 *  Array of indices to vectors in the dataset.
		 */
		std::vector<IndexType> vind;

		size_t m_leaf_max_size;


		/**
		 * The dataset used by this index
		 */
		const DatasetAdaptor &dataset; //!< The source of our data

		const KDTreeSingleIndexAdaptorParams index_params;

		size_t m_size; //!< Number of current poins in the dataset
		size_t m_size_at_index_build; //!< Number of points in the dataset when the index was built
		int dim;  //!< Dimensionality of each data point


		/*--------------------- Internal Data Structures --------------------------*/
		struct Node
		{
			/** Union used because a node can be either a LEAF node or a non-leaf node, so both data fields are never used simultaneously */
			union {
				struct {
					IndexType    left, right;  //!< Indices of points in leaf node
				} lr;
				struct {
					int          divfeat; //!< Dimension used for subdivision.
					DistanceType divlow, divhigh; //!< The values used for subdivision.
				} sub;
			};
			Node* child1, * child2;  //!< Child nodes (both=NULL mean its a leaf node)
		};
		typedef Node* NodePtr;


		struct Interval
		{
			ElementType low, high;
		};

		/** Define "BoundingBox" as a fixed-size or variable-size container depending on "DIM" */
		typedef typename array_or_vector_selector<DIM,Interval>::container_t BoundingBox;

		/** Define "distance_vector_t" as a fixed-size or variable-size container depending on "DIM" */
		typedef typename array_or_vector_selector<DIM,DistanceType>::container_t distance_vector_t;

		/** The KD-tree used to find neighbours */
		NodePtr root_node;
		BoundingBox root_bbox;

		/**
		 * Pooled memory allocator.
		 *
		 * Using a pooled memory allocator is more efficient
		 * than allocating memory directly when there is a large
		 * number small of memory allocations.
		 */
		PooledAllocator pool;

	public:

		Distance distance;

		/**
		 * KDTree constructor
		 *
		 * Refer to docs in README.md or online in https://github.com/jlblancoc/nanoflann
		 *
		 * The KD-Tree point dimension (the length of each point in the datase, e.g. 3 for 3D points)
		 * is determined by means of:
		 *  - The \a DIM template parameter if >0 (highest priority)
		 *  - Otherwise, the \a dimensionality parameter of this constructor.
		 *
		 * @param inputData Dataset with the input features
		 * @param params Basically, the maximum leaf node size
		 */
		KDTreeSingleIndexAdaptor(const int dimensionality, const DatasetAdaptor& inputData, const KDTreeSingleIndexAdaptorParams& params = KDTreeSingleIndexAdaptorParams() ) :
			dataset(inputData), index_params(params), root_node(NULL), distance(inputData)
		{
			m_size = dataset.kdtree_get_point_count();
			m_size_at_index_build = m_size;
			dim = dimensionality;
			if (DIM>0) dim=DIM;
			m_leaf_max_size = params.leaf_max_size;

			// Create a permutable array of indices to the input vectors.
			init_vind();
		}

		/** Standard destructor */
		~KDTreeSingleIndexAdaptor() { }

		/** Frees the previously-built index. Automatically called within buildIndex(). */
		void freeIndex()
		{
			pool.free_all();
			root_node=NULL;
			m_size_at_index_build = 0;
		}

		/**
		 * Builds the index
		 */
		void buildIndex()
		{
			init_vind();
			freeIndex();
			m_size_at_index_build = m_size;
			if(m_size == 0) return;
			computeBoundingBox(root_bbox);
			root_node = divideTree(0, m_size, root_bbox );   // construct the tree
		}

		/** Returns number of points in dataset  */
		size_t size() const { return m_size; }

		/** Returns the length of each point in the dataset */
		size_t veclen() const {
			return static_cast<size_t>(DIM>0 ? DIM : dim);
		}

		/**
		 * Computes the inde memory usage
		 * Returns: memory used by the index
		 */
		size_t usedMemory() const
		{
			return pool.usedMemory+pool.wastedMemory+dataset.kdtree_get_point_count()*sizeof(IndexType);  // pool memory and vind array memory
		}

		/** \name Query methods
		  * @{ */

		/**
		 * Find set of nearest neighbors to vec[0:dim-1]. Their indices are stored inside
		 * the result object.
		 *
		 * Params:
		 *     result = the result object in which the indices of the nearest-neighbors are stored
		 *     vec = the vector for which to search the nearest neighbors
		 *
		 * \tparam RESULTSET Should be any ResultSet<DistanceType>
         * \return  True if the requested neighbors could be found.
		 * \sa knnSearch, radiusSearch
		 */
		template <typename RESULTSET>
		bool findNeighbors(RESULTSET& result, const ElementType* vec, const SearchParams& searchParams) const
		{
			assert(vec);
            if (size() == 0)
                return false;
			if (!root_node)
                throw std::runtime_error("[nanoflann] findNeighbors() called before building the index.");
			float epsError = 1+searchParams.eps;

			distance_vector_t dists; // fixed or variable-sized container (depending on DIM)
			dists.assign((DIM>0 ? DIM : dim) ,0); // Fill it with zeros.
			DistanceType distsq = computeInitialDistances(vec, dists);
			searchLevel(result, vec, root_node, distsq, dists, epsError);  // "count_leaf" parameter removed since was neither used nor returned to the user.
            return result.full();
		}

		/**
		 * Find the "num_closest" nearest neighbors to the \a query_point[0:dim-1]. Their indices are stored inside
		 * the result object.
		 *  \sa radiusSearch, findNeighbors
		 * \note nChecks_IGNORED is ignored but kept for compatibility with the original FLANN interface.
		 */
		inline void knnSearch(const ElementType *query_point, const size_t num_closest, IndexType *out_indices, DistanceType *out_distances_sq, const int /* nChecks_IGNORED */ = 10) const
		{
			nanoflann::KNNResultSet<DistanceType,IndexType> resultSet(num_closest);
			resultSet.init(out_indices, out_distances_sq);
			this->findNeighbors(resultSet, query_point, nanoflann::SearchParams());
		}

		/**
		 * Find all the neighbors to \a query_point[0:dim-1] within a maximum radius.
		 *  The output is given as a vector of pairs, of which the first element is a point index and the second the corresponding distance.
		 *  Previous contents of \a IndicesDists are cleared.
		 *
		 *  If searchParams.sorted==true, the output list is sorted by ascending distances.
		 *
		 *  For a better performance, it is advisable to do a .reserve() on the vector if you have any wild guess about the number of expected matches.
		 *
		 *  \sa knnSearch, findNeighbors, radiusSearchCustomCallback
		 * \return The number of points within the given radius (i.e. indices.size() or dists.size() )
		 */
		size_t radiusSearch(const ElementType *query_point,const DistanceType radius, std::vector<std::pair<IndexType,DistanceType> >& IndicesDists, const SearchParams& searchParams) const 
		{
			RadiusResultSet<DistanceType,IndexType> resultSet(radius,IndicesDists);
			const size_t nFound = radiusSearchCustomCallback(query_point,resultSet,searchParams);
			if (searchParams.sorted)
				std::sort(IndicesDists.begin(),IndicesDists.end(), IndexDist_Sorter() );
			return nFound;
		}

		/** 
		 * Just like radiusSearch() but with a custom callback class for each point found in the radius of the query.
		 * See the source of RadiusResultSet<> as a start point for your own classes.
		 * \sa radiusSearch
		 */
		template <class SEARCH_CALLBACK>
		size_t radiusSearchCustomCallback(const ElementType *query_point,SEARCH_CALLBACK &resultSet, const SearchParams& searchParams = SearchParams() ) const
		{
			this->findNeighbors(resultSet, query_point, searchParams);
			return resultSet.size();
		}

		/** @} */

	private:
		/** Make sure the auxiliary list \a vind has the same size than the current dataset, and re-generate if size has changed. */
		void init_vind()
		{
			// Create a permutable array of indices to the input vectors.
			m_size = dataset.kdtree_get_point_count();
			if (vind.size()!=m_size) vind.resize(m_size);
			for (size_t i = 0; i < m_size; i++) vind[i] = i;
		}

		/// Helper accessor to the dataset points:
		inline ElementType dataset_get(size_t idx, int component) const {
			return dataset.kdtree_get_pt(idx,component);
		}


		void save_tree(FILE* stream, NodePtr tree)
		{
			save_value(stream, *tree);
			if (tree->child1!=NULL) {
				save_tree(stream, tree->child1);
			}
			if (tree->child2!=NULL) {
				save_tree(stream, tree->child2);
			}
		}


		void load_tree(FILE* stream, NodePtr& tree)
		{
			tree = pool.allocate<Node>();
			load_value(stream, *tree);
			if (tree->child1!=NULL) {
				load_tree(stream, tree->child1);
			}
			if (tree->child2!=NULL) {
				load_tree(stream, tree->child2);
			}
		}


		void computeBoundingBox(BoundingBox& bbox)
		{
			bbox.resize((DIM>0 ? DIM : dim));
			if (dataset.kdtree_get_bbox(bbox))
			{
				// Done! It was implemented in derived class
			}
			else
			{
				const size_t N = dataset.kdtree_get_point_count();
				if (!N) throw std::runtime_error("[nanoflann] computeBoundingBox() called but no data points found.");
				for (int i=0; i<(DIM>0 ? DIM : dim); ++i) {
					bbox[i].low =
					bbox[i].high = dataset_get(0,i);
				}
				for (size_t k=1; k<N; ++k) {
					for (int i=0; i<(DIM>0 ? DIM : dim); ++i) {
						if (dataset_get(k,i)<bbox[i].low) bbox[i].low = dataset_get(k,i);
						if (dataset_get(k,i)>bbox[i].high) bbox[i].high = dataset_get(k,i);
					}
				}
			}
		}


		/**
		 * Create a tree node that subdivides the list of vecs from vind[first]
		 * to vind[last].  The routine is called recursively on each sublist.
		 *
		 * @param left index of the first vector
		 * @param right index of the last vector
		 */
		NodePtr divideTree(const IndexType left, const IndexType right, BoundingBox& bbox)
		{
			NodePtr node = pool.allocate<Node>(); // allocate memory

			/* If too few exemplars remain, then make this a leaf node. */
			if ( (right-left) <= m_leaf_max_size) {
				node->child1 = node->child2 = NULL;    /* Mark as leaf node. */
				node->lr.left = left;
				node->lr.right = right;

				// compute bounding-box of leaf points
				for (int i=0; i<(DIM>0 ? DIM : dim); ++i) {
					bbox[i].low = dataset_get(vind[left],i);
					bbox[i].high = dataset_get(vind[left],i);
				}
				for (IndexType k=left+1; k<right; ++k) {
					for (int i=0; i<(DIM>0 ? DIM : dim); ++i) {
						if (bbox[i].low>dataset_get(vind[k],i)) bbox[i].low=dataset_get(vind[k],i);
						if (bbox[i].high<dataset_get(vind[k],i)) bbox[i].high=dataset_get(vind[k],i);
					}
				}
			}
			else {
				IndexType idx;
				int cutfeat;
				DistanceType cutval;
				middleSplit_(&vind[0]+left, right-left, idx, cutfeat, cutval, bbox);

				node->sub.divfeat = cutfeat;

				BoundingBox left_bbox(bbox);
				left_bbox[cutfeat].high = cutval;
				node->child1 = divideTree(left, left+idx, left_bbox);

				BoundingBox right_bbox(bbox);
				right_bbox[cutfeat].low = cutval;
				node->child2 = divideTree(left+idx, right, right_bbox);

				node->sub.divlow = left_bbox[cutfeat].high;
				node->sub.divhigh = right_bbox[cutfeat].low;

				for (int i=0; i<(DIM>0 ? DIM : dim); ++i) {
					bbox[i].low = std::min(left_bbox[i].low, right_bbox[i].low);
					bbox[i].high = std::max(left_bbox[i].high, right_bbox[i].high);
				}
			}

			return node;
		}


		void computeMinMax(IndexType* ind, IndexType count, int element, ElementType& min_elem, ElementType& max_elem)
		{
			min_elem = dataset_get(ind[0],element);
			max_elem = dataset_get(ind[0],element);
			for (IndexType i=1; i<count; ++i) {
				ElementType val = dataset_get(ind[i],element);
				if (val<min_elem) min_elem = val;
				if (val>max_elem) max_elem = val;
			}
		}

		void middleSplit_(IndexType* ind, IndexType count, IndexType& index, int& cutfeat, DistanceType& cutval, const BoundingBox& bbox)
		{
			const DistanceType EPS=static_cast<DistanceType>(0.00001);
			ElementType max_span = bbox[0].high-bbox[0].low;
			for (int i=1; i<(DIM>0 ? DIM : dim); ++i) {
				ElementType span = bbox[i].high-bbox[i].low;
				if (span>max_span) {
					max_span = span;
				}
			}
			ElementType max_spread = -1;
			cutfeat = 0;
			for (int i=0; i<(DIM>0 ? DIM : dim); ++i) {
				ElementType span = bbox[i].high-bbox[i].low;
				if (span>(1-EPS)*max_span) {
					ElementType min_elem, max_elem;
					computeMinMax(ind, count, cutfeat, min_elem, max_elem);
					ElementType spread = max_elem-min_elem;;
					if (spread>max_spread) {
						cutfeat = i;
						max_spread = spread;
					}
				}
			}
			// split in the middle
			DistanceType split_val = (bbox[cutfeat].low+bbox[cutfeat].high)/2;
			ElementType min_elem, max_elem;
			computeMinMax(ind, count, cutfeat, min_elem, max_elem);

			if (split_val<min_elem) cutval = min_elem;
			else if (split_val>max_elem) cutval = max_elem;
			else cutval = split_val;

			IndexType lim1, lim2;
			planeSplit(ind, count, cutfeat, cutval, lim1, lim2);

			if (lim1>count/2) index = lim1;
			else if (lim2<count/2) index = lim2;
			else index = count/2;
		}


		/**
		 *  Subdivide the list of points by a plane perpendicular on axe corresponding
		 *  to the 'cutfeat' dimension at 'cutval' position.
		 *
		 *  On return:
		 *  dataset[ind[0..lim1-1]][cutfeat]<cutval
		 *  dataset[ind[lim1..lim2-1]][cutfeat]==cutval
		 *  dataset[ind[lim2..count]][cutfeat]>cutval
		 */
		void planeSplit(IndexType* ind, const IndexType count, int cutfeat, DistanceType cutval, IndexType& lim1, IndexType& lim2)
		{
			/* Move vector indices for left subtree to front of list. */
			IndexType left = 0;
			IndexType right = count-1;
			for (;; ) {
				while (left<=right && dataset_get(ind[left],cutfeat)<cutval) ++left;
				while (right && left<=right && dataset_get(ind[right],cutfeat)>=cutval) --right;
				if (left>right || !right) break;  // "!right" was added to support unsigned Index types
				std::swap(ind[left], ind[right]);
				++left;
				--right;
			}
			/* If either list is empty, it means that all remaining features
			 * are identical. Split in the middle to maintain a balanced tree.
			 */
			lim1 = left;
			right = count-1;
			for (;; ) {
				while (left<=right && dataset_get(ind[left],cutfeat)<=cutval) ++left;
				while (right && left<=right && dataset_get(ind[right],cutfeat)>cutval) --right;
				if (left>right || !right) break;  // "!right" was added to support unsigned Index types
				std::swap(ind[left], ind[right]);
				++left;
				--right;
			}
			lim2 = left;
		}

		DistanceType computeInitialDistances(const ElementType* vec, distance_vector_t& dists) const
		{
			assert(vec);
			DistanceType distsq = DistanceType();

			for (int i = 0; i < (DIM>0 ? DIM : dim); ++i) {
				if (vec[i] < root_bbox[i].low) {
					dists[i] = distance.accum_dist(vec[i], root_bbox[i].low, i);
					distsq += dists[i];
				}
				if (vec[i] > root_bbox[i].high) {
					dists[i] = distance.accum_dist(vec[i], root_bbox[i].high, i);
					distsq += dists[i];
				}
			}

			return distsq;
		}

		/**
		 * Performs an exact search in the tree starting from a node.
		 * \tparam RESULTSET Should be any ResultSet<DistanceType>
		 */
		template <class RESULTSET>
		void searchLevel(RESULTSET& result_set, const ElementType* vec, const NodePtr node, DistanceType mindistsq,
						 distance_vector_t& dists, const float epsError) const
		{
			/* If this is a leaf node, then do check and return. */
			if ((node->child1 == NULL)&&(node->child2 == NULL)) {
				//count_leaf += (node->lr.right-node->lr.left);  // Removed since was neither used nor returned to the user.
				DistanceType worst_dist = result_set.worstDist();
				for (IndexType i=node->lr.left; i<node->lr.right; ++i) {
					const IndexType index = vind[i];// reorder... : i;
					DistanceType dist = distance(vec, index, (DIM>0 ? DIM : dim));
					if (dist<worst_dist) {
						result_set.addPoint(dist,vind[i]);
					}
				}
				return;
			}

			/* Which child branch should be taken first? */
			int idx = node->sub.divfeat;
			ElementType val = vec[idx];
			DistanceType diff1 = val - node->sub.divlow;
			DistanceType diff2 = val - node->sub.divhigh;

			NodePtr bestChild;
			NodePtr otherChild;
			DistanceType cut_dist;
			if ((diff1+diff2)<0) {
				bestChild = node->child1;
				otherChild = node->child2;
				cut_dist = distance.accum_dist(val, node->sub.divhigh, idx);
			}
			else {
				bestChild = node->child2;
				otherChild = node->child1;
				cut_dist = distance.accum_dist( val, node->sub.divlow, idx);
			}

			/* Call recursively to search next level down. */
			searchLevel(result_set, vec, bestChild, mindistsq, dists, epsError);

			DistanceType dst = dists[idx];
			mindistsq = mindistsq + cut_dist - dst;
			dists[idx] = cut_dist;
			if (mindistsq*epsError<=result_set.worstDist()) {
				searchLevel(result_set, vec, otherChild, mindistsq, dists, epsError);
			}
			dists[idx] = dst;
		}

	public:
		/**  Stores the index in a binary file.
		  *   IMPORTANT NOTE: The set of data points is NOT stored in the file, so when loading the index object it must be constructed associated to the same source of data points used while building it.
		  * See the example: examples/saveload_example.cpp
		  * \sa loadIndex  */
		void saveIndex(FILE* stream)
		{
			save_value(stream, m_size);
			save_value(stream, dim);
			save_value(stream, root_bbox);
			save_value(stream, m_leaf_max_size);
			save_value(stream, vind);
			save_tree(stream, root_node);
		}

		/**  Loads a previous index from a binary file.
		  *   IMPORTANT NOTE: The set of data points is NOT stored in the file, so the index object must be constructed associated to the same source of data points used while building the index.
		  * See the example: examples/saveload_example.cpp
		  * \sa loadIndex  */
		void loadIndex(FILE* stream)
		{
			load_value(stream, m_size);
			load_value(stream, dim);
			load_value(stream, root_bbox);
			load_value(stream, m_leaf_max_size);
			load_value(stream, vind);
			load_tree(stream, root_node);
		}

	};   // class KDTree


	/** An L2-metric KD-tree adaptor for working with data directly stored in an Eigen Matrix, without duplicating the data storage.
	  *  Each row in the matrix represents a point in the state space.
	  *
	  *  Example of usage:
	  * \code
	  * 	Eigen::Matrix<num_t,Dynamic,Dynamic>  mat;
	  * 	// Fill out "mat"...
	  *
	  * 	typedef KDTreeEigenMatrixAdaptor< Eigen::Matrix<num_t,Dynamic,Dynamic> >  my_kd_tree_t;
	  * 	const int max_leaf = 10;
	  * 	my_kd_tree_t   mat_index(dimdim, mat, max_leaf );
	  * 	mat_index.index->buildIndex();
	  * 	mat_index.index->...
	  * \endcode
	  *
	  *  \tparam DIM If set to >0, it specifies a compile-time fixed dimensionality for the points in the data set, allowing more compiler optimizations.
	  *  \tparam Distance The distance metric to use: nanoflann::metric_L1, nanoflann::metric_L2, nanoflann::metric_L2_Simple, etc.
	  */
	template <class MatrixType, int DIM = -1, class Distance = nanoflann::metric_L2>
	struct KDTreeEigenMatrixAdaptor
	{
		typedef KDTreeEigenMatrixAdaptor<MatrixType,DIM,Distance> self_t;
		typedef typename MatrixType::Scalar              num_t;
		typedef typename MatrixType::Index IndexType;
		typedef typename Distance::template traits<num_t,self_t>::distance_t metric_t;
		typedef KDTreeSingleIndexAdaptor< metric_t,self_t,DIM,IndexType>  index_t;

		index_t* index; //! The kd-tree index for the user to call its methods as usual with any other FLANN index.

		/// Constructor: takes a const ref to the matrix object with the data points
		KDTreeEigenMatrixAdaptor(const int dimensionality, const MatrixType &mat, const int leaf_max_size = 10) : m_data_matrix(mat)
		{
			const IndexType dims = mat.cols();
			if (dims!=dimensionality) throw std::runtime_error("Error: 'dimensionality' must match column count in data matrix");
			if (DIM>0 && static_cast<int>(dims)!=DIM)
				throw std::runtime_error("Data set dimensionality does not match the 'DIM' template argument");
			index = new index_t( dims, *this /* adaptor */, nanoflann::KDTreeSingleIndexAdaptorParams(leaf_max_size ) );
			index->buildIndex();
		}
	private:
		/** Hidden copy constructor, to disallow copying this class (Not implemented) */
		KDTreeEigenMatrixAdaptor(const self_t&);
	public:

		~KDTreeEigenMatrixAdaptor() {
			delete index;
		}

		const MatrixType &m_data_matrix;

		/** Query for the \a num_closest closest points to a given point (entered as query_point[0:dim-1]).
		  *  Note that this is a short-cut method for index->findNeighbors().
		  *  The user can also call index->... methods as desired.
		  * \note nChecks_IGNORED is ignored but kept for compatibility with the original FLANN interface.
		  */
		inline void query(const num_t *query_point, const size_t num_closest, IndexType *out_indices, num_t *out_distances_sq, const int /* nChecks_IGNORED */ = 10) const
		{
			nanoflann::KNNResultSet<num_t,IndexType> resultSet(num_closest);
			resultSet.init(out_indices, out_distances_sq);
			index->findNeighbors(resultSet, query_point, nanoflann::SearchParams());
		}

		/** @name Interface expected by KDTreeSingleIndexAdaptor
		  * @{ */

		const self_t & derived() const {
			return *this;
		}
		self_t & derived()       {
			return *this;
		}

		// Must return the number of data points
		inline size_t kdtree_get_point_count() const {
			return m_data_matrix.rows();
		}

		// Returns the L2 distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
		inline num_t kdtree_distance(const num_t *p1, const IndexType idx_p2,IndexType size) const
		{
			num_t s=0;
			for (IndexType i=0; i<size; i++) {
				const num_t d= p1[i]-m_data_matrix.coeff(idx_p2,i);
				s+=d*d;
			}
			return s;
		}

		// Returns the dim'th component of the idx'th point in the class:
		inline num_t kdtree_get_pt(const IndexType idx, int dim) const {
			return m_data_matrix.coeff(idx,IndexType(dim));
		}

		// Optional bounding-box computation: return false to default to a standard bbox computation loop.
		//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
		//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
		template <class BBOX>
		bool kdtree_get_bbox(BBOX& /*bb*/) const {
			return false;
		}

		/** @} */

	}; // end of KDTreeEigenMatrixAdaptor
	/** @} */

/** @} */ // end of grouping
} // end of NS


#endif /* NANOFLANN_HPP_ */

// *** END *** /root/zeno/nanoflann-master/include/nanoflann.hpp ***

// *** START *** Scannerbase.h ***
// Generated by Flexc++ V1.08.00 on Wed, 28 Sep 2016 16:13:25 -0400

#ifndef ScannerBASE_H_INCLUDED
#define ScannerBASE_H_INCLUDED

#include <limits>
#include <iostream>
#include <deque>
#include <string>
#include <vector>
#include <memory>

// $insert baseIncludes
#include <cctype>


class ScannerBase
{
                // idx: rule, value: tail length (NO_INCREMENTS if no tail)
    typedef std::vector<int> VectorInt;

    enum        // RuleFlagsCount Indices, see s_rfc__[]
    {
        RULE = 0,
        FLAGS,
        ACCCOUNT,
    };

    enum
    {
        FINAL = 1,
        INCREMENT = 2,
        COUNT = 4,
        BOL = 8
    };

    enum 
    {
        AT_EOF = -1
    };

protected:
    enum Leave__
    {};

    enum class ActionType__
    {
        CONTINUE,               // transition succeeded, go on
        ECHO_CH,                // echo ch itself (d_matched empty)
        ECHO_FIRST,             // echo d_matched[0], push back the rest
        MATCH,                  // matched a rule
        RETURN,                 // no further continuation, lex returns 0.
    };

    enum class PostEnum__
    {
        END,                    // postCode called when lex__() ends 
        POP,                    // postCode called after switching files
        RETURN,                 // postCode called when lex__() returns
        WIP                     // postCode called when a non-returning rule
                                // was matched
    };

public:
    enum class StartCondition__ {
        // $insert startCondNames
        INITIAL
    };

private:
    struct FinData            // Info about intermediate matched rules while
    {                           // traversing the DFA
        size_t rule;
        size_t matchLen;
        size_t tailCount;
    };

    struct Final
    {
        FinData atBOL;
        FinData notAtBOL;
    };

        // class Input encapsulates all input operations. 
        // Its member get() returns the next input character
// $insert inputInterface
    class Input
    {
        std::deque<unsigned char> d_deque;  // pending input chars
        std::istream *d_in;                 // ptr for easy streamswitching
        size_t d_lineNr;                    // line count

        public:
            Input();
                                       // iStream: dynamically allocated
            Input(std::istream *iStream, size_t lineNr = 1);
            size_t get();                   // the next range
            void reRead(size_t ch);         // push back 'ch' (if < 0x100)
                                            // push back str from idx 'fmIdx'
            void reRead(std::string const &str, size_t fmIdx);
            size_t lineNr() const
            {
                return d_lineNr;
            }
            void close()                    // force closing the stream
            {
                delete d_in;
                d_in = 0;                   // switchStreams also closes
            }

        private:
            size_t next();                  // obtain the next character
    };

protected:

    struct StreamStruct
    {
        std::string pushedName;
        Input pushedInput;
    };

private:

    std::vector<StreamStruct>    d_streamStack;

    std::string     d_filename;             // name of the currently processed
    static size_t   s_istreamNr;            // file. With istreams it receives
                                            // the name "<istream #>", where
                                            // # is the sequence number of the 
                                            // istream (starting at 1)
    StartCondition__  d_startCondition;
    size_t          d_state;
    int             d_nextState;
    std::shared_ptr<std::ostream> d_out;
    bool            d_sawEOF;               // saw EOF: ignore tailCount
    bool            d_atBOL;                // the matched text starts at BOL
    std::vector<size_t> d_tailCount;         
    Final d_final;                          // 1st for BOL rules
                                            
                                            // only used interactively:
    std::istream *d_in;                     // points to the input stream
    std::shared_ptr<std::istringstream> d_line; // holds line fm d_in
    
    Input           d_input;
    std::string     d_matched;              // matched characters
    bool            d_return;               // return after a rule's action 
    bool            d_more = false;         // set to true by more()

protected:
    std::istream   *d_in__;
    int d_token__;                          // returned by lex__



    int     const (*d_dfaBase__)[44];

    static int     const s_dfa__[][44];
    static int     const (*s_dfaBase__[])[44];
    enum: bool { s_interactive__ = false };
    enum: size_t {
        s_rangeOfEOF__           = 41,
        s_finacIdx__             = 42,
        s_nRules__               = 16,
        s_maxSizeofStreamStack__ = 10
    };
    static size_t  const s_ranges__[];
    static size_t  const s_rfc__[][3];

public:
    ScannerBase(ScannerBase const &other)             = delete;
    ScannerBase &operator=(ScannerBase const &rhs)    = delete;

    bool                debug()     const;
    std::string const  &filename()  const;
    std::string const  &matched()   const;

    size_t              length()    const;
    size_t              lineNr()    const;

    void                setDebug(bool onOff);

    void                switchOstream(std::ostream &out);
    void                switchOstream(std::string const &outfilename);


    void                switchStreams(std::istream &in,
                                      std::ostream &out = zeno_cxx_fout->ofs);

    void                switchIstream(std::string const &infilename);
    void                switchStreams(std::string const &infilename,
                                      std::string const &outfilename);


// $insert interactiveDecl

protected:
    ScannerBase(std::istream &in, std::ostream &out);
    ScannerBase(std::string const &infilename, std::string const &outfilename);

    StartCondition__  startCondition() const;   // current start condition
    bool            popStream();
    std::ostream   &out();
    void            begin(StartCondition__ startCondition);
    void            echo() const;
    void            leave(int retValue) const;

//    `accept(n)' returns all but the first `n' characters of the current
// token back to the input stream, where they will be rescanned when the
// scanner looks for the next match.
//  So, it matches n of the characters in the input buffer, and so it accepts
//  n characters, rescanning the rest. 
    void            accept(size_t nChars = 0);      // former: less
    void            redo(size_t nChars = 0);        // rescan the last nChar
                                                    // characters, reducing
                                                    // length() by nChars
    void            more();
    void            push(size_t ch);                // push char to Input
    void            push(std::string const &txt);   // same: chars


    std::vector<StreamStruct> const &streamStack() const;

    void            pushStream(std::istream &curStream);
    void            pushStream(std::string const &curName);


    void            setFilename(std::string const &name);
    void            setMatched(std::string const &text);

    static std::string istreamName__();
        
        // members used by lex__(): they end in __ and should not be used
        // otherwise.

    ActionType__    actionType__(size_t range); // next action
    bool            return__();                 // 'return' from codeblock
    size_t          matched__(size_t ch);       // handles a matched rule
    size_t          getRange__(int ch);         // convert char to range
    size_t          get__();                    // next character
    size_t          state__() const;            // current state 
    void            continue__(int ch);         // handles a transition
    void            echoCh__(size_t ch);        // echoes ch, sets d_atBOL
    void            echoFirst__(size_t ch);     // handles unknown input
    void            inspectRFCs__();            // update d_tailCount
    void            noReturn__();               // d_return to false
    void            print__() const;            // optionally print token
    void            pushFront__(size_t ch);     // return char to Input
    void            reset__();                  // prepare for new cycle
                                                // next input stream:
    void            switchStream__(std::istream &in, size_t lineNr);   

private:
    void p_pushStream(std::string const &name, std::istream *streamPtr);
    void determineMatchedSize(FinData const &final);
    bool atFinalState();
};

inline std::ostream &ScannerBase::out()
{
    return *d_out;
}

inline void ScannerBase::push(size_t ch)
{
    d_input.reRead(ch);
}

inline void ScannerBase::push(std::string const &str)
{
    d_input.reRead(str, 0);
}

inline bool ScannerBase::atFinalState()
{
    return d_final.notAtBOL.rule != std::numeric_limits<size_t>::max() || 
            (d_atBOL && d_final.atBOL.rule != std::numeric_limits<size_t>::max());
}

inline void ScannerBase::setFilename(std::string const &name)
{
    d_filename = name;
}

inline void ScannerBase::setMatched(std::string const &text)
{
    d_matched = text;
}

inline std::string const &ScannerBase::matched() const
{
    return d_matched;
}

inline ScannerBase::StartCondition__ ScannerBase::startCondition() const
{
    return d_startCondition;
}

inline std::string const &ScannerBase::filename() const
{
    return d_filename;
}

inline void ScannerBase::echo() const
{
    *d_out << d_matched;
}

inline size_t ScannerBase::length() const
{
    return d_matched.size();
}

inline void ScannerBase::leave(int retValue) const
{
    throw static_cast<Leave__>(retValue);
}

inline size_t ScannerBase::lineNr() const
{
    return d_input.lineNr();
}

inline void ScannerBase::more()
{
    d_more = true;
}

inline void ScannerBase::begin(StartCondition__ startCondition)
{
    d_dfaBase__ = 
        s_dfaBase__[static_cast<int>(d_startCondition = startCondition)];
}

inline size_t ScannerBase::state__() const
{
    return d_state;
}

inline size_t ScannerBase::get__()
{
    return d_input.get();
}

inline bool ScannerBase::return__()
{
    return d_return;
}

inline void ScannerBase::noReturn__()
{
    d_return = false;
}


#endif //  ScannerBASE_H_INCLUDED





// *** END *** Scannerbase.h ***

// *** START *** Scanner.h ***
// ================================================================
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
// Date:    Wed Feb 19 11:52:22 2014 EDT
//
// Time-stamp: <2014-05-30 16:43:33 dcj>
//
// ================================================================

// Generated by Flexc++ V1.08.00 on Fri, 30 May 2014 16:42:18 -0400

#ifndef Scanner_H_INCLUDED_
#define Scanner_H_INCLUDED_

// $insert baseclass_h
// inlined #include "Scannerbase.h"


// $insert classHead
class Scanner: public ScannerBase
{
    public:
        explicit Scanner(std::istream &in = std::cin,
                                std::ostream &out = zeno_cxx_fout->ofs);

        Scanner(std::string const &infile, std::string const &outfile);
        
        // $insert lexFunctionDecl
        int lex();

    private:
        int lex__();
        int executeAction__(size_t ruleNr);

        void print();
        void preCode();     // re-implement this function for code that must 
                            // be exec'ed before the patternmatching starts

        void postCode(PostEnum__ type);    
                            // re-implement this function for code that must 
                            // be exec'ed after the rules's actions.
};

// $insert scannerConstructors
inline Scanner::Scanner(std::istream &in, std::ostream &out)
:
    ScannerBase(in, out)
{}

inline Scanner::Scanner(std::string const &infile, std::string const &outfile)
:
    ScannerBase(infile, outfile)
{}

// $insert inlineLexFunction
inline int Scanner::lex()
{
    return lex__();
}

inline void Scanner::preCode() 
{
    // optionally replace by your own code
}

inline void Scanner::postCode(PostEnum__ /* type */ ) 
{
    // optionally replace by your own code
}

inline void Scanner::print() 
{
    print__();
}


#endif // Scanner_H_INCLUDED_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** Scanner.h ***

// *** START *** CovarianceMatrix.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Wed Feb 17 09:02:26 2016 EDT
//
// Time-stamp: <2016-08-26 12:06:17 dcj>
//
// ================================================================

#ifndef COVARIANCE_MATRIX_H_
#define COVARIANCE_MATRIX_H_

// ================================================================

#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>

// ================================================================

/// A covariance matrix.  Variables in the matrix are accessed by user-defined
/// ID numbers.
///
template <class T>
class CovarianceMatrix {
public:
  CovarianceMatrix(const CovarianceMatrix<T> & original);
  CovarianceMatrix();

  ~CovarianceMatrix();

  void add(unsigned int idToAdd, T variance);

  void copy(unsigned int sourceId, unsigned int destId);

  void clear(unsigned int idToClear);

  void remove(unsigned int idToRemove);

  void propagate(unsigned int toId, 
		 unsigned int fromIdA, T df_dA);

  void propagate(unsigned int toId,
		 unsigned int fromIdA, T df_dA,
		 unsigned int fromIdB, T df_dB);

  T getCovariance(unsigned int idA, 
		  unsigned int idB) const;

  void setCovariance(unsigned int idA, 
		     unsigned int idB, 
		     T covariance);

  unsigned int getSize() const;

  void print() const;

private:
  unsigned int idToMatrixIndex(unsigned int id) const;

  T getCovarianceFromIndexes(unsigned int indexA, 
			     unsigned int indexB) const;

  void setCovarianceFromIndexes(unsigned int indexA, 
				unsigned int indexB,
				T covariance);

  std::vector<unsigned int> activeIds;

  std::vector<std::vector<T> > matrix;
};

// ================================================================

/// Copy constructor.
///
template <class T>
CovarianceMatrix<T>::
CovarianceMatrix(const CovarianceMatrix<T> & original)
  : activeIds(original.activeIds),
    matrix(original.matrix)
{

}

/// Construct an empty covariance matrix.
///
template <class T>
CovarianceMatrix<T>::
CovarianceMatrix()
  : activeIds(),
    matrix()
{

}

template <class T>
CovarianceMatrix<T>::
~CovarianceMatrix()
{

}

/// Add a new variable with the given ID and variance.  Covariances of this
/// variable default to 0.
///
template <class T>
void
CovarianceMatrix<T>::
add(unsigned int idToAdd, T variance)
{
  activeIds.push_back(idToAdd);

  matrix.emplace_back(activeIds.size(), 0);

  matrix.back().back() = variance;
}

/// Copy the variance and covariances from one variable in the matrix to
/// another.  The covariance bewteen the variables and the variance of the
/// dest variable are set to the variance of the source variable.
///
template <class T>
void
CovarianceMatrix<T>::
copy(unsigned int sourceId, unsigned int destId) 
{
  unsigned int sourceIndex = idToMatrixIndex(sourceId);
  unsigned int destIndex   = idToMatrixIndex(destId);

  T sourceVariance = getCovarianceFromIndexes(sourceIndex, sourceIndex);

  setCovarianceFromIndexes(sourceIndex, destIndex, sourceVariance); 

  for (unsigned int i = 0; i < activeIds.size(); ++i) {
    T sourceCovariance = getCovarianceFromIndexes(i, sourceIndex);

    setCovarianceFromIndexes(i, destIndex, sourceCovariance);
  }
}

/// Set the variance and covariances of the given variable to zero.
///
template <class T>
void
CovarianceMatrix<T>::
clear(unsigned int idToClear)
{
  unsigned int indexToClear = idToMatrixIndex(idToClear);

  for (unsigned int i = 0; i < activeIds.size(); ++i) {
    setCovarianceFromIndexes(i, indexToClear, 0);
  }
}

/// Remove the given variable from the matrix.
///
template <class T>
void
CovarianceMatrix<T>::
remove(unsigned int idToRemove)
{
  unsigned int idIndex = idToMatrixIndex(idToRemove);

  for (unsigned int i = idIndex + 1; i < activeIds.size(); ++i) {
    matrix.at(i).erase(matrix.at(i).begin() + idIndex);
  }

  matrix.erase(matrix.begin() + idIndex);

  activeIds.erase(activeIds.begin() + idIndex);
}

template <class T>
T 
CovarianceMatrix<T>::
getCovariance(unsigned int idA, 
	      unsigned int idB) const
{
  unsigned int indexA = idToMatrixIndex(idA);
  unsigned int indexB = idToMatrixIndex(idB);

  return getCovarianceFromIndexes(indexA, indexB);
}

template <class T>
void 
CovarianceMatrix<T>::
setCovariance(unsigned int idA, 
	      unsigned int idB, 
	      T covariance)
{
  unsigned int indexA = idToMatrixIndex(idA);
  unsigned int indexB = idToMatrixIndex(idB);

  return setCovarianceFromIndexes(indexA, indexB, covariance);
}

template <class T>
unsigned int
CovarianceMatrix<T>::
getSize() const
{
  return activeIds.size();
}

template <class T>
void
CovarianceMatrix<T>::
print() const
{
  for (unsigned int i = 0; i < matrix.size(); ++ i) {
    for (unsigned int j = 0; j <= i; ++ j) {
      zeno_cxx_fout->ofs << matrix[i][j] << " ";
    }

    zeno_cxx_fout->ofs << std::endl;
  } 
}

/// Perform fist-order propagation of covariance from a variable A to a new
/// variable defined as f(A), given the derivative df/dA.
///
template <class T>
void 
CovarianceMatrix<T>::
propagate(unsigned int toId, 
	  unsigned int fromIdA, T df_dA)
{
  unsigned int indexA = idToMatrixIndex(fromIdA);

  unsigned int indexX = idToMatrixIndex(toId);

  for (unsigned int indexY = 0; indexY < matrix.size(); ++ indexY) {
    T covariance = 
      getCovarianceFromIndexes(indexY, indexA) * df_dA;

    setCovarianceFromIndexes(indexX, indexY, covariance);
  }
}

/// Perform fist-order propagation of covariance from variables A and B to a new
/// variable defined as f(A, B), given the partial derivatives df/dA and df/dB.
///
template <class T>
void 
CovarianceMatrix<T>::
propagate(unsigned int toId,
	  unsigned int fromIdA, T df_dA,
	  unsigned int fromIdB, T df_dB)
{
  unsigned int indexA = idToMatrixIndex(fromIdA);
  unsigned int indexB = idToMatrixIndex(fromIdB);

  unsigned int indexX = idToMatrixIndex(toId);

  for (unsigned int indexY = 0; indexY < matrix.size(); ++ indexY) {
    T covariance = 
      getCovarianceFromIndexes(indexY, indexA) * df_dA +
      getCovarianceFromIndexes(indexY, indexB) * df_dB;

    setCovarianceFromIndexes(indexX, indexY, covariance);
  }
}

/// Convert the given variable ID to an internal matrix index.
///
template <class T>
unsigned int 
CovarianceMatrix<T>::
idToMatrixIndex(unsigned int id) const
{
  std::vector<unsigned int>::const_iterator indexIt;

  indexIt = std::lower_bound(activeIds.begin(),
			     activeIds.end(),
			     id);

  unsigned int index = indexIt - activeIds.begin();

  return index;
}

template <class T>
T 
CovarianceMatrix<T>::
getCovarianceFromIndexes(unsigned int indexA, 
			 unsigned int indexB) const
{
  if (indexA < indexB) {
    std::swap(indexA, indexB);
  }

  T covariance = matrix.at(indexA).at(indexB);

  return covariance;
}

template <class T>
void 
CovarianceMatrix<T>::
setCovarianceFromIndexes(unsigned int indexA, 
			 unsigned int indexB,
			 T covariance)
{
  if (indexA < indexB) {
    std::swap(indexA, indexB);
  }

  matrix.at(indexA).at(indexB) = covariance;
}

// ================================================================

#endif  // #ifndef COVARIANCE_MATRIX_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** CovarianceMatrix.h ***

// *** START *** Geometry/Vector3.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Tue Apr 16 12:20:59 2013 EDT
//
// Time-stamp: <2017-03-24 12:54:38 dcj>
//
// ================================================================

#ifndef VECTOR3_H_
#define VECTOR3_H_

// ================================================================

#include <cstdio>
#include <cmath>
#include <cstring>
#include <ostream>
//#include <xmmintrin.h>

/// Represents a 3-dimensional vector.
///
template <class eleT>
class Vector3
{
 public:
  Vector3();
  Vector3(const Vector3<eleT> & b);
  Vector3(eleT newX, eleT newY, eleT newZ);

  eleT getX() const;
  eleT getY() const;
  eleT getZ() const;

  eleT getI() const;
  eleT getJ() const;
  eleT getK() const;

  void getXYZ(eleT & newX, eleT & newY, eleT & newZ) const;

  eleT get(int dimension) const;

  eleT getMin() const;
  eleT getMax() const;

  eleT dot(const Vector3<eleT> & b) const;
  Vector3<eleT> cross(const Vector3<eleT> & b) const;

  eleT getMagnitude() const;
  eleT getMagnitudeSqr() const;

  eleT getL1Magnitude() const;
  eleT getL1MagnitudeSqr() const;

  void setX(eleT newX);
  void setY(eleT newY);
  void setZ(eleT newZ);

  void setI(eleT newI);
  void setJ(eleT newJ);
  void setK(eleT newK);

  void setXYZ(eleT newX, eleT newY, eleT newZ);
  void setXYZ(eleT * newComponents);

  void set(int dimension, eleT value);

  void capBelow(const Vector3<eleT> & minValue);
  void capAbove(const Vector3<eleT> & maxValue);

  void normalize();

  Vector3<eleT> normalized() const;

  void translate(eleT deltaX, eleT deltaY, eleT deltaZ);

  void rotate(eleT cosTheta, eleT sinTheta, const Vector3<eleT> & axis);

  //in [col][row] format
  void leftMatrixMult3x3(eleT matrix[4][4]);

  void sort();

  Vector3<eleT> roundDown() const;
  Vector3<eleT> roundUp() const;

  void add(int dimension, eleT b);
  void mult(int dimension, eleT b);

  template <class otherEleT> Vector3<eleT>
  operator+(const Vector3<otherEleT> & b) const;

  template <class otherEleT> Vector3<eleT>
  operator-(const Vector3<otherEleT> & b) const;

  template <class otherEleT> Vector3<eleT>
  operator*(const Vector3<otherEleT> & b) const;

  template <class otherEleT> Vector3<eleT>
  operator/(const Vector3<otherEleT> & b) const;

  Vector3<eleT> operator+(eleT b) const;
  Vector3<eleT> operator-(eleT b) const;
  Vector3<eleT> operator*(eleT b) const;
  Vector3<eleT> operator/(eleT b) const;

  Vector3<eleT> & operator+=(const Vector3<eleT> & rhs);
  Vector3<eleT> & operator-=(const Vector3<eleT> & rhs);
  Vector3<eleT> & operator*=(const Vector3<eleT> & rhs);
  Vector3<eleT> & operator/=(const Vector3<eleT> & rhs);

  Vector3<eleT> & operator+=(eleT b);
  Vector3<eleT> & operator-=(eleT b);
  Vector3<eleT> & operator*=(eleT b);
  Vector3<eleT> & operator/=(eleT b);

  template <class otherEleT>
  Vector3<eleT> & operator=(const Vector3<otherEleT> & rhs);

 private:
  void swap(eleT & a, eleT & b);
  
  eleT invSqrt(eleT x);

  eleT components[3];
};

template <class eleT>
std::ostream & operator<<(std::ostream & os, const Vector3<eleT> & rhs)
{
  os << "< " 
     << rhs.getX() << " , " << rhs.getY() << " , " << rhs.getZ() 
     << " >";

  return os;
}

template <class eleT>
Vector3<eleT>::Vector3()
{
  setXYZ(0, 0, 0);
}

template <class eleT>
Vector3<eleT>::Vector3(const Vector3<eleT> & b)
{
  setXYZ(b.getX(), b.getY(), b.getZ());
}

template <class eleT>
Vector3<eleT>::Vector3(eleT newX, eleT newY, eleT newZ)
{
  setXYZ(newX, newY, newZ);
}

template <class eleT>
eleT Vector3<eleT>::getX() const
{
  return components[0];
}

template <class eleT>
eleT Vector3<eleT>::getY() const
{
  return components[1];
}

template <class eleT>
eleT Vector3<eleT>::getZ() const
{
  return components[2];
}

template <class eleT>
eleT Vector3<eleT>::getI() const
{
  return getX();
}

template <class eleT>
eleT Vector3<eleT>::getJ() const
{
  return getY();
}

template <class eleT>
eleT Vector3<eleT>::getK() const
{
  return getZ();
}

template <class eleT>
void Vector3<eleT>::getXYZ(eleT & newX, eleT & newY, eleT & newZ) const
{
  newX = getX();
  newY = getY();
  newZ = getZ();
}

template <class eleT>
eleT Vector3<eleT>::get(int dimension) const
{
  return components[dimension];

  // switch (dimension) {
  //   case 0: return getX();
  //   case 1: return getY();
  //   case 2: return getZ();
  //   default:
  //     printf("Error: tried to return invalid dimension %i from vector\n",
  //            dimension);
  //     return 0;
  // }
}

template <class eleT>
eleT Vector3<eleT>::getMin() const
{
  if (get(0) < get(1)) {
    if (get(0) < get(2)) {
      return get(0);
    }
    else {
      return get(2);
    }
  }
  else {
    if (get(1) < get(2)) {
      return get(1);
    }
    else {
      return get(2);
    }
  }
}

template <class eleT>
eleT Vector3<eleT>::getMax() const
{
  if (get(0) > get(1)) {
    if (get(0) > get(2)) {
      return get(0);
    }
    else {
      return get(2);
    }
  }
  else {
    if (get(1) > get(2)) {
      return get(1);
    }
    else {
      return get(2);
    }
  }
}

template <class eleT>
eleT Vector3<eleT>::getMagnitude() const
{
  return sqrt(getX()*getX() + getY()*getY() + getZ()*getZ());
}

template <class eleT>
eleT Vector3<eleT>::getMagnitudeSqr() const
{
  return getX()*getX() + getY()*getY() + getZ()*getZ();
}

template <class eleT>
eleT Vector3<eleT>::getL1Magnitude() const
{
  return fabs(getX()) + fabs(getY()) + fabs(getZ());
}

template <class eleT>
eleT Vector3<eleT>::getL1MagnitudeSqr() const
{
  return pow(fabs(getX()) + fabs(getY()) + fabs(getZ()), 2);
}

template <class eleT>
void Vector3<eleT>::setX(eleT newX)
{
  components[0] = newX;
}

template <class eleT>
void Vector3<eleT>::setY(eleT newY)
{
  components[1] = newY;
}

template <class eleT>
void Vector3<eleT>::setZ(eleT newZ)
{
  components[2] = newZ;
}

template <class eleT>
void Vector3<eleT>::setI(eleT newI)
{
  setX(newI);
}

template <class eleT>
void Vector3<eleT>::setJ(eleT newJ)
{
  setY(newJ);
}

template <class eleT>
void Vector3<eleT>::setK(eleT newK)
{
  setZ(newK);
}

template <class eleT>
void Vector3<eleT>::setXYZ(eleT newX, eleT newY, eleT newZ)
{
  setX(newX);
  setY(newY);
  setZ(newZ);
}

template <class eleT>
void Vector3<eleT>::setXYZ(eleT *newComponents)
{
  memcpy(components, newComponents, 3*sizeof(eleT));
}

template <class eleT>
void Vector3<eleT>::set(int dimension, eleT value)
{
  components[dimension] = value;

  // switch (dimension) {
  //   case 0: setX(value); break;
  //   case 1: setY(value); break;
  //   case 2: setZ(value); break;
  //   default:
  //     printf("Error: tried to set invalid dimension %i in vector\n",
  //            dimension);
  //     break;
  // }
}

template <class eleT>
Vector3<eleT> Vector3<eleT>::roundDown() const
{
  return Vector3<eleT>(floor(getX()), floor(getY()), floor(getZ()));
}

template <class eleT>
Vector3<eleT> Vector3<eleT>::roundUp() const
{
  return Vector3<eleT>(ceil(getX()), ceil(getY()), ceil(getZ()));
}

template <class eleT>
void Vector3<eleT>::add(int dimension, eleT b)
{
  set(dimension, get(dimension) + b);
}

template <class eleT>
void Vector3<eleT>::mult(int dimension, eleT b)
{
  set(dimension, get(dimension) * b);
}

template <class eleT>
template <class otherEleT>
Vector3<eleT> Vector3<eleT>::operator+(const Vector3<otherEleT> & b) const
{
  Vector3<eleT> result(getX() + b.getX(), getY() + b.getY(),
                       getZ() + b.getZ());

  return result;
}

template <class eleT>
template <class otherEleT>
Vector3<eleT> Vector3<eleT>::operator-(const Vector3<otherEleT> & b) const
{
  Vector3<eleT> result(getX() - b.getX(), getY() - b.getY(),
                       getZ() - b.getZ());

  return result;
}

template <class eleT>
template <class otherEleT>
Vector3<eleT> Vector3<eleT>::operator*(const Vector3<otherEleT> & b) const
{
  Vector3<eleT> result(getX() * b.getX(), getY() * b.getY(),
                       getZ() * b.getZ());

  return result;
}

template <class eleT>
template <class otherEleT>
Vector3<eleT> Vector3<eleT>::operator/(const Vector3<otherEleT> & b) const
{
  Vector3<eleT> result(getX() / b.getX(), getY() / b.getY(),
                       getZ() / b.getZ());

  return result;
}

template <class eleT>
Vector3<eleT> Vector3<eleT>::operator+(eleT b) const
{
  Vector3<eleT> result(getX()+b, getY()+b, getZ()+b);

  return result;
}

template <class eleT>
Vector3<eleT> Vector3<eleT>::operator-(eleT b) const
{
  Vector3<eleT> result(getX()-b, getY()-b, getZ()-b);

  return result;
}

template <class eleT>
Vector3<eleT> Vector3<eleT>::operator*(eleT b) const
{
  Vector3<eleT> result(getX()*b, getY()*b, getZ()*b);

  return result;
}

template <class eleT>
Vector3<eleT> Vector3<eleT>::operator/(eleT b) const
{
  Vector3<eleT> result(getX()/b, getY()/b, getZ()/b);

  return result;
}

template <class eleT>
Vector3<eleT> & Vector3<eleT>::operator+=(const Vector3<eleT> & rhs)
{
  setXYZ(getX() + rhs.getX(), getY() + rhs.getY(), getZ() + rhs.getZ());

  return *this;
}

template <class eleT>
Vector3<eleT> & Vector3<eleT>::operator-=(const Vector3<eleT> & rhs)
{
  setXYZ(getX() - rhs.getX(), getY() - rhs.getY(), getZ() - rhs.getZ());

  return *this;
}

template <class eleT>
Vector3<eleT> & Vector3<eleT>::operator*=(const Vector3<eleT> & rhs)
{
  setXYZ(getX() * rhs.getX(), getY() * rhs.getY(), getZ() * rhs.getZ());

  return *this;
}

template <class eleT>
Vector3<eleT> & Vector3<eleT>::operator/=(const Vector3<eleT> & rhs)
{
  setXYZ(getX() / rhs.getX(), getY() / rhs.getY(), getZ() / rhs.getZ());

  return *this;
}

template <class eleT>
Vector3<eleT> & Vector3<eleT>::operator+=(eleT b)
{
  setXYZ(getX()+b, getY()+b, getZ()+b);

  return *this;
}

template <class eleT>
Vector3<eleT> & Vector3<eleT>::operator-=(eleT b)
{
  setXYZ(getX()-b, getY()-b, getZ()-b);

  return *this;
}

template <class eleT>
Vector3<eleT> & Vector3<eleT>::operator*=(eleT b)
{
  setXYZ(getX()*b, getY()*b, getZ()*b);

  return *this;
}

template <class eleT>
Vector3<eleT> & Vector3<eleT>::operator/=(eleT b)
{
  setXYZ(getX()/b, getY()/b, getZ()/b);

  return *this;
}

template <class eleT>
template <class otherEleT>
Vector3<eleT> & Vector3<eleT>::operator=(const Vector3<otherEleT> & rhs)
{
  setXYZ((eleT)rhs.getX(), (eleT)rhs.getY(), (eleT)rhs.getZ());

  return *this;
}

template <class eleT>
void Vector3<eleT>::translate(eleT deltaX, eleT deltaY, eleT deltaZ)
{
  setXYZ(getX() + deltaX, getY() + deltaY, getZ() + deltaZ);
}

template <class eleT>
void Vector3<eleT>::rotate(eleT cosTheta, eleT sinTheta,
			   const Vector3<eleT> & axis)
{
  Vector3<eleT> u(axis);
  u.normalize();

  eleT matrix[4][4];

  matrix[0][0] = cosTheta + pow(u.getX(), 2)*(1 - cosTheta); 
  matrix[0][1] = u.getX()*u.getY()*(1 - cosTheta) - u.getZ()*sinTheta;
  matrix[0][2] = u.getX()*u.getZ()*(1 - cosTheta) + u.getY()*sinTheta;
  matrix[0][3] = 0;

  matrix[1][0] = u.getY()*u.getX()*(1 - cosTheta) + u.getZ()*sinTheta;
  matrix[1][1] = cosTheta + pow(u.getY(), 2)*(1 - cosTheta);
  matrix[1][2] = u.getY()*u.getZ()*(1 - cosTheta) - u.getX()*sinTheta;
  matrix[1][3] = 0;

  matrix[2][0] = u.getZ()*u.getX()*(1 - cosTheta) - u.getY()*sinTheta;
  matrix[2][1] = u.getZ()*u.getY()*(1 - cosTheta) + u.getX()*sinTheta;
  matrix[2][2] = cosTheta + pow(u.getZ(), 2)*(1 - cosTheta);
  matrix[2][3] = 0;

  matrix[3][0] = 0;
  matrix[3][1] = 0;
  matrix[3][2] = 0;
  matrix[3][3] = 1;

  leftMatrixMult3x3(matrix);
}

template <class eleT>
eleT Vector3<eleT>::dot(const Vector3<eleT> & b) const
{
  return (getX()*b.getX() + getY()*b.getY() + getZ()*b.getZ());
}

template <class eleT>
Vector3<eleT> Vector3<eleT>::cross(const Vector3<eleT> & b) const
{
  eleT crossX = getY()*b.getZ() - getZ()*b.getY();
  eleT crossY = getZ()*b.getX() - getX()*b.getZ();
  eleT crossZ = getX()*b.getY() - getY()*b.getX();

  Vector3<eleT> result(crossX, crossY, crossZ);

  return result;
}

template <class eleT>
void Vector3<eleT>::capBelow(const Vector3<eleT> & minValue)
{
  if (getX() < minValue.getX()) setX(minValue.getX());
  if (getY() < minValue.getY()) setY(minValue.getY());
  if (getZ() < minValue.getZ()) setZ(minValue.getZ());
}

template <class eleT>
void Vector3<eleT>::capAbove(const Vector3<eleT> & maxValue)
{
  if (getX() > maxValue.getX()) setX(maxValue.getX());
  if (getY() > maxValue.getY()) setY(maxValue.getY());
  if (getZ() > maxValue.getZ()) setZ(maxValue.getZ());
}

template <class eleT>
void Vector3<eleT>::normalize()
{
  eleT magnitudeSqr = getMagnitudeSqr();

  if (magnitudeSqr == 0)
    return;

  eleT invMagnitude = invSqrt(magnitudeSqr);

  setXYZ(getX()*invMagnitude, getY()*invMagnitude, getZ()*invMagnitude);
}

template <class eleT>
Vector3<eleT> Vector3<eleT>::normalized() const
{
  Vector3<eleT> normalized(*this);
  normalized.normalize();

  return normalized;
}

//in [col][row] format
template <class eleT>
void Vector3<eleT>::leftMatrixMult3x3(eleT matrix[4][4])
{
  eleT _x = getX();
  eleT _y = getY();
  eleT _z = getZ();

  setX(_x * matrix[0][0] + _y * matrix[1][0] + _z * matrix[2][0]);
  setY(_x * matrix[0][1] + _y * matrix[1][1] + _z * matrix[2][1]);
  setZ(_x * matrix[0][2] + _y * matrix[1][2] + _z * matrix[2][2]);
  /*
    setX(_x * matrix[0][0] + _y * matrix[0][1] + _z * matrix[0][2]);
    setY(_x * matrix[1][0] + _y * matrix[1][1] + _z * matrix[1][2]);
    setZ(_x * matrix[2][0] + _y * matrix[2][1] + _z * matrix[2][2]);
  */
}

template <class eleT>
void Vector3<eleT>::sort()
{
  if(components[1]<components[0]) 
    swap(components[0],components[1]);
  
  if(components[2]<components[0]) 
    swap(components[0],components[2]);
  
  if(components[2]<components[1]) 
    swap(components[1],components[2]);
}

template <class eleT>
void Vector3<eleT>::swap(eleT & a, eleT & b)
{
  eleT temp = a;
  a = b;
  b = temp;
}

template <class eleT>
eleT Vector3<eleT>::invSqrt(eleT x)
{
  return 1. / sqrt(x);

  //return _mm_cvtss_si32( _mm_rsqrt_ss( _mm_set_ss( x ) ) );

  //union {
  //	float xFloat;
  //	int xInt;
  //};
  //xFloat = x;
  //xInt = 0x5f3759df - (xInt >> 1); //compute initial guess for Newton iteration
  //x = xFloat;
  //float xhalf = 0.5f*x;
  //x = x * (1.5f - xhalf * x * x); //Newton iteration step
  //x = x * (1.5f - xhalf * x * x);
  //x = x * (1.5f - xhalf * x * x);
  //return x;
}

// ================================================================

#endif  // #ifndef VECTOR3_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** Geometry/Vector3.h ***

// *** START *** Geometry/Matrix3x3.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Tue Apr 16 12:20:59 2013 EDT
//
// Time-stamp: <2017-03-24 18:05:28 dcj>
//
// ================================================================

#ifndef MATRIX3X3_H_
#define MATRIX3X3_H_

// ================================================================

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cassert>
#include <ostream>
#include <algorithm>

// inlined #include "Vector3.h"

/// Represents a 3x3 matrix.
///
template <class eleT>
class Matrix3x3
{
 public:
  Matrix3x3();
  Matrix3x3(const Matrix3x3<eleT> & matrix);
  Matrix3x3(const eleT * matrix);
  Matrix3x3(eleT matrix00, eleT matrix01, eleT matrix02,
	    eleT matrix10, eleT matrix11, eleT matrix12,
	    eleT matrix20, eleT matrix21, eleT matrix22);

  eleT get(int row, int col) const;
  eleT get(int component) const;

  void set(int row, int col, eleT value);
  void set(int component, eleT value);

  void symmetrize();

  void getEigenValues(Vector3<eleT> & eigenValues) const;

  void addRow(int row, const Vector3<eleT> & b);
  void addCol(int col, const Vector3<eleT> & b);

  void multRow(int row, const Vector3<eleT> & b);
  void multCol(int col, const Vector3<eleT> & b);

  template <class otherEleT> Matrix3x3<eleT>
  operator+(const Matrix3x3<otherEleT> & b) const;

  template <class otherEleT> Matrix3x3<eleT>
  operator-(const Matrix3x3<otherEleT> & b) const;

  template <class otherEleT> Matrix3x3<eleT>
  operator*(const Matrix3x3<otherEleT> & b) const;

  template <class otherEleT> Matrix3x3<eleT>
  operator/(const Matrix3x3<otherEleT> & b) const;

  Matrix3x3<eleT> operator+(eleT b) const;
  Matrix3x3<eleT> operator-(eleT b) const;
  Matrix3x3<eleT> operator*(eleT b) const;
  Matrix3x3<eleT> operator/(eleT b) const;

  Matrix3x3<eleT> & operator+=(const Matrix3x3<eleT> & rhs);
  Matrix3x3<eleT> & operator-=(const Matrix3x3<eleT> & rhs);
  Matrix3x3<eleT> & operator*=(const Matrix3x3<eleT> & rhs);
  Matrix3x3<eleT> & operator/=(const Matrix3x3<eleT> & rhs);

  Matrix3x3<eleT> & operator+=(eleT b);
  Matrix3x3<eleT> & operator-=(eleT b);
  Matrix3x3<eleT> & operator*=(eleT b);
  Matrix3x3<eleT> & operator/=(eleT b);

  template <class otherEleT>
  Matrix3x3<eleT> & operator=(const Matrix3x3<otherEleT> & rhs);

 private:
  eleT components[3*3];
};

template <class eleT>
std::ostream & operator<<(std::ostream & os, const Matrix3x3<eleT> & rhs)
{
  os << "[ " 
     << rhs.get(0, 0) << " , " 
     << rhs.get(0, 1) << " , " 
     << rhs.get(0, 2) << " ,"
     << std::endl
     << "  "
     << rhs.get(1, 0) << " , " 
     << rhs.get(1, 1) << " , " 
     << rhs.get(1, 2) << " ,"
     << std::endl
     << "  "
     << rhs.get(2, 0) << " , " 
     << rhs.get(2, 1) << " , " 
     << rhs.get(2, 2) << " ]"
     << std::endl;

  return os;
}

template <class eleT>
Matrix3x3<eleT>::Matrix3x3()
{
  for (int i = 0; i < 3*3; i++) {
    components[i] = 0;
  }
}

template <class eleT>
Matrix3x3<eleT>::Matrix3x3(const Matrix3x3<eleT> & matrix)
{
  for (int i = 0; i < 3*3; i++) {
    components[i] = matrix.components[i];
  }
}

template <class eleT>
Matrix3x3<eleT>::Matrix3x3(const eleT * matrix)
{
  for (int i = 0; i < 3*3; i++) {
    components[i] = matrix[i];
  }
}

template <class eleT>
Matrix3x3<eleT>::Matrix3x3(eleT matrix00, eleT matrix01, eleT matrix02,
			   eleT matrix10, eleT matrix11, eleT matrix12,
			   eleT matrix20, eleT matrix21, eleT matrix22)
{
  set(0, 0, matrix00);
  set(0, 1, matrix01);
  set(0, 2, matrix02);
  set(1, 0, matrix10);
  set(1, 1, matrix11);
  set(1, 2, matrix12);
  set(2, 0, matrix20);
  set(2, 1, matrix21);
  set(2, 2, matrix22);
}

template <class eleT>
eleT Matrix3x3<eleT>::get(int row, int col) const
{
  assert(row >= 0 && row < 3);
  assert(col >= 0 && col < 3);

  return components[row*3 + col];
}

template <class eleT>
eleT Matrix3x3<eleT>::get(int component) const
{
  assert(component >= 0 && component < 9);

  return components[component];
}

template <class eleT>
void Matrix3x3<eleT>::set(int row, int col, eleT value)
{
  assert(row >= 0 && row < 3);
  assert(col >= 0 && col < 3);

  components[row*3 + col] = value;
}

template <class eleT>
void Matrix3x3<eleT>::set(int component, eleT value)
{
  assert(component >= 0 && component < 9);

  components[component] = value;
}

template <class eleT>
void Matrix3x3<eleT>::symmetrize()
{
  for (int row = 0; row < 3; row++) {
    for (int col = row + 1; col < 3; col++) {
      eleT average = (get(row, col) + get(col, row)) / (eleT)2;

      set(row, col, average);
      set(col, row, average);
    }
  }
}

template <class eleT>
void Matrix3x3<eleT>::getEigenValues(Vector3<eleT> & eigenValues) const
{
  // Jacobi
  
  // Joachim Kopp
  // Efficient numerical diagonalization of hermitian 3x3 matrices
  // Int. J. Mod. Phys. C 19 (2008) 523-548
  // arXiv.org: physics/0610206
  
  assert(get(1, 0) == get(0, 1));
  assert(get(2, 0) == get(0, 2));
  assert(get(2, 1) == get(1, 2));

  const int dim = 3;

  const int maxJacobiIter = 1000;

  eleT workEigenVals[3];

  eleT workMatrix[3][3];

  for (int i = 0; i < dim; i++)
    for (int j = 0; j < dim; j++)
      workMatrix[i][j] = get(i, j);
  
  for (int i = 0; i < dim; i++)
    workEigenVals[i] = workMatrix[i][i];

  int numJacobiIter = 0;

  // Do Jacobi iterations
  while ((workMatrix[0][1] != 0.) ||
         (workMatrix[0][2] != 0.) ||
	 (workMatrix[1][2] != 0.)) {

    // Do sweep
    for (int row = 0; row < dim; row++) {
      for (int col = row + 1; col < dim; col++) {
	
	if ((fabs(workEigenVals[row]) + fabs(workMatrix[row][col]) ==
	     fabs(workEigenVals[row])) &&
	    
	    (fabs(workEigenVals[col]) + fabs(workMatrix[row][col]) ==
	     fabs(workEigenVals[col]))) {
	  
	  workMatrix[row][col] = 0.;
	}
	else {  
	  // Calculate Jacobi transformation

	  eleT tan_phi = eleT();
	  
	  eleT h = workEigenVals[col] - workEigenVals[row];
	  
	  if (fabs(h) + fabs(workMatrix[row][col]) ==
	      fabs(h)) {
	    
	    tan_phi = workMatrix[row][col] / h;
	  }
	  else {
	    eleT phi = 0.5 * h / workMatrix[row][col];
	    
	    if (phi < 0.)
	      tan_phi = -1. / (sqrt(1. + (phi*phi)) - phi);
	    else
	      tan_phi = 1. / (sqrt(1. + (phi*phi)) + phi);
	  }
	    
	  eleT cos_phi = 1. / sqrt(1. + (tan_phi*tan_phi));
	  eleT sin_phi = tan_phi * cos_phi;

	  // Apply Jacobi transformation

	  workEigenVals[row] -= tan_phi * workMatrix[row][col];
	  workEigenVals[col] += tan_phi * workMatrix[row][col];
	  
	  workMatrix[row][col] = 0.;
	    
	  for (int i = 0; i < row; i++) {
	    tan_phi = workMatrix[i][row];
	    workMatrix[i][row] = cos_phi*tan_phi - sin_phi*workMatrix[i][col];
	    workMatrix[i][col] = sin_phi*tan_phi + cos_phi*workMatrix[i][col];
	  }
	    
	  for (int i = row + 1; i < col; i++) {
	    tan_phi = workMatrix[row][i];
	    workMatrix[row][i] = cos_phi*tan_phi - sin_phi*workMatrix[i][col];
	    workMatrix[i][col] = sin_phi*tan_phi + cos_phi*workMatrix[i][col];
	  }
	    
	  for (int i = col + 1; i < dim; i++) {
	    tan_phi = workMatrix[row][i];
	    workMatrix[row][i] = cos_phi*tan_phi - sin_phi*workMatrix[col][i];
	    workMatrix[col][i] = sin_phi*tan_phi + cos_phi*workMatrix[col][i];
	  }
	}
      }
    }

    ++ numJacobiIter;

    assert(numJacobiIter < maxJacobiIter);
  }

  eigenValues.setXYZ(workEigenVals[0], workEigenVals[1], workEigenVals[2]);
  eigenValues.sort();
}

template <class eleT>
void Matrix3x3<eleT>::addRow(int row, const Vector3<eleT> & b)
{
  assert(row >= 0 && row < 3);

  for (int col = 0; col < 3; col++) {
    eleT sum = get(row, col) + b.get(col);

    set(row, col, sum);
  }
}

template <class eleT>
void Matrix3x3<eleT>::addCol(int col, const Vector3<eleT> & b)
{
  assert(col >= 0 && col < 3);

  for (int row = 0; row < 3; row++) {
    eleT sum = get(row, col) + b.get(row);

    set(row, col, sum);
  }
}

template <class eleT>
void Matrix3x3<eleT>::multRow(int row, const Vector3<eleT> & b)
{
  assert(row >= 0 && row < 3);

  for (int col = 0; col < 3; col++) {
    eleT product = get(row, col) * b.get(col);

    set(row, col, product);
  }
}

template <class eleT>
void Matrix3x3<eleT>::multCol(int col, const Vector3<eleT> & b)
{
  assert(col >= 0 && col < 3);

  for (int row = 0; row < 3; row++) {
    eleT product = get(row, col) * b.get(row);

    set(row, col, product);
  }
}

template <class eleT>
template <class otherEleT>
Matrix3x3<eleT> Matrix3x3<eleT>::operator+(const Matrix3x3<otherEleT> & b) const
{
  eleT result[3*3];

  for (int i = 0; i < 3*3; i++) {
    result[i] = components[i] + b.components[i];
  }

  return Matrix3x3<eleT>(result);
}

template <class eleT>
template <class otherEleT>
Matrix3x3<eleT> Matrix3x3<eleT>::operator-(const Matrix3x3<otherEleT> & b) const
{
  eleT result[3*3];

  for (int i = 0; i < 3*3; i++) {
    result[i] = components[i] - b.components[i];
  }

  return Matrix3x3<eleT>(result);
}

template <class eleT>
template <class otherEleT>
Matrix3x3<eleT> Matrix3x3<eleT>::operator*(const Matrix3x3<otherEleT> & b) const
{
  eleT result[3*3];

  for (int i = 0; i < 3*3; i++) {
    result[i] = components[i] * b.components[i];
  }

  return Matrix3x3<eleT>(result);
}

template <class eleT>
template <class otherEleT>
Matrix3x3<eleT> Matrix3x3<eleT>::operator/(const Matrix3x3<otherEleT> & b) const
{
  eleT result[3*3];

  for (int i = 0; i < 3*3; i++) {
    result[i] = components[i] / b.components[i];
  }

  return Matrix3x3<eleT>(result);
}

template <class eleT>
Matrix3x3<eleT> Matrix3x3<eleT>::operator+(eleT b) const
{
  eleT result[3*3];

  for (int i = 0; i < 3*3; i++) {
    result[i] = components[i] + b;
  }

  return Matrix3x3<eleT>(result);
}

template <class eleT>
Matrix3x3<eleT> Matrix3x3<eleT>::operator-(eleT b) const
{
  eleT result[3*3];

  for (int i = 0; i < 3*3; i++) {
    result[i] = components[i] - b;
  }

  return Matrix3x3<eleT>(result);
}

template <class eleT>
Matrix3x3<eleT> Matrix3x3<eleT>::operator*(eleT b) const
{
  eleT result[3*3];

  for (int i = 0; i < 3*3; i++) {
    result[i] = components[i] * b;
  }

  return Matrix3x3<eleT>(result);
}

template <class eleT>
Matrix3x3<eleT> Matrix3x3<eleT>::operator/(eleT b) const
{
  eleT result[3*3];

  for (int i = 0; i < 3*3; i++) {
    result[i] = components[i] / b;
  }

  return Matrix3x3<eleT>(result);
}

template <class eleT>
Matrix3x3<eleT> & Matrix3x3<eleT>::operator+=(const Matrix3x3<eleT> & rhs)
{
  for (int i = 0; i < 3*3; i++) {
    components[i] += rhs.components[i];
  }

  return *this;
}

template <class eleT>
Matrix3x3<eleT> & Matrix3x3<eleT>::operator-=(const Matrix3x3<eleT> & rhs)
{
  for (int i = 0; i < 3*3; i++) {
    components[i] -= rhs.components[i];
  }

  return *this;
}

template <class eleT>
Matrix3x3<eleT> & Matrix3x3<eleT>::operator*=(const Matrix3x3<eleT> & rhs)
{
  for (int i = 0; i < 3*3; i++) {
    components[i] *= rhs.components[i];
  }

  return *this;
}

template <class eleT>
Matrix3x3<eleT> & Matrix3x3<eleT>::operator/=(const Matrix3x3<eleT> & rhs)
{
  for (int i = 0; i < 3*3; i++) {
    components[i] /= rhs.components[i];
  }

  return *this;
}

template <class eleT>
Matrix3x3<eleT> & Matrix3x3<eleT>::operator+=(eleT b)
{
  for (int i = 0; i < 3*3; i++) {
    components[i] += b;
  }

  return *this;
}

template <class eleT>
Matrix3x3<eleT> & Matrix3x3<eleT>::operator-=(eleT b)
{
  for (int i = 0; i < 3*3; i++) {
    components[i] -= b;
  }

  return *this;
}

template <class eleT>
Matrix3x3<eleT> & Matrix3x3<eleT>::operator*=(eleT b)
{
  for (int i = 0; i < 3*3; i++) {
    components[i] *= b;
  }

  return *this;
}

template <class eleT>
Matrix3x3<eleT> & Matrix3x3<eleT>::operator/=(eleT b)
{
  for (int i = 0; i < 3*3; i++) {
    components[i] /= b;
  }

  return *this;
}

template <class eleT>
template <class otherEleT>
Matrix3x3<eleT> & Matrix3x3<eleT>::operator=(const Matrix3x3<otherEleT> & rhs)
{
  for (int i = 0; i < 3*3; i++) {
    otherEleT rhsComponent = rhs.get(i);
    set(i, (eleT)rhsComponent);
  }

  return *this;
}

// ================================================================

#endif  // #ifndef MATRIX3X3_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** Geometry/Matrix3x3.h ***

// *** START *** Uncertain.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Mon Nov 16 15:04:44 2015 EDT
//
// Time-stamp: <2017-03-24 15:16:11 dcj>
//
// ================================================================

#ifndef UNCERTAIN_H_
#define UNCERTAIN_H_

// ================================================================

#include <iostream>
#include <cmath>
#include <cassert>

// inlined #include "CovarianceMatrix.h"

// inlined #include "Geometry/Vector3.h"
// inlined #include "Geometry/Matrix3x3.h"

// ================================================================

template <class T>
class Uncertain;

template <class T>
Uncertain<T> fabs(const Uncertain<T> & A); 

template <class T>
Uncertain<T> log(const Uncertain<T> & A); 

template <class T>
Uncertain<T> exp(const Uncertain<T> & A);

template <class T>
Uncertain<T> pow(const Uncertain<T> & A, const Uncertain<T> & B);

template <class T>
Uncertain<T> sqrt(const Uncertain<T> & A);

template <class T>
Uncertain<T> sin(const Uncertain<T> & A);

template <class T>
Uncertain<T> cos(const Uncertain<T> & A);

template <class T>
Uncertain<T> atan2(const Uncertain<T> & A, const Uncertain<T> & B);

// ================================================================

/// Represents a value with uncertainty as a mean and variance.
/// Handles uncertainty propagation through arithmatic operations and
/// tracks covariance between variables.
///
template <class T>
class Uncertain {
public:
  Uncertain(const Uncertain<T> & original);
  Uncertain(T mean = 0, T variance = 0);

  ~Uncertain();

  T getMean() const;
  T getVariance() const;

  T getStdDev() const;

  Uncertain<T> & operator=(const Uncertain<T> & rhs);
  Uncertain<T> & operator=(const T & rhs);

  Uncertain<T> & operator+=(const Uncertain<T> & rhs);
  Uncertain<T> & operator+=(const T & rhs);

  Uncertain<T> & operator-=(const Uncertain<T> & rhs);
  Uncertain<T> & operator-=(const T & rhs);

  Uncertain<T> & operator*=(const Uncertain<T> & rhs);
  Uncertain<T> & operator*=(const T & rhs);

  Uncertain<T> & operator/=(const Uncertain<T> & rhs);
  Uncertain<T> & operator/=(const T & rhs);

  friend Uncertain fabs<T>(const Uncertain & A); 
  friend Uncertain log<T>(const Uncertain & A); 
  friend Uncertain exp<T>(const Uncertain & A);
  friend Uncertain pow<T>(const Uncertain & A, const Uncertain & B);
  friend Uncertain sqrt<T>(const Uncertain & A);
  friend Uncertain sin<T>(const Uncertain & A);
  friend Uncertain cos<T>(const Uncertain & A);
  friend Uncertain atan2<T>(const Uncertain & A, const Uncertain & B);

  static Vector3<Uncertain<T> > zip(Vector3<T> const & mean,
				    Vector3<T> const & variance);

  static Matrix3x3<Uncertain<T> > zip(Matrix3x3<T> const & mean,
				      Matrix3x3<T> const & variance);

private:
  static T firstOrderVariance(T varA, T df_dA);

  static T firstOrderVariance(T varA, T df_dA,
			      T varB, T df_dB);

  static T firstOrderVariance(T varA, T df_dA, 
			      T covAB,
			      T varB, T df_dB);

  static T secondOrderMean(T firstOrderMean, 
			   T varA, T d2f_dA2);

  static T secondOrderMean(T firstOrderMean, 
			   T varA, T d2f_dA2,
			   T varB, T d2f_dB2);

  static T secondOrderMean(T firstOrderMean, 
			   T varA, T d2f_dA2,
			   T covAB, T d2f_dAdB, 
			   T varB, T d2f_dB2);

  static T secondOrderVariance(T firstOrderVariance, 
			       T varA, T d2f_dA2);

  static T secondOrderVariance(T firstOrderVariance, 
			       T varA, T d2f_dA2,
			       T d2f_dAdB,  
			       T varB, T d2f_dB2);

  static T secondOrderVariance(T firstOrderVariance, 
			       T varA, T d2f_dA2,
			       T covAB, T d2f_dAdB, 
			       T varB, T d2f_dB2);

  void updateFirstOrder(unsigned int idA, T varA, T df_dA);

  void updateFirstOrder(unsigned int idA, T varA, T df_dA,
			unsigned int idB, T varB, T df_dB);

  void updateSecondOrder(T varA, T d2f_dA2);

  void updateSecondOrder(T varA, T d2f_dA2,
			 T d2f_dAdB,
			 T varB, T d2f_dB2);

  static const bool propagateUncertainty = true;
  static const bool useCovariance        = true;
  static const bool useSecondOrder       = false;

  static const bool debug = false;

  T mean;
  T variance;

  unsigned int id;

  static unsigned int nextId;

  static CovarianceMatrix<T> covarianceMatrix;
};

// ================================================================

template <class T>
unsigned int Uncertain<T>::nextId = 0;

template <class T>
CovarianceMatrix<T> Uncertain<T>::covarianceMatrix;

// ================================================================

/// Copy constructor.
///
template <class T>
Uncertain<T>::
Uncertain(const Uncertain<T> & original) 
  : mean(original.mean),
    variance(original.variance),
    id()
{
  if (useCovariance) {
    id = nextId ++;

    covarianceMatrix.add(id, 0);
    covarianceMatrix.copy(original.id, id);
  }

  if (debug) {
    zeno_cxx_fout->ofs << "Construct new " << id 
	      << " (" << covarianceMatrix.getSize() << ")" << std::endl;

    covarianceMatrix.print();
  }
}

/// Constructs a new Uncertain value with the given mean and variance.
///
template <class T>
Uncertain<T>::
Uncertain(T mean, T variance)
  : mean(mean),
    variance(variance),
    id() 
{
  if (useCovariance) {
    id = nextId ++;

    covarianceMatrix.add(id, variance);
  }

  if (debug) {
    zeno_cxx_fout->ofs << "Construct copy " << id
	      << " (" << covarianceMatrix.getSize() << ")" << std::endl;

    covarianceMatrix.print();
  }
}

template <class T>
Uncertain<T>::
~Uncertain() 
{
  if (useCovariance) {
    covarianceMatrix.remove(id);
  }

  if (debug) {
    zeno_cxx_fout->ofs << "Destruct " << id
	      << " (" << covarianceMatrix.getSize() << ")" << std::endl;

    covarianceMatrix.print();
  }
}

template <class T>
T
Uncertain<T>::
getMean() const 
{
  return mean;
}


template <class T>
T
Uncertain<T>::
getVariance() const 
{
  return variance;
}

template <class T>
T
Uncertain<T>::
getStdDev() const 
{
  return sqrt(variance);
}

template <class T>
Uncertain<T> & 
Uncertain<T>::
operator=(const Uncertain<T> & rhs)
{
  mean     = rhs.mean;
  variance = rhs.variance;

  if (useCovariance) {
    covarianceMatrix.copy(rhs.id, id);
  }

  if (debug) {
    zeno_cxx_fout->ofs << id << " = " << rhs.id << std::endl;

    covarianceMatrix.print();
  }

  return *this;
}

template <class T>
Uncertain<T> & 
Uncertain<T>::
operator=(const T & rhs)
{
  mean     = rhs;
  variance = 0;

  if (useCovariance) {
    covarianceMatrix.clear(id);
  }

  return *this;
}

template <class T>
Uncertain<T> & 
Uncertain<T>::
operator+=(const Uncertain<T> & rhs)
{
  const T meanA = this->mean;
  const T meanB = rhs.mean;

  const T varA = this->variance;
  const T varB = rhs.variance;

  Uncertain<T> result;

  result.mean = meanA + meanB;

  if (propagateUncertainty) {
    if (useCovariance) {
      covarianceMatrix.propagate(result.id,
				 this->id, 1,
				 rhs.id, 1);

      result.variance = covarianceMatrix.getCovariance(result.id, 
						       result.id);
    }
    else {
      result.variance = varA + varB;
    }
  }

  *this = result;

  return *this;
}

template <class T>
Uncertain<T> & 
Uncertain<T>::
operator+=(const T & rhs)
{
  *this += Uncertain<T>(rhs);

  return *this;
}

template <class T>
Uncertain<T> & 
Uncertain<T>::
operator-=(const Uncertain<T> & rhs)
{
  const T meanA = this->mean;
  const T meanB = rhs.mean;

  const T varA = this->variance;
  const T varB = rhs.variance;

  Uncertain<T> result;

  result.mean = meanA - meanB;

  if (propagateUncertainty) {
    if (useCovariance) {
      covarianceMatrix.propagate(result.id,
				 this->id, 1,
				 rhs.id, -1);

      result.variance = covarianceMatrix.getCovariance(result.id, 
						       result.id);
    }
    else {
      result.variance = varA + varB;
    }
  }

  *this = result;

  return *this;
}

template <class T>
Uncertain<T> & 
Uncertain<T>::
operator-=(const T & rhs)
{
  *this -= Uncertain<T>(rhs);

  return *this;
}

template <class T>
Uncertain<T> & 
Uncertain<T>::
operator*=(const Uncertain<T> & rhs)
{
  const T meanA = this->mean;
  const T meanB = rhs.mean;

  const T varA = this->variance;
  const T varB = rhs.variance;

  Uncertain<T> result;

  result.mean = meanA * meanB;

  if (propagateUncertainty) {
    T df_dA = meanB;
    T df_dB = meanA;

    if (useCovariance) {
      covarianceMatrix.propagate(result.id,
				 this->id, df_dA,
				 rhs.id, df_dB);

      result.variance = covarianceMatrix.getCovariance(result.id, 
						       result.id);
    }
    else {
      result.variance = 
	pow(meanA, 2) * varB +
	pow(meanB, 2) * varA +
	varA * varB;
    }
  }

  *this = result;

  return *this;
}

template <class T>
Uncertain<T> & 
Uncertain<T>::
operator*=(const T & rhs)
{
  *this *= Uncertain<T>(rhs);

  return *this;
}

template <class T>
Uncertain<T> & 
Uncertain<T>::
operator/=(const Uncertain<T> & rhs)
{
  const T meanA = this->mean;
  const T meanB = rhs.mean;

  const T varA = this->variance;
  const T varB = rhs.variance;

  Uncertain<T> result;

  result.mean = meanA / meanB;

  if (propagateUncertainty) {
    T df_dA = pow(meanB, -1);
    T df_dB = meanA * -1 * pow(meanB, -2);

    result.updateFirstOrder(this->id, varA, df_dA,
			    rhs.id, varB, df_dB);

    if (useSecondOrder) {
      T d2f_dA2  = 0;
      T d2f_dB2  = meanA * 2 * pow(meanB, -3);
      T d2f_dAdB = -1 * pow(meanB, -2);

      result.updateSecondOrder(varA, d2f_dA2,
			       d2f_dAdB,
			       varB, d2f_dB2);
    }
  }

  *this = result;

  return *this;
}

template <class T>
Uncertain<T> & 
Uncertain<T>::
operator/=(const T & rhs)
{
  *this /= Uncertain<T>(rhs);

  return *this;
}

/// Combines a Vector3 of means and a Vector3 of variances into a Vector3
/// of Uncertain values, each with a mean and variance.
///
template <class T>
Vector3<Uncertain<T> >
Uncertain<T>:: 
zip(Vector3<T> const & mean,
    Vector3<T> const & variance)
{
  Vector3<Uncertain<T> > zipped;

  for (int dim = 0; dim < 3; dim++) {
    zipped.set(dim, Uncertain<T>(mean.get(dim),
				 variance.get(dim)));
  }

  return zipped;
}

/// Combines a Matrix3x3 of means and a Matrix3x3 of variances into a Matrix3x3
/// of Uncertain values, each with a mean and variance.
///
template <class T>
Matrix3x3<Uncertain<T> > 
Uncertain<T>::
zip(Matrix3x3<T> const & mean,
    Matrix3x3<T> const & variance)
{
  Matrix3x3<Uncertain<T> > zipped;

  for (int component = 0; component < 3*3; component++) {
    zipped.set(component, Uncertain<T>(mean.get(component),
				       variance.get(component)));
  }

  return zipped;
}

/// Performs first-order propagation of variance for the function
/// x = f(A)
/// given the variance of A and the derivative df/dA.
///
template <class T>
T 
Uncertain<T>::
firstOrderVariance(T varA, T df_dA)
{
  T varX = varA * pow(df_dA, 2);

  return varX;
}

/// Performs first-order propagation of variance for the function
/// x = f(A, B)
/// given the variances of A and B and the partial derivatives df/dA and df/dB.
/// Assumes the covariance of A and B is 0.
///
template <class T>
T 
Uncertain<T>::
firstOrderVariance(T varA, T df_dA, 
		   T varB, T df_dB)
{
  T varX = 
    varA * pow(df_dA, 2) +
    varB * pow(df_dB, 2);

  return varX;
}

/// Performs first-order propagation of variance for the function
/// x = f(A, B)
/// given the variances of A and B, the partial derivatives df/dA and df/dB,
/// and the covariance of A and B.
///
template <class T>
T 
Uncertain<T>::
firstOrderVariance(T varA, T df_dA, 
		   T covAB,
		   T varB, T df_dB)
{
  T varX = 
    varA * pow(df_dA, 2) +
    2 * covAB * df_dA * df_dB +
    varB * pow(df_dB, 2);

  return varX;
}

/// Performs second-order propagation of mean for the function
/// x = f(A)
/// given the variance of A, the second derivative d^2f/dA^2,
/// and the first-order mean of x.
///
template <class T>
T Uncertain<T>::secondOrderMean(T firstOrderMean, 
				T varA, T d2f_dA2)
{
  T meanX = firstOrderMean + 0.5 * varA * d2f_dA2;

  return meanX;
}

/// Performs second-order propagation of mean for the function
/// x = f(A, B)
/// given the variances of A and B, the second partial derivatives 
/// d^2f/dA^2 and d^2f/dB^2,
/// and the first-order mean of x.
/// Assumes the covariance of A and B is 0.
///
template <class T>
T 
Uncertain<T>::
secondOrderMean(T firstOrderMean, 
		T varA, T d2f_dA2, 
		T varB, T d2f_dB2)
{
  T meanX = 
    firstOrderMean +
    0.5 * varA * d2f_dA2 +
    0.5 * varB * d2f_dB2;

  return meanX;
}

/// Performs second-order propagation of mean for the function
/// x = f(A, B)
/// given the variances of A and B, the second partial derivatives 
/// d^2f/dA^2, d^2f/dB^2 and d^2f/dAdB,
/// the covariance of A and B,
/// and the first-order mean of x.
///
template <class T>
T 
Uncertain<T>::
secondOrderMean(T firstOrderMean, 
		T varA, T d2f_dA2,
		T covAB, T d2f_dAdB, 
		T varB, T d2f_dB2)
{
  T meanX = 
    firstOrderMean +
    0.5 * varA * d2f_dA2 +
    covAB * d2f_dAdB +
    0.5 * varB * d2f_dB2;

  return meanX;
}

/// Performs second-order propagation of variance for the function
/// x = f(A)
/// given the variance of A, the second derivative d^2f/dA^2,
/// and the first-order variance of x.
///
template <class T>
T 
Uncertain<T>::
secondOrderVariance(T firstOrderVariance, 
		    T varA, T d2f_dA2)
{
  T varX = firstOrderVariance + 0.5 * pow(varA, 2) * pow(d2f_dA2, 2);

  return varX;
}

/// Performs second-order propagation of variance for the function
/// x = f(A, B)
/// given the variances of A and B, the second partial derivatives 
/// d^2f/dA^2, d^2f/dB^2, and d^2f/dAdB,
/// and the first-order variance of x.
/// Assumes the covariance of A and B is 0.
///
template <class T>
T 
Uncertain<T>::
secondOrderVariance(T firstOrderVariance, 
		    T varA, T d2f_dA2, 
		    T d2f_dAdB, 
		    T varB, T d2f_dB2)
{
  T varX =
    firstOrderVariance +
    0.5 * pow(varA, 2) * pow(d2f_dA2, 2) +
    (varA * varB) * pow(d2f_dAdB, 2) +
    0.5 * pow(varB, 2) * pow(d2f_dB2, 2);

  return varX;
}

/// Performs second-order propagation of variance for the function
/// x = f(A, B)
/// given the variances of A and B, the second partial derivatives 
/// d^2f/dA^2, d^2f/dB^2 and d^2f/dAdB,
/// the covariance of A and B,
/// and the first-order variance of x.
///
template <class T>
T 
Uncertain<T>::
secondOrderVariance(T firstOrderVariance, 
		    T varA, T d2f_dA2,
		    T covAB, T d2f_dAdB, 
		    T varB, T d2f_dB2)
{
  T varX =
    firstOrderVariance +
    0.5 * pow(varA, 2) * pow(d2f_dA2, 2) +
    2 * varA * covAB * d2f_dA2 * d2f_dAdB +
    pow(covAB, 2) * d2f_dA2 * d2f_dB2 +
    (varA * varB + pow(covAB, 2)) * pow(d2f_dAdB, 2) +
    2 * covAB * varB * d2f_dB2 * d2f_dAdB +
    0.5 * pow(varB, 2) * pow(d2f_dB2, 2);

  return varX;
}

template <class T>
void
Uncertain<T>::
updateFirstOrder(unsigned int idA, T varA, T df_dA)
{
  if (useCovariance) {
    covarianceMatrix.propagate(this->id,
			       idA, df_dA);

    this->variance = covarianceMatrix.getCovariance(this->id, 
						    this->id);
  }
  else {
    this->variance = firstOrderVariance(varA, df_dA);
  }
}

template <class T>
void
Uncertain<T>::
updateFirstOrder(unsigned int idA, T varA, T df_dA,
		 unsigned int idB, T varB, T df_dB)
{
  if (useCovariance) {
    covarianceMatrix.propagate(this->id,
			       idA, df_dA,
			       idB, df_dB);

    this->variance = covarianceMatrix.getCovariance(this->id, 
						    this->id);
  }
  else {
    this->variance = firstOrderVariance(varA, df_dA, 
					varB, df_dB);
  }
}

template <class T>
void
Uncertain<T>::
updateSecondOrder(T varA, T d2f_dA2)
{
  this->mean = secondOrderMean(this->mean, 
			       varA, d2f_dA2);
      
  this->variance = secondOrderVariance(this->variance, 
				       varA, d2f_dA2);
}

template <class T>
void
Uncertain<T>::
updateSecondOrder(T varA, T d2f_dA2,
		  T d2f_dAdB,
		  T varB, T d2f_dB2)
{
  this->mean = secondOrderMean(this->mean, 
			       varA, d2f_dA2,
			       varB, d2f_dB2);
      
  this->variance = secondOrderVariance(this->variance, 
				       varA, d2f_dA2,
				       d2f_dAdB, 
				       varB, d2f_dB2);
}

// ================================================================

template <class T>
Uncertain<T> fabs(const Uncertain<T> & A) 
{
  const T meanA = A.mean;
  const T varA  = A.variance;

  Uncertain<T> result;

  result.mean     = fabs(meanA);
  result.variance = 0;

  if (Uncertain<T>::propagateUncertainty) {
    T df_dA = (A.mean < 0) ? -1 : 1;

    result.updateFirstOrder(A.id, varA, df_dA);

    if (Uncertain<T>::useSecondOrder) {
      T d2f_dA2 = 0;

      result.updateSecondOrder(varA, d2f_dA2);
    }
  }

  return result;
}

template <class T>
Uncertain<T> log(const Uncertain<T> & A) 
{
  const T meanA = A.mean;
  const T varA  = A.variance;

  Uncertain<T> result;

  result.mean     = log(meanA);
  result.variance = 0;

  if (Uncertain<T>::propagateUncertainty) {
    T df_dA = 1 / meanA;

    result.updateFirstOrder(A.id, varA, df_dA);

    if (Uncertain<T>::useSecondOrder) {
      T d2f_dA2 = -pow(meanA, -2);

      result.updateSecondOrder(varA, d2f_dA2);
    }
  }

  return result;
}

template <class T>
Uncertain<T> exp(const Uncertain<T> & A) 
{
  const T meanA = A.mean;
  const T varA  = A.variance;

  Uncertain<T> result;

  result.mean     = exp(meanA);
  result.variance = 0;

  if (Uncertain<T>::propagateUncertainty) {
    T df_dA = exp(meanA);

    result.updateFirstOrder(A.id, varA, df_dA);

    if (Uncertain<T>::useSecondOrder) {
      T d2f_dA2 = exp(meanA);

      result.updateSecondOrder(varA, d2f_dA2);
    }
  }

  return result;
}

template <class T>
Uncertain<T> pow(const Uncertain<T> & A, const Uncertain<T> & B)
{
  const T meanA = A.mean;
  const T meanB = B.mean;

  const T varA = A.variance;
  const T varB = B.variance;

  //handle negative bases with integer exponents
  if (meanA < 0) {
    assert((int)meanB == meanB);

    if ((int)meanB % 2 == 0) {
      return pow(A * -1., B);
    }
    else {
      return pow(A * -1., B) * -1.;
    }
  }
  else {
    Uncertain<T> result;

    result.mean      = pow(meanA, meanB);
    result.variance  = 0;

    if (Uncertain<T>::propagateUncertainty) {
      T df_dA = meanB * pow(meanA, meanB - 1);
      T df_dB = log(meanA) * pow(meanA, meanB);

      result.updateFirstOrder(A.id, varA, df_dA,
			      B.id, varB, df_dB);

      if (Uncertain<T>::useSecondOrder) {
	T d2f_dA2  = meanB * (meanB - 1) * pow(meanA, meanB - 2);
	T d2f_dB2  = pow(log(meanA), 2) * pow(meanA, meanB);
	T d2f_dAdB = 
	  pow(meanA, meanB - 1) + meanB * log(meanA) * pow(meanA, meanB - 1);

	result.updateSecondOrder(varA, d2f_dA2,
				 d2f_dAdB,
				 varB, d2f_dB2);
      }
    }

    return result;
  }
}

template <class T>
Uncertain<T> pow(const Uncertain<T> & A, T b)
{
  return pow(A, Uncertain<T>(b));
}

template <class T>
Uncertain<T> pow(T a, const Uncertain<T> & B)
{
  return pow(Uncertain<T>(a), B);
}

template <class T>
Uncertain<T> sqrt(const Uncertain<T> & A) 
{
  const T meanA = A.mean;
  const T varA  = A.variance;

  Uncertain<T> result;

  result.mean     = sqrt(meanA);
  result.variance = 0;

  //can only take sqrt with mean == 0 if variance == 0, since otherwise 
  //sometimes the distribution would be negative
  if (meanA == 0) {
    assert(varA == 0);
  }
  else {
    if (Uncertain<T>::propagateUncertainty) {
      T df_dA = 0.5 * pow(meanA, -0.5);

      result.updateFirstOrder(A.id, varA, df_dA);

      if (Uncertain<T>::useSecondOrder) {
	T d2f_dA2 = -0.25 * pow(meanA, -1.5);

	result.updateSecondOrder(varA, d2f_dA2);
      }
    }
  }

  return result;
}

template <class T>
Uncertain<T> sin(const Uncertain<T> & A) 
{
  const T meanA = A.mean;
  const T varA  = A.variance;

  Uncertain<T> result;

  result.mean     = sin(meanA);
  result.variance = 0;

  if (Uncertain<T>::propagateUncertainty) {
    T df_dA = cos(meanA);

    result.updateFirstOrder(A.id, varA, df_dA);

    if (Uncertain<T>::useSecondOrder) {
      T d2f_dA2 = -sin(meanA);

      result.updateSecondOrder(varA, d2f_dA2);
    }
  }

  return result;
}

template <class T>
Uncertain<T> cos(const Uncertain<T> & A) 
{
  const T meanA = A.mean;
  const T varA  = A.variance;

  Uncertain<T> result;

  result.mean     = cos(meanA);
  result.variance = 0;

  if (Uncertain<T>::propagateUncertainty) {
    T df_dA = -sin(meanA);

    result.updateFirstOrder(A.id, varA, df_dA);

    if (Uncertain<T>::useSecondOrder) {
      T d2f_dA2 = -cos(meanA);

      result.updateSecondOrder(varA, d2f_dA2);
    }
  }

  return result;
}

template <class T>
Uncertain<T> atan2(const Uncertain<T> & A, const Uncertain<T> & B)
{
  const T meanA = A.mean;
  const T meanB = B.mean;

  const T varA = A.variance;
  const T varB = B.variance;

  Uncertain<T> result;

  result.mean      = atan2(meanA, meanB);
  result.variance  = 0;

  if (Uncertain<T>::propagateUncertainty) {
    const T B2A2 = pow(meanB, 2) + pow(meanA, 2);

    T df_dA = meanB / B2A2;
    T df_dB = -meanA / B2A2;

    result.updateFirstOrder(A.id, varA, df_dA,
			    B.id, varB, df_dB);

    if (Uncertain<T>::useSecondOrder) {
      T d2f_dA2  = meanB * -1 * pow(B2A2, -2) * 2 * meanA;
      T d2f_dB2  = -meanA * -1 * pow(B2A2, -2) * 2 * meanB;
      T d2f_dAdB = pow(B2A2, -1) + meanB * -1 * pow(B2A2, -2) * 2 * meanB;

      result.updateSecondOrder(varA, d2f_dA2,
			       d2f_dAdB,
			       varB, d2f_dB2);
    }
  }

  return result;
}

template <class T>
Uncertain<T> atan2(const Uncertain<T> & A, T b)
{
  return atan2(A, Uncertain<T>(b));
}

template <class T>
Uncertain<T> atan2(T a, const Uncertain<T> & B)
{
  return atan2(Uncertain<T>(a), B);
}

// ================================================================

template <class T>
std::ostream & operator<<(std::ostream & os, const Uncertain<T> & rhs)
{
  os << rhs.getMean() << " +/- " << rhs.getStdDev();

  return os;
}

template <class T>
bool operator==(const Uncertain<T> & A, const Uncertain<T> & B)
{
  return A.getMean() == B.getMean();
}

template <class T>
bool operator==(const Uncertain<T> & A, T b)
{
  return A == Uncertain<T>(b);
}

template <class T>
bool operator==(T a, const Uncertain<T> & B)
{
  return Uncertain<T>(a) == B;
}

template <class T>
bool operator!=(const Uncertain<T> & A, const Uncertain<T> & B)
{
  return !(A == B);
}

template <class T>
bool operator!=(const Uncertain<T> & A, T b)
{
  return A != Uncertain<T>(b);
}

template <class T>
bool operator!=(T a, const Uncertain<T> & B)
{
  return Uncertain<T>(a) != B;
}

template <class T>
bool operator<(const Uncertain<T> & A, const Uncertain<T> & B)
{
  return A.getMean() < B.getMean();
}

template <class T>
bool operator<(const Uncertain<T> & A, T b)
{
  return A < Uncertain<T>(b);
}

template <class T>
bool operator<(T a, const Uncertain<T> & B)
{
  return Uncertain<T>(a) < B;
}

template <class T>
bool operator>(const Uncertain<T> & A, const Uncertain<T> & B)
{
  return B < A;
}

template <class T>
bool operator>(const Uncertain<T> & A, T b)
{
  return A > Uncertain<T>(b);
}

template <class T>
bool operator>(T a, const Uncertain<T> & B)
{
  return Uncertain<T>(a) > B;
}

template <class T>
bool operator<=(const Uncertain<T> & A, const Uncertain<T> & B)
{
  return !(A > B);
}

template <class T>
bool operator<=(const Uncertain<T> & A, T b)
{
  return A <= Uncertain<T>(b);
}

template <class T>
bool operator<=(T a, const Uncertain<T> & B)
{
  return Uncertain<T>(a) <= B;
}

template <class T>
bool operator>=(const Uncertain<T> & A, const Uncertain<T> & B)
{
  return !(A < B);
}

template <class T>
bool operator>=(const Uncertain<T> & A, T b)
{
  return A >= Uncertain<T>(b);
}

template <class T>
bool operator>=(T a, const Uncertain<T> & B)
{
  return Uncertain<T>(a) >= B;
}

template <class T>
inline Uncertain<T> operator+(Uncertain<T> lhs, const Uncertain<T> & rhs)
{
  lhs += rhs;
  return lhs;
}

template <class T>
inline Uncertain<T> operator+(Uncertain<T> lhs, const T & rhs) 
{
  lhs += rhs;
  return lhs;
}

template <class T>
inline Uncertain<T> operator+(const T & lhs, const Uncertain<T> & rhs)
{
  Uncertain<T> uncertainLHS(lhs);

  uncertainLHS += rhs;

  return uncertainLHS;
}

template <class T>
inline Uncertain<T> operator-(Uncertain<T> lhs, const Uncertain<T> & rhs)
{
  lhs -= rhs;
  return lhs;
}

template <class T>
inline Uncertain<T> operator-(Uncertain<T> lhs, const T & rhs)
{
  lhs -= rhs;
  return lhs;
}

template <class T>
inline Uncertain<T> operator-(const T & lhs, const Uncertain<T> & rhs)
{
  Uncertain<T> uncertainLHS(lhs);

  uncertainLHS -= rhs;

  return uncertainLHS;
}

template <class T>
inline Uncertain<T> operator*(Uncertain<T> lhs, const Uncertain<T> & rhs)
{
  lhs *= rhs;
  return lhs;
}

template <class T>
inline Uncertain<T> operator*(Uncertain<T> lhs, const T & rhs)
{
  lhs *= rhs;
  return lhs;
}

template <class T>
inline Uncertain<T> operator*(const T & lhs, const Uncertain<T> & rhs)
{
  Uncertain<T> uncertainLHS(lhs);

  uncertainLHS *= rhs;

  return uncertainLHS;
}

template <class T>
inline Uncertain<T> operator/(Uncertain<T> lhs, const Uncertain<T> & rhs)
{
  lhs /= rhs;
  return lhs;
}

template <class T>
inline Uncertain<T> operator/(Uncertain<T> lhs, const T & rhs)
{
  lhs /= rhs;
  return lhs;
}

template <class T>
inline Uncertain<T> operator/(const T & lhs, const Uncertain<T> & rhs)
{
  Uncertain<T> uncertainLHS(lhs);

  uncertainLHS /= rhs;

  return uncertainLHS;
}

// ================================================================

#endif  // #ifndef UNCERTAIN_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** Uncertain.h ***

// *** START *** Units.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Tue Jan 12 17:05:37 2016 EDT
//
// Time-stamp: <2016-08-30 16:13:19 dcj>
//
// ================================================================

#ifndef UNITS_H_
#define UNITS_H_

// ================================================================

#include <string>

// inlined #include "Uncertain.h"

// ================================================================

/// Contains symbolic constants representing units.  Also contains functions 
/// for getting conversion factors and offsets and human-readable names.
///
class Units
{
public:
  enum class Length {m, cm, nm, A, L};
  enum class Temperature {C, K};
  enum class Mass {Da, kDa, g, kg};
  enum class Viscosity {p, cp};

  static Uncertain<double> getFactor(Length fromUnit, Length toUnit);
  static Uncertain<double> getFactor(Mass fromUnit, Mass toUnit);
  static Uncertain<double> getFactor(Viscosity fromUnit, Viscosity toUnit);

  static Uncertain<double> getOffset(Temperature fromUnit, Temperature toUnit);

  static std::string getName(Length unit);
  static std::string getName(Temperature unit);
  static std::string getName(Mass unit);
  static std::string getName(Viscosity unit);

  static Uncertain<double> kB();

private:
  Units() {}
};

// ================================================================

#endif  // #ifndef UNITS_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** Units.h ***

// *** START *** Parameters.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Tue Jan 05 16:39:23 2016 EDT
//
// Time-stamp: <2016-09-28 16:15:01 dcj>
//
// ================================================================

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

// ================================================================

#include <string>

// inlined #include "Units.h"
// inlined #include "Geometry/Vector3.h"

// ================================================================

/// Collects all the program parameters in one place.  Includes both
/// parameters read from the command line and from the bod file.
/// For some parameters, tracks whether they 
/// have been manually set or are still at their default value.
///
class Parameters
{
public:
  Parameters();
  ~Parameters();

  void parseCommandLine(int argc, char **argv);

  void print() const;

  std::string getInputFileName() const; 

  int getMpiSize() const;
  void setMpiSize(int mpiSize);

  int getMpiRank() const;
  void setMpiRank(int mpiRank);

  int getNumThreads() const;

  int getSeed() const;

  double getFracErrorBound() const;

  long long getTotalNumWalks() const;
  bool getTotalNumWalksWasSet() const;

  long long getTotalNumSamples() const;
  bool getTotalNumSamplesWasSet() const;

  double getMaxErrorCapacitance() const;
  bool getMaxErrorCapacitanceWasSet() const;

  double getMaxErrorPolarizability() const;
  bool getMaxErrorPolarizabilityWasSet() const;

  double getMaxErrorVolume() const;
  bool getMaxErrorVolumeWasSet() const;

  bool getComputeFormWasSet() const;

  long long getMinTotalNumWalks() const;

  long long getMinTotalNumSamples() const;

  std::string getSurfacePointsFileName() const;
  std::string getInteriorPointsFileName() const;

  bool getPrintCounts() const;
  bool getPrintBenchmarks() const;

  void setSkinThickness(double skinThickness);
  double getSkinThickness() const;
  bool getSkinThicknessWasSet() const;

  void setLaunchCenter(Vector3<double> launchCenter);
  Vector3<double> getLaunchCenter() const;
  bool getLaunchCenterWasSet() const;

  void setLaunchRadius(double launchRadius);
  double getLaunchRadius() const;
  bool getLaunchRadiusWasSet() const;

  void setLengthScale(double number, Units::Length unit);
  double getLengthScaleNumber() const;
  Units::Length getLengthScaleUnit() const;
  bool getLengthScaleWasSet() const;

  void setTemperature(double number, Units::Temperature unit);
  double getTemperatureNumber() const;
  Units::Temperature getTemperatureUnit() const;
  bool getTemperatureWasSet() const;

  void setMass(double number, Units::Mass unit);
  double getMassNumber() const;
  Units::Mass getMassUnit() const;
  bool getMassWasSet() const;

  void setSolventViscosity(double number, Units::Viscosity unit);
  double getSolventViscosityNumber() const;
  Units::Viscosity getSolventViscosityUnit() const;
  bool getSolventViscosityWasSet() const;

  void setBuoyancyFactor(double buoyancyFactor);
  double getBuoyancyFactor() const;
  bool getBuoyancyFactorWasSet() const;

  void mpiSend() const;
  void mpiReceive();

private:
  std::string inputFileName; 

  int mpiSize;
  int mpiRank;

  int numThreads;

  int seed;

  double fracErrorBound;

  long long totalNumWalks;
  bool totalNumWalksWasSet;

  long long totalNumSamples;
  bool totalNumSamplesWasSet;

  double maxErrorCapacitance;
  bool maxErrorCapacitanceWasSet;

  double maxErrorPolarizability;
  bool maxErrorPolarizabilityWasSet;

  double maxErrorVolume;
  bool maxErrorVolumeWasSet;

  bool computeFormWasSet;

  long long minTotalNumWalks;

  long long minTotalNumSamples;

  std::string surfacePointsFileName;
  std::string interiorPointsFileName;

  bool printCounts;
  bool printBenchmarks;

  // .bod parameters

  double skinThickness;
  bool skinThicknessWasSet;

  Vector3<double> launchCenter;
  bool launchCenterWasSet;

  double launchRadius;
  bool launchRadiusWasSet;

  double lengthScale;
  Units::Length lengthScaleUnit;
  bool lengthScaleWasSet;

  double temperature;
  Units::Temperature temperatureUnit;
  bool temperatureWasSet;

  double mass;
  Units::Mass massUnit;
  bool massWasSet;

  double solventViscosity;
  Units::Viscosity solventViscosityUnit;
  bool solventViscosityWasSet;

  double buoyancyFactor;
  bool buoyancyFactorWasSet;
};

// ================================================================

#endif  // #ifndef PARAMETERS_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** Parameters.h ***

// *** START *** Geometry/Sphere.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Thu Nov 13 12:27:37 2014 EDT
//
// Time-stamp: <2016-09-19 15:30:56 dcj>
//
// ================================================================

#ifndef SPHERE_H
#define SPHERE_H

#include <cmath>
#include <ostream>

// inlined #include "Vector3.h"

// ================================================================

/// Represents a sphere.
///
template <class T>
class Sphere {
public:
  Sphere();
  Sphere(Sphere<T> const & b);
  Sphere(Vector3<T> const & center, T radius);

  void setCenter(Vector3<T> const & center);
  Vector3<T> getCenter() const;

  void setRadiusSqr(T radiusSqr);
  T getRadiusSqr() const;

  void setRadius(T radius);
  T getRadius() const;

  T getMaxCoord(int dim) const;
  T getMinCoord(int dim) const;

  T getVolume() const;

private:
  Vector3<T> center;
  T radius;
};

template <class T>
std::ostream & operator<<(std::ostream & os, const Sphere<T> & rhs)
{
  os << rhs.getCenter() << ", " << rhs.getRadius();

  return os;
}

template <class T>
Sphere<T>::Sphere() {

}

template <class T>
Sphere<T>::Sphere(Sphere<T> const & b) 
: center(b.center),
  radius(b.radius) {

}

template <class T>
Sphere<T>::Sphere(Vector3<T> const & center, T radius) 
: center(center),
  radius(radius) {

}

template <class T>
void 
Sphere<T>::setCenter(Vector3<T> const & center) {
  this->center = center;
}

template <class T>
Vector3<T> 
Sphere<T>::getCenter() const {
  return center;
}

template <class T>
void 
Sphere<T>::setRadiusSqr(T radiusSqr) {
  this->radius = sqrt(radiusSqr);
}

template <class T>
T 
Sphere<T>::getRadiusSqr() const {
  return pow(radius, 2);
}

template <class T>
void 
Sphere<T>::setRadius(T radius) {
  this->radius = radius;
}

template <class T>
T 
Sphere<T>::getRadius() const {
  return radius;
}

/// Returns the maximum coordinate value of any point on the sphere along the
/// given dimension.
///
template <class T>
T 
Sphere<T>::getMaxCoord(int dim) const {
  return center.get(dim) + radius;
}

/// Returns the minimum coordinate value of any point on the sphere along the
/// given dimension.
///
template <class T>
T 
Sphere<T>::getMinCoord(int dim) const {
  return center.get(dim) - radius;
}

template <class T>
T 
Sphere<T>::getVolume() const {
  return (4./3.)*M_PI*pow(radius, 3);
}

// ================================================================

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** Geometry/Sphere.h ***

// *** START *** Geometry/Spheres.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Thu Nov 13 12:27:37 2014 EDT
//
// Time-stamp: <2016-09-19 16:04:34 dcj>
//
// ================================================================

#ifndef SPHERES_H
#define SPHERES_H

#ifdef USE_MPI
#include <mpi.h>
#endif

#include <vector>

// inlined #include "Sphere.h"

// ================================================================

/// Represents a set of spheres.
///
template <class T>
class Spheres {
public:
  Spheres();
  ~Spheres();

  void add(Sphere<T> const & sphere);

  std::vector<Sphere<T> > const & getVector() const;

  bool isEmpty() const;

  void mpiSend() const;
  void mpiReceive();

private:
  std::vector<Sphere<T> > spheres;
};

/// Construct an empty set of spheres.
///
template <class T>
Spheres<T>::Spheres() 
  : spheres() {

}

template <class T>
Spheres<T>::~Spheres() {

}

/// Add a sphere to the set.
///
template <class T>
void 
Spheres<T>::add(Sphere<T> const & sphere) {

  spheres.push_back(sphere);
}

/// Returns the set of spheres as a vector.
///
template <class T>
std::vector<Sphere<T> > const &
Spheres<T>::getVector() const {

  return spheres;
}

template <class T>
bool 
Spheres<T>::isEmpty() const {

  return (spheres.size() == 0);
}

/// Broadcasts the set of spheres over MPI.
///
template <class T>
void
Spheres<T>::mpiSend() const {
#ifdef USE_MPI
  int numSpheres = spheres.size();

  MPI_Bcast(&numSpheres, 1, MPI_INT, 0, MPI_COMM_WORLD);

  double * sphereArray = new double[numSpheres*4];

  for (int i = 0; i < numSpheres; i++) {
    sphereArray[i*4 + 0] = spheres.at(i).getCenter().get(0);
    sphereArray[i*4 + 1] = spheres.at(i).getCenter().get(1);
    sphereArray[i*4 + 2] = spheres.at(i).getCenter().get(2);
    sphereArray[i*4 + 3] = spheres.at(i).getRadius();
  }

  MPI_Bcast(sphereArray, numSpheres*4, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  delete [] sphereArray;
#endif
}

/// Receives a set of spheres over MPI and adds it to the current set.
///
template <class T>
void
Spheres<T>::mpiReceive() {
#ifdef USE_MPI
  int numSpheres = 0;

  MPI_Bcast(&numSpheres, 1, MPI_INT, 0, MPI_COMM_WORLD);

  double * sphereArray = new double[numSpheres * 4];

  MPI_Bcast(sphereArray, numSpheres*4, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  for (int i = 0; i < numSpheres; i++) {
    spheres.emplace_back(Vector3<double>(sphereArray[i*4 + 0],
					 sphereArray[i*4 + 1],
					 sphereArray[i*4 + 2]),
			 sphereArray[i*4 + 3]);
  }

  delete [] sphereArray;
#endif
}

// ================================================================

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** Geometry/Spheres.h ***

// *** START *** Parserbase.h ***
// Generated by Bisonc++ V4.05.00 on Wed, 28 Sep 2016 16:13:25 -0400

#ifndef ParserBase_h_included
#define ParserBase_h_included

#include <exception>
#include <vector>
#include <iostream>

// $insert preincludes
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace // anonymous
{
    struct PI__;
}


// $insert polymorphic
enum class Tag__
{
    STRING_TYPE,
    FLOAT_TYPE,
};

namespace Meta__
{
    template <Tag__ tag>
    struct TypeOfBase;

    template <typename Tp_>
    struct TagOf;

// $insert polymorphicSpecializations
    template <>
    struct TagOf<std::string>
    {
        static Tag__ const tag = Tag__::STRING_TYPE;
    };

    template <>
    struct TagOf<double>
    {
        static Tag__ const tag = Tag__::FLOAT_TYPE;
    };

    template <>
    struct TypeOfBase<Tag__::STRING_TYPE>
    {
        typedef std::string DataType;
    };

    template <>
    struct TypeOfBase<Tag__::FLOAT_TYPE>
    {
        typedef double DataType;
    };


        // determining the nature of a polymorphic semantic value:
        // if it's a class-type, use 'Type const &' as returntype of const
        // functions; if it's a built-in type (like 'int') use Type:

    struct ClassType
    {
        char _[2];
    };
    
    struct BuiltinType
    {
        char _;
    };

    template <typename T>
    BuiltinType test(...);

    template <typename T>
    ClassType test(void (T::*)());

    template <Tag__ tg_>
    struct TypeOf: public TypeOfBase<tg_>
    {
        typedef typename TypeOfBase<tg_>::DataType DataType;
        enum: bool 
        { 
            isBuiltinType = sizeof(test<DataType>(0)) == sizeof(BuiltinType)
        };

        typedef typename std::conditional<
                    isBuiltinType, DataType, DataType const &
                >::type ReturnType;
    };

        // The Base class: 
        // Individual semantic value classes are derived from this class.
        // This class offers a member returning the value's Tag__
        // and two member templates get() offering const/non-const access to
        // the actual semantic value type.
    class Base
    {
        Tag__ d_tag;
    
        protected:
            Base(Tag__ tag);

        public:
            Base(Base const &other) = delete;
            virtual ~Base();

            Tag__ tag() const;
    
            template <Tag__ tg_>
            typename TypeOf<tg_>::ReturnType get() const;
    
            template <Tag__ tg_>
            typename TypeOf<tg_>::DataType &get();
    };
    
        // The class Semantic is derived from Base. It stores a particular
        // semantic value type. The stored data are declared 'mutable' to
        // allow the definitions of a const and non-const conversion operator.
        // This way, const objects continue to offer non-modifiable data
    template <Tag__ tg_>
    class Semantic: public Base
    {
        typedef typename TypeOf<tg_>::DataType DataType;
    
        mutable DataType d_data;
    
        public:
            typedef typename TypeOf<tg_>::ReturnType ReturnType;
    
                // The default constructor and constructors for 
                // defined data types are available
            Semantic();
            Semantic(DataType const &data);
            Semantic(DataType &&tmp);

                // Conversion operators allow const/non-const access to d_data
            operator ReturnType() const;
            operator DataType &();
    };

        // The class Stype wraps the shared_ptr holding a pointer to Base.
        // It becomes the polymorphic STYPE__
        // Constructors expect (l/r-value) references to defined semantic
        // value types.
        // It also wraps Base's get members, allowing constructions like
        // $$.get<INT> to be used, rather than $$->get<INT>.
        // Furthermore, its operator= can be used to assign a Semantic *
        // directly to the SType object. The free functions (in the parser's
        // namespace (if defined)) semantic__ can be used to obtain a 
        // Semantic *. 
    class SType: public std::shared_ptr<Base>
    {
        public:
            SType() = default;
            SType(SType const &other) = default;
            SType(SType &&tmp) = default;
        
            SType &operator=(SType const &rhs) = default;
            SType &operator=(SType &&tmp) = default;
            template <typename Tp_>
            SType &operator=(Tp_ &&value);

            Tag__ tag() const;

                // this get()-member checks for 0-pointer and correct tag 
                // in shared_ptr<Base>, and may throw a logic_error
            template <Tag__ tg_>                    
            typename TypeOf<tg_>::ReturnType get() const;
    
                // this get()-member checks for 0-pointer and correct tag 
                // in shared_ptr<Base>, and resets the shared_ptr's Base * 
                // to point to Meta::__Semantic<tg_>() if not
            template <Tag__ tg_>
            typename TypeOf<tg_>::DataType &get();

                // the data()-members do not check, and may result in a 
                // bad_cast exception or segfault if used incorrectly

            template <Tag__ tg_>
            typename TypeOf<tg_>::ReturnType data() const;

            template <Tag__ tg_>
            typename TypeOf<tg_>::DataType &data();
    };

}  // namespace Meta__

class ParserBase
{
    public:
// $insert tokens

    // Symbolic tokens:
    enum Tokens__
    {
        INT = 257,
        FLOAT,
        SPHERE,
        ATOM,
        ST,
        RLAUNCH,
        HUNITS,
        UNITS,
        TEMP,
        MASS,
        VISCOSITY,
        BF,
        STRING,
    };

// $insert STYPE
    typedef Meta__::SType STYPE__;


    private:
        int d_stackIdx__;
        std::vector<size_t>   d_stateStack__;
        std::vector<STYPE__>  d_valueStack__;

    protected:
        enum Return__
        {
            PARSE_ACCEPT__ = 0,   // values used as parse()'s return values
            PARSE_ABORT__  = 1
        };
        enum ErrorRecovery__
        {
            DEFAULT_RECOVERY_MODE__,
            UNEXPECTED_TOKEN__,
        };
        bool        d_debug__;
        size_t      d_nErrors__;
        size_t      d_requiredTokens__;
        size_t      d_acceptedTokens__;
        int         d_token__;
        int         d_nextToken__;
        size_t      d_state__;
        STYPE__    *d_vsp__;
        STYPE__     d_val__;
        STYPE__     d_nextVal__;

        ParserBase();

        void ABORT() const;
        void ACCEPT() const;
        void ERROR_() const;
        void clearin();
        bool debug() const;
        void pop__(size_t count = 1);
        void push__(size_t nextState);
        void popToken__();
        void pushToken__(int token);
        void reduce__(PI__ const &productionInfo);
        void errorVerbose__();
        size_t top__() const;

    public:
        void setDebug(bool mode);
}; 

inline bool ParserBase::debug() const
{
    return d_debug__;
}

inline void ParserBase::setDebug(bool mode)
{
    d_debug__ = mode;
}

inline void ParserBase::ABORT() const
{
    throw PARSE_ABORT__;
}

inline void ParserBase::ACCEPT() const
{
    throw PARSE_ACCEPT__;
}

inline void ParserBase::ERROR_() const
{
    throw UNEXPECTED_TOKEN__;
}

// $insert polymorphicInline
namespace Meta__
{

inline Base::Base(Tag__ tag)
:
    d_tag(tag)
{}

inline Tag__ Base::tag() const
{
    return d_tag;
}

template <Tag__ tg_>
inline Semantic<tg_>::Semantic()
:
    Base(tg_),
    d_data(typename TypeOf<tg_>::DataType())
{}

template <Tag__ tg_>
inline Semantic<tg_>::Semantic(typename TypeOf<tg_>::DataType const &data)
:
    Base(tg_),
    d_data(data)
{}

template <Tag__ tg_>
inline Semantic<tg_>::Semantic(typename TypeOf<tg_>::DataType &&tmp)
:
    Base(tg_),
    d_data(std::move(tmp))
{}

template <Tag__ tg_>
inline Semantic<tg_>::operator ReturnType() const
{
    return d_data;
}

template <Tag__ tg_>
inline Semantic<tg_>::operator typename Semantic<tg_>::DataType &()
{
    return d_data;
}

template <Tag__ tg_>
inline typename TypeOf<tg_>::ReturnType Base::get() const
{
    return dynamic_cast<Semantic<tg_> const &>(*this);
}

template <Tag__ tg_>
inline typename TypeOf<tg_>::DataType &Base::get()
{
    return dynamic_cast<Semantic<tg_> &>(*this);
}

inline Tag__ SType::tag() const
{
    return std::shared_ptr<Base>::get()->tag();
}
    
template <Tag__ tg_>
inline typename TypeOf<tg_>::ReturnType SType::get() const
{
    if (std::shared_ptr<Base>::get() == 0  || tag() != tg_)
        throw std::logic_error("undefined semantic value requested");

    return std::shared_ptr<Base>::get()->get<tg_>();
}

template <Tag__ tg_>
inline typename TypeOf<tg_>::DataType &SType::get()
{
                    // if we're not yet holding a (tg_) value, initialize to 
                    // a Semantic<tg_> holding a default value
    if (std::shared_ptr<Base>::get() == 0 || tag() != tg_)
        reset(new Semantic<tg_>());

    return std::shared_ptr<Base>::get()->get<tg_>();
}

template <Tag__ tg_>
inline typename TypeOf<tg_>::ReturnType SType::data() const
{
    return std::shared_ptr<Base>::get()->get<tg_>();
}

template <Tag__ tg_>
inline typename TypeOf<tg_>::DataType &SType::data()
{
    return std::shared_ptr<Base>::get()->get<tg_>();
}

template <bool, typename Tp_>
struct Assign;

template <typename Tp_>
struct Assign<true, Tp_>
{
    static SType &assign(SType *lhs, Tp_ &&tp);
};

template <typename Tp_>
struct Assign<false, Tp_>
{
    static SType &assign(SType *lhs, Tp_ const &tp);
};

template <>
struct Assign<false, SType>
{
    static SType &assign(SType *lhs, SType const &tp);
};

template <typename Tp_>
inline SType &Assign<true, Tp_>::assign(SType *lhs, Tp_ &&tp)
{
    lhs->reset(new Semantic<TagOf<Tp_>::tag>(std::move(tp)));
    return *lhs;
}

template <typename Tp_>
inline SType &Assign<false, Tp_>::assign(SType *lhs, Tp_ const &tp)
{
    lhs->reset(new Semantic<TagOf<Tp_>::tag>(tp));
    return *lhs;
}

inline SType &Assign<false, SType>::assign(SType *lhs, SType const &tp)
{
    return lhs->operator=(tp);
}

template <typename Tp_>
inline SType &SType::operator=(Tp_ &&rhs) 
{
    return Assign<
                std::is_rvalue_reference<Tp_ &&>::value, 
                typename std::remove_reference<Tp_>::type
           >::assign(this, std::forward<Tp_>(rhs));
}

} // namespace Meta__

// As a convenience, when including ParserBase.h its symbols are available as
// symbols in the class Parser, too.
#define Parser ParserBase


#endif



// *** END *** Parserbase.h ***

// *** START *** Parser.h ***
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
// Date:    Wed Feb 19 11:59:38 2014 EDT
//
// Time-stamp: <2016-09-22 13:10:16 dcj>
//
// ================================================================

// Generated by Bisonc++ V4.04.01 on Wed, 19 Feb 2014 11:59:38 -0500

#ifndef Parser_h_included
#define Parser_h_included

#include <vector>
#include <string>

// inlined #include "Parameters.h"
// inlined #include "Geometry/Spheres.h"

// $insert baseclass
// inlined #include "Parserbase.h"
// $insert scanner.h
// inlined #include "Scanner.h"

// ================================================================

#undef Parser
class Parser: public ParserBase
{
    // $insert scannerobject
    Scanner d_scanner;

    Parameters * parameters;
    Spheres<double> * spheres;

    public:
        Parser(std::istream &in,
	       Parameters * parameters, 
	       Spheres<double> * spheres);

        int parse();

    private:
        void error(char const *msg);    // called on (syntax) errors
        int lex();                      // returns the next token from the
                                        // lexical scanner. 
        void print();                   // use, e.g., d_token, d_loc

	void addSphere(double x, double y, double z, double r);
	void setST(double skinThickness);
	void setRLAUNCH(double launchRadius);
	void setHUNITS(double number, std::string unitString);
	void setUNITS(std::string unitString);
	void setTEMP(double number, std::string unitString);
	void setMASS(double number, std::string unitString);
	void setVISCOSITY(double number, std::string unitString);
	void setBF(double buoyancyFactor);

    // support functions for parse():
        void executeAction(int ruleNr);
        void errorRecovery();
        int lookup(bool recovery);
        void nextToken();
        void print__();
        void exceptionHandler__(std::exception const &exc);
};


#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** Parser.h ***

// *** START *** Scanner.ih ***
// ================================================================
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
// Date:    Wed Feb 19 11:52:22 2014 EDT
//
// Time-stamp: <2014-02-24 13:48:05 dcj>
//
// ================================================================

// Generated by Flexc++ V1.05.00 on Wed, 19 Feb 2014 11:52:22 -0500

// $insert class_h
// inlined #include "Scanner.h"

// inlined #include "Parser.h"

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:
// *** END *** Scanner.ih ***

// *** START *** Parser.ih ***
// ================================================================
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
// Date:    Wed Feb 19 11:59:38 2014 EDT
//
// Time-stamp: <2014-02-24 13:46:38 dcj>
//
// ================================================================

// Generated by Bisonc++ V4.04.01 on Wed, 19 Feb 2014 11:59:38 -0500

    // Include this file in the sources of the class Parser.

// $insert class.h
// inlined #include "Parser.h"


inline void Parser::error(char const *msg)
{
    std::cerr << msg << '\n';
}

// $insert lex
inline int Parser::lex()
{
    return d_scanner.lex();
}

inline void Parser::print()         
{
    print__();           // displays tokens if --print was specified
}

inline void Parser::exceptionHandler__(std::exception const & /* exc */ )         
{
    throw;              // re-implement to handle exceptions thrown by actions
}


    // Add here includes that are only required for the compilation 
    // of Parser's sources.



    // UN-comment the next using-declaration if you want to use
    // int Parser's sources symbols from the namespace std without
    // specifying std::

//using namespace std;

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:
// *** END *** Parser.ih ***

// *** START *** ResultsZeno.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Wed Apr 22 11:11:48 2015 EDT
//
// Time-stamp: <2016-08-29 16:45:45 dcj>
//
// ================================================================

#ifndef RESULTS_ZENO_H_
#define RESULTS_ZENO_H_

// ================================================================

#include <iostream>
#include <algorithm>
#include <cmath>
#include <cassert>

// inlined #include "Geometry/Vector3.h"
// inlined #include "Geometry/Matrix3x3.h"
// inlined #include "Geometry/Sphere.h"

// inlined #include "Uncertain.h"

// ================================================================

/// Collects results from the Walk-on-Spheres computation.
///
class ResultsZeno {
public:
  ResultsZeno(Sphere<double> const & boundingSphere,
	      int numThreads,
	      bool saveHitPoints);

  ~ResultsZeno();

  template <class RandomNumberGenerator>
  void recordHit(int threadNum,
		 Vector3<double> const & startPoint,
		 Vector3<double> const & endPoint,
		 Vector3<double> const & endPointNormal,
		 RandomNumberGenerator * randomNumberGenerator);

  void recordMiss(int threadNum);

  void reduce();

  void gatherHitPoints();

  double getNumWalks() const;

  Uncertain<double> getNumHits() const;

  Vector3<Uncertain<double> > getKPlus() const;
  Vector3<Uncertain<double> > getKMinus() const;

  Matrix3x3<Uncertain<double> > getVPlus() const;
  Matrix3x3<Uncertain<double> > getVMinus() const;

  bool getSaveHitPoints() const;

  std::vector<Vector3<double> > const * getPoints() const;
  std::vector<Vector3<double> > const * getNormals() const;
  std::vector<Vector3<char> > const * getCharges() const;

private:
  void updateVariance(int threadNum,
		      double hitMissData,
		      Vector3<double> const & KPlusData, 
		      Vector3<double> const & KMinusData,
		      Matrix3x3<double> const & VPlusData, 
		      Matrix3x3<double> const & VMinusData);

  template <class T>
  void updateItemVariance(T const & data,
			  double num,
			  T * mean,
			  T * M2);

  double const boundingSphereRadius;
  Vector3<double> const boundingSphereCenter;

  int const numThreads;

  bool saveHitPoints;

  double * numWalks;

  double * hitMissMean;
  double * hitMissM2;

  Vector3<double> * KPlus;
  Vector3<double> * KMinus;

  Vector3<double> * KPlusMean;
  Vector3<double> * KMinusMean;

  Vector3<double> * KPlusM2;
  Vector3<double> * KMinusM2;

  Matrix3x3<double> * VPlus;
  Matrix3x3<double> * VMinus;

  Matrix3x3<double> * VPlusMean;
  Matrix3x3<double> * VMinusMean;

  Matrix3x3<double> * VPlusM2;
  Matrix3x3<double> * VMinusM2;

  double numWalksReduced;

  double numHitsReduced;
  double numHitsVarianceReduced;

  Vector3<double> KPlusReduced;
  Vector3<double> KMinusReduced;

  Vector3<double> KPlusVarianceReduced;
  Vector3<double> KMinusVarianceReduced;

  Matrix3x3<double> VPlusReduced;
  Matrix3x3<double> VMinusReduced;

  Matrix3x3<double> VPlusVarianceReduced;
  Matrix3x3<double> VMinusVarianceReduced;

  std::vector<Vector3<double> > * points;
  std::vector<Vector3<double> > * normals;
  std::vector<Vector3<char> > * charges;

  std::vector<Vector3<double> > gatheredPoints;
  std::vector<Vector3<double> > gatheredNormals;
  std::vector<Vector3<char> > gatheredCharges;

  bool reduced;
  bool hitPointsGathered;
};

/// Record a hit from the given thread number of a walk from the given start
/// point to the given end point with the given end point surface normal.
/// Walker charges are assigned using the given random number generator.
///
template <class RandomNumberGenerator>
void 
ResultsZeno::
recordHit(int threadNum,
	  Vector3<double> const & startPoint,
	  Vector3<double> const & endPoint,
	  Vector3<double> const & endPointNormal,
	  RandomNumberGenerator * randomNumberGenerator) {

  assert(threadNum >= 0 && threadNum < numThreads);

  reduced = false;

  Vector3<double> normalizedStartPoint = startPoint - boundingSphereCenter;
  Vector3<double> normalizedEndPoint   = endPoint - boundingSphereCenter;

  Vector3<char> walkCharges;

  double hitMissData = 1;

  Vector3<double> KPlusData(0, 0, 0);
  Vector3<double> KMinusData(0, 0, 0);

  Matrix3x3<double> VPlusData(0, 0, 0, 0, 0, 0, 0, 0, 0);
  Matrix3x3<double> VMinusData(0, 0, 0, 0, 0, 0, 0, 0, 0);

  numWalks[threadNum]++;

  for (int dim = 0; dim < 3; dim++) {
    double probability = 
      0.5 + normalizedStartPoint.get(dim)/(2*boundingSphereRadius);

    if (probability > randomNumberGenerator->getRandIn01()) {
      //c[dim] == +1

      walkCharges.set(dim, '+');

      KPlusData.add(dim, 1);
      VPlusData.addRow(dim, normalizedEndPoint);
    }
    else {
      //c[dim] == -1

      walkCharges.set(dim, '-');

      KMinusData.add(dim, 1);
      VMinusData.addRow(dim, normalizedEndPoint);
    }
  }

  KPlus[threadNum]  += KPlusData;
  KMinus[threadNum] += KMinusData;

  VPlus[threadNum]  += VPlusData;
  VMinus[threadNum] += VMinusData;

  updateVariance(threadNum,
		 hitMissData,
		 KPlusData, 
		 KMinusData,
		 VPlusData, 
		 VMinusData);

  if (saveHitPoints) {
    hitPointsGathered = false;

    points[threadNum].push_back(endPoint);
    normals[threadNum].push_back(endPointNormal);
    charges[threadNum].push_back(walkCharges);
  }
}

template <class T>
void
ResultsZeno::
updateItemVariance(T const & data,
		   double num,
		   T * mean,
		   T * M2) {

  T delta = data - (*mean);
  (*mean) += delta / num;
  (*M2) += delta * (data - (*mean));
}

// ================================================================

#endif  // #ifndef RESULTS_ZENO_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** ResultsZeno.h ***

// *** START *** ResultsInterior.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Wed Apr 22 11:11:48 2015 EDT
//
// Time-stamp: <2016-08-29 14:59:25 dcj>
//
// ================================================================

#ifndef RESULTS_INTERIOR_H_
#define RESULTS_INTERIOR_H_

// ================================================================

#include <vector>

// inlined #include "Geometry/Vector3.h"

// inlined #include "Uncertain.h"

// ================================================================

/// Collects results from the Interior Sampling computation.
///
class ResultsInterior {
public:
  ResultsInterior(int numThreads,
		  bool saveHitPoints);

  ~ResultsInterior();

  void recordHit(int threadNum,
		 Vector3<double> const & point);

  void recordMiss(int threadNum);

  void reduce();

  void gatherHitPoints();

  Uncertain<double> getNumHits() const;

  Matrix3x3<Uncertain<double> > getHitPointsSqrSum() const;

  Vector3<Uncertain<double> > getHitPointsSum() const;

  double getNumSamples() const;

  bool getSaveHitPoints() const;

  std::vector<Vector3<double> > const * getPoints() const;

private:
  void updateVariance(int threadNum,
		      double hitMissData,
		      Matrix3x3<double> const & hitPointsSqrData,
		      Vector3<double> const & hitPointsData);

  template <class T>
  void updateItemVariance(T const & data,
			  double num,
			  T * mean,
			  T * M2);

  template <class T>
  void reduceItem(T const & mean,
		  T const & M2,
		  double num,
		  T * sumReduced,
		  T * sumVarianceReduced);

  int const numThreads;

  bool saveHitPoints;

  double * numSamples;

  double * hitMissMean;
  double * hitMissM2;

  Matrix3x3<double> * hitPointsSqrMean;
  Matrix3x3<double> * hitPointsSqrM2;

  Vector3<double> * hitPointsMean;
  Vector3<double> * hitPointsM2;

  double numSamplesReduced;

  double numHitsReduced;
  double numHitsVarianceReduced;

  Matrix3x3<double> hitPointsSqrSumReduced;
  Matrix3x3<double> hitPointsSqrSumVarianceReduced;

  Vector3<double> hitPointsSumReduced;
  Vector3<double> hitPointsSumVarianceReduced;

  std::vector<Vector3<double> > * points;

  std::vector<Vector3<double> > gatheredPoints;

  bool reduced;
  bool hitPointsGathered;
};

template <class T>
void
ResultsInterior::
updateItemVariance(T const & data,
		   double num,
		   T * mean,
		   T * M2) {

  T delta = data - (*mean);
  (*mean) += delta / num;
  (*M2) += delta * (data - (*mean));
}

template <class T>
void
ResultsInterior::
reduceItem(T const & mean,
	   T const & M2,
	   double num,
	   T * sumReduced,
	   T * sumVarianceReduced) {

    T sum = mean * num;

    (*sumReduced) += sum;

    T variance = M2 / (num - 1);
    T meanVariance = variance / num;
    T sumVariance = meanVariance * pow(num, 2);

    (*sumVarianceReduced) += sumVariance;
}

// ================================================================

#endif  // #ifndef RESULTS_INTERIOR_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** ResultsInterior.h ***

// *** START *** ResultsCompiler.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Wed Apr 22 11:11:48 2015 EDT
//
// Time-stamp: <2016-08-29 14:42:11 dcj>
//
// ================================================================

#ifndef RESULTS_COMPILER_H_
#define RESULTS_COMPILER_H_

// ================================================================

#include <vector>
#include <array>
#include <cstdint>

// inlined #include "ResultsZeno.h"
// inlined #include "ResultsInterior.h"
// inlined #include "Uncertain.h"
// inlined #include "Parameters.h"

// inlined #include "Geometry/Vector3.h"
// inlined #include "Geometry/Matrix3x3.h"
// inlined #include "Geometry/Sphere.h"

// ================================================================

/// Takes results from the Walk-on-Spheres and Interior Sampling computations
/// and derives physical quantities from them.
///
class ResultsCompiler {
public:
  ResultsCompiler(Parameters const & parameters);

  ~ResultsCompiler();

  void compile(ResultsZeno const * resultsZeno,
	       ResultsInterior const * resultsInterior,
	       Sphere<double> const & boundingSphere,
	       bool computeForm);

  void print(bool printCounts) const;

  Uncertain<double> getCapacitance() const;

  Uncertain<double> getMeanPolarizability() const;

  Uncertain<double> getVolume() const;

private:
  using BigUInt = uint_fast64_t;

  static const unsigned int numFormFactors = 81;

  Uncertain<double> 
  computeCapacitance(Uncertain<double> const & t, 
		     double boundingSphereRadius) const;

  Matrix3x3<Uncertain<double> > 
  computePolarizability(Uncertain<double> const & t,
			Vector3<Uncertain<double> > const & u,
			Matrix3x3<Uncertain<double> > const & v,
			Matrix3x3<Uncertain<double> > const & w,
			double boundingSphereRadius) const;

  Uncertain<double> 
  computeMeanPolarizability(Matrix3x3<Uncertain<double> > const & 
			    polarizabilityTensor) const;

  Uncertain<double> 
  computePadeApproximant(Matrix3x3<Uncertain<double> > const & 
			 polarizabilityTensor) const;

  Uncertain<double> 
  computePadeApproximant(Uncertain<double> x1, 
			 Uncertain<double> x2) const;

  Uncertain<double>
  computeVolume(Uncertain<double> const & numInteriorHits, 
		double numInteriorSamples,
		double boundingSphereVolume) const;

  Uncertain<double>
  computeIntrinsicConductivity(Uncertain<double> const & meanPolarizability,
			       Uncertain<double> const & volume) const;

  Uncertain<double>
  computeCapacitanceOfASphere(Uncertain<double> const & volume) const;

  Uncertain<double>
  computeHydrodynamicRadius(Uncertain<double> const & capacitance) const;

  Uncertain<double> 
  computeViscometricRadius(Uncertain<double> const & meanPolarizability,
			   Uncertain<double> const & padeApproximant) const;

  Uncertain<double> 
  computeIntrinsicViscosity(Uncertain<double> const & padeApproximant,
			    Uncertain<double> const & intrinsicConductivity) 
    const;
			    
  Uncertain<double>
  computeIntrinsicViscosityConventional
  (Uncertain<double> const & padeApproximant,
   Uncertain<double> const & meanPolarizability,
   double mass) const;

  Uncertain<double>
  computeFrictionCoefficient(double solventViscosity,
			     Uncertain<double> const & hydrodynamicRadius) 
    const;

  Uncertain<double>
  computeDiffusionCoefficient(double temperature,
			      double solventViscosity,
			      Uncertain<double> const & hydrodynamicRadius) 
    const;

  Uncertain<double>
  computeSedimentationCoefficient(double mass,
				  double buoyancyFactor,
				  double solventViscosity,
				  Uncertain<double> const & hydrodynamicRadius)
    const;

  Matrix3x3<Uncertain<double> >
  computeGyrationTensor(Matrix3x3<Uncertain<double> > const & hitPointsSqrSum,
			Vector3<Uncertain<double> > const & hitPointsSum,
			Uncertain<double> const & numInteriorHits) const;

  std::array<double, numFormFactors>
  computeFormFactorQs(double boundingSphereRadius) const;

  std::array<double, numFormFactors>
  computeFormFactors(std::vector<Vector3<double> > const & interiorPoints,
		     std::array<double, 
		     numFormFactors> const & 
		     formFactorQs) const;

  void
  computeFormFactorsThread(int threadNum,
			   std::vector<Vector3<double> > const & 
			   interiorPoints,
			   std::array<double, 
			   numFormFactors> const & 
			   formFactorQs,
			   BigUInt startPairIndex,
			   BigUInt endPairIndex,
			   std::array<double, numFormFactors> * 
			   threadFormFactors) const;

  void indexToIJ(BigUInt index, BigUInt * i, BigUInt * j) const;

  Parameters const * parameters;

  double boundingSphereRadius;
  Vector3<double> boundingSphereCenter;

  Uncertain<double> t;
  Vector3<Uncertain<double> > u;
  Matrix3x3<Uncertain<double> > v;
  Matrix3x3<Uncertain<double> > w;

  Uncertain<double> numInteriorHits;

  Uncertain<double> capacitance;
  Matrix3x3<Uncertain<double> > polarizabilityTensor;
  Uncertain<double> meanPolarizability;
  Vector3<Uncertain<double> > polarizabilityEigenvalues;
  Uncertain<double> volume;
  Uncertain<double> intrinsicConductivity;
  Uncertain<double> capacitanceOfASphere;
  Uncertain<double> hydrodynamicRadius;
  Uncertain<double> q_eta;
  Uncertain<double> viscometricRadius;
  Uncertain<double> intrinsicViscosity;
  Uncertain<double> intrinsicViscosityConventional;
  Uncertain<double> frictionCoefficient;
  Uncertain<double> diffusionCoefficient;
  Uncertain<double> sedimentationCoefficient;
  Matrix3x3<Uncertain<double> > gyrationTensor;
  Vector3<Uncertain<double> > gyrationEigenvalues;
  std::array<double, numFormFactors> formFactorQs;
  std::array<double, numFormFactors> formFactors;

  bool intrinsicViscosityConventionalComputed;
  bool frictionCoefficientComputed;
  bool diffusionCoefficientComputed;
  bool sedimentationCoefficientComputed;

  bool resultsZenoCompiled;
  bool resultsInteriorCompiled;
  bool formResultsCompiled;
};

// ================================================================

#endif  // #ifndef RESULTS_COMPILER_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** ResultsCompiler.h ***

// *** START *** NearestSurfacePoint/PointFromSphereCenters.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Mon Aug 17 17:01:47 2015 EDT
//
// Time-stamp: <2016-09-20 14:58:01 dcj>
//
// ================================================================

#ifndef POINT_FROM_SPHERE_CENTERS_H
#define POINT_FROM_SPHERE_CENTERS_H

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

// ================================================================

/// Finds the nearest point on the surface of an object represented by
/// sphere centers stored in spatial data structures, with spheres of each
/// radius being stored in a separate data structure.
///
template <class SphereCenterModel>
class PointFromSphereCenters
{
public:
  PointFromSphereCenters(SphereCenterModel const & sphereCenterModel);
  ~PointFromSphereCenters();

  void findNearestPoint(Vector3<double> const & queryPoint,
			double fracErrorBound,
			Vector3<double> * nearestPointNormal,
			double * distance) const;

private:
  SphereCenterModel const * sphereCenterModel;
};

template <class SphereCenterModel>
PointFromSphereCenters<SphereCenterModel>::
PointFromSphereCenters(SphereCenterModel const & sphereCenterModel) 
  : sphereCenterModel(&sphereCenterModel) {
  
}

template <class SphereCenterModel>
PointFromSphereCenters<SphereCenterModel>::
~PointFromSphereCenters() {

}

/// Compute the distance from the given query point to the nearest point on the 
/// surface of an object, allowing for the given relative error in distance.  
/// Also compute the surface normal at that point.
///
template <class SphereCenterModel>
void 
PointFromSphereCenters<SphereCenterModel>::
findNearestPoint(Vector3<double> const & queryPoint,
		 double fracErrorBound, 
		 Vector3<double> * nearestPointNormal,
		 double * distance) const {

  Sphere<double> const * nearestSphere = NULL;

  double minDistance = std::numeric_limits<double>::max();

  for (unsigned int radiusNum = 0; 
       radiusNum < sphereCenterModel->getNumRadii(); 
       radiusNum ++) {

    double centerDistSqr = -1;

    Sphere<double> const * foundSphere = NULL;

    sphereCenterModel->findNearestSphere(radiusNum,
					 queryPoint,
					 fracErrorBound,
					 &foundSphere, 
					 &centerDistSqr); 

    double foundDistance = 
      sqrt(centerDistSqr) - foundSphere->getRadius();

    if (foundDistance < minDistance) {
      nearestSphere = foundSphere;
      minDistance   = foundDistance;
    }
  }

  (*nearestPointNormal) = queryPoint - nearestSphere->getCenter();
  (*distance)           = minDistance;
}

// ================================================================

#endif  // #ifndef POINT_FROM_SPHERE_CENTERS_H

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** NearestSurfacePoint/PointFromSphereCenters.h ***

// *** START *** InsideOutside/InOutSphereCenters.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Mon Aug 17 17:01:47 2015 EDT
//
// Time-stamp: <2016-09-20 14:36:04 dcj>
//
// ================================================================

#ifndef IN_OUT_SPHERE_CENTERS_H
#define IN_OUT_SPHERE_CENTERS_H

// inlined #include "../Geometry/Vector3.h"

// ================================================================

/// Tests whether a point is inside or outside an object represented by
/// sphere centers stored in spatial data structures, with spheres of each
/// radius being stored in a separate data structure.
///
template <class SphereCenterModel>
class InOutSphereCenters
{
public:
  InOutSphereCenters(SphereCenterModel const & sphereCenterModel);
  ~InOutSphereCenters();

  bool isInside(Vector3<double> const & queryPoint,
		double fracErrorBound) const;

private:
  SphereCenterModel const * sphereCenterModel;
};

template <class SphereCenterModel>
InOutSphereCenters<SphereCenterModel>::
InOutSphereCenters(SphereCenterModel const & sphereCenterModel) 
  : sphereCenterModel(&sphereCenterModel) {
  
}

template <class SphereCenterModel>
InOutSphereCenters<SphereCenterModel>::
~InOutSphereCenters() {

}

/// Return whether the given query point is inside the object, allowing for
/// the given relative error in distance.
///
template <class SphereCenterModel>
bool 
InOutSphereCenters<SphereCenterModel>::
isInside(Vector3<double> const & queryPoint,
	 double fracErrorBound) const {

  for (unsigned int radiusNum = 0; 
       radiusNum < sphereCenterModel->getNumRadii(); 
       radiusNum ++) {

    double centerDistSqr = -1;

    Sphere<double> const * foundSphere = NULL;

    sphereCenterModel->findNearestSphere(radiusNum,
					 queryPoint,
					 fracErrorBound,
					 &foundSphere, 
					 &centerDistSqr); 

    if (centerDistSqr < foundSphere->getRadiusSqr()) {
      return true;
    }
  }

  return false;
}

// ================================================================

#endif  // #ifndef IN_OUT_SPHERE_CENTERS_H

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** InsideOutside/InOutSphereCenters.h ***

// *** START *** BoundingSphere/BoundingSphereAABB.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Thu Feb 12 16:39:41 2015 EDT
//
// Time-stamp: <2016-09-22 13:15:59 dcj>
//
// ================================================================

#ifndef BOUNDING_SPHERE_AABB_H
#define BOUNDING_SPHERE_AABB_H

// inlined #include "../Geometry/Vector3.h"
// inlined #include "../Geometry/Sphere.h"

/// Generates the tightest bounding sphere around the tightest
/// Axis-Aligned Bounding-Box around the given object.
///
template <class T>
class BoundingSphereAABB {
 public:
  static Sphere<T> generate(std::vector<Sphere<T> > const & spheres);

 private:
  static Sphere<T> generate(Vector3<T> const & minCornerCoords,
			    Vector3<T> const & maxCornerCoords);
};

/// Returns the bounding sphere for the given set of spheres.
///
template <class T>
Sphere<T> 
BoundingSphereAABB<T>::generate(std::vector<Sphere<T> > const & spheres) {

  Vector3<T> minCornerCoords(std::numeric_limits<T>::max(), 
			     std::numeric_limits<T>::max(),
			     std::numeric_limits<T>::max());

  Vector3<T> maxCornerCoords(0, 0, 0);

  for (typename std::vector<Sphere<T> >::const_iterator it = spheres.begin();
       it != spheres.end();
       ++it) {

    for (int dim = 0; dim < 3; dim++) {

      T minCoord = it->getMinCoord(dim);
      T maxCoord = it->getMaxCoord(dim);

      if (minCoord < minCornerCoords.get(dim)) {
	minCornerCoords.set(dim, minCoord);
      }

      if (maxCoord > maxCornerCoords.get(dim)) {
	maxCornerCoords.set(dim, maxCoord);
      }
    }
  }

  return generate(minCornerCoords,
		  maxCornerCoords);
}

/// Returns the bounding sphere for a box with the given max and min
/// coordinate values.
///
template <class T>
Sphere<T> 
BoundingSphereAABB<T>::generate(Vector3<T> const & minCornerCoords,
				Vector3<T> const & maxCornerCoords) {

  Vector3<T> AABBDiagonal((maxCornerCoords - minCornerCoords) / 2);

  Vector3<T> boundingSphereCenter(minCornerCoords + AABBDiagonal);

  T boundingSphereRadius = AABBDiagonal.getMagnitude();

  Sphere<T> boundingSphere(boundingSphereCenter, boundingSphereRadius);

  return boundingSphere;
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** BoundingSphere/BoundingSphereAABB.h ***

// *** START *** SpherePoint/RandomSpherePointMarsaglia.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Thu Feb 12 17:24:57 2015 EDT
//
// Time-stamp: <2016-09-19 17:43:13 dcj>
//
// ================================================================

#ifndef RANDOM_SPHERE_POINT_MARSAGLIA_H
#define RANDOM_SPHERE_POINT_MARSAGLIA_H

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

/// Generates random sample points on a sphere from a uniform
/// distribution using the Marsaglia method. 
///
/// Marsaglia, George. Choosing a Point from the Surface of a Sphere. Ann. Math. Statist. 43 (1972), no. 2, 645--646. doi:10.1214/aoms/1177692644. http://projecteuclid.org/euclid.aoms/1177692644.
///
template <class T, class RNG>
class RandomSpherePointMarsaglia {
 public:
  static Vector3<T> generate(RNG * rng, Sphere<T> const & sphere);
};

/// Generates a random point on the given sphere.
///
template <class T, class RNG>
Vector3<T> 
RandomSpherePointMarsaglia<T, RNG>::generate(RNG * rng, 
					     Sphere<T> const & sphere) {

  //generate point uniformly distributed on unit sphere

  T x = 0, y = 0, lengthSqr = 0;

  do {
    x = rng->getRandInRange(-1, 1);
    y = rng->getRandInRange(-1, 1);
    
    lengthSqr = x*x + y*y;
  } 
  while (lengthSqr > 1);

  T scale = 2*sqrt(1 - lengthSqr);

  Vector3<T> spherePoint(scale*x,
			 scale*y,
			 2*lengthSqr - 1);

  //move point onto requested sphere

  spherePoint *= sphere.getRadius();

  spherePoint += sphere.getCenter();

  return spherePoint;
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** SpherePoint/RandomSpherePointMarsaglia.h ***

// *** START *** SpherePoint/BiasedSpherePointDirect.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Thu Feb 12 17:24:57 2015 EDT
//
// Time-stamp: <2016-09-19 14:57:03 dcj>
//
// ================================================================

#ifndef BIASED_SPHERE_POINT_DIRECT_H
#define BIASED_SPHERE_POINT_DIRECT_H

#include <cmath>
#include <cassert>

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

/// Generates random sample points on a sphere from a biased (non-uniform)
/// distribution using a direct (non-iterative) method.  The distribution is
/// suitable for reinserting random walkers that have left the launch sphere. 
///
template <class T, 
          class RandomNumberGenerator, 
          class RandomSpherePointGenerator>
class BiasedSpherePointDirect {
 public:
  static Vector3<T> generate(RandomNumberGenerator * rng, 
			     Sphere<T> const & sphere,
			     Vector3<T> const & distributionCenter,
			     T alpha);

 private:
  static T computeCosTheta(T alpha, T R);
};

/// Generates a random point on the given sphere from a distribution centered
/// at the given point with distribution parameter "alpha".
///
template <class T, 
          class RandomNumberGenerator, 
          class RandomSpherePointGenerator>
Vector3<T> 
BiasedSpherePointDirect<T, 
                        RandomNumberGenerator,
                        RandomSpherePointGenerator>::
  generate(RandomNumberGenerator * rng, 
	   Sphere<T> const & sphere,
	   Vector3<T> const & distributionCenter,
	   T alpha) {

  //generate point on unit sphere centered at origin distributed around Z axis

  T R = rng->getRandInRange(0, 1);

  T cosTheta = computeCosTheta(alpha, R);

  T sinTheta = sqrt(1 - pow(cosTheta, 2));

  T phi = rng->getRandInRange(0, 2*M_PI);

  Vector3<T> point(sinTheta * cos(phi),
		   sinTheta * sin(phi),
		   cosTheta);

  //rotate point to be distributed around requested distribution center

  Vector3<T> recenteredDistributionCenter =
    distributionCenter - sphere.getCenter();

  recenteredDistributionCenter.normalize();

  //will cause problems if recenteredDistributionCenter is close to <0, 0, 1>
  Vector3<T> rotationAxis =
    recenteredDistributionCenter.cross(Vector3<T>(0, 0, 1));

  T sinRotationAngle = rotationAxis.getMagnitude();

  T cosRotationAngle =
    recenteredDistributionCenter.dot(Vector3<T>(0, 0, 1));

  point.rotate(cosRotationAngle, sinRotationAngle, rotationAxis);

  //move point onto requested sphere

  point *= sphere.getRadius();
  point += sphere.getCenter();

  return point;
}

template <class T, 
          class RandomNumberGenerator, 
          class RandomSpherePointGenerator>
T 
BiasedSpherePointDirect<T, 
                        RandomNumberGenerator,
                        RandomSpherePointGenerator>::
  computeCosTheta(T alpha, T R) {

  
    T num = (-pow(1 - alpha, 2) + 
	     2*(1 - alpha)*(1 + pow(alpha, 2))*R + 
	     2*alpha*(1 + pow(alpha, 2))*pow(R, 2));

    T den = pow(1 - alpha + 2*alpha*R, 2);

    assert(den != 0);

    T cosTheta = num/den;

    //correct for numerical errors
    if (cosTheta > 1) {
      // std::cerr << "Warning: cosTheta > 1 (cosTheta == " << cosTheta << ")" 
      //           << std::endl;

      cosTheta = 1;
    }

    return cosTheta;
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** SpherePoint/BiasedSpherePointDirect.h ***

// *** START *** SpherePoint/RandomBallPointRejection.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Thu Feb 12 17:24:57 2015 EDT
//
// Time-stamp: <2016-09-19 15:06:06 dcj>
//
// ================================================================

#ifndef RANDOM_BALL_POINT_REJECTION_H
#define RANDOM_BALL_POINT_REJECTION_H

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

/// Generates random sample points inside a sphere from a uniform
/// distribution using a rejection (iterative) method. 
///
template <class T, class RNG>
class RandomBallPointRejection {
 public:
  static Vector3<T> generate(RNG * rng, Sphere<T> const & ball);
};

/// Generates a random point inside the given sphere.
///
template <class T, class RNG>
Vector3<T> 
RandomBallPointRejection<T, RNG>::generate(RNG * rng, 
					   Sphere<T> const & ball) {

  //generate point uniformly distributed on unit ball

  T x = 0, y = 0, z = 0, lengthSqr = 0;

  do {
    x = rng->getRandInRange(-1, 1);
    y = rng->getRandInRange(-1, 1);
    z = rng->getRandInRange(-1, 1);
    
    lengthSqr = x*x + y*y + z*z;
  } 
  while (lengthSqr > 1);

  Vector3<T> ballPoint(x, y, z);

  //move point onto requested ball

  ballPoint *= ball.getRadius();

  ballPoint += ball.getCenter();

  return ballPoint;
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** SpherePoint/RandomBallPointRejection.h ***

// *** START *** Walker/WalkerExterior.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Fri Feb 13 13:31:22 2015 EDT
//
// Time-stamp: <2016-09-20 17:22:53 dcj>
//
// ================================================================

#ifndef WALKER_EXTERIOR_H
#define WALKER_EXTERIOR_H

#include <vector>

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

/// Performs random walks starting on a bounding sphere and determines whether 
/// they hit an object, allowing for a given relative error in distance.
///
template <class T, 
  class RandomNumberGenerator, 
  class NearestSurfacePointFinder,
  class RandomSpherePointGenerator,
  class BiasedSpherePointGenerator>
class WalkerExterior {
 public:
  WalkerExterior(RandomNumberGenerator * randomNumberGenerator, 
		 Sphere<T> const & boundingSphere, 
		 NearestSurfacePointFinder const & nearestSurfacePointFinder,
		 T fracErrorBound,
		 T shellThickness);

  ~WalkerExterior();

  void walk(bool * hitObject, int * numSteps,
	    Vector3<T> * startPoint, Vector3<T> * endPoint, 
	    Vector3<T> * normal);

 private:
  RandomNumberGenerator * randomNumberGenerator;
  Sphere<T> const * boundingSphere;
  NearestSurfacePointFinder const * nearestSurfacePointFinder;
  T fracErrorBound;
  T shellThickness;
};

template <class T, 
  class RandomNumberGenerator, 
  class NearestSurfacePointFinder,
  class RandomSpherePointGenerator,
  class BiasedSpherePointGenerator>
WalkerExterior<T, 
               RandomNumberGenerator, 
               NearestSurfacePointFinder, 
               RandomSpherePointGenerator,
               BiasedSpherePointGenerator>::
  WalkerExterior(RandomNumberGenerator * randomNumberGenerator, 
		 Sphere<T> const & boundingSphere, 
		 NearestSurfacePointFinder const & nearestSurfacePointFinder,
		 T fracErrorBound,
		 T shellThickness) :
  randomNumberGenerator(randomNumberGenerator), 
  boundingSphere(&boundingSphere),
  nearestSurfacePointFinder(&nearestSurfacePointFinder), 
  fracErrorBound(fracErrorBound),
  shellThickness(shellThickness) {

}

template <class T, 
  class RandomNumberGenerator, 
  class NearestSurfacePointFinder,
  class RandomSpherePointGenerator,
  class BiasedSpherePointGenerator>
WalkerExterior<T, 
               RandomNumberGenerator, 
               NearestSurfacePointFinder, 
               RandomSpherePointGenerator,
               BiasedSpherePointGenerator>::
  ~WalkerExterior() {

}

/// Perform a random walk and determine whether it hits the object, the number
/// of steps it took, its start and end points, and the surface normal of its
/// hit point.
///
template <class T, 
  class RandomNumberGenerator, 
  class NearestSurfacePointFinder,
  class RandomSpherePointGenerator,
  class BiasedSpherePointGenerator>
void 
WalkerExterior<T, 
               RandomNumberGenerator, 
               NearestSurfacePointFinder, 
               RandomSpherePointGenerator,
               BiasedSpherePointGenerator>::
  walk(bool * hitObject, int * numSteps,
       Vector3<T> * startPoint, Vector3<T> * endPoint,
       Vector3<T> * normal) {

  *hitObject = false;
  *numSteps  = 0;

  Vector3<T> position = 
    RandomSpherePointGenerator::generate(randomNumberGenerator, 
					 *boundingSphere);

  *startPoint = position;

  for (;;) {

    T minDistance = 0;
  
    nearestSurfacePointFinder->findNearestPoint(position,
						fracErrorBound, 
						normal,
						&minDistance);

    if (minDistance < shellThickness) {
      //walker is absorbed

      *endPoint  = position;
      *hitObject = true;
      return;
    }

    (*numSteps)++;

    Sphere<T> stepSphere(position, minDistance);

    position = RandomSpherePointGenerator::generate(randomNumberGenerator, 
						    stepSphere);

    T centerDistSqr = 
      (position - boundingSphere->getCenter()).getMagnitudeSqr();

    if (centerDistSqr > boundingSphere->getRadiusSqr()) {
      //walker left bounding sphere

      T alpha = boundingSphere->getRadius() / sqrt(centerDistSqr);

      if (randomNumberGenerator->getRandInRange(0, 1) > (1 - alpha)) {
	//walker is replaced

	position = BiasedSpherePointGenerator::generate(randomNumberGenerator, 
							*boundingSphere,
							position,
							alpha);
      }
      else {
	//walker escapes

	*hitObject = false;
	return;
      }
    }
  }
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** Walker/WalkerExterior.h ***

// *** START *** Walker/SamplerInterior.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Fri Feb 13 13:31:22 2015 EDT
//
// Time-stamp: <2016-09-20 17:22:40 dcj>
//
// ================================================================

#ifndef SAMPLER_INTERIOR_H
#define SAMPLER_INTERIOR_H

#include <vector>

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

/// Samples random points inside a bounding sphere and determines whether they
/// hit an object, allowing for a given relative error in distance.
///
template <class T, 
  class RandomNumberGenerator, 
  class InsideOutsideTester,
  class RandomBallPointGenerator>
class SamplerInterior {
 public:
  SamplerInterior(RandomNumberGenerator * randomNumberGenerator, 
		  Sphere<T> const & boundingSphere, 
		  InsideOutsideTester const & insideOutsideTester,
		  T fracErrorBound);

  ~SamplerInterior();

  void sample(bool * hitObject,
	      Vector3<T> * hitPoint);

 private:
  RandomNumberGenerator * randomNumberGenerator;
  Sphere<T> const * boundingSphere;
  InsideOutsideTester const * insideOutsideTester;
  T fracErrorBound;
};

template <class T, 
  class RandomNumberGenerator, 
  class InsideOutsideTester,
  class RandomBallPointGenerator>
SamplerInterior<T, 
               RandomNumberGenerator, 
               InsideOutsideTester, 
               RandomBallPointGenerator>::
  SamplerInterior(RandomNumberGenerator * randomNumberGenerator, 
		 Sphere<T> const & boundingSphere, 
		 InsideOutsideTester const & insideOutsideTester,
		 T fracErrorBound) :
  randomNumberGenerator(randomNumberGenerator), 
  boundingSphere(&boundingSphere),
  insideOutsideTester(&insideOutsideTester), 
  fracErrorBound(fracErrorBound) {

}

template <class T, 
  class RandomNumberGenerator, 
  class InsideOutsideTester,
  class RandomBallPointGenerator>
SamplerInterior<T, 
               RandomNumberGenerator, 
               InsideOutsideTester, 
               RandomBallPointGenerator>::
  ~SamplerInterior() {

}

/// Compute a random point and determine whether it hits the object.
///
template <class T, 
  class RandomNumberGenerator, 
  class InsideOutsideTester,
  class RandomBallPointGenerator>
void 
SamplerInterior<T, 
               RandomNumberGenerator, 
               InsideOutsideTester, 
               RandomBallPointGenerator>::
  sample(bool * hitObject,
	 Vector3<T> * hitPoint) {

  Vector3<T> position = 
    RandomBallPointGenerator::generate(randomNumberGenerator, *boundingSphere);

  *hitObject = 
    insideOutsideTester->isInside(position,
				  fracErrorBound);

  *hitPoint = position;
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** Walker/SamplerInterior.h ***

// *** START *** Timer.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Wed May 21 16:21:57 2014 EDT
//
// Time-stamp: <2016-08-30 11:45:53 dcj>
//
// ================================================================

#ifndef TIMER_H_
#define TIMER_H_

// ================================================================

#include <chrono>

// ================================================================

/// A stopwatch for benchmarking.
///
class Timer
{
public:
  Timer();
  ~Timer();

  void start();
  void stop();
  void reset();

  double getTime() const;

private:
  bool running;

  std::chrono::high_resolution_clock::time_point startTime;

  double elapsedTime;
};
// ================================================================

#endif  // #ifndef TIMER_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** Timer.h ***

// *** START *** SphereCenterModel/NanoFLANNDatasetAdaptor.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Wed Nov 05 15:29:00 2014 EDT
//
// Time-stamp: <2016-09-20 16:19:40 dcj>
//
// ================================================================

#include <vector>
#include <cassert>

// #include <nanoflann.hpp>

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

// ================================================================

#ifndef NANOFLANNDATASETADAPTOR_H
#define NANOFLANNDATASETADAPTOR_H

/// Adaptor class used by the nanoFLANN library to access sphere center data.
///
template <typename DistanceType, typename ComponentType>
class NanoFLANNDatasetAdaptor {
 public:
  NanoFLANNDatasetAdaptor(int nPts,
			  int * originalSphereIndexes,
			  std::vector<Sphere<double> > const * originalSpheres);

  ~NanoFLANNDatasetAdaptor();

  /// Must return the number of data points
  size_t kdtree_get_point_count() const;

  /// Must return the Euclidean (L2) distance between the vector "p1[0:size-1]"
  /// and the data point with index "idx_p2" stored in the class:
  DistanceType kdtree_distance(const ComponentType *p1, const size_t idx_p2, 
			       size_t size) const;

  /// Must return the dim'th component of the idx'th point in the class:
  ComponentType kdtree_get_pt(const size_t idx, int dim) const;

  /// Optional bounding-box computation: return false to default to a standard 
  /// bbox computation loop.
  /// Return true if the BBOX was already computed by the class and returned in
  /// "bb" so it can be avoided to redo it again.
  /// Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 
  /// for point clouds)
  template <class BBOX>
  bool kdtree_get_bbox(BBOX &bb) const;

 private:
  int nPts;

  ComponentType * points;
};

template <typename DistanceType, typename ComponentType>
NanoFLANNDatasetAdaptor<DistanceType, ComponentType>::
NanoFLANNDatasetAdaptor(int nPts,
			int * originalSphereIndexes, 
			std::vector<Sphere<double> > const * originalSpheres)
  : nPts(nPts),
    points(NULL) {

  points = new ComponentType[nPts * 3];

  assert(points != NULL);

  for (int i = 0; i < nPts; i++) {
    const Vector3<double> center = 
      originalSpheres->at(originalSphereIndexes[i]).getCenter();

    for (int dim = 0; dim < 3; dim++) {
      points[i*3 + dim] = center.get(dim);
    }
  }
}

template <typename DistanceType, typename ComponentType>
NanoFLANNDatasetAdaptor<DistanceType, ComponentType>::
  ~NanoFLANNDatasetAdaptor () {

  delete [] points;
}

template <typename DistanceType, typename ComponentType>
size_t 
NanoFLANNDatasetAdaptor<DistanceType, ComponentType>::
  kdtree_get_point_count() const {

  return nPts;
}

template <typename DistanceType, typename ComponentType>
DistanceType 
NanoFLANNDatasetAdaptor<DistanceType, ComponentType>::
  kdtree_distance(const ComponentType *p1, 
		  const size_t idx_p2, 
		  size_t size) const {

  assert(size == 3);

  DistanceType distSqr = 0;

  for (int dim = 0; dim < 3; dim++) {
    distSqr += pow(p1[dim] - points[idx_p2*3 + dim], 2);
  }

  return distSqr;
}

template <typename DistanceType, typename ComponentType>
ComponentType 
NanoFLANNDatasetAdaptor<DistanceType, ComponentType>::
  kdtree_get_pt(const size_t idx, 
		int dim) const {

  return points[idx*3 + dim];
}

template <typename DistanceType, typename ComponentType>
template <class BBOX>
bool 
NanoFLANNDatasetAdaptor<DistanceType, ComponentType>::
kdtree_get_bbox(BBOX & /* bb */ ) const {

  return false;
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** SphereCenterModel/NanoFLANNDatasetAdaptor.h ***

// *** START *** SphereCenterModel/NanoFLANNSort.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Fri Aug 15 14:35:03 2014 EDT
//
// Time-stamp: <2016-09-20 16:48:15 dcj>
//
// ================================================================

#ifndef NANOFLANNSORT_H
#define NANOFLANNSORT_H

#include <vector>

// #include <nanoflann.hpp>

// inlined #include "NanoFLANNDatasetAdaptor.h"

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

// ================================================================

/// Spatial data structure based on the nanoFLANN library for storing spheres.
/// Spheres are sorted by radius, and the centers of the spheres of each radius
/// are stored in a separate nanoFLANN data structure.
///
class NanoFLANNSort {
 public:
  NanoFLANNSort();

  ~NanoFLANNSort();

  void preprocess(std::vector<Sphere<double> > const & spheres,
		  double fracErrorBound);

  unsigned int getNumRadii() const;

  void findNearestSphere(int radiusNum,
			 Vector3<double> const & queryPoint,
			 double fracErrorBound,
			 Sphere<double> const * * nearestSphere, 
			 double * centerDistSqr) const;

  void printDataStructureStats() const;
  void printSearchStats() const;

 private:
  typedef NanoFLANNDatasetAdaptor<double, double> DatasetAdaptorType;

  typedef nanoflann::L2_Simple_Adaptor<double, DatasetAdaptorType> MetricType;

  typedef nanoflann::KDTreeSingleIndexAdaptor<MetricType, 
                                              DatasetAdaptorType, 3, int> 
    KDTreeType;

  struct NanoFLANNInstance {
    double sphereRadiusSqr;

    int nPts;
    
    DatasetAdaptorType * dataset;
    KDTreeType * kdTree;

    int * originalSphereIndexes;
  };

  const std::vector<Sphere<double> > * originalSpheres;

  std::vector<NanoFLANNInstance> nanoFLANNInstances;
};

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** SphereCenterModel/NanoFLANNSort.h ***

// *** START *** RandomNumber/SPRNG.h ***
// ================================================================
// 
// Disclaimer:  IMPORTANT:  This software was developed at the
// National Institute of Standards and Technology by employees of the
// Federal Government in the course of their official duties.
// Pursuant to title 17 Section 105 of the United States Code this
// software is not subject to copyright protection and is in the
// public domain.  This is an experimental system.  NIST assumes no
// responsibility whatsoever for its use by other parties, and makes
// no guarantees, expressed or implied, about its quality,
// reliability, or any other characteristic.  We would appreciate
// acknowledgement if the software is used.  This software can be
// redistributed and/or modified freely provided that any derivative
// works bear some notice that they are derived from it, and any
// modified versions bear some notice that they have been modified.
// 
// ================================================================

// ================================================================
// 
// Authors: Derek Juba <derek.juba@nist.gov>
// Date:    Mon Dec 29 11:44:20 2014 EDT
//
// Time-stamp: <2016-09-20 15:02:25 dcj>
//
// ================================================================

#ifndef SPRNG_H_
#define SPRNG_H_

class Sprng;

// ================================================================

/// Generates random numbers using the SPRNG library.
///
class SPRNG
{
public:
  SPRNG(int streamNum, int numStreams, int seed);
  ~SPRNG();

  double getRandIn01();
  double getRandInRange(double min, double max);

private:
  Sprng *stream;
};

// ================================================================

#endif  // #ifndef SPRNG_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** RandomNumber/SPRNG.h ***

// *** START *** cmdline.h ***
/** @file cmdline.h
 *  @brief The header file for the command line option parser
 *  generated by GNU Gengetopt version 2.22.6
 *  http://www.gnu.org/software/gengetopt.
 *  DO NOT modify this file, since it can be overwritten
 *  @author GNU Gengetopt by Lorenzo Bettini */

#ifndef CMDLINE_H
#define CMDLINE_H

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
// inlined #include "config.h"
#endif

#include <stdio.h> /* for FILE */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CMDLINE_PARSER_PACKAGE
/** @brief the program name (used for printing errors) */
#define CMDLINE_PARSER_PACKAGE "zeno"
#endif

#ifndef CMDLINE_PARSER_PACKAGE_NAME
/** @brief the complete program name (used for help and version) */
#define CMDLINE_PARSER_PACKAGE_NAME "zeno"
#endif

#ifndef CMDLINE_PARSER_VERSION
/** @brief the program version */
#define CMDLINE_PARSER_VERSION "5.0.1"
#endif

/** @brief Where the command line options are stored */
struct gengetopt_args_info
{
  const char *help_help; /**< @brief Print help and exit help description.  */
  const char *full_help_help; /**< @brief Print help, including hidden options, and exit help description.  */
  const char *version_help; /**< @brief Print version and exit help description.  */
  char * input_file_arg;	/**< @brief Input file name.  */
  char * input_file_orig;	/**< @brief Input file name original value given at command line.  */
  const char *input_file_help; /**< @brief Input file name help description.  */
  #if defined(HAVE_LONG_LONG) || defined(HAVE_LONG_LONG_INT)
  long long int num_walks_arg;	/**< @brief Number of walk-on-spheres walks to perform.  */
  #else
  long num_walks_arg;	/**< @brief Number of walk-on-spheres walks to perform.  */
  #endif
  char * num_walks_orig;	/**< @brief Number of walk-on-spheres walks to perform original value given at command line.  */
  const char *num_walks_help; /**< @brief Number of walk-on-spheres walks to perform help description.  */
  #if defined(HAVE_LONG_LONG) || defined(HAVE_LONG_LONG_INT)
  long long int num_interior_samples_arg;	/**< @brief Number of interior samples to take.  */
  #else
  long num_interior_samples_arg;	/**< @brief Number of interior samples to take.  */
  #endif
  char * num_interior_samples_orig;	/**< @brief Number of interior samples to take original value given at command line.  */
  const char *num_interior_samples_help; /**< @brief Number of interior samples to take help description.  */
  double max_rsd_capacitance_arg;	/**< @brief Perform walk-on-spheres walks until the relative standard deviation of the capacitance drops below this value.  Relative standard deviation is defined as (Standard_Deviation/Mean)*100%.  */
  char * max_rsd_capacitance_orig;	/**< @brief Perform walk-on-spheres walks until the relative standard deviation of the capacitance drops below this value.  Relative standard deviation is defined as (Standard_Deviation/Mean)*100% original value given at command line.  */
  const char *max_rsd_capacitance_help; /**< @brief Perform walk-on-spheres walks until the relative standard deviation of the capacitance drops below this value.  Relative standard deviation is defined as (Standard_Deviation/Mean)*100% help description.  */
  double max_rsd_polarizability_arg;	/**< @brief Perform walk-on-spheres walks until the relative standard deviation of the mean electric polarizability drops below this value.  Relative standard deviation is defined as (Standard_Deviation/Mean)*100%.  */
  char * max_rsd_polarizability_orig;	/**< @brief Perform walk-on-spheres walks until the relative standard deviation of the mean electric polarizability drops below this value.  Relative standard deviation is defined as (Standard_Deviation/Mean)*100% original value given at command line.  */
  const char *max_rsd_polarizability_help; /**< @brief Perform walk-on-spheres walks until the relative standard deviation of the mean electric polarizability drops below this value.  Relative standard deviation is defined as (Standard_Deviation/Mean)*100% help description.  */
  double max_rsd_volume_arg;	/**< @brief Take interior samples until the relative standard deviation of volume drops below this value.  Relative standard deviation is defined as (Standard_Deviation/Mean)*100%.  */
  char * max_rsd_volume_orig;	/**< @brief Take interior samples until the relative standard deviation of volume drops below this value.  Relative standard deviation is defined as (Standard_Deviation/Mean)*100% original value given at command line.  */
  const char *max_rsd_volume_help; /**< @brief Take interior samples until the relative standard deviation of volume drops below this value.  Relative standard deviation is defined as (Standard_Deviation/Mean)*100% help description.  */
  #if defined(HAVE_LONG_LONG) || defined(HAVE_LONG_LONG_INT)
  long long int min_num_walks_arg;	/**< @brief Minimum number of walk-on-spheres walks to perform when using max-rsd stopping conditions (default=1000).  */
  #else
  long min_num_walks_arg;	/**< @brief Minimum number of walk-on-spheres walks to perform when using max-rsd stopping conditions (default=1000).  */
  #endif
  char * min_num_walks_orig;	/**< @brief Minimum number of walk-on-spheres walks to perform when using max-rsd stopping conditions original value given at command line.  */
  const char *min_num_walks_help; /**< @brief Minimum number of walk-on-spheres walks to perform when using max-rsd stopping conditions help description.  */
  #if defined(HAVE_LONG_LONG) || defined(HAVE_LONG_LONG_INT)
  long long int min_num_interior_samples_arg;	/**< @brief Minimum number of interior samples to take when using max-rsd stopping conditions (default=10000).  */
  #else
  long min_num_interior_samples_arg;	/**< @brief Minimum number of interior samples to take when using max-rsd stopping conditions (default=10000).  */
  #endif
  char * min_num_interior_samples_orig;	/**< @brief Minimum number of interior samples to take when using max-rsd stopping conditions original value given at command line.  */
  const char *min_num_interior_samples_help; /**< @brief Minimum number of interior samples to take when using max-rsd stopping conditions help description.  */
  const char *compute_form_help; /**< @brief Compute form factor help description.  */
  int num_threads_arg;	/**< @brief Number of threads to use  (default=Number of logical cores).  */
  char * num_threads_orig;	/**< @brief Number of threads to use  (default=Number of logical cores) original value given at command line.  */
  const char *num_threads_help; /**< @brief Number of threads to use  (default=Number of logical cores) help description.  */
  int seed_arg;	/**< @brief Seed for the random number generator  (default=Randomly set).  */
  char * seed_orig;	/**< @brief Seed for the random number generator  (default=Randomly set) original value given at command line.  */
  const char *seed_help; /**< @brief Seed for the random number generator  (default=Randomly set) help description.  */
  double frac_error_bound_arg;	/**< @brief Fractional error bound for nearest neighbor search (default='0').  */
  char * frac_error_bound_orig;	/**< @brief Fractional error bound for nearest neighbor search original value given at command line.  */
  const char *frac_error_bound_help; /**< @brief Fractional error bound for nearest neighbor search help description.  */
  char * surface_points_file_arg;	/**< @brief Name of file for writing the surface points from Walk-on-Spheres.  */
  char * surface_points_file_orig;	/**< @brief Name of file for writing the surface points from Walk-on-Spheres original value given at command line.  */
  const char *surface_points_file_help; /**< @brief Name of file for writing the surface points from Walk-on-Spheres help description.  */
  char * interior_points_file_arg;	/**< @brief Name of file for writing the interior sample points.  */
  char * interior_points_file_orig;	/**< @brief Name of file for writing the interior sample points original value given at command line.  */
  const char *interior_points_file_help; /**< @brief Name of file for writing the interior sample points help description.  */
  const char *print_counts_help; /**< @brief Print statistics related to counts of hit points help description.  */
  const char *print_benchmarks_help; /**< @brief Print detailed RAM and timing information help description.  */
  
  unsigned int help_given ;	/**< @brief Whether help was given.  */
  unsigned int full_help_given ;	/**< @brief Whether full-help was given.  */
  unsigned int version_given ;	/**< @brief Whether version was given.  */
  unsigned int input_file_given ;	/**< @brief Whether input-file was given.  */
  unsigned int num_walks_given ;	/**< @brief Whether num-walks was given.  */
  unsigned int num_interior_samples_given ;	/**< @brief Whether num-interior-samples was given.  */
  unsigned int max_rsd_capacitance_given ;	/**< @brief Whether max-rsd-capacitance was given.  */
  unsigned int max_rsd_polarizability_given ;	/**< @brief Whether max-rsd-polarizability was given.  */
  unsigned int max_rsd_volume_given ;	/**< @brief Whether max-rsd-volume was given.  */
  unsigned int min_num_walks_given ;	/**< @brief Whether min-num-walks was given.  */
  unsigned int min_num_interior_samples_given ;	/**< @brief Whether min-num-interior-samples was given.  */
  unsigned int compute_form_given ;	/**< @brief Whether compute-form was given.  */
  unsigned int num_threads_given ;	/**< @brief Whether num-threads was given.  */
  unsigned int seed_given ;	/**< @brief Whether seed was given.  */
  unsigned int frac_error_bound_given ;	/**< @brief Whether frac-error-bound was given.  */
  unsigned int surface_points_file_given ;	/**< @brief Whether surface-points-file was given.  */
  unsigned int interior_points_file_given ;	/**< @brief Whether interior-points-file was given.  */
  unsigned int print_counts_given ;	/**< @brief Whether print-counts was given.  */
  unsigned int print_benchmarks_given ;	/**< @brief Whether print-benchmarks was given.  */

} ;

/** @brief The additional parameters to pass to parser functions */
struct cmdline_parser_params
{
  int override; /**< @brief whether to override possibly already present options (default 0) */
  int initialize; /**< @brief whether to initialize the option structure gengetopt_args_info (default 1) */
  int check_required; /**< @brief whether to check that all required options were provided (default 1) */
  int check_ambiguity; /**< @brief whether to check for options already specified in the option structure gengetopt_args_info (default 0) */
  int print_errors; /**< @brief whether getopt_long should print an error message for a bad option (default 1) */
} ;

/** @brief the purpose string of the program */
extern const char *gengetopt_args_info_purpose;
/** @brief the usage string of the program */
extern const char *gengetopt_args_info_usage;
/** @brief the description string of the program */
extern const char *gengetopt_args_info_description;
/** @brief all the lines making the help output */
extern const char *gengetopt_args_info_help[];
/** @brief all the lines making the full help output (including hidden options) */
extern const char *gengetopt_args_info_full_help[];

/**
 * The command line parser
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser (int argc, char **argv,
  struct gengetopt_args_info *args_info);

/**
 * The command line parser (version with additional parameters - deprecated)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param override whether to override possibly already present options
 * @param initialize whether to initialize the option structure my_args_info
 * @param check_required whether to check that all required options were provided
 * @return 0 if everything went fine, NON 0 if an error took place
 * @deprecated use cmdline_parser_ext() instead
 */
int cmdline_parser2 (int argc, char **argv,
  struct gengetopt_args_info *args_info,
  int override, int initialize, int check_required);

/**
 * The command line parser (version with additional parameters)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param params additional parameters for the parser
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_ext (int argc, char **argv,
  struct gengetopt_args_info *args_info,
  struct cmdline_parser_params *params);

/**
 * Save the contents of the option struct into an already open FILE stream.
 * @param outfile the stream where to dump options
 * @param args_info the option struct to dump
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_dump(FILE *outfile,
  struct gengetopt_args_info *args_info);

/**
 * Save the contents of the option struct into a (text) file.
 * This file can be read by the config file parser (if generated by gengetopt)
 * @param filename the file where to save
 * @param args_info the option struct to save
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_file_save(const char *filename,
  struct gengetopt_args_info *args_info);

/**
 * Print the help
 */
void cmdline_parser_print_help(void);
/**
 * Print the full help (including hidden options)
 */
void cmdline_parser_print_full_help(void);
/**
 * Print the version
 */
void cmdline_parser_print_version(void);

/**
 * Initializes all the fields a cmdline_parser_params structure 
 * to their default values
 * @param params the structure to initialize
 */
void cmdline_parser_params_init(struct cmdline_parser_params *params);

/**
 * Allocates dynamically a cmdline_parser_params structure and initializes
 * all its fields to their default values
 * @return the created and initialized cmdline_parser_params structure
 */
struct cmdline_parser_params *cmdline_parser_params_create(void);

/**
 * Initializes the passed gengetopt_args_info structure's fields
 * (also set default values for options that have a default)
 * @param args_info the structure to initialize
 */
void cmdline_parser_init (struct gengetopt_args_info *args_info);
/**
 * Deallocates the string fields of the gengetopt_args_info structure
 * (but does not deallocate the structure itself)
 * @param args_info the structure to deallocate
 */
void cmdline_parser_free (struct gengetopt_args_info *args_info);

/**
 * Checks that all the required options were specified
 * @param args_info the structure to check
 * @param prog_name the name of the program that will be used to print
 *   possible errors
 * @return
 */
int cmdline_parser_required (struct gengetopt_args_info *args_info,
  const char *prog_name);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CMDLINE_H */

// *** END *** cmdline.h ***

// *** START *** RandomNumber/Rand.h ***
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
// Date:    Mon Dec 29 11:44:20 2014 EDT
//
// Time-stamp: <2015-05-18 14:40:04 derekj>
//
// ================================================================

#ifndef RAND_H_
#define RAND_H_

// ================================================================

class Rand
{
public:
  Rand(int streamNum, int numStreams, int seed);
  ~Rand();

  double getRandIn01();
  double getRandInRange(double min, double max);
};
// ================================================================

#endif  // #ifndef RAND_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** RandomNumber/Rand.h ***

#endif // US_ZENO_CXX_H
