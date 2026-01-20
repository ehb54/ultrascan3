#!/usr/bin/env perl
use strict;
use warnings;
use File::Temp qw/tempdir tempfile/;
use File::Basename qw/dirname/;
use File::Spec;
use File::Copy qw/move/;
use List::Util qw/uniq/;

# --------- CONFIG: regexes of paths to remove (Perl regex literals) ----------
# Write patterns with qr{...} so you can use / without escaping and add modifiers.
# Examples:
#   qr{^etc/hosts$}
#   qr{^opt/pkg/tmp(?:/|$)}            # directory and its contents
#   qr{(?:^|/)\.DS_Store$}            # any .DS_Store anywhere
#   qr{^var/log/.*\.log$}             # all .log files under var/log
my @REMOVE_PATTERNS = (
    qr{libc\.so}
  # qr{^etc/hosts$},
  # qr{^opt/pkg/tmp(?:/|$)},
  # qr{(?:^|/)\.DS_Store$},
);
# -----------------------------------------------------------------------------


# Usage: perl tar_fixup_regex.pl archive.tar.xz [--dry-run]
my ($archive, $dry) = @ARGV;
die "Usage: $0 archive.tar.xz [--dry-run]\n" unless defined $archive;
$dry = ($dry && $dry eq '--dry-run') ? 1 : 0;

if (!@REMOVE_PATTERNS) {
  print "No removal patterns defined; nothing to do.\n";
  exit 0;
}

# -- helper: normalize member name like tar -t output to a canonical path
sub norm {
  my ($p) = @_;
  $p =~ s{^\./}{};
  $p =~ s{^/+}{};
  $p =~ s{/$}{};   # keep logical dir name without trailing slash
  return $p;
}

# 1) List archive contents
open my $LS, "-|", "tar", "-tJf", $archive
  or die "Failed to run 'tar -tJf $archive': $!\n";
my @members;
my %members_set;
while (<$LS>) {
  chomp;
  my $p = norm($_);
  next unless length $p;
  push @members, $p unless $members_set{$p}++;
}
close $LS;

if (!@members) {
  print "Archive appears empty; nothing to do.\n";
  exit 0;
}

# 2) Find matches for any regex in @REMOVE_PATTERNS
my @matches;
MEMBER: for my $m (@members) {
  for my $re (@REMOVE_PATTERNS) {
    if ($m =~ $re) { push @matches, $m; next MEMBER; }
  }
}
@matches = uniq @matches;

if (!@matches) {
  print "No archive members matched removal patterns. No changes made.\n";
  exit 0;
}

print "Archive: $archive\nMatched for removal (" . scalar(@matches) . "):\n",
      (map { "  $_\n" } @matches);

if ($dry) {
  print "[DRY-RUN] Skipping rebuild.\n";
  exit 0;
}

# 3) Build an exclude file for tar:
#    - exact path and './path'
#    - if it’s a directory (i.e., any member starts with 'path/'), exclude subtree
my %is_dir;
{
  # precompute for speed
  my %prefixes = map { $_ => 1 } @matches;
  for my $m (@members) {
    for my $p (@matches) {
      if (index($m, "$p/") == 0) { $is_dir{$p} = 1; }
    }
  }
}

my ($EXCL_FH, $EXCL) = tempfile("exclude_XXXX", TMPDIR => 1, UNLINK => 1);
for my $p (@matches) {
  print $EXCL_FH "$p\n";
  print $EXCL_FH "./$p\n";
  if ($is_dir{$p}) {
    # also exclude subtree patterns
    print $EXCL_FH "$p/*\n$p/**\n./$p/*\n./$p/**\n";
  }
}
close $EXCL_FH;

# 4) Extract everything except excluded items
my $work = tempdir("tarfix_XXXX", TMPDIR => 1, CLEANUP => 1);
my @x_cmd = ("tar", "-xJf", $archive, "-C", $work, "--exclude-from", $EXCL);
print "Extracting (excluding matches)...\n";
system(@x_cmd) == 0 or die "Extraction failed (@x_cmd): $?";

# 5) Rebuild to a temp archive, then atomically replace the original
my $arch_dir = dirname($archive);
my $tmp_arch = File::Spec->catfile($arch_dir, ".repack_$$.tar.xz");
my @c_cmd = ("tar", "-cJf", $tmp_arch, "-C", $work, ".");
print "Creating new archive...\n";
system(@c_cmd) == 0 or die "Creation failed (@c_cmd): $?";

move($tmp_arch, $archive)
  or die "Failed to replace '$archive' with new archive '$tmp_arch': $!\n";

print "Done. Updated archive written to $archive\n";
