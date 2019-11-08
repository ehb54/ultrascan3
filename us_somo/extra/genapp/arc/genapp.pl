#!/usr/bin/perl

$usage = "
usage: $0 { options } file

looks for file in source/file
and produces output in output/
uses templates in templates/

generally: .cpp and .h files are created in output/

options:
 -d debug

";

while ( $ARGV[ 0 ] =~ /^-/ )
{
    $arg = shift;
    
    if ( $arg =~ /-d/ )
    {
        $debug++;
        next;
    }

    die $usage;
}

#----------- global subroutines ---------------

sub err {
    die "$0: Error: $_[0]\n";
}


sub add_includes {
    push @includes, $_[ 0 ] if !$existing_includes{ $_[ 0 ] };
    $existing_includes{ $_[ 0 ] }++;
}

#----------- setup arrays -----------

#   valid outside of _elements & _variables

@valid_token_array = (
                      "prefix",
                      "helpprefix",
                      "name",
                      "title",
                      "credit",
                      "include",
                      "input",
                      "_variables",
                      "_elements",
                      "template"
                      );

for ( $i = 0; $i < @valid_token_array; $i++ )
{
    $valid_tokens{ $valid_token_array[ $i ] }++;
}

@capture_token_array = (
                        "prefix",
                        "helpprefix",
                        "name",
                        "title",
                        "include",
                        "input",
                        "credit",
                        "template"
                      );

for ( $i = 0; $i < @capture_token_array; $i++ )
{
    $capture_tokens{ $capture_token_array[ $i ] }++;
}

@capture_exactly_one_token_array = (
                                    "prefix",
                                    "name",
                                    "title"
                                    );

for ( $i = 0; $i < @capture_exactly_one_token_array; $i++ )
{
    $capture_exactly_one_tokens{ $capture_exactly_one_token_array[ $i ] }++;
}

@required_token_array = (
                         "prefix",
                         "name",
                         "title",
                         "credit",
                         "template"
                         );

for ( $i = 0; $i < @required_token_array; $i++ )
{
    $required_tokens{ $required_token_array[ $i ] }++;
}

@known_element_token_array = (
                              "editor",
                              "row",
                              "variable",
                              "button",
                              "progress"
                              );

for ( $i = 0; $i < @known_element_token_array; $i++ )
{
    $known_element_tokens{ $known_element_token_array[ $i ] }++;
}

@known_variable_token_array = (
                               "input"
                               );

for ( $i = 0; $i < @known_variable_token_array; $i++ )
{
    $known_variable_tokens{ $known_variable_token_array[ $i ] }++;
}

@known_variable_type_array = (
                              "double",
                              "string",
                              "int",
                              "uint"
                              );

for ( $i = 0; $i < @known_variable_type_array; $i++ )
{
    $known_variable_types{ $known_variable_type_array[ $i ] }++;
}

#----------- process source -----------

$file = shift || die $usage;
$fin = "source/$file";
$f = $fin;
die "$f does not exist\n" if !-e $f;
open IN, $f || err( "$f open error $!" );
while ( <IN> )
{
    chomp;
    next if /^\s*$/;
    next if /^\s*#.*$/;
    s/^\s*//;
    s/\s*$//;
    push @source, $_;
}
close IN;

sub error_on_line {
    return "$0: $fin line " . ( $i + 1 ) . ": $_[0]\n";
}

