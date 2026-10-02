#!/usr/bin/perl
# strip_pre_qt5.pl -- remove the preprocessor conditions that are decided for
# every Qt that SOMO builds against: keep the live branch, drop the dead one,
# and reduce a mixed condition to the part that is left.
#
#   strip_pre_qt5.pl [--min-qt 5.15] [--dry-run] file ...
#
# A condition is changed only when each QT_VERSION comparison in it has the
# same value for every Qt from --min-qt (default 5.15) up to, but not
# including, 7.0. Anything else, e.g. QT_VERSION >= QT_VERSION_CHECK(6, 0, 0),
# is left exactly as written. "#if 1" and "#if 0" lines whose comment names
# QT_VERSION (old version checks switched by hand) are resolved as well.
#
# As in the C preprocessor, a "#if" inside a /* */ comment is not a directive.
# Only files that change are rewritten, and each is listed with its counts.
#
# ehb54/ultrascan-tickets#1105

use strict;
use warnings;
use Getopt::Long;

my $usage   = "usage: $0 [--min-qt 5.15] [--dry-run] file ...\n";
my $min_qt  = "5.15";
my $dry_run = 0;
if ( !GetOptions( "min-qt=s" => \$min_qt, "dry-run" => \$dry_run ) || !@ARGV ) {
   die $usage;
}
if ( $min_qt !~ /^(\d+)\.(\d+)(?:\.(\d+))?$/ ) {
   die "--min-qt wants a version such as 5.15 or 5.15.2\n";
}
my $lo = ( $1 << 16 ) | ( $2 << 8 ) | ( $3 // 0 );   # oldest Qt built against
my $hi = 0x070000;                                  # first Qt not built against

my %flip = ( '<' => '>', '<=' => '>=', '>' => '<', '>=' => '<=', '==' => '==', '!=' => '!=' );

my $files_changed = 0;
for my $file ( @ARGV ) {
   $files_changed += process_file( $file );
}
print "$files_changed file(s) " . ( $dry_run ? "would change" : "changed" ) . "\n";

# --- one file ----------------------------------------------------------------

sub process_file {
   my ( $file ) = @_;
   open( my $in, '<', $file ) or die "$file: $!\n";
   my @lines = <$in>;
   close( $in );

   my @out;
   my @stack;            # one frame per open #if group, see below
   my $in_comment = 0;   # a /* */ comment is open
   my $resolved   = 0;   # #if lines decided outright
   my $rewritten  = 0;   # #if lines reduced to the undecided part

   # Frames: kind 'pass' (a group this script leaves alone), 'skip' (a group
   # inside a dropped branch), or 'res' (a group whose #if was decided or
   # reduced). A 'res' frame records whether an opening #if was written out
   # ('open'), whether a branch has been kept for certain ('taken'), and whether
   # the current branch is kept ('active').
   for my $line ( @lines ) {
      my $starts_in_comment = $in_comment;
      $in_comment = scan_comments( $line, $in_comment );
      my $active = !grep { !$_->{ active } } @stack;

      my $dir;
      if ( !$starts_in_comment && $line =~ /^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b/ ) {
         $dir = $1;
      }
      if ( !$dir ) {
         if ( $active ) {
            push @out, $line;
         }
         next;
      }

      if ( $dir eq 'if' || $dir eq 'ifdef' || $dir eq 'ifndef' ) {
         if ( !$active ) {
            push @stack, { kind => 'skip', active => 0 };
            next;
         }
         my $v = $dir eq 'if' ? condition( $line, $file ) : undef;
         if ( !$v ) {
            push @stack, { kind => 'pass', active => 1 };
            push @out, $line;
         } elsif ( $v->[ 0 ] eq 'T' ) {
            push @stack, { kind => 'res', open => 0, taken => 1, active => 1 };
            $resolved++;
         } elsif ( $v->[ 0 ] eq 'F' ) {
            push @stack, { kind => 'res', open => 0, taken => 0, active => 0 };
            $resolved++;
         } else {
            push @stack, { kind => 'res', open => 1, taken => 0, active => 1 };
            push @out, with_condition( $line, $v->[ 1 ] );
            $rewritten++;
         }
         next;
      }

      if ( !@stack ) {
         die "$file: #$dir without #if: $line";
      }
      my $top = $stack[ -1 ];
      if ( $dir eq 'endif' ) {
         pop @stack;
         my $parent_active = !grep { !$_->{ active } } @stack;
         if ( $parent_active && ( $top->{ kind } eq 'pass' || ( $top->{ kind } eq 'res' && $top->{ open } ) ) ) {
            push @out, $line;
         }
         next;
      }
      if ( $top->{ kind } eq 'skip' ) {
         next;
      }
      if ( $top->{ kind } eq 'pass' ) {
         if ( $dir eq 'elif' && condition( $line, $file ) ) {
            die "$file: decided QT_VERSION condition on an #elif of an unrelated #if: $line";
         }
         if ( !grep { !$_->{ active } } @stack[ 0 .. $#stack - 1 ] ) {
            push @out, $line;
         }
         next;
      }

      # #elif / #else of a decided or reduced group (its parent is active)
      if ( $top->{ taken } ) {
         $top->{ active } = 0;
         next;
      }
      if ( $dir eq 'else' ) {
         if ( $top->{ open } ) {
            push @out, $line;
         } else {
            $top->{ taken } = 1;
         }
         $top->{ active } = 1;
         next;
      }
      my $v = condition( $line, $file );
      if ( $v && $v->[ 0 ] eq 'F' ) {
         $top->{ active } = 0;
         $resolved++;
      } elsif ( $v && $v->[ 0 ] eq 'T' ) {
         if ( $top->{ open } ) {
            ( my $else = $line ) =~ s/^(\s*#\s*)elif\b.*/$1else/;
            push @out, $else;
         }
         $top->{ taken }  = 1;
         $top->{ active } = 1;
         $resolved++;
      } else {
         my $l = $v ? with_condition( $line, $v->[ 1 ] ) : $line;
         if ( $v ) {
            $rewritten++;
         }
         if ( !$top->{ open } ) {
            $l =~ s/^(\s*#\s*)elif\b/$1if/;   # the first branch still standing opens the group
            $top->{ open } = 1;
         }
         push @out, $l;
         $top->{ active } = 1;
      }
   }
   if ( @stack ) {
      die "$file: #if without #endif\n";
   }
   if ( $in_comment ) {
      die "$file: unterminated /* comment\n";
   }

   if ( !$resolved && !$rewritten ) {
      return 0;
   }
   printf "%-64s %4d resolved %3d reduced %5d lines removed\n", $file, $resolved, $rewritten, @lines - @out;
   if ( !$dry_run ) {
      open( my $o, '>', $file ) or die "$file: $!\n";
      print $o @out;
      close( $o ) or die "$file: $!\n";
   }
   return 1;
}

# Whether a /* */ comment is open after $line, given whether one was open
# before it. String and character literals and // comments are skipped.
sub scan_comments {
   my ( $line, $in ) = @_;
   my $i = 0;
   my $n = length( $line );
   while ( $i < $n ) {
      if ( $in ) {
         my $e = index( $line, '*/', $i );
         if ( $e < 0 ) {
            return 1;
         }
         $i  = $e + 2;
         $in = 0;
         next;
      }
      my $c2 = substr( $line, $i, 2 );
      if ( $c2 eq '//' ) {
         return 0;
      }
      if ( $c2 eq '/*' ) {
         $in = 1;
         $i += 2;
         next;
      }
      my $c = substr( $line, $i, 1 );
      $i++;
      if ( $c eq '"' || $c eq "'" ) {
         while ( $i < $n ) {
            my $d = substr( $line, $i, 1 );
            $i += $d eq '\\' ? 2 : 1;
            if ( $d eq $c ) {
               last;
            }
         }
      }
   }
   return $in;
}

# --- #if / #elif conditions ----------------------------------------------------

# Split a directive line into ( prefix, condition, trailing comment ).
sub split_directive {
   my ( $line ) = @_;
   ( my $body = $line ) =~ s/\r?\n\z//;
   if ( $body !~ /^(\s*#\s*(?:if|elif)\b\s*)(.*?)(\s*(?:\/\/.*|\/\*.*)?)$/ ) {
      die "cannot read directive: $line";
   }
   return ( $1, $2, $3 );
}

# The line with its condition replaced (prefix, comment and newline kept).
sub with_condition {
   my ( $line, $cond ) = @_;
   my ( $pre, undef, $trail ) = split_directive( $line );
   my ( $eol ) = $line =~ /(\r?\n)\z/;
   return $pre . $cond . $trail . ( $eol // '' );
}

# undef if the directive is left alone; otherwise [ 'T' ], [ 'F' ], or
# [ 'U', $condition_still_needed ].
sub condition {
   my ( $line, $file ) = @_;
   my ( undef, $cond, $trail ) = split_directive( $line );
   my $ours = $cond =~ /\bQT_VERSION\b/ || ( $cond =~ /^\s*[01]\s*$/ && $trail =~ /\bQT_VERSION\b/ );
   if ( !$ours ) {
      return undef;
   }
   my $v = eval {
      my $ast = parse( $cond );
      evaluate( $ast, $cond );
   };
   if ( !$v ) {
      warn "$file: left alone, cannot read condition ($@): $line";
      return undef;
   }
   if ( $v->[ 0 ] eq 'U' ) {
      if ( !$v->[ 2 ] ) {
         return undef;   # nothing in it is decided
      }
      return [ 'U', strip_outer_parens( $v->[ 1 ] ) ];
   }
   return $v;
}

# "( a || b )" -> "a || b" when the outer parentheses enclose everything.
sub strip_outer_parens {
   my ( $s ) = @_;
   while ( $s =~ /^\(\s*(.*?)\s*\)$/ ) {
      my $inner = $1;
      my $depth = 0;
      for my $c ( split //, $inner ) {
         $depth += $c eq '(' ? 1 : $c eq ')' ? -1 : 0;
         if ( $depth < 0 ) {
            return $s;
         }
      }
      $s = $inner;
   }
   return $s;
}

# --- a parser for the #if expression subset SOMO uses ---------------------------
# || && ! comparisons, parentheses, defined( X ) / defined X, numbers,
# identifiers, and function-like macros such as QT_VERSION_CHECK( 6, 0, 0 ).
# Nodes record their span in the source so untouched parts are kept verbatim.

my @tok;   # tokens of the condition being parsed
my $p;     # index of the next token

sub parse {
   my ( $s ) = @_;
   @tok = ();
   pos( $s ) = 0;
   while ( pos( $s ) < length( $s ) ) {
      if ( $s =~ /\G\s+/gc ) {
         next;
      }
      my $at = pos( $s );
      if ( $s =~ /\G(\|\||&&|==|!=|<=|>=|[<>!(),])/gc ) {
         push @tok, { type => 'op', val => $1, at => $at, end => pos( $s ) };
      } elsif ( $s =~ /\G(0[xX][0-9a-fA-F]+|\d+)[uUlL]*/gc ) {
         my $lit = $1;   # copy first: the match below resets $1
         my $num = $lit =~ /^0[xX]/ ? hex( $lit ) : $lit + 0;
         push @tok, { type => 'num', val => $num, at => $at, end => pos( $s ) };
      } elsif ( $s =~ /\G([A-Za-z_]\w*)/gc ) {
         push @tok, { type => 'id', val => $1, at => $at, end => pos( $s ) };
      } else {
         die "unexpected '" . substr( $s, $at, 1 ) . "'\n";
      }
   }
   $p = 0;
   my $ast = parse_or();
   if ( $p < @tok ) {
      die "unexpected '$tok[ $p ]{ val }'\n";
   }
   return $ast;
}

sub peek_op {
   my ( @vals ) = @_;
   if ( $p >= @tok || $tok[ $p ]{ type } ne 'op' ) {
      return 0;
   }
   return scalar grep { $_ eq $tok[ $p ]{ val } } @vals;
}

sub expect_op {
   my ( $val ) = @_;
   if ( !peek_op( $val ) ) {
      die "expected '$val'\n";
   }
   return $tok[ $p++ ];
}

sub parse_list {   # a run of $next-level nodes joined by $op
   my ( $kind, $op, $next ) = @_;
   my @parts = ( $next->() );
   while ( peek_op( $op ) ) {
      $p++;
      push @parts, $next->();
   }
   if ( @parts == 1 ) {
      return $parts[ 0 ];
   }
   return { kind => $kind, parts => \@parts, at => $parts[ 0 ]{ at }, end => $parts[ -1 ]{ end } };
}

sub parse_or {
   return parse_list( 'or', '||', \&parse_and );
}

sub parse_and {
   return parse_list( 'and', '&&', \&parse_eq );
}

sub parse_binary {   # left-associative comparisons at one precedence level
   my ( $next, @ops ) = @_;
   my $l = $next->();
   while ( peek_op( @ops ) ) {
      my $op = $tok[ $p++ ]{ val };
      my $r  = $next->();
      $l = { kind => 'cmp', op => $op, l => $l, r => $r, at => $l->{ at }, end => $r->{ end } };
   }
   return $l;
}

sub parse_eq {
   return parse_binary( \&parse_rel, '==', '!=' );
}

sub parse_rel {
   return parse_binary( \&parse_unary, '<', '<=', '>', '>=' );
}

sub parse_unary {
   if ( peek_op( '!' ) ) {
      my $t   = $tok[ $p++ ];
      my $arg = parse_unary();
      return { kind => 'not', arg => $arg, at => $t->{ at }, end => $arg->{ end } };
   }
   return parse_primary();
}

sub parse_primary {
   if ( $p >= @tok ) {
      die "condition ends too early\n";
   }
   my $t = $tok[ $p++ ];
   if ( $t->{ type } eq 'op' && $t->{ val } eq '(' ) {
      my $arg   = parse_or();
      my $close = expect_op( ')' );
      return { kind => 'paren', arg => $arg, at => $t->{ at }, end => $close->{ end } };
   }
   if ( $t->{ type } eq 'num' ) {
      return { kind => 'num', val => $t->{ val }, at => $t->{ at }, end => $t->{ end } };
   }
   if ( $t->{ type } ne 'id' ) {
      die "unexpected '$t->{ val }'\n";
   }
   if ( $t->{ val } eq 'defined' ) {
      my $paren = peek_op( '(' );
      if ( $paren ) {
         $p++;
      }
      if ( $p >= @tok || $tok[ $p ]{ type } ne 'id' ) {
         die "defined without a name\n";
      }
      my $name = $tok[ $p++ ];
      my $end  = $paren ? expect_op( ')' )->{ end } : $name->{ end };
      return { kind => 'defined', name => $name->{ val }, at => $t->{ at }, end => $end };
   }
   if ( peek_op( '(' ) ) {
      $p++;
      my @args;
      if ( !peek_op( ')' ) ) {
         push @args, parse_or();
         while ( peek_op( ',' ) ) {
            $p++;
            push @args, parse_or();
         }
      }
      my $close = expect_op( ')' );
      return { kind => 'call', name => $t->{ val }, args => \@args, at => $t->{ at }, end => $close->{ end } };
   }
   return { kind => 'id', name => $t->{ val }, at => $t->{ at }, end => $t->{ end } };
}

# --- evaluation over the supported Qt range ----------------------------------------

# A constant compared with QT_VERSION: a number or QT_VERSION_CHECK( a, b, c ).
sub version_value {
   my ( $n ) = @_;
   if ( $n->{ kind } eq 'num' ) {
      return $n->{ val };
   }
   if ( $n->{ kind } eq 'call' && $n->{ name } eq 'QT_VERSION_CHECK' && @{ $n->{ args } } == 3 ) {
      my @v = map { $_->{ kind } eq 'num' ? $_->{ val } : undef } @{ $n->{ args } };
      if ( grep { !defined( $_ ) } @v ) {
         return undef;
      }
      return ( $v[ 0 ] << 16 ) | ( $v[ 1 ] << 8 ) | $v[ 2 ];
   }
   return undef;
}

# "QT_VERSION $op $n" for every QT_VERSION in [ $lo, $hi ): 'T', 'F', or undef.
sub decide {
   my ( $op, $n ) = @_;
   my ( $first, $last ) = ( $lo, $hi - 1 );
   my %when = (
      '<'  => [ $last <  $n, $first >= $n ],
      '<=' => [ $last <= $n, $first >  $n ],
      '>'  => [ $first >  $n, $last <= $n ],
      '>=' => [ $first >= $n, $last <  $n ],
      '==' => [ $first == $n && $last == $n, $n < $first || $n > $last ],
      '!=' => [ $n < $first || $n > $last, $first == $n && $last == $n ],
   );
   my ( $true, $false ) = @{ $when{ $op } };
   return $true ? 'T' : $false ? 'F' : undef;
}

# [ 'T' ], [ 'F' ], or [ 'U', $text, $changed ], where $text is what is still
# needed and $changed says whether it differs from the source text.
sub evaluate {
   my ( $n, $src ) = @_;
   my $k    = $n->{ kind };
   my $orig = substr( $src, $n->{ at }, $n->{ end } - $n->{ at } );

   if ( $k eq 'cmp' ) {
      my ( $l, $r ) = ( $n->{ l }, $n->{ r } );
      my $d;
      if ( $l->{ kind } eq 'id' && $l->{ name } eq 'QT_VERSION' && defined( version_value( $r ) ) ) {
         $d = decide( $n->{ op }, version_value( $r ) );
      } elsif ( $r->{ kind } eq 'id' && $r->{ name } eq 'QT_VERSION' && defined( version_value( $l ) ) ) {
         $d = decide( $flip{ $n->{ op } }, version_value( $l ) );
      }
      return $d ? [ $d ] : [ 'U', $orig, 0 ];
   }
   if ( $k eq 'num' ) {
      return [ $n->{ val } ? 'T' : 'F' ];
   }
   if ( $k eq 'not' ) {
      my $v = evaluate( $n->{ arg }, $src );
      if ( $v->[ 0 ] ne 'U' ) {
         return [ $v->[ 0 ] eq 'T' ? 'F' : 'T' ];
      }
      return $v->[ 2 ] ? [ 'U', "!$v->[ 1 ]", 1 ] : [ 'U', $orig, 0 ];
   }
   if ( $k eq 'paren' ) {
      my $v = evaluate( $n->{ arg }, $src );
      if ( $v->[ 0 ] ne 'U' ) {
         return $v;
      }
      return $v->[ 2 ] ? [ 'U', "( $v->[ 1 ] )", 1 ] : [ 'U', $orig, 0 ];
   }
   if ( $k eq 'and' || $k eq 'or' ) {
      my ( $absorbing, $neutral ) = $k eq 'and' ? ( 'F', 'T' ) : ( 'T', 'F' );
      my @keep;
      my $changed = 0;
      for my $part ( @{ $n->{ parts } } ) {
         my $v = evaluate( $part, $src );
         if ( $v->[ 0 ] eq $absorbing ) {
            return [ $absorbing ];
         }
         if ( $v->[ 0 ] eq $neutral ) {
            $changed = 1;
            next;
         }
         $changed ||= $v->[ 2 ];
         push @keep, $v->[ 1 ];
      }
      if ( !@keep ) {
         return [ $neutral ];
      }
      if ( !$changed ) {
         return [ 'U', $orig, 0 ];
      }
      return [ 'U', join( $k eq 'and' ? ' && ' : ' || ', @keep ), 1 ];
   }
   return [ 'U', $orig, 0 ];   # defined(), identifiers, other macros
}
