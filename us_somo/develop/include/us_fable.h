// -- start: fem.hpp ----------------------
#ifndef FEM_HPP
#define FEM_HPP

// -- start: fem/do.hpp ----------------------
#ifndef FEM_DO_HPP
#define FEM_DO_HPP

#define FEM_DO(i, f, l) for (i = f; i <= l; i++)

namespace fem {

   class do_safe {
         int *i_;
         int l_;

      public:
         do_safe(int *i, int f, int l) : i_(i), l_(l) { (*i) = f; }

         operator bool() { return (*i_) <= l_; }

         void incr() { (*i_)++; }
   };

} // namespace fem

#define FEM_DO_SAFE(i, f, l) for (fem::do_safe fem_do(&(i), f, l); fem_do; fem_do.incr())

namespace fem {

   class dostep {
         int *i_;
         int l_;
         int s_;

      public:
         dostep(int *i, int f, int l, int s) : i_(i), l_(l), s_(s) { (*i) = f; }

         operator bool() {
            if (s_ < 0)
               return (*i_) >= l_;
            return (*i_) <= l_;
         }

         void incr() { (*i_) += s_; }
   };

} // namespace fem

#define FEM_DOSTEP(i, f, l, s) for (fem::dostep fem_do(&(i), f, l, s); fem_do; fem_do.incr())

#endif // GUARD
// --  end: fem/do.hpp ----------------------
// -- start: fem/data_of_type.hpp ----------------------
#ifndef FEM_DATA_OF_TYPE_HPP
#define FEM_DATA_OF_TYPE_HPP

// -- start: fem/arr.hpp ----------------------
#ifndef FEM_ARR_HPP
#define FEM_ARR_HPP

// -- start: fem/arr_ref.hpp ----------------------
#ifndef FEM_ARR_REF_HPP
#define FEM_ARR_REF_HPP

// -- start: fem/dimension.hpp ----------------------
#ifndef FEM_DIMENSION_HPP
#define FEM_DIMENSION_HPP

// -- start: fem/arr_and_str_indices.hpp ----------------------
#ifndef FEM_ARR_AND_STR_INDICES_HPP
#define FEM_ARR_AND_STR_INDICES_HPP

// -- start: fem/size_t.hpp ----------------------
#ifndef FEM_SIZE_T_HPP
#define FEM_SIZE_T_HPP

#include <cstddef>
#include <limits>

namespace fem {

   typedef std::size_t size_t;
   typedef std::ptrdiff_t ssize_t;

   static const size_t size_t_max = std::numeric_limits<size_t>::max();
   static const ssize_t ssize_t_max = std::numeric_limits<ssize_t>::max();

   template<typename T>
   struct array_of_2 {
         T elems[ 2 ];

         array_of_2() {}

         array_of_2(T const &i, T const &j) {
            elems[ 0 ] = i;
            elems[ 1 ] = j;
         }
   };

   typedef array_of_2<size_t> size_t_2;
   typedef array_of_2<ssize_t> ssize_t_2;

} // namespace fem

#endif // GUARD
// --  end: fem/size_t.hpp ----------------------

namespace fem {

   static const size_t arr_dim_max = 6;

   template<size_t Ndims>
   struct arr_and_str_indices;

   template<size_t Ndims>
   struct arr_index_data {
         size_t actual_number_of_dimensions;
         ssize_t elems[ Ndims ];

         arr_index_data() : actual_number_of_dimensions(0) {}

         explicit arr_index_data(ssize_t i1) : actual_number_of_dimensions(1) { elems[ 0 ] = i1; }

         arr_index_data(ssize_t i1, ssize_t i2) : actual_number_of_dimensions(2) {
            elems[ 0 ] = i1;
            elems[ 1 ] = i2;
         }

         arr_index_data(ssize_t i1, ssize_t i2, ssize_t i3) : actual_number_of_dimensions(3) {
            elems[ 0 ] = i1;
            elems[ 1 ] = i2;
            elems[ 2 ] = i3;
         }

         arr_index_data(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4) : actual_number_of_dimensions(4) {
            elems[ 0 ] = i1;
            elems[ 1 ] = i2;
            elems[ 2 ] = i3;
            elems[ 3 ] = i4;
         }

         arr_index_data(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4, ssize_t i5) : actual_number_of_dimensions(5) {
            elems[ 0 ] = i1;
            elems[ 1 ] = i2;
            elems[ 2 ] = i3;
            elems[ 3 ] = i4;
            elems[ 4 ] = i5;
         }

         arr_index_data(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4, ssize_t i5, ssize_t i6) :
             actual_number_of_dimensions(6) {
            elems[ 0 ] = i1;
            elems[ 1 ] = i2;
            elems[ 2 ] = i3;
            elems[ 3 ] = i4;
            elems[ 4 ] = i5;
            elems[ 5 ] = i6;
         }

         inline arr_and_str_indices<Ndims> operator()(int first, int last) const;
   };

   typedef arr_index_data<arr_dim_max> arr_index;

   struct str_index {
         int first;
         int last;

         str_index(int first_, int last_) : first(first_), last(last_) {}
   };

   template<size_t Ndims>
   struct arr_and_str_indices {
         arr_index_data<Ndims> arr_ix;
         str_index str_ix;

         arr_and_str_indices(arr_index_data<Ndims> const &arr_ix_, str_index const &str_ix_) :
             arr_ix(arr_ix_), str_ix(str_ix_) {}
   };

   template<size_t Ndims>
   inline arr_and_str_indices<Ndims> arr_index_data<Ndims>::operator()(int first, int last) const {
      return arr_and_str_indices<Ndims>(*this, str_index(first, last));
   }

} // namespace fem

#endif // GUARD
// --  end: fem/arr_and_str_indices.hpp ----------------------
// -- start: fem/error_utils.hpp ----------------------
#ifndef FEM_ERROR_UTILS_HPP
#define FEM_ERROR_UTILS_HPP

// -- start: tbxx/error_utils.hpp ----------------------
#ifndef TBXX_ERROR_UTILS_H
#define TBXX_ERROR_UTILS_H

// -- start: tbxx/libc_backtrace.hpp ----------------------
#ifndef TBXX_LIBC_BACKTRACE_HPP
#define TBXX_LIBC_BACKTRACE_HPP

#include <cstdlib>
#include <cstring>
#include <ostream>

#if defined(__GNUC__)
#if defined(__linux) || (defined(__APPLE_CC__) && __APPLE_CC__ >= 5465)
#include <execinfo.h>
#define TBXX_LIBC_BACKTRACE_HAVE_EXECINFO_H
#if ((__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))) && !defined(__EDG_VERSION__)
#include <cxxabi.h>
#define TBXX_LIBC_BACKTRACE_HAVE_CXXABI_H
#endif
#endif
#endif

namespace tbxx {
   namespace libc_backtrace {

      inline bool show_if_possible(std::ostream &ostream, int n_frames_skip = 0) {
         static bool active = false;
         if (active)
            return false;
         active = true;
         bool result = false;
#if defined(TBXX_LIBC_BACKTRACE_HAVE_EXECINFO_H)
         static const int max_frames = 1024;
         void *array[ max_frames ];
         int size = backtrace(array, max_frames);
         ostream << "libc backtrace (" << size - n_frames_skip << " frames, most recent call last):" << std::endl;
         char **strings = backtrace_symbols(array, size);
         for (int i = size - 1; i >= n_frames_skip; i--) {
            char *s = strings[ i ];
#if defined(TBXX_LIBC_BACKTRACE_HAVE_CXXABI_H)
            const char *m_bgn = 0;
#if defined(__APPLE_CC__)
            if (std::strlen(s) >= 52 && std::strncmp(s + 40, "0x", 2) == 0) {
               m_bgn = std::strchr(s + 40, ' ');
            }
#else // __linux
            m_bgn = std::strchr(s, '(');
#endif
            if (m_bgn != 0) {
               m_bgn++;
               const char *m_end = std::strchr(
                  m_bgn,
#if defined(__APPLE_CC__)
                  ' '
#else // __linux
                  '+'
#endif
               );
               long n = m_end - m_bgn;
               if (n > 0) {
                  char *mangled = static_cast<char *>(std::malloc(n + 1));
                  if (mangled != 0) {
                     std::strncpy(mangled, m_bgn, n);
                     mangled[ n ] = '\0';
                     char *demangled = abi::__cxa_demangle(mangled, 0, 0, 0);
                     std::free(mangled);
                     if (demangled != 0) {
                        long n1 = m_bgn - s;
                        long n2 = std::strlen(demangled);
                        long n3 = std::strlen(m_end);
                        char *b = static_cast<char *>(std::malloc(static_cast<size_t>(n1 + n2 + n3 + 1)));
                        if (b != 0) {
                           std::strncpy(b, s, n1);
                           std::strncpy(b + n1, demangled, n2);
                           std::strncpy(b + n1 + n2, m_end, n3);
                           b[ n1 + n2 + n3 ] = '\0';
                           s = b;
                        }
                        std::free(demangled);
                     }
                  }
               }
            }
#endif // TBXX_LIBC_BACKTRACE_HAVE_CXXABI_H
            ostream << "  " << s << std::endl;
            if (s != strings[ i ])
               std::free(s);
            result = true;
         }
         std::free(strings);
#endif // TBXX_LIBC_BACKTRACE_HAVE_EXECINFO_H
         active = false;
         return result;
      }

   } // namespace libc_backtrace
} // namespace tbxx

#endif // GUARD
// --  end: tbxx/libc_backtrace.hpp ----------------------

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#define TBXX_CHECK_POINT std::cout << __FILE__ << "(" << __LINE__ << ")" << std::endl
#define TBXX_CHECK_POINT_MSG(msg) std::cout << msg << " @ " __FILE__ << "(" << __LINE__ << ")" << std::endl
#define TBXX_EXAMINE(A) std::cout << "variable " << (#A) << ": " << A << std::endl

namespace tbxx {
   namespace error_utils {

      //! Simple debugging aid: add call to code, recompile, run in gdb, use "where"
      inline int segfault_if(bool condition) {
         if (condition) {
            int *ptr = 0;
            return *ptr;
         }
         return 0;
      }

      inline std::string file_and_line_as_string(const char *file, long line) {
         std::ostringstream o;
         o << file << "(" << line << ")";
         return o.str();
      }

      inline std::string file_and_line_as_string_with_backtrace(const char *file, long line) {
         std::ostringstream o;
         libc_backtrace::show_if_possible(o, 1);
         o << file << "(" << line << ")";
         return o.str();
      }

   } // namespace error_utils
} // namespace tbxx

