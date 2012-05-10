#!/usr/bin/perl

while ( $ARGV[ 0 ] =~ /^-/ )
{
    $arg = shift;
    
    if ( $arg =~ /-d/ )
    {
        $debug++;
        next;
    }

    if ( $arg =~ /-c/ )
    {
        $commit++;
        next;
    }

    if ( $arg =~ /-s/ )
    {
        $show_diffs++;
        next;
    }

    die "usage: $0 { options }
 valid options:
  -d debug
  -c commit differences
  -s show differences";
}

$us3 = $ENV{'us3'} || die "can't find env variable us3\n";
$us3somo  = "$us3/somo";
$us3d     = "$us3/somo/develop";
$us3stage = "$us3/somo/develop_stage";

$errors .= "directory $us3d does not exist\n" if !-d $us3d;
$errors .= "directory $us3stage does not exist\n" if !-d $us3stage;

chdir $us3stage || die "can't change to dir $us3stage $!\n";

mkdir "../develop" if !-e "../develop";

if ( !-e "../develop/include" || 
     !-e "../develop/src" ||
     !-e "../develop/generic.pri" )
{
    $cmd = "find . -depth -print | cpio -pdmv ../develop/
cd ../develop
sh revision.sh
";
    print $cmd;
    print `$cmd`;
    exit;
}

print "getting file list\n";
@list = `find . -name "*.cpp" -o -name "*.h" -o -name "*.pro" -o -name "*.pri" -o -name "*.xpm"`;

grep chomp, @list;
@list = grep !/src\/moc\/moc/, @list;
@list = grep !/qmake_image_collection\.cpp/, @list;
@list = grep !/us_revision\.h/, @list;

chdir $us3somo || die "can't change to dir $us3somo $!\n";

print "checking for differences\n";

for ( $i = 0; $i < @list; $i++ )
{
    $f = $list[ $i ];
    if ( !-e "$us3d/$f" )
    {
        push @copy, $f;
        print "will copy: $us3d: $f (does not exist)\n";
    } else {
        @diff = `diff $us3stage/$f $us3d/$f`;
        if ( @diff )
        {
            push @copy, $f;
            print "will copy: $us3d: $f (differences)\n";
            print ( join '', @diff ) if $show_diffs;
        }
    }
}
            
if ( $commit )
{
    for ( $i = 0; $i < @copy; $i++ )
    {
        $f = $copy[ $i ];
        print "copying over $f\n";
        `cp $us3stage/$f $us3d/$f`;
    }
}