for ( $i = 0; $i < @source; $i++ )
{
    $_ = $source[ $i ];
    /^(\S+)/;
    $tok = $1;
    /^(\S+)\s+(.*$)/;
    $arg = $2;
    print "$0: $fin: line " . ( $i + 1 ) . ": token <$tok> arg <$arg>\n" if $debug > 2;
               
    if ( $tok =~ /^_elements/ )
    { 
        if ( $in_variables )
        {
            $errors .= error_on_line( "can not define _variables within _elements" );
            next;
        }
        $in_elements++;
        print "in elements\n" if $debug > 2;
        next;
    }

    if ( $tok =~ /^_variables/ )
    { 
        if ( $in_elements )
        {
            $errors .= error_on_line( "can not define _elements within _variables" );
            next;
        }
        $in_variables++;
        print "in variables\n" if $debug > 2;
        next;
    }

    if ( $tok =~ /^_end/ )
    { 
        undef $in_elements;
        undef $in_variables;
        print "out variables or elements\n" if $debug > 2;
        next;
    }

    if ( 
         !$in_elements &&
         !$in_variables &&
         !$valid_tokens{ $tok } )
    {
        $errors .= error_on_line( "unrecognized token $tok" );
        next;
    }
        
    if ( 
         !$in_elements &&
         !$in_variables &&
         $capture_tokens{ $tok } )
    {
        if ( $capture_exactly_one{ $tok } &&
             $capture{ $tok } )
        {
            $errors .= error_on_line( "duplicate token $tok, only one allowed" );
        }
        $capture{ $tok } = $arg;
        push @templates, "templates/$arg" if $tok =~ /template/;
        push @credits  , $arg             if $tok =~ /credit/;
        push @inputs   , $arg             if $tok =~ /input/;
        add_includes( $arg )              if $tok =~ /include/;
    }

    if ( $in_elements )
    {
        if ( !$known_element_tokens{ $tok } )
        {
            $errors .= error_on_line( "unknown element token <$tok>" );
            next;
        }
        push @elements, $_;
    }

    if ( $in_variables )
    {
        if ( !$known_variable_tokens{ $tok } )
        {
            $errors .= error_on_line( "unknown variable token <$tok>" );
            next;
        }
        push @variables, $_;
    }
}

foreach $i ( keys %required_tokens )
{
    $errors .= "$0: $fin: missing required token $i\n" if !$capture{ $i };
}

for ( $i = 0; $i < @templates; $i++ )
{
    $errors .= "$0: $fin: missing template file $templates[ $i ]\n" if !-e $templates[ $i ];
}

die $errors if $errors;
undef $errors;
print "required tokens, all templates found\n" if $debug;

# ---------- assemble basic substitution variables -----------

$prefix_name = $capture{ "prefix" } . $capture{ "name" };
$subs{ "___definename___"  }    = uc( $prefix_name );
$subs{ "___filename___"    }    = lc( $prefix_name );
$subs{ "___title___"       }    = $capture{ "title" };
$subs{ "___helpname___"    }    = $capture{ "helpprefix" } . $capture{ "name" };

{
    $classname = $capture{ "prefix" };
    my @n = split /_/, $capture{ "name" };
    for ( $i = 0; $i < @n; $i++ )
    {
        $classname .= uc( substr( $n[ $i ], 0, 1 ) ) . lc( substr( $n[ $i ], 1, length( $n[ $i ] ) - 1 ) ) . "_";
    }
    $classname =~ s/_$//;
}

$subs{ "___classname___"   } = $classname;

{
    $inputs = "";
    my $private;
    my $spaces = ' 'x( 6 + length( $subs{ "___classname___" } ) + 1 ) ;
    my $cpp_spaces = ' 'x( 3 + 2 * length( $subs{ "___classname___" } ) );
    $cpp_input_variables = "(\n";
    for ( $i = 0; $i < @inputs; $i++ )
    {
        $_ = $inputs[ $i ];
        s/^input\s+//;
        /^(.*)\s+(\w+)$/;
        my $type = $1;
        my $name = $2;
        if ( !length( $name ) )
        {
            $errors .= "$0: $fin: empty input variable name type $type\n";
            next;
        }
        if ( $input_variables_names{ $name } )
        {
            $errors .= "$0: $fin: duplicate input variable name $name\n";
            next;
        }
        # $existing_names{ $name }++;
        $input_variables{ $name }++;

        $input_variable_type{ $name } = $type;
        $private .=
            "      $type " . ' 'x( 39 - length( $type ) ) . "$name;\n";
        $inputs  .=
            $spaces . "$type " . ' 'x( 39 - length( $type ) ) . "$name,\n";
        $cpp_input_variables  .=
            $cpp_spaces . "$type " . ' 'x( 39 - length( $type ) ) . "$name,\n";
        $cpp_inputs .=
            "   this->$name " . ' 'x( 36 - length( $name ) ) . "= $name;\n";
    }
    $inputs .= 
        $spaces . "QWidget *                               p = 0,\n" .
        $spaces . "const char *                            name = 0\n" .
        $spaces . ");\n";

    $cpp_input_variables .= 
        $cpp_spaces . "QWidget *                               p,\n" .
        $cpp_spaces . "const char *                            name\n" .
        $cpp_spaces . ") : QFrame( p, name )\n";

    $subs{ "___private_end___" }       .= $private;
}        

$subs{ "___input_variables___"     } = $inputs;
$subs{ "___cpp_input_variables___" } = $cpp_input_variables;
$subs{ "___cpp_inputs___"          } = $cpp_inputs;

# --------- cpp specific -----------

