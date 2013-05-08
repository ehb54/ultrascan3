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
                      "_query_response",
                      "_custom_code",
                      "template",
                      "filehandler",
                      "required_control_parameters",
                      "required_variable",
                      "saveload",
                      "cmdline"
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
                        "template",
                        "filehandler",
                        "required_control_parameters",
                        "required_variable",
                        "saveload",
                        "cmdline"
                      );

for ( $i = 0; $i < @capture_token_array; $i++ )
{
    $capture_tokens{ $capture_token_array[ $i ] }++;
}

@capture_exactly_one_token_array = (
                                    "prefix",
                                    "name",
                                    "title",
                                    "saveload",
                                    "cmdline"
                                    );

for ( $i = 0; $i < @capture_exactly_one_token_array; $i++ )
{
    $capture_exactly_one_tokens{ $capture_exactly_one_token_array[ $i ] }++;
}

@required_token_array = (
                         "prefix",
                         "name",
                         "title",
#                         "credit",
                         "template"
                         );

for ( $i = 0; $i < @required_token_array; $i++ )
{
    $required_tokens{ $required_token_array[ $i ] }++;
}

@known_element_token_array = (
                              "editor",
                              "row",
                              "label",
                              "showhidelabel",
                              "endshowhidelabel",
                              "hide",
                              "variable",
                              "button",
                              "checkbox",
                              "progress"
                              );

for ( $i = 0; $i < @known_element_token_array; $i++ )
{
    $known_element_tokens{ $known_element_token_array[ $i ] }++;
}

@known_variable_token_array = (
                               "input", # handled in "input" or otherwise
                               "map"    # handled via input map
                               );

for ( $i = 0; $i < @known_variable_token_array; $i++ )
{
    $known_variable_tokens{ $known_variable_token_array[ $i ] }++;
}

@known_variable_type_array = (
                              "double",
                              "string",
                              "infile",
                              "int",
                              "uint"
                              );

for ( $i = 0; $i < @known_variable_type_array; $i++ )
{
    $known_variable_types{ $known_variable_type_array[ $i ] }++;
}

@known_query_response_token_array = (
                                     "q",
                                     "r",
                                     "e"
                                     );

