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

#if defined( WIN32 )
#define M_PI       3.14159265358979323846
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

		inline void init() { clear(); }
		inline void clear() { m_indices_dists.clear(); }

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
			clear();
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

// *** START *** Geometry/Vector3.h ***
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
// Date:    Tue Apr 16 12:20:59 2013 EDT
//
// Time-stamp: <2015-10-26 11:53:14 dcj>
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
  eleT invSqrt(eleT x);

  eleT components[3];
};

template <class eleT>
std::ostream & operator<<(std::ostream & os, const Vector3<eleT> & rhs)
{
  os << "<" << rhs.getX() << ", " << rhs.getY() << ", " << rhs.getZ() << ">";

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

// *** START *** Geometry/Sphere.h ***
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
// Date:    Thu Nov 13 12:27:37 2014 EDT
//
// Time-stamp: <2015-05-28 11:23:11 derekj>
//
// ================================================================

#ifndef SPHERE_H
#define SPHERE_H

#include <cmath>
#include <ostream>

// inlined #include "Vector3.h"

// ================================================================

template <class T>
class Sphere {
public:
  Sphere();
  Sphere(Sphere<T> const & b);
  Sphere(Vector3<T> const & center, T radius);

  Vector3<T> getCenter() const;

  T getRadiusSqr() const;
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
Vector3<T> 
Sphere<T>::getCenter() const {
  return center;
}

template <class T>
T 
Sphere<T>::getRadiusSqr() const {
  return radius*radius;
}

template <class T>
T 
Sphere<T>::getRadius() const {
  return radius;
}

template <class T>
T 
Sphere<T>::getMaxCoord(int dim) const {
  return center.get(dim) + radius;
}

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

// *** START *** SpherePoint/QuasiRandomSpherePointsRegularCube.h ***
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
// Date:    Thu Feb 12 17:24:57 2015 EDT
//
// Time-stamp: <2015-04-20 14:46:56 derekj>
//
// ================================================================

#ifndef QUASI_RANDOM_SPHERE_POINTS_REGULAR_CUBE_H
#define QUASI_RANDOM_SPHERE_POINTS_REGULAR_CUBE_H

#include <vector>
#include <cmath>

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

template <class T>
class QuasiRandomSpherePointsRegularCube {
 public:
  static void generate(Sphere<T> const * sphere,
		       int minNumPoints,
		       std::vector<Vector3<T> > & points);
};

template <class T>
void
QuasiRandomSpherePointsRegularCube<T>::
  generate(Sphere<T> const * sphere,
	   int minNumPoints,
	   std::vector<Vector3<T> > & points) {

  points.clear();

  //generate points on unit cube centered at origin

  double pointsPerFace = minNumPoints / 6.;
  double pointsPerEdge = ceil(sqrt(pointsPerFace));

  double pointSpacing = 1 / pointsPerEdge;

  for (double u = (pointSpacing / 2) - 0.5; u < 0.5; u += pointSpacing) {
    for (double v = (pointSpacing / 2) - 0.5; v < 0.5; v += pointSpacing) {

      points.push_back(Vector3<T>(u, v, -0.5));
      points.push_back(Vector3<T>(u, v,  0.5));
      points.push_back(Vector3<T>(u, -0.5, v));
      points.push_back(Vector3<T>(u,  0.5, v));
      points.push_back(Vector3<T>(-0.5, u, v));
      points.push_back(Vector3<T>( 0.5, u, v));
    }
  }

  for (typename std::vector<Vector3<T> >::iterator pointIt = points.begin(); 
       pointIt != points.end();
       ++pointIt) {

    //project point onto unit sphere centered at origin 

    pointIt->normalize();

    //move point onto requested sphere

    (*pointIt) *= sphere->getRadius();
    (*pointIt) += sphere->getCenter();
  }
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** SpherePoint/QuasiRandomSpherePointsRegularCube.h ***

// *** START *** PointModel/NanoFLANNPointDatasetAdaptor.h ***
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
// Date:    Wed Nov 05 15:29:00 2014 EDT
//
// Time-stamp: <2015-08-31 17:30:36 dcj>
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

template <typename DistanceType, typename ComponentType>
class NanoFLANNDatasetAdaptor {
 public:
  NanoFLANNDatasetAdaptor(std::vector<Vector3<ComponentType> > const * 
			  samplePoints);

  ~NanoFLANNDatasetAdaptor();

  // Must return the number of data points
  size_t kdtree_get_point_count() const;

  // Must return the Euclidean (L2) distance between the vector "p1[0:size-1]"
  // and the data point with index "idx_p2" stored in the class:
  DistanceType kdtree_distance(const ComponentType *p1, const size_t idx_p2, 
			       size_t size) const;

  // Must return the dim'th component of the idx'th point in the class:
  ComponentType kdtree_get_pt(const size_t idx, int dim) const;

  // Optional bounding-box computation: return false to default to a standard 
  // bbox computation loop.
  // Return true if the BBOX was already computed by the class and returned in
  // "bb" so it can be avoided to redo it again.
  // Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 
  // for point clouds)
  template <class BBOX>
  bool kdtree_get_bbox(BBOX &bb) const;

 private:
  int nPts;

  ComponentType * points;
};

template <typename DistanceType, typename ComponentType>
NanoFLANNDatasetAdaptor<DistanceType, ComponentType>::
NanoFLANNDatasetAdaptor(std::vector<Vector3<ComponentType> > const * 
			samplePoints)
  : nPts(0),
    points(NULL) {

  nPts = samplePoints->size();

  points = new ComponentType[nPts * 3];

  assert(points != NULL);

  for (int i = 0; i < nPts; i++) {
    for (int dim = 0; dim < 3; dim++) {
      points[i*3 + dim] = samplePoints->at(i).get(dim);
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
  kdtree_get_bbox(BBOX &bb) const {

  return false;
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** PointModel/NanoFLANNPointDatasetAdaptor.h ***

// *** START *** PointModel/NanoFLANNPoint.h ***
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
// Date:    Fri Aug 15 14:35:03 2014 EDT
//
// Time-stamp: <2015-08-31 15:05:08 dcj>
//
// ================================================================

#ifndef NANO_FLANN_POINT_H
#define NANO_FLANN_POINT_H

#include <vector>

// #include <nanoflann.hpp>

// inlined #include "NanoFLANNPointDatasetAdaptor.h"

// inlined #include "../Geometry/Vector3.h"

// ================================================================

class NanoFLANNPoint {
 public:
  NanoFLANNPoint();

  ~NanoFLANNPoint();

  void preprocess(std::vector<Vector3<double> > const * points,
		  std::vector<Vector3<double> > const * normals,
		  double fracErrorBound);

  void findNearestPoint(Vector3<double> const * queryPoint,
			double fracErrorBound,
			Vector3<double> & nearestPoint,
			Vector3<double> & nearestPointNormal) const;

  void printDataStructureStats() const;
  void printSearchStats() const;

 private:
  typedef NanoFLANNDatasetAdaptor<double, double> DatasetAdaptorType;

  typedef nanoflann::L2_Simple_Adaptor<double, DatasetAdaptorType> MetricType;

  typedef nanoflann::KDTreeSingleIndexAdaptor<MetricType, 
                                              DatasetAdaptorType, 3, int> 
    KDTreeType;

  struct NanoFLANNInstance {
    DatasetAdaptorType * dataset;
    KDTreeType * kdTree;
  };

  const std::vector<Vector3<double> > * originalPoints;
  const std::vector<Vector3<double> > * originalNormals;

  NanoFLANNInstance nanoFLANNInstance;
};

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** PointModel/NanoFLANNPoint.h ***