$subs{ "___cpp_header___" } .= "#include \"../include/" . $subs{ "___filename___" } . ".h\"\n";


# ---------- assemble variables substitution variables -----------

{
    my $private;
    my $private_slots;
    for ( $i = 0; $i < @variables; $i++ )
    {
        $_ = $variables[ $i ];
        /^(\S+)/;
        my $tok = $1;
        s/^$tok\s*//;
        /^(\S+)/;
        my $type = $1;
        s/^$type\s*//;
        /^(\S+)/;
        my $format = $1;
        s/^$format\s*//;
        /^(\S+)/;
        my $min_val = $1;
        s/^$min_val\s*//;
        /^(\S+)/;
        my $max_val = $1;
        s/^$max_val\s*//;
        /^(\S+)/;
        my $name = $1;
        s/^$name\s*//;
        /^(\S+)/;
        my $short_desc = $1;
        s/^$short_desc\s*//;
        /^(\S+)/;
        my $desc = $_;

        $short_desc =~ /^\"(.*)\"$/;
        $short_desc = $1 if $1;

        $desc =~ /^\"(.*)\"$/;
        $desc = $1 if $1;

        print "variable $i tok <$tok> type <$type> format <$format> min <$min_val> max <$max_val> name <$name> short <$short_desc> long <$desc>\n" if $debug > 1;

        if ( !$known_variable_types{ $type } )
        {
            $errors .= "$0: $fin: unknown variable type $type for variable $name\n";
            next;
        }

        if ( $existing_names{ $name } )
        {
            $errors .= "$0: $fin: duplicate variable name $name\n";
            next;
        }
        $existing_names{ $name }++;

        if ( !$input_variables{ $name } )
        {
            $warnings .= "$0: $fin: WARNING: input variable not defined for variable name $name\n";
        }

        $variable_names        { $name }++;
        $variable_type         { $name } = $type;
        $variable_format       { $name } = $format;
        $variable_min          { $name } = $min;
        $variable_max          { $name } = $max;
        $variable_short_desc   { $name } = $short_desc;
        $variable_desc         { $name } = $desc;
    }
}

# ---------- base includes -----------

add_includes( "qlabel.h" );
add_includes( "qstring.h" );
add_includes( "qlayout.h" );

# ---------- assemble element substitution variables -----------

