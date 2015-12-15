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

// *** START *** Scannerbase.h ***
// Generated by Flexc++ V1.08.00 on Tue, 20 Oct 2015 17:44:37 -0400

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



    int     const (*d_dfaBase__)[37];

    static int     const s_dfa__[][37];
    static int     const (*s_dfaBase__[])[37];
    enum: bool { s_interactive__ = false };
    enum: size_t {
        s_rangeOfEOF__           = 34,
        s_finacIdx__             = 35,
        s_nRules__               = 9,
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

inline void Scanner::postCode(PostEnum__ type) 
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

// *** START *** Parserbase.h ***
// Generated by Bisonc++ V4.05.00 on Tue, 20 Oct 2015 17:44:37 -0400

#ifndef ParserBase_h_included
#define ParserBase_h_included

#include <exception>
#include <vector>
#include <iostream>


namespace // anonymous
{
    struct PI__;
}



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
        STRING,
    };

// $insert STYPE
typedef double STYPE__;


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
        void ERROR() const;
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

inline void ParserBase::ERROR() const
{
    throw UNEXPECTED_TOKEN__;
}


// As a convenience, when including ParserBase.h its symbols are available as
// symbols in the class Parser, too.
#define Parser ParserBase


#endif



// *** END *** Parserbase.h ***

// *** START *** Parser.h ***
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
// Time-stamp: <2015-02-13 16:56:54 dcj>
//
// ================================================================

// Generated by Bisonc++ V4.04.01 on Wed, 19 Feb 2014 11:59:38 -0500

#ifndef Parser_h_included
#define Parser_h_included

#include <vector>

// inlined #include "Geometry/Sphere.h"

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

    std::vector<Sphere<double> > * spheres;

    public:
        Parser(std::istream &in, std::vector<Sphere<double> > * spheres);

        int parse();

    private:
        void error(char const *msg);    // called on (syntax) errors
        int lex();                      // returns the next token from the
                                        // lexical scanner. 
        void print();                   // use, e.g., d_token, d_loc

	void addSphere(double x, double y, double z, double r);

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

inline void Parser::exceptionHandler__(std::exception const &exc)         
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
#define CMDLINE_PARSER_PACKAGE "ZenoC++"
#endif

#ifndef CMDLINE_PARSER_PACKAGE_NAME
/** @brief the complete program name (used for help and version) */
#define CMDLINE_PARSER_PACKAGE_NAME "ZenoC++"
#endif

#ifndef CMDLINE_PARSER_VERSION
/** @brief the program version */
#define CMDLINE_PARSER_VERSION "0.1"
#endif

enum enum_input_primitive { input_primitive__NULL = -1, input_primitive_arg_sphere = 0, input_primitive_arg_voxel };

/** @brief Where the command line options are stored */
struct gengetopt_args_info
{
  const char *help_help; /**< @brief Print help and exit help description.  */
  const char *version_help; /**< @brief Print version and exit help description.  */
  char * input_file_arg;	/**< @brief Input file name.  */
  char * input_file_orig;	/**< @brief Input file name original value given at command line.  */
  const char *input_file_help; /**< @brief Input file name help description.  */
  enum enum_input_primitive input_primitive_arg;	/**< @brief Type of input primitive (default='sphere').  */
  char * input_primitive_orig;	/**< @brief Type of input primitive original value given at command line.  */
  const char *input_primitive_help; /**< @brief Type of input primitive help description.  */
  int num_walks_arg;	/**< @brief Number of walks.  */
  char * num_walks_orig;	/**< @brief Number of walks original value given at command line.  */
  const char *num_walks_help; /**< @brief Number of walks help description.  */
  int num_interior_samples_arg;	/**< @brief Number of sample points for volume computation  (default=num-walks).  */
  char * num_interior_samples_orig;	/**< @brief Number of sample points for volume computation  (default=num-walks) original value given at command line.  */
  const char *num_interior_samples_help; /**< @brief Number of sample points for volume computation  (default=num-walks) help description.  */
  int num_threads_arg;	/**< @brief Number of threads (default='1').  */
  char * num_threads_orig;	/**< @brief Number of threads original value given at command line.  */
  const char *num_threads_help; /**< @brief Number of threads help description.  */
  int seed_arg;	/**< @brief Seed for the random number generator (default='0').  */
  char * seed_orig;	/**< @brief Seed for the random number generator original value given at command line.  */
  const char *seed_help; /**< @brief Seed for the random number generator help description.  */
  double frac_error_bound_arg;	/**< @brief Fractional error bound for nearest neighbor search (default='0').  */
  char * frac_error_bound_orig;	/**< @brief Fractional error bound for nearest neighbor search original value given at command line.  */
  const char *frac_error_bound_help; /**< @brief Fractional error bound for nearest neighbor search help description.  */
  double shell_thickness_arg;	/**< @brief Thickness of shell for absorbing walkers  (default=LaunchSphereRadius * 10^-6).  */
  char * shell_thickness_orig;	/**< @brief Thickness of shell for absorbing walkers  (default=LaunchSphereRadius * 10^-6) original value given at command line.  */
  const char *shell_thickness_help; /**< @brief Thickness of shell for absorbing walkers  (default=LaunchSphereRadius * 10^-6) help description.  */
  char * surface_points_file_arg;	/**< @brief Name of file into which to write the surface points from the Exterior problem.  */
  char * surface_points_file_orig;	/**< @brief Name of file into which to write the surface points from the Exterior problem original value given at command line.  */
  const char *surface_points_file_help; /**< @brief Name of file into which to write the surface points from the Exterior problem help description.  */
  const char *print_counts_help; /**< @brief Print statistics related to counts of hit points help description.  */
  const char *print_benchmarks_help; /**< @brief Print detailed RAM and timing information help description.  */
  
  unsigned int help_given ;	/**< @brief Whether help was given.  */
  unsigned int version_given ;	/**< @brief Whether version was given.  */
  unsigned int input_file_given ;	/**< @brief Whether input-file was given.  */
  unsigned int input_primitive_given ;	/**< @brief Whether input-primitive was given.  */
  unsigned int num_walks_given ;	/**< @brief Whether num-walks was given.  */
  unsigned int num_interior_samples_given ;	/**< @brief Whether num-interior-samples was given.  */
  unsigned int num_threads_given ;	/**< @brief Whether num-threads was given.  */
  unsigned int seed_given ;	/**< @brief Whether seed was given.  */
  unsigned int frac_error_bound_given ;	/**< @brief Whether frac-error-bound was given.  */
  unsigned int shell_thickness_given ;	/**< @brief Whether shell-thickness was given.  */
  unsigned int surface_points_file_given ;	/**< @brief Whether surface-points-file was given.  */
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
 * The config file parser (deprecated version)
 * @param filename the name of the config file
 * @param args_info the structure where option information will be stored
 * @param override whether to override possibly already present options
 * @param initialize whether to initialize the option structure my_args_info
 * @param check_required whether to check that all required options were provided
 * @return 0 if everything went fine, NON 0 if an error took place
 * @deprecated use cmdline_parser_config_file() instead
 */
int cmdline_parser_configfile (const char *filename,
  struct gengetopt_args_info *args_info,
  int override, int initialize, int check_required);

/**
 * The config file parser
 * @param filename the name of the config file
 * @param args_info the structure where option information will be stored
 * @param params additional parameters for the parser
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_config_file (const char *filename,
  struct gengetopt_args_info *args_info,
  struct cmdline_parser_params *params);

/**
 * Checks that all the required options were specified
 * @param args_info the structure to check
 * @param prog_name the name of the program that will be used to print
 *   possible errors
 * @return
 */
int cmdline_parser_required (struct gengetopt_args_info *args_info,
  const char *prog_name);

extern const char *cmdline_parser_input_primitive_values[];  /**< @brief Possible values for input-primitive. */


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CMDLINE_H */

// *** END *** cmdline.h ***

// *** START *** Geometry/Matrix3x3.h ***
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
// Time-stamp: <2015-09-03 18:17:43 dcj>
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
  void cubicsolver(eleT coeff[4], 
		   eleT roots[3]) const;

  eleT components[3*3];
};

template <class eleT>
std::ostream & operator<<(std::ostream & os, const Matrix3x3<eleT> & rhs)
{
  os << "[" 
     << rhs.get(0, 0) << ", " << rhs.get(0, 1) << ", " << rhs.get(0, 2) << ","
     << std::endl
     << " "
     << rhs.get(1, 0) << ", " << rhs.get(1, 1) << ", " << rhs.get(1, 2) << ","
     << std::endl
     << " "
     << rhs.get(2, 0) << ", " << rhs.get(2, 1) << ", " << rhs.get(2, 2) << "]"
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
      eleT average = (get(row, col) + get(col, row)) / 2;

      set(row, col, average);
      set(col, row, average);
    }
  }
}