#define TBXX_ASSERT(condition) \
   if (!(condition)) { \
      throw std::runtime_error( \
         tbxx::error_utils::file_and_line_as_string(__FILE__, __LINE__) + ": ASSERT(" #condition ") failure."); \
   }

#define TBXX_UNREACHABLE_ERROR() \
   std::runtime_error( \
      "Control flow passes through branch that should be unreachable: " \
      + tbxx::error_utils::file_and_line_as_string(__FILE__, __LINE__))

#define TBXX_NOT_IMPLEMENTED() \
   std::runtime_error("Not implemented: " + tbxx::error_utils::file_and_line_as_string(__FILE__, __LINE__))

#endif // GUARD
// --  end: tbxx/error_utils.hpp ----------------------

#define FEM_THROW_UNHANDLED(message) \
   throw std::runtime_error(tbxx::error_utils::file_and_line_as_string(__FILE__, __LINE__) + ": UNHANDLED: " + message)

#endif // GUARD
// --  end: fem/error_utils.hpp ----------------------
// -- start: fem/star.hpp ----------------------
#ifndef FEM_STAR_HPP
#define FEM_STAR_HPP

namespace fem {

   struct star_type {};

   static const star_type star = star_type();

} // namespace fem

#endif // GUARD
// --  end: fem/star.hpp ----------------------
#include <algorithm>

namespace fem {

   template<size_t Ndims>
   struct dims_base_1 {
         size_t all[ Ndims ];

         size_t size_1d(size_t n_dims = Ndims) const {
            size_t result = 1;
            for (size_t i = 0; i < n_dims; i++)
               result *= all[ i ];
            return result;
         }

         template<size_t BufferNdims>
         void set_dims(dims_base_1<BufferNdims> const &source) {
            for (size_t i = 0; i < Ndims; i++)
               all[ i ] = source.all[ i ];
         }
   };

   template<size_t Ndims>
   struct dim_data : dims_base_1<Ndims> {
         ssize_t origin[ Ndims ];

         template<size_t BufferNdims>
         void set_dims(dim_data<BufferNdims> const &source) {
            dims_base_1<Ndims>::set_dims(source);
            for (size_t i = 0; i < Ndims; i++)
               origin[ i ] = source.origin[ i ];
         }
   };

   template<size_t Ndims>
   struct dims;

   template<>
   struct dims<1> : dim_data<1> {
         size_t index_1d(ssize_t i1) const { return i1 - this->origin[ 0 ]; }
   };

   template<>
   struct dims<2> : dim_data<2> {
         size_t index_1d(ssize_t i1, ssize_t i2) const {
            return (i2 - this->origin[ 1 ]) * this->all[ 0 ] + (i1 - this->origin[ 0 ]);
         }
   };

   template<>
   struct dims<3> : dim_data<3> {
         size_t index_1d(ssize_t i1, ssize_t i2, ssize_t i3) const {
            return ((i3 - this->origin[ 2 ]) * this->all[ 1 ] + (i2 - this->origin[ 1 ])) * this->all[ 0 ]
                   + (i1 - this->origin[ 0 ]);
         }
   };

   template<>
   struct dims<4> : dim_data<4> {
         size_t index_1d(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4) const {
            return (((i4 - this->origin[ 3 ]) * this->all[ 2 ] + (i3 - this->origin[ 2 ])) * this->all[ 1 ]
                    + (i2 - this->origin[ 1 ]))
                      * this->all[ 0 ]
                   + (i1 - this->origin[ 0 ]);
         }
   };

   template<>
   struct dims<5> : dim_data<5> {
         size_t index_1d(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4, ssize_t i5) const {
            return ((((i5 - this->origin[ 4 ]) * this->all[ 3 ] + (i4 - this->origin[ 3 ])) * this->all[ 2 ]
                     + (i3 - this->origin[ 2 ]))
                       * this->all[ 1 ]
                    + (i2 - this->origin[ 1 ]))
                      * this->all[ 0 ]
                   + (i1 - this->origin[ 0 ]);
         }
   };

   template<>
   struct dims<6> : dim_data<6> {
         size_t index_1d(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4, ssize_t i5, ssize_t i6) const {
            return (((((i6 - this->origin[ 5 ]) * this->all[ 4 ] + (i5 - this->origin[ 4 ])) * this->all[ 3 ]
                      + (i4 - this->origin[ 3 ]))
                        * this->all[ 2 ]
                     + (i3 - this->origin[ 2 ]))
                       * this->all[ 1 ]
                    + (i2 - this->origin[ 1 ]))
                      * this->all[ 0 ]
                   + (i1 - this->origin[ 0 ]);
         }
   };

   struct dim_buffer : dims<arr_dim_max> {
         size_t actual_number_of_dimensions;

         dim_buffer() : actual_number_of_dimensions(0) {}

         dim_buffer(dim_buffer const &other) : actual_number_of_dimensions(other.actual_number_of_dimensions) {
            std::copy(other.all, other.all + actual_number_of_dimensions, all);
            std::copy(other.origin, other.origin + actual_number_of_dimensions, origin);
         }

         template<size_t BufferNdims>
         dim_buffer(dims<BufferNdims> const &source) : actual_number_of_dimensions(BufferNdims) {
            this->set_dims(source);
         }

         template<unsigned I>
         void set_origin_all_star() {
            actual_number_of_dimensions = (I + 1);
            all[ I ] = size_t_max;
            origin[ I ] = 1;
         }

         template<unsigned I>
         void set_origin_all(ssize_t first, star_type const &) {
            actual_number_of_dimensions = (I + 1);
            all[ I ] = size_t_max - 1;
            origin[ I ] = first;
         }

         template<unsigned I>
         void set_origin_all(ssize_t first, ssize_t last) {
            actual_number_of_dimensions = (I + 1);
            all[ I ] = last - first + 1;
            origin[ I ] = first;
         }

         size_t actual_size_1d() const { return size_1d(actual_number_of_dimensions); }

         template<size_t MaxNdims>
         size_t actual_index_1d(arr_index_data<MaxNdims> const &arr_index) const {
            TBXX_ASSERT(arr_index.actual_number_of_dimensions == actual_number_of_dimensions);
            size_t i = actual_number_of_dimensions;
            if (i == 0)
               return 0;
            i--;
            size_t result = arr_index.elems[ i ] - origin[ i ];
            while (i != 0) {
               i--;
               result *= all[ i ];
               result += arr_index.elems[ i ] - origin[ i ];
            }
            return result;
         }
   };

   struct dim_chain : dim_buffer {
         dim_chain(star_type const &) { this->set_origin_all_star<0>(); }

         dim_chain(size_t n) { this->set_origin_all<0>(1, n); }

         dim_chain(ssize_t first, star_type const &) { this->set_origin_all<0>(first, star_type()); }

         dim_chain(ssize_t first, ssize_t last) { this->set_origin_all<0>(first, last); }

         dim_chain &dim2(star_type const &) {
            this->set_origin_all_star<1>();
            return *this;
         }

         dim_chain &dim2(size_t n) {
            this->set_origin_all<1>(1, n);
            return *this;
         }

         dim_chain &dim2(ssize_t first, star_type const &) {
            this->set_origin_all<1>(first, star_type());
            return *this;
         }

         dim_chain &dim2(ssize_t first, ssize_t last) {
            this->set_origin_all<1>(first, last);
            return *this;
         }

         dim_chain &dim3(star_type const &) {
            this->set_origin_all_star<2>();
            return *this;
         }

         dim_chain &dim3(size_t n) {
            this->set_origin_all<2>(1, n);
            return *this;
         }

         dim_chain &dim3(ssize_t first, star_type const &) {
            this->set_origin_all<2>(first, star_type());
            return *this;
         }

         dim_chain &dim3(ssize_t first, ssize_t last) {
            this->set_origin_all<2>(first, last);
            return *this;
         }

         dim_chain &dim4(star_type const &) {
            this->set_origin_all_star<3>();
            return *this;
         }

         dim_chain &dim4(size_t n) {
            this->set_origin_all<3>(1, n);
            return *this;
         }

         dim_chain &dim4(ssize_t first, star_type const &) {
            this->set_origin_all<3>(first, star_type());
            return *this;
         }

         dim_chain &dim4(ssize_t first, ssize_t last) {
            this->set_origin_all<3>(first, last);
            return *this;
         }

         dim_chain &dim5(star_type const &) {
            this->set_origin_all_star<4>();
            return *this;
         }

         dim_chain &dim5(size_t n) {
            this->set_origin_all<4>(1, n);
            return *this;
         }

         dim_chain &dim5(ssize_t first, star_type const &) {
            this->set_origin_all<4>(first, star_type());
            return *this;
         }

         dim_chain &dim5(ssize_t first, ssize_t last) {
            this->set_origin_all<4>(first, last);
            return *this;
         }

         dim_chain &dim6(star_type const &) {
            this->set_origin_all_star<5>();
            return *this;
         }

         dim_chain &dim6(size_t n) {
            this->set_origin_all<5>(1, n);
            return *this;
         }

         dim_chain &dim6(ssize_t first, star_type const &) {
            this->set_origin_all<5>(first, star_type());
            return *this;
         }

         dim_chain &dim6(ssize_t first, ssize_t last) {
            this->set_origin_all<5>(first, last);
            return *this;
         }
   };

   inline dim_chain dim1(star_type const &) {
      return dim_chain(star_type());
   }

   inline dim_chain dim1(size_t n) {
      return dim_chain(n);
   }

   inline dim_chain dim1(ssize_t first, star_type const &) {
      return dim_chain(first, star_type());
   }

   inline dim_chain dim1(ssize_t first, ssize_t last) {
      return dim_chain(first, last);
   }

   inline dims<1> dimension(star_type const &) {
      dims<1> result;
      result.all[ 0 ] = size_t_max;
      result.origin[ 0 ] = 1;
      return result;
   }

   inline dims<1> dimension(size_t n) {
      dims<1> result;
      result.all[ 0 ] = n;
      result.origin[ 0 ] = 1;
      return result;
   }

   inline dims<2> dimension(size_t n1, star_type const &) {
      dims<2> result;
      result.all[ 0 ] = n1;
      result.all[ 1 ] = size_t_max;
      result.origin[ 0 ] = 1;
      result.origin[ 1 ] = 1;
      return result;
   }

   inline dims<2> dimension(size_t n1, size_t n2) {
      dims<2> result;
      result.all[ 0 ] = n1;
      result.all[ 1 ] = n2;
      result.origin[ 0 ] = 1;
      result.origin[ 1 ] = 1;
      return result;
   }

   inline dims<3> dimension(size_t n1, size_t n2, star_type const &) {
      dims<3> result;
      result.all[ 0 ] = n1;
      result.all[ 1 ] = n2;
      result.all[ 2 ] = size_t_max;
      result.origin[ 0 ] = 1;
      result.origin[ 1 ] = 1;
      result.origin[ 2 ] = 1;
      return result;
   }

   inline dims<3> dimension(size_t n1, size_t n2, size_t n3) {
      dims<3> result;
      result.all[ 0 ] = n1;
      result.all[ 1 ] = n2;
      result.all[ 2 ] = n3;
      result.origin[ 0 ] = 1;
      result.origin[ 1 ] = 1;
      result.origin[ 2 ] = 1;
      return result;
   }

   inline dims<4> dimension(size_t n1, size_t n2, size_t n3, star_type const &) {
      dims<4> result;
      result.all[ 0 ] = n1;
      result.all[ 1 ] = n2;
      result.all[ 2 ] = n3;
      result.all[ 3 ] = size_t_max;
      result.origin[ 0 ] = 1;
      result.origin[ 1 ] = 1;
      result.origin[ 2 ] = 1;
      result.origin[ 3 ] = 1;
      return result;
   }

   inline dims<4> dimension(size_t n1, size_t n2, size_t n3, size_t n4) {
      dims<4> result;
      result.all[ 0 ] = n1;
      result.all[ 1 ] = n2;
      result.all[ 2 ] = n3;
      result.all[ 3 ] = n4;
      result.origin[ 0 ] = 1;
      result.origin[ 1 ] = 1;
      result.origin[ 2 ] = 1;
      result.origin[ 3 ] = 1;
      return result;
   }

   inline dims<5> dimension(size_t n1, size_t n2, size_t n3, size_t n4, star_type const &) {
      dims<5> result;
      result.all[ 0 ] = n1;
      result.all[ 1 ] = n2;
      result.all[ 2 ] = n3;
      result.all[ 3 ] = n4;
      result.all[ 4 ] = size_t_max;
      result.origin[ 0 ] = 1;
      result.origin[ 1 ] = 1;
      result.origin[ 2 ] = 1;
      result.origin[ 3 ] = 1;
      result.origin[ 4 ] = 1;
      return result;
   }

   inline dims<5> dimension(size_t n1, size_t n2, size_t n3, size_t n4, size_t n5) {
      dims<5> result;
      result.all[ 0 ] = n1;
      result.all[ 1 ] = n2;
      result.all[ 2 ] = n3;
      result.all[ 3 ] = n4;
      result.all[ 4 ] = n5;
      result.origin[ 0 ] = 1;
      result.origin[ 1 ] = 1;
      result.origin[ 2 ] = 1;
      result.origin[ 3 ] = 1;
      result.origin[ 4 ] = 1;
      return result;
   }

   inline dims<6> dimension(size_t n1, size_t n2, size_t n3, size_t n4, size_t n5, star_type const &) {
      dims<6> result;
      result.all[ 0 ] = n1;
      result.all[ 1 ] = n2;
      result.all[ 2 ] = n3;
      result.all[ 3 ] = n4;
      result.all[ 4 ] = n5;
      result.all[ 5 ] = size_t_max;
      result.origin[ 0 ] = 1;
      result.origin[ 1 ] = 1;
      result.origin[ 2 ] = 1;
      result.origin[ 3 ] = 1;
      result.origin[ 4 ] = 1;
      result.origin[ 5 ] = 1;
      return result;
   }

   inline dims<6> dimension(size_t n1, size_t n2, size_t n3, size_t n4, size_t n5, size_t n6) {
      dims<6> result;
      result.all[ 0 ] = n1;
      result.all[ 1 ] = n2;
      result.all[ 2 ] = n3;
      result.all[ 3 ] = n4;
      result.all[ 4 ] = n5;
      result.all[ 5 ] = n6;
      result.origin[ 0 ] = 1;
      result.origin[ 1 ] = 1;
      result.origin[ 2 ] = 1;
      result.origin[ 3 ] = 1;
      result.origin[ 4 ] = 1;
      result.origin[ 5 ] = 1;
      return result;
   }

} // namespace fem

#endif // GUARD
// --  end: fem/dimension.hpp ----------------------
// -- start: fem/zero.hpp ----------------------
#ifndef FEM_ZERO_HPP
#define FEM_ZERO_HPP

// -- start: fem/data_type_star.hpp ----------------------
#ifndef FEM_DATA_TYPE_STAR_HPP
#define FEM_DATA_TYPE_STAR_HPP

// -- start: fem/utils/int_types.hpp ----------------------
#ifndef FEM_UTILS_INT_TYPES_HPP
#define FEM_UTILS_INT_TYPES_HPP

namespace fem {
   namespace utils {

      // int type names as used in boost/cstdint.hpp

      typedef signed char int8_t;
      typedef signed short int16_t;
      typedef signed int int32_t;
#if defined(__i386__) || defined(__ppc__) || defined(_MSC_VER)
      typedef signed long long int64_t;
#else
      typedef signed long int64_t;
#endif

   } // namespace utils
} // namespace fem

#endif // GUARD
// --  end: fem/utils/int_types.hpp ----------------------

namespace fem {

   typedef bool logical_star_1;

   typedef utils::int8_t integer_star_1;
   typedef utils::int16_t integer_star_2;
#if defined(_MSC_VER)
   typedef int integer_star_4;
#else
   typedef utils::int32_t integer_star_4;
#endif
   typedef utils::int64_t integer_star_8;

   typedef float real_star_4;
   typedef double real_star_8;
   typedef long double real_star_16;

   static const unsigned actual_sizeof_long_double =
#if defined(__i386__) || defined(__x86_64__)
      10
#else
      sizeof(long double)
#endif
      ;

} // namespace fem

#endif // GUARD
// --  end: fem/data_type_star.hpp ----------------------
#include <complex>

namespace fem {

   /*! Herb Sutter: Why Not Specialize Function Templates?
     C/C++ Users Journal, 19(7), July 2001.
     http://www.gotw.ca/publications/mill17.htm
   */
   template<typename T>
   struct zero_impl;

   template<typename T>
   T zero() {
      return zero_impl<T>::get();
   }

#define FEM_ZERO_IMPL(T, ZERO) \
   template<> \
   struct zero_impl<T> { \
         static T get() { \
            return ZERO; \
         } \
   };

   FEM_ZERO_IMPL(char, '\0')
   FEM_ZERO_IMPL(logical_star_1, false)
   FEM_ZERO_IMPL(integer_star_1, 0)
   FEM_ZERO_IMPL(integer_star_2, 0)
   FEM_ZERO_IMPL(integer_star_4, 0)
   FEM_ZERO_IMPL(integer_star_8, 0)
   FEM_ZERO_IMPL(real_star_4, 0.f)
   FEM_ZERO_IMPL(real_star_8, 0.)
   FEM_ZERO_IMPL(real_star_16, 0.L)
#define FEM_LOC std::complex<float>
   FEM_ZERO_IMPL(FEM_LOC, 0.f)
#undef FEM_LOC
#define FEM_LOC std::complex<double>
   FEM_ZERO_IMPL(FEM_LOC, 0.)
#undef FEM_LOC
#define FEM_LOC std::complex<long double>
   FEM_ZERO_IMPL(FEM_LOC, 0.L)
#undef FEM_LOC

   //      static const char byte0 = zero<char>();
   static const char char0 = zero<char>();
   static const bool bool0 = zero<bool>();
   static const int int0 = zero<int>();
   static const float float0 = zero<float>();
   //   static const double double0 = zero<double>();
   // static const long double long_double0 = zero<long double>();
   static const std::complex<float> cfloat0;
   static const std::complex<double> cdouble0;

   enum fill0_type { fill0 };
   enum no_fill0_type { no_fill0 };

} // namespace fem

#endif // GUARD
// --  end: fem/zero.hpp ----------------------
#include <cstring>

namespace fem {

   template<typename T, size_t Ndims = 1>
   struct arr_cref {
      protected:
         T const *elems_;
         dims<Ndims> dims_;

         arr_cref() {}

      public:
         arr_cref(T const &val) : elems_(&val) {}

         template<size_t OtherNdims>
         arr_cref(arr_cref<T, OtherNdims> const &other) : elems_(other.begin()) {}

         template<size_t OtherNdims, size_t BufferNdims>
         arr_cref(arr_cref<T, OtherNdims> const &other, dims<BufferNdims> const &dims) : elems_(other.begin()) {
            (*this)(dims);
         }

         template<size_t BufferNdims>
         arr_cref(T const &val, dims<BufferNdims> const &dims) : elems_(&val) {
            (*this)(dims);
         }

         T const *begin() const { return elems_; }

         T const *end() const { return elems_ + size_1d(); }

#if defined(_MSC_VER)
         // workaround, breaks const-correctness, but only on this platform
         operator T &() const { return const_cast<T *>(elems_)[ 0 ]; }
#else
         operator T const &() const { return elems_[ 0 ]; }
#endif

         template<size_t BufferNdims>
         void operator()(dims<BufferNdims> const &dims) {
            dims_.set_dims(dims);
         }

         size_t size_1d() const { return dims_.size_1d(); }

         T const &operator()(ssize_t i1) const { return elems_[ dims_.index_1d(i1) ]; }

         T const &operator()(ssize_t i1, ssize_t i2) const { return elems_[ dims_.index_1d(i1, i2) ]; }

         T const &operator()(ssize_t i1, ssize_t i2, ssize_t i3) const { return elems_[ dims_.index_1d(i1, i2, i3) ]; }

         T const &operator()(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4) const {
            return elems_[ dims_.index_1d(i1, i2, i3, i4) ];
         }

         T const &operator()(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4, ssize_t i5) const {
            return elems_[ dims_.index_1d(i1, i2, i3, i4, i5) ];
         }

         T const &operator()(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4, ssize_t i5, ssize_t i6) const {
            return elems_[ dims_.index_1d(i1, i2, i3, i4, i5, i6) ];
         }
   };

   template<typename T, size_t Ndims = 1>
   struct arr_ref : arr_cref<T, Ndims> {
      protected:
         arr_ref() {}

      public:
         arr_ref(T &val) : arr_cref<T, Ndims>(val) {}

         template<size_t OtherNdims>
         arr_ref(arr_ref<T, OtherNdims> const &other) : arr_cref<T, Ndims>(other) {}

         template<size_t BufferNdims>
         arr_ref(T &val, dims<BufferNdims> const &dims) : arr_cref<T, Ndims>(val, dims) {}

         template<size_t BufferNdims>
         arr_ref(T &val, dims<BufferNdims> const &dims, no_fill0_type const &) : arr_cref<T, Ndims>(val, dims) {}

         template<size_t BufferNdims>
         arr_ref(T &val, dims<BufferNdims> const &dims, fill0_type const &) : arr_cref<T, Ndims>(val, dims) {
            std::memset(this->begin(), 0, this->dims_.size_1d() * sizeof(T));
         }

         T *begin() const { return const_cast<T *>(this->elems_); }

#if !defined(_MSC_VER)
         operator T &() const { return *(this->begin()); }
#endif

         template<size_t BufferNdims>
         void operator()(dims<BufferNdims> const &dims) {
            arr_cref<T, Ndims>::operator()(dims);
         }

         T &operator()(ssize_t i1) const { return this->begin()[ this->dims_.index_1d(i1) ]; }

         T &operator()(ssize_t i1, ssize_t i2) const { return this->begin()[ this->dims_.index_1d(i1, i2) ]; }

         T &operator()(ssize_t i1, ssize_t i2, ssize_t i3) const {
            return this->begin()[ this->dims_.index_1d(i1, i2, i3) ];
         }

         T &operator()(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4) const {
            return this->begin()[ this->dims_.index_1d(i1, i2, i3, i4) ];
         }

         T &operator()(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4, ssize_t i5) const {
            return this->begin()[ this->dims_.index_1d(i1, i2, i3, i4, i5) ];
         }

         T &operator()(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4, ssize_t i5, ssize_t i6) const {
            return this->begin()[ this->dims_.index_1d(i1, i2, i3, i4, i5, i6) ];
         }
   };

} // namespace fem

#endif // GUARD
// --  end: fem/arr_ref.hpp ----------------------

namespace fem {

   template<typename T, size_t Ndims = 1>
   struct arr : arr_ref<T, Ndims> {
      private:
         arr(arr const &);

         arr const &operator=(arr const &);

      public:
         template<size_t BufferNdims>
         explicit arr(dims<BufferNdims> const &dims) : arr_ref<T, Ndims>(*(new T[ dims.size_1d(Ndims) ]), dims) {}

         template<size_t BufferNdims>
         arr(dims<BufferNdims> const &dims, no_fill0_type const &) :
             arr_ref<T, Ndims>(*(new T[ dims.size_1d(Ndims) ]), dims, no_fill0) {}

         template<size_t BufferNdims>
         arr(dims<BufferNdims> const &dims, fill0_type const &) :
             arr_ref<T, Ndims>(*(new T[ dims.size_1d(Ndims) ]), dims, fill0) {}

         ~arr() { delete[] this->elems_; }

         operator T &() { return *this->begin(); }
   };

} // namespace fem

#endif // GUARD
// --  end: fem/arr.hpp ----------------------
// -- start: fem/str_arr_ref.hpp ----------------------
#ifndef FEM_STR_ARR_REF_HPP
#define FEM_STR_ARR_REF_HPP

// -- repeat not included:  fem/arr_ref.hpp ----------------------
// -- start: fem/str.hpp ----------------------
#ifndef FEM_STR_HPP
#define FEM_STR_HPP

// -- start: fem/str_ref.hpp ----------------------
#ifndef FEM_STR_REF_HPP
#define FEM_STR_REF_HPP

// -- start: fem/utils/misc.hpp ----------------------
#ifndef FEM_UTILS_MISC_HPP
#define FEM_UTILS_MISC_HPP

// -- repeat not included:  fem/size_t.hpp ----------------------

namespace fem {
   namespace utils {

      template<typename T>
      struct hide {
            T hidden;
      };

      struct noncopyable // clone of boost::noncopyable
      {
         protected:
            noncopyable() {}

         private:
            noncopyable(noncopyable const &);
            noncopyable const &operator=(noncopyable const &);
      };

      template<typename T, size_t SmallSize = 256>
      struct simple_buffer : noncopyable {
            T small_space[ SmallSize ];
            T *space;

            explicit simple_buffer(size_t size) : space(size <= SmallSize ? small_space : new T[ size ]) {}

            ~simple_buffer() {
               if (space != small_space)
                  delete[] space;
            }
      };

      // simlar to std::auto_ptr, but all member functions const
      template<typename T>
      struct slick_ptr {
         protected:
            mutable T *ptr;

         public:
            explicit slick_ptr(T *p = 0) : ptr(p) {}

            slick_ptr(slick_ptr const &other) : ptr(other.release()) {}

            slick_ptr &operator=(slick_ptr const &other) {
               reset(other.release());
               return *this;
            }

            ~slick_ptr() { delete ptr; }

            void reset(T *p = 0) const {
               if (p != ptr)
                  slick_ptr<T>(p).swap(*this);
            }

            T *release() const {
               T *result = ptr;
               ptr = 0;
               return result;
            }

            T *get() const { return ptr; }

            T *operator->() const { return ptr; }

            T &operator*() const { return *ptr; }

            operator bool() const { return (ptr != 0); }

            void swap(slick_ptr const &other) const {
               T *tmp = other.ptr;
               other.ptr = ptr;
               ptr = tmp;
            }
      };

   } // namespace utils
} // namespace fem

#endif // GUARD
// --  end: fem/utils/misc.hpp ----------------------
// -- start: fem/utils/string.hpp ----------------------
#ifndef FEM_UTILS_STRING_HPP
#define FEM_UTILS_STRING_HPP

// -- repeat not included:  fem/size_t.hpp ----------------------
// -- start: fem/utils/char.hpp ----------------------
#ifndef FEM_UTILS_CHAR_HPP
#define FEM_UTILS_CHAR_HPP

namespace fem {
   namespace utils {

      inline bool is_end_of_line(int c) {
         return (c == '\n');
      }

      inline bool is_whitespace(int c) {
         return (c == ' ' || c == '\t' || c == '\r' || is_end_of_line(c));
      }

      //! Assumes ASCII or similar.
      inline bool is_digit(int c) {
         return (c >= '0' && c <= '9');
      }

      //! Assumes ASCII or similar.
      inline int digit_as_int(int c) {
         return c - '0';
      }

      inline char int_as_digit(int i) {
         return "0123456789"[ i ];
      }

      //! To avoid locale environment surprises (assumes ASCII or similar).
      inline int to_lower(int c) {
         if (c < 'A')
            return c;
         if (c > 'Z')
            return c;
         return c + ('a' - 'A');
      }

      //! To avoid locale environment surprises (assumes ASCII or similar).
      inline int to_upper(int c) {
         if (c < 'a')
            return c;
         if (c > 'z')
            return c;
         return c - ('a' - 'A');
      }

      //! To avoid locale environment surprises (assumes ASCII or similar).
      inline bool is_upper_a_through_z(int c) {
         return (c >= 'A' && c <= 'Z');
      }

      //! To avoid locale environment surprises (assumes ASCII or similar).
      inline bool is_lower_a_through_z(int c) {
         return (c >= 'a' && c <= 'z');
      }

      //! To avoid locale environment surprises (assumes ASCII or similar).
      inline bool is_a_through_z(int c) {
         return (is_upper_a_through_z(c) || is_lower_a_through_z(c));
      }

   } // namespace utils
} // namespace fem

#endif // GUARD
// --  end: fem/utils/char.hpp ----------------------
// -- repeat not included:  tbxx/error_utils.hpp ----------------------
#include <algorithm>
#include <cstring>

namespace fem {
   namespace utils {

      inline bool starts_with(char const *str, unsigned start, unsigned stop, char const *substr) {
         for (unsigned j = start; j < stop;) {
            if (*substr == '\0')
               break;
            if (str[ j++ ] != *substr++)
               return false;
         }
         return (start != stop);
      }

      inline bool ends_with_char(std::string const &str, int c) {
         unsigned i = str.size();
         if (i == 0)
            return false;
         return (str[ i - 1 ] == c);
      }

      // compare with fable/__init__.py
      inline int unsigned_integer_scan(char const *code, unsigned start, unsigned stop) {
         unsigned i = start;
         for (; i < stop; i++) {
            int c = code[ i ];
            if (!is_digit(c))
               break;
         }
         if (i == start)
            return -1;
         return i;
      }

      //! Assumes ASCII or similar.
      inline unsigned unsigned_integer_value(char const *str, unsigned start, unsigned stop) {
         unsigned result = 0;
         unsigned i = start;
         for (; i < stop; i++) {
            result *= 10;
            result += (str[ i ] - '0');
         }
         return result;
      }

      inline unsigned unsigned_integer_value(char const *str, unsigned stop) {
         return unsigned_integer_value(str, 0, stop);
      }

      inline int signed_integer_value(char const *str, unsigned start, unsigned stop) {
         bool negative;
         if (str[ start ] == '-') {
            negative = true;
            start++;
         }
         else {
            negative = false;
            if (str[ start ] == '+')
               start++;
         }
         int result = unsigned_integer_value(str, start, stop);
         if (negative)
            result *= -1;
         return result;
      }

      inline void copy_with_blank_padding(char const *src, size_t src_size, char *dest, size_t dest_size) {
         if (dest_size < src_size) {
            std::memmove(dest, src, dest_size);
         }
         else {
            std::memmove(dest, src, src_size);
            for (size_t i = src_size; i < dest_size; i++) {
               dest[ i ] = ' ';
            }
         }
      }

      inline void copy_with_blank_padding(char const *src, char *dest, size_t dest_size) {
         size_t i;
         for (i = 0; i < dest_size && *src != '\0'; i++) {
            dest[ i ] = *src++;
         }
         for (; i < dest_size; i++) {
            dest[ i ] = ' ';
         }
      }

      inline bool string_eq(char const *lhs, size_t lhs_size, char const *rhs, size_t rhs_size) {
         static const char blank = ' ';
         if (lhs_size < rhs_size) {
            return string_eq(rhs, rhs_size, lhs, lhs_size);
         }
         if (std::memcmp(lhs, rhs, rhs_size) != 0)
            return false;
         for (size_t i = rhs_size; i < lhs_size; i++) {
            if (lhs[ i ] != blank)
               return false;
         }
         return true;
      }

      inline bool string_eq(char const *lhs, size_t lhs_size, char const *rhs) {
         static const char blank = ' ';
         for (size_t i = 0; i < lhs_size; i++) {
            if (*rhs == '\0') {
               for (; i < lhs_size; i++) {
                  if (lhs[ i ] != blank)
                     return false;
               }
               return true;
            }
            if (*rhs++ != lhs[ i ])
               return false;
         }
         while (*rhs != '\0') {
            if (*rhs++ != blank)
               return false;
         }
         return true;
      }

      inline int string_compare_lexical(char const *lhs, size_t lhs_size, char const *rhs, size_t rhs_size) {
         size_t n = std::max(lhs_size, rhs_size);
         for (size_t i = 0; i < n; i++) {
            char l = (i < lhs_size ? lhs[ i ] : ' ');
            char r = (i < rhs_size ? rhs[ i ] : ' ');
            if (l < r)
               return -1;
            if (l > r)
               return 1;
         }
         return 0;
      }

      inline size_t find_leading_blank_padding(char const *str, size_t stop) {
         size_t i = 0;
         while (i != stop) {
            if (str[ i ] != ' ')
               break;
            i++;
         }
         return i;
      }

      inline size_t find_trailing_blank_padding(char const *str, size_t stop) {
         size_t i = stop;
         while (i != 0) {
            i--;
            if (str[ i ] != ' ') {
               i++;
               break;
            }
         }
         return i;
      }

      inline size_t_2 find_leading_and_trailing_blank_padding(char const *str, size_t stop) {
         return size_t_2(find_leading_blank_padding(str, stop), find_trailing_blank_padding(str, stop));
      }

      inline std::string strip_leading_and_trailing_blank_padding(std::string const &str) {
         size_t_2 indices = find_leading_and_trailing_blank_padding(str.data(), str.size());
         if (indices.elems[ 0 ] == 0 && indices.elems[ 1 ] == str.size()) {
            return str;
         }
         return std::string(str.data() + indices.elems[ 0 ], indices.elems[ 1 ] - indices.elems[ 0 ]);
      }

      inline std::string to_lower(std::string const &str) {
         std::string result = str;
         size_t n = str.size();
         for (size_t i = 0; i < n; i++) {
            result[ i ] = to_lower(result[ i ]);
         }
         return result;
      }

      inline int keyword_index(char const *valid_vals[], std::string const &val, char const *throw_info = 0) {
         std::string val_norm = to_lower(strip_leading_and_trailing_blank_padding(val));
         for (int i = 0; valid_vals[ i ] != 0; i++) {
            if (std::strcmp(valid_vals[ i ], val_norm.c_str()) == 0) {
               return i;
            }
         }
         if (throw_info != 0) {
            std::ostringstream o;
            o << throw_info << ": invalid keyword: \"" << val << "\"";
            throw std::runtime_error(o.str());
         }
         return -1;
      }

      //! Assumes ASCII or similar.
      inline std::string format_char_for_display(int c) {
         std::ostringstream o;
         bool printable = (c >= ' ' && c <= '~');
         if (printable) {
            if (c == '"') {
               o << "'\"' (double quote, ";
            }
            else if (c == '\'') {
               o << "\"'\" (single quote, ";
            }
            else {
               o << "\"" << static_cast<char>(c) << "\" (";
            }
         }
         o << "ordinal=" << (c < 0 ? c + 256 : c);
         if (printable)
            o << ")";
         return o.str();
      }

      inline void string_reverse_in_place(char *s, size_t s_size) {
         if (s_size == 0)
            return;
         size_t i = 0;
         size_t j = s_size - 1;
         while (i < j) {
            std::swap(s[ i ], s[ j ]);
            i++;
            j--;
         }
      }

      //! Assumes ASCII or similar.
      inline int int_to_string(char *buffer, int buffer_size, int width, int value, int left_padding_character = ' ') {
         int i = 0;
         while (value != 0) {
            if (i == buffer_size)
               return -1;
            buffer[ i++ ] = int_as_digit(value % 10);
            value /= 10;
         }
         while (i < width)
            buffer[ i++ ] = left_padding_character;
         string_reverse_in_place(buffer, i);
         return i;
      }

      template<typename VectorOfStringType>
      unsigned split_comma_separated(VectorOfStringType &result, char const *c_str) {
         for (unsigned i = 0;; i++) {
            char c = c_str[ i ];
            if (c == '\0')
               return i;
            if (c == ',' || is_whitespace(c))
               continue;
            for (unsigned i_start = i++;; i++) {
               char c = c_str[ i ];
               if (c == '\0' || c == ',' || is_whitespace(c)) {
                  result.push_back(std::string(c_str + i_start, i - i_start));
                  if (c == '\0')
                     return i;
                  break;
               }
            }
         }
      }

   } // namespace utils
} // namespace fem

#endif // GUARD
// --  end: fem/utils/string.hpp ----------------------

namespace fem {

   struct str_cref {
      protected:
         char const *elems_;
         int len_;

      public:
         str_cref(char const *elems, int len) : elems_(elems), len_(len) {}

         str_cref(char const *elems) : elems_(elems), len_(std::strlen(elems)) {}

         str_cref(str_cref const &other, int len) : elems_(other.elems()), len_(len) {}

         char const *elems() const { return elems_; }

         int len() const { return len_; }

         operator std::string() const { return std::string(elems_, len_); }

         char const &operator[](int i) const { return elems_[ i ]; }

         str_cref operator()(int first, int last) const { return str_cref(elems_ + first - 1, last - first + 1); }

         bool operator==(char const *rhs) const { return utils::string_eq(elems_, len_, rhs); }

         bool operator!=(char const *rhs) const { return !((*this) == rhs); }

         bool operator==(str_cref const &rhs) const { return utils::string_eq(elems_, len_, rhs.elems_, rhs.len_); }

         bool operator!=(str_cref const &rhs) const { return !((*this) == rhs); }

         bool operator<(str_cref const &rhs) const {
            return (utils::string_compare_lexical(elems(), len(), rhs.elems(), rhs.len()) < 0);
         }

         bool operator<=(str_cref const &rhs) const {
            return (utils::string_compare_lexical(elems(), len(), rhs.elems(), rhs.len()) <= 0);
         }

         bool operator>(str_cref const &rhs) const {
            return (utils::string_compare_lexical(elems(), len(), rhs.elems(), rhs.len()) > 0);
         }

         bool operator>=(str_cref const &rhs) const {
            return (utils::string_compare_lexical(elems(), len(), rhs.elems(), rhs.len()) >= 0);
         }
   };

   int inline len(str_cref const &s) {
      return s.len();
   }

   bool inline operator==(char const *lhs, str_cref const &rhs) {
      return (rhs == lhs);
   }

   bool inline operator!=(char const *lhs, str_cref const &rhs) {
      return (rhs != lhs);
   }

   struct str_addends {
         str_cref lhs, rhs;
         std::string sum;

         str_addends(str_cref const &lhs_, str_cref const &rhs_) : lhs(lhs_), rhs(rhs_) {}

         operator str_cref() {
            int sum_len = lhs.len() + rhs.len();
            if (sum_len != 0 && sum.size() == 0) {
               sum = std::string(lhs) + std::string(rhs);
            }
            return str_cref(sum.data(), sum_len);
         }
   };

   inline str_addends operator+(str_cref const &lhs, str_cref const &rhs) {
      return str_addends(lhs, rhs);
   }

   template<int StrLen>
   struct str;

   struct str_ref : str_cref {
         str_ref(char *elems, int len) : str_cref(elems, len) {}

         str_ref(str_ref const &other, int len) : str_cref(other, len) {}

         char *elems() const { return const_cast<char *>(elems_); }

         char &operator[](int i) const { return elems()[ i ]; }

         void operator=(char const *rhs) { utils::copy_with_blank_padding(rhs, elems(), len()); }

         void operator=(std::string const &rhs) {
            utils::copy_with_blank_padding(rhs.data(), rhs.size(), elems(), len());
         }

         void operator=(str_cref const &rhs) { utils::copy_with_blank_padding(rhs.elems(), rhs.len(), elems(), len()); }

         void operator=(str_ref const &rhs) { (*this) = static_cast<str_cref>(rhs); }

         void operator=(str_addends const &addends) {
            int ll = addends.lhs.len();
            int n_from_rhs = len() - ll;
            if (n_from_rhs <= 0) {
               std::memmove(elems(), addends.lhs.elems(), len());
            }
            else {
               utils::simple_buffer<char> buffer((len()));
               std::memcpy(buffer.space, addends.lhs.elems(), ll);
               utils::copy_with_blank_padding(addends.rhs.elems(), addends.rhs.len(), buffer.space + ll, n_from_rhs);
               std::memcpy(elems(), buffer.space, len());
            }
         }

         template<int StrLen>
         void operator=(str<StrLen> const &rhs);

         str_ref operator()(int first, int last) const { return str_ref(elems() + first - 1, last - first + 1); }
   };

} // namespace fem

#endif // GUARD
// --  end: fem/str_ref.hpp ----------------------
// -- repeat not included:  fem/zero.hpp ----------------------

namespace fem {

   template<int StrLen>
   struct str {
         char elems[ StrLen ];

         str() { std::memset(elems, '\0', StrLen); }

         str(char c) { std::memset(elems, c, StrLen); }

         str(char const *s) { utils::copy_with_blank_padding(s, elems, StrLen); }

         str(str_cref const &other) { utils::copy_with_blank_padding(other.elems(), other.len(), elems, StrLen); }

         template<int OtherStrLen>
         str(str<OtherStrLen> const &other) {
            utils::copy_with_blank_padding(other.elems, OtherStrLen, elems, StrLen);
         }

         str(str_addends const &addends) { str_ref(*this) = addends; }

         operator str_cref() const { return str_cref(elems, StrLen); }

         operator str_ref() { return str_ref(elems, StrLen); }

         void operator=(char const *rhs) { str_ref(*this) = rhs; }

         void operator=(std::string const &rhs) { str_ref(*this) = rhs; }

         void operator=(str_cref const &rhs) { str_ref(*this) = rhs; }

         template<int OtherStrLen>
         void operator=(str<OtherStrLen> const &rhs) {
            str_ref(*this) = str_cref(rhs);
         }

         void operator=(str_addends const &addends) { str_ref(*this) = addends; }

         operator std::string() const { return std::string(elems, StrLen); }

         bool operator==(char const *rhs) const { return (str_cref(*this) == rhs); }

         bool operator!=(char const *rhs) const { return (str_cref(*this) != rhs); }

         bool operator==(str_cref const &rhs) const { return (str_cref(*this) == rhs); }

         bool operator!=(str_cref const &rhs) const { return (str_cref(*this) != rhs); }

         bool operator<(str_cref const &rhs) const { return (str_cref(*this) < rhs); }

         bool operator<=(str_cref const &rhs) const { return (str_cref(*this) <= rhs); }

         bool operator>(str_cref const &rhs) const { return (str_cref(*this) > rhs); }

         bool operator>=(str_cref const &rhs) const { return (str_cref(*this) >= rhs); }

         str_cref operator()(int first, int last) const { return str_cref(*this)(first, last); }

         str_ref operator()(int first, int last) { return str_ref(*this)(first, last); }
   };

   template<int StrLen>
   inline void str_ref::operator=(str<StrLen> const &rhs) {
      utils::copy_with_blank_padding(rhs.elems, StrLen, elems(), len());
   }

   template<int StrLen>
   struct zero_impl<str<StrLen>> {
         static str<StrLen> get() { return str<StrLen>(); }
   };

} // namespace fem

#endif // GUARD
// --  end: fem/str.hpp ----------------------

namespace fem {

   template<size_t Ndims = 1>
   struct str_arr_cref {
      protected:
         char const *elems_;
         int len_;
         dims<Ndims> dims_;

         str_arr_cref() {}

      public:
         str_arr_cref(char const *elems, int len) : elems_(elems), len_(len) {}

         template<size_t BufferNdims>
         str_arr_cref(char const *elems, int len, dims<BufferNdims> const &dims) : elems_(elems), len_(len) {
            (*this)(dims);
         }

         str_arr_cref(str_cref const &other) : elems_(other.elems()), len_(other.len()) {}

         template<size_t BufferNdims>
         str_arr_cref(str_cref const &other, dims<BufferNdims> const &dims) : elems_(other.elems()), len_(other.len()) {
            (*this)(dims);
         }

         str_arr_cref(str_cref const &other, int len) : elems_(other.elems()), len_(len) {}

         template<size_t BufferNdims>
         str_arr_cref(str_cref const &other, int len, dims<BufferNdims> const &dims) :
             elems_(other.elems()), len_(len) {
            (*this)(dims);
         }

         template<int StrLen>
         str_arr_cref(str<StrLen> const &other) : elems_(other.elems), len_(StrLen) {}

         template<int StrLen, size_t BufferNdims>
         str_arr_cref(str<StrLen> const &other, dims<BufferNdims> const &dims) : elems_(other.elems), len_(StrLen) {
            (*this)(dims);
         }

         template<int StrLen, size_t OtherNdims>
         str_arr_cref(arr_cref<str<StrLen>, OtherNdims> const &other) : elems_(other.begin()->elems), len_(StrLen) {}

         template<int StrLen, size_t OtherNdims, size_t BufferNdims>
         str_arr_cref(arr_cref<str<StrLen>, OtherNdims> const &other, dims<BufferNdims> const &dims) :
             elems_(other.begin()->elems), len_(StrLen) {
            (*this)(dims);
         }

         template<size_t OtherNdims, size_t BufferNdims>
         str_arr_cref(str_arr_cref<OtherNdims> const &other, dims<BufferNdims> const &dims) :
             elems_(other.begin()), len_(other.len()) {
            (*this)(dims);
         }

         char const *begin() const { return elems_; }

         int len() const { return len_; }

         template<size_t BufferNdims>
         void operator()(dims<BufferNdims> const &dims) {
            dims_.set_dims(dims);
         }

         size_t size_1d() const { return dims_.size_1d(); }

         str_cref operator[](size_t i) const { return str_cref(&elems_[ len_ * i ], len_); }

         str_cref operator()(ssize_t i1) const { return str_cref(&elems_[ len_ * dims_.index_1d(i1) ], len_); }

         str_cref operator()(ssize_t i1, ssize_t i2) const {
            return str_cref(&elems_[ len_ * dims_.index_1d(i1, i2) ], len_);
         }

         str_cref operator()(ssize_t i1, ssize_t i2, ssize_t i3) const {
            return str_cref(&elems_[ len_ * dims_.index_1d(i1, i2, i3) ], len_);
         }

         str_cref operator()(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4) const {
            return str_cref(&elems_[ len_ * dims_.index_1d(i1, i2, i3, i4) ], len_);
         }

         str_cref operator()(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4, ssize_t i5) const {
            return str_cref(&elems_[ len_ * dims_.index_1d(i1, i2, i3, i4, i5) ], len_);
         }

         str_cref operator()(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4, ssize_t i5, ssize_t i6) const {
            return str_cref(&elems_[ len_ * dims_.index_1d(i1, i2, i3, i4, i5, i6) ], len_);
         }
   };

   template<size_t Ndims = 1>
   struct str_arr_ref : str_arr_cref<Ndims> {
      protected:
         str_arr_ref() {}

      public:
         str_arr_ref(char *elems, int len) : str_arr_cref<Ndims>(elems, len) {}

         template<size_t BufferNdims>
         str_arr_ref(char const *elems, int len, dims<BufferNdims> const &dims) : str_arr_cref<Ndims>(elems, len) {
            (*this)(dims);
         }

         str_arr_ref(str_ref const &other) : str_arr_cref<Ndims>(other) {}

         template<size_t BufferNdims>
         str_arr_ref(str_ref const &other, dims<BufferNdims> const &dims) : str_arr_cref<Ndims>(other, dims) {}

         str_arr_ref(str_ref const &other, int len) : str_arr_cref<Ndims>(other, len) {}

         template<size_t BufferNdims>
         str_arr_ref(str_ref const &other, int len, dims<BufferNdims> const &dims) :
             str_arr_cref<Ndims>(other, len, dims) {}

         template<int StrLen>
         str_arr_ref(str<StrLen> const &other) : str_arr_cref<Ndims>(other) {}

         template<int StrLen, size_t OtherNdims>
         str_arr_ref(arr_ref<str<StrLen>, OtherNdims> const &other) : str_arr_cref<Ndims>(other) {}

         template<int StrLen, size_t BufferNdims>
         str_arr_ref(str<StrLen> const &other, int len, dims<BufferNdims> const &dims) :
             str_arr_cref<Ndims>(other.elems, len, dims) {}

         template<int StrLen, size_t BufferNdims>
         str_arr_ref(str<StrLen> const &other, dims<BufferNdims> const &dims) : str_arr_cref<Ndims>(other, dims) {}

         template<int StrLen, size_t BufferNdims>
         str_arr_ref(str<StrLen> const &other, dims<BufferNdims> const &dims, no_fill0_type const &) :
             str_arr_cref<Ndims>(other, dims) {}

         template<int StrLen, size_t BufferNdims>
         str_arr_ref(str<StrLen> const &other, dims<BufferNdims> const &dims, fill0_type const &) :
             str_arr_cref<Ndims>(other, dims) {
            std::memset(this->begin(), char0, StrLen * this->size_1d());
         }

         template<int StrLen, size_t OtherNdims, size_t BufferNdims>
         str_arr_ref(arr_ref<str<StrLen>, OtherNdims> const &other, dims<BufferNdims> const &dims) :
             str_arr_cref<Ndims>(other, dims) {}

         template<size_t OtherNdims, size_t BufferNdims>
         str_arr_ref(str_arr_ref<OtherNdims> const &other, dims<BufferNdims> const &dims) :
             str_arr_cref<Ndims>(other, dims) {}

         char *begin() const { return const_cast<char *>(this->elems_); }

         operator char *() const { return this->begin(); }

         template<size_t BufferNdims>
         void operator()(dims<BufferNdims> const &dims) {
            str_arr_cref<Ndims>::operator()(dims);
         }

         str_ref operator[](size_t i) const { return str_ref(&this->begin()[ this->len() * i ], this->len()); }

         str_ref operator()(ssize_t i1) const {
            return str_ref(&this->begin()[ this->len() * this->dims_.index_1d(i1) ], this->len());
         }

         str_ref operator()(ssize_t i1, ssize_t i2) const {
            return str_ref(&this->begin()[ this->len() * this->dims_.index_1d(i1, i2) ], this->len());
         }

         str_ref operator()(ssize_t i1, ssize_t i2, ssize_t i3) const {
            return str_ref(&this->begin()[ this->len() * this->dims_.index_1d(i1, i2, i3) ], this->len());
         }

         str_ref operator()(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4) const {
            return str_ref(&this->begin()[ this->len() * this->dims_.index_1d(i1, i2, i3, i4) ], this->len());
         }

         str_ref operator()(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4, ssize_t i5) const {
            return str_ref(&this->begin()[ this->len() * this->dims_.index_1d(i1, i2, i3, i4, i5) ], this->len());
         }

         str_ref operator()(ssize_t i1, ssize_t i2, ssize_t i3, ssize_t i4, ssize_t i5, ssize_t i6) const {
            return str_ref(&this->begin()[ this->len() * this->dims_.index_1d(i1, i2, i3, i4, i5, i6) ], this->len());
         }
   };

} // namespace fem

#endif // GUARD
// --  end: fem/str_arr_ref.hpp ----------------------
// -- repeat not included:  fem/utils/misc.hpp ----------------------

namespace fem {

   template<typename T>
   struct data_of_type : utils::noncopyable {
         T const *values;
         size_t values_size;
         size_t value_index;

         data_of_type(T const *values_, size_t values_size_) :
             values(values_), values_size(values_size_), value_index(0) {}

         ~data_of_type() { TBXX_ASSERT(value_index == values_size); }

#define FEM_LOC(V) \
   data_of_type &operator,(V &val) { \
      TBXX_ASSERT(value_index < values_size); \
      val = values[ value_index++ ]; \
      return *this; \
   }
         FEM_LOC(char)
         FEM_LOC(bool)
         FEM_LOC(int)
         FEM_LOC(float)
         FEM_LOC(double)
#undef FEM_LOC

         template<typename OtherT, size_t Ndims>
         data_of_type &operator,(arr_ref<OtherT, Ndims> &val) {
            size_t n = val.size_1d();
            OtherT *val_begin = val.begin();
            T v;
            for (size_t i = 0; i < n; i++) {
               (*this), v;
               val_begin[ i ] = static_cast<OtherT>(v);
            }
            return *this;
         }
   };

   struct data_of_type_str : utils::noncopyable {
         char const **values;
         size_t values_size;
         size_t value_index;

         data_of_type_str(char const **values_, size_t values_size_) :
             values(values_), values_size(values_size_), value_index(0) {}

         ~data_of_type_str() { TBXX_ASSERT(value_index == values_size); }

         data_of_type_str &operator,(str_ref val) {
            TBXX_ASSERT(value_index < values_size);
            val = values[ value_index++ ];
            return *this;
         }

         template<int StrLen, size_t Ndims>
         data_of_type_str &operator,(arr_ref<str<StrLen>, Ndims> &val) {
            size_t n = val.size_1d();
            str<StrLen> *val_begin = val.begin();
            for (size_t i = 0; i < n; i++)
               (*this), val_begin[ i ];
            return *this;
         }

         template<size_t Ndims>
         data_of_type_str &operator,(str_arr_ref<Ndims> &val) {
            size_t n = val.size_1d();
            for (size_t i = 0; i < n; i++)
               (*this), val[ i ];
            return *this;
         }
   };

} // namespace fem

#define FEM_VALUES_AND_SIZE values, sizeof(values) / sizeof(*values)

#endif // GUARD
// --  end: fem/data_of_type.hpp ----------------------
// -- repeat not included:  fem/error_utils.hpp ----------------------
// -- start: fem/format.hpp ----------------------
#ifndef FEM_FORMAT_HPP
#define FEM_FORMAT_HPP

// -- start: fem/io_exceptions.hpp ----------------------
#ifndef FEM_IO_EXCEPTIONS_HPP
#define FEM_IO_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace fem {

   struct io_err : std::runtime_error {
         explicit io_err(std::string const &msg) : std::runtime_error(msg) {}
   };

   struct read_end : std::runtime_error {
         explicit read_end(std::string const &msg) : std::runtime_error(msg) {}
   };

} // namespace fem