{
    my $private;
    my $private_slots;
    my $setup_gui;
    my $finalize_gui;

    $private .= 
        "      QLabel *                                lbl_title;\n";

    $setup_gui .= 
        "\n" .
        "   lbl_title =  new QLabel      ( tr( " . $capture{ "title" } . " ), this );\n" .
        "   lbl_title -> setFrameStyle   ( QFrame::WinPanel | QFrame::Raised );\n" .
        "   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );\n" .
        "   lbl_title -> setMinimumHeight( minHeight1 );\n" .
        "   lbl_title -> setPalette      ( QPalette( USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame ) );\n" .
        "   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );\n"
        ;

    $finalize_gui .= 
        "   background->addWidget( lbl_title );\n" 
        ;

    for ( $i = 0; $i < @credits; $i++ )
    {
        my $tag = "lbl_credits_" . ( $i + 1 );
        $private .= 
            "      QLabel *                                $tag;\n";

        $setup_gui .= 
            "\n" .
            "   $tag =  new QLabel      ( $credits[ i ], this );\n" .
            "   $tag -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );\n" .
            "   $tag -> setMinimumHeight( minHeight1 );\n" .
            "   $tag -> setPalette      ( QPalette( USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal ) );\n" .
            "   $tag -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold ) );\n"
            ;

        $finalize_gui .= 
            "   background->addWidget( $tag );\n" 
            ;
    }

    $finalize_gui .= 
        "   background->addSpacing( 4 );\n" 
        ;

#   all elements go in a hbl, that gets pushed along with a "row"

    $finalize_gui .= 
        "   QHBoxLayout * hbl = new QHBoxLayout( 0 );\n" .
        "   hbl->addSpacing( 4 );\n" 
        ;

    for ( $i = 0; $i < @elements; $i++ )
    {
        $_ = $elements[ $i ];
        /^(\S+)/;
        my $tok = $1;
        s/^$tok\s*//;
        /^(\S+)/;
        my $name = $1;
        s/^$name\s*//;
        my $desc = $_;
        if ( !$desc )
        {
            $desc = '"' . uc( substr( $name, 0, 1 )  ) . substr( $name, 1, length( $name ) - 1 ) . '"';
        }

        if ( $tok =~ /^row$/ )
        {
            $finalize_gui .= 
                "   hbl->addSpacing( 4 );\n" .
                "   background->addLayout( hbl );\n" .
                "   hbl = new QHBoxLayout( 0 );\n" .
                "   hbl->addSpacing( 4 );\n" 
                ;
        }
            

        if ( $name &&  $tok !~ /^variable$/ && $existing_names{ $name } )
        {
            $errors .= "$0: $fin: duplicate variable name $name\n";
            next;
        }
        $existing_names{ $name }++;

        print "for element $i tok <$tok> name <$name> desc <$desc>\n" if $debug > 1;

        if ( $tok =~ /^button/ )
        {
            if ( $existing_button{ $name } )
            {
                $errors .= "$0: $fin: duplicate element button $name\n";
                next;
            }
            $existing_button{ $name }++;
            add_includes( "qpushbutton.h" );

            my $button = "pb_$name";
            $private .= 
                "      QPushButton *                           $button;\n";
            $private_slots .= 
                "      void                                    $name();\n";

            $setup_gui .= 
                "\n" .
                "   $button =  new QPushButton ( tr( $desc ), this );\n" .
                "   $button -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );\n" .
                "   $button -> setMinimumHeight( minHeight1 );\n" .
                "   $button -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );\n" .
                "   connect( $button, SIGNAL( clicked() ), SLOT( $name() ) );\n" 
                ;
            $cpp_functions .= 
                "\n" .
                "void " . $subs{ "___classname___" } . "::$name()\n" .
                "{\n" .
                "}\n" 
                ;

            $finalize_gui .= 
                "   hbl->addWidget( $button );\n" 
                ;
        }

        if ( $tok =~ /^progress/ )
        {
            if ( $existing_progress{ $name } )
            {
                $errors .= "$0: $fin: duplicate element progress $name\n";
                next;
            }
            $existing_progress{ $name }++;
            add_includes( "qprogressbar.h" );

            $private .= 
                "      QProgressBar *                          $name;\n";

            $setup_gui .= 
                "\n" .
                "   $name =  new QProgressBar( this, \"$name\" );\n" .
                "   $name -> setMinimumHeight( minHeight1 );\n" .
                "   $name -> setPalette      ( QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal ) );\n" .
                "   $name -> reset           ();\n"
                ;

            $finalize_gui .= 
                "   hbl->addWidget( $name );\n" 
                ;
        }
             
        if ( $tok =~ /^editor/ )
        {
            if ( $existing_editor{ $name } )
            {
                $errors .= "$0: $fin: duplicate element editor $name\n";
                next;
            }
            $existing_editor{ $name }++;
            add_includes( "qprinter.h" );
            add_includes( "qframe.h" );
            add_includes( "qfont.h" );
            add_includes( "qmenubar.h" );
            add_includes( "qpopupmenu.h" );

            $private .= 
                "      QTextEdit *                             $name;\n" .
                "      QMenuBar *                              ${name}_menubar;\n";

            $private_slots .= 
                "      void                                    ${name}_clear_display();\n" .
                "      void                                    ${name}_update_font();\n" .
                "      void                                    ${name}_save();\n";

            $setup_gui .= 
                "\n" .
                "   $name =  new QTextEdit( this );\n" .
                "   $name -> setPalette( QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal ) );\n" .
                "   $name ->setReadOnly( true );\n" .
                "\n" .
                "   QFrame * ${name}_frame = new QFrame( this );\n" .
                "   ${name}_frame   -> setMinimumHeight( minHeight1 );\n" .
                "   ${name}_menubar =  new QMenuBar    ( ${name}_frame, \"menu\" );\n" .
                "   ${name}_menubar -> setMinimumHeight( minHeight1 - 5 );\n" .
                "   ${name}_menubar -> setPalette      ( QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal ) );\n" .
                "   QPopupMenu * ${name}_file = new QPopupMenu( $name );\n" .
                "   ${name}_menubar -> insertItem( tr(\"&File\"), ${name}_file );\n" .
                "   ${name}_file    -> insertItem( tr(\"Font\"),  this, SLOT( ${name}_update_font() ),    ALT+Key_F );\n" .
                "   ${name}_file    -> insertItem( tr(\"Save\"),  this, SLOT( ${name}_save() )       ,    ALT+Key_S );\n" .
                "   ${name}_file    -> insertItem( tr(\"Clear Display\"), this, SLOT( ${name}_clear_display() ),   ALT+Key_X );\n" .
                "   $name -> setWordWrap ( QTextEdit::WidgetWidth );\n" .
                "   $name -> setMinimumHeight(300);\n" 
                ;

            $finalize_gui .= 
                "   {\n" .
                "      QBoxLayout *vbl_editor_group = new QVBoxLayout( 0 );\n" .
                "      vbl_editor_group->addWidget( ${name}_frame );\n" .
                "      vbl_editor_group->addWidget( $name );\n" .
                "      hbl->addLayout( vbl_editor_group );\n" .
                "   }\n"
                ;
        }                

        if ( $tok =~ /^variable/ )
        {
            if ( !$variable_names{ $name } )
            {
                $errors .= "$0: $fin: unknown variable element reference $name\n";
                next;
            }

            add_includes( "qlineedit.h" );

            $private .= 
                "      QLabel *                                lbl_$name;\n" .
                "      QLineEdit *                             le_$name;\n";

            $private_slots .= 
                "      void                                    update_$name( const QString & );\n";

            $setup_gui .= 
                "\n" .
                "   lbl_${name} =  new QLabel      ( tr( \"" . $variable_desc{ $name } . "\" ), this );\n" .
                "   lbl_${name} -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );\n" .
                "   lbl_${name} -> setMinimumHeight( minHeight1 );\n" .
                "   lbl_${name} -> setPalette      ( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label ) );\n" .
                "   lbl_${name} -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold ) );\n" .
                "\n" .
                "   le_${name} = new QLineEdit     ( this, \"${name} Line Edit\" );\n" .
                "   le_${name} ->setText           ( QString( \"\" ).sprintf( \"%" . $variable_format{ $name } . "\", $name ) );\n" .
                "   le_${name} ->setAlignment      ( Qt::AlignCenter | Qt::AlignVCenter );\n" .
                "   le_${name} ->setPalette        ( QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal ) );\n" .
                "   le_${name} ->setFont           ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));\n" .
                "   le_${name} ->setMinimumHeight  ( minHeight1 );\n" .
                "   le_${name} ->setMinimumWidth   ( 80 );\n" .
                "   connect( le_${name}, SIGNAL( textChanged( const QString & ) ), SLOT( update_${name}( const QString & ) ) );\n" 
                ;

            my $totype;
            $totype = ".toDouble()" if $variable_type{ $name } =~ /^double$/;
            $totype = ".toInt()"    if $variable_type{ $name } =~ /^int$/;
            $totype = ".toUInt()"   if $variable_type{ $name } =~ /^uint$/;
            $cpp_functions .= 
                "\n" .
                "void " . $subs{ "___classname___" } . "::update_$name( const QString & )\n" .
                "{\n" .
                "   $name = le_${name}->text()${totype};\n" .
                "}\n" 
                ;
            $finalize_gui .= 
                "   hbl->addWidget( lbl_${name} );\n" .
                "   hbl->addWidget( le_${name} );\n" 
                ;
        }
    }

    $subs{ "___private___"       } .= $private;
    $subs{ "___private_slots___" } .= $private_slots;
    $subs{ "___setup_gui___"     } .= $setup_gui;
    $subs{ "___cpp_functions___" } .= $cpp_functions;
    $subs{ "___finalize_gui___"  } .= $finalize_gui;
}
    
# ---------- post processing of subs -----------

{
    my $inc;
    for ( $i = 0; $i < @includes; $i++ )
    {
        $inc .= "#include \"$includes[ $i ]\"\n";
    }
    $subs{ "___includes___" } = $inc;
}

die $errors if $errors;
undef $errors;
warn $warnings if $warnings;
undef $warnings;

if ( $debug )
{
    foreach $i ( keys %subs )
    {
        print "substitutions: <$i> : <$subs{$i}>\n";
    }
}

# ----------- process each template and make output ----------

for ( $i = 0; $i < @templates; $i++ )
{
    $ftin = $templates[ $i ];
    $f = $ftin;
    open IN, $f || err( "$f can not open $!" );

    $ftin =~ /(\.\w+)$/;
    $fout = "output/" . $subs{ "___filename___" } . $1;
    $f = $fout;
    open OUT, ">$f" || err( "can not open $f for writing $!" );
    
    while ( <IN> )
    {
        foreach $sub ( keys %subs )
        {
            s/$sub/$subs{$sub}/g;
        }
        print OUT $_;
    }
    close $OUT;
    print ">$fout\n";
}

    