template <class eleT>
void Matrix3x3<eleT>::getEigenValues(Vector3<eleT> & eigenValues) const
{
  eleT coeffs[4];
  eleT roots[3];

  assert(get(1, 0) == get(0, 1));
  assert(get(2, 0) == get(0, 2));
  assert(get(2, 1) == get(1, 2));

  // compute Rg tensor
  eleT Sxx=get(0, 0);
  eleT Syy=get(1, 1);
  eleT Szz=get(2, 2);
  eleT Sxy=get(0, 1);
  eleT Sxz=get(0, 2);
  eleT Syz=get(1, 2);

  // Compute coefficents
  coeffs[0]=1;
  coeffs[1]=-1.*(Sxx+Syy+Szz);
  coeffs[2]=Sxx*Syy+Sxx*Szz+Syy*Szz-Sxy*Sxy-Sxz*Sxz-Syz*Syz;
  coeffs[3]=Sxz*Sxz*Syy+Sxy*Sxy*Szz+Syz*Syz*Sxx-2.*Sxy*Sxz*Syz-Sxx*Syy*Szz;

  // Find eigenvalues
  cubicsolver(coeffs,roots);

  eigenValues.setXYZ(roots[0], roots[1], roots[2]);
}

template <class eleT>
void Matrix3x3<eleT>::cubicsolver(eleT coeff[4], 
				  eleT roots[3]) const
{
// solve the cubic equation and sort roots

  eleT delta0 = coeff[1]*coeff[1]-3.*coeff[0]*coeff[2];
  eleT delta1 = 2.*pow(coeff[1],3.)-9.*coeff[0]*coeff[1]*coeff[2]+27.*coeff[0]*coeff[0]*coeff[3];

  eleT ththeta = pow(delta0,3.)-pow(delta1,2.)/4.;
  if(ththeta < 1e-15) {ththeta=0;}
  ththeta = atan2(sqrt(ththeta),delta1/2.)/3.;

  eleT cosv = cos(ththeta);
  eleT sinv = sin(ththeta);

  roots[0] = -1./(3.*coeff[0])*(coeff[1]+2.*sqrt(delta0)*cosv);
  roots[1] = -1./(3.*coeff[0])*(coeff[1]+sqrt(delta0)*(-1.*cosv-sqrt(3.)*sinv));
  roots[2] = -1./(3.*coeff[0])*(coeff[1]+sqrt(delta0)*(-1.*cosv+sqrt(3.)*sinv));

  // sort the roots
  if(roots[1]>roots[0]) 
    std::swap(roots[0],roots[1]);
  if(roots[2]>roots[0]) 
    std::swap(roots[0],roots[2]);
  if(roots[2]>roots[1]) 
    std::swap(roots[1],roots[2]);
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
    components[i] = (eleT)rhs.components[i];
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

// *** START *** Results.h ***
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
// Date:    Wed Apr 22 11:11:48 2015 EDT
//
// Time-stamp: <2015-10-26 18:20:00 dcj>
//
// ================================================================

#ifndef RESULTS_H_
#define RESULTS_H_

// ================================================================

#ifdef USE_MPI
#include <mpi.h>
#endif

#include <iostream>
#include <algorithm>
#include <cmath>
#include <cassert>

// inlined #include "Geometry/Vector3.h"
// inlined #include "Geometry/Matrix3x3.h"
// inlined #include "Geometry/Sphere.h"

// ================================================================

template <class RandomNumberGenerator>
class Results {
public:
  Results(RandomNumberGenerator * randomNumberGenerator,
	  Sphere<double> const * boundingSphere,
	  int numThreads,
	  bool saveHitPoints);

  ~Results();

  void recordHit(int threadNum,
		 Vector3<double> const * startPoint,
		 Vector3<double> const * endPoint,
		 Vector3<double> const * endPointNormal);

  void recordMiss(int threadNum);

  void reduce();

  std::vector<Vector3<double> > const * getPoints() const;
  std::vector<Vector3<double> > const * getNormals() const;
  std::vector<Vector3<char> > const * getCharges() const;

  void print(bool printCounts, double volume) const;

private:
  double computePadeApproximant(Matrix3x3<double> const * polarizabilityTensor)
    const;

  double computeIntrinsicViscosity(double padeApproximant,
				   double volume,
				   double t,
				   Vector3<double> const * u,
				   Matrix3x3<double> const * v,
				   Matrix3x3<double> const * w) const;

  double computeViscometricRadius(double meanPolarizability,
				  double padeApproximant) const;

  double padeApproximant(double x1, double x2) const;

  void reduceHitPoints();

  RandomNumberGenerator * randomNumberGenerator;

  double const boundingSphereRadius;
  Vector3<double> const boundingSphereCenter;

  int const numThreads;

  bool saveHitPoints;

  double * numWalks;

  Vector3<double> * KPlus;
  Vector3<double> * KMinus;

  Matrix3x3<double> * VPlus;
  Matrix3x3<double> * VMinus;

  double numWalksReduced;

  Vector3<double> KPlusReduced;
  Vector3<double> KMinusReduced;

  Matrix3x3<double> VPlusReduced;
  Matrix3x3<double> VMinusReduced;

  std::vector<Vector3<double> > * points;
  std::vector<Vector3<double> > * normals;
  std::vector<Vector3<char> > * charges;

  std::vector<Vector3<double> > reducedPoints;
  std::vector<Vector3<double> > reducedNormals;
  std::vector<Vector3<char> > reducedCharges;

  bool reduced;
};

template <class RandomNumberGenerator>
Results<RandomNumberGenerator>::
Results(RandomNumberGenerator * randomNumberGenerator,
	Sphere<double> const * boundingSphere,
	int numThreads,
	bool saveHitPoints) 
  : randomNumberGenerator(randomNumberGenerator),
    boundingSphereRadius(boundingSphere->getRadius()),
    boundingSphereCenter(boundingSphere->getCenter()),
    numThreads(numThreads),
    saveHitPoints(saveHitPoints),
    numWalks(NULL),
    KPlus(NULL),
    KMinus(NULL),
    VPlus(NULL),
    VMinus(NULL),
    numWalksReduced(0),
    KPlusReduced(0, 0, 0),
    KMinusReduced(0, 0, 0),
    VPlusReduced(0, 0, 0, 0, 0, 0, 0, 0, 0),
    VMinusReduced(0, 0, 0, 0, 0, 0, 0, 0, 0),
    points(NULL),
    normals(NULL),
    charges(NULL),
    reducedPoints(),
    reducedNormals(),
    reducedCharges(),
    reduced(true) {

  numWalks = new double[numThreads];

  KPlus  = new Vector3<double>[numThreads];
  KMinus = new Vector3<double>[numThreads];

  VPlus  = new Matrix3x3<double>[numThreads];
  VMinus = new Matrix3x3<double>[numThreads];

  for (int threadNum = 0; threadNum < numThreads; threadNum++) {
    numWalks[threadNum] = 0;

    KPlus[threadNum].setXYZ(0, 0, 0);
    KMinus[threadNum].setXYZ(0, 0, 0);

    for (int component = 0; component < 3*3; component++) {
      VPlus[threadNum].set(component, 0);
      VMinus[threadNum].set(component, 0);
    }
  }

  points  = new std::vector<Vector3<double> >[numThreads];
  normals = new std::vector<Vector3<double> >[numThreads];
  charges = new std::vector<Vector3<char> >[numThreads];
}

template <class RandomNumberGenerator>
Results<RandomNumberGenerator>::
~Results() {
  delete [] numWalks;

  delete [] KPlus;
  delete [] KMinus;

  delete [] VPlus;
  delete [] VMinus;

  delete [] points;
  delete [] normals;
  delete [] charges;
}

template <class RandomNumberGenerator>
void 
Results<RandomNumberGenerator>::
recordHit(int threadNum,
	  Vector3<double> const * startPoint,
	  Vector3<double> const * endPoint,
	  Vector3<double> const * endPointNormal) {

  assert(threadNum >= 0 && threadNum < numThreads);

  reduced = false;

  Vector3<double> normalizedStartPoint = *startPoint - boundingSphereCenter;
  Vector3<double> normalizedEndPoint   = *endPoint - boundingSphereCenter;

  Vector3<char> walkCharges;

  numWalks[threadNum]++;

  for (int dim = 0; dim < 3; dim++) {
    double probability = 
      0.5 + normalizedStartPoint.get(dim)/(2*boundingSphereRadius);

    if (probability > randomNumberGenerator->getRandIn01()) {
      //c[dim] == +1

      walkCharges.set(dim, '+');

      KPlus[threadNum].add(dim, 1);
      VPlus[threadNum].addRow(dim, normalizedEndPoint);
    }
    else {
      //c[dim] == -1

      walkCharges.set(dim, '-');

      KMinus[threadNum].add(dim, 1);
      VMinus[threadNum].addRow(dim, normalizedEndPoint);
    }
  }

  points[threadNum].push_back(*endPoint);
  normals[threadNum].push_back(*endPointNormal);
  charges[threadNum].push_back(walkCharges);
}

template <class RandomNumberGenerator>
void 
Results<RandomNumberGenerator>::
recordMiss(int threadNum) {
  assert(threadNum >= 0 && threadNum < numThreads);

  reduced = false;

  numWalks[threadNum]++;
}

template <class RandomNumberGenerator>
void 
Results<RandomNumberGenerator>::
reduce() {
  numWalksReduced = 0;

  KPlusReduced.setXYZ(0, 0, 0);
  KMinusReduced.setXYZ(0, 0, 0);

  for (int component = 0; component < 3*3; component++) {
    VPlusReduced.set(component, 0);
    VMinusReduced.set(component, 0);
  }

  for (int threadNum = 0; threadNum < numThreads; threadNum++) {
    numWalksReduced += numWalks[threadNum];

    KPlusReduced  += KPlus[threadNum];
    KMinusReduced += KMinus[threadNum];

    VPlusReduced  += VPlus[threadNum];
    VMinusReduced += VMinus[threadNum];
  }

#ifdef USE_MPI
  double sendbuf[25];

  int offset = 0;

  sendbuf[offset++] = numWalksReduced;

  for (int i = 0; i < 3; i++) {
    sendbuf[offset++] = KPlusReduced.get(i);
  }

  for (int i = 0; i < 3; i++) {
    sendbuf[offset++] = KMinusReduced.get(i);
  }

  for (int i = 0; i < 9; i++) {
    sendbuf[offset++] = VPlusReduced.get(i);
  }

  for (int i = 0; i < 9; i++) {
    sendbuf[offset++] = VMinusReduced.get(i);
  }

  double recvbuf[25];

  for (int i = 0; i < 25; i++) {
    recvbuf[i] = 0;
  }

  MPI_Reduce(sendbuf, recvbuf, 25, MPI_DOUBLE,
	     MPI_SUM, 0, MPI_COMM_WORLD);

  offset = 0;

  numWalksReduced = recvbuf[offset++];

  for (int i = 0; i < 3; i++) {
    KPlusReduced.set(i, recvbuf[offset++]);
  }

  for (int i = 0; i < 3; i++) {
    KMinusReduced.set(i, recvbuf[offset++]);
  }

  for (int i = 0; i < 9; i++) {
    VPlusReduced.set(i, recvbuf[offset++]);
  }

  for (int i = 0; i < 9; i++) {
    VMinusReduced.set(i, recvbuf[offset++]);
  }
#endif

  if (saveHitPoints) {
    reduceHitPoints();
  }

  reduced = true;
}

template <class RandomNumberGenerator>
void 
Results<RandomNumberGenerator>::
reduceHitPoints() {
  for (int threadNum = 0; threadNum < numThreads; threadNum++) {
    reducedPoints.insert(reducedPoints.end(), 
			 points[threadNum].begin(), 
			 points[threadNum].end());

    reducedNormals.insert(reducedNormals.end(),
			  normals[threadNum].begin(),
			  normals[threadNum].end());

    reducedCharges.insert(reducedCharges.end(),
			  charges[threadNum].begin(),
			  charges[threadNum].end());
  }

#ifdef USE_MPI
  int mpiSize = 0;
  int mpiRank = 0;

  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);

  int * arrayLengths = new int[mpiSize];

  int arrayLength = reducedPoints.size() * 3;

  MPI_Allgather(&arrayLength, 1, MPI_INT,
		arrayLengths, 1, MPI_INT,
		MPI_COMM_WORLD);

  int combinedArrayLength = 0;

  for (int i = 0; i < mpiSize; i++) {
    combinedArrayLength += arrayLengths[i];
  }

  double * combinedPointArray  = new double[combinedArrayLength];
  double * combinedNormalArray = new double[combinedArrayLength];
  char *   combinedChargeArray = new char[combinedArrayLength];

  int * combinedArrayOffsets = new int[mpiSize];

  combinedArrayOffsets[0] = 0;

  for (int i = 1; i < mpiSize; i++) {
    combinedArrayOffsets[i] = combinedArrayOffsets[i - 1] + arrayLengths[i - 1];
  }

  double * pointArray  = new double[arrayLengths[mpiRank]];
  double * normalArray = new double[arrayLengths[mpiRank]];
  char *   chargeArray = new char[arrayLengths[mpiRank]];

  for (int index = 0; index < arrayLengths[mpiRank]; index++) {
    pointArray[index]  = reducedPoints[index / 3].get(index % 3);
    normalArray[index] = reducedNormals[index / 3].get(index % 3);
    chargeArray[index] = reducedCharges[index / 3].get(index % 3);
  }

  MPI_Allgatherv(pointArray, 
		 arrayLengths[mpiRank], MPI_DOUBLE,
		 combinedPointArray, 
		 arrayLengths, combinedArrayOffsets, MPI_DOUBLE,
		 MPI_COMM_WORLD);

  MPI_Allgatherv(normalArray, 
		 arrayLengths[mpiRank], MPI_DOUBLE,
		 combinedNormalArray, 
		 arrayLengths, combinedArrayOffsets, MPI_DOUBLE,
		 MPI_COMM_WORLD);

  MPI_Allgatherv(chargeArray,
		 arrayLengths[mpiRank], MPI_BYTE,
		 combinedChargeArray,
		 arrayLengths, combinedArrayOffsets, MPI_BYTE,
		 MPI_COMM_WORLD);

  reducedPoints.clear();
  reducedNormals.clear();
  reducedCharges.clear();

  reducedPoints.reserve(combinedArrayLength / 3);
  reducedNormals.reserve(combinedArrayLength / 3);
  reducedCharges.reserve(combinedArrayLength / 3);

  for (int index = 0; index < combinedArrayLength; index += 3) {
    reducedPoints.emplace_back(combinedPointArray[index + 0],
			       combinedPointArray[index + 1],
			       combinedPointArray[index + 2]);

    reducedNormals.emplace_back(combinedNormalArray[index + 0],
				combinedNormalArray[index + 1],
				combinedNormalArray[index + 2]);

    reducedCharges.emplace_back(combinedChargeArray[index + 0],
				combinedChargeArray[index + 1],
				combinedChargeArray[index + 2]);
  }

  delete [] arrayLengths;

  delete [] combinedPointArray;
  delete [] combinedNormalArray;
  delete [] combinedChargeArray;

  delete [] combinedArrayOffsets;

  delete [] pointArray;
  delete [] normalArray;
  delete [] chargeArray;
#endif
}