for ( $i = 0; $i < @known_query_response_token_array; $i++ )
{
    $known_query_response_tokens{ $known_query_response_token_array[ $i ] }++;
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
    undef $in_custom_code  if $in_custom_code && /^\s*_end/;
    $in_custom_code = true if /^\s*_custom_code/;
    if ( !$in_custom_code )
    {
        s/^\s*//;
        s/\s*$//;
    }
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
        if ( $in_query_response )
        {
            $errors .= error_on_line( "can not define _query_response within _elements" );
            next;
        }
        if ( $in_custom_code )
        {
            $errors .= error_on_line( "can not define _query_response within _custom_code" );
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
        if ( $in_query_response )
        {
            $errors .= error_on_line( "can not define _query_response within _variables" );
            next;
        }
        if ( $in_custom_code )
        {
            $errors .= error_on_line( "can not define _query_response within _custom_code" );
            next;
        }
        $in_variables++;
        print "in variables\n" if $debug > 2;
        next;
    }

    if ( $tok =~ /^_query_response/ )
    { 
        if ( $in_elements )
        {
            $errors .= error_on_line( "can not define _elements within _query_response" );
            next;
        }
        if ( $in_variables )
        {
            $errors .= error_on_line( "can not define _variables within _query_response" );
            next;
        }
        if ( $in_custom_code )
        {
            $errors .= error_on_line( "can not define _variables within _custom_code" );
            next;
        }
        $in_query_response++;
        print "in query response\n" if $debug > 2;
        next;
    }

    if ( $tok =~ /^_custom_code/ )
    { 
        if ( $in_elements )
        {
            $errors .= error_on_line( "can not define _elements within _query_response" );
            next;
        }
        if ( $in_variables )
        {
            $errors .= error_on_line( "can not define _variables within _query_response" );
            next;
        }
        if ( $in_query_response )
        {
            $errors .= error_on_line( "can not define _query_response within _custom_code" );
            next;
        }
        $in_custom_code++;
        $arg =~ s/^\s+//;
        $arg =~ s/\s+$//;
        $last_custom_code_subs_name = $arg;
        print "in custom_code\n" if $debug > 2;
        if ( $arg !~ /^___\w+___$/ )
        {
            $errors .= error_on_line( "custom code name must be of format ___name___" );
            next;
        }
        next;
    }

    if ( $tok =~ /^_end/ )
    { 
        undef $in_elements;
        undef $in_variables;
        undef $in_query_response;
        undef $in_custom_code;
        print "out variables, elements, query_response or custom_code\n" if $debug > 2;
        next;
    }

    if ( 
         !$in_elements &&
         !$in_variables &&
         !$in_query_response &&
         !$in_custom_code &&
         !$valid_tokens{ $tok } )
    {
        $errors .= error_on_line( "unrecognized token $tok" );
        next;
    }
        
    if ( 
         !$in_elements &&
         !$in_variables &&
         !$in_query_response &&
         !$in_custom_code &&
         $capture_tokens{ $tok } )
    {
        if ( $capture_exactly_one{ $tok } &&
             $capture{ $tok } )
        {
            $errors .= error_on_line( "duplicate token $tok, only one allowed" );
        }
        $capture{ $tok } = $arg;

        push @templates                  , "templates/$arg" if $tok =~ /template/;
        push @credits                    , $arg             if $tok =~ /credit/;
        push @inputs                     , $arg             if $tok =~ /input/;
        push @required_control_parameters, $arg             if $tok =~ /required_control_parameters/;
        push @filehandler                , $arg             if $tok =~ /filehandler/;

        add_includes( $arg )                if $tok =~ /include/;
        if ( $tok =~ /required_variable/ )
        {
            my $line = $arg;
            $line =~ /^(\S+)\s*(.*)$/;
            $required_variable{ $1 }++;
            $required_variable_default{ $1 } = $2;
        }
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

    if ( $in_query_response )
    {
        if ( !$known_query_response_tokens{ $tok } )
        {
            $errors .= error_on_line( "unknown query response token <$tok>" );
            next;
        }
        if ( $tok =~ /q/ )
        {
            push @query, $arg;
        }
        if ( $tok =~ /r/ )
        {
            if ( $arg !~ /^___\w+___$/ )
            {
                $errors .= error_on_line( "invalid query response response format <$arg>" );
                next;
            }
                
            $response[ $#query ] = $arg;
        }
        if ( $tok =~ /e/ )
        {
            $query_response_enum[ $#query ] = $arg;
        }
    }

    if ( $in_custom_code )
    {
        $subs{ $last_custom_code_name } .= $source[ $i ] . "\n";
    }
}

if (
    $in_elements ||
    $in_variables ||
    $in_query_response ||
    $in_custom_code 
    )
{
    $errors .= "$0: $fin: unterminated elements, variables, query_response or custom_code\n";
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
        $cpp_spaces . ")";

    $subs{ "___private_end___" }       .= $private;
}        

$subs{ "___input_variables___"     } = $inputs;
$subs{ "___cpp_input_variables___" } = $cpp_input_variables;
$subs{ "___cpp_inputs___"          } = $cpp_inputs;

# --------- cpp specific -----------

$subs{ "___cpp_header___" } .= "#include \"../include/" . $subs{ "___filename___" } . ".h\"\n";

# --------- saveload ---------

if ( $capture{ "saveload" } )
{
    my $private;
    $private .= "      void" . ' 'x36 . "update_fields();\n";

    $subs{ "___private_end___" } .= $private;
}

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

        $maximum_desc_length = length( $desc ) if maximum_desc_length < length( $desc );

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

        if ( $tok =~ /^input$/ && !$input_variables{ $name } )
        {
            $warnings .= "$0: $fin: WARNING: input variable not defined for variable name $name\n";
        }

        $variable_names        { $name }++;
        $variable_method       { $name } = $tok;
        $variable_type         { $name } = $type;
        $variable_format       { $name } = $format;
        $variable_min          { $name } = $min;
        $variable_max          { $name } = $max;
        $variable_short_desc   { $name } = $short_desc;
        $variable_desc         { $name } = $desc;
        {
            my @l = split /\\n/, $desc;
            $variable_desc_lines   { $name } = scalar( @l );
        }
    }
    $maximum_desc_length = int( .6 * $maximum_desc_length );
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
    my $cpp_functions;
    my $cpp_functions_update_fields;
    my $cpp_clean_parameters;
    my $used_show_hide;
    my %show_hide_active;

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
        my $default = $desc;
        $default =~ s/\s+(.*|)$//;

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

        if ( $name &&  $tok =~ /^(endshowhidelabel|hide)$/ && !$existing_names{ $name } )
        {
            $errors .= "$0: $fin: $tok must be after showhidelabel $name\n";
            next;
        }

        if ( $name &&  $tok !~ /^(variable|checkbox|endshowhidelabel|hide)$/ && $existing_names{ $name } )
        {
            $errors .= "$0: $fin: duplicate variable name $name\n";
            next;
        }
        $existing_names{ $name }++;

        print "for element $i tok <$tok> name <$name> desc <$desc>\n" if $debug > 1;

        if ( $tok =~ /^label$/ )
        {
            add_includes( "qlabel.h" );
            add_includes( "qwidget.h" );
            add_includes( "vector" );

            my $label = "lbl_$name";
            $private .= 
                "      QLabel *                                $label;\n" .
                "";

            $setup_gui .= 
                "\n" .
                "   $label =  new QLabel      ( tr( $desc ), this );\n" .
                "   $label -> setFrameStyle   ( QFrame::WinPanel | QFrame::Raised );\n" .
                "   $label -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );\n" .
                "   $label -> setMinimumHeight( minHeight1 );\n" .
                "   $label -> setPalette      ( QPalette( USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame ) );\n" .
                "   $label -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );\n" .
                "\n";
                ;

            {
                my $key;
                foreach $key ( keys %show_hide_active )
                {
                    $setup_gui .= "   widgets_${key}.push_back( $label );\n";
                }
            }

            $finalize_gui .= 
                "   hbl->addWidget( $label );\n" 
                ;
        }

        if ( $tok =~ /^showhidelabel$/ )
        {
            add_includes( "qlabel.h" );
            add_includes( "us_mqlabel.h" );

            my $label = "lbl_$name";
            $private .= 
                "      QLabel *                                $label;\n" .
                "      vector < QWidget * >                    widgets_$name;\n" .
                "";

            $setup_gui .= 
                "\n" .
                "   $label =  new mQLabel     ( tr( $desc ), this );\n" .
                "   $label -> setFrameStyle   ( QFrame::WinPanel | QFrame::Raised );\n" .
                "   $label -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );\n" .
                "   $label -> setMinimumHeight( minHeight1 );\n" .
                "   $label -> setPalette      ( QPalette( USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame ) );\n" .
                "   $label -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );\n" .
                "   connect( $label, SIGNAL( pressed() ), SLOT( hide_$name() ) );\n" .
                "\n";
                ;

            {
                my $key;
                foreach $key ( keys %show_hide_active )
                {
                    $setup_gui .= "   widgets_${key}.push_back( $label );\n";
                }
            }

            $finalize_gui .= 
                "   hbl->addWidget( $label );\n" 
                ;

            $private_slots .=
                "      void                                    hide_$name();\n";

            $cpp_functions .= <<__END
void $subs{ "___classname___" }::hide_$name()
{
   hide_widgets( widgets_$name, widgets_${name}.size() && widgets_${name}[ 0 ]->isVisible() );
}
__END
            ;

            if ( !$used_show_hide )
            {
                $used_show_hide = true;

                $private .= 
                    "      void                                    hide_widgets( vector < QWidget * >, bool do_hide = true, bool do_resize = true );\n" .
                    "";

                $cpp_functions .= <<__END

void $subs{ "___classname___" }::hide_widgets( vector < QWidget * > w, bool do_hide, bool do_resize )
{
   for ( unsigned int i = 0; i < ( unsigned int )w.size(); i++ )
   {
       do_hide ? w[ i ]->hide() : w[ i ]->show();
   }
   if ( do_resize )
   {
       qApp->processEvents();
       resize( 0, 0 );
   }
}
__END
                ;
            }                
            $show_hide_active{ $name } = true;
        }

        if ( $tok =~ /^endshowhidelabel$/ )
        {
            delete( $show_hide_active{ $name } );
        }

        if ( $tok =~ /^hide$/ )
        {
            $finalize_gui .= 
                "   hide_widgets( widgets_$name, true, false );\n"
                ;
        }

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

            {
                my $key;
                foreach $key ( keys %show_hide_active )
                {
                    $setup_gui .= "   widgets_${key}.push_back( $button );\n";
                }
            }

            my $function_code;

            if ( $name =~ /^save$/ && $capture{ "saveload" } )
            {
                my $l = $capture{ "saveload" };
                $l =~ /^(\w+)\s*(\S.*)$/;
                my $arg1 = $1;
                $l = $2;
                $l =~ /^(\w+)\s*(\S.*)$/;
                my $arg2 = $1;
                my $arg3 = $2;
 
                if ( $arg1 =~ /^json$/ )
                {
                    add_includes( "qfiledialog.h" );
                    add_includes( "qmessagebox.h" );
                    add_includes( "qregexp.h" );
                    add_includes( "us_json.h" );
                    $function_code .= <<__END
   QString fn = QFileDialog::getSaveFileName( 
                                              QString::null, 
                                              "*.$arg2",
                                              this,
                                              tr( QString( "%1: Save" ).arg( $capture{ "title" } ) ),
                                              tr( "Save the parameters" ) 
                                              );

   if( !fn.isEmpty() )
   {
      if ( !fn.contains( QRegExp( "\\\\.$arg2\$" ) ) )
      {
         fn += ".$arg2";
      }
      QFile f( fn );
      if ( !f.open( IO_WriteOnly ) )
      {
         QMessageBox::information( this,
                                   tr( QString( "%1: Save" ).arg( $capture{ "title" } ) ),
                                   QString( tr( "Could not open file %1 for writing" ) )
                                   .arg( fn ) 
                                   );
         return;
      }
      QTextStream ts( &f );
      ts << US_Json::compose( $arg3 );
      f.close();
   }
__END
                          ;
                }
            } 

            if ( $name =~ /^load$/ && $capture{ "saveload" } )
            {
                my $l = $capture{ "saveload" };
                $l =~ /^(\w+)\s*(\S.*)$/;
                my $arg1 = $1;
                $l = $2;
                $l =~ /^(\w+)\s*(\S.*)$/;
                my $arg2 = $1;
                my $arg3 = $2;
 
                if ( $arg1 =~ /^json$/ )
                {
                    add_includes( "qfiledialog.h" );
                    add_includes( "qmessagebox.h" );
                    add_includes( "us_json.h" );
                    $function_code .= <<__END
   QString fn = QFileDialog::getOpenFileName( 
                                              QString::null, 
                                              "*.$arg2",
                                              this,
                                              tr( QString( "%1: Open" ).arg( $capture{ "title" } ) ),
                                              tr( "Load parameters" ) 
                                              );
   if( !fn.isEmpty() )
   {
      QFile f( fn );
      if ( !f.open( IO_ReadOnly ) )
      {
          QMessageBox::information( 
                                    this,
                                    tr( QString( "%1: Open" ).arg( $capture{ "title" } ) ),
                                    QString( tr( "Could not open file %1 for reading" ) )
                                    .arg( fn ) 
                                    );
          return;
      }
      QString qs;
      QTextStream ts( &f );
      while ( !ts.atEnd() )
      {
          qs += ts.readLine();
      }
      f.close();
      $arg3 = US_Json::split( qs );
      update_fields();
   }
__END
                          ;
                }
            } 

            $cpp_functions .= 
                "\n" .
                "void " . $subs{ "___classname___" } . "::$name()\n" .
                "{\n" . $function_code .
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

            {
                my $key;
                foreach $key ( keys %show_hide_active )
                {
                    $setup_gui .= "   widgets_${key}.push_back( $name );\n";
                }
            }

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


            {
                my $key;
                foreach $key ( keys %show_hide_active )
                {
                    $setup_gui .=
                        "   widgets_${key}.push_back( $name );\n" .
                        "   widgets_${key}.push_back( ${name}_menubar );\n" .
                        "   widgets_${key}.push_back( ${name}_file );\n" .
                            "";
                }
            }

            $finalize_gui .= 
                "   {\n" .
                "      QBoxLayout *vbl_editor_group = new QVBoxLayout( 0 );\n" .
                "      vbl_editor_group->addWidget( ${name}_frame );\n" .
                "      vbl_editor_group->addWidget( $name );\n" .
                "      hbl->addLayout( vbl_editor_group );\n" .
                "   }\n"
                ;
        }                

        if ( $tok =~ /^variable$/ )
        {
            if ( !$variable_names{ $name } )
            {
                $errors .= "$0: $fin: unknown variable element reference $name\n";
                next;
            }

            add_includes( "qlineedit.h" );
            add_includes( "qfontmetrics.h" );

            $private .= 
                "      QLabel *                                lbl_$name;\n" .
                "      QLineEdit *                             le_$name;\n";

            $private_slots .= 
                "      void                                    update_$name( const QString & );\n";

            $setup_gui .= 
                "\n" .
                "   lbl_${name} = new QLabel      ( tr( \"" . $variable_desc{ $name } . "\" ), this );\n" .
                "   lbl_${name} ->setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );\n" .
                "   lbl_${name} ->setMinimumHeight( minHeight1 " . ( $variable_desc_lines{ $name } > 1 ? "*  $variable_desc_lines{ $name } " : "" ) . ");\n" .
                "   lbl_${name} ->setPalette      ( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label ) );\n" .
                "   lbl_${name} ->setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold ) );\n" .
                "   lbl_${name} ->setMinimumWidth ( QFontMetrics( lbl_${name}->font() ).maxWidth() * $maximum_desc_length );\n" .
                "\n" .
                "   le_${name} = new QLineEdit     ( this, \"${name} Line Edit\" );\n";

            {
                my $key;
                foreach $key ( keys %show_hide_active )
                {
                    $setup_gui .= 
                        "   widgets_${key}.push_back( lbl_${name} );\n" .
                        "   widgets_${key}.push_back( le_${name} );\n" .
                        "";
                }
            }

            if ( $variable_method{ $name } =~ /^input$/ )
            {
                $setup_gui .= 
                    "   le_${name} ->setText           ( QString( \"\" ).sprintf( \"%" . $variable_format{ $name } . "\", $name ) );\n"
                    ;
            }

            if ( $variable_method{ $name } =~ /^map$/ )
            {
                if ( $capture{ "saveload" } )
                {
                    if ( $variable_type{ $name } =~ /^infile$/ )
                    {
                        $cpp_functions_update_fields .= 
                            "   disconnect( le_${name}, SIGNAL( textChanged( const QString & ) ), 0, 0 );\n";
                    }
                    $cpp_functions_update_fields .= 
                    "   le_${name}" . ( ' 'x( 45 - length($name) ) ) . "->setText( parameters->count( \"$name\" ) ? ( *parameters )[ \"$name\" ] : \"\" );\n" 
                        ;
                    if ( $variable_type{ $name } =~ /^infile$/ )
                    {
                        $cpp_functions_update_fields .= 
                            "   connect( le_${name}, SIGNAL( textChanged( const QString & ) ), SLOT( update_${name}( const QString & ) ) );\n";
                    }
                }
                if ( $required_variable{ $name } )
                {
                    $required_variable_used{ $name }++;
                    $setup_gui .= <<__END
   if ( !parameters->count( "$name" ) )
   {
      (*parameters)[ "$name" ] = "$required_variable_default{ $name }";
   }
   le_${name} ->setText           ( ( *parameters )[ "$name" ] );
__END
                        ;
                 } else {
                    $setup_gui .= 
                    "   le_${name} ->setText           ( parameters->count( \"$name\" ) ? ( *parameters )[ \"$name\" ] : \"\" );\n" 
                        ;

                    $cpp_clean_parameters .= <<__END
   if ( parameters->count( "$name" ) &&
        (*parameters)[ "$name" ].isEmpty() )
   {
      parameters->erase( "$name" );
   }
__END
                   ;
                }
            }

            $setup_gui .= 
                "   le_${name} ->setAlignment      ( Qt::AlignCenter | Qt::AlignVCenter );\n" .
                "   le_${name} ->setPalette        ( QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal ) );\n" .
                "   le_${name} ->setFont           ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));\n" .
                "   le_${name} ->setMinimumHeight  ( minHeight1 " . ( $variable_desc_lines{ $name } > 1 ? "*  $variable_desc_lines{ $name } " : "" ) . ");\n" .
                "   le_${name} ->setMinimumWidth   ( 150 );\n" .
                "   connect( le_${name}, SIGNAL( textChanged( const QString & ) ), SLOT( update_${name}( const QString & ) ) );\n" 
                ;

            $cpp_functions .= 
                "\n" .
                "void " . $subs{ "___classname___" } . "::update_$name( const QString & )\n" .
                "{\n"
                ;
            if ( $variable_type{ $name } =~ /^infile$/ )
            {
                add_includes( "qfile.h"       );
                add_includes( "qfiledialog.h" );
                $cpp_functions .= <<__END
   QString filename = QFileDialog::getOpenFileName(
                                                   QString::null,
                                                   QString::null,
                                                   this,
                                                   "open file dialog",
                                                   tr( "Select a file for $variable_desc{ $name }" )
                                                   );

   disconnect( le_${name}, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_${name}->setText( filename );
   connect( le_${name}, SIGNAL( textChanged( const QString & ) ), SLOT( update_${name}( const QString & ) ) );
__END
               ;
            }
                
            if ( $variable_method{ $name } =~ /^input$/ )
            {
                my $totype;
                $totype = ".toDouble()" if $variable_type{ $name } =~ /^double$/;
                $totype = ".toInt()"    if $variable_type{ $name } =~ /^int$/;
                $totype = ".toUInt()"   if $variable_type{ $name } =~ /^uint$/;
                $cpp_functions .= 
                    "   $name = le_${name}->text()${totype};\n"
                    ;
            }
            if ( $variable_method{ $name } =~ /^map$/ )
            {
                $cpp_functions .= 
                    "   ( *parameters )[ \"$name\" ] = le_${name}->text();\n"
                    ;
            }

            $cpp_functions .= 
                "}\n" 
                ;
            $finalize_gui .= 
                "   hbl->addWidget( lbl_${name} );\n" .
                "   hbl->addWidget( le_${name} );\n" 
                ;
        }

        if ( $tok =~ /^checkbox$/ )
        {
            if ( !$variable_names{ $name } )
            {
                $errors .= "$0: $fin: unknown checkbox element reference $name\n";
                next;
            }

            add_includes( "qcheckbox.h" );

            $private .= 
                "      QCheckBox *                             cb_$name;\n";

            $private_slots .= 
                "      void                                    set_$name();\n";

            $setup_gui .= 
                "\n" .
                "   cb_${name} = new QCheckBox    ( tr( \"" . $variable_desc{ $name } . "\" ), this );\n" .
                "   cb_${name} ->setMinimumHeight ( minHeight1 " . ( $variable_desc_lines{ $name } > 1 ? "*  $variable_desc_lines{ $name } " : "" ) . ");\n" .
                "   cb_${name} ->setPalette       ( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal ) );\n" .
                "   cb_${name} ->setFont          ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold ) );\n" .
                "   cb_${name} ->setMinimumWidth  ( QFontMetrics( cb_${name}->font() ).maxWidth() * $maximum_desc_length );\n" .
                "\n";

            {
                my $key;
                foreach $key ( keys %show_hide_active )
                {
                    $setup_gui .= "   widgets_${key}.push_back( cb_${name} );\n";
                }
            }

            if ( $variable_method{ $name } =~ /^input$/ )
            {
                $errors .= "$0: $fin: input method not supported for checkbox, use map\n";
                next;
            }

            if ( $variable_method{ $name } =~ /^map$/ )
            {
                if ( $capture{ "saveload" } )
                {
                    $cpp_functions_update_fields .= 
                    "   cb_${name}" . ( ' 'x( 45 - length($name) ) ) . "->setChecked( parameters->count( \"$name\" ) && ( *parameters )[ \"$name\" ] == \"true\" ? true : false );\n"
                        ;
                }

                if ( $required_variable{ $name } )
                {
                    $required_variable_used{ $name }++;
                    $setup_gui .= <<__END
   if ( !parameters->count( "$name" ) )
   {
      (*parameters)[ "$name" ] = "$required_variable_default{ $name }";
   }
   cb_${name} ->setChecked        ( parameters->count( "$name" ) && ( *parameters )[ "$name" ] == \"true\" ? true : false );
__END
                        ;
                 } else {
                     if ( $default eq "true" )
                     {
                         $setup_gui .= 
                             "   if ( !parameters->count( \"$name\" ) )\n" .
                             "   {\n" .
                             "      ( *parameters )[ \"$name\" ] = \"true\";\n" .
                             "   }\n" .
                             "";
                     } 
                     $setup_gui .= 
                         "   cb_${name} ->setChecked        ( parameters->count( \"$name\" ) && ( *parameters )[ \"$name\" ] == \"true\" ? true : false );\n" 
                         ;

                     if ( $default ne "true" )
                     {
                         $cpp_clean_parameters .= <<__END
   if ( parameters->count( "$name" ) &&
        ( (*parameters)[ "$name" ].isEmpty() ||
          (*parameters)[ "$name" ] == \"false\" ) )
   {
      parameters->erase( "$name" );
   }
__END
                         ;
                     }
                }
            }

            $setup_gui .= 
                "   connect( cb_${name}, SIGNAL( clicked() ), SLOT( set_${name}() ) );\n"
                ;

            $cpp_functions .= 
                "\n" .
                "void " . $subs{ "___classname___" } . "::set_$name()\n" .
                "{\n"
                ;
            if ( $variable_method{ $name } =~ /^input$/ )
            {
#                my $totype;
#                $totype = ".toDouble()" if $variable_type{ $name } =~ /^double$/;
#                $totype = ".toInt()"    if $variable_type{ $name } =~ /^int$/;
#                $totype = ".toUInt()"   if $variable_type{ $name } =~ /^uint$/;
#                $cpp_functions .= 
#                    "   $name = le_${name}->text()${totype};\n"
#                    ;
            }

            if ( $variable_method{ $name } =~ /^map$/ )
            {
                $cpp_functions .= 
                    "   ( *parameters )[ \"$name\" ] = cb_${name}->isChecked() ? \"true\" : \"false\";\n"
                    ;
            }

            $cpp_functions .= 
                "}\n" 
                ;
            $finalize_gui .= 
                "   hbl->addWidget( cb_${name} );\n" 
                ;
        }
    }

    $subs{ "___private___"               } .= $private;
    $subs{ "___private_slots___"         } .= $private_slots;
    $subs{ "___setup_gui___"             } .= $setup_gui;
    $subs{ "___cpp_functions___"         } .= $cpp_functions;
    $subs{ "___cpp_clean_parameters___"  } .= $cpp_clean_parameters;
    $subs{ "___finalize_gui___"          } .= $finalize_gui;

    if ( $capture{ "saveload" } )
    {
        my $cpp_functions_add =
                "\nvoid " . $subs{ "___classname___" } . "::update_fields()\n" .
                "{\n" . $cpp_functions_update_fields .
                "}\n" ;

        $subs{ "___cpp_functions___" } .= $cpp_functions_add;
    }
}
    

foreach $i ( keys %required_variable )
{
    $errors .= "$0: error: required variable $i not used\n"       if !$required_variable_used{ $i };
    $errors .= "$0: error: required variable $i not a variable\n" if !$variable_names        { $i };
}

# ---------- filehandler -----------

{
    my @valid_token_array = (
                             "rename_length6",
                             "rename_length8",
                             "append",
                             "basename",
                             "rename_unique_and_save"
                             );
    my %valid_tokens;
    for ( $i = 0; $i < @valid_token_array; $i++ )
    {
        $valid_tokens{ $valid_token_array[ $i ] }++;
    }

    for ( $i = 0; $i < @filehandler; $i++ )
    {
        my @tokens = split /\s+/, $filehandler[ $i ];
        if ( @tokens < 2 )
        {
            $errors .= "$0: error: filehandler insufficient tokens: $filehandler[ $i ]\n";
            next;
        }

        if ( !$valid_tokens{ $tokens[ 1 ] } )
        {
            $errors .= "$0: error: filehandler invalid token: $tokens[ 1 ]\n";
            next;
        }

        if ( $tokens[ 0 ] !~ /^___\w+___$/ )
        {
            $errors .= "$0: error: filehandler invalid section format: $tokens[ 0 ]\n";
            next;
        }

# code to handle various tokens
        if ( $tokens[ 1 ] =~ /^rename_length6$/ )
        {
            if ( @tokens < 4 )
            {
                $errors .= "$0: error: filehandler insufficient tokens: $filehandler[ $i ]\n";
                next;
            }
            $subs{ $tokens[ 0 ] } .= <<__END
   if ( control_parameters.count( "$tokens[ 2 ]" ) )
   {
      QFileInfo fi( control_parameters[ "$tokens[ 2 ]" ] );
      QString baseName = fi.baseName ( true  );
      if ( baseName.length() > 6 )
      {
          QString ext      = fi.extension( false );
          int     use_len  = 6;
          QString destname = baseName.left( use_len );
          int     copy     = 0;
          while ( QFile::exists( destname ) )
          {
             QString copy_str = QString( "%1" ).arg( copy );
             if ( copy_str.length() > 5 )
             {
                errormsg = 
                    QString( "Error: $capture{ "name" }: too many attempts to create unique 6 character name for %1" )
                    .arg( control_parameters[ "$tokens[ 2 ]" ] );
                return false;
             }
             use_len = 6 - copy_str.length();
             destname = baseName.left( use_len ) + QString( "%1" ).arg( ++copy );
          }
          control_parameters[ "$tokens[ 3 ]" ] = 
              baseName.left( 6 ) + "." + ext;
      } else {
          control_parameters[ "$tokens[ 3 ]" ] = 
             control_parameters[ "$tokens[ 2 ]" ];
      }
   } else {
      errormsg = "Error: $capture{ "name" }: parameter $tokens[ 2 ] must be defined";
      return false;
   }
__END
            ;
        }

        if ( $tokens[ 1 ] =~ /^rename_length8$/ )
        {
            if ( @tokens < 4 )
            {
                $errors .= "$0: error: filehandler insufficient tokens: $filehandler[ $i ]\n";
                next;
            }
            $subs{ $tokens[ 0 ] } .= <<__END
   if ( control_parameters.count( "$tokens[ 2 ]" ) )
   {
      QFileInfo fi( control_parameters[ "$tokens[ 2 ]" ] );
      QString baseName = fi.baseName ( true  );
      if ( baseName.length() > 8 )
      {
          QString ext      = fi.extension( false );
          int     use_len  = 8;
          QString destname = baseName.left( use_len );
          int     copy     = 0;
          while ( QFile::exists( destname ) )
          {
             QString copy_str = QString( "%1" ).arg( copy );
             if ( copy_str.length() > 7 )
             {
                errormsg = 
                    QString( "Error: $capture{ "name" }: too many attempts to create unique 8 character name for %1" )
                    .arg( control_parameters[ "$tokens[ 2 ]" ] );
                return false;
             }
             use_len = 8 - copy_str.length();
             destname = baseName.left( use_len ) + QString( "%1" ).arg( ++copy );
          }
          control_parameters[ "$tokens[ 3 ]" ] = 
              baseName.left( 8 ) + "." + ext;
      } else {
          control_parameters[ "$tokens[ 3 ]" ] = 
             control_parameters[ "$tokens[ 2 ]" ];
      }
   } else {
      errormsg = "Error: $capture{ "name" }: parameter $tokens[ 2 ] must be defined";
      return false;
   }
__END
            ;
        }


        if ( $tokens[ 1 ] =~ /^basename$/ )
        {
            if ( @tokens < 4 )
            {
                $errors .= "$0: error: filehandler insufficient tokens: $filehandler[ $i ]\n";
                next;
            }
            $subs{ $tokens[ 0 ] } .= <<__END
   if ( control_parameters.count( "$tokens[ 2 ]" ) )
   {
      QFileInfo fi( control_parameters[ "$tokens[ 2 ]" ] );
      control_parameters[ "$tokens[ 3 ]" ] = fi.baseName( true );
   } else {
      errormsg = "Error: $capture{ "name" }: parameter $tokens[ 2 ] must be defined";
      return false;
   }
__END
            ;
        }
        if ( $tokens[ 1 ] =~ /^append$/ )
        {
            if ( @tokens < 5 )
            {
                $errors .= "$0: error: filehandler insufficient tokens: $filehandler[ $i ]\n";
                next;
            }
            $subs{ $tokens[ 0 ] } .= <<__END
   if ( control_parameters.count( "$tokens[ 3 ]" ) )
   {
      control_parameters[ "$tokens[ 4 ]" ] = 
          control_parameters[ "$tokens[ 3 ]" ] + "$tokens[ 2 ]";
   }
__END
            ;
        }
        if ( $tokens[ 1 ] =~ /^rename_unique_and_save$/ )
        {
            if ( @tokens < 4 )
            {
                $errors .= "$0: error: filehandler insufficient tokens: $filehandler[ $i ]\n";
                next;
            }
            $subs{ $tokens[ 0 ] } .= <<__END
   if ( control_parameters.count( "$tokens[ 2 ]" ) &&
        control_parameters.count( "$tokens[ 3 ]" ) )
   {
      QFileInfo fi( control_parameters[ "$tokens[ 2 ]" ] );
      QString baseName = fi.baseName ( true  );
      QString ext      = fi.extension( false );

      if ( !fi.exists() )
      {
          errormsg = QString( "Error: $capture{ "name" }: the expected output file %1 does not exist" )
              .arg( control_parameters[ "$tokens[ 2 ]" ] );
          return false;
      }

      int copy = 0;
      QString destfile = control_parameters[ "$tokens[ 3 ]" ] + "." + ext;
      while ( QFile::exists( destfile ) )
      {
          destfile = control_parameters[ "$tokens[ 3 ]" ] + QString( "_%1" ).arg( ++copy ) + "." + ext;
      }
      QDir qd;
      if ( !qd.rename( control_parameters[ "$tokens[ 2 ]" ], destfile ) )
      {
          errormsg = QString( "Error renaming %1 to %2" )
              .arg( control_parameters[ "$tokens[ 2 ]" ] )
              .arg( destfile );
          return false;
      }
      output_files << destfile;
   }
__END
            ;
        }
    }
}       

# ---------- query response variables -----------

{
    $subs{ "___cmdline_run_application___"            }  = "run_" . $capture{ "name" } . "()";

    my $cmdline_interface_init;
    my $cmdline_interface_post_processing;
    my $runcmd = $capture{ "name" } . "run";
    my $multiple_rx_insert =
        ' 'x22 . "\"$runcmd|\"\n";
    my %inserted;
    $inserted{ $runcmd }++;

    my $multiple_run_insert = <<__END
      if ( option == "$runcmd" )
      {
         if ( !$subs{ "___cmdline_run_application___" } )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
      }         
__END
    ;
        
    my $multiple_rx_file_insert;
    my %inserted_file;
    for ( $i = 0; $i < @required_control_parameters; $i++ )
    {
        $rs = $required_control_parameters[ $i ];
        $rsl = lc( $rs );
        $multiple_rx_insert .=
            ' 'x20 . "\"$rsl|\"\n" if !$inserted{ $rs };
        $inserted{ $rs }++;

        $subs{ "___cmdline_interface_init___" } .= <<__END
   if ( !control_parameters.count( "$rsl" ) )
   {
      errormsg = 
         "Error: $capture{ "name" }: required parameter $rsl not defined";
      return false;
   }
__END
        ;

        if ( $rs =~ /file/ )
        {
            $multiple_rx_file_insert .=
                ' 'x20 . "\"$rsl|\"\n" if !$inserted_file{ $rs };
            $inserted_file{ $rs }++;
        }
    }

    for ( $i = 0; $i < @query; $i++ )
    {
        $q = $query   [ $i ];
        $r = $response[ $i ];
        $r =~ /^___(\w+)___$/;
        $rs = $1;
        $rsl = lc( $rs );

        if ( $runcmd eq $rs )
        {
            $errors .= "Error: run command $runcmd duplicated in variable list\n";
        }

        if ( $r ne "___run___" )
        {
            $multiple_rx_insert .=
                ' 'x20 . "\"$rsl|\"\n" if !$inserted{ $rs };
            $inserted{ $rs }++;

            if ( $rs =~ /file/ )
            {
                $multiple_rx_file_insert .=
                    ' 'x20 . "\"$rsl|\"\n" if !$inserted_file{ $rs };
                $inserted_file{ $rs }++;
            }

            $cmdline_interface_init .= <<__END
   if ( control_parameters.count( "$rsl" ) )
   {
      app_text << "$q";
      response << control_parameters[ "$rsl" ];
   }
__END
            ;
        } else {
            $cmdline_interface_init .= <<__END
   app_text << "$q";
   response << "___run___";
__END
            ;
        }   
    }

    my @cmds = split /\s+/, $capture{ "cmdline" };
    for ( $i = 0; $i < @cmds; $i++ )
    {
        $cmdline_interface_init .= 
            "   args     << \"$cmds[$i]\";\n";
    }

    $subs{ "___cmdline_interface_init___"             } .= $cmdline_interface_init;
    $subs{ "___cmdline_interface_post_processing___"  } .= $cmdline_interface_post_processing;
    $subs{ "___multiple_rx_insert___"                 } .= $multiple_rx_insert;
    $subs{ "___multiple_rx_file_insert___"            } .= $multiple_rx_file_insert;
    $subs{ "___multiple_run_insert___"                } .= $multiple_run_insert;
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