#endif // GUARD
// --  end: fem/io_exceptions.hpp ----------------------
// -- repeat not included:  fem/str_ref.hpp ----------------------
// -- repeat not included:  fem/utils/char.hpp ----------------------
// -- repeat not included:  fem/utils/misc.hpp ----------------------
// -- start: fem/utils/token.hpp ----------------------
#ifndef FEM_UTILS_TOKEN_HPP
#define FEM_UTILS_TOKEN_HPP

#include <string>

namespace fem {
   namespace utils {

      struct token {
            std::string type;
            std::string value;

            token(std::string type_, std::string value_) : type(type_), value(value_) {}

            token(std::string type_, char value_) : type(type_), value(1, value_) {}
      };

   } // namespace utils
} // namespace fem

#endif // GUARD
// --  end: fem/utils/token.hpp ----------------------
#include <cstring>
#include <stdexcept>
#include <vector>

namespace fem {
   namespace format {

      struct tokenizer : utils::noncopyable {
         protected:
            char *code;
            char *str_buf;
            unsigned stop;
            unsigned i;

         public:
            std::vector<utils::token> tokens;

            tokenizer(char const *fmt, unsigned fmt_stop) {
               utils::simple_buffer<char> buffer(fmt_stop * 2);
               code = buffer.space;
               str_buf = code + fmt_stop;
               stop = 0;
               for (i = 0; i < fmt_stop; i++) {
                  char c = fmt[ i ];
                  if (c == ' ' || c == '\t')
                     continue;
                  if (c != '\'' && c != '"') {
                     code[ stop++ ] = utils::to_lower(c);
                     continue;
                  }
                  code[ stop++ ] = c;
                  char opening_quote = c;
                  for (i++; i < fmt_stop; i++) {
                     c = fmt[ i ];
                     code[ stop++ ] = c;
                     if (c == opening_quote) {
                        if (i + 1 == fmt_stop)
                           break;
                        if (fmt[ i + 1 ] != c)
                           break;
                        i++;
                        code[ stop++ ] = c;
                     }
                  }
               }
               if (stop == 0) {
                  throw std::runtime_error("Invalid FORMAT specification: empty string");
               }
               stop--;
               if (code[ 0 ] != '(')
                  raise_invalid();
               if (code[ stop ] != ')')
                  raise_invalid();
               tokens.reserve(32); // uncritical; avoids reallocation in most cases
               i = 1;
               while (i < stop)
                  process();
               code = 0;
               str_buf = 0;
            }

         protected:
            void raise_invalid() {
               throw std::runtime_error("Invalid FORMAT specification: " + std::string(code, stop + 1));
            }

            bool starts_with(char const *substr, unsigned start) {
               return utils::starts_with(code, start, stop, substr);
            }

            int unsigned_integer_scan(unsigned start) { return utils::unsigned_integer_scan(code, start, stop); }

            void add_token(char const *type, unsigned start) {
               tokens.push_back(utils::token(type, std::string(&code[ start ], &code[ i ])));
            }

            void add_token_string() {
               unsigned str_size = 0;
               char opening_quote = code[ i ];
               for (i++; i < stop; i++) {
                  char c = code[ i ];
                  if (c == opening_quote) {
                     i++;
                     if (i == stop || code[ i ] != opening_quote) {
                        tokens.push_back(utils::token("string", std::string(str_buf, str_size)));
                        return;
                     }
                  }
                  str_buf[ str_size++ ] = c;
               }
               raise_invalid();
            }

            void process() {
               unsigned i_code = i;
               char c = code[ i_code ];
               if (c == ',') {
                  i++;
                  return;
               }
               if (c == 'x') {
                  i++;
                  add_token("format", i_code);
                  return;
               }
               if (std::strchr("():/$", c) != 0) {
                  i++;
                  add_token("op", i_code);
                  return;
               }
               if (c == '\'' || c == '"') {
                  add_token_string();
                  return;
               }
               if (c == '+' || c == '-') {
                  int j = unsigned_integer_scan(i_code + 1);
                  if (j < 0 || code[ j ] != 'p')
                     raise_invalid();
                  i = j + 1;
                  add_token("format", i_code);
                  return;
               }
               int j = unsigned_integer_scan(i_code);
               if (j > 0) {
                  i = j;
                  if (starts_with("h", j)) {
                     throw std::runtime_error(
                        "FATAL: Not supported: FORMAT Hollerith edit descriptor: " + std::string(code, stop + 1));
                  }
                  if (starts_with("x", i) || starts_with("p", i)) {
                     i++;
                     add_token("format", i_code);
                     return;
                  }
                  add_token("integer", i_code);
                  return;
               }
               if (std::strchr("defgiz", c) != 0) {
                  i++;
                  j = unsigned_integer_scan(i);
                  if (j > 0) {
                     if (starts_with(".", j)) {
                        j = unsigned_integer_scan(j + 1);
                        if (j < 0)
                           raise_invalid();
                     }
                     i = j;
                  }
                  add_token("format", i_code);
                  return;
               }
               if (c == 'a' || c == 'l') {
                  i++;
                  j = unsigned_integer_scan(i_code + 1);
                  if (j > 0)
                     i = j;
                  add_token("format", i_code);
                  return;
               }
               if (starts_with("bn", i_code) || starts_with("bz", i_code)) {
                  i += 2;
                  add_token("format", i_code);
                  return;
               }
               if (c == 's') {
                  i++;
                  if (starts_with("p", i) || starts_with("s", i)) {
                     i++;
                  }
                  add_token("format", i_code);
                  return;
               }
               if (c == 't') {
                  i++;
                  if (starts_with("l", i) || starts_with("r", i)) {
                     i++;
                  }
                  j = unsigned_integer_scan(i);
                  if (j < 0)
                     raise_invalid();
                  i = j;
                  add_token("format", i_code);
                  return;
               }
               raise_invalid();
            }
      };

      struct repeat_point {
            unsigned i_fmt;
            unsigned n;
            bool wait_for_closing_parenthesis;

            repeat_point(unsigned i_fmt_, unsigned n_, bool wait_for_closing_parenthesis_ = false) :
                i_fmt(i_fmt_), n(n_), wait_for_closing_parenthesis(wait_for_closing_parenthesis_) {}
      };

      struct token_loop {
            std::vector<utils::token> fmt_tokens;
            unsigned i_fmt;
            unsigned i_fmt_wrap;
            unsigned simple_repeat;
            std::vector<format::repeat_point> repeat_points;

            token_loop() : i_fmt(0), i_fmt_wrap(0), simple_repeat(0) {}

            token_loop(str_cref fmt) : i_fmt(0), i_fmt_wrap(0), simple_repeat(0) {
               format::tokenizer tz(fmt.elems(), fmt.len());
               fmt_tokens.swap(tz.tokens);
               repeat_points.reserve(32); // uncritical; avoids reallocation in most
            } // cases

            utils::token const *next_executable_token(bool final = false) {
               if (simple_repeat != 0) {
                  simple_repeat--;
                  i_fmt--;
                  return &fmt_tokens[ i_fmt++ ];
               }
               while (true) {
                  if (i_fmt == fmt_tokens.size()) {
                     if (final) {
                        return 0;
                     }
                     if (fmt_tokens.size() == 0) {
                        throw io_err("Empty format string but data editing requested.");
                     }
                     i_fmt = i_fmt_wrap;
                     static const utils::token op_slash("op", "/");
                     return &op_slash;
                  }
                  utils::token const *t = &fmt_tokens[ i_fmt++ ];
                  std::string const &tv = t->value;
                  if (t->type == "integer") {
                     if (i_fmt == fmt_tokens.size()) {
                        throw std::runtime_error("Trailing lone repeat count in format string.");
                     }
                     unsigned n = utils::unsigned_integer_value(tv.data(), tv.size());
                     if (n == 0) {
                        throw std::runtime_error("Zero repeat count in format string.");
                     }
                     t = &fmt_tokens[ i_fmt++ ];
                     if (t->type == "op" && t->value == "(") {
                        repeat_points.push_back(format::repeat_point(i_fmt, n));
                        if (repeat_points.size() == 1) {
                           i_fmt_wrap = i_fmt - 2;
                        }
                     }
                     else {
                        simple_repeat = n - 1;
                        return t;
                     }
                  }
                  else {
                     if (t->type == "op") {
                        char tv0 = tv[ 0 ];
                        if (tv0 == '(') {
                           repeat_points.push_back(format::repeat_point(i_fmt, 1));
                           if (repeat_points.size() == 1) {
                              i_fmt_wrap = i_fmt - 1;
                           }
                        }
                        else if (tv0 == ')') {
                           if (repeat_points.size() == 0) {
                              throw std::runtime_error("Unexpected closing parenthesis in format string.");
                           }
                           repeat_point &rp = repeat_points.back();
                           rp.n--;
                           if (rp.n == 0) {
                              repeat_points.pop_back();
                           }
                           else {
                              i_fmt = rp.i_fmt;
                           }
                        }
                        else {
                           return t;
                        }
                     }
                     else {
                        return t;
                     }
                  }
               }
            }
      };

   } // namespace format
} // namespace fem

#endif // GUARD
// --  end: fem/format.hpp ----------------------
// -- start: fem/intrinsics.hpp ----------------------
#ifndef FEM_INTRINSICS_HPP
#define FEM_INTRINSICS_HPP

// -- repeat not included:  fem/str_ref.hpp ----------------------
#include <cmath>

namespace fem {

   template<typename T>
   inline int if_arithmetic(T const &value) {
      if (value == 0)
         return 0;
      if (value > 0)
         return 1;
      return -1;
   }

   template<typename V, typename S>
   inline V sign(V const &value, S const &sign_source) {
      if (sign_source < 0) {
         if (value > 0)
            return -value;
      }
      else if (value < 0) {
         return -value;
      }
      return value;
   }

   inline double dsign(double const &value, double const &sign_source) {
      return sign(value, sign_source);
   }

   inline int isign(int const &value, int const &sign_source) {
      return sign(value, sign_source);
   }


   template<typename T>
   inline float real(T const &value) {
      return static_cast<float>(value);
   }

   template<typename T>
   inline float sngl(T const &value) {
      return static_cast<float>(value);
   }


   template<typename T>
   inline double dble(T const &value) {
      return static_cast<double>(value);
   }

   inline float sqrt(float const &x) {
      return std::sqrt(x);
   }

   inline double sqrt(double const &x) {
      return std::sqrt(x);
   }

   inline double dsqrt(double const &x) {
      return std::sqrt(x);
   }

   inline float cos(float const &angle) {
      return std::cos(angle);
   }

   inline double cos(double const &angle) {
      return std::cos(angle);
   }

   inline float acos(float const &angle) {
      return std::acos(angle);
   }

   inline double dacos(double const &angle) {
      return std::acos(angle);
   }

   inline double acos(double const &angle) {
      return dacos(angle);
   }

   inline float sin(float const &angle) {
      return std::sin(angle);
   }

   inline double sin(double const &angle) {
      return std::sin(angle);
   }

   inline float asin(float const &angle) {
      return std::asin(angle);
   }

   inline double dasin(double const &angle) {
      return std::asin(angle);
   }

   inline double asin(double const &angle) {
      return dasin(angle);
   }

   inline float tan(float const &angle) {
      return std::tan(angle);
   }

   inline double dtan(double const &angle) {
      return std::tan(angle);
   }

   inline double tan(double const &angle) {
      return dtan(angle);
   }

   inline float atan(float const &angle) {
      return std::atan(angle);
   }

   inline double datan(double const &angle) {
      return std::atan(angle);
   }

   inline double atan(double const &angle) {
      return datan(angle);
   }

   inline float atan2(float const &y, float const &x) {
      return std::atan2(y, x);
   }

   inline double datan2(double const &y, double const &x) {
      return std::atan2(y, x);
   }

   inline double atan2(double const &y, double const &x) {
      return datan2(y, x);
   }

   inline float exp(float const &x) {
      return std::exp(x);
   }

   inline double exp(double const &x) {
      return std::exp(x);
   }

   inline double dexp(double const &x) {
      return std::exp(x);
   }

   inline float alog10(float const &x) {
      return std::log10(x);
   }

   inline double alog10(double const &x) {
      return std::log10(x);
   }

   inline float alog(float const &x) {
      return std::log(x);
   }

   inline double dlog(double const &x) {
      return std::log(x);
   }

   template<typename T>
   inline int fint(T const &val) {
      return static_cast<int>(val);
   }

   template<typename T>
   inline int aint(T const &val) {
      return static_cast<int>(val);
   }

   template<typename T>
   inline float ffloat(T const &val) {
      return static_cast<float>(val);
   }

   inline int mod(int const &v1, int const &v2) {
      return v1 % v2;
   }

   inline float amod(float const &v1, float const &v2) {
      return std::fmod(v1, v2);
   }

   inline float mod(float const &v1, float const &v2) {
      return std::fmod(v1, v2);
   }

   inline double dmod(double const &v1, double const &v2) {
      return std::fmod(v1, v2);
   }

   inline double mod(double const &v1, double const &v2) {
      return std::fmod(v1, v2);
   }

   inline int iabs(int const &v) {
      return std::abs(v);
   }

   inline double dabs(double const &v) {
      return std::abs(v);
   }

   inline int min0(int const &v1, int const &v2) {
      return std::min(v1, v2);
   }

   inline int min(int const &v1, int const &v2) {
      return std::min(v1, v2);
   }

   inline float amin1(float const &v1, float const &v2) {
      return std::min(v1, v2);
   }

   inline float amin1(float const &v1, float const &v2, float const &v3) {
      return amin1(amin1(v1, v2), v3);
   }

   inline float amin1(float const &v1, float const &v2, float const &v3, float const &v4) {
      return amin1(amin1(v1, v2, v3), v4);
   }

   inline float amin1(float const &v1, float const &v2, float const &v3, float const &v4, float const &v5) {
      return amin1(amin1(v1, v2, v3, v4), v5);
   }

   inline float min(float const &v1, float const &v2) {
      return std::min(v1, v2);
   }

   inline double dmin1(double const &v1, double const &v2) {
      return std::min(v1, v2);
   }

   inline double min(double const &v1, double const &v2) {
      return std::min(v1, v2);
   }

   inline float amin0(int const &v1, float const &v2) {
      return std::min(static_cast<float>(v1), v2);
   }

   inline float min(int const &v1, float const &v2) {
      return std::min(static_cast<float>(v1), v2);
   }

   inline float min1(float const &v1, int const &v2) {
      return std::min(v1, static_cast<float>(v2));
   }

   inline float min(float const &v1, int const &v2) {
      return std::min(v1, static_cast<float>(v2));
   }

   inline int min(int const &v1, int const &v2, int const &v3) {
      return min(v1, min(v2, v3));
   }

   inline int min(int const &v1, int const &v2, int const &v3, int const &v4) {
      return min(v1, min(v2, v3, v4));
   }

   inline float min(float const &v1, float const &v2, float const &v3) {
      return min(v1, min(v2, v3));
   }

   inline float min(float const &v1, float const &v2, float const &v3, float const &v4) {
      return min(v1, min(v2, v3, v4));
   }

   inline double min(double const &v1, double const &v2, double const &v3) {
      return min(v1, min(v2, v3));
   }

   inline double min(double const &v1, double const &v2, double const &v3, double const &v4) {
      return min(v1, min(v2, v3, v4));
   }

   inline int max0(int const &v1, int const &v2) {
      return std::max(v1, v2);
   }

   inline int max(int const &v1, int const &v2) {
      return std::max(v1, v2);
   }

   inline float amax1(float const &v1, float const &v2) {
      return std::max(v1, v2);
   }

   inline float amax1(float const &v1, float const &v2, float const &v3) {
      return amax1(amax1(v1, v2), v3);
   }

   inline float amax1(float const &v1, float const &v2, float const &v3, float const &v4) {
      return amax1(amax1(v1, v2, v3), v4);
   }

   inline float amax1(float const &v1, float const &v2, float const &v3, float const &v4, float const &v5) {
      return amax1(amax1(v1, v2, v3, v4), v5);
   }