template <class RandomNumberGenerator>
std::vector<Vector3<double> > const * 
Results<RandomNumberGenerator>::
getPoints() const {
  assert(saveHitPoints);
  assert(reduced);

  return &reducedPoints;
}

template <class RandomNumberGenerator>
std::vector<Vector3<double> > const * 
Results<RandomNumberGenerator>:: 
getNormals() const {
  assert(saveHitPoints);
  assert(reduced);

  return &reducedNormals;
}

template <class RandomNumberGenerator>
std::vector<Vector3<char> > const * 
Results<RandomNumberGenerator>:: 
getCharges() const {
  assert(saveHitPoints);
  assert(reduced);

  return &reducedCharges;
}

template <class RandomNumberGenerator>
void 
Results<RandomNumberGenerator>::
print(bool printCounts, double volume) const {

  assert(reduced);

  double t = (KPlusReduced.get(0) + KMinusReduced.get(0))/numWalksReduced;

  Vector3<double> u = (KPlusReduced - KMinusReduced)/numWalksReduced;

  Matrix3x3<double> v = (VPlusReduced + VMinusReduced)/numWalksReduced;

  Matrix3x3<double> w = (VPlusReduced - VMinusReduced)/numWalksReduced;

  double capacity = t * boundingSphereRadius;

  Matrix3x3<double> polarizabilityTensor;

  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      double element = 
	12*M_PI*pow(boundingSphereRadius, 2)*
	(w.get(row, col) - u.get(row)*v.get(row, col)/t);

      polarizabilityTensor.set(row, col, element);
    }
  }

  polarizabilityTensor.symmetrize();

  double meanPolarizability = 
    (polarizabilityTensor.get(0, 0) +
     polarizabilityTensor.get(1, 1) +
     polarizabilityTensor.get(2, 2)) / 3;

  Vector3<double> eigenValues;
  polarizabilityTensor.getEigenValues(eigenValues);

  double q_eta = computePadeApproximant(&polarizabilityTensor);

  double viscometricRadius = computeViscometricRadius(meanPolarizability,
						      q_eta);

  double intrinsicViscosity = 
    computeIntrinsicViscosity(q_eta, volume, t, &u, &v, &w);

  zeno_cxx_fout->ofs << std::scientific
	    << std::endl
	    << "Capacitance: " << capacity << std::endl
	    << std::endl
	    << "Electrostatic polarizability tensor: " << std::endl
	    << std::endl
	    << polarizabilityTensor 
	    << std::endl
	    << "Eigenvalues: " << eigenValues << std::endl
	    << "Mean electrostatic polarizability: " << meanPolarizability 
	    << std::endl
	    << std::endl
	    << "Volume: " << volume << std::endl
	    << std::endl
	    << "Pade approximant (q_eta): " << q_eta << std::endl
	    << "Viscometric radius:       " << viscometricRadius << std::endl
	    << "Intrinsic viscosity:      " << intrinsicViscosity << std::endl
	    << std::endl;

  if (printCounts) {
    zeno_cxx_fout->ofs << std::scientific
	      << "Counts:" << std::endl
	      << std::endl
	      << "t = " << t << std::endl
	      << std::endl
	      << "u = " << u << std::endl
	      << std::endl
	      << "v = " << std::endl << v
	      << std::endl
	      << "w = " << std::endl << w
	      << std::endl;
  }
}

