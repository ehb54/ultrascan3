#!/usr/bin/perl

sub parsegc {
    open(GCF, $_[0]);
    my @gcf = <GCF>;
    close(GCF);
    grep(chomp, @gcf);
    my $i = 0;
    $analysis_type = $gcf[$i]; $i++;
    $db_login_user = $gcf[$i]; $i++;
    $db_login_password = $gcf[$i]; $i++;
    $db_login_database = $gcf[$i]; $i++;
    $db_login_host = $gcf[$i]; $i++;
    $no_experiments = $gcf[$i]; $i++;
    undef @exp_file_info;
    undef @exp_dir; 
    undef @exp_cell;
    undef @exp_lambda;
    undef @exp_channel;
    for(my $j = 0; $j < $no_experiments; $j++) {
	$exp_file_info[$j] = $gcf[$i]; $i++;
	$exp_dir[$j] = $gcf[$i]; $i++;
	$exp_cell[$j] = $gcf[$i]; $i++;
	$exp_lambda[$j] = $gcf[$i]; $i++;
	$exp_channel[$j] = $gcf[$i]; $i++;
    }
    if($analysis_type eq 'GA_SC') {
	$ga_sc_constraint_file = $gcf[$i]; $i++;
    }
    $exp_file_infos = join ', ', @exp_file_info;
    $exp_dirs = join ', ', @exp_dir;
    $exp_cells = join ', ', @exp_cell;
    $exp_lambdas = join ', ', @exp_lambda;
    $exp_channels = join ', ', @exp_channel;
    undef @ga_s;
    undef @ga_s_min;
    undef @ga_s_max; 
    undef @ga_ff0;
    undef @ga_ff0_min;
    undef @ga_ff0_max;
    undef $ga_params_demes;
    undef $ga_params_generations;
    undef $ga_params_crossover;
    undef $ga_params_mutation;
    undef $ga_params_plague;
    undef $ga_Params_elitism;
    undef $ga_params_migration_rate;
    undef $ga_params_genes;
    undef $ga_params_initial_solutes;
    undef $ga_params_random_seed;
    undef $ga_params_monte_carlo;
    undef $ga_params_regularization;
    undef $ga_ss;
    undef $ga_s_mins;
    undef $ga_smaxs;
    undef $ga_ff0s;
    undef $ga_ff0_mins;
    undef $ga_ff0_maxs;
    if($analysis_type eq 'GA') {
	$ga_params_demes = $gcf[$i]; $i++;
	$ga_params_generations = $gcf[$i]; $i++;
	$ga_params_crossover = $gcf[$i]; $i++;
	$ga_params_mutation = $gcf[$i]; $i++;
	$ga_params_plague = $gcf[$i]; $i++;
	$ga_Params_elitism = $gcf[$i]; $i++;
	$ga_params_migration_rate = $gcf[$i]; $i++;
	$ga_params_genes = $gcf[$i]; $i++;
	$ga_params_initial_solutes = $gcf[$i]; $i++;
	$ga_params_random_seed = $gcf[$i]; $i++;
	$monte_carlo = $gcf[$i]; $i++;
	$ga_params_regularization = $gcf[$i]; $i++;
	for (my $j=0; $j < $ga_params_initial_solutes; $j++) {
	    $ga_s[$j] = $gcf[$i]; $i++;
	    $ga_s_min[$j] = $gcf[$i]; $i++;
	    $ga_s_max[$j] = $gcf[$i]; $i++;
	    $ga_ff0[$j] = $gcf[$i]; $i++;
	    $ga_ff0_min[$j] = $gcf[$i]; $i++;
	    $ga_ff0_max[$j] = $gcf[$i]; $i++;
	}
	$ga_ss = join ', ', @ga_s;
	$ga_s_mins = join ', ', @ga_s_min;
	$ga_s_maxs = join ', ', @ga_s_max;
	$ga_ff0s = join ', ', @ga_ff0;
	$ga_ff0_mins = join ', ', @ga_ff0_min;
	$ga_ff0_maxs = join ', ', @ga_ff0_max;
	$fit_meniscus = $gcf[$i]; $i++;
	$meniscus_range = $gcf[$i]; $i++;
	$meniscus_gridpoints = 0;
	$fit_ti_noise = $gcf[$i]; $i++;
	$fit_ri_noise = $gcf[$i]; $i++;
	$email = $gcf[$i]; $i++;
	$HPCAnalysisID = $gcf[$i]; $i++;
    }
    if($analysis_type eq 'GA_MW') {
	$ga_params_demes = $gcf[$i]; $i++;
	$ga_params_generations = $gcf[$i]; $i++;
	$ga_params_crossover = $gcf[$i]; $i++;
	$ga_params_mutation = $gcf[$i]; $i++;
	$ga_params_plague = $gcf[$i]; $i++;
	$ga_Params_elitism = $gcf[$i]; $i++;
	$ga_params_migration_rate = $gcf[$i]; $i++;
	$ga_params_genes = $gcf[$i]; $i++;
	$ga_params_initial_solutes = length($gcf[$i]); $i++;
	$ga_params_random_seed = $gcf[$i]; $i++;
	$monte_carlo = $gcf[$i]; $i++;
	$ga_params_regularization = $gcf[$i]; $i++;
	$j = 0;
	$ga_s_min[$j] = $gcf[$i]; $i++;
	$ga_s_max[$j] = $gcf[$i]; $i++;
	$ga_s[$j] = ($ga_s_min[$j] + $ga_s_max[$j])/2.0;
	$ga_ff0_min[$j] = $gcf[$i]; $i++;
	$ga_ff0_max[$j] = $gcf[$i]; $i++;
	$ga_ff0[$j] = ($ga_ff0_min[$j] + $ga_ff0_max[$j])/2.0;
	$ga_ss = join ', ', @ga_s;
	$ga_s_mins = join ', ', @ga_s_min;
	$ga_s_maxs = join ', ', @ga_s_max;
	$ga_ff0s = join ', ', @ga_ff0;
	$ga_ff0_mins = join ', ', @ga_ff0_min;
	$ga_ff0_maxs = join ', ', @ga_ff0_max;
	$fit_meniscus = $gcf[$i]; $i++;
	$meniscus_range = $gcf[$i]; $i++;
	$meniscus_gridpoints = 0;
	$fit_ti_noise = $gcf[$i]; $i++;
	$fit_ri_noise = $gcf[$i]; $i++;
	$email = $gcf[$i]; $i++;
	$HPCAnalysisID = $gcf[$i]; $i++;
    }
    if($analysis_type eq 'GA_SC') {
	$ga_params_demes = $gcf[$i]; $i++;
	$ga_params_generations = $gcf[$i]; $i++;
	$ga_params_crossover = $gcf[$i]; $i++;
	$ga_params_mutation = $gcf[$i]; $i++;
	$ga_params_plague = $gcf[$i]; $i++;
	$ga_Params_elitism = $gcf[$i]; $i++;
	$ga_params_migration_rate = $gcf[$i]; $i++;
	$ga_params_genes = $gcf[$i]; $i++;
	$ga_params_random_seed = $gcf[$i]; $i++;
	$monte_carlo = $gcf[$i]; $i++;
	$ga_params_regularization = $gcf[$i]; $i++;
	$fit_meniscus = $gcf[$i]; $i++;
	$meniscus_range = $gcf[$i]; $i++;
	$meniscus_gridpoints = 0;
	$fit_ti_noise = $gcf[$i]; $i++;
	$fit_ri_noise = $gcf[$i]; $i++;
	$email = $gcf[$i]; $i++;
	$HPCAnalysisID = $gcf[$i]; $i++;
    }
    undef $sa2d_params_ff0_min;
    undef $sa2d_params_ff0_max;
    undef $sa2d_params_ff0_resolution;
    undef $sa2d_params_s_min;
    undef $sa2d_params_s_max;
    undef $sa2d_params_s_resolution;
    undef $sa2d_params_regularization;
    undef $sa2d_params_uniform_grid_repetition;
    undef $sa2d_params_use_iterative;
    undef $sa2d_params_max_iterations;
    if($analysis_type eq 'SA2D' ||
       $analysis_type eq '2DSA') {
	$sa2d_params_ff0_min = $gcf[$i]; $i++;
	$sa2d_params_ff0_max = $gcf[$i]; $i++;
	$sa2d_params_ff0_resolution = $gcf[$i]; $i++;
	$sa2d_params_s_min = $gcf[$i]; $i++;
	$sa2d_params_s_max = $gcf[$i]; $i++;
	$sa2d_params_s_resolution = $gcf[$i]; $i++;
	$sa2d_params_regularization = $gcf[$i]; $i++;
	$sa2d_params_uniform_grid_repetition = $gcf[$i]; $i++;
	$fit_meniscus = $gcf[$i]; $i++;
	$meniscus_range = $gcf[$i]; $i++;
	$meniscus_gridpoints = $gcf[$i]; $i++;
	$fit_ti_noise = $gcf[$i]; $i++;
	$fit_ri_noise = $gcf[$i]; $i++;
	$email = $gcf[$i]; $i++;
	$sa2d_params_use_iterative = $gcf[$i]; $i++;
	$sa2d_params_max_iterations = $gcf[$i]; $i++;
	$monte_carlo = $gcf[$i]; $i++;
	$HPCAnalysisID = $gcf[$i]; $i++;
    }
    if($analysis_type eq '2DSA_MW') {
	$sa2d_params_ff0_min = $gcf[$i]; $i++;
	$sa2d_params_ff0_max = $gcf[$i]; $i++;
	$sa2d_params_ff0_resolution = $gcf[$i]; $i++;
	$sa2d_params_s_min = $gcf[$i]; $i++;
	$sa2d_params_s_max = $gcf[$i]; $i++;
	$sa2d_params_s_resolution = $gcf[$i]; $i++;
	$sa2d_params_regularization = $gcf[$i]; $i++;
	$sa2d_params_uniform_grid_repetition = $gcf[$i]; $i++;
	$fit_meniscus = $gcf[$i]; $i++;
	$meniscus_range = $gcf[$i]; $i++;
	$meniscus_gridpoints = $gcf[$i]; $i++;
	$fit_ti_noise = $gcf[$i]; $i++;
	$fit_ri_noise = $gcf[$i]; $i++;
	$email = $gcf[$i]; $i++;
	$sa2d_params_use_iterative = $gcf[$i]; $i++;
	$sa2d_params_max_iterations = $gcf[$i]; $i++;
	$monte_carlo = $gcf[$i]; $i++;
	$sa2d_params_max_mer = $gcf[$i]; $i++;
	$HPCAnalysisID = $gcf[$i]; $i++;
    }




    $dbstring = "$db_login_user|$db_login_database|$db_login_host|$email|$no_experiments|$exp_file_infos|$exp_dirs|$exp_cells|$exp_lambdas|$exp_channels|$analysis_type|$monte_carlo|$fit_ti_noise|$fit_ri_noise|$fit_meniscus|$meniscus_range|$meniscus_gridpoints";
    $dbstring .= "|$ga_params_demes|$ga_params_generations|$ga_params_crossover|$ga_params_mutation|$ga_params_plague|$ga_Params_elitism|$ga_params_migration_rate|$ga_params_genes|$ga_params_initial_solutes|$ga_params_random_seed";
    $dbstring .= "|$ga_ss|$ga_s_mins|$ga_s_maxs|$ga_ff0s|$ga_ff0_mins|$ga_ff0_maxs";
    $dbstring .= "|$sa2d_params_ff0_min|$sa2d_params_ff0_max|$sa2d_params_ff0_resolution|$sa2d_params_s_min|$sa2d_params_s_max|$sa2d_params_s_resolution|$sa2d_params_regularization|$sa2d_params_uniform_grid_repetition|$sa2d_params_use_iterative|$sa2d_params_max_iterations";
    $db_minstring = "$db_login_user|$db_login_database|$db_login_host|$email|$no_experiments|$exp_file_infos|$exp_cells|$exp_lambdas|$exp_channels|$analysis_type|$monte_carlo|$fit_ti_noise|$fit_ri_noise|$fit_meniscus|$meniscus_gridpoints";
    $db_minstring .= "|$ga_params_demes|$ga_params_generations|$ga_params_genes|$ga_params_initial_solutes|";
    $db_minstring .= "|$sa2d_params_ff0_resolution|$sa2d_params_s_resolution|$sa2d_params_uniform_grid_repetition|$sa2d_params_use_iterative|$sa2d_params_max_iterations";

    $dbstring_title = "db_login_user|db_login_database|db_login_host|email|no_experiments|exp_file_infos|exp_dirs|exp_cells|exp_lambdas|exp_channels|analysis_type|monte_carlo|fit_ti_noise|fit_ri_noise|fit_meniscus|meniscus_range|meniscus_gridpoints";
    $dbstring_title .= "|ga_params_demes|ga_params_generations|ga_params_crossover|ga_params_mutation|ga_params_plague|ga_Params_elitism|ga_params_migration_rate|ga_params_genes|ga_params_initial_solutes|ga_params_random_seed";
    $dbstring_title .= "|ga_ss|ga_s_mins|ga_s_maxs|ga_ff0s|ga_ff0_mins|ga_ff0_maxs";
    $dbstring_title .= "|sa2d_params_ff0_min|sa2d_params_ff0_max|sa2d_params_ff0_resolution|sa2d_params_s_min|sa2d_params_s_max|sa2d_params_s_resolution|sa2d_params_regularization|sa2d_params_uniform_grid_repetition|sa2d_params_use_iterative|sa2d_params_max_iterations";
    $db_minstring_title = "db_login_user|db_login_database|db_login_host|email|no_experiments|exp_file_infos|exp_cells|exp_lambdas|exp_channels|analysis_type|monte_carlo|fit_ti_noise|fit_ri_noise|fit_meniscus|meniscus_gridpoints";
    $db_minstring_title .= "|ga_params_demes|ga_params_generations|ga_params_genes|ga_params_initial_solutes|";
    $db_minstring_title .= "|sa2d_params_ff0_resolution|sa2d_params_s_resolution|sa2d_params_uniform_grid_repetition|sa2d_params_use_iterative|sa2d_params_max_iterations";
}

return 1;
__END__
$file = shift;
chomp $file;
&parsegc($file);
print "hpcanalysis id $HPCAnalysisID contraintfile $ga_sc_constraint_file\n";

# return 1;
# comment for testing 

$file = shift;
chomp $file;
&parsegc($file);
print "<$dbstring_title>\n<$dbstring>\nmin<$db_minstring_title><$db_minstring>\n";
