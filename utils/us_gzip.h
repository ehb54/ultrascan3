//! \file us_gzip.h
#ifndef US_GZIP_H
#define US_GZIP_H

#include <qstring.h>
#include <sys/types.h>
#include "us_extern.h"

// Error codes

#define   GZIP_OK                 0
#define   GZIP_NOEXIST            1
#define   GZIP_NOTFILE            2
#define   GZIP_NOREAD             3
#define   GZIP_NOTGZ              4
#define   GZIP_OPTIONNOTSUPPORTED 5
#define   GZIP_OUTFILEEXISTS      6
#define   GZIP_CRCERROR           7
#define   GZIP_READERROR          8
#define   GZIP_WRITEERROR         9
#define   GZIP_LENGTHERROR       10
#define   GZIP_FILENAMEERROR     11
#define   GZIP_INTERNAL          12


typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;


/*!  A class to provide gzip compression and decompression.  It is limited
 *   to level 9 compression.  This is a port of the GPLed verion of gzip
 *   for Qt4. */

class US_UTIL_EXTERN US_Gzip
{
  public:
    US_Gzip();

    /*! Compress a file
     * \param filename  The file to be compressed.
     * \return An error code.  Zero for no error */
    int     gzip   ( const QString& );


    /*! Decompress a file
     * \param filename  The file to be compressed.
     * \returns An error code.  Zero for no error */
    int     gunzip ( const QString& ); 

    /*! Explain an error
     * \param error  The error code that was returned gzip or gunzip
     * \return A string that corresponds to the error code */ 
    QString explain( const int ); 

    //! For definition of different compression levels
    typedef struct config
    {
      ush good_length; /*!< reduce lazy search above this match length */
      ush max_lazy;    /*!< do not perform lazy search above this match length */
      ush nice_length; /*!< quit search above this match length */
      ush max_chain;   /*!< maximum length of a single chain */
    } config;

  private:
    off_t    bytes_in;      /* number of input bytes */
    off_t    bytes_out;     /* number of output bytes */

    ulg      bb;            /* bit buffer */
    unsigned bk;            /* bits in bit buffer */
    unsigned outcnt;        /* bytes in output buffer */

    unsigned inptr;
    unsigned hufts;         /* track memory usage */
    unsigned insize;

    unsigned crc;           /* current crc value */

    int      ifd;           /* input file descriptor */
    int      ofd;           /* output file descriptor */

#define INBUFSIZ    0x8000  /* input buffer size */
#define INBUF_EXTRA     64  /* required by unlzw() */
#define OUTBUFSIZ    16384  /* output buffer size */
#define OUTBUF_EXTRA  2048  /* required by unlzw() */

#define WSIZE       0x8000  /* window size--must be a power of two, and */
                            /*  at least 32K for zip's deflate method */

    /*! Huffman code lookup table entry--this entry is four bytes for machines
       that have 16-bit pointers (e.g. PC's in the small or medium model).
       Valid extra bits are 0..13.  e == 15 is EOB (end of block), e == 16
       means that v is a literal, 16 < e < 32 means that v is a pointer to
       the next table, which codes e - 16 bits, and lastly e == 99 indicates
       an unused code.  If a code with e == 99 is looked up, this implies an
       error in the data. */
   
    struct huft 
    {
       uch e;                /*!< number of extra bits or operation */
       uch b;                /*!< number of bits in this code or subcode */
       union 
       {
          ush          n;     /*!< literal, length base, or distance base */
          struct huft* t;     /*!< pointer to next level of table */
       } v;                   /*!< a simple name for the union */
    };

    uch inbuf [ INBUFSIZ  + INBUF_EXTRA  ];
    uch outbuf[ OUTBUFSIZ + OUTBUF_EXTRA ];
    uch window[ 2L * WSIZE               ];

    int     treat_file     ( const QString&, bool );
    QString make_ofname    ( const QString&, bool );
    int     huft_build     ( unsigned*, unsigned, unsigned, ush*, ush*,
                                         struct huft**, int* );
    int     huft_free      ( struct huft * );
    int     inflate_codes  ( struct huft*, struct huft*, int, int );
    int     inflate_stored ( void );
    int     inflate_fixed  ( void );
    int     inflate_dynamic( void );
    int     inflate_block  ( int* );
    int     inflate        ( void );