template <class RandomNumberGenerator>
double 
Results<RandomNumberGenerator>::
computePadeApproximant(Matrix3x3<double> const * polarizabilityTensor)
  const {

  double alpha1 = polarizabilityTensor->get(0, 0);
  double alpha2 = polarizabilityTensor->get(1, 1);
  double alpha3 = polarizabilityTensor->get(2, 2);

  //sort
  if (alpha2 < alpha1) std::swap(alpha1, alpha2);
  if (alpha3 < alpha1) std::swap(alpha1, alpha3);
  if (alpha3 < alpha2) std::swap(alpha2, alpha3);

  double x1 = log(alpha2/alpha1);
  double x2 = log(alpha3/alpha2);

  double q_eta = padeApproximant(x1, x2);

  return q_eta;
}

template <class RandomNumberGenerator>
double 
Results<RandomNumberGenerator>::
computeViscometricRadius(double meanPolarizability,
			 double padeApproximant) const {

  double alpha = meanPolarizability;
  double q_eta = padeApproximant;

  double viscometricRadius = pow((3*q_eta*alpha)/(10*M_PI), 1./3.);

  return viscometricRadius;
}

template <class RandomNumberGenerator>
double 
Results<RandomNumberGenerator>::
computeIntrinsicViscosity(double padeApproximant,
			  double volume,
			  double t,
			  Vector3<double> const * u,
			  Matrix3x3<double> const * v,
			  Matrix3x3<double> const * w) const {

  double q_eta = padeApproximant;

  double eta = 0;

  for (int i = 0; i < 3; i++) {
    eta += q_eta * w->get(i, i) - u->get(i) * q_eta * v->get(i, i) / t;
  }

  eta *= 4*M_PI*pow(boundingSphereRadius, 2)/volume;

  return eta;
}

template <class RandomNumberGenerator>
double 
Results<RandomNumberGenerator>::
padeApproximant(double x1, double x2) const {
  const double delta_i[4] = {4.8,    0.66,  -1.247,  0.787};
  const double k_i[4]     = {0,      1.04,   2.012,  2.315};
  const double b_i[4]     = {0.68,  -7.399,  1.048,  0.136};
  const double t_i[4]     = {0,      1.063,  0.895,  4.993};
  const double B_i[4]     = {1.925, -8.611,  1.652, -0.120};
  const double q_i[4]     = {0,      1.344,  2.029,  1.075};
  const double c_i[4]     = {13.43,  16.17,  0.51,  -5.86};
  const double r_i[4]     = {0,      0.489,  0.879,  2.447};
  const double A_i[4]     = {16.23, -15.92,  14.83, -3.74};
  const double v_i[4]     = {0,      0.462,  1.989,  4.60};
  const double m_i[4]     = {2.786,  0.293, -0.11,   0.012};
  const double u_i[4]     = {0,      0.556,  2.034,  3.024};

  double delta = 0;
  double b     = 0;
  double B     = 0;
  double c     = 0;
  double A     = 0;
  double m     = 0;

  for (int i = 0; i < 4; i++) {
    delta += delta_i[i] * exp(-k_i[i] * x1);
    b     += b_i[i]     * exp(-t_i[i] * x1);
    B     += B_i[i]     * exp(-q_i[i] * x1);
    c     += c_i[i]     * exp(-r_i[i] * x1);
    A     += A_i[i]     * exp(-v_i[i] * x1);
    m     += m_i[i]     * exp(-u_i[i] * x1);
  }

  double q_eta = 
    (delta*A + c*x2 + b*pow(x2, 2) + 4*pow(x2, m)) /
    (6*A + 6*c*x2/delta + B*pow(x2, 2) + 5*pow(x2, m));

  return q_eta;
}

// ================================================================

#endif  // #ifndef RESULTS_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** Results.h ***

// *** START *** ResultsVolume.h ***
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
// Date:    Wed Apr 22 11:11:48 2015 EDT
//
// Time-stamp: <2015-05-26 13:47:15 derekj>
//
// ================================================================

#ifndef RESULTS_VOLUME_H_
#define RESULTS_VOLUME_H_

// ================================================================

class ResultsVolume {
public:
  ResultsVolume(int numThreads);

  ~ResultsVolume();

  void recordHit(int threadNum);

  void recordMiss(int threadNum);

  void reduce();

  double getVolume(double boundingShapeVolume) const;

private:
  int const numThreads;

  int * numHits;
  int * numMisses;

  int reducedNumHits;
  int reducedNumMisses;

  bool reduced;
};

// ================================================================

#endif  // #ifndef RESULTS_VOLUME_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** ResultsVolume.h ***

// *** START *** Geometry/Voxels.h ***
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
// Date:    Fri Sep 11 09:43:30 2015 EDT
//
// Time-stamp: <2015-10-26 13:07:38 dcj>
//
// ================================================================

#ifndef VOXELS_H
#define VOXELS_H

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <zlib.h>

// inlined #include "Vector3.h"

// ================================================================

template <class T>
class Voxels {
public:
  Voxels();
  ~Voxels();

  bool loadFitsGz(char const * inputFileName);

  void sampleSurface();

  unsigned short getData(int i, int j, int k) const;

  Vector3<int> const * getNumVoxels() const;
  Vector3<double> const * getVoxelDims() const;

  std::vector<Vector3<T> > const * getPoints() const;
  std::vector<Vector3<T> > const * getNormals() const;

  T getDiscRadius() const;

private:
  unsigned short reverseEndianness(unsigned short itemToReverse);

  bool surfaceSampled;

  Vector3<int> numVoxels;
  Vector3<double> voxelDims;

  unsigned short * data;

  std::vector<Vector3<T> > points;
  std::vector<Vector3<T> > normals;

  T discRadius;
};

template <class T>
Voxels<T>::Voxels() 
: surfaceSampled(true),
  numVoxels(),
  voxelDims(),
  data(NULL),
  points(),
  normals(),
  discRadius() {

}

template <class T>
Voxels<T>::~Voxels() {
  delete [] data;
}

template <class T>
bool 
Voxels<T>::loadFitsGz(char const * inputFileName) {

  gzFile inputFile = gzopen(inputFileName, "rb");

  if (inputFile == NULL) {
    printf("Error opening input fits file %s\n", inputFileName);
    return false;
  }

  int bitsPerPixel = 0;

  int numVoxelsArr[3];
  double voxelDimsArr[3];
  char unitsArr[3*81];

  int readError = 0;

  char fitsHeaderLine[81];
  readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  sscanf(fitsHeaderLine, "SIMPLE  = %*c");
  readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  sscanf(fitsHeaderLine, "BITPIX  = %d", &bitsPerPixel);
  readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  sscanf(fitsHeaderLine, "NAXIS   = %*d");
  readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  sscanf(fitsHeaderLine, "NAXIS1  = %d", numVoxelsArr + 0);
  readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  sscanf(fitsHeaderLine, "NAXIS2  = %d", numVoxelsArr + 1);
  readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  sscanf(fitsHeaderLine, "NAXIS3  = %d", numVoxelsArr + 2);
  readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  sscanf(fitsHeaderLine, "CDELT1  = %lf", voxelDimsArr + 0);
  readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  sscanf(fitsHeaderLine, "CDELT2  = %lf", voxelDimsArr + 1);
  readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  sscanf(fitsHeaderLine, "CDELT3  = %lf", voxelDimsArr + 2);
  readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  sscanf(fitsHeaderLine, "CTYPE1  = %s", unitsArr + 0*81);
  readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  sscanf(fitsHeaderLine, "CTYPE2  = %s", unitsArr + 1*81);
  readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  sscanf(fitsHeaderLine, "CTYPE3  = %s", unitsArr + 2*81);
  readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  sscanf(fitsHeaderLine, "END");

  numVoxels.setXYZ(numVoxelsArr[0],
		   numVoxelsArr[1],
		   numVoxelsArr[2]);

  voxelDims.setXYZ(voxelDimsArr[0],
		   voxelDimsArr[1],
		   voxelDimsArr[2]);

  for (int numHeaderLines = 13; numHeaderLines < 36; numHeaderLines++) {
    readError = readError || (gzgets(inputFile, fitsHeaderLine, 81) == NULL);
  }

  if (readError) {
    printf("Error reading input fits file %s\n", inputFileName);
    return false;
  }

  int numElements = numVoxelsArr[0] * numVoxelsArr[1] * numVoxelsArr[2];

  delete [] data;

  data = new unsigned short[numElements];

  if (data == NULL) {
    printf("Error allocating memory for output data\n");
    return false;
  }

  if (bitsPerPixel == 8) {
    unsigned char * inputData = new unsigned char[numElements];

    if (inputData == NULL) {
      printf("Error allocating memory for input data\n");
      return false;
    }

    int elementsRead = 
      gzread(inputFile, inputData, sizeof(unsigned char) * numElements);

    if (elementsRead != (int)sizeof(unsigned char) * numElements) {
      printf("Error reading input file %s\n", inputFileName);
      return false;
    }

    for (int i = 0; i < numElements; i++) {
      data[i] = inputData[i];
    }

    delete [] inputData;
  }
  else if (bitsPerPixel == 16) {
    unsigned short * inputData = new unsigned short[numElements];

    if (inputData == NULL) {
      printf("Error allocating memory for input data\n");
      return false;
    }

    int elementsRead = 
      gzread(inputFile, inputData, sizeof(unsigned short) * numElements);

    if (elementsRead != (int)sizeof(unsigned short) * numElements) {
      printf("Error reading input file %s\n", inputFileName);
      return false;
    }

    for (int i = 0; i < numElements; i++) {
      data[i] = reverseEndianness(inputData[i]);
    }

    delete [] inputData;
  }
  else {
    printf("Error: invalid number of bits per pixel %d\n", bitsPerPixel);
    return false;
  }

  gzclose(inputFile);

  surfaceSampled = false;

  return true;
}