   inline float max(float const &v1, float const &v2, float const &v3, float const &v4) {
      return amax1(v1, v2, v3, v4);
   }

   inline float max(float const &v1, float const &v2) {
      return std::max(v1, v2);
   }

   inline double dmax1(double const &v1, double const &v2) {
      return std::max(v1, v2);
   }

   inline double max(double const &v1, double const &v2) {
      return std::max(v1, v2);
   }

   inline float amax0(int const &v1, float const &v2) {
      return std::max(static_cast<float>(v1), v2);
   }

   inline float max(int const &v1, float const &v2) {
      return std::max(static_cast<float>(v1), v2);
   }

   inline float max1(float const &v1, int const &v2) {
      return std::max(v1, static_cast<float>(v2));
   }

   inline float max(float const &v1, int const &v2) {
      return std::max(v1, static_cast<float>(v2));
   }

   inline double max(float const &v1, double const &v2) {
      return std::max(static_cast<double>(v1), v2);
   }

   inline int max(int const &v1, int const &v2, int const &v3) {
      return max(max(v1, v2), v3);
   }

   inline float max(float const &v1, float const &v2, float const &v3) {
      return max(max(v1, v2), v3);
   }

   inline double max(double const &v1, double const &v2, double const &v3) {
      return max(max(v1, v2), v3);
   }

   template<typename T>
   inline T pow2(T const &base) {
      return base * base;
   }

   template<typename T>
   inline T pow3(T const &base) {
      return base * base * base;
   }

   template<typename T>
   inline T pow4(T const &base) {
      T p2 = pow2(base);
      return p2 * p2;
   }

   inline int pow(int const &base, int const &exponent) {
      if (exponent < 0)
         return 0;
      int result = 1;
      for (int i = 0; i < exponent; i++) {
         result *= base;
      }
      return result;
   }

   inline float pow(int const &base, float const &exponent) {
      return std::pow(static_cast<float>(base), exponent);
   }

   inline double pow(int const &base, double const &exponent) {
      return std::pow(static_cast<double>(base), exponent);
   }

   inline float pow(float const &base, int const &exponent) {
      return std::pow(base, exponent);
   }

   inline double pow(double const &base, int const &exponent) {
      return std::pow(base, exponent);
   }

   inline float pow(float const &base, float const &exponent) {
      return std::pow(base, exponent);
   }

   inline double pow(double const &base, float const &exponent) {
      return std::pow(base, static_cast<double>(exponent));
   }

   inline double pow(float const &base, double const &exponent) {
      return std::pow(static_cast<double>(base), exponent);
   }

   inline double pow(double const &base, double const &exponent) {
      return std::pow(base, exponent);
   }

   using std::abs;
   using std::log;

   inline int ichar(str_cref c) {
      if (c.len() == 0) {
         std::ostringstream o;
         o << "ichar() argument must be a one-character string,"
           << " but actual string length is " << c.len() << ".";
         throw std::runtime_error(o.str());
      }
      return static_cast<int>(c[ 0 ]);
   }

   inline str<1> fchar(int i) {
      return str<1>(static_cast<char>(i));
   }

   inline int len_trim(str_cref c) {
      return static_cast<int>(utils::find_trailing_blank_padding(c.elems(), c.len()));
   }

   inline int index(str_cref sentence, str_cref word) {
      size_t i = std::string(sentence).find(std::string(word));
      if (i == std::string::npos)
         return 0;
      return static_cast<int>(i + 1);
   }

   inline int lnblnk(str_cref c) {
      return len_trim(c);
   }


   inline int nint(float const &value) {
      return static_cast<int>(std::floor(value + 0.5));
   }

   inline int idnint(float const &value) {
      return static_cast<int>(std::floor(value + 0.5));
   }


   inline int nint(double const &value) {
      return static_cast<int>(std::floor(value + 0.5));
   }

   inline std::complex<float> cmplx(float const &re, float const &im) {
      return std::complex<float>(re, im);
   }

   inline std::complex<double> dcmplx(double const &re, double const &im) {
      return std::complex<double>(re, im);
   }

   inline std::complex<double> cmplx(double const &re, double const &im) {
      return dcmplx(re, im);
   }

   //! Assumes ASCII or similar.
   inline bool llt(str_cref a, str_cref b) {
      return a < b;
   }

   //! Assumes ASCII or similar.
   inline bool lle(str_cref a, str_cref b) {
      return a <= b;
   }

   //! Assumes ASCII or similar.
   inline bool lgt(str_cref a, str_cref b) {
      return a > b;
   }

   //! Assumes ASCII or similar.
   inline bool lge(str_cref a, str_cref b) {
      return a >= b;
   }

} // namespace fem

#endif // GUARD
// --  end: fem/intrinsics.hpp ----------------------
// -- start: fem/main.hpp ----------------------
#ifndef FEM_MAIN_HPP
#define FEM_MAIN_HPP

// -- start: fem/intrinsics_extra.hpp ----------------------
#ifndef FEM_INTRINSICS_EXTRA_HPP
#define FEM_INTRINSICS_EXTRA_HPP

// -- repeat not included:  fem/str.hpp ----------------------
#include <cstdio>
#include <cstdlib>
#include <ctime>

namespace fem {

   inline void getenv(str_cref key, str_ref result) {
      std::string k = utils::strip_leading_and_trailing_blank_padding(key);
      char *v = std::getenv(k.c_str());
      result = (v == 0 ? " " : v);
   }

   inline void date(str_ref result) {
      static const char *months[] = {
         "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
      };
      std::time_t now = std::time(0);
      std::tm const *tm(std::localtime(&now));
      str<10> buf;
      std::sprintf(buf.elems, "%02d-%s-%02d", tm->tm_mday, months[ tm->tm_mon ], tm->tm_year % 100);
      result = buf;
   }

   inline void time(str_ref result) {
      std::time_t now = std::time(0);
      std::tm const *tm(std::localtime(&now));
      str<10> buf;
      std::sprintf(buf.elems, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
      result = buf;
   }

   inline double user_plus_system_time() {
      static std::clock_t t_start = std::clock();
      return static_cast<double>(std::clock() - t_start) / static_cast<double>(CLOCKS_PER_SEC);
   }

   inline void cpu_time(float &result) {
      result = static_cast<float>(user_plus_system_time());
   }

   inline void cpu_time(double &result) {
      result = user_plus_system_time();
   }

   inline int system(str_cref command) {
      return std::system(std::string(command).c_str());
   }

} // namespace fem

#endif // GUARD
// --  end: fem/intrinsics_extra.hpp ----------------------
// -- start: fem/stop.hpp ----------------------
#ifndef FEM_STOP_HPP
#define FEM_STOP_HPP

// -- repeat not included:  fem/size_t.hpp ----------------------
#include <sstream>
#include <stdexcept>
#include <string>

namespace fem {

   struct stop_info : public std::exception {
         std::string source_file;
         size_t source_line;
         ssize_t digits;
         std::string message;

      protected:
         mutable std::string what_buffer;

      public:
         stop_info(const char *source_file_, size_t source_line_, ssize_t digits_) throw() :
             source_file(source_file_), source_line(source_line_), digits(digits_) {}

         stop_info(const char *source_file_, size_t source_line_, std::string const &message_) throw() :
             source_file(source_file_), source_line(source_line_), digits(-1), message(message_) {}

         ~stop_info() throw() {}

         const char *what() const throw() {
            if (what_buffer.size() == 0) {
               std::ostringstream o;
               o << "STOP at " << source_file << "(" << source_line << ")";
               if (message.size() != 0) {
                  o << ": " << message;
               }
               else if (digits > 0) {
                  o << ": code=" << digits;
               }
               what_buffer = o.str();
               what_buffer.push_back('\0');
            }
            return what_buffer.data();
         }
   };

} // namespace fem

#define FEM_STOP(arg) throw fem::stop_info(__FILE__, __LINE__, arg)

#endif // GUARD
// --  end: fem/stop.hpp ----------------------
// -- repeat not included:  fem/utils/int_types.hpp ----------------------
// -- repeat not included:  fem/utils/string.hpp ----------------------
#include <cstdio>
#include <vector>

#if defined(_MSC_VER)
#include <fcntl.h>
#include <io.h>
#endif

namespace fem {

