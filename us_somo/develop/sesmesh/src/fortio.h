// sesmesh rcoal: Fortran-compatible input and output.
//
// The Fortran rcoal is a gfortran program built with -fdefault-real-8, and it writes its
// outputs with list-directed WRITE and the E17.8 edit descriptor.  The functions here
// reproduce gfortran's output byte for byte, so that this port's outputs can be compared with
// the Fortran's with cmp.  They read input the way list-directed READ does for the plain
// numeric records of .c3p, .c3v and MSMS files.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace sesmesh::rcoal {

// Ew.d: 0.dddd...E+xx, right-justified in w columns (default scale factor 0).
inline std::string fort_e( double v, int w, int d ) {
   char buf[64];
   std::snprintf( buf, sizeof buf, "%.*e", d - 1, v );
   const char* p = buf;
   bool neg = *p == '-';
   if ( neg ) {
      ++p;
   }
   std::string digits;
   digits += *p++;
   if ( *p == '.' ) {
      ++p;
   }
   while ( *p && *p != 'e' ) {
      digits += *p++;
   }
   int ex = std::atoi( p + 1 ) + 1;
   if ( v == 0 ) {
      ex = 0;
   }
   std::string s = neg ? "-0." : "0.";
   s += digits;
   char eb[16];
   int ae = std::abs( ex );
   if ( ae <= 99 ) {
      std::snprintf( eb, sizeof eb, "E%c%02d", ex < 0 ? '-' : '+', ae );
   } else {
      std::snprintf( eb, sizeof eb, "%c%03d", ex < 0 ? '-' : '+', ae );
   }
   s += eb;
   if ( int( s.size() ) < w ) {
      s.insert( 0, w - s.size(), ' ' );
   } else if ( int( s.size() ) > w ) {
      s.assign( w, '*' );
   }
   return s;
}

// gfortran's list-directed form of a REAL(8): G25.17E3 with scale factor 1, i.e.
// F20.(17-k) followed by 5 blanks when 0.1 <= |v| < 1e17, else 1PE25.17E3.
inline std::string fort_list_real( double v ) {
   const int w = 25, d = 17;
   bool neg = std::signbit( v );
   double m = std::fabs( v );
   std::string s;
   if ( std::isnan( v ) ) {
      s = "NaN";
      s.insert( 0, w - s.size(), ' ' );
      return s;
   }
   if ( std::isinf( v ) ) {
      s = neg ? "-Infinity" : "Infinity";
      s.insert( 0, w - s.size(), ' ' );
      return s;
   }
   if ( m == 0 ) {
      s = neg ? "-0." : "0.";
      s += std::string( d - 1, '0' );
      s.insert( 0, 20 - s.size(), ' ' );
      return s + "     ";
   }
   char buf[64];
   std::snprintf( buf, sizeof buf, "%.*e", d - 1, m );
   std::string digits;
   const char* p = buf;
   digits += *p++;
   ++p;   // '.'
   while ( *p && *p != 'e' ) {
      digits += *p++;
   }
   int x = std::atoi( p + 1 );
   int k = x + 1;   // digits before the point
   if ( k >= 0 && k <= d ) {
      if ( k == 0 ) {
         s = "0." + digits;
      } else {
         s = digits.substr( 0, k ) + "." + digits.substr( k );
      }
      if ( neg ) {
         s = "-" + s;
      }
      if ( s.size() < 20 ) {
         s.insert( 0, 20 - s.size(), ' ' );
      }
      return s + "     ";
   }
   s = digits.substr( 0, 1 ) + "." + digits.substr( 1 );
   char eb[16];
   std::snprintf( eb, sizeof eb, "E%c%03d", x < 0 ? '-' : '+', std::abs( x ) );
   s += eb;
   if ( neg ) {
      s = "-" + s;
   }
   if ( int( s.size() ) < w ) {
      s.insert( 0, w - s.size(), ' ' );
   }
   return s;
}