template <class T>
unsigned short
Voxels<T>::reverseEndianness(unsigned short itemToReverse) {

  unsigned short reversedItem = 0;

  for (unsigned int byteIndex = 0; 
       byteIndex < sizeof(itemToReverse); 
       byteIndex++) {
    ((char *) &reversedItem)[byteIndex] = 
      ((char *) &itemToReverse)[sizeof(itemToReverse) - 1 - byteIndex];
  }

  return reversedItem;
}

template <class T>
void 
Voxels<T>::sampleSurface() {

  points.clear();
  normals.clear();

  discRadius = voxelDims.getMin() * sqrt(2);

  double maxSampleDistance = voxelDims.getMin();

  Vector3<T> pointSpacing
    (voxelDims.getI() / ceil(voxelDims.getI() / maxSampleDistance),
     voxelDims.getJ() / ceil(voxelDims.getJ() / maxSampleDistance),
     voxelDims.getK() / ceil(voxelDims.getK() / maxSampleDistance));

  std::vector<Vector3<T> > voxelPoints;

  for (int k = 0; k < numVoxels.getK(); k++) {
  for (int j = 0; j < numVoxels.getJ(); j++) {
  for (int i = 0; i < numVoxels.getI(); i++) {

    if (getData(i, j, k) != 0) {

      //generate points on unit cube with corner at origin

      if ((k + 1 >= numVoxels.getK()) || 
	  (getData(i, j, k + 1) == 0)) {

	for (T u = (pointSpacing.getI() / 2); 
	     u < 1; 
	     u += pointSpacing.getI()) {

	  for (T v = (pointSpacing.getJ() / 2); 
	       v < 1; 
	       v += pointSpacing.getJ()) {

	    voxelPoints.emplace_back(u, v, 1);
	    normals.emplace_back(0, 0,  1);
	  }
	}
      }

      if ((k - 1 < 0) || 
	  (getData(i, j, k - 1) == 0)) {

	for (T u = (pointSpacing.getI() / 2); 
	     u < 1; 
	     u += pointSpacing.getI()) {

	  for (T v = (pointSpacing.getJ() / 2); 
	       v < 1; 
	       v += pointSpacing.getJ()) {

	    voxelPoints.emplace_back(u, v, 0);
	    normals.emplace_back(0, 0, -1);
	  }
	}
      }

      if ((j + 1 >= numVoxels.getJ()) ||
	  (getData(i, j + 1, k) == 0)) {

	for (T u = (pointSpacing.getI() / 2); 
	     u < 1; 
	     u += pointSpacing.getI()) {

	  for (T v = (pointSpacing.getK() / 2); 
	       v < 1; 
	       v += pointSpacing.getK()) {

	    voxelPoints.emplace_back(u, 1, v);
	    normals.emplace_back(0,  1, 0);
	  }
	}
      }

      if ((j - 1 < 0) ||
	  (getData(i, j - 1, k) == 0)) {

	for (T u = (pointSpacing.getI() / 2); 
	     u < 1; 
	     u += pointSpacing.getI()) {

	  for (T v = (pointSpacing.getK() / 2); 
	       v < 1; 
	       v += pointSpacing.getK()) {

	    voxelPoints.emplace_back(u, 0, v);
	    normals.emplace_back(0, -1, 0);
	  }
	}
      }

      if ((i + 1 >= numVoxels.getI()) ||
	  (getData(i + 1, j, k) == 0)) {

	for (T u = (pointSpacing.getJ() / 2); 
	     u < 1; 
	     u += pointSpacing.getJ()) {

	  for (T v = (pointSpacing.getK() / 2); 
	       v < 1; 
	       v += pointSpacing.getK()) {

	    voxelPoints.emplace_back(1, u, v);
	    normals.emplace_back( 1, 0, 0);
	  }
	}
      }

      if ((i - 1 < 0) ||
	  (getData(i - 1, j, k) == 0)) {

	for (T u = (pointSpacing.getJ() / 2); 
	     u < 1; 
	     u += pointSpacing.getJ()) {

	  for (T v = (pointSpacing.getK() / 2); 
	       v < 1; 
	       v += pointSpacing.getK()) {

	    voxelPoints.emplace_back(0, u, v);
	    normals.emplace_back(-1, 0, 0);
	  }
	}
      }

      //move points to correct position

      for (typename std::vector<Vector3<T> >::iterator pointIt = 
	     voxelPoints.begin();
	   pointIt != voxelPoints.end();
	   ++pointIt) {

	(*pointIt) += Vector3<T>(i, j, k);
	(*pointIt) *= voxelDims;
      }

      points.insert(points.end(),
		    voxelPoints.begin(),
		    voxelPoints.end());

      voxelPoints.clear();
    }
  }
  }
  }

  surfaceSampled = true;
}

template <class T>
unsigned short 
Voxels<T>::getData(int i, int j, int k) 
const {

  assert(i >= 0 && i < numVoxels.getI() &&
	 j >= 0 && j < numVoxels.getJ() &&
	 k >= 0 && k < numVoxels.getK());

  int index =
    k * numVoxels.getJ() * numVoxels.getI() +
    j * numVoxels.getI() +
    i;

  return data[index];
}

template <class T>
Vector3<int> const * 
Voxels<T>::getNumVoxels() 
const {
  return &numVoxels;
}

template <class T>
Vector3<double> const * 
Voxels<T>::getVoxelDims() 
const {
  return &voxelDims;
}

template <class T>
std::vector<Vector3<T> > const * 
Voxels<T>::getPoints() 
const {
  assert(surfaceSampled);

  return &points;
}

template <class T>
std::vector<Vector3<T> > const * 
Voxels<T>::getNormals() 
const {
  assert(surfaceSampled);

  return &normals;
}

template <class T>
T 
Voxels<T>::getDiscRadius() 
const {
  assert(surfaceSampled);

  return discRadius;
}

// ================================================================

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** Geometry/Voxels.h ***

// *** START *** NearestSurfacePoint/PointFromDiscs.h ***
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
// Date:    Mon Aug 17 17:01:47 2015 EDT
//
// Time-stamp: <2015-10-26 14:28:22 dcj>
//
// ================================================================

#ifndef POINT_FROM_DISCS_H
#define POINT_FROM_DISCS_H

// inlined #include "../Geometry/Vector3.h"

// ================================================================

template <class DiscModel>
class PointFromDiscs
{
public:
  PointFromDiscs(DiscModel const * discModel);
  ~PointFromDiscs();

  void findNearestPoint(Vector3<double> const * queryPoint,
			double fracErrorBound,
			Vector3<double> & nearestPointNormal,
			double & distance) const;

private:
  DiscModel const * discModel;
};

template <class DiscModel>
PointFromDiscs<DiscModel>::
PointFromDiscs(DiscModel const * discModel) 
  : discModel(discModel) {
  
}

template <class DiscModel>
PointFromDiscs<DiscModel>::
~PointFromDiscs() {

}

template <class DiscModel>
void 
PointFromDiscs<DiscModel>::
findNearestPoint(Vector3<double> const * queryPoint,
		 double fracErrorBound, 
		 Vector3<double> & nearestPointNormal,
		 double & distance) const {

  Vector3<double> foundPoint;
  Vector3<double> foundPointNormal;

  discModel->findNearestPoint(queryPoint,
			      fracErrorBound,
			      foundPoint, 
			      foundPointNormal);

  double discRadius = discModel->getDiscRadius();

  double discHeight = (*queryPoint - foundPoint).dot(foundPointNormal);

  Vector3<double> displacement = 
    *queryPoint - (foundPoint + foundPointNormal * discHeight);

  double distanceOnDiscSqr = displacement.getMagnitudeSqr();

  if (distanceOnDiscSqr < pow(discRadius, 2)) {
    distance = discHeight;
  }
  else {
    Vector3<double> discEdge = 
      displacement.normalized() * discRadius + foundPoint;

    distance = (discEdge - *queryPoint).getMagnitude();
  }

  nearestPointNormal = foundPointNormal;
}

// ================================================================

#endif  // #ifndef POINT_FROM_DISCS_H

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** NearestSurfacePoint/PointFromDiscs.h ***

// *** START *** NearestSurfacePoint/PointFromPoints.h ***
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
// Date:    Mon Aug 17 17:01:47 2015 EDT
//
// Time-stamp: <2015-08-31 15:12:13 dcj>
//
// ================================================================

#ifndef POINT_FROM_POINTS_H
#define POINT_FROM_POINTS_H

// inlined #include "../Geometry/Vector3.h"

// ================================================================

template <class PointModel>
class PointFromPoints
{
public:
  PointFromPoints(PointModel const * pointModel);
  ~PointFromPoints();

  void findNearestPoint(Vector3<double> const * queryPoint,
			double fracErrorBound,
			Vector3<double> & nearestPointNormal,
			double & distance) const;

private:
  PointModel const * pointModel;
};

