#!/usr/bin/perl

$addcout = '
static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}
';

@cpps = `find . -name "*.cpp"`;
grep chomp, @cpps;
for ( $i = 0; $i < @cpps; $i++ )
{
    $f = $cpps[$i];
    print "$f:\n";
    open(IN, $f) || die "can't open $f $!\n";
    @f = <IN>;
    close IN;

    $lastinc = 0;
    for ( $j = 0; $j < @f; $j++ )
    {
        $f[$j] =~ s/QComboBox/Q3ComboBox/g;
        $f[$j] =~ s/global_colors.Qt::gray/global_colors.gray/;
        $f[$j] =~ s/file.rename\( oldhome \+ oldfile\[i\], home \+ newfile\[i\], true/file.rename\( oldhome + oldfile[i], home + newfile[i]/;
        $f[$j] =~ s/dir.rename\( olddir, newdir, true/dir.rename\( olddir, newdir/;
        if ( $f[$j] =~ /pep->.+ = sequence->contains/ ) {
            $f[$j] =~ s/sequence->contains/sequence->count/;
            $f[$j] =~ s/, false/, Qt::CaseInsensitive/;
        }
        if ( $f =~ /us_matrix.cpp/ )
        {
            $f[$j] =~ s/debug\(/qDebug\(/;
        }

        $f[$j] .= $addcout if $f[$j] =~ /note: this program uses cout and\/or cerr and this should be replaced/;

        $f[$j] = "// $f[$j]" if $f[$j] =~ /QSqlQuery/;
    }


#    $f = "$f.new";
    open(OUT, ">$f") || die "can't open $f for writing $!\n";
    print OUT @f;
    close OUT;
    print "written: $f\n";
}
    
    

@h = `find . -name "*.h"`;
grep chomp, @h;
for ( $i = 0; $i < @h; $i++ )
{
    $f = $h[$i];
    print "$f:\n";
    open(IN, $f) || die "can't open $f $!\n";
    @f = <IN>;
    close IN;

    $lastinc = 0;
    for ( $j = 0; $j < @f; $j++ )
    {
        $f[$j] =~ s/QComboBox/Q3ComboBox/g;
        $f[$j] =~ s/qcombobox.h/Q3ComboBox/g;
    }


#    $f = "$f.new";
    open(OUT, ">$f") || die "can't open $f for writing $!\n";
    print OUT @f;
    close OUT;
    print "written: $f\n";
}
    
   
@rm = `find . -name "us_imgviewer.*"`;
for ( $i = 0; $i < @rm; $i++ )
{
    unlink $rm[$i];
}