// A list-directed output record, as gfortran writes it: a blank at the start, a blank
// between items except between two character items, INTEGER as I11, REAL(8) as above,
// CHARACTER as is (a CHARACTER*80 variable with its trailing blanks).
class ListRecord {
 public:
   ListRecord& c( const std::string& s ) {
      sep( true );
      out_ += s;
      return *this;
   }
   // A CHARACTER*80 variable.
   ListRecord& c80( const std::string& s ) {
      std::string t = s.substr( 0, 80 );
      t.resize( 80, ' ' );
      return c( t );
   }
   ListRecord& i( long v ) {
      sep( false );
      char buf[32];
      std::snprintf( buf, sizeof buf, "%11ld", v );
      out_ += buf;
      return *this;
   }
   ListRecord& r( double v ) {
      sep( false );
      out_ += fort_list_real( v );
      return *this;
   }
   const std::string& str() const { return out_; }

 private:
   void sep( bool is_char ) {
      if ( first_ ) {
         out_ += ' ';
         first_ = false;
      } else if ( !( is_char && last_char_ ) ) {
         out_ += ' ';
      }
      last_char_ = is_char;
   }
   std::string out_;
   bool first_ = true, last_char_ = false;
};

// Reads a file the way list-directed READ does for numeric records: each read starts a new
// line, takes as many blank- or comma-separated values as it needs from that line and the
// following ones, and ignores the rest of the last line it used.
class ListReader {
 public:
   explicit ListReader( const std::string& path ) : path_( path ) {
      FILE* f = std::fopen( path.c_str(), "rb" );
      if ( !f ) {
         throw std::runtime_error( "cannot open " + path );
      }
      std::fseek( f, 0, SEEK_END );
      long n = std::ftell( f );
      std::fseek( f, 0, SEEK_SET );
      buf_.resize( n > 0 ? n : 0 );
      if ( n > 0 && std::fread( &buf_[0], 1, n, f ) != size_t( n ) ) {
         std::fclose( f );
         throw std::runtime_error( "cannot read " + path );
      }
      std::fclose( f );
      buf_.push_back( '\0' );
   }
   bool eof() const { return pos_ + 1 >= buf_.size(); }
   // Skips the rest of the current line (a READ with an empty list).
   void skip_line() {
      while ( buf_[pos_] && buf_[pos_] != '\n' ) {
         ++pos_;
      }
      if ( buf_[pos_] == '\n' ) {
         ++pos_;
      }
   }
   // Reads n values into v, starting at the current line.
   void read( std::vector<double>& v, size_t n ) {
      v.clear();
      while ( v.size() < n ) {
         // skip separators within the line
         while ( buf_[pos_] == ' ' || buf_[pos_] == '\t' || buf_[pos_] == ',' || buf_[pos_] == '\r' ) {
            ++pos_;
         }
         if ( buf_[pos_] == '\n' ) {
            ++pos_;
            continue;
         }
         if ( !buf_[pos_] ) {
            throw std::runtime_error( "unexpected end of " + path_ );
         }
         char* end;
         double x = std::strtod( &buf_[pos_], &end );
         if ( end == &buf_[pos_] ) {
            throw std::runtime_error( "bad number in " + path_ + " near: " +
                                      std::string( &buf_[pos_], std::min<size_t>( 20, buf_.size() - 1 - pos_ ) ) );
         }
         pos_ = end - &buf_[0];
         v.push_back( x );
      }
      skip_line();
   }
   // The next line as text (for header and comment lines).
   std::string line() {
      size_t s = pos_;
      skip_line();
      std::string l( &buf_[s], pos_ - s );
      while ( !l.empty() && ( l.back() == '\n' || l.back() == '\r' ) ) {
         l.pop_back();
      }
      return l;
   }

 private:
   std::string path_;
   std::vector<char> buf_;
   size_t pos_ = 0;
};

}  // namespace sesmesh::rcoal