template <class PointModel>
PointFromPoints<PointModel>::
PointFromPoints(PointModel const * pointModel) 
  : pointModel(pointModel) {
  
}

template <class PointModel>
PointFromPoints<PointModel>::
~PointFromPoints() {

}

template <class PointModel>
void 
PointFromPoints<PointModel>::
findNearestPoint(Vector3<double> const * queryPoint,
		 double fracErrorBound, 
		 Vector3<double> & nearestPointNormal,
		 double & distance) const {

  Vector3<double> foundPoint;
  Vector3<double> foundPointNormal;

  pointModel->findNearestPoint(queryPoint,
			       fracErrorBound,
			       foundPoint, 
			       foundPointNormal);

  nearestPointNormal = foundPointNormal;

  distance = (foundPoint - *queryPoint).getMagnitude();
}

// ================================================================

#endif  // #ifndef POINT_FROM_POINTS_H

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** NearestSurfacePoint/PointFromPoints.h ***

// *** START *** NearestSurfacePoint/PointFromSphereCenters.h ***
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
// Date:    Mon Aug 17 17:01:47 2015 EDT
//
// Time-stamp: <2015-08-24 15:41:13 dcj>
//
// ================================================================

#ifndef POINT_FROM_SPHERE_CENTERS_H
#define POINT_FROM_SPHERE_CENTERS_H

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

// ================================================================

template <class SphereCenterModel>
class PointFromSphereCenters
{
public:
  PointFromSphereCenters(SphereCenterModel const * sphereCenterModel);
  ~PointFromSphereCenters();

  void findNearestPoint(Vector3<double> const * queryPoint,
			double fracErrorBound,
			Vector3<double> & nearestPointNormal,
			double & distance) const;

private:
  SphereCenterModel const * sphereCenterModel;
};

template <class SphereCenterModel>
PointFromSphereCenters<SphereCenterModel>::
PointFromSphereCenters(SphereCenterModel const * sphereCenterModel) 
  : sphereCenterModel(sphereCenterModel) {
  
}

template <class SphereCenterModel>
PointFromSphereCenters<SphereCenterModel>::
~PointFromSphereCenters() {

}

template <class SphereCenterModel>
void 
PointFromSphereCenters<SphereCenterModel>::
findNearestPoint(Vector3<double> const * queryPoint,
		 double fracErrorBound, 
		 Vector3<double> & nearestPointNormal,
		 double & distance) const {

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
					 foundSphere, 
					 centerDistSqr); 

    double foundDistance = 
      sqrt(centerDistSqr) - foundSphere->getRadius();

    if (foundDistance < minDistance) {
      nearestSphere = foundSphere;
      minDistance   = foundDistance;
    }
  }

  nearestPointNormal = *queryPoint - nearestSphere->getCenter();
  distance           = minDistance;
}

// ================================================================

#endif  // #ifndef POINT_FROM_SPHERE_CENTERS_H

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** NearestSurfacePoint/PointFromSphereCenters.h ***

// *** START *** NearestSurfacePoint/PointFromSpheres.h ***
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
// Date:    Mon Aug 17 17:01:47 2015 EDT
//
// Time-stamp: <2015-10-05 15:08:19 dcj>
//
// ================================================================

#ifndef POINT_FROM_SPHERES_H
#define POINT_FROM_SPHERES_H

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

// ================================================================

template <class SphereModel>
class PointFromSpheres
{
public:
  PointFromSpheres(SphereModel const * sphereModel);
  ~PointFromSpheres();

  void findNearestPoint(Vector3<double> const * queryPoint,
			double fracErrorBound,
			Vector3<double> & nearestPointNormal,
			double & distance) const;

private:
  SphereModel const * sphereModel;
};

template <class SphereModel>
PointFromSpheres<SphereModel>::
PointFromSpheres(SphereModel const * sphereModel) 
  : sphereModel(sphereModel) {
  
}

template <class SphereModel>
PointFromSpheres<SphereModel>::
~PointFromSpheres() {

}

template <class SphereModel>
void 
PointFromSpheres<SphereModel>::
findNearestPoint(Vector3<double> const * queryPoint,
		 double fracErrorBound, 
		 Vector3<double> & nearestPointNormal,
		 double & distance) const {

  Sphere<double> const * nearestSphere = NULL;

  double minDistance = std::numeric_limits<double>::max();

  sphereModel->findNearestSphere(queryPoint,
				 fracErrorBound,
				 nearestSphere, 
				 minDistance);

  nearestPointNormal = *queryPoint - nearestSphere->getCenter();
  distance           = minDistance;
}

// ================================================================

#endif  // #ifndef POINT_FROM_SPHERES_H

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** NearestSurfacePoint/PointFromSpheres.h ***

// *** START *** InsideOutside/InOutPoints.h ***
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
// Date:    Mon Aug 17 17:01:47 2015 EDT
//
// Time-stamp: <2015-08-31 15:12:01 dcj>
//
// ================================================================

#ifndef IN_OUT_POINTS_H
#define IN_OUT_POINTS_H

// inlined #include "../Geometry/Vector3.h"

// ================================================================

template <class PointModel>
class InOutPoints
{
public:
  InOutPoints(PointModel const * pointModel);
  ~InOutPoints();

  bool isInside(Vector3<double> const * queryPoint,
		double fracErrorBound) const;

private:
  PointModel const * pointModel;
};

template <class PointModel>
InOutPoints<PointModel>::
InOutPoints(PointModel const * pointModel) 
  : pointModel(pointModel) {
  
}

template <class PointModel>
InOutPoints<PointModel>::
~InOutPoints() {

}

template <class PointModel>
bool 
InOutPoints<PointModel>::
isInside(Vector3<double> const * queryPoint,
	 double fracErrorBound) const {
  
  bool hitObject = false;

  Vector3<double> nearestPoint;
  Vector3<double> nearestPointNormal;

  pointModel->findNearestPoint(queryPoint,
			       fracErrorBound,
			       nearestPoint,
			       nearestPointNormal);

  Vector3<double> displacement = *queryPoint - nearestPoint;

  double dot = displacement.dot(nearestPointNormal);

  if (dot < 0) {
    hitObject = true;
  }
  else {
    hitObject = false;
  }

  return hitObject;
}

// ================================================================

#endif  // #ifndef IN_OUT_POINTS_H

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** InsideOutside/InOutPoints.h ***

// *** START *** InsideOutside/InOutSphereCenters.h ***
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
// Date:    Mon Aug 17 17:01:47 2015 EDT
//
// Time-stamp: <2015-08-24 16:01:40 dcj>
//
// ================================================================

#ifndef IN_OUT_SPHERE_CENTERS_H
#define IN_OUT_SPHERE_CENTERS_H

// inlined #include "../Geometry/Vector3.h"

// ================================================================

template <class SphereCenterModel>
class InOutSphereCenters
{
public:
  InOutSphereCenters(SphereCenterModel const * sphereCenterModel);
  ~InOutSphereCenters();

  bool isInside(Vector3<double> const * queryPoint,
		double fracErrorBound) const;

private:
  SphereCenterModel const * sphereCenterModel;
};

template <class SphereCenterModel>
InOutSphereCenters<SphereCenterModel>::
InOutSphereCenters(SphereCenterModel const * sphereCenterModel) 
  : sphereCenterModel(sphereCenterModel) {
  
}

template <class SphereCenterModel>
InOutSphereCenters<SphereCenterModel>::
~InOutSphereCenters() {

}