    int     fill_inbuf     ( int );
    void    flush_window   ( void );
    ulg     updcrc         ( uch*, unsigned );
    void    write_buf      ( int, void*, unsigned );
    char*   base_name      ( char* );
    void    flush_outbuf   ( void );

    // deflate variables and defines

    typedef ush      Pos;
    typedef unsigned IPos;
    /* A Pos is an index in the character window. We use short instead of int to
     * save space in the various tables. IPos is used only for parameter passing.  */

    long             block_start;
    /* window position at the beginning of the current output block. Gets
     * negative when the window is moved backwards.  */

    unsigned         ins_h;         /* hash index of string to be inserted */
    unsigned         lookahead;     /* number of valid bytes ahead in window */
    int              eofile;        /* flag set at end of input file */
    unsigned int     max_lazy_match;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4.  */

    unsigned         max_chain_length;
    /* To speed up deflation, hash chains are never searched beyond this length.
     * A higher limit improves compression ratio but degrades the speed.  */

    unsigned int     prev_length;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation. */

    unsigned         strstart;      /* start of string to insert */
    unsigned         match_start;   /* start of matching string */

#define max_insert_length  max_lazy_match
    /* Insert new strings in the hash table only if the match length
     * is not greater than this length. This saves time but degrades compression.
     * max_insert_length is used only for compression levels <= 3. */

    unsigned         good_match;
    /* Use a faster search when the previous match is longer than this */

    int              nice_match; /* Stop searching when current match exceeds this */

#define tab_prefix prev        /* hash link (see deflate.c) */
#define head ( prev + WSIZE )  /* hash head (see deflate.c) */

#define BITS 16
    ush              tab_prefix[ 1L << BITS ];

    unsigned short   bi_buf;
    /* Output buffer. bits are inserted starting at the bottom (least significant
     * bits). */

#define Buf_size ( 8 * 2 * sizeof( char ) )
    /* Number of bits used within bi_buf. (bi_buf might be implemented on
     * more than 16 bits on some systems.)  */

    int              bi_valid;
    /* Number of valid bits in bi_buf.  All bits above the last valid bit
     * are always zero. */

    unsigned         last_lit;    /* running index in inbuf */

#define MAX_BITS     15
    /* All codes must not exceed MAX_BITS bits */

#define MAX_BL_BITS   7
    /* Bit length codes must not exceed MAX_BL_BITS bits */

#define LENGTH_CODES 29
    /* number of length codes, not counting the special END_BLOCK code */

#define LITERALS    256
    /* number of literal bytes 0..255 */

#define END_BLOCK   256
    /* end of block literal code */

#define D_CODES      30
    /* number of distance codes */

#define BL_CODES     19
    /* number of codes used to transfer the bit lengths */

#define L_CODES ( LITERALS + 1 + LENGTH_CODES )
    /* number of Literal or Length codes, including the END_BLOCK code */

#define Freq fc.freq
#define Code fc.code
#define Dad  dl.dad
#define Len  dl.len

#define HEAP_SIZE ( 2 * L_CODES + 1 )

/* Data structure describing a single value and its code string. */
   typedef struct ct_data 
   {
      union 
      {
         ush  freq;       /* frequency count */
         ush  code;       /* bit string */
      } fc;
      union 
      {
         ush  dad;          /* father node in Huffman tree */
         ush  len;          /* length of bit string */

      } dl;
   } ct_data;

      ct_data          dyn_ltree[ HEAP_SIZE ];       /* literal and length tree */
      ct_data          dyn_dtree[ 2 * D_CODES + 1 ]; /* distance tree */

      ct_data          static_ltree[ L_CODES + 2 ];
      /* The static literal tree. Since the bit lengths are imposed, there is no
       * need for the L_CODES extra codes used during heap construction. However
       * The codes 286 and 287 are needed to build a canonical tree (see ct_init
       * below).  */

      ct_data          static_dtree[ D_CODES ];
      /* The static distance tree. (Actually a trivial tree since all codes use
       * 5 bits.) */