   inline bool check_fem_utils_int_types() {
      bool result = true;
#define FABLE_LOC(bits, expected_sz) \
   { \
      int sz = static_cast<int>(sizeof(fem::utils::int##bits##_t)); \
      if (sz != expected_sz) { \
         std::fprintf(stderr, "FATAL: sizeof(fem::utils::int%d_t) is %d but should be %d.\n", bits, sz, expected_sz); \
         result = false; \
      } \
   }
      FABLE_LOC(8, 1)
      FABLE_LOC(16, 2)
      FABLE_LOC(32, 4)
      FABLE_LOC(64, 8)
#undef FABLE_LOC
      if (!result) {
         std::fprintf(
            stderr, "NOTE: fem/utils/int_types.hpp"
                    " needs to be adjusted for this platform.\n");
      }
      return result;
   }

   inline int main_with_catch(int argc, char const *argv[], void (*callable)(int argc, char const *argv[])) {
      user_plus_system_time();
      if (!check_fem_utils_int_types()) {
         return 255;
      }
#if defined(_MSC_VER)
      // if (_setmode(_fileno(stdin), _O_BINARY) == -1) {
      // std::cerr
      // << "FATAL: error switching stdin to binary mode ("
      // << __FILE__ << ", line " << __LINE__ << ")" << std::endl;
      // return 255;
      // }
#endif
      try {
         callable(argc, argv);
      } catch (fem::stop_info const &info) {
         std::fflush(stdout);
         std::fprintf(stderr, "%s\n", info.what());
         std::fflush(stderr);
      } catch (std::exception const &e) {
         std::fflush(stdout);
         char const *what = e.what();
         if (what == 0)
            what = "null";
         std::fprintf(stderr, "std::exception what(): %s\n", what);
         std::fflush(stderr);
         return 1;
      } catch (...) {
         std::fflush(stdout);
         std::fprintf(stderr, "Terminated by unknown C++ exception.\n");
         std::fflush(stderr);
         return 2;
      }
      return 0;
   }

   static const std::string dynamic_parameters_option("--fem-dynamic-parameters");

   struct command_line_arguments {
         std::vector<std::string> buffer;
         std::vector<std::string> dynamic_parameters_fields;

         command_line_arguments() {}

         command_line_arguments(int argc, char const *argv[]) {
            for (int i = 0; i < argc; i++) {
               char const *arg = argv[ i ];
               if (utils::starts_with(
                      arg,
                      /*start*/ 0,
                      /*stop*/ dynamic_parameters_option.size(), dynamic_parameters_option.c_str())) {
                  size_t j = dynamic_parameters_option.size();
                  if (arg[ j ] == '=')
                     j++;
                  utils::split_comma_separated(dynamic_parameters_fields, arg + j);
               }
               else {
                  buffer.push_back(std::string(arg));
               }
            }
         }
   };

   struct dynamic_parameters_from {
         command_line_arguments const &command_line_args;
         int i_fld;

         dynamic_parameters_from(command_line_arguments const &command_line_args_, int max_number_of_flds) :
             command_line_args(command_line_args_), i_fld(0) {
            int n_flds = static_cast<int>(command_line_args.dynamic_parameters_fields.size());
            if (n_flds > max_number_of_flds) {
               std::ostringstream o;
               o << "Too many " << dynamic_parameters_option << " fields (given: " << n_flds
                 << ", max. expected: " << max_number_of_flds << ")";
               throw std::runtime_error(o.str());
            }
         }

         template<typename T>
         dynamic_parameters_from &reset_if_given(T &value) {
            int n_flds = static_cast<int>(command_line_args.dynamic_parameters_fields.size());
            if (i_fld < n_flds) {
               std::string const &fld = command_line_args.dynamic_parameters_fields[ i_fld ];
               std::istringstream i(fld);
               i >> value;
               if (i.fail()) {
                  std::ostringstream o;
                  o << "Invalid " << dynamic_parameters_option << " field (field " << (i_fld + 1) << "): \"" << fld
                    << "\"";
                  throw std::runtime_error(o.str());
               }
               i_fld++;
            }
            return *this;
         }
   };

   template<typename D>
   struct dynamic_parameters_capsule {
         D dynamic_params;

         dynamic_parameters_capsule(command_line_arguments const &command_line_args) :
             dynamic_params(command_line_args) {}
   };

} // namespace fem

#endif // GUARD
// --  end: fem/main.hpp ----------------------
// -- start: fem/major_types.hpp ----------------------
#ifndef FEM_MAJOR_TYPES_HPP
#define FEM_MAJOR_TYPES_HPP

// -- start: fem/arr_size.hpp ----------------------
#ifndef FEM_ARR_SIZE_HPP
#define FEM_ARR_SIZE_HPP

// -- repeat not included:  fem/arr_ref.hpp ----------------------

namespace fem {

   template<size_t Size, typename T, size_t Ndims = 1>
   struct arr_size : arr_ref<T, Ndims> {
         T elems_memory[ Size ];

      private:
         arr_size(arr_size const &);

         arr_size const &operator=(arr_size const &);

      public:
         arr_size() : arr_ref<T, Ndims>(*elems_memory) {}

         template<size_t BufferNdims>
         explicit arr_size(dims<BufferNdims> const &dims) : arr_ref<T, Ndims>(*elems_memory, dims) {}

         template<size_t BufferNdims>
         arr_size(dims<BufferNdims> const &dims, no_fill0_type const &) :
             arr_ref<T, Ndims>(*elems_memory, dims, no_fill0) {}

         template<size_t BufferNdims>
         arr_size(dims<BufferNdims> const &dims, fill0_type const &) : arr_ref<T, Ndims>(*elems_memory, dims, fill0) {}

         operator T &() { return *elems_memory; }
   };

   template<size_t Size, typename T>
   struct arr_1d : arr_size<Size, T, 1> {
         typedef arr_size<Size, T, 1> base_t;

         arr_1d() : base_t(dimension(Size)) {}

         explicit arr_1d(no_fill0_type const &) : base_t(dimension(Size), no_fill0) {}

         explicit arr_1d(fill0_type const &) : base_t(dimension(Size), fill0) {}

         template<size_t BufferNdims>
         explicit arr_1d(dims<BufferNdims> const &dims) : base_t(dims) {}

         template<size_t BufferNdims>
         arr_1d(dims<BufferNdims> const &dims, no_fill0_type const &) : base_t(dims, no_fill0) {}

         template<size_t BufferNdims>
         arr_1d(dims<BufferNdims> const &dims, fill0_type const &) : base_t(dims, fill0) {}
   };

   template<size_t Size1, size_t Size2, typename T>
   struct arr_2d : arr_size<Size1 * Size2, T, 2> {
         typedef arr_size<Size1 * Size2, T, 2> base_t;

         arr_2d() : base_t(dimension(Size1, Size2)) {}

         explicit arr_2d(no_fill0_type const &) : base_t(dimension(Size1, Size2), no_fill0) {}

         explicit arr_2d(fill0_type const &) : base_t(dimension(Size1, Size2), fill0) {}

         template<size_t BufferNdims>
         explicit arr_2d(dims<BufferNdims> const &dims) : base_t(dims) {}

         template<size_t BufferNdims>
         arr_2d(dims<BufferNdims> const &dims, no_fill0_type const &) : base_t(dims, no_fill0) {}

         template<size_t BufferNdims>
         arr_2d(dims<BufferNdims> const &dims, fill0_type const &) : base_t(dims, fill0) {}
   };

   template<size_t Size1, size_t Size2, size_t Size3, typename T>
   struct arr_3d : arr_size<Size1 * Size2 * Size3, T, 3> {
         typedef arr_size<Size1 * Size2 * Size3, T, 3> base_t;

         arr_3d() : base_t(dimension(Size1, Size2, Size3)) {}

         explicit arr_3d(no_fill0_type const &) : base_t(dimension(Size1, Size2, Size3), no_fill0) {}

         explicit arr_3d(fill0_type const &) : base_t(dimension(Size1, Size2, Size3), fill0) {}

         template<size_t BufferNdims>
         explicit arr_3d(dims<BufferNdims> const &dims) : base_t(dims) {}

         template<size_t BufferNdims>
         arr_3d(dims<BufferNdims> const &dims, no_fill0_type const &) : base_t(dims, no_fill0) {}

         template<size_t BufferNdims>
         arr_3d(dims<BufferNdims> const &dims, fill0_type const &) : base_t(dims, fill0) {}
   };

} // namespace fem

#endif // GUARD
// --  end: fem/arr_size.hpp ----------------------
// -- start: fem/data.hpp ----------------------
#ifndef FEM_DATA_HPP
#define FEM_DATA_HPP

// -- repeat not included:  fem/arr.hpp ----------------------
// -- repeat not included:  fem/str_arr_ref.hpp ----------------------
// -- repeat not included:  fem/utils/misc.hpp ----------------------
#include <typeinfo>
#include <vector>

namespace fem {

   struct datum {
         // similar to boost::any
         struct placeholder {
               virtual ~placeholder() {}
               virtual std::type_info const &type() const = 0;
               virtual placeholder *clone() const = 0;
         };

         template<typename T>
         struct holder : placeholder {
               T held;

               holder(T const &value) : held(value) {}

               virtual std::type_info const &type() const { return typeid(T); }

               virtual placeholder *clone() const { return new holder(held); }

            private:
               holder const &operator=(holder const &);
         };

         placeholder *content;
         mutable unsigned repeats;

         datum() : content(0), repeats(0) {}

         datum(char const &value) : content(new holder<char>(value)), repeats(1) {}

         datum(bool const &value) : content(new holder<bool>(value)), repeats(1) {}

         datum(int const &value) : content(new holder<int>(value)), repeats(1) {}

         datum(float const &value) : content(new holder<float>(value)), repeats(1) {}

         datum(double const &value) : content(new holder<double>(value)), repeats(1) {}

         template<typename T>
         datum(T const &value) : content(new holder<std::string>(std::string(value))), repeats(1) {}

         datum(datum const &other) : content(other.content ? other.content->clone() : 0), repeats(other.repeats) {}

         ~datum() { delete content; }

         // required for std::vector<datum> (Apple g++ 4.0.1, Visual C++ 9.0)
         // but not actually used
         datum const &operator=(datum const & /* rhs */) { throw TBXX_UNREACHABLE_ERROR(); }

         void set_repeats(unsigned value) const { repeats = value; }

         void throw_type_mismatch(char const *target_type) const {
            throw std::runtime_error(
               std::string("DATA type mismatch: target type ") + target_type + " vs. " + content->type().fileName()
               + " value");
         }
   };

   inline datum const &operator*(unsigned repeats, datum const &value) {
      value.set_repeats(repeats);
      return value;
   }

   struct data_buffer {
         utils::slick_ptr<std::vector<datum>> objects;

         data_buffer() : objects(new std::vector<datum>) {}

         template<typename T>
         data_buffer &operator,(T const &val) {
            objects->push_back(datum(val));
            return *this;
         }
   };

   struct data_values {
         data_buffer values;
         size_t value_index;
         size_t repeat_index;

         data_values() : value_index(0), repeat_index(0) {}

         data_values(data_buffer const &values_) : values(values_), value_index(0), repeat_index(0) {}

         // XXX
         void report_types() const {
            size_t n = values.objects->size();
            for (size_t i = 0; i != n; i++) {
               datum const &obj = (*(values.objects))[ i ];
               std::cout << "data_buffer[" << i << "]: " << obj.content->type().fileName() << std::endl;
            }
         }

         datum const &next_datum() {
            datum const &result = (*(values.objects))[ value_index ];
            TBXX_ASSERT(result.content != 0);
            TBXX_ASSERT(result.repeats > 0);
            repeat_index++;
            if (repeat_index == result.repeats) {
               value_index++;
               repeat_index = 0;
            }
            return result;
         }

         data_values &operator,(bool &val) {
            datum const &tab_val = next_datum();
            if (tab_val.content->type() == typeid(bool)) {
               val = static_cast<datum::holder<bool> *>(tab_val.content)->held;
            }
            else {
               tab_val.throw_type_mismatch("bool");
            }
            return *this;
         }

         data_values &operator,(char &val) {
            datum const &tab_val = next_datum();
            if (tab_val.content->type() == typeid(int)) {
               val = static_cast<char>(static_cast<datum::holder<int> *>(tab_val.content)->held);
            }
            else if (tab_val.content->type() == typeid(char)) {
               val = static_cast<datum::holder<char> *>(tab_val.content)->held;
            }
            else {
               tab_val.throw_type_mismatch("char");
            }
            return *this;
         }

         data_values &operator,(integer_star_2 &val) {
            datum const &tab_val = next_datum();
            if (tab_val.content->type() == typeid(int)) {
               val = static_cast<integer_star_2>(static_cast<datum::holder<int> *>(tab_val.content)->held);
            }
            else {
               tab_val.throw_type_mismatch("integer_star_2");
            }
            return *this;
         }

         data_values &operator,(int &val) {
            datum const &tab_val = next_datum();
            if (tab_val.content->type() == typeid(int)) {
               val = static_cast<datum::holder<int> *>(tab_val.content)->held;
            }
            else {
               tab_val.throw_type_mismatch("int");
            }
            return *this;
         }

         data_values &operator,(float &val) {
            datum const &tab_val = next_datum();
            if (tab_val.content->type() == typeid(int)) {
               val = static_cast<float>(static_cast<datum::holder<int> *>(tab_val.content)->held);
            }
            else if (tab_val.content->type() == typeid(float)) {
               val = static_cast<datum::holder<float> *>(tab_val.content)->held;
            }
            else {
               tab_val.throw_type_mismatch("float");
            }
            return *this;
         }

         data_values &operator,(double &val) {
            datum const &tab_val = next_datum();
            if (tab_val.content->type() == typeid(int)) {
               val = static_cast<double>(static_cast<datum::holder<int> *>(tab_val.content)->held);
            }
            else if (tab_val.content->type() == typeid(float)) {
               val = static_cast<double>(static_cast<datum::holder<float> *>(tab_val.content)->held);
            }
            else if (tab_val.content->type() == typeid(double)) {
               val = static_cast<datum::holder<double> *>(tab_val.content)->held;
            }
            else {
               tab_val.throw_type_mismatch("double");
            }
            return *this;
         }

         data_values &operator,(str_ref val) {
            datum const &tab_val = next_datum();
            if (tab_val.content->type() == typeid(std::string)) {
               std::string const &tab_str = static_cast<datum::holder<std::string> *>(tab_val.content)->held;
               val = tab_str.c_str();
            }
            else {
               tab_val.throw_type_mismatch("string");
            }
            return *this;
         }

         template<typename T, size_t Ndims>
         data_values &operator,(arr_ref<T, Ndims> &val) {
            size_t n = val.size_1d();
            T *val_begin = val.begin();
            for (size_t i = 0; i < n; i++)
               (*this), val_begin[ i ];
            return *this;
         }

         template<size_t Ndims>
         data_values &operator,(str_arr_ref<Ndims> &val) {
            size_t n = val.size_1d();
            for (size_t i = 0; i < n; i++)
               (*this), val[ i ];
            return *this;
         }
   };

   struct values_type {
         template<typename T>
         data_buffer operator,(T const &val) const {
            return data_buffer(), val;
         }
   };

   static const values_type values = values_type();

   inline data_values data(data_buffer const &values) {
      return data_values(values);
   }
} // namespace fem

#endif // GUARD
// --  end: fem/data.hpp ----------------------
// -- start: fem/read.hpp ----------------------
#ifndef FEM_READ_HPP
#define FEM_READ_HPP

// -- start: fem/common.hpp ----------------------
#ifndef FEM_COMMON_HPP
#define FEM_COMMON_HPP

// -- start: fem/close_chain.hpp ----------------------
#ifndef FEM_CLOSE_CHAIN_HPP
#define FEM_CLOSE_CHAIN_HPP

// -- start: fem/io.hpp ----------------------
#ifndef FEM_IO_HPP
#define FEM_IO_HPP

// -- repeat not included:  fem/io_exceptions.hpp ----------------------
// -- repeat not included:  fem/utils/misc.hpp ----------------------
// -- start: fem/utils/path.hpp ----------------------
#ifndef FEM_UTILS_PATH_HPP
#define FEM_UTILS_PATH_HPP

// -- repeat not included:  fem/utils/char.hpp ----------------------

#include <cstdio>
#include <cstring>
#include <errno.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_MSC_VER)

#include <direct.h> // for _getcwd
// -- repeat not included:  io.h ----------------------

#define FEM_UTILS_PATH_STRUCT_STAT struct _stat
#define FEM_UTILS_PATH_STAT _stat
#define FEM_UTILS_PATH_GETCWD _getcwd
#define FEM_UTILS_PATH_FTRUNCATE _chsize

#else

#include <unistd.h> // for ftruncate

#define FEM_UTILS_PATH_STRUCT_STAT struct stat
#define FEM_UTILS_PATH_STAT stat
#define FEM_UTILS_PATH_GETCWD getcwd
#define FEM_UTILS_PATH_FTRUNCATE ftruncate

#endif

namespace fem {
   namespace utils {
      namespace path {

         inline bool exists(char const *path) {
            FEM_UTILS_PATH_STRUCT_STAT buf;
            int stat_result = FEM_UTILS_PATH_STAT(path, &buf);
            return (stat_result == 0 || errno != ENOENT);
         }

         inline char const *split_drive(char const *path) {
#if defined(_MSC_VER)
            if (is_a_through_z(path[ 0 ]) && path[ 1 ] == ':')
               return path + 2;
#endif
            return path;
         }

         inline bool is_absolute(char const *path, bool drive_split_already = false) {
            if (!drive_split_already)
               path = split_drive(path);
#if defined(_MSC_VER)
            return (path[ 0 ] == '/' || path[ 0 ] == '\\'); // emulates Python
#else
            return (path[ 0 ] == '/');
#endif
         }

         inline const char *separator() {
#if defined(_MSC_VER)
            return "\\";
#else
            return "/";
#endif
         }

         inline std::string absolute(char const *path) {
            char const *path_without_drive = split_drive(path);
            if (is_absolute(path_without_drive, /*drive_split_already*/ true)) {
               return std::string(path);
            }
            std::string result;
            static const size_t buf_size = 10000; // ad-hoc
            char buf[ buf_size ];
            char *getcwd_result = FEM_UTILS_PATH_GETCWD(buf, buf_size);
            if (getcwd_result == 0) {
               int en = errno;
               std::string msg = "fem::utils::path::absolute(): ";
               if (en != 0) {
                  msg += std::strerror(en);
               }
               else {
                  msg += "unknown error";
               }
               throw std::runtime_error(msg);
            }
            else {
               if (path != path_without_drive) {
                  result += std::string(path, path_without_drive);
               }
               result += getcwd_result;
               result += separator();
               result += path_without_drive;
            }
            return result;
         }

         inline bool truncate_file_at_current_position(std::FILE *fp) {
            long curr_pos = std::ftell(fp);
            if (curr_pos < 0)
               return false;
            if (FEM_UTILS_PATH_FTRUNCATE(fileno(fp), curr_pos) != 0)
               return false;
            std::fflush(fp);
            if (std::fseek(fp, 0L, SEEK_END) != 0)
               return false;
            return true;
         }

      } // namespace path
   } // namespace utils
} // namespace fem

#endif // GUARD
// --  end: fem/utils/path.hpp ----------------------
// -- start: fem/utils/random.hpp ----------------------
#ifndef FEM_UTILS_RANDOM_HPP
#define FEM_UTILS_RANDOM_HPP

// -- repeat not included:  fem/size_t.hpp ----------------------
#include <ctime>
#include <string>

namespace fem {
   namespace utils {

      inline std::string random_name_simple(size_t size) {
         static bool first_call = true;
         static size_t random_state;
         static const size_t random_mod = 225150U;
         if (first_call) {
            first_call = false;
            random_state = static_cast<size_t>(std::time(0)) % random_mod;
         }
         std::string result;
         result.reserve(size);
         for (size_t i = 0; i < size; i++) {
            random_state = (random_state * 9538U + 50294U) % random_mod;
            size_t j = random_state % (i == 0 ? 26U : 36U);
            result.push_back("abcdefghijklmnopqrstuvwxyz0123456789"[ j ]);
         }
         return result;
      }

   } // namespace utils
} // namespace fem

#endif // GUARD
// --  end: fem/utils/random.hpp ----------------------
// -- start: fem/utils/simple_streams.hpp ----------------------
#ifndef FEM_UTILS_SIMPLE_STREAMS_HPP
#define FEM_UTILS_SIMPLE_STREAMS_HPP

// -- repeat not included:  fem/size_t.hpp ----------------------
#include <cstdio>
#include <string>

#if defined(_MSC_VER) || _MSC_VER <= 1310 // Visual C++ 7.1
#define std__ferror ferror
#endif

namespace fem {
   namespace utils {

      static const int stream_end = 256;
      static const int stream_err = 257;

      inline bool is_stream_end(int c) {
         return c == stream_end;
      }

      inline bool is_stream_err(int c) {
         return c == stream_err;
      }

      inline bool is_stream_end_or_err(int c) {
         return c >= stream_end;
      }

      struct simple_ostream {
            virtual ~simple_ostream() {}
            virtual void put(char c) = 0;
            virtual void put(char const *str, size_t str_sz) = 0;
            virtual void flush() = 0;
            virtual bool err() = 0;
      };

      struct simple_istream {
            virtual ~simple_istream() {}
            virtual int get() = 0;
            virtual void backup() = 0;
      };

      struct simple_ostream_to_char_ptr_and_size : simple_ostream {
            char *s;
            size_t sz;
            ssize_t i;

            simple_ostream_to_char_ptr_and_size(char *string, size_t size) : s(string), sz(size), i(0) {}

            void put(char c) {
               if (i < 0)
                  return;
               if (i < ( int ) sz)
                  s[ i++ ] = c;
               else
                  i = -1;
            }

            void put(char const *str, size_t str_sz) {
               if (i < 0)
                  return;
               if (i + str_sz <= sz) {
                  std::memcpy(s + i, str, str_sz);
                  i += str_sz;
               }
               else {
                  ssize_t j = sz - i;
                  std::memcpy(s + i, str, j);
                  i = -1;
               }
            }

            void flush() {}

            bool err() { return (i < 0); }
      };

      struct simple_ostream_to_std_string : simple_ostream {
            std::string &s;

            simple_ostream_to_std_string(std::string &string) : s(string) {}

            void put(char c) { s.push_back(c); }

            void put(char const *str, size_t str_sz) {
               for (size_t i = 0; i < str_sz; i++) {
                  s.push_back(str[ i ]);
               }
            }

            void flush() {}

            bool err() { return false; }
      };

      struct simple_ostream_to_c_file : simple_ostream {
            FILE *f;

            simple_ostream_to_c_file(FILE *f_) : f(f_) {}

            void put(char c) { std::fputc(c, f); }

            void put(char const *str, size_t str_sz) {
               for (size_t i = 0; i < str_sz; i++) {
                  std::fputc(str[ i ], f);
               }
            }

            void flush() { std::fflush(f); }

            bool err() { return std__ferror(f); }
      };

      struct simple_istream_from_c_str : simple_istream {
            char const *s;

            simple_istream_from_c_str(char const *string) : s(string) {}

            int get() {
               if (*s == '\0')
                  return stream_end;
               return *s++;
            }

            void backup() { s--; }
      };

      struct simple_istream_from_char_ptr_and_size : simple_istream {
            char const *s;
            size_t sz;
            size_t i;

            simple_istream_from_char_ptr_and_size(char const *string, size_t size) : s(string), sz(size), i(0) {}

            int get() {
               if (i == sz)
                  return stream_end;
               return s[ i++ ];
            }

            void backup() { i--; }
      };

      struct simple_istream_from_std_string : simple_istream {
            std::string s;
            size_t i;

            simple_istream_from_std_string(std::string const &string) : s(string), i(0) {}

            int get() {
               if (i == s.size())
                  return stream_end;
               return s[ i++ ];
            }

            void backup() { i--; }
      };

      struct simple_istream_from_c_file : simple_istream {
            FILE *f;
            int last_get_result;

            simple_istream_from_c_file(FILE *f_) : f(f_) {}

            int get() {
               last_get_result = std::fgetc(f);
               if (last_get_result == EOF) {
                  last_get_result = (std__ferror(f) ? stream_err : stream_end);
               }
               return last_get_result;
            }

            void backup() { std::ungetc(last_get_result, f); }
      };

   } // namespace utils
} // namespace fem

#endif // GUARD
// --  end: fem/utils/simple_streams.hpp ----------------------
// -- repeat not included:  fem/utils/string.hpp ----------------------
#include <cstdio>
#include <map>

namespace fem {

   static const char file_not_specified[] = "";
   enum unformatted_type { unformatted };

   enum io_modes { io_unformatted = 0, io_list_directed, io_formatted };

   static const char end_of_unformatted_record = static_cast<char>(0xAAU);

   struct std_file {
         std::FILE *ptr;

         std_file(std::FILE *ptr_ = 0) : ptr(ptr_) {}
   };

   inline bool is_std_io_unit(int unit) {
      return (unit == 0 || unit == 5 || unit == 6);
   }

   struct io_unit {
         int number;
         std::string file_name;
         std_file stream;
         bool prev_op_was_write;

         static const char **ac_keywords() {
            static const char *result[] = { "sequential", "direct", 0 };
            return result;
         }
         enum access_types { ac_sequential = 0, ac_direct, ac_undef };
         access_types access;

         static const char **fm_keywords() {
            static const char *result[] = { "formatted", "unformatted", 0 };
            return result;
         }
         enum form_types { fm_formatted = 0, fm_unformatted, fm_undef };
         form_types form;

         unsigned recl;

         static const char **bl_keywords() {
            static const char *result[] = { "null", "zero", 0 };
            return result;
         }
         enum blank_types { bl_null = 0, bl_zero, bl_undef };
         blank_types blank;

         static const char **st_keywords() {
            static const char *result[] = { "old", "new", "scratch", "unknown", 0 };
            return result;
         }
         enum status_types { st_old = 0, st_new, st_scratch, st_unknown, st_undef };
         status_types status;

         /*! f77_std 12.9.7
        If no error condition or end-of-file condition exists, the
        value of ios is zero. If an error condition exists, the value
        of ios is positive. If an end-of-file condition exists and
        no error condition exists, the value of ios is negative.
        XXX NOT IMPLEMENTED
      */
         int iostat;

         io_unit(int number_, std::string file_name_ = std::string(""), std_file stream_ = std_file(0)) :
             number(number_), file_name(file_name_), stream(stream_), prev_op_was_write(false), access(ac_undef),
             form(fm_undef), recl(0), blank(bl_undef), status(st_undef), iostat(0) {}

         std::string get_file_name_set_default_if_necessary() {
            if (file_name.size() == 0 && !is_std_io_unit(number)) {
               if (status != st_scratch) {
                  char buffer[ 64 ];
                  int actual_width = utils::int_to_string(
                     buffer, sizeof(buffer),
                     /*width*/ 3,
                     /*value*/ number,
                     /*left_padding_character*/ '0');
                  TBXX_ASSERT(actual_width > 0);
                  file_name = "fem_io_unit_" + std::string(buffer, actual_width);
               }
               else {
                  size_t run_away_counter = 0;
                  while (true) {
                     file_name = "fem_io_unit_scratch_" + utils::random_name_simple(8);
                     if (!utils::path::exists(file_name.c_str())) {
                        break;
                     }
                     run_away_counter++;
                     TBXX_ASSERT(run_away_counter < 1000);
                  }
               }
            }
            return file_name;
         }

         void open(int *iostat_ptr) {
            if (status == st_undef) {
               status = st_unknown; // f77_std 12.10.1
            }
            if (status == st_scratch) {
               /* f77_std 12.10.1: SCRATCH must not be specified with a named file. */
               /* ifort compile-time warning; file name simply ignored in this
               implementation. */
               file_name.clear();
            }
            if (access == ac_undef) {
               access = ac_sequential; // f77_std 12.10.1
            }
            { // f77_std 12.10.1, end of paragraph under RECL = rl:
               // This specifier [recl] must be given when a file is being
               // connected for direct access; otherwise, it must be omitted.
               if (access == ac_direct) {
                  if (recl == 0) {
                     std::ostringstream o;
                     o << "OPEN error: unit=" << number << " connected for DIRECT access but recl=0";
                     throw io_err(o.str());
                  }
               }
               else {
                  if (recl != 0) {
                     std::ostringstream o;
                     o << "OPEN error: unit=" << number << " connected for SEQUENTIAL access but recl=" << recl
                       << " (it must be omitted)";
                     throw io_err(o.str());
                  }
               }
            }
            if (form == fm_undef) {
               if (access == ac_direct) {
                  form = fm_unformatted;
               }
               else {
                  form = fm_formatted;
               }
            }
            if (blank == bl_undef) {
               blank = bl_null; // f77_std 12.10.1
            }
            bool file_exists = utils::path::exists(get_file_name_set_default_if_necessary().c_str());
            if (status == st_old) {
               if (!file_exists) {
                  iostat = 1;
                  if (iostat_ptr != 0) {
                     *iostat_ptr = iostat;
                  }
                  return;
                  std::ostringstream o;
                  o << "OPEN OLD error: file does not exist: " << file_name;
                  throw io_err(o.str());
               }
            }
            else if (status == st_new) {
               if (file_exists) {
                  iostat = 1;
                  if (iostat_ptr != 0) {
                     *iostat_ptr = iostat;
                     return;
                  }
                  std::ostringstream o;
                  o << "OPEN NEW error: file exists already: " << file_name;
                  throw io_err(o.str());
               }
            }
            if (access == ac_direct) {
               throw TBXX_NOT_IMPLEMENTED();
            }
            stream.ptr = std::us_fopen(file_name.c_str(), "ab+");
            if (stream.ptr == 0 || std::fseek(stream.ptr, 0L, SEEK_SET) != 0) {
               if (status == st_new) {
                  iostat = 1;
               }
               else {
                  stream.ptr = std::us_fopen(file_name.c_str(), "rb");
                  if (stream.ptr == 0 || std::fseek(stream.ptr, 0L, SEEK_SET) != 0) {
                     iostat = 1;
                  }
               }
               if (iostat == 1) {
                  if (iostat_ptr != 0) {
                     *iostat_ptr = iostat;
                     return;
                  }
                  throw io_err("Error opening file: " + file_name);
               }
            }
            if (status == st_new) {
               /* f77_std 12.10.1: Successful execution of an OPEN statement with
               NEW specified creates the file and changes the status to OLD. */
               status = st_old;
            }
         }

         void close(int *iostat_ptr = 0, bool status_delete = false) {
            if (iostat_ptr != 0)
               *iostat_ptr = 0; // XXX
            if (is_std_io_unit(number))
               return;
            if (stream.ptr != 0) {
               std::fclose(stream.ptr);
               stream.ptr = 0;
            }
            if (status == st_scratch || status_delete) {
               std::remove(file_name.c_str());
            }
         }

         void backspace(int * /* iostat_ptr */) { throw TBXX_NOT_IMPLEMENTED(); }

         void endfile(int * /* iostat_ptr */) {
            if (is_std_io_unit(number)) {
               throw TBXX_NOT_IMPLEMENTED();
            }
            if (!utils::path::truncate_file_at_current_position(stream.ptr)) {
               throw io_err("ENDFILE failure: " + file_name);
            }
            prev_op_was_write = false;
         }

         void rewind(int *iostat_ptr) {
            if (stream.ptr == 0 || std::fseek(stream.ptr, 0L, SEEK_SET) != 0) {
               iostat = 1;
               if (iostat_ptr != 0) {
                  *iostat_ptr = iostat;
                  return;
               }
               throw io_err("Error rewinding file: " + file_name);
            }
            prev_op_was_write = false;
         }
   };

   struct io : utils::noncopyable {
         std::map<int, io_unit> units;

         io() {
            units.insert(std::make_pair(0, io_unit(0, "", stderr)));
            units.insert(std::make_pair(5, io_unit(5, "", stdin)));
            units.insert(std::make_pair(6, io_unit(6, "", stdout)));
         }

         ~io() {
            typedef std::map<int, io_unit>::iterator it;
            it e = units.end();
            for (it i = units.begin(); i != e; i++) {
               i->second.close();
            }
         }

         io_unit *unit_ptr(int unit, bool auto_open = false) {
            typedef std::map<int, io_unit>::iterator it;
            it map_iter = units.find(unit);
            if (map_iter == units.end()) {
               if (!auto_open)
                  return 0;
               map_iter = units.insert(std::make_pair(unit, io_unit(unit))).first;
               map_iter->second.open(/*iostat_ptr*/ 0);
            }
            return &(map_iter->second);
         }

         //! Easy C++ access.
         std::string file_name_of_unit(int unit) {
            io_unit *u_ptr = unit_ptr(unit);
            if (u_ptr == 0)
               return "";
            return u_ptr->file_name;
         }

         inline struct inquire_chain inquire_unit(int unit);

         inline struct inquire_chain inquire_file(std::string file, bool blank_padding_removed_already = false);

         inline struct open_chain open(
            int unit, std::string file = std::string(), bool blank_padding_removed_already = false);

         inline struct close_chain close(int unit);

         inline struct file_positioning_chain backspace(int unit);

         inline struct file_positioning_chain endfile(int unit);

         inline struct file_positioning_chain rewind(int unit);

         inline bool is_opened_simple(std::string const &file_name) const {
            typedef std::map<int, io_unit>::const_iterator it;
            it e = units.end();
            for (it i = units.begin(); i != e; i++) {
               if (i->second.file_name == file_name) {
                  return true;
               }
            }
            return false;
         }

         utils::slick_ptr<utils::simple_ostream> simple_ostream(int unit) {
            io_unit *u_ptr = unit_ptr(unit, /*auto_open*/ true);
            std_file &sf = u_ptr->stream;
            if (!u_ptr->prev_op_was_write) {
               if (!is_std_io_unit(unit)) {
                  if (!utils::path::truncate_file_at_current_position(sf.ptr)) {
                     throw io_err("Cannot truncate file for writing: " + u_ptr->file_name);
                  }
               }
               u_ptr->prev_op_was_write = true;
            }
            return utils::slick_ptr<utils::simple_ostream>(new utils::simple_ostream_to_c_file(sf.ptr));
         }

         utils::slick_ptr<utils::simple_istream> simple_istream(int unit) {
            io_unit *u_ptr = unit_ptr(unit, /*auto_open*/ true);
            u_ptr->prev_op_was_write = false;
            return utils::slick_ptr<utils::simple_istream>(new utils::simple_istream_from_c_file(u_ptr->stream.ptr));
         }

         void flush(int unit) {
            io_unit *u_ptr = unit_ptr(unit);
            if (u_ptr != 0) {
               int status = std::fflush(u_ptr->stream.ptr);
               if (status != 0) {
                  throw io_err("Error flushing file: " + u_ptr->file_name);
               }
            }
         }
   };

} // namespace fem

#endif // GUARD
// --  end: fem/io.hpp ----------------------

namespace fem {

   struct close_chain {
         mutable io *io_ptr;
         int unit;
         int *iostat_ptr;
         bool status_delete;

      private:
         close_chain const &operator=(close_chain const &); // not implemented

      public:
         close_chain(io *io_ptr_, int unit_) : io_ptr(io_ptr_), unit(unit_), iostat_ptr(0), status_delete(false) {}

         close_chain(close_chain const &other) :
             io_ptr(other.io_ptr), unit(other.unit), iostat_ptr(other.iostat_ptr), status_delete(other.status_delete) {
            other.io_ptr = 0;
         }

         ~close_chain() {
            if (io_ptr == 0)
               return;
            io_unit *u_ptr = io_ptr->unit_ptr(unit);
            if (u_ptr != 0) {
               u_ptr->close(iostat_ptr, status_delete);
               io_ptr->units.erase(io_ptr->units.find(unit));
            }
         }

         close_chain &status(std::string const &val) {
            static const char *keywords[] = { "keep", "delete", 0 };
            int i = utils::keyword_index(keywords, val, "CLOSE STATUS");
            status_delete = (i == 1);
            return *this;
         }

         close_chain &iostat(int &val) {
            iostat_ptr = &val;
            return *this;
         }
   };

   inline close_chain io::close(int unit) {
      return close_chain(this, unit);
   }

} // namespace fem

#endif // GUARD
// --  end: fem/close_chain.hpp ----------------------
// -- start: fem/file_positioning_chain.hpp ----------------------
#ifndef FEM_FILE_POSITIONING_CHAIN_HPP
#define FEM_FILE_POSITIONING_CHAIN_HPP

// -- repeat not included:  fem/io.hpp ----------------------

namespace fem {

   struct file_positioning_chain {
         char const *io_function;
         mutable io_unit *u_ptr;
         int *iostat_ptr;

      private:
         file_positioning_chain const &operator=(file_positioning_chain const &); // not implemented

      public:
         file_positioning_chain(char const *io_function_, io_unit *u_ptr_) :
             io_function(io_function_), u_ptr(u_ptr_), iostat_ptr(0) {}

         file_positioning_chain(file_positioning_chain const &other) :
             io_function(other.io_function), u_ptr(other.u_ptr), iostat_ptr(other.iostat_ptr) {
            other.u_ptr = 0;
         }

         ~file_positioning_chain() {
            if (u_ptr == 0) {
               throw TBXX_NOT_IMPLEMENTED(); // should open file file
            }
            if (std::strcmp(io_function, "backspace") == 0) {
               u_ptr->backspace(iostat_ptr);
            }
            else if (std::strcmp(io_function, "endfile") == 0) {
               u_ptr->endfile(iostat_ptr);
            }
            else if (std::strcmp(io_function, "rewind") == 0) {
               u_ptr->rewind(iostat_ptr);
            }
            else {
               throw TBXX_UNREACHABLE_ERROR();
            }
         }

         file_positioning_chain &iostat(int &val) {
            iostat_ptr = &val;
            return *this;
         }
   };

   inline file_positioning_chain io::backspace(int unit) {
      return file_positioning_chain("backspace", unit_ptr(unit, /*auto_open*/ true));
   }

   inline file_positioning_chain io::endfile(int unit) {
      return file_positioning_chain("endfile", unit_ptr(unit, /*auto_open*/ true));
   }

   inline file_positioning_chain io::rewind(int unit) {
      return file_positioning_chain("rewind", unit_ptr(unit, /*auto_open*/ true));
   }

} // namespace fem

#endif // GUARD
// --  end: fem/file_positioning_chain.hpp ----------------------
// -- start: fem/inquire_chain.hpp ----------------------
#ifndef FEM_INQUIRE_CHAIN_HPP
#define FEM_INQUIRE_CHAIN_HPP

// -- repeat not included:  fem/io.hpp ----------------------
// -- repeat not included:  fem/str_ref.hpp ----------------------

namespace fem {

   struct inquire_chain {
         mutable io *io_ptr;
         int unit;
         std::string file;
         bool have_file;

      private:
         inquire_chain const &operator=(inquire_chain const &); // not implemented

      public:
         inquire_chain(io *io_ptr_, int unit_) : io_ptr(io_ptr_), unit(unit_), have_file(false) {}

         inquire_chain(io *io_ptr_, std::string const &file_) :
             io_ptr(io_ptr_), unit(0), file(utils::strip_leading_and_trailing_blank_padding(file_)), have_file(true) {}

         inquire_chain(inquire_chain const &other) :
             io_ptr(other.io_ptr), unit(other.unit), file(other.file), have_file(other.have_file) {
            other.io_ptr = 0;
         }

         ~inquire_chain() {
            if (io_ptr == 0)
               return;
            // TODO
         }

         inquire_chain &iostat(int & /* val */) { throw TBXX_NOT_IMPLEMENTED(); }

         inquire_chain &exist(bool &val) {
            // f77_std 12.10.3.3
            if (have_file) {
               val = utils::path::exists(file.c_str());
            }
            else {
               val = (io_ptr->unit_ptr(unit) != 0);
            }
            return *this;
         }

         inquire_chain &opened(bool &val) {
            if (have_file) {
               val = io_ptr->is_opened_simple(file);
            }
            else {
               io_unit *u_ptr = io_ptr->unit_ptr(unit);
               val = (u_ptr != 0);
            }
            return *this;
         }

         inquire_chain &number(int & /* val */) { throw TBXX_NOT_IMPLEMENTED(); }

         inquire_chain &named(bool &) { throw TBXX_NOT_IMPLEMENTED(); }

         inquire_chain &name(str_ref val) {
            if (have_file) {
               val = utils::path::absolute(file.c_str()).c_str();
            }
            else {
               io_unit *u_ptr = io_ptr->unit_ptr(unit);
               if (u_ptr == 0) {
                  val = " ";
               }
               else {
                  val = u_ptr->file_name.c_str();
               }
            }
            return *this;
         }

         inquire_chain &access(str_ref val) {
            if (have_file) {
               throw TBXX_NOT_IMPLEMENTED();
            }
            else {
               io_unit *u_ptr = io_ptr->unit_ptr(unit);
               if (u_ptr == 0) {
                  val = "UNDEFINED";
               }
               else if (u_ptr->access == io_unit::ac_sequential) {
                  val = "SEQUENTIAL";
               }
               else if (u_ptr->access == io_unit::ac_direct) {
                  val = "DIRECT";
               }
               else {
                  val = "UNDEFINED";
               }
            }
            return *this;
         }

         inquire_chain &sequential(str_ref val) {
            if (have_file) {
               throw TBXX_NOT_IMPLEMENTED();
            }
            else {
               io_unit *u_ptr = io_ptr->unit_ptr(unit);
               if (u_ptr != 0 && u_ptr->access == io_unit::ac_sequential) {
                  val = "YES";
               }
               else {
                  val = "NO";
               }
            }
            return *this;
         }

         inquire_chain &direct(str_ref val) {
            if (have_file) {
               throw TBXX_NOT_IMPLEMENTED();
            }
            else {
               io_unit *u_ptr = io_ptr->unit_ptr(unit);
               if (u_ptr != 0 && u_ptr->access == io_unit::ac_direct) {
                  val = "YES";
               }
               else {
                  val = "NO";
               }
            }
            return *this;
         }

         inquire_chain &form(str_ref) { throw TBXX_NOT_IMPLEMENTED(); }

         inquire_chain &formatted(str_ref) { throw TBXX_NOT_IMPLEMENTED(); }

         inquire_chain &unformatted(str_ref) { throw TBXX_NOT_IMPLEMENTED(); }

         inquire_chain &recl(int &) { throw TBXX_NOT_IMPLEMENTED(); }

         inquire_chain &nextrec(int &) { throw TBXX_NOT_IMPLEMENTED(); }

         inquire_chain &blank(str_ref val) {
            if (have_file) {
               throw TBXX_NOT_IMPLEMENTED();
            }
            else {
               io_unit *u_ptr = io_ptr->unit_ptr(unit);
               if (u_ptr == 0) {
                  val = "UNDEFINED";
               }
               else if (u_ptr->blank == io_unit::bl_null) {
                  val = "NULL";
               }
               else if (u_ptr->blank == io_unit::bl_null) {
                  val = "ZERO";
               }
               else {
                  val = "UNDEFINED";
               }
            }
            return *this;
         }
   };

   inline inquire_chain io::inquire_unit(int unit) {
      return inquire_chain(this, unit);
   }

   inline inquire_chain io::inquire_file(std::string file, bool blank_padding_removed_already) {
      if (!blank_padding_removed_already) {
         file = utils::strip_leading_and_trailing_blank_padding(file);
      }
      return inquire_chain(this, file);
   }

} // namespace fem

#endif // GUARD
// --  end: fem/inquire_chain.hpp ----------------------
// -- repeat not included:  fem/main.hpp ----------------------
// -- start: fem/open_chain.hpp ----------------------
#ifndef FEM_OPEN_CHAIN_HPP
#define FEM_OPEN_CHAIN_HPP

// -- repeat not included:  fem/io.hpp ----------------------

namespace fem {

   struct open_chain {
         mutable io_unit *u_ptr;
         int *iostat_ptr;

      private:
         open_chain const &operator=(open_chain const &); // not implemented

      public:
         open_chain(io_unit *u_ptr_) : u_ptr(u_ptr_), iostat_ptr(0) {}

         open_chain(open_chain const &other) : u_ptr(other.u_ptr), iostat_ptr(other.iostat_ptr) { other.u_ptr = 0; }

         ~open_chain() {
            if (u_ptr == 0)
               return;
            u_ptr->open(iostat_ptr);
         }

         open_chain &access(std::string const &val) {
            int i = utils::keyword_index(io_unit::ac_keywords(), val, "OPEN ACCESS");
            u_ptr->access = static_cast<io_unit::access_types>(i);
            return *this;
         }

         open_chain &form(std::string const &val) {
            int i = utils::keyword_index(io_unit::fm_keywords(), val, "OPEN FORM");
            u_ptr->form = static_cast<io_unit::form_types>(i);
            return *this;
         }

         open_chain &recl(int const &val) {
            if (val <= 0) {
               throw io_err("Invalid OPEN RECL: value is zero or negative,"
                            " but must be greater than zero");
            }
            u_ptr->recl = static_cast<unsigned>(val);
            return *this;
         }

         open_chain &blank(std::string const &val) {
            int i = utils::keyword_index(io_unit::bl_keywords(), val, "OPEN BLANK");
            u_ptr->blank = static_cast<io_unit::blank_types>(i);
            return *this;
         }

         open_chain &status(std::string const &val) {
            int i = utils::keyword_index(io_unit::st_keywords(), val, "OPEN STATUS");
            u_ptr->status = static_cast<io_unit::status_types>(i);
            return *this;
         }

         open_chain &iostat(int &val) {
            iostat_ptr = &val;
            return *this;
         }
   };

   inline open_chain io::open(int unit, std::string file, bool blank_padding_removed_already) {
      if (!blank_padding_removed_already) {
         file = utils::strip_leading_and_trailing_blank_padding(file);
      }
      typedef std::map<int, io_unit>::iterator it;
      it map_iter = units.find(unit);
      if (map_iter != units.end()) {
         if (file.size() == 0) {
            file = map_iter->second.file_name; // f77_std 12.10.1.1
         }
         else if (file == map_iter->second.file_name) {
            throw TBXX_NOT_IMPLEMENTED(); // f77_std 12.10.1.1
         }
         map_iter->second.close();
         units.erase(map_iter);
      }
      map_iter = units.insert(std::make_pair(unit, io_unit(unit, file))).first;
      return open_chain(&(map_iter->second));
   }

} // namespace fem

#endif // GUARD
// --  end: fem/open_chain.hpp ----------------------
// -- repeat not included:  fem/utils/misc.hpp ----------------------

namespace fem {

   struct common {
         command_line_arguments command_line_args;
         fem::io io;

         common() {}

         common(int argc, char const *argv[]) : command_line_args(argc, argv) {}

         int iargc() const {
            size_t n = command_line_args.buffer.size();
            if (n == 0)
               return 0;
            return static_cast<int>(n) - 1;
         }

         void getarg(int pos, str_ref value) const {
            if (pos < 0 || pos >= ( int ) command_line_args.buffer.size()) {
               value = " ";
            }
            else {
               value = command_line_args.buffer[ pos ];
            }
         }
   };

   struct cmn_sve {
      private:
         // simplified boost::any, with pointer holder instead of value holder
         struct placeholder {
               virtual ~placeholder() {}
         };

         template<typename T>
         struct holder : placeholder {
               T *ptr;

               holder(T *ptr_) : ptr(ptr_) {}

               ~holder() { delete ptr; }

            private:
               holder(holder const &);
               holder const &operator=(holder const &);
         };

         placeholder *content;

         cmn_sve(cmn_sve const &);
         cmn_sve const &operator=(cmn_sve const &);

      public:
         cmn_sve() : content(0) {}

         ~cmn_sve() { delete content; }

         bool is_called_first_time() { return (content == 0); }

         template<typename T>
         void construct() {
            content = new holder<T>(new T);
         }

         template<typename T, typename D>
         void construct(D const &dynamic_parameters) {
            content = new holder<T>(new T(dynamic_parameters));
         }

         template<typename T>
         T &get() {
            return *(static_cast<holder<T> *>(content)->ptr);
         }
   };

   template<typename T>
   void no_operation_to_avoid_unused_variable_warning(const T &) {}

} // namespace fem

#define FEM_CMN_SVE(FUNC) \
   bool is_called_first_time = cmn.FUNC##_sve.is_called_first_time(); \
   if (is_called_first_time) { \
      cmn.FUNC##_sve.construct<FUNC##_save>(); \
   } \
   FUNC##_save &sve = cmn.FUNC##_sve.get<FUNC##_save>(); \
   fem::no_operation_to_avoid_unused_variable_warning(sve)

#define FEM_CMN_SVE_DYNAMIC_PARAMETERS(FUNC) \
   bool is_called_first_time = cmn.FUNC##_sve.is_called_first_time(); \
   if (is_called_first_time) { \
      cmn.FUNC##_sve.construct<FUNC##_save>(cmn.dynamic_params); \
   } \
   FUNC##_save &sve = cmn.FUNC##_sve.get<FUNC##_save>()

#endif // GUARD
// --  end: fem/common.hpp ----------------------
// -- repeat not included:  fem/format.hpp ----------------------
// -- repeat not included:  fem/star.hpp ----------------------
// -- repeat not included:  fem/str_arr_ref.hpp ----------------------
// -- repeat not included:  fem/utils/misc.hpp ----------------------
// -- start: fem/utils/string_to_double_fmt.hpp ----------------------
#ifndef FEM_UTILS_STRING_TO_DOUBLE_FMT_HPP
#define FEM_UTILS_STRING_TO_DOUBLE_FMT_HPP

// -- start: fem/utils/string_to_double.hpp ----------------------
#ifndef FEM_UTILS_STRING_TO_DOUBLE_HPP
#define FEM_UTILS_STRING_TO_DOUBLE_HPP

// -- repeat not included:  fem/utils/char.hpp ----------------------
// -- repeat not included:  fem/utils/simple_streams.hpp ----------------------
// -- start: tbxx/optional_copy.hpp ----------------------
#ifndef TBXX_OPTIONAL_COPY_H
#define TBXX_OPTIONAL_COPY_H

namespace tbxx {

   //! Optional allocation of value with new, with value-copy semantics.
   template<typename ValueType>
   class optional_copy {
      public:
         typedef ValueType value_type;

      protected:
         ValueType *ptr_;

      public:
         optional_copy() : ptr_(0) {}

         ~optional_copy() { delete ptr_; }

         optional_copy(optional_copy const &other) : ptr_(other.ptr_ == 0 ? 0 : new ValueType(*(other.ptr_))) {}

         optional_copy &operator=(optional_copy const &other) {
            delete ptr_;
            ptr_ = (other.ptr_ == 0 ? 0 : new ValueType(*(other.ptr_)));
            return *this;
         }

         explicit optional_copy(ValueType const &value) : ptr_(new ValueType(value)) {}

         optional_copy &operator=(ValueType const &value) {
            delete ptr_;
            ptr_ = new ValueType(value);
            return *this;
         }

         void release() {
            delete ptr_;
            ptr_ = 0;
         }

         operator bool() const { return (ptr_ != 0); }

         ValueType *get() { return ptr_; }

         ValueType const *get() const { return ptr_; }

         ValueType *operator->() { return ptr_; }

         ValueType const *operator->() const { return ptr_; }

         ValueType &operator*() { return *ptr_; }

         ValueType const &operator*() const { return *ptr_; }
   };

   /*! \brief Same behavior as optional_copy, but with operator[] as
     shortcut for (*get())[]
   */
   template<typename ContainerType>
   class optional_container : public optional_copy<ContainerType> {
      public:
         optional_container() {}

         optional_container(optional_container const &other) : optional_copy<ContainerType>(other) {}

         optional_container &operator=(optional_container const &other) {
            delete this->ptr_;
            this->ptr_ = (other.ptr_ == 0 ? 0 : new ContainerType(*(other.ptr_)));
            return *this;
         }

         explicit optional_container(ContainerType const &container) : optional_copy<ContainerType>(container) {}

         optional_container &operator=(ContainerType const &container) {
            delete this->ptr_;
            this->ptr_ = new ContainerType(container);
            return *this;
         }

         typename ContainerType::value_type &operator[](typename ContainerType::size_type const &i) {
            return (*this->ptr_)[ i ];
         }

         typename ContainerType::value_type const &operator[](typename ContainerType::size_type const &i) const {
            return (*this->ptr_)[ i ];
         }
   };

} // namespace tbxx

#endif // GUARD
// --  end: tbxx/optional_copy.hpp ----------------------

namespace fem {
   namespace utils {

      template<size_t Size>
      struct string_to_double_ad_hoc_limts;

      // Simple upper estimates, not guarding against floating-point errors,
      // but guarding against unusual sizeof(double).
      template<>
      struct string_to_double_ad_hoc_limts<8> {
            static const unsigned max_digit_count = 16;
            static const unsigned max_exp_int = 308;
      };

      // Ideally in string_to_double_ad_hoc_limts<8>, but it does not
      // seem to be straightforward to do.
      static const double one_e_minus_0_16[] = { 1e0,  1e-1,  1e-2,  1e-3,  1e-4,  1e-5,  1e-6,  1e-7, 1e-8,
                                                 1e-9, 1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16 };
      static const double one_e_two_pow_0_8[] = { 1e1, 1e2, 1e4, 1e8, 1e16, 1e32, 1e64, 1e128, 1e256 };

      struct string_to_double {
            double result;
            tbxx::optional_copy<std::string> error_message;
            bool stream_end;

            string_to_double() {}

            string_to_double(simple_istream &inp, unsigned decimal_point_substitute = 0, int exp_scale_substitute = 0) {
               reset();
               convert(inp, decimal_point_substitute, exp_scale_substitute);
            }

            void reset() {
               result = 0;
               error_message.release();
               stream_end = false;
            }

            void set_error_message(int c) {
               static const std::string inp_err("Input error while reading floating-point value.");
               static const std::string inp_eoi("End of input while reading floating-point value.");
               static const char *err_inv = "Invalid character while reading floating-point value: ";
               if (is_stream_err(c)) {
                  error_message = inp_err;
               }
               else if (is_stream_end(c)) {
                  error_message = inp_eoi;
                  stream_end = true;
               }
               else {
                  error_message = err_inv + format_char_for_display(c);
               }
            }

            void convert(
               simple_istream &inp,
               unsigned decimal_point_substitute = 0, // "d" part of Fortran "w.d" format
               int exp_scale_substitute = 0) // Fortran "P" scaling
            {
               typedef string_to_double_ad_hoc_limts<sizeof(double)> ad_hoc_limits;
               static const std::string err_oor("Out-of-range error while reading floating-point value.");
               int c = inp.get();
               while (is_whitespace(c)) {
                  c = inp.get();
               }
               bool sign_flag;
               if (c == '-') {
                  sign_flag = true;
                  c = inp.get();
               }
               else {
                  sign_flag = false;
                  if (c == '+') {
                     c = inp.get();
                  }
               }
               bool had_dot = false;
               bool had_digit = false;
               bool had_19 = false;
               int rexp = -1;
               int dexp = 0;
               if (c == '.') {
                  had_dot = true;
                  c = inp.get();
               }
               while (is_digit(c)) {
                  had_digit = true;
                  int d = digit_as_int(c);
                  if (d != 0) {
                     had_19 = true;
                     if (dexp <= ( int ) ad_hoc_limits::max_digit_count) {
                        result += d * one_e_minus_0_16[ dexp ];
                     }
                  }
                  if (had_19) {
                     if (!had_dot)
                        rexp++;
                     dexp++;
                  }
                  else if (had_dot) {
                     rexp--;
                  }
                  c = inp.get();
                  if (c == '.') {
                     if (had_dot) {
                        break;
                     }
                     had_dot = true;
                     c = inp.get();
                  }
               }
               if (!had_digit) {
                  set_error_message(c);
                  return;
               }
               if (sign_flag)
                  result *= -1;
               if (!had_dot && decimal_point_substitute > 0) {
                  rexp -= decimal_point_substitute;
               }
               int exp_int = 0;
               if (c == 'e' || c == 'E' || c == 'd' || c == 'D') {
                  c = inp.get();
                  bool exp_sign_flag;
                  if (c == '-') {
                     exp_sign_flag = true;
                     c = inp.get();
                  }
                  else {
                     exp_sign_flag = false;
                     if (c == '+') {
                        c = inp.get();
                     }
                  }
                  if (!is_digit(c)) {
                     result = 0;
                     set_error_message(c);
                     return;
                  }
                  int exp_int_limit = rexp;
                  if (exp_int_limit < 0)
                     exp_int_limit *= -1;
                  exp_int_limit += ad_hoc_limits::max_exp_int;
                  exp_int = digit_as_int(c);
                  while (true) {
                     c = inp.get();
                     if (!is_digit(c)) {
                        break;
                     }
                     exp_int *= 10;
                     exp_int += digit_as_int(c);
                     if (exp_int > exp_int_limit) {
                        result = 0;
                        error_message = err_oor;
                        return;
                     }
                  }
                  if (exp_sign_flag)
                     exp_int *= -1;
               }
               else {
                  exp_int = -exp_scale_substitute;
               }
               if (!is_stream_end_or_err(c)) {
                  inp.backup();
               }
               exp_int += rexp;
               bool exp_sign_flag = (exp_int < 0);
               if (exp_sign_flag)
                  exp_int *= -1;
               // using ideas found in ruby-1.8.6.tar.gz, missing/strtod.c
               double exp_double = 1;
               for (unsigned i = 0; exp_int != 0; i++) {
                  if (exp_int & 1)
                     exp_double *= one_e_two_pow_0_8[ i ];
                  exp_int >>= 1;
               }
               if (exp_sign_flag)
                  result /= exp_double;
               else
                  result *= exp_double;
            }
      };

   } // namespace utils
} // namespace fem

#endif // GUARD
// --  end: fem/utils/string_to_double.hpp ----------------------

namespace fem {
   namespace utils {

      struct string_to_double_fmt : string_to_double {
            string_to_double_fmt() {}

            string_to_double_fmt(simple_istream &inp, unsigned w, unsigned d, bool blanks_zero, int exp_scale) {
               reset();
               if (w == 0)
                  return;
               std::string buf;
               buf.reserve(w);
               for (unsigned i = 0; i < w; i++) {
                  int c = inp.get();
                  if (is_stream_err(c)) {
                     set_error_message(c);
                     return;
                  }
                  if (is_stream_end(c)) {
                     if (i == 0) {
                        set_error_message(c);
                        return;
                     }
                     break;
                  }
                  if (is_end_of_line(c)) {
                     inp.backup();
                     break;
                  }
                  if (c == ' ') {
                     if (blanks_zero)
                        buf += '0';
                  }
                  else {
                     buf += c;
                  }
               }
               if (buf.size() == 0)
                  return;
               simple_istream_from_std_string buf_inp(buf);
               convert(buf_inp, d, exp_scale);
            }
      };

   } // namespace utils
} // namespace fem

#endif // GUARD
// --  end: fem/utils/string_to_double_fmt.hpp ----------------------

#define IOSTAT_OK 0
#define IOSTAT_ERROR 1
#define IOSTAT_END -1

namespace fem {

   class read_loop // TODO copy-constructor potential performance problem
   {
      private:
         utils::slick_ptr<utils::simple_istream> inp;
         bool first_inp_get;
         format::token_loop fmt_loop;
         bool blanks_zero;
         int exp_scale;
         io_modes io_mode;
         int *iostat_ptr;

      public:
         read_loop(common &cmn, int const &unit, unformatted_type const &) :
             inp(cmn.io.simple_istream(unit)), first_inp_get(true), blanks_zero(false), exp_scale(0),
             io_mode(io_unformatted), iostat_ptr(0) {}

         read_loop(common &cmn, int const &unit, star_type const &) :
             inp(cmn.io.simple_istream(unit)), first_inp_get(true), blanks_zero(false), exp_scale(0),
             io_mode(io_list_directed), iostat_ptr(0) {}

         read_loop(common &cmn, int const &unit, str_cref fmt) :
             inp(cmn.io.simple_istream(unit)), first_inp_get(true), fmt_loop(fmt), blanks_zero(false), exp_scale(0),
             io_mode(io_formatted), iostat_ptr(0) {}

         read_loop(str_cref const &internal_file, star_type const &) :
             inp(utils::slick_ptr<utils::simple_istream>(
                new utils::simple_istream_from_char_ptr_and_size(internal_file.elems(), internal_file.len()))),
             first_inp_get(true), blanks_zero(false), exp_scale(0), io_mode(io_list_directed), iostat_ptr(0) {}

         read_loop(str_cref const &internal_file, str_cref fmt) :
             inp(utils::slick_ptr<utils::simple_istream>(
                new utils::simple_istream_from_char_ptr_and_size(internal_file.elems(), internal_file.len()))),
             first_inp_get(true), fmt_loop(fmt), blanks_zero(false), exp_scale(0), io_mode(io_formatted),
             iostat_ptr(0) {}

         read_loop &rec(int const &) {
            inp.reset();
            throw TBXX_NOT_IMPLEMENTED();
         }

         read_loop &iostat(int &iostat) {
            this->iostat_ptr = &iostat;
            iostat = IOSTAT_OK;
            return *this;
         }

         std::string const &next_edit_descriptor() {
            while (true) {
               utils::token const *t = fmt_loop.next_executable_token();
               std::string const &tv = t->value;
               if (t->type == "string") {
                  inp.reset();
                  throw TBXX_NOT_IMPLEMENTED();
               }
               else if (t->type == "op") {
                  if (tv[ 0 ] == ':') {
                     // ignored
                  }
                  else if (tv[ 0 ] == '/') {
                     skip_to_end_of_line();
                     int c = inp_get();
                     if (utils::is_stream_end(c)) {
                        inp.reset();
                        if (this->iostat_ptr != 0)
                           *iostat_ptr = IOSTAT_END;
                        throw read_end("End of input while reading string");
                     }
                     inp->backup();
                  }
                  else if (tv[ 0 ] == '$') {
                     inp.reset();
                     throw TBXX_NOT_IMPLEMENTED();
                  }
                  else {
                     inp.reset();
                     throw TBXX_UNREACHABLE_ERROR();
                  }
               }
               else if (t->type == "format") {
                  if (utils::ends_with_char(tv, 'x')) {
                     if (tv.size() == 1) {
                        process_fmt_x(1);
                     }
                     else {
                        process_fmt_x(utils::signed_integer_value(tv.data(), 0, tv.size() - 1));
                     }
                  }
                  else if (std::strchr("adefgilz", tv[ 0 ]) != 0) {
                     return tv;
                  }
                  else if (utils::ends_with_char(tv, 'p')) {
                     if (tv.size() == 1) {
                        exp_scale = 1;
                     }
                     else {
                        exp_scale = utils::signed_integer_value(tv.data(), 0, tv.size() - 1);
                     }
                  }
                  else if (tv[ 0 ] == 't') {
                     inp.reset();
                     throw TBXX_NOT_IMPLEMENTED();
                  }
                  else if (tv[ 0 ] == 's') {
                     inp.reset();
                     throw TBXX_NOT_IMPLEMENTED();
                  }
                  else if (tv[ 0 ] == 'b') {
                     blanks_zero = (tv[ 1 ] == 'z');
                  }
                  else {
                     inp.reset();
                     throw TBXX_UNREACHABLE_ERROR();
                  }
               }
               else {
                  inp.reset();
                  throw TBXX_UNREACHABLE_ERROR();
               }
            }
         }

         int inp_get() {
            int result = inp->get();
            if (utils::is_stream_err(result)) {
               inp.reset();
               if (this->iostat_ptr != 0)
                  *iostat_ptr = IOSTAT_ERROR;
               throw io_err("Error during read");
            }
            if (first_inp_get || io_mode == io_unformatted) {
               first_inp_get = false;
               if (utils::is_stream_end(result)) {
                  inp.reset();
                  if (this->iostat_ptr != 0)
                     *iostat_ptr = IOSTAT_END;
                  throw read_end("End of input during read");
               }
            }
            if (io_mode == io_formatted && result == '\r') {
               int next_char = inp->get();
               if (next_char == '\n') {
                  result = '\n';
               }
               else {
                  inp->backup();
               }
            }
            return result;
         }

         void process_fmt_x(unsigned n) {
            for (unsigned i = 0; i < n; i++) {
               int c = inp_get();
               if (c == utils::stream_end) {
                  return;
               }
               if (utils::is_end_of_line(c)) {
                  inp->backup();
                  return;
               }
            }
         }

         read_loop &operator,(char & /* val */) {
            inp.reset();
            throw TBXX_NOT_IMPLEMENTED();
            return *this;
         }

         read_loop &operator,(bool &val) {
            if (io_mode == io_unformatted) {
               from_stream_unformatted(reinterpret_cast<char *>(&val), sizeof(bool));
            }
            else if (io_mode == io_list_directed) {
               inp.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            else {
               inp.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

         read_loop &operator,(integer_star_1 & /* val */) {
            if (io_mode == io_unformatted) {
               inp.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            else if (io_mode == io_list_directed) {
               inp.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            else {
               inp.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

         read_loop &operator,(integer_star_2 &val) {
            if (io_mode == io_unformatted) {
               from_stream_unformatted(reinterpret_cast<char *>(&val), sizeof(integer_star_2));
            }
            else if (io_mode == io_list_directed) {
               inp.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            else {
               inp.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

         read_loop &operator,(integer_star_4 &val) {
            if (io_mode == io_unformatted) {
               from_stream_unformatted(reinterpret_cast<char *>(&val), sizeof(integer_star_4));
            }
            else if (io_mode == io_list_directed) {
               val = static_cast<int>(read_star_long());
            }
            else {
               std::string const &ed = next_edit_descriptor();
               int n = ed.size();
               if (ed[ 0 ] == 'i' && n > 1) {
                  n = utils::unsigned_integer_value(ed.data(), 1, n);
                  val = static_cast<int>(read_fmt_long(n));
               }
               else {
                  val = static_cast<int>(read_star_long());
               }
            }
            return *this;
         }

         read_loop &operator,(integer_star_8 &val) {
            if (io_mode == io_unformatted) {
               from_stream_unformatted(reinterpret_cast<char *>(&val), sizeof(integer_star_8));
            }
            else if (io_mode == io_list_directed) {
               inp.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            else {
               inp.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

         read_loop &operator,(float &val) {
            if (io_mode == io_unformatted) {
               from_stream_unformatted(reinterpret_cast<char *>(&val), sizeof(float));
            }
            else {
               val = static_cast<float>((io_mode == io_formatted ? read_fmt_double() : read_star_double()));
            }
            return *this;
         }

         read_loop &operator,(double &val) {
            if (io_mode == io_unformatted) {
               from_stream_unformatted(reinterpret_cast<char *>(&val), sizeof(double));
            }
            else {
               val = (io_mode == io_formatted ? read_fmt_double() : read_star_double());
            }
            return *this;
         }

         read_loop &operator,(std::complex<float> &val) {
            if (io_mode == io_unformatted) {
               float re, im;
               from_stream_unformatted(reinterpret_cast<char *>(&re), sizeof(float));
               from_stream_unformatted(reinterpret_cast<char *>(&im), sizeof(float));
               val = std::complex<float>(re, im);
            }
            else if (io_mode == io_list_directed) {
               inp.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            else {
               inp.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

         read_loop &operator,(std::complex<double> &val) {
            if (io_mode == io_unformatted) {
               double re, im;
               from_stream_unformatted(reinterpret_cast<char *>(&re), sizeof(double));
               from_stream_unformatted(reinterpret_cast<char *>(&im), sizeof(double));
               val = std::complex<double>(re, im);
            }
            else if (io_mode == io_list_directed) {
               inp.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            else {
               inp.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

         read_loop &operator,(str_ref const &val) {
            if (io_mode == io_unformatted) {
               from_stream_unformatted(val.elems(), val.len());
            }
            else {
               int vl = val.len();
               int n = vl;
               if (io_mode == io_formatted) {
                  std::string const &ed = next_edit_descriptor();
                  if (ed[ 0 ] == 'a' && ed.size() > 1) {
                     n = utils::unsigned_integer_value(ed.data(), 1, ed.size());
                  }
               }
               for (int i = 0; i < n - vl; i++) {
                  int c = inp_get();
                  if (utils::is_stream_end(c)) {
                     if (i == 0) {
                        inp.reset();
                        if (this->iostat_ptr != 0)
                           *iostat_ptr = IOSTAT_END;
                        throw read_end("End of input while reading string");
                     }
                     break;
                  }
                  if (utils::is_end_of_line(c)) {
                     inp->backup();
                  }
               }
               for (int i = 0; i < vl; i++) {
                  int c = inp_get();
                  if (utils::is_stream_end(c)) {
                     if (i == 0) {
                        inp.reset();
                        if (this->iostat_ptr != 0)
                           *iostat_ptr = IOSTAT_END;
                        throw read_end("End of input while reading string");
                     }
                     val[ i ] = ' ';
                  }
                  else if (utils::is_end_of_line(c)) {
                     inp->backup();
                     val[ i ] = ' ';
                  }
                  else {
                     val[ i ] = c;
                  }
               }
            }
            return *this;
         }

         template<typename T, size_t Ndims>
         read_loop &operator,(arr_ref<T, Ndims> const &val) {
            T *v = val.begin();
            size_t n = val.size_1d();
            for (size_t i = 0; i < n; i++) {
               (*this), v[ i ];
            }
            return *this;
         }

         template<size_t Ndims>
         read_loop &operator,(str_arr_ref<Ndims> const &val) {
            size_t n = val.size_1d();
            int l = val.len();
            char *val_begin = val.begin();
            for (size_t i = 0; i < n; i++) {
               (*this), str_ref(&val_begin[ i * l ], l);
            }
            return *this;
         }

         void skip_to_end_of_line() {
            while (true) {
               int c = inp_get();
               if (utils::is_stream_end(c) || utils::is_end_of_line(c)) {
                  break;
               }
            }
         }

         ~read_loop() {
            if (inp.get() == 0)
               return;
            if (io_mode == io_unformatted) {
               skip_to_end_of_unformatted_record();
            }
            else {
               skip_to_end_of_line();
            }
         }

         long read_fmt_long(unsigned n) {
            bool had_non_blank = false;
            bool negative = false;
            long result = 0;
            for (unsigned i = 0; i < n; i++) {
               int c = inp_get();
               if (utils::is_stream_end(c)) {
                  break;
               }
               if (c == ',') {
                  break;
               }
               if (utils::is_end_of_line(c)) {
                  inp->backup();
                  break;
               }
               if (c != ' ') {
                  if (!had_non_blank) {
                     had_non_blank = true;
                     if (c == '-') {
                        negative = true;
                        continue;
                     }
                     if (c == '+') {
                        continue;
                     }
                  }
                  if (!utils::is_digit(c)) {
                     inp.reset();
                     if (this->iostat_ptr != 0)
                        *iostat_ptr = IOSTAT_ERROR;
                     throw io_err("Invalid character while reading integer value.");
                  }
                  result *= 10;
                  result += utils::digit_as_int(c);
               }
            }
            if (negative)
               result *= -1;
            return result;
         }

         long read_star_long() {
            while (true) { // loop scanning for first non-whitespace
               int c = inp_get();
               if (utils::is_stream_end(c)) {
                  inp.reset();
                  if (this->iostat_ptr != 0)
                     *iostat_ptr = IOSTAT_END;
                  throw read_end("End of input while reading integer value");
               }
               if (!utils::is_whitespace(c)) {
                  bool negative = (c == '-');
                  if (negative || c == '+') {
                     c = inp_get();
                     if (utils::is_stream_end(c)) {
                        read_end("End of input while reading integer value");
                     }
                  }
                  long result = 0;
                  while (true) { // loop collecting digits
                     if (!utils::is_digit(c)) {
                        io_err("Invalid character while reading integer value.");
                     }
                     result *= 10;
                     result += utils::digit_as_int(c);
                     c = inp_get();
                     if (utils::is_stream_end(c) || utils::is_whitespace(c) || c == ',') {
                        if (negative)
                           result *= -1;
                        if (utils::is_end_of_line(c))
                           inp->backup();
                        return result;
                     }
                  }
               }
            }
         }

         void throw_if_conv_error_message(utils::string_to_double const &conv) {
            if (conv.error_message) {
               inp.reset();
               if (conv.stream_end) {
                  if (this->iostat_ptr != 0)
                     *iostat_ptr = IOSTAT_END;
                  throw read_end(*conv.error_message);
               }
               if (this->iostat_ptr != 0)
                  *iostat_ptr = IOSTAT_ERROR;
               throw io_err(*conv.error_message);
            }
         }

         double read_fmt_double() {
            std::string const &ed = next_edit_descriptor();
            int n = ed.size();
            if (n < 2 || std::strchr("defg", ed[ 0 ]) == 0) {
               return read_star_double();
            }
            int iw = utils::unsigned_integer_scan(ed.data(), 1, ed.size());
            int w = utils::unsigned_integer_value(ed.data(), 1, iw);
            int d = 0;
            if (iw + 1 != ( int ) ed.size()) {
               d = utils::unsigned_integer_value(ed.data(), iw + 1, ed.size());
            }
            utils::string_to_double_fmt conv(*inp, w, d, blanks_zero, exp_scale);
            throw_if_conv_error_message(conv);
            first_inp_get = false;
            return conv.result;
         }

         double read_star_double() {
            utils::string_to_double conv(*inp);
            throw_if_conv_error_message(conv);
            first_inp_get = false;
            int c = inp_get();
            if (utils::is_stream_end(c) || utils::is_whitespace(c) || c == ',') {
               if (utils::is_end_of_line(c))
                  inp->backup();
               return conv.result;
            }
            inp.reset();
            if (this->iostat_ptr != 0)
               *iostat_ptr = IOSTAT_ERROR;
            throw io_err("Invalid character while reading floating-point value: " + utils::format_char_for_display(c));
         }

         void from_stream_unformatted(char *target, unsigned target_size) {
            for (unsigned i = 0; i < target_size; i++) {
               int ic = inp_get();
               char c = static_cast<char>(ic);
               if (c == end_of_unformatted_record) {
                  if (inp_get() != ic) {
                     inp.reset();
                     if (this->iostat_ptr != 0)
                        *iostat_ptr = IOSTAT_END;
                     throw read_end("End of record during unformatted read");
                  }
               }
               target[ i ] = c;
            }
         }

         void skip_to_end_of_unformatted_record() {
            while (true) {
               char c = static_cast<char>(inp_get());
               if (c == end_of_unformatted_record) {
                  if (inp_get() == 0) {
                     break;
                  }
               }
            }
         }
   };

   struct read_from_string : public read_loop {
         read_from_string(std::string const &internal_file, str_cref fmt) :
             read_loop(str_cref(internal_file.data(), static_cast<int>(internal_file.size())), fmt) {}
   };

   struct common_read {
         common &cmn;

         common_read(common &cmn_) : cmn(cmn_) {}

         read_loop operator()(int unit, unformatted_type const &) {
            read_loop result(cmn, unit, unformatted);
            return result;
         }

         read_loop operator()(int unit, star_type const &) {
            read_loop result(cmn, unit, star);
            return result;
         }

         read_loop operator()(int const &unit, str_cref fmt) {
            read_loop result(cmn, unit, fmt);
            return result;
         }

         read_loop operator()(str_cref const &internal_file, star_type const &) {
            read_loop result(internal_file, star);
            return result;
         }

         read_loop operator()(str_cref const &internal_file, str_cref fmt) {
            read_loop result(internal_file, fmt);
            return result;
         }
   };

} // namespace fem

#endif // GUARD
// --  end: fem/read.hpp ----------------------
// -- start: fem/variant.hpp ----------------------
#ifndef FEM_VARIANT_HPP
#define FEM_VARIANT_HPP

// -- repeat not included:  fem/str_arr_ref.hpp ----------------------
// -- start: fem/utils/equivalence.hpp ----------------------
#ifndef FEM_UTILS_EQUIVALENCE_HPP
#define FEM_UTILS_EQUIVALENCE_HPP

// -- repeat not included:  fem/size_t.hpp ----------------------
// -- repeat not included:  tbxx/error_utils.hpp ----------------------
#include <algorithm>
#include <vector>

namespace fem {
   namespace utils {
      namespace equivalence {

         struct array_alignment {
               size_t members_size;
               std::vector<ssize_t> diff_matrix;
               std::vector<ssize_t> diffs0;

               array_alignment(size_t members_size_) :
                   members_size(members_size_), diff_matrix(members_size * (members_size - 1), ssize_t_max) {}

               static std::string msg_prefix() { return "equivalence::array_alignment: "; }

               void add_anchor(size_t i0, ssize_t a0, size_t i1, ssize_t a1) {
                  static const char *msg_directly = "directly conflicting input";
                  if (i0 == i1) {
                     if (a0 != a1) {
                        throw std::runtime_error(msg_prefix() + msg_directly);
                     }
                  }
                  else {
                     size_t n = members_size;
                     size_t i;
                     ssize_t d;
                     if (i0 < i1) {
                        i = i0 * n + i1;
                        d = a0 - a1;
                     }
                     else {
                        i = i1 * n + i0;
                        d = a1 - a0;
                     }
                     ssize_t dd = diff_matrix[ i ];
                     if (dd == ssize_t_max) {
                        diff_matrix[ i ] = d;
                     }
                     else if (dd != d) {
                        throw std::runtime_error(msg_prefix() + msg_directly);
                     }
                  }
               }

               void infer_diffs0_from_diff_matrix() {
                  size_t n = members_size;
                  std::vector<size_t> cluster_indices(n);
                  for (size_t i = 0; i < n; i++)
                     cluster_indices[ i ] = i;
                  std::vector<std::vector<ssize_t_2>> clusters(n);
                  for (size_t li0 = 0; li0 < n - 1; li0++) {
                     for (size_t li1 = li0 + 1; li1 < n; li1++) {
                        size_t i0 = li0;
                        size_t i1 = li1;
                        size_t i = i0 * n + i1;
                        ssize_t d = diff_matrix[ i ];
                        if (d != ssize_t_max) {
                           size_t ci0 = cluster_indices[ i0 ];
                           size_t ci1 = cluster_indices[ i1 ];
                           if (ci0 == ci1) {
                              continue;
                           }
                           if (ci0 > ci1) {
                              std::swap(ci0, ci1);
                              d *= -1;
                              std::swap(i0, i1);
                           }
                           if (ci1 != i1) {
                              continue;
                           }
                           std::vector<ssize_t_2> &c0 = clusters[ ci0 ];
                           std::vector<ssize_t_2> &c1 = clusters[ ci1 ];
                           if (ci0 != i0) {
                              for (size_t i = 0; i != c0.size(); i++) {
                                 ssize_t const *c0e = c0[ i ].elems;
                                 if (c0e[ 0 ] == ( int ) i0) {
                                    d += c0e[ 1 ];
                                    break;
                                 }
                              }
                           }
                           c0.push_back(ssize_t_2(ci1, d));
                           cluster_indices[ ci1 ] = ci0;
                           for (size_t i = 0; i != c1.size(); i++) {
                              ssize_t const *c1e = c1[ i ].elems;
                              c0.push_back(ssize_t_2(c1e[ 0 ], c1e[ 1 ] + d));
                              cluster_indices[ c1e[ 0 ] ] = ci0;
                           }
                           clusters[ ci1 ].clear();
                        }
                     }
                  }
                  for (size_t i = 0; i < n; i++) {
                     if (cluster_indices[ i ] != 0) {
                        throw std::runtime_error(msg_prefix() + "insufficient input");
                     }
                  }
                  std::vector<ssize_t_2> &c0 = clusters[ 0 ];
                  TBXX_ASSERT(c0.size() == n - 1);
                  diffs0.clear();
                  diffs0.resize(n, ssize_t_max);
                  diffs0[ 0 ] = 0;
                  for (size_t i = 0; i != c0.size(); i++) {
                     ssize_t const *c0e = c0[ i ].elems;
                     TBXX_ASSERT(c0e[ 0 ] != 0);
                     TBXX_ASSERT(diffs0[ c0e[ 0 ] ] == ssize_t_max);
                     diffs0[ c0e[ 0 ] ] = c0e[ 1 ];
                  }
                  for (size_t i0 = 0; i0 < n - 1; i0++) {
                     for (size_t i1 = i0 + 1; i1 < n; i1++) {
                        size_t i = i0 * n + i1;
                        ssize_t d = diff_matrix[ i ];
                        if (d != ssize_t_max && diffs0[ i1 ] - diffs0[ i0 ] != d) {
                           throw std::runtime_error(msg_prefix() + "indirectly conflicting input");
                        }
                     }
                  }
               }
         };

      } // namespace equivalence
   } // namespace utils
} // namespace fem

#endif // GUARD
// --  end: fem/utils/equivalence.hpp ----------------------
// -- repeat not included:  fem/utils/misc.hpp ----------------------
// -- repeat not included:  tbxx/optional_copy.hpp ----------------------

namespace fem {

   using tbxx::optional_copy;

   struct variant_member {
         size_t type_size;
         optional_copy<dim_buffer> dims;

         variant_member() {}

         variant_member(size_t type_size_) : type_size(type_size_) {}

         template<typename DimsType>
         variant_member(size_t type_size_, DimsType const &dims_) : type_size(type_size_), dims(dims_) {}

         size_t number_of_bytes() const {
            if (!dims)
               return type_size;
            return type_size * dims->actual_size_1d();
         }

         size_t actual_index_1d(arr_index const &arr_ix) {
            TBXX_ASSERT(dims.get() != 0);
            return dims->actual_index_1d(arr_ix);
         }
   };

   template<typename T>
   struct mbr : variant_member {
         mbr() : variant_member(sizeof(T)) {}

         template<typename DimsType>
         mbr(DimsType const &dims_) : variant_member(sizeof(T), dims_) {}
   };

   template<int StrLen>
   struct mbr<str<StrLen>> : variant_member {
         mbr() : variant_member(StrLen) {}

         template<typename DimsType>
         mbr(DimsType const &dims_) : variant_member(StrLen, dims_) {}
   };

   struct equivalence {
         static const size_t buffer_capacity = 7;
         size_t members_size;
         variant_member members[ buffer_capacity ];
         utils::equivalence::array_alignment array_alignment;
         size_t align_i_mbr;
         size_t align_byte_offset;

         equivalence(variant_member const &mbr_1, variant_member const &mbr_2) :
             members_size(2), array_alignment(members_size), align_i_mbr(size_t_max), align_byte_offset(size_t_max) {
            members[ 0 ] = mbr_1;
            members[ 1 ] = mbr_2;
         }

         equivalence(variant_member const &mbr_1, variant_member const &mbr_2, variant_member const &mbr_3) :
             members_size(3), array_alignment(members_size), align_i_mbr(size_t_max), align_byte_offset(size_t_max) {
            members[ 0 ] = mbr_1;
            members[ 1 ] = mbr_2;
            members[ 2 ] = mbr_3;
         }

         equivalence(
            variant_member const &mbr_1, variant_member const &mbr_2, variant_member const &mbr_3,
            variant_member const &mbr_4) :
             members_size(4), array_alignment(members_size), align_i_mbr(size_t_max), align_byte_offset(size_t_max) {
            members[ 0 ] = mbr_1;
            members[ 1 ] = mbr_2;
            members[ 2 ] = mbr_3;
            members[ 3 ] = mbr_4;
         }

         equivalence(
            variant_member const &mbr_1, variant_member const &mbr_2, variant_member const &mbr_3,
            variant_member const &mbr_4, variant_member const &mbr_5) :
             members_size(5), array_alignment(members_size), align_i_mbr(size_t_max), align_byte_offset(size_t_max) {
            members[ 0 ] = mbr_1;
            members[ 1 ] = mbr_2;
            members[ 2 ] = mbr_3;
            members[ 3 ] = mbr_4;
            members[ 4 ] = mbr_5;
         }

         equivalence(
            variant_member const &mbr_1, variant_member const &mbr_2, variant_member const &mbr_3,
            variant_member const &mbr_4, variant_member const &mbr_5, variant_member const &mbr_6) :
             members_size(6), array_alignment(members_size), align_i_mbr(size_t_max), align_byte_offset(size_t_max) {
            members[ 0 ] = mbr_1;
            members[ 1 ] = mbr_2;
            members[ 2 ] = mbr_3;
            members[ 3 ] = mbr_4;
            members[ 4 ] = mbr_5;
            members[ 5 ] = mbr_6;
         }

         equivalence(
            variant_member const &mbr_1, variant_member const &mbr_2, variant_member const &mbr_3,
            variant_member const &mbr_4, variant_member const &mbr_5, variant_member const &mbr_6,
            variant_member const &mbr_7) :
             members_size(7), array_alignment(members_size), align_i_mbr(size_t_max), align_byte_offset(size_t_max) {
            members[ 0 ] = mbr_1;
            members[ 1 ] = mbr_2;
            members[ 2 ] = mbr_3;
            members[ 3 ] = mbr_4;
            members[ 4 ] = mbr_5;
            members[ 5 ] = mbr_6;
            members[ 6 ] = mbr_7;
         }

         template<size_t Jmbr>
         equivalence &align(size_t index_1d = 0, size_t str_offset = 0) {
            align_i_mbr = Jmbr - 1;
            align_byte_offset = members[ align_i_mbr ].type_size * index_1d + str_offset;
            return *this;
         }

         template<size_t Jmbr>
         equivalence &align(arr_index const &arr_ix) {
            align<Jmbr>(members[ Jmbr - 1 ].actual_index_1d(arr_ix));
            return *this;
         }

         template<size_t Jmbr>
         equivalence &align(arr_and_str_indices<arr_dim_max> const &indices) {
            align<Jmbr>(
               members[ Jmbr - 1 ].actual_index_1d(indices.arr_ix), static_cast<size_t>(indices.str_ix.first - 1));
            return *this;
         }

         template<size_t Jmbr>
         equivalence &align(str_index const &ix) {
            align<Jmbr>(0, static_cast<size_t>(ix.first - 1));
            return *this;
         }

         template<size_t Jmbr>
         equivalence &with(size_t index_1d = 0, size_t str_offset = 0) {
            array_alignment.add_anchor(
               align_i_mbr, align_byte_offset, Jmbr - 1, members[ Jmbr - 1 ].type_size * index_1d + str_offset);
            return *this;
         }

         template<size_t Jmbr>
         equivalence &with(arr_index const &arr_ix) {
            with<Jmbr>(members[ Jmbr - 1 ].actual_index_1d(arr_ix));
            return *this;
         }

         template<size_t Jmbr>
         equivalence &with(arr_and_str_indices<arr_dim_max> const &indices) {
            with<Jmbr>(members[ Jmbr - 1 ].actual_index_1d(indices.arr_ix), static_cast<size_t>(indices.str_ix.first - 1));
            return *this;
         }

         template<size_t Jmbr>
         equivalence &with(str_index const &ix) {
            with<Jmbr>(0, static_cast<size_t>(ix.first - 1));
            return *this;
         }
   };

   struct variant_core : utils::noncopyable {
         size_t use_count;
         size_t size;
         char *ptr;

         variant_core() : use_count(0), size(0), ptr(0) {}

         ~variant_core() { delete[] ptr; }

         void grow_if_necessary(size_t new_size) {
            if (size < new_size) {
               TBXX_ASSERT(use_count == 1);
               char *new_ptr = new char[ new_size ];
               std::memcpy(new_ptr, ptr, size);
               delete[] ptr;
               std::memset(new_ptr + size, 0, new_size - size);
               size = new_size;
               ptr = new_ptr;
            }
         }
   };

   struct variant_bind_info {
         size_t offset;
         size_t type_size;
   };

   typedef std::vector<variant_bind_info> variant_bindings;

   struct variant_core_and_bindings {
         variant_core core;
         variant_bindings bindings;
   };

   struct variant_allocate_chain {
         variant_core *core;
         variant_bindings *bindings;
         bool is_common_variant;
         size_t curr_bytes;
         size_t end_bytes;

         variant_allocate_chain(variant_core *core_, variant_bindings *bindings_, bool is_common_variant_) :
             core(core_), bindings(bindings_), is_common_variant(is_common_variant_), curr_bytes(0), end_bytes(0) {}

         ~variant_allocate_chain() { core->grow_if_necessary(end_bytes); }

         variant_allocate_chain &operator,(variant_member const &m) {
            TBXX_ASSERT(is_common_variant);
            bindings->resize(bindings->size() + 1);
            variant_bind_info &bi = bindings->back();
            bi.offset = curr_bytes;
            bi.type_size = m.type_size;
            size_t nb = m.number_of_bytes();
            curr_bytes += nb;
            end_bytes = std::max(end_bytes, curr_bytes);
            return *this;
         }

         variant_allocate_chain &operator,(equivalence &e) {
            e.array_alignment.infer_diffs0_from_diff_matrix();
            size_t prev_bindings_size = bindings->size();
            bindings->reserve(prev_bindings_size + e.members_size);
            size_t origin_bytes = 0;
            if (!is_common_variant) {
               for (size_t i_mbr = 0; i_mbr < e.members_size; i_mbr++) {
                  ssize_t diff0 = e.array_alignment.diffs0[ i_mbr ];
                  if (diff0 < 0) {
                     origin_bytes = std::max(origin_bytes, static_cast<size_t>(-diff0));
                  }
               }
            }
            for (size_t i_mbr = 0; i_mbr < e.members_size; i_mbr++) {
               variant_member const &m = e.members[ i_mbr ];
               bindings->resize(bindings->size() + 1);
               variant_bind_info &bi = bindings->back();
               ssize_t diff0 = e.array_alignment.diffs0[ i_mbr ];
               if (is_common_variant && diff0 < 0 && static_cast<size_t>(-diff0) > curr_bytes) {
                  throw std::runtime_error("EQUIVALENCE crosses beginning of COMMON block");
               }
               bi.offset = static_cast<size_t>(static_cast<ssize_t>(curr_bytes + origin_bytes) + diff0);
               bi.type_size = m.type_size;
               end_bytes = std::max(end_bytes, bi.offset + m.number_of_bytes());
            }
            if (is_common_variant) {
               curr_bytes += e.members[ 0 ].number_of_bytes();
            }
            else {
               curr_bytes = end_bytes;
            }
            return *this;
         }
   };

   struct variant_bind_chain {
         variant_core *core;
         variant_bindings *bindings;
         bool is_common_variant;
         size_t bind_index;

         variant_bind_chain(variant_core *core_, variant_bindings *bindings_, bool is_common_variant_) :
             core(core_), bindings(bindings_), is_common_variant(is_common_variant_), bind_index(0) {
            core->use_count++;
         }

         ~variant_bind_chain() { core->use_count--; }

         variant_allocate_chain allocate() { return variant_allocate_chain(core, bindings, is_common_variant); }

         template<typename T>
         T &bind() {
            variant_bind_info &bi = (*bindings)[ bind_index++ ];
            return *reinterpret_cast<T *>(core->ptr + bi.offset);
         }

         str_ref bind_str() {
            variant_bind_info &bi = (*bindings)[ bind_index++ ];
            return str_ref(core->ptr + bi.offset, bi.type_size);
         }
   };

   struct common_variant : variant_bind_chain {
         common_variant(variant_core &core, variant_bindings &bindings) :
             variant_bind_chain(&core, &bindings, /*is_common_variant*/ true) {}
   };

   struct save_equivalences : variant_bind_chain {
         save_equivalences(variant_core_and_bindings &core_and_bindings) :
             variant_bind_chain(
                &core_and_bindings.core, &core_and_bindings.bindings,
                /*is_common_variant*/ false) {}
   };

   struct local_equivalences : utils::hide<variant_core_and_bindings>, variant_bind_chain {
         local_equivalences() : variant_bind_chain(&hidden.core, &hidden.bindings, /*is_common_variant*/ false) {}
   };

} // namespace fem

#endif // GUARD
// --  end: fem/variant.hpp ----------------------
// -- start: fem/write.hpp ----------------------
#ifndef FEM_WRITE_HPP
#define FEM_WRITE_HPP

// -- repeat not included:  fem/common.hpp ----------------------
// -- repeat not included:  fem/format.hpp ----------------------
// -- repeat not included:  fem/star.hpp ----------------------
// -- repeat not included:  fem/str_arr_ref.hpp ----------------------
// -- start: fem/utils/double_to_string.hpp ----------------------
#ifndef FEM_UTILS_DOUBLE_TO_STRING_HPP
#define FEM_UTILS_DOUBLE_TO_STRING_HPP

// -- repeat not included:  fem/utils/char.hpp ----------------------
// -- repeat not included:  fem/utils/int_types.hpp ----------------------

namespace fem {
   namespace utils {

      struct double_to_string_scientific_notation {
            static const int buffer_capacity = 256;
            char buffer[ buffer_capacity ];

            double_to_string_scientific_notation() {}

            double_to_string_scientific_notation(
               double value,
               int w, // "w" part of Fortran "w.d" format
               int d, // "d" part of Fortran "w.d" format
               int exp_scale, // Fortran "P" scaling
               char e_or_d) {
               TBXX_ASSERT(w > 0);
               TBXX_ASSERT(w < buffer_capacity);
               TBXX_ASSERT(d >= 0);
               int nd = d;
               int na = d;
               if (exp_scale == 0) {
                  // pass
               }
               else if (exp_scale > 0) {
                  nd++;
                  na -= exp_scale - 1;
               }
               else if (exp_scale < 0) {
                  nd += exp_scale;
               }
               if (value == 0) {
                  if (w < 1 + d + 4 || d == 0 || nd <= 0) {
                     std::memset(buffer, '*', w);
                     return;
                  }
                  int i_buf = w;
                  buffer[ --i_buf ] = '0';
                  buffer[ --i_buf ] = '0';
                  buffer[ --i_buf ] = '+';
                  buffer[ --i_buf ] = e_or_d;
                  for (int i = 0; i < d; i++) {
                     buffer[ --i_buf ] = '0';
                  }
                  buffer[ --i_buf ] = '.';
                  if (i_buf != 0) {
                     buffer[ --i_buf ] = '0';
                     while (i_buf != 0) {
                        buffer[ --i_buf ] = ' ';
                     }
                  }
                  return;
               }
               //
               struct pow_tab_entry {
                     int i;
                     double g;
                     double l;
               };
               static const pow_tab_entry pow_tab[] = { { 256, 1e256, 1e-256 }, { 128, 1e128, 1e-128 },
                                                        { 64, 1e064, 1e-064 },  { 32, 1e032, 1e-032 },
                                                        { 16, 1e016, 1e-016 },  { 8, 1e008, 1e-008 },
                                                        { 4, 1e004, 1e-004 },   { 2, 1e002, 1e-002 },
                                                        { 1, 1e001, 1e-001 } };
               bool v_negative = (value < 0);
               if (v_negative)
                  value = -value;
               int required_w = (v_negative ? 2 : 1) + nd + 4;
               if (w < required_w || d == 0 || nd <= 0 || na < 0) {
                  std::memset(buffer, '*', w);
                  return;
               }
               int iexp = 0;
               if (value > 1) {
                  for (int i_t = 0; i_t < 9; i_t++) {
                     pow_tab_entry const &t = pow_tab[ i_t ];
                     if (value > t.g) {
                        iexp += t.i;
                        value *= t.l;
                     }
                  }
                  iexp++;
                  value *= 0.1;
               }
               else if (value < 1) {
                  for (int i_t = 0; i_t < 9; i_t++) {
                     pow_tab_entry const &t = pow_tab[ i_t ];
                     if (value < t.l) {
                        iexp -= t.i;
                        value *= t.g;
                     }
                  }
               }
               static const int max_nd_significant = 16;
               static const double multipliers_dbl[] = { 1e0, 1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7, 1e8,
                                                         1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16 };
               typedef utils::int64_t i64;
               TBXX_ASSERT(sizeof(i64) >= sizeof(double));
               static i64 multipliers_i64[ max_nd_significant + 1 ] = { 0 };
               if (multipliers_i64[ 0 ] == 0) {
                  multipliers_i64[ 0 ] = 1;
                  for (int i = 1; i <= max_nd_significant; i++) {
                     multipliers_i64[ i ] = multipliers_i64[ i - 1 ] * 10;
                  }
               }
               int nd_significant = std::min(nd, max_nd_significant);
               int nd_unknown = nd - nd_significant;
               double multiplier = multipliers_dbl[ nd_significant ];
               i64 ival = static_cast<i64>(value * multiplier + 0.5);
               if (ival == multipliers_i64[ nd_significant ]) {
                  ival = multipliers_i64[ nd_significant - 1 ];
                  iexp++;
               }
               iexp -= exp_scale;
               //
               int i_buf = w;
               bool e_negative = (iexp < 0);
               if (e_negative)
                  iexp = -iexp;
               while (iexp != 0) {
                  buffer[ --i_buf ] = int_as_digit(iexp % 10);
                  iexp /= 10;
               }
               while (i_buf > w - 2) {
                  buffer[ --i_buf ] = '0';
               }
               buffer[ --i_buf ] = (e_negative ? '-' : '+');
               if (i_buf != w - 4)
                  buffer[ --i_buf ] = e_or_d;
               //
               for (int i = 0; i < na; i++) {
                  if (nd_unknown != 0) {
                     buffer[ --i_buf ] = '0';
                     nd_unknown--;
                  }
                  else {
                     buffer[ --i_buf ] = int_as_digit(ival % 10);
                     ival /= 10;
                  }
               }
               buffer[ --i_buf ] = '.';
               if (ival == 0) {
                  if (i_buf != 0) {
                     buffer[ --i_buf ] = '0';
                  }
               }
               else {
                  do {
                     if (nd_unknown != 0) {
                        buffer[ --i_buf ] = '0';
                        nd_unknown--;
                     }
                     else {
                        buffer[ --i_buf ] = int_as_digit(ival % 10);
                        ival /= 10;
                     }
                  } while (ival != 0);
               }
               if (i_buf == 0) {
                  if (v_negative)
                     buffer[ 0 ] = '-';
               }
               else {
                  buffer[ --i_buf ] = (v_negative ? '-' : ' ');
                  while (i_buf != 0) {
                     buffer[ --i_buf ] = ' ';
                  }
               }
            }
      };

   } // namespace utils
} // namespace fem

#endif // GUARD
// --  end: fem/utils/double_to_string.hpp ----------------------
// -- repeat not included:  fem/utils/misc.hpp ----------------------
// -- start: fem/utils/real_as_string.hpp ----------------------
#ifndef FEM_UTILS_REAL_AS_STRING_HPP
#define FEM_UTILS_REAL_AS_STRING_HPP

#include <cstdio>

namespace fem {
   namespace utils {

      struct float_as_string_list_directed {
            char begin[ 64 ];
            int n;

            float_as_string_list_directed(double const &val) // intentionally forcing promotion to double
            {
#if defined(_MSC_VER)
               char const *cfmte = "%15.7E";
               char const *cfmt = cfmte;
#else
               char const *cfmt = "%14.7E";
#endif
               float av = (val < 0 ? -val : val);
               if (av < 1e3) {
                  if (av < 1e1) {
                     if (av < 1e-1) /*pass*/
                        ;
                     else if (av < 1e0)
                        cfmt = "%10.7f    ";
                     else
                        cfmt = "%10.6f    ";
                  }
                  else if (av < 1e2)
                     cfmt = "%10.5f    ";
                  else
                     cfmt = "%10.4f    ";
               }
               else if (av < 1e5) {
                  if (av < 1e4)
                     cfmt = "%10.3f    ";
                  else
                     cfmt = "%10.2f    ";
               }
               else if (av < 1e7) {
                  if (av < 1e6)
                     cfmt = "%10.1f    ";
                  else
                     cfmt = "%9.0f.    ";
               }
               n = std::sprintf(begin, cfmt, val);
#if defined(_MSC_VER)
               if (cfmt == cfmte && n == 15 && begin[ 12 ] == '0') {
                  begin[ 12 ] = begin[ 13 ];
                  begin[ 13 ] = begin[ 14 ];
                  begin[ 14 ] = '\0';
                  n = 14;
               }
#endif
            }
      };

      struct double_as_string_list_directed {
            char buffer[ 64 ];
            char *begin;
            int n;

            double_as_string_list_directed(double const &val) : begin(buffer) {
               char const *cfmte = "%23.15E";
               char const *cfmt = cfmte;
               double av = (val < 0 ? -val : val);
               if (av < 1e7) {
                  if (av < 1e3) {
                     if (av < 1e1) {
                        if (av < 1e-1) /*pass*/
                           ;
                        else if (av < 1e0)
                           cfmt = "%18.15f     ";
                        else
                           cfmt = "%18.14f     ";
                     }
                     else if (av < 1e2)
                        cfmt = "%18.13f     ";
                     else
                        cfmt = "%18.12f     ";
                  }
                  else if (av < 1e5) {
                     if (av < 1e4)
                        cfmt = "%18.11f     ";
                     else
                        cfmt = "%18.10f     ";
                  }
                  else if (av < 1e6)
                     cfmt = "%18.9f     ";
                  else
                     cfmt = "%18.8f     ";
               }
               else if (av < 1e11) {
                  if (av < 1e9) {
                     if (av < 1e8)
                        cfmt = "%18.7f     ";
                     else
                        cfmt = "%18.6f     ";
                  }
                  else if (av < 1e10)
                     cfmt = "%18.5f     ";
                  else
                     cfmt = "%18.4f     ";
               }
               else if (av < 1e13) {
                  if (av < 1e12)
                     cfmt = "%18.3f     ";
                  else
                     cfmt = "%18.2f     ";
               }
               else if (av < 1e14)
                  cfmt = "%18.1f     ";
               else if (av < 1e15)
                  cfmt = "%17.0f.     ";
               n = std::sprintf(buffer, cfmt, val);
               if (n == 23 && cfmt == cfmte) {
                  char c = buffer[ 20 ];
                  if ((c == '+' || c == '-') && buffer[ 0 ] == ' ') {
                     begin++;
                     buffer[ 24 ] = '\0';
                     buffer[ 23 ] = buffer[ 22 ];
                     buffer[ 22 ] = buffer[ 21 ];
                     buffer[ 21 ] = '0';
                  }
               }
            }
      };

   } // namespace utils
} // namespace fem

#endif // GUARD
// --  end: fem/utils/real_as_string.hpp ----------------------

#if defined(_MSC_VER)
#define FEM_WRITE_CRLF true
#else
#define FEM_WRITE_CRLF false
#endif

namespace fem {

   struct write_loop_base {
         bool write_crlf;
         unsigned pos;
         bool prev_was_string;
         int exp_scale;
         unsigned number_of_x_held;
         bool suppress_new_line_at_end;

         write_loop_base(bool write_crlf_ = false) :
             write_crlf(write_crlf_), pos(0), prev_was_string(false), exp_scale(0), number_of_x_held(0),
             suppress_new_line_at_end(false) {}
   };

   class write_loop : write_loop_base
   // TODO copy-constructor potential performance problem
   {
      private:
         utils::slick_ptr<utils::simple_ostream> out;
         int internal_file_len;
         io_modes io_mode;
         format::token_loop fmt_loop;
         bool terminated_by_colon;

      public:
         write_loop(common &cmn, int const &unit, unformatted_type const &) :
             write_loop_base(FEM_WRITE_CRLF && !is_std_io_unit(unit)), out(cmn.io.simple_ostream(unit)),
             internal_file_len(-1), io_mode(io_unformatted), terminated_by_colon(false) {}

         write_loop(common &cmn, int const &unit, star_type const &) :
             write_loop_base(FEM_WRITE_CRLF && !is_std_io_unit(unit)), out(cmn.io.simple_ostream(unit)),
             internal_file_len(-1), io_mode(io_list_directed), terminated_by_colon(false) {}

         write_loop(common &cmn, int const &unit, str_cref fmt) :
             write_loop_base(FEM_WRITE_CRLF && !is_std_io_unit(unit)), out(cmn.io.simple_ostream(unit)),
             internal_file_len(-1), io_mode(io_formatted), fmt_loop(fmt), terminated_by_colon(false) {}

         write_loop(str_ref const &internal_file, star_type const &) :
             out(utils::slick_ptr<utils::simple_ostream>(
                new utils::simple_ostream_to_char_ptr_and_size(internal_file.elems(), internal_file.len()))),
             internal_file_len(internal_file.len()), io_mode(io_list_directed), terminated_by_colon(false) {}

         write_loop(str_ref const &internal_file, str_cref fmt) :
             out(utils::slick_ptr<utils::simple_ostream>(
                new utils::simple_ostream_to_char_ptr_and_size(internal_file.elems(), internal_file.len()))),
             internal_file_len(internal_file.len()), io_mode(io_formatted), fmt_loop(fmt), terminated_by_colon(false) {}

         template<size_t Ndims>
         write_loop(str_arr_ref<Ndims> const &internal_file, star_type const &) :
             out(utils::slick_ptr<utils::simple_ostream>(
                new utils::simple_ostream_to_char_ptr_and_size(internal_file.begin(), internal_file.len()))),
             internal_file_len(internal_file.len()), io_mode(io_list_directed), terminated_by_colon(false) {}

         template<size_t Ndims>
         write_loop(str_arr_ref<Ndims> const &internal_file, str_cref fmt) :
             out(utils::slick_ptr<utils::simple_ostream>(
                new utils::simple_ostream_to_char_ptr_and_size(internal_file.begin(), internal_file.len()))),
             internal_file_len(internal_file.len()), io_mode(io_formatted), fmt_loop(fmt), terminated_by_colon(false) {}

         std::string const &next_edit_descriptor(bool final = false) {
            while (true) {
               if (terminated_by_colon) {
                  static const std::string empty("");
                  return empty;
               }
               utils::token const *t = fmt_loop.next_executable_token(final);
               if (t == 0) {
                  static const std::string empty("");
                  return empty;
               }
               std::string const &tv = t->value;
               if (t->type == "string") {
                  to_stream_fmt(tv.data(), tv.size());
               }
               else if (t->type == "op") {
                  if (tv[ 0 ] == ':') {
                     if (final)
                        terminated_by_colon = true;
                  }
                  else if (tv[ 0 ] == '/') {
                     if (write_crlf) {
                        to_stream_fmt("\r\n", 2);
                     }
                     else {
                        to_stream_fmt("\n", 1);
                     }
                  }
                  else if (tv[ 0 ] == '$') {
                     suppress_new_line_at_end = true;
                  }
                  else {
                     out.reset();
                     throw TBXX_UNREACHABLE_ERROR();
                  }
               }
               else if (t->type == "format") {
                  if (utils::ends_with_char(tv, 'x')) {
                     unsigned n = tv.size();
                     if (n != 1)
                        n = utils::unsigned_integer_value(tv.data(), n - 1);
                     number_of_x_held += n;
                  }
                  else if (std::strchr("adefgilz", tv[ 0 ]) != 0) {
                     return tv;
                  }
                  else if (utils::ends_with_char(tv, 'p')) {
                     if (tv.size() == 1) {
                        exp_scale = 1;
                     }
                     else {
                        exp_scale = utils::signed_integer_value(tv.data(), 0, tv.size() - 1);
                     }
                  }
                  else if (tv[ 0 ] == 't') {
                     out.reset();
                     throw TBXX_NOT_IMPLEMENTED();
                  }
                  else if (tv[ 0 ] == 's') {
                     out.reset();
                     throw TBXX_NOT_IMPLEMENTED();
                  }
                  else if (tv[ 0 ] == 'b') {
                     out.reset();
                     throw TBXX_NOT_IMPLEMENTED();
                  }
                  else {
                     out.reset();
                     throw TBXX_UNREACHABLE_ERROR();
                  }
               }
               else {
                  out.reset();
                  throw TBXX_UNREACHABLE_ERROR();
               }
            }
         }

         write_loop &operator,(char const &val) {
            if (io_mode == io_unformatted) {
               to_stream_unformatted(&val, 1);
            }
            else if (io_mode == io_list_directed) {
               to_stream(&val, 1, /*space*/ !prev_was_string);
               prev_was_string = true;
            }
            else {
               out.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

         write_loop &operator,(char const *val) {
            if (io_mode == io_unformatted) {
               to_stream_unformatted(val, std::strlen(val));
            }
            else if (io_mode == io_list_directed) {
               to_stream_star(val, std::strlen(val), /*space*/ !prev_was_string);
               prev_was_string = true;
            }
            else {
               std::string const &ed = next_edit_descriptor();
               int l = std::strlen(val);
               if (ed[ 0 ] == 'a') {
                  int n = ed.size();
                  if (n > 1) {
                     n = utils::unsigned_integer_value(ed.data() + 1, n - 1);
                     int b = n - l;
                     if (b < 0) {
                        l = n;
                     }
                     else {
                        for (int i = 0; i < b; i++)
                           to_stream_fmt(" ", 1);
                     }
                  }
                  to_stream_fmt(val, l);
               }
               else {
                  to_stream_fmt(val, l);
               }
            }
            return *this;
         }

         write_loop &operator,(bool const &val) {
            if (io_mode == io_unformatted) {
               to_stream_unformatted(reinterpret_cast<char const *>(&val), sizeof(bool));
            }
            else if (io_mode == io_list_directed) {
               to_stream_star((val ? "T" : "F"), 1);
               prev_was_string = false;
            }
            else {
               std::string const &ed = next_edit_descriptor();
               int n = ed.size();
               if (ed[ 0 ] == 'l' && n > 1) {
                  n = utils::unsigned_integer_value(ed.data() + 1, n - 1);
               }
               else {
                  n = 1;
               }
               for (int i = 1; i < n; i++)
                  to_stream_fmt(" ", 1);
               to_stream_fmt((val ? "T" : "F"), 1);
            }
            return *this;
         }

         write_loop &operator,(integer_star_1 const &val) {
            if (io_mode == io_unformatted) {
               to_stream_unformatted(reinterpret_cast<char const *>(&val), sizeof(integer_star_1));
            }
            else if (io_mode == io_list_directed) {
               char buf[ 64 ];
               int n = std::sprintf(buf, "%4d", static_cast<int>(val));
               to_stream_star(buf, n);
               prev_was_string = false;
            }
            else {
               out.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

         write_loop &operator,(integer_star_2 const &val) {
            if (io_mode == io_unformatted) {
               to_stream_unformatted(reinterpret_cast<char const *>(&val), sizeof(integer_star_2));
            }
            else if (io_mode == io_list_directed) {
               char buf[ 64 ];
               int n = std::sprintf(buf, "%6d", static_cast<int>(val));
               to_stream_star(buf, n);
               prev_was_string = false;
            }
            else {
               out.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

         write_loop &operator,(integer_star_4 const &val) {
            if (io_mode == io_unformatted) {
               to_stream_unformatted(reinterpret_cast<char const *>(&val), sizeof(integer_star_4));
            }
            else if (io_mode == io_list_directed) {
               char buf[ 64 ];
               int n = std::sprintf(buf, "%11d", val);
               to_stream_star(buf, n);
               prev_was_string = false;
            }
            else {
               std::string const &ed = next_edit_descriptor();
               if (ed[ 0 ] == 'i') {
                  int n = ed.size();
                  TBXX_ASSERT(n + 2 < 64);
                  char fmt[ 64 ];
                  fmt[ 0 ] = '%';
                  std::strncpy(fmt + 1, ed.data() + 1, n - 1);
                  fmt[ n ] = 'd';
                  fmt[ n + 1 ] = '\0';
                  char buf[ 64 ];
                  n = std::sprintf(buf, fmt, val);
                  to_stream_fmt(buf, n);
               }
               else {
                  char buf[ 64 ];
                  int n = std::sprintf(buf, " %d", val);
                  to_stream_fmt(buf, n);
               }
            }
            return *this;
         }

         write_loop &operator,(integer_star_8 const &val) {
            if (io_mode == io_unformatted) {
               to_stream_unformatted(reinterpret_cast<char const *>(&val), sizeof(integer_star_8));
            }
            else if (io_mode == io_list_directed) {
               // TODO faster implementation
               std::ostringstream o;
               o.width(21);
               o << val;
               std::string s = o.str();
               to_stream_star(s.data(), s.size());
               prev_was_string = false;
            }
            else {
               out.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

      protected: // implementation detail
         void to_stream_fmt_double(double const &val) {
            std::string const &ed = next_edit_descriptor();
            if (ed[ 0 ] == 'f') {
               int n = ed.size();
               TBXX_ASSERT(n + 2 < 64);
               char fmt[ 64 ];
               fmt[ 0 ] = '%';
               std::strncpy(fmt + 1, ed.data() + 1, n - 1);
               fmt[ n ] = 'f';
               fmt[ n + 1 ] = '\0';
               char buf[ 64 ];
               n = std::sprintf(buf, fmt, val);
               to_stream_fmt(buf, n);
            }
            else if ((ed[ 0 ] == 'd' || ed[ 0 ] == 'e') && ed.size() > 1) {
               int es = ed.size();
               int nw = utils::unsigned_integer_scan(ed.data(), 1, es);
               TBXX_ASSERT(nw > 0);
               int w = utils::unsigned_integer_value(ed.data(), 1, nw);
               int d = 0;
               if (nw != es) {
                  TBXX_ASSERT(ed[ nw ] == '.');
                  TBXX_ASSERT(nw + 1 != es);
                  d = utils::unsigned_integer_value(ed.data(), nw + 1, es);
               }
               utils::double_to_string_scientific_notation conv(val, w, d, exp_scale, utils::to_upper(ed[ 0 ]));
               to_stream_fmt(conv.buffer, w);
            }
            else {
               char buf[ 64 ];
               int n = std::sprintf(buf, " %.6g", val);
               to_stream_fmt(buf, n);
            }
         }

      public:
         write_loop &operator,(float const &val) {
            if (io_mode == io_unformatted) {
               to_stream_unformatted(reinterpret_cast<char const *>(&val), sizeof(float));
            }
            else if (io_mode == io_list_directed) {
               utils::float_as_string_list_directed conv(val);
               to_stream(conv.begin, conv.n);
               prev_was_string = false;
            }
            else {
               to_stream_fmt_double(static_cast<double>(val));
            }
            return *this;
         }

         write_loop &operator,(double const &val) {
            if (io_mode == io_unformatted) {
               to_stream_unformatted(reinterpret_cast<char const *>(&val), sizeof(double));
            }
            else if (io_mode == io_list_directed) {
               utils::double_as_string_list_directed conv(val);
               to_stream(conv.begin, conv.n);
               prev_was_string = false;
            }
            else {
               to_stream_fmt_double(val);
            }
            return *this;
         }

         write_loop &operator,(long double const &val) {
            if (io_mode == io_unformatted) {
               to_stream_unformatted(reinterpret_cast<char const *>(&val), actual_sizeof_long_double);
            }
            else if (io_mode == io_list_directed) {
               out.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            else {
               out.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

         write_loop &operator,(std::complex<float> const &val) {
            if (io_mode == io_unformatted) {
               float re = val.real();
               float im = val.imag();
               to_stream_unformatted(reinterpret_cast<char const *>(&re), sizeof(float));
               to_stream_unformatted(reinterpret_cast<char const *>(&im), sizeof(float));
            }
            else if (io_mode == io_list_directed) {
               utils::float_as_string_list_directed conv_re(val.real());
               utils::float_as_string_list_directed conv_im(val.imag());
               to_stream_star_complex(conv_re.begin, conv_re.n, conv_im.begin, conv_im.n);
            }
            else {
               out.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

         write_loop &operator,(std::complex<double> const &val) {
            if (io_mode == io_unformatted) {
               double re = val.real();
               double im = val.imag();
               to_stream_unformatted(reinterpret_cast<char const *>(&re), sizeof(double));
               to_stream_unformatted(reinterpret_cast<char const *>(&im), sizeof(double));
            }
            else if (io_mode == io_list_directed) {
               utils::double_as_string_list_directed conv_re(val.real());
               utils::double_as_string_list_directed conv_im(val.imag());
               to_stream_star_complex(conv_re.begin, conv_re.n, conv_im.begin, conv_im.n);
            }
            else {
               out.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

         write_loop &operator,(std::complex<long double> const &val) {
            if (io_mode == io_unformatted) {
               long double re = val.real();
               long double im = val.imag();
               to_stream_unformatted(reinterpret_cast<char const *>(&re), actual_sizeof_long_double);
               to_stream_unformatted(reinterpret_cast<char const *>(&im), actual_sizeof_long_double);
            }
            else if (io_mode == io_list_directed) {
               out.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            else {
               out.reset();
               throw TBXX_NOT_IMPLEMENTED();
            }
            return *this;
         }

         write_loop &operator,(str_cref const &val) {
            if (io_mode == io_unformatted) {
               to_stream_unformatted(val.elems(), val.len());
            }
            else if (io_mode == io_list_directed) {
               to_stream(val.elems(), val.len(), /*space*/ !prev_was_string);
               prev_was_string = true;
            }
            else {
               std::string const &ed = next_edit_descriptor();
               int n = ed.size();
               if (ed[ 0 ] == 'a' && n > 1) {
                  n = utils::unsigned_integer_value(ed.data() + 1, n - 1);
                  to_stream(val.elems(), std::min(val.len(), n));
                  for (int i = val.len(); i < n; i++) {
                     to_stream(" ", 1);
                  }
               }
               else {
                  to_stream(val.elems(), val.len());
               }
            }
            return *this;
         }

         write_loop &operator,(str_addends const &val) {
            int ll = val.lhs.len();
            int rl = val.rhs.len();
            utils::simple_buffer<char> buffer(ll + rl);
            char *b = buffer.space;
            std::memcpy(b, val.lhs.elems(), ll);
            std::memcpy(b + ll, val.rhs.elems(), rl);
            (*this), str_cref(b, ll + rl);
            return *this;
         }

         template<typename T, size_t Ndims>
         write_loop &operator,(arr_cref<T, Ndims> const &val) {
            size_t n = val.size_1d();
            T const *val_begin = val.begin();
            for (size_t i = 0; i < n; i++) {
               (*this), val_begin[ i ];
            }
            return *this;
         }

         template<size_t Ndims>
         write_loop &operator,(str_arr_cref<Ndims> const &val) {
            size_t n = val.size_1d();
            int l = val.len();
            char const *val_begin = val.begin();
            for (size_t i = 0; i < n; i++) {
               (*this), str_cref(&val_begin[ i * l ], l);
            }
            return *this;
         }

         ~write_loop() {
            if (out.get() == 0)
               return;
            if (internal_file_len < 0) {
               if (io_mode == io_unformatted) {
                  out->put(end_of_unformatted_record);
                  out->put('\0');
               }
               else {
                  if (io_mode == io_list_directed) {
                     if (pos == 0)
                        out->put(' ');
                  }
                  else {
                     next_edit_descriptor(/*final*/ true);
                  }
                  if (!suppress_new_line_at_end) {
                     if (write_crlf) {
                        out->put("\r\n", 2);
                     }
                     else {
                        out->put('\n');
                     }
                  }
               }
               out->flush();
            }
            else {
               if (io_mode == io_unformatted) {
                  out.reset();
                  throw TBXX_NOT_IMPLEMENTED();
               }
               else {
                  if (io_mode == io_list_directed) {
                     if (pos == 0) {
                        out->put(' ');
                        pos++;
                     }
                  }
                  else {
                     next_edit_descriptor(/*final*/ true);
                  }
                  while (( int ) pos < internal_file_len) {
                     out->put(' ');
                     pos++;
                  }
               }
            }
         }

      private:
         void to_stream(char const *buf, unsigned n, bool space = true) {
            switch (io_mode) {
               case io_unformatted:
                  to_stream_unformatted(buf, n);
                  break;
               case io_list_directed:
                  to_stream_star(buf, n, space);
                  break;
               default:
                  to_stream_fmt(buf, n);
            }
         }

         void to_stream_unformatted(char const *buf, unsigned n) {
            for (unsigned i = 0; i < n; i++) {
               char c = buf[ i ];
               out->put(c);
               if (c == end_of_unformatted_record) {
                  out->put(c);
               }
            }
         }

         void to_stream_fmt(char const *buf, unsigned n) {
            while (number_of_x_held != 0) {
               out->put(" ", 1);
               number_of_x_held--;
            }
            out->put(buf, n);
         }

         void to_stream_star(char const *buf, unsigned n, bool space = true) {
            if (pos == 0) {
               out->put(' ');
               pos = 1;
            }
            else if (pos + (space ? 1 : 0) + n > 80) {
               if (write_crlf) {
                  out->put("\r\n ", 3);
               }
               else {
                  out->put("\n ", 2);
               }
               pos = 1;
            }
            else if (space) {
               out->put(' ');
               pos++;
            }
            out->put(buf, n);
            pos += n;
         }

         void to_stream_star_strip_leading_and_trailing_blank_padding(
            char const *buf, unsigned n, bool /* space=true */) {
            size_t_2 indices = utils::find_leading_and_trailing_blank_padding(buf, n);
            to_stream_star(
               buf + indices.elems[ 0 ], indices.elems[ 1 ] - indices.elems[ 0 ],
               /*space*/ false);
         }

         void to_stream_star_complex(char const *buf_re, unsigned n_re, char const *buf_im, unsigned n_im) {
            to_stream_star("(", 1);
            {
               to_stream_star_strip_leading_and_trailing_blank_padding(buf_re, n_re, /*space*/ false);
            }
            to_stream_star(",", 1, /*space*/ false);
            {
               to_stream_star_strip_leading_and_trailing_blank_padding(buf_im, n_im, /*space*/ false);
            }
            to_stream_star(")", 1, /*space*/ false);
            prev_was_string = false;
         }
   };

   struct common_write {
         common &cmn;

         common_write(common &cmn_) : cmn(cmn_) {}

         write_loop operator()(int unit, unformatted_type const &) {
            write_loop result(cmn, unit, unformatted);
            return result;
         }

         write_loop operator()(int unit, star_type const &) {
            write_loop result(cmn, unit, star);
            return result;
         }

         write_loop operator()(int const &unit, str_cref fmt) {
            write_loop result(cmn, unit, fmt);
            return result;
         }

         write_loop operator()(str_ref const &internal_file, star_type const &) {
            write_loop result(internal_file, star);
            return result;
         }

         write_loop operator()(str_ref const &internal_file, str_cref fmt) {
            write_loop result(internal_file, fmt);
            return result;
         }

         template<size_t Ndims>
         write_loop operator()(str_arr_ref<Ndims> const &internal_file, star_type const &) {
            write_loop result(internal_file, star);
            return result;
         }

         template<size_t Ndims>
         write_loop operator()(str_arr_ref<Ndims> const &internal_file, str_cref fmt) {
            write_loop result(internal_file, fmt);
            return result;
         }
   };

} // namespace fem

#endif // GUARD
// --  end: fem/write.hpp ----------------------

namespace fem {
   namespace major_types {

      using fem::arr;
      using fem::arr_1d;
      using fem::arr_2d;
      using fem::arr_3d;
      using fem::arr_cref;
      using fem::arr_index;
      using fem::arr_ref;
      using fem::arr_size;
      using fem::common_read;
      using fem::common_variant;
      using fem::common_write;
      using fem::datum;
      using fem::dim1;
      using fem::dimension;
      using fem::equivalence;
      using fem::local_equivalences;
      using fem::read_loop;
      using fem::save_equivalences;
      using fem::star;
      using fem::str_arr_cref;
      using fem::str_arr_ref;
      using fem::str_cref;
      using fem::str_index;
      using fem::str_ref;
      using fem::values;
      using fem::write_loop;

   } // namespace major_types
} // namespace fem

#endif // GUARD
// --  end: fem/major_types.hpp ----------------------

#endif // GUARD
// --  end: fem.hpp ----------------------