template <class SphereCenterModel>
bool 
InOutSphereCenters<SphereCenterModel>::
isInside(Vector3<double> const * queryPoint,
	 double fracErrorBound) const {

  for (unsigned int radiusNum = 0; 
       radiusNum < sphereCenterModel->getNumRadii(); 
       radiusNum ++) {

    double centerDistSqr = -1;

    Sphere<double> const * foundSphere = NULL;

    sphereCenterModel->findNearestSphere(radiusNum,
					 queryPoint,
					 fracErrorBound,
					 foundSphere, 
					 centerDistSqr); 

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

// *** START *** InsideOutside/InOutSpheres.h ***
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
// Date:    Mon Aug 17 17:01:47 2015 EDT
//
// Time-stamp: <2015-10-05 15:08:05 dcj>
//
// ================================================================

#ifndef IN_OUT_SPHERES_H
#define IN_OUT_SPHERES_H

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

// ================================================================

template <class SphereModel>
class InOutSpheres
{
public:
  InOutSpheres(SphereModel const * sphereModel);
  ~InOutSpheres();

  bool isInside(Vector3<double> const * queryPoint,
		double fracErrorBound) const;

private:
  SphereModel const * sphereModel;
};

template <class SphereModel>
InOutSpheres<SphereModel>::
InOutSpheres(SphereModel const * sphereModel) 
  : sphereModel(sphereModel) {
  
}

template <class SphereModel>
InOutSpheres<SphereModel>::
~InOutSpheres() {

}

template <class SphereModel>
bool 
InOutSpheres<SphereModel>::
isInside(Vector3<double> const * queryPoint,
	 double fracErrorBound) const {

  Sphere<double> const * nearestSphere = NULL;

  double minDistance = std::numeric_limits<double>::max();

  sphereModel->findNearestSphere(queryPoint,
				 fracErrorBound,
				 nearestSphere, 
				 minDistance);

  if (minDistance <= 0) {
    return true;
  }

  return false;
}

// ================================================================

#endif  // #ifndef IN_OUT_SPHERES_H

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** InsideOutside/InOutSpheres.h ***

// *** START *** SphereCenterModel/NanoFLANNDatasetAdaptor.h ***
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
// Time-stamp: <2015-02-10 16:18:09 dcj>
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
  NanoFLANNDatasetAdaptor(int nPts,
			  int * originalSphereIndexes,
			  std::vector<Sphere<double> > const * originalSpheres);

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
  kdtree_get_bbox(BBOX &bb) const {

  return false;
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** SphereCenterModel/NanoFLANNDatasetAdaptor.h ***

// *** START *** SphereCenterModel/NanoFLANNSort.h ***
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
// Time-stamp: <2015-08-24 15:29:05 dcj>
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

class NanoFLANNSort {
 public:
  NanoFLANNSort();

  ~NanoFLANNSort();

  void preprocess(std::vector<Sphere<double> > const * spheres,
		  double fracErrorBound);

  unsigned int getNumRadii() const;

  void findNearestSphere(int radiusNum,
			 Vector3<double> const * queryPoint,
			 double fracErrorBound,
			 Sphere<double> const * & nearestSphere, 
			 double & centerDistSqr) const;

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

// *** START *** DiscModel/NanoFLANNDisc.h ***
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
// Time-stamp: <2015-10-19 14:55:00 dcj>
//
// ================================================================

#ifndef NANO_FLANN_DISC_H
#define NANO_FLANN_DISC_H

#include <vector>

// inlined #include "../Geometry/Vector3.h"

// inlined #include "../PointModel/NanoFLANNPoint.h"

// ================================================================

class NanoFLANNDisc {
 public:
  NanoFLANNDisc();

  ~NanoFLANNDisc();

  void preprocess(std::vector<Vector3<double> > const * points,
		  std::vector<Vector3<double> > const * normals,
		  double discRadius,
		  double fracErrorBound);

  void findNearestPoint(Vector3<double> const * queryPoint,
			double fracErrorBound,
			Vector3<double> & nearestPoint,
			Vector3<double> & nearestPointNormal) const;

  double getDiscRadius() const;

  void printDataStructureStats() const;
  void printSearchStats() const;

 private:
  NanoFLANNPoint nanoFLANNPoint;

  double discRadius;
};

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** DiscModel/NanoFLANNDisc.h ***

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

// *** START *** BoundingSphere/BoundingSphereAABB.h ***
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
// Date:    Thu Feb 12 16:39:41 2015 EDT
//
// Time-stamp: <2015-10-15 09:31:01 dcj>
//
// ================================================================

#ifndef BOUNDING_SPHERE_AABB_H
#define BOUNDING_SPHERE_AABB_H

// inlined #include "../Geometry/Vector3.h"
// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Voxels.h"

template <class T>
class BoundingSphereAABB {
 public:
  static Sphere<T> generate(std::vector<Sphere<T> > const * spheres);
  static Sphere<T> generate(Voxels<T> const * voxels);

 private:
  static Sphere<T> generate(Vector3<T> const * minCornerCoords,
			    Vector3<T> const * maxCornerCoords);
};

template <class T>
Sphere<T> 
BoundingSphereAABB<T>::generate(std::vector<Sphere<T> > const * spheres) {

  Vector3<T> minCornerCoords(std::numeric_limits<T>::max(), 
			     std::numeric_limits<T>::max(),
			     std::numeric_limits<T>::max());

  Vector3<T> maxCornerCoords(0, 0, 0);

  for (typename std::vector<Sphere<T> >::const_iterator it = spheres->begin();
       it != spheres->end();
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

  return generate(&minCornerCoords,
		  &maxCornerCoords);
}

template <class T>
Sphere<T> 
BoundingSphereAABB<T>::generate(Voxels<T> const * voxels) {

  Vector3<T> minCornerCoords(std::numeric_limits<T>::max(), 
			     std::numeric_limits<T>::max(),
			     std::numeric_limits<T>::max());

  Vector3<T> maxCornerCoords(0, 0, 0);

  //  for (typename std::vector<Vector3<T> >::const_iterator it = 
  //	 voxels->getPoints()->begin();
  //       it != voxels->getPoints()->end();
  //       ++it) {

  //    for (int dim = 0; dim < 3; dim++) {

  //      if (it->get(dim) < minCornerCoords.get(dim)) {
  //	minCornerCoords.set(dim, it->get(dim));
  //      }

  //      if (it->get(dim) > maxCornerCoords.get(dim)) {
  //	maxCornerCoords.set(dim, it->get(dim));
  //      }
  //    }
  //  }

  for (int k = 0; k < voxels->getNumVoxels()->getK(); k++) {
  for (int j = 0; j < voxels->getNumVoxels()->getJ(); j++) {
  for (int i = 0; i < voxels->getNumVoxels()->getI(); i++) {

    Vector3<T> ijk(i, j, k);

    Vector3<T> lowCornerCoords  = ijk * *(voxels->getVoxelDims());
    Vector3<T> highCornerCoords = lowCornerCoords + *(voxels->getVoxelDims());

    if (voxels->getData(i, j, k) != 0) {
      for (int dim = 0; dim < 3; dim++) {
	if (lowCornerCoords.get(dim) < minCornerCoords.get(dim)) {
	  minCornerCoords.set(dim, lowCornerCoords.get(dim));
	}

	if (highCornerCoords.get(dim) > maxCornerCoords.get(dim)) {
	  maxCornerCoords.set(dim, highCornerCoords.get(dim));
	}
      }
    }
  }
  }
  }

  return generate(&minCornerCoords,
		  &maxCornerCoords);
}

template <class T>
Sphere<T> 
BoundingSphereAABB<T>::generate(Vector3<T> const * minCornerCoords,
				Vector3<T> const * maxCornerCoords) {

  Vector3<T> AABBDiagonal((*maxCornerCoords - *minCornerCoords) / 2);

  Vector3<T> boundingSphereCenter(*minCornerCoords + AABBDiagonal);

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
// Time-stamp: <2015-02-23 10:37:24 dcj>
//
// ================================================================

#ifndef RANDOM_SPHERE_POINT_MARSAGLIA_H
#define RANDOM_SPHERE_POINT_MARSAGLIA_H

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

template <class T, class RNG>
class RandomSpherePointMarsaglia {
 public:
  static Vector3<T> generate(RNG * rng, Sphere<T> const * sphere);
};

template <class T, class RNG>
Vector3<T> 
RandomSpherePointMarsaglia<T, RNG>::generate(RNG * rng, 
					     Sphere<T> const * sphere) {

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

  spherePoint *= sphere->getRadius();

  spherePoint += sphere->getCenter();

  return spherePoint;
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** SpherePoint/RandomSpherePointMarsaglia.h ***

// *** START *** SpherePoint/RandomSpherePointPolar.h ***
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
// Time-stamp: <2015-02-23 10:37:41 dcj>
//
// ================================================================

#ifndef RANDOM_SPHERE_POINT_POLAR_H
#define RANDOM_SPHERE_POINT_POLAR_H

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

template <class T, class RNG>
class RandomSpherePointPolar {
 public:
  static Vector3<T> generate(RNG * rng, Sphere<T> const * sphere);
};

template <class T, class RNG>
Vector3<T> 
RandomSpherePointPolar<T, RNG>::generate(RNG * rng, 
					 Sphere<T> const * sphere) {

  //generate point uniformly distributed on unit sphere

  T cosTheta = rng->getRandInRange(-1, 1);
  T sinTheta = sqrt(1 - pow(cosTheta, 2));
  T phi      = rng->getRandInRange(0, 2*M_PI);

  Vector3<T> spherePoint(sinTheta * sin(phi),
			 cosTheta,
			 sinTheta * cos(phi));

  //move point onto requested sphere

  spherePoint *= sphere->getRadius();

  spherePoint += sphere->getCenter();

  return spherePoint;
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** SpherePoint/RandomSpherePointPolar.h ***

// *** START *** SpherePoint/BiasedSpherePointRejection.h ***
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
// Time-stamp: <2015-03-02 13:23:05 dcj>
//
// ================================================================

#ifndef BIASED_SPHERE_POINT_REJECTION_H
#define BIASED_SPHERE_POINT_REJECTION_H

#include <cmath>

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

template <class T, 
          class RandomNumberGenerator, 
          class RandomSpherePointGenerator>
class BiasedSpherePointRejection {
 public:
  static Vector3<T> generate(RandomNumberGenerator * rng, 
			     Sphere<T> const * sphere,
			     Vector3<T> const * distributionCenter,
			     T alpha);

 private:
  static T samplePDF(T alpha, T cosTheta);
};

template <class T, 
          class RandomNumberGenerator, 
          class RandomSpherePointGenerator>
Vector3<T> 
BiasedSpherePointRejection<T, 
                           RandomNumberGenerator,
                           RandomSpherePointGenerator>::
  generate(RandomNumberGenerator * rng, 
	   Sphere<T> const * sphere,
	   Vector3<T> const * distributionCenter,
	   T alpha) {

  Vector3<T> candidatePoint;

  bool rejectPoint = false;

  do {
    candidatePoint = RandomSpherePointGenerator::generate(rng, sphere);

    Vector3<T> recenteredCandidatePoint = 
      candidatePoint - sphere->getCenter();

    Vector3<T> recenteredDistributionCenter = 
      *distributionCenter - sphere->getCenter();

    T cosTheta = 
      recenteredCandidatePoint.dot(recenteredDistributionCenter) /
      (recenteredCandidatePoint.getMagnitude() * 
       recenteredDistributionCenter.getMagnitude());

    T probabilityDensity = samplePDF(alpha, cosTheta);

    rejectPoint = (rng->getRandInRange(0, 1) > probabilityDensity);
  } 
  while(rejectPoint);

  return candidatePoint;
}

template <class T, 
          class RandomNumberGenerator, 
          class RandomSpherePointGenerator>
T 
BiasedSpherePointRejection<T, 
                           RandomNumberGenerator,
                           RandomSpherePointGenerator>::
  samplePDF(T alpha, T cosTheta) {

  return 
    (1 - pow(alpha, 2)) / 
    (4*M_PI*pow((1 - 2*alpha*cosTheta + pow(alpha, 2)), 1.5));
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** SpherePoint/BiasedSpherePointRejection.h ***

// *** START *** SpherePoint/BiasedSpherePointDirect.h ***
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
// Time-stamp: <2015-07-22 12:49:41 dcj>
//
// ================================================================

#ifndef BIASED_SPHERE_POINT_DIRECT_H
#define BIASED_SPHERE_POINT_DIRECT_H

#include <cmath>
#include <cassert>

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

template <class T, 
          class RandomNumberGenerator, 
          class RandomSpherePointGenerator>
class BiasedSpherePointDirect {
 public:
  static Vector3<T> generate(RandomNumberGenerator * rng, 
			     Sphere<T> const * sphere,
			     Vector3<T> const * distributionCenter,
			     T alpha);

 private:
  static T computeCosTheta(T alpha, T R);
};

template <class T, 
          class RandomNumberGenerator, 
          class RandomSpherePointGenerator>
Vector3<T> 
BiasedSpherePointDirect<T, 
                        RandomNumberGenerator,
                        RandomSpherePointGenerator>::
  generate(RandomNumberGenerator * rng, 
	   Sphere<T> const * sphere,
	   Vector3<T> const * distributionCenter,
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
    *distributionCenter - sphere->getCenter();

  recenteredDistributionCenter.normalize();

  Vector3<T> rotationAxis =
    recenteredDistributionCenter.cross(Vector3<T>(0, 0, 1));

  T sinRotationAngle = rotationAxis.getMagnitude();

  T cosRotationAngle =
    recenteredDistributionCenter.dot(Vector3<T>(0, 0, 1));

  point.rotate(cosRotationAngle, sinRotationAngle, rotationAxis);

  //move point onto requested sphere

  point *= sphere->getRadius();
  point += sphere->getCenter();

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
// Time-stamp: <2015-05-27 16:21:59 derekj>
//
// ================================================================

#ifndef RANDOM_BALL_POINT_REJECTION_H
#define RANDOM_BALL_POINT_REJECTION_H

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

template <class T, class RNG>
class RandomBallPointRejection {
 public:
  static Vector3<T> generate(RNG * rng, Sphere<T> const * ball);
};

template <class T, class RNG>
Vector3<T> 
RandomBallPointRejection<T, RNG>::generate(RNG * rng, 
					   Sphere<T> const * ball) {

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

  ballPoint *= ball->getRadius();

  ballPoint += ball->getCenter();

  return ballPoint;
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** SpherePoint/RandomBallPointRejection.h ***

// *** START *** Walker/WalkerExterior.h ***
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
// Date:    Fri Feb 13 13:31:22 2015 EDT
//
// Time-stamp: <2015-09-18 10:54:15 dcj>
//
// ================================================================

#ifndef WALKER_EXTERIOR_H
#define WALKER_EXTERIOR_H

#include <vector>

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

template <class T, 
  class RandomNumberGenerator, 
  class NearestSurfacePointFinder,
  class RandomSpherePointGenerator,
  class BiasedSpherePointGenerator>
class WalkerExterior {
 public:
  WalkerExterior(RandomNumberGenerator * randomNumberGenerator, 
		 Sphere<T> const * boundingSphere, 
		 NearestSurfacePointFinder const * nearestSurfacePointFinder,
		 T fracErrorBound,
		 T shellThickness);

  ~WalkerExterior();

  void walk(bool & hitObject, int & numSteps,
	    Vector3<T> & startPoint, Vector3<T> & endPoint, 
	    Vector3<T> & normal);

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
		 Sphere<T> const * boundingSphere, 
		 NearestSurfacePointFinder const * nearestSurfacePointFinder,
		 T fracErrorBound,
		 T shellThickness) :
  randomNumberGenerator(randomNumberGenerator), 
  boundingSphere(boundingSphere),
  nearestSurfacePointFinder(nearestSurfacePointFinder), 
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
  walk(bool & hitObject, int & numSteps,
       Vector3<T> & startPoint, Vector3<T> & endPoint,
       Vector3<T> & normal) {

  hitObject = false;
  numSteps  = 0;

  Vector3<T> position = 
    RandomSpherePointGenerator::generate(randomNumberGenerator, boundingSphere);

  startPoint = position;

  for (;;) {

    T minDistance = 0;
  
    nearestSurfacePointFinder->findNearestPoint(&position,
						fracErrorBound, 
						normal,
						minDistance);

    if (minDistance < shellThickness) {
      //walker is absorbed

      endPoint  = position;
      hitObject = true;
      return;
    }

    numSteps++;

    Sphere<T> stepSphere(position, minDistance);

    position = RandomSpherePointGenerator::generate(randomNumberGenerator, 
						    &stepSphere);

    T centerDistSqr = 
      (position - boundingSphere->getCenter()).getMagnitudeSqr();

    if (centerDistSqr > boundingSphere->getRadiusSqr()) {
      //walker left bounding sphere

      T alpha = boundingSphere->getRadius() / sqrt(centerDistSqr);

      if (randomNumberGenerator->getRandInRange(0, 1) > (1 - alpha)) {
	//walker is replaced

	position = BiasedSpherePointGenerator::generate(randomNumberGenerator, 
							boundingSphere,
							&position,
							alpha);
      }
      else {
	//walker escapes

	hitObject = false;
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
// Date:    Fri Feb 13 13:31:22 2015 EDT
//
// Time-stamp: <2015-08-17 17:31:48 dcj>
//
// ================================================================

#ifndef SAMPLER_INTERIOR_H
#define SAMPLER_INTERIOR_H

#include <vector>

// inlined #include "../Geometry/Sphere.h"
// inlined #include "../Geometry/Vector3.h"

template <class T, 
  class RandomNumberGenerator, 
  class InsideOutsideTester,
  class RandomBallPointGenerator>
class SamplerInterior {
 public:
  SamplerInterior(RandomNumberGenerator * randomNumberGenerator, 
		  Sphere<T> const * boundingSphere, 
		  InsideOutsideTester const * insideOutsideTester,
		  T fracErrorBound);

  ~SamplerInterior();

  void sample(bool & hitObject);

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
		 Sphere<T> const * boundingSphere, 
		 InsideOutsideTester const * insideOutsideTester,
		 T fracErrorBound) :
  randomNumberGenerator(randomNumberGenerator), 
  boundingSphere(boundingSphere),
  insideOutsideTester(insideOutsideTester), 
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

template <class T, 
  class RandomNumberGenerator, 
  class InsideOutsideTester,
  class RandomBallPointGenerator>
void 
SamplerInterior<T, 
               RandomNumberGenerator, 
               InsideOutsideTester, 
               RandomBallPointGenerator>::
  sample(bool & hitObject) {

  Vector3<T> position = 
    RandomBallPointGenerator::generate(randomNumberGenerator, boundingSphere);

  hitObject = 
    insideOutsideTester->isInside(&position,
				  fracErrorBound);
}

#endif

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// End:

// *** END *** Walker/SamplerInterior.h ***

// *** START *** MemMonitor.h ***
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
// Date:    Wed May 21 17:30:46 2014 EDT
//
// Time-stamp: <2014-11-12 17:37:38 dcj>
//
// ================================================================

#ifndef MEMMONITOR_H_
#define MEMMONITOR_H_

// ================================================================

class MemMonitor
{
public:
  MemMonitor();
  ~MemMonitor();

  void sample();

  float getCurrentUsage() const;
  float getPeakUsage() const;

private:
  float currentUsage;
  float peakUsage;
};
// ================================================================

#endif  // #ifndef MEMMONITOR_H_

// ================================================================

// Local Variables:
// time-stamp-line-limit: 30
// mode: c++
// End:

// *** END *** MemMonitor.h ***

// *** START *** Timer.h ***
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
// Date:    Wed May 21 16:21:57 2014 EDT
//
// Time-stamp: <2015-03-24 17:29:48 derekj>
//
// ================================================================

#ifndef TIMER_H_
#define TIMER_H_

// ================================================================

#ifdef USE_MPI
#include <mpi.h>
#else
#include <ctime>
#endif

// ================================================================

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

#ifdef USE_MPI
  double startTime;
#else
  clock_t startTime;
#endif

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