      ct_data          bl_tree[ 2 * BL_CODES + 1 ];
      /* Huffman tree for the bit lengths */

#define MIN_MATCH    3
#define MAX_MATCH  258
      /* The minimum and maximum match lengths */

      uch              length_code[ MAX_MATCH - MIN_MATCH + 1 ];
      /* length code for each normalized match length (0 == MIN_MATCH) */

      uch              dist_code[ 512 ];
      /* distance codes. The first 256 values correspond to the distances
       * 3 .. 258, the last 256 values correspond to the top 8 bits of
       * the 15 bit distances.  */

      unsigned         last_dist;   /* running index in d_buf */
      unsigned         last_flags;  /* running index in flag_buf */
      uch              flags;       /* current flags not yet saved in flag_buf */
      uch              flag_bit;    /* current bit used in flags */
      /* bits are filled in flags starting at bit 0 (least significant).
       * Note: these flags are overkill in the current code since we don't
       * take advantage of DIST_BUFSIZE == LIT_BUFSIZE.  */

#define DIST_BUFSIZE 0x8000 /* buffer for distances, see trees.c */
      ush              d_buf[ DIST_BUFSIZE ];

#ifndef LIT_BUFSIZE
#  ifdef SMALL_MEM
#    define LIT_BUFSIZE  0x2000
#  else
#  ifdef MEDIUM_MEM
#    define LIT_BUFSIZE  0x4000
#  else
#    define LIT_BUFSIZE  0x8000
#  endif
#  endif
#endif

      uch              flag_buf[ LIT_BUFSIZE / 8 ];

      typedef struct tree_desc
      {
         ct_data*       dyn_tree;      /* the dynamic tree */
         ct_data*       static_tree;   /* corresponding static tree or NULL */
         int*           extra_bits;    /* extra bits for each code or NULL */
         int            extra_base;    /* base index for extra_bits */
         int            elems;         /* max number of elements in the tree */
         int            max_length;    /* max bit length for the codes */
         int            max_code;      /* largest code with non zero frequency */
      } tree_desc;

      tree_desc        l_desc; 
      tree_desc        d_desc;
      tree_desc        bl_desc;

      ulg              opt_len;       /* bit length of current block with optimal trees */
      ulg              static_len;    /* bit length of current block with static trees */
      off_t            compressed_len; /* total bit length of compressed file */
      int*             file_method;   /* pointer to DEFLATE or STORE */

      int              base_length[ LENGTH_CODES ];
      int              base_dist[ D_CODES ];

      int              heap[ 2 * L_CODES + 1 ]; /* heap used to build the Huffman trees */
      int              heap_len;      /* number of elements in the heap */
      int              heap_max;      /* element of largest frequency */
      /* The sons of heap[n] are heap[ 2 * n ] and heap[ 2 * n + 1 ]. heap[ 0 ] 
       * is not used.  The same heap array is used to build all trees.  */

      uch              depth[ 2 * L_CODES + 1 ];
      /* Depth of each subtree used as tie breaker for trees of equal frequency */

      ush              bl_count[ MAX_BITS + 1 ];
      /* Number of codes at each bit length for an optimal tree */

      // Deflate methods
      off_t    deflate        ( void );
      void     lm_init        ( void );
      int      file_read      ( char*, unsigned int );
      void     fill_window    ( void );
      int      longest_match  ( IPos );
      void     ct_init        ( void );
      int      ct_tally       ( int, int );
      off_t    flush_block    ( char*, ulg, int );
      void     bi_init        ( void );
      void     build_tree     ( tree_desc* );
      int      build_bl_tree  ( void );
      void     copy_block     ( char*, unsigned, int );
      void     send_bits      ( int, int );
      void     compress_block ( ct_data*, ct_data* );
      void     send_all_trees ( int, int, int );
      void     send_tree      ( ct_data*, int );
      void     init_block     ( void );
      void     bi_windup      ( void );
      unsigned bi_reverse     ( unsigned, int );
      void     gen_bitlen     ( tree_desc* );
      void     gen_codes      ( ct_data*, int );
      void     pqdownheap     ( ct_data*, int );
      void     scan_tree      ( ct_data*, int );

};
#endif

