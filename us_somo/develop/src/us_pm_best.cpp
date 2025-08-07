#include "../include/us_pm.h"

#define USPM_BEST_DELTA_MIN 1e-3

// #define USPM_USE_CA 0
#define USPM_SEQ_PEGGED_LIMIT 3

void US_PM::set_best_delta(double best_delta_start, double best_delta_divisor,
                           double best_delta_min) {
  this->best_delta_start = best_delta_start;
  this->best_delta_divisor = best_delta_divisor;
  this->best_delta_min = best_delta_min;

  if (this->best_delta_min < USPM_BEST_DELTA_MIN) {
    this->best_delta_min = USPM_BEST_DELTA_MIN;
    if (us_log) {
      us_log->log(QString("Notice: best delta mininimum requested %1 is below "
                          "hard limit %2, reset to %3\n")
                      .arg(best_delta_min)
                      .arg(USPM_BEST_DELTA_MIN)
                      .arg(this->best_delta_min));
    }
  }
  best_delta_size_min = 5e-1;
  best_delta_size_max = max_dimension_d;

  delta_min = best_delta_size_min;
  theta_min = M_PI * 1e-3;
}

bool US_PM::best_vary_one_param(unsigned int param_to_vary,
                                vector<double>& params, set<pm_point>& model,
                                double& best_fitness) {
  // use compute_delta_I
  double best_fit = 1e99;
  double prev_fit = 1e99;
  double this_fit;

  double delta = best_delta_start;
  // double prev_size;
  double best_size = 1e-99;

  double low_limit = best_delta_size_min;
  double high_limit = best_delta_size_max;

  vector<double> I_result(q_points);

  set<pm_point> this_model;
  set<pm_point> prev_model;
  vector<vector<complex<float> > > Av;

  vector<double> best_params;

  map<double, double> fitnesses;

  while (delta >= best_delta_min) {
    double last_fitness_1_pos = -1e99;
    double last_fitness_2_pos = -1e99;
    double last_fitness_3_pos = -1e99;
    prev_fit = 1e99;

    prev_model.clear();
    Av.clear();

    if (us_log) {
      us_log->log(
          QString(
              "best_vary_one_param: ------------ this limit %1 %2 delta %3\n")
              .arg(low_limit)
              .arg(high_limit)
              .arg(delta));
    }
    unsigned int steps_without_change = 0;
    for (params[param_to_vary] = low_limit; params[param_to_vary] <= high_limit;
         params[param_to_vary] += delta) {
      bool skip = false;
      create_model(params, this_model);
      // cout << QString( "create model size %1 prev model size %2\n" ).arg(
      // this_model.size() ).arg( prev_model.size() );
      if (this_model.size() && prev_model.size() != this_model.size()) {
        //             if ( USPM_USE_CA )
        //             {
        //                compute_I( this_model, I_result );
        //             } else {
        compute_delta_I(this_model, prev_model, Av, I_result);
        //             }
        this_fit = fitness2(I_result);
        fitnesses[params[param_to_vary]] = this_fit;
      } else {
        skip = true;
      }
      if (!skip) {
        // write_model( tmp_name( "", params ), this_model );
        //             QString qs =
        //                QString( "%1 fitness^2 %2 beads %3 params:" )
        //                .arg( object_names[ (int) params[ 0 ] ] )
        //                .arg( this_fit, 0, 'g', 8 )
        //                .arg( this_model.size() )
        //                ;
        //             for ( int i = 1; i <= (int)object_m0_parameters[ (int)
        //             params[ 0 ] ]; ++i )
        //             {
        //                qs += QString( " %1" ).arg( params[ i ] );
        //             }
        //             cout << qs.toLatin1().data() << endl;
        if (us_log) {
          us_log->log(US_Vector::qs_vector(
              QString("best_vary_one_param: %1beads %2 fitness %3, params:")
                  .arg(this_fit < prev_fit ? "**" : "  ")
                  .arg(this_model.size())
                  .arg(this_fit),
              params));
        }

        last_fitness_3_pos = last_fitness_2_pos;
        last_fitness_2_pos = last_fitness_1_pos;
        last_fitness_1_pos = params[param_to_vary];

        if (this_fit < prev_fit) {
          best_fit = this_fit;
          best_size = params[param_to_vary];
          best_params = params;
          model = this_model;
        }
        if (this_fit > prev_fit) {
          break;
        }
        prev_fit = this_fit;
        // prev_size = params[ param_to_vary ];
        steps_without_change = 0;
      } else {
        //             QString qs =
        //                QString( "skipping %1 (empty or identical to previous)
        //                beads %2 prev %3 params:" ) .arg( object_names[ (int)
        //                params[ 0 ] ] ) .arg( this_model.size() ) .arg(
        //                prev_model.size() )
        //                ;
        //             for ( int i = 1; i <= (int)object_m0_parameters[ (int)
        //             params[ 0 ] ]; ++i )
        //             {
        //                qs += QString( " %1" ).arg( params[ i ] );
        //             }
        //             cout << qs.toLatin1().data() << endl;
        if (steps_without_change > 100) {
          if (us_log) {
            us_log->log(
                "best_vary_one_param: Too many steps without model change, "
                "terminating inner loop\n");
          }
          break;
        }
        steps_without_change++;
      }
      prev_model = this_model;
    }
    if (last_fitness_3_pos == -1e99 || last_fitness_1_pos == -1e99) {
      if (best_size != -1e99) {
        if (us_log) {
          us_log->log(
              "best_vary_one_param: found nothing to recenter on, centering on "
              "best +/- delta\n");
        }
        last_fitness_3_pos = best_size - delta;
        if (last_fitness_3_pos < delta_min) {
          last_fitness_3_pos = delta_min;
        }
        last_fitness_1_pos = best_size + delta;
      } else {
        if (us_log) {
          us_log->log(
              "best_vary_one_param: found nothing to recenter on, terminating "
              "outer loop\n");
        }
        break;
      }
    }

    low_limit = last_fitness_3_pos;
    high_limit = last_fitness_1_pos;
    if (best_size == high_limit) {
      if (us_log) {
        us_log->log(
            "best_vary_one_param: best found at end, extending interval by 5 "
            "delta\n");
      }
      high_limit += 5e0 * delta;
    } else {
      delta /= best_delta_divisor;
    }
  }
  best_fitness = best_fit;
  if (us_log) {
    us_log->log(US_Vector::qs_vector(
        QString("best_vary_one_param: ----end----- fitness %1, params:")
            .arg(best_fitness),
        best_params));
  }
  return true;
}

bool US_PM::best_vary_two_param(unsigned int param_to_vary_1,
                                unsigned int param_to_vary_2,
                                vector<double>& params, set<pm_point>& model,
                                double& best_fitness) {
  set<pm_point> this_model;
  use_CYJ = false;

  QString qs_timer =
      QString("%1 2 params to vary").arg(object_names[(int)params[0]]);

  us_timers.clear_timers();
  us_timers.init_timer(qs_timer);
  us_timers.start_timer(qs_timer);

  double delta = best_delta_start;
  double low_limit = best_delta_size_min;
  double high_limit = best_delta_size_max;

  // double prev_size;
  // double best_size;

  double best_fit = 1e99;
  double prev_fit = 1e99;
  double this_fit;

  map<double, double> fitnesses;
  while (delta > best_delta_min) {
    double last_fitness_1_pos = -1e0;
    double last_fitness_2_pos = -1e0;
    double last_fitness_3_pos = -1e0;
    prev_fit = 1e99;

    if (us_log) {
      us_log->log(
          QString(
              "best_vary_two_param: ------------ this limit %1 %2 delta %3\n")
              .arg(low_limit)
              .arg(high_limit)
              .arg(delta));
    }
    for (params[param_to_vary_2] = low_limit;
         params[param_to_vary_2] <= high_limit;
         params[param_to_vary_2] += delta) {
      best_vary_one_param(param_to_vary_1, params, this_model, this_fit);
      fitnesses[params[param_to_vary_2]] = this_fit;

      last_fitness_3_pos = last_fitness_2_pos;
      last_fitness_2_pos = last_fitness_1_pos;
      last_fitness_1_pos = params[param_to_vary_2];

      if (us_log) {
        us_log->log(US_Vector::qs_vector(
            QString("best_vary_two_param: %1beads %2 fitness %3, params:")
                .arg(this_fit < prev_fit ? "**" : "  ")
                .arg(this_model.size())
                .arg(this_fit),
            params));
      }

      if (this_fit < prev_fit) {
        best_fit = this_fit;
        // best_size = params[ param_to_vary_2 ];
        model = this_model;
      }
      if (this_fit > prev_fit) {
        break;
      }
      prev_fit = this_fit;
      // prev_size = params[ param_to_vary_2 ];
    }
    if (last_fitness_3_pos < 0e0 || last_fitness_1_pos < 0e0) {
      if (us_log) {
        us_log->log(
            "best_vary_two_param: found nothing to recenter on, terminating "
            "outer loop\n");
      }
      break;
    }
    low_limit = last_fitness_3_pos;
    high_limit = last_fitness_1_pos;
    delta /= best_delta_divisor;
  }
  us_timers.end_timer(qs_timer);
  if (us_log) {
    us_log->log(us_timers.list_times());
  }

  best_fitness = best_fit;
  return true;
}

// set params[ 0 ] and run

bool US_PM::best_md0(vector<double>& params, set<pm_point>& model,
                     double finest_conversion, double coarse_conversion,
                     double refinement_range_pct, double conversion_divisor) {
  if (!zero_md0_params(params)) {
    error_msg = "best_md0: " + error_msg;
    return false;
  }

  if (!set_grid_size(coarse_conversion)) {
    return false;
  }

#if defined(USE_MPI)
  {
    int errorno = -29000;
    pm_msg msg;
    msg.type = PM_NEW_GRID_SIZE;
    msg.grid_conversion_factor = coarse_conversion;

    for (int i = 1; i < npes; ++i) {
      if (MPI_SUCCESS !=
          MPI_Send(&msg, sizeof(pm_msg), MPI_CHAR, i, PM_MSG, MPI_COMM_WORLD)) {
        if (us_log) {
          us_log->log(
              QString("%1: MPI send failed in best_ga() PM_NEW_GRID_SIZE\n")
                  .arg(myrank));
        }
        MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
        exit(errorno - myrank);
      }
    }
  }
#endif

  vector<double> org_low_fparams;
  vector<double> org_high_fparams;
  set_limits(params, org_low_fparams, org_high_fparams);

  vector<double> low_fparams = org_low_fparams;
  vector<double> high_fparams = org_high_fparams;

  vector<double> next_low_fparams = low_fparams;
  vector<double> next_high_fparams = high_fparams;

  double new_grid_conversion_factor = grid_conversion_factor;

  while (grid_conversion_factor >= finest_conversion) {
    // need params[] from best model and ability to set limits for best model
    // have to add to best_*, maybe add general best_model( params )

    if (us_log) {
      us_log->log(
          "--------------------------------------------------------------------"
          "--\n");
      us_log->log(QString("best_md0: grid cube side: %1\n")
                      .arg(grid_conversion_factor));
      us_log->log(
          "--------------------------------------------------------------------"
          "--\n");
      us_log->log(US_Vector::qs_vector2("param limits before clipping:",
                                        next_low_fparams, next_high_fparams));
    }

    clip_limits(next_low_fparams, org_low_fparams, org_high_fparams);
    clip_limits(next_high_fparams, org_low_fparams, org_high_fparams);
    low_fparams = next_low_fparams;
    high_fparams = next_high_fparams;

    if (us_log) {
      us_log->log(
          US_Vector::qs_vector2("param limits:", low_fparams, high_fparams));
    }

    if (!best_md0(params, low_fparams, high_fparams, model)) {
      return false;
    }

    // convert limits to new factor

    if (grid_conversion_factor == finest_conversion) {
      break;
    }

    new_grid_conversion_factor = grid_conversion_factor / conversion_divisor;
    if (new_grid_conversion_factor < finest_conversion) {
      new_grid_conversion_factor = finest_conversion;
    }

    if (us_log) {
      us_log->log(US_Vector::qs_vector("result params:", params));
      us_log->log(US_Vector::qs_vector2(
          "previous limits before rescaling:", low_fparams, high_fparams));
    }

    for (int i = 0; i < (int)low_fparams.size(); i++) {
      next_low_fparams[i] =
          params[i + 1] - grid_conversion_factor * refinement_range_pct / 100e0;
      next_high_fparams[i] =
          params[i + 1] + grid_conversion_factor * refinement_range_pct / 100e0;

      next_low_fparams[i] *=
          grid_conversion_factor / new_grid_conversion_factor;
      next_high_fparams[i] *=
          grid_conversion_factor / new_grid_conversion_factor;
      low_fparams[i] *= grid_conversion_factor / new_grid_conversion_factor;
      high_fparams[i] *= grid_conversion_factor / new_grid_conversion_factor;
    }

    if (us_log) {
      us_log->log(US_Vector::qs_vector2(
          "previous limits after rescaling:", low_fparams, high_fparams));
      us_log->log(US_Vector::qs_vector2(
          "new limits after rescaling:", next_low_fparams, next_high_fparams));
    }

    if (!set_grid_size(new_grid_conversion_factor)) {
      return false;
    }
#if defined(USE_MPI)
    {
      int errorno = -29000;
      pm_msg msg;
      msg.type = PM_NEW_GRID_SIZE;
      msg.grid_conversion_factor = new_grid_conversion_factor;

      for (int i = 1; i < npes; ++i) {
        if (MPI_SUCCESS != MPI_Send(&msg, sizeof(pm_msg), MPI_CHAR, i, PM_MSG,
                                    MPI_COMM_WORLD)) {
          if (us_log) {
            us_log->log(
                QString("%1: MPI send failed in best_ga() PM_NEW_GRID_SIZE\n")
                    .arg(myrank));
          }
          MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
          exit(errorno - myrank);
        }
      }
    }
#endif
  }
  return true;
}

bool US_PM::best_vary_one_param(unsigned int param_to_vary,
                                vector<double>& params,
                                vector<double>& low_fparams,
                                vector<double>& high_fparams,
                                set<pm_point>& model, double& best_fitness) {
  // use compute_delta_I
  double best_fit = 1e99;
  double prev_fit = 1e99;
  double this_fit;

  double delta = best_delta_start;
  // double prev_size;
  double best_size = -1e99;

  double low_limit = low_fparams[param_to_vary - 1];
  double high_limit = high_fparams[param_to_vary - 1];

  vector<double> I_result(q_points);

  if (high_limit - low_limit <= best_delta_min) {
    if (us_log) {
      us_log->log(
          QString("best_vary_one_param: parameter range %1:%2 is smaller than "
                  "minimum delta %3, returning average model\n")
              .arg(low_limit)
              .arg(high_limit)
              .arg(best_delta_min));
    }
    params[param_to_vary] = (high_limit + low_limit) / 2e0;
    create_model(params, model);
    compute_I(model, I_result);
    best_fitness = fitness2(I_result);
    return true;
  }

  if ((high_limit - low_limit) / 10e0 < delta) {
    delta = (high_limit - low_limit) / 10e0;
  }
  if (delta < best_delta_min) {
    delta = best_delta_min;
  }

  vector<double> best_params;

  set<pm_point> this_model;
  set<pm_point> prev_model;
  vector<vector<complex<float> > > Av;

  map<double, double> fitnesses;

  while (delta >= best_delta_min) {
    double last_fitness_1_pos = -1e99;
    double last_fitness_2_pos = -1e99;
    double last_fitness_3_pos = -1e99;
    prev_fit = 1e99;

    if (us_log) {
      us_log->log(
          QString(
              "best_vary_one_param: ------------ this limit %1 %2 delta %3\n")
              .arg(low_limit)
              .arg(high_limit)
              .arg(delta));
    }
    unsigned int steps_without_change = 0;
    for (params[param_to_vary] = low_limit; params[param_to_vary] <= high_limit;
         params[param_to_vary] += delta) {
      bool skip = false;
      create_model(params, this_model);
      // cout << QString( "create model size %1 prev model size %2\n" ).arg(
      // this_model.size() ).arg( prev_model.size() );
      if (this_model.size() && prev_model.size() != this_model.size()) {
        //             if ( USPM_USE_CA )
        //             {
        //                compute_I( this_model, I_result );
        //             } else {
        compute_delta_I(this_model, prev_model, Av, I_result);
        //             }
        this_fit = fitness2(I_result);
        fitnesses[params[param_to_vary]] = this_fit;
      } else {
        skip = true;
      }
      if (!skip) {
        // write_model( tmp_name( "", params ), this_model );
        //             QString qs =
        //                QString( "%1 fitness^2 %2 beads %3 params:" )
        //                .arg( object_names[ (int) params[ 0 ] ] )
        //                .arg( this_fit, 0, 'g', 8 )
        //                .arg( this_model.size() )
        //                ;
        //             for ( int i = 1; i <= (int)object_m0_parameters[ (int)
        //             params[ 0 ] ]; ++i )
        //             {
        //                qs += QString( " %1" ).arg( params[ i ] );
        //             }
        //             cout << qs.toLatin1().data() << endl;
        if (us_log) {
          us_log->log(US_Vector::qs_vector(
              QString("best_vary_one_param: %1beads %2 fitness %3, params:")
                  .arg(this_fit < prev_fit ? "**" : "  ")
                  .arg(this_model.size())
                  .arg(this_fit),
              params));
        }

        last_fitness_3_pos = last_fitness_2_pos;
        last_fitness_2_pos = last_fitness_1_pos;
        last_fitness_1_pos = params[param_to_vary];

        if (this_fit < prev_fit) {
          best_fit = this_fit;
          best_size = params[param_to_vary];
          model = this_model;
          best_params = params;
        }
        if (this_fit > prev_fit) {
          break;
        }
        prev_fit = this_fit;
        // prev_size = params[ param_to_vary ];
        steps_without_change = 0;
      } else {
        //             QString qs =
        //                QString( "skipping %1 (empty or identical to previous)
        //                beads %2 prev %3 params:" ) .arg( object_names[ (int)
        //                params[ 0 ] ] ) .arg( this_model.size() ) .arg(
        //                prev_model.size() )
        //                ;
        //             for ( int i = 1; i <= (int)object_m0_parameters[ (int)
        //             params[ 0 ] ]; ++i )
        //             {
        //                qs += QString( " %1" ).arg( params[ i ] );
        //             }
        //             cout << qs.toLatin1().data() << endl;
        if (steps_without_change > 100) {
          if (us_log) {
            us_log->log(
                "best_vary_one_param: Too many steps without model change, "
                "terminating inner loop\n");
          }
          break;
        }
        steps_without_change++;
      }
      prev_model = this_model;
    }
    if (last_fitness_3_pos == -1e99 || last_fitness_1_pos == -1e99) {
      if (best_size != -1e99) {
        if (us_log) {
          us_log->log(
              "best_vary_one_param: found nothing to recenter on, centering on "
              "best +/- delta\n");
        }
        last_fitness_3_pos = best_size - delta;
        if (last_fitness_3_pos < delta_min) {
          last_fitness_3_pos = delta_min;
        }
        last_fitness_1_pos = best_size + delta;
      } else {
        if (us_log) {
          us_log->log(
              "best_vary_one_param: found nothing to recenter on, terminating "
              "outer loop\n");
        }
        break;
      }
    }

    low_limit = last_fitness_3_pos;
    high_limit = last_fitness_1_pos;
    if (best_size == high_limit) {
      if (us_log) {
        us_log->log(
            "best_vary_one_param: best found at end, extending interval by 20 "
            "delta\n");
      }
      low_limit = best_size;
      high_limit += 20e0 * delta;
    } else {
      prev_model.clear();
      Av.clear();
      delta /= best_delta_divisor;
    }
  }
  best_fitness = best_fit;
  params = best_params;
  if (us_log) {
    us_log->log(US_Vector::qs_vector(
        QString("best_vary_one_param: ----end----- fitness %1, params:")
            .arg(best_fitness),
        params));
  }
  return true;
}

bool US_PM::best_vary_two_param(unsigned int param_to_vary_1,
                                unsigned int param_to_vary_2,
                                vector<double>& params,
                                vector<double>& low_fparams,
                                vector<double>& high_fparams,
                                set<pm_point>& model, double& best_fitness) {
  set<pm_point> this_model;
  use_CYJ = false;

  QString qs_timer =
      QString("%1 2 params to vary").arg(object_names[(int)params[0]]);

  us_timers.clear_timers();
  us_timers.init_timer(qs_timer);
  us_timers.start_timer(qs_timer);

  double delta = best_delta_start;
  // double prev_size;
  // double best_size;

  double low_limit = low_fparams[param_to_vary_2 - 1];
  double high_limit = high_fparams[param_to_vary_2 - 1];

  double best_fit = 1e99;
  double prev_fit = 1e99;
  double this_fit;

  if (high_limit - low_limit <= best_delta_min) {
    if (us_log) {
      us_log->log(
          QString("best_vary_two_param: parameter range %1:%2 is smaller than "
                  "minimum delta %3, returning average model\n")
              .arg(low_limit)
              .arg(high_limit)
              .arg(best_delta_min));
    }
    params[param_to_vary_2] = (high_limit + low_limit) / 2e0;
    return best_vary_one_param(param_to_vary_1, params, low_fparams,
                               high_fparams, model, best_fitness);
  }

  if ((high_limit - low_limit) / 10e0 < delta) {
    delta = (high_limit - low_limit) / 10e0;
  }
  if (delta < best_delta_min) {
    delta = best_delta_min;
  }

  vector<double> best_params;
  map<double, double> fitnesses;

  while (delta > best_delta_min) {
    double last_fitness_1_pos = -1e0;
    double last_fitness_2_pos = -1e0;
    double last_fitness_3_pos = -1e0;
    prev_fit = 1e99;

    if (us_log) {
      us_log->log(
          QString(
              "best_vary_two_param: ------------ this limit %1 %2 delta %3\n")
              .arg(low_limit)
              .arg(high_limit)
              .arg(delta));
    }
    for (params[param_to_vary_2] = low_limit;
         params[param_to_vary_2] <= high_limit;
         params[param_to_vary_2] += delta) {
      best_vary_one_param(param_to_vary_1, params, low_fparams, high_fparams,
                          this_model, this_fit);
      fitnesses[params[param_to_vary_2]] = this_fit;

      last_fitness_3_pos = last_fitness_2_pos;
      last_fitness_2_pos = last_fitness_1_pos;
      last_fitness_1_pos = params[param_to_vary_2];

      if (us_log) {
        us_log->log(US_Vector::qs_vector(
            QString("best_vary_two_param: %1beads %2 fitness %3, params:")
                .arg(this_fit < prev_fit ? "**" : "  ")
                .arg(this_model.size())
                .arg(this_fit),
            params));
      }

      if (this_fit < prev_fit) {
        best_fit = this_fit;
        // best_size = params[ param_to_vary_2 ];
        model = this_model;
        best_params = params;
      }
      if (this_fit > prev_fit) {
        break;
      }
      prev_fit = this_fit;
      // prev_size = params[ param_to_vary_2 ];
    }
    if (last_fitness_3_pos < 0e0 || last_fitness_1_pos < 0e0) {
      if (us_log) {
        us_log->log(
            "best_vary_two_param: found nothing to recenter on, terminating "
            "outer loop\n");
      }
      break;
    }
    low_limit = last_fitness_3_pos;
    high_limit = last_fitness_1_pos;
    delta /= best_delta_divisor;
  }
  us_timers.end_timer(qs_timer);
  if (us_log) {
    us_log->log(us_timers.list_times());
  }

  best_fitness = best_fit;
  params = best_params;
  if (us_log) {
    us_log->log(US_Vector::qs_vector(
        QString("best_vary_two_param: ----end----- fitness %1, params:")
            .arg(best_fitness),
        params));
  }
  return true;
}

bool US_PM::grid_search(vector<double>& params, vector<double>& delta,
                        vector<double>& low_fparams,
                        vector<double>& high_fparams, set<pm_point>& model) {
  QString gp;

  if (!zero_md0_params(params)) {
    error_msg = "grid_search: " + error_msg;
    return false;
  }

  if (delta.size() != low_fparams.size() ||
      delta.size() != high_fparams.size()) {
    error_msg =
        "grid_search: delta, low & high must all be set and equally sized";
    return false;
  }

  if (delta.size() != params.size() - 1) {
    error_msg = "grid_search: incorrect # of params";
    return false;
  }

  if (delta.size() > 4) {
    error_msg = "grid_search: max of 4 params currently supported";
    return false;
  }

  // compute # of points
  if (us_log) {
    us_log->log(US_Vector::qs_vector2("grid_search: low, high: ", low_fparams,
                                      high_fparams));
    us_log->log(US_Vector::qs_vector("grid_search: delta: ", delta));
  }

  int points = 1;
  for (int i = 0; i < (int)low_fparams.size(); i++) {
    points *= 1 + (high_fparams[i] - low_fparams[i]) / delta[i];
  }
  if (us_log) {
    us_log->log(QString("grid_search: points %1\n").arg(points));
  }

  for (int i = 0; i < (int)low_fparams.size(); i++) {
    params[i + 1] = low_fparams[i];
  }

  double best_fit = 1e99;
  vector<double> best_params;
  set<pm_point> best_model;
  vector<double> I_result;

  while (params.back() <= high_fparams.back()) {
    create_model(params, model);
    compute_I(model, I_result);
    double this_fit = fitness2(I_result);
    if (us_log) {
      us_log->log(US_Vector::qs_vector(
          QString("grid_search: %1fitness2 %2, beads %3 params")
              .arg(this_fit < best_fit ? "**" : "  ")
              .arg(this_fit, 0, 'g', 8)
              .arg(model.size()),
          params));
    }
    for (int i = 1; i < (int)params.size(); i++) {
      gp += QString("%1 ").arg(params[i]);
    }
    gp += QString("%1\n").arg(this_fit);

    if (this_fit < best_fit) {
      best_params = params;
      best_model = model;
      best_fit = this_fit;
    }
    params[1] += delta[0];
    if (params[1] > high_fparams[0] && high_fparams.size() > 1) {
      gp += "\n";
      params[1] = low_fparams[0];
      params[2] += delta[1];
      if (params[2] > high_fparams[1] && high_fparams.size() > 2) {
        params[2] = low_fparams[1];
        params[3] += delta[2];
        if (params[3] > high_fparams[2] && high_fparams.size() > 3) {
          params[3] = low_fparams[2];
          params[4] += delta[3];
        }
      }
    }
  }
  model = best_model;
  params = best_params;
  if (us_log) {
    us_log->log(US_Vector::qs_vector(
        QString("grid_search: ++fitness2 %1, beads %2 params")
            .arg(best_fit, 0, 'g', 8)
            .arg(model.size()),
        params));
  }

  if (us_log) {
    us_log->log(QString("gnuplot.txt:\n%1").arg(gp));
  }

  return true;
}

bool US_PM::best_md0_ga(vector<double>& params, set<pm_point>& model,
                        unsigned int steps_to_ga, unsigned int points_max,
                        double finest_conversion, double coarse_conversion,
                        double refinement_range_pct,
                        double conversion_divisor) {
  if (us_log) {
    us_log->log(QString("best_md0_ga: steps_to_ga            %1\n"
                        "             points_max             %2\n"
                        "             finest_conversion      %3\n"
                        "             coarse_conversion      %4\n"
                        "             refinement_range_pct   %5\n"
                        "             conversion_divisor     %6\n")
                    .arg(steps_to_ga)
                    .arg(points_max)
                    .arg(finest_conversion)
                    .arg(coarse_conversion)
                    .arg(refinement_range_pct)
                    .arg(conversion_divisor));
  }

  if (!steps_to_ga) {
    error_msg = "best_md0_ga: minimum steps is 1";
    return false;
  }

  if (!zero_md0_params(params)) {
    error_msg = "best_md0_ga: " + error_msg;
    return false;
  }

  if (!set_grid_size(coarse_conversion)) {
    return false;
  }
#if defined(USE_MPI)
  {
    int errorno = -29000;
    pm_msg msg;
    msg.type = PM_NEW_GRID_SIZE;
    msg.grid_conversion_factor = coarse_conversion;

    for (int i = 1; i < npes; ++i) {
      if (MPI_SUCCESS !=
          MPI_Send(&msg, sizeof(pm_msg), MPI_CHAR, i, PM_MSG, MPI_COMM_WORLD)) {
        if (us_log) {
          us_log->log(
              QString("%1: MPI send failed in best_ga() PM_NEW_GRID_SIZE\n")
                  .arg(myrank));
        }
        MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
        exit(errorno - myrank);
      }
    }
  }
#endif

  vector<double> org_low_fparams;
  vector<double> org_high_fparams;
  set_limits(params, org_low_fparams, org_high_fparams);

  vector<double> low_fparams = org_low_fparams;
  vector<double> high_fparams = org_high_fparams;

  vector<double> next_low_fparams = low_fparams;
  vector<double> next_high_fparams = high_fparams;

  double new_grid_conversion_factor = grid_conversion_factor;

  unsigned int steps = 0;

  vector<int> types;
  vector<double> fparams;
  split(params, types, fparams);

  for (; grid_conversion_factor >= finest_conversion; ++steps) {
    // need params[] from best model and ability to set limits for best model
    // have to add to best_*, maybe add general best_model( params )

    if (us_log) {
      us_log->log(
          "--------------------------------------------------------------------"
          "-----\n");
      us_log->log(QString("best_md0_ga: grid cube side: %1\n")
                      .arg(grid_conversion_factor));
      us_log->log(
          "--------------------------------------------------------------------"
          "-----\n");
      us_log->log(US_Vector::qs_vector2("param limits before clipping:",
                                        next_low_fparams, next_high_fparams));
    }

    clip_limits(next_low_fparams, org_low_fparams, org_high_fparams);
    clip_limits(next_high_fparams, org_low_fparams, org_high_fparams);
    low_fparams = next_low_fparams;
    high_fparams = next_high_fparams;

    if (us_log) {
      us_log->log(
          US_Vector::qs_vector2("param limits:", low_fparams, high_fparams));
    }

    if (steps >= steps_to_ga) {
      pm_ga_individual best_individual;
      //          ga_population = 25;
      //          ga_generations = 10;
      //          ga_elitism = 1;
      //          ga_early_termination = 3;

      if (!ga_run(types, best_individual, points_max, low_fparams,
                  high_fparams)) {
        return false;
      }
      model = best_individual.model;
      ga_delta_to_fparams(best_individual.v, ga_fparams);
      join(params, types, ga_fparams);
    } else {
      if (!best_md0(params, low_fparams, high_fparams, model)) {
        return false;
      }
    }

    if (0) {
      QString oname =
          QString("/root/ultrascan/somo/saxs/tmp/last_md0_best_gc_%1")
              .arg(grid_conversion_factor)
              .replace(".", "_");
      write_model(oname, model, params);
      write_I(oname, model);
    }

    // convert limits to new factor

    if (grid_conversion_factor == finest_conversion) {
      break;
    }

    new_grid_conversion_factor = grid_conversion_factor / conversion_divisor;
    if (new_grid_conversion_factor < finest_conversion) {
      new_grid_conversion_factor = finest_conversion;
    }

    if (us_log) {
      us_log->log(US_Vector::qs_vector("result params:", params));
      us_log->log(US_Vector::qs_vector2(
          "previous limits before rescaling:", low_fparams, high_fparams));
      us_log->log(QString("grid_conversion_factor: %1")
                      .arg(new_grid_conversion_factor));
    }

    if (!rescale_params(params, next_low_fparams, next_high_fparams,
                        new_grid_conversion_factor, refinement_range_pct)) {
      return false;
    }

    //       rescale_params( types, low_fparams , new_grid_conversion_factor );
    //       rescale_params( types, high_fparams, new_grid_conversion_factor );

    // if ( us_log )
    // {
    //    us_log->log( US_Vector::qs_vector2( "previous limits after
    //    rescaling:", low_fparams, high_fparams ) );
    // }

    if (us_log) {
      us_log->log(US_Vector::qs_vector2(
          "new limits after rescaling:", next_low_fparams, next_high_fparams));
    }

    if (!set_grid_size(new_grid_conversion_factor)) {
      return false;
    }
#if defined(USE_MPI)
    {
      int errorno = -29000;
      pm_msg msg;
      msg.type = PM_NEW_GRID_SIZE;
      msg.grid_conversion_factor = new_grid_conversion_factor;

      for (int i = 1; i < npes; ++i) {
        if (MPI_SUCCESS != MPI_Send(&msg, sizeof(pm_msg), MPI_CHAR, i, PM_MSG,
                                    MPI_COMM_WORLD)) {
          if (us_log) {
            us_log->log(
                QString(
                    "%1: MPI send failed in best_md0_ga() PM_NEW_GRID_SIZE\n")
                    .arg(myrank));
          }
          MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
          exit(errorno - myrank);
        }
      }
    }
#endif
  }
  return true;
}

bool US_PM::approx_max_dimension(vector<double>& params,
                                 double coarse_conversion,
                                 unsigned int& approx_max_d) {
  // run a coarse spheroid & use to determine maxd
  if (!set_grid_size(coarse_conversion)) {
    return false;
  }
  /* approx_max_dimension runs solely on master
     #if defined( USE_MPI )
     {
     int errorno = -29000;
     pm_msg msg;
     msg.type = PM_NEW_GRID_SIZE;
     msg.grid_conversion_factor = coarse_conversion;

     for ( int i = 1; i < npes; ++i )
     {
     if ( MPI_SUCCESS != MPI_Send( &msg,
     sizeof( pm_msg ),
     MPI_CHAR,
     i,
     PM_MSG,
     MPI_COMM_WORLD ) )
     {
     if ( us_log )
     {
     us_log->log( QString( "%1: MPI send failed in best_ga() PM_NEW_GRID_SIZE\n"
     ).arg( myrank ) );
     }
     MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
     exit( errorno - myrank );
     }
     }
     }
     #endif
  */

  params.resize(3);
  params[0] = 2e0;
  params[1] = 0e0;
  params[2] = 0e0;

  set<pm_point> model;
  vector<double> low_fparams;
  vector<double> high_fparams;

  set_limits(params, low_fparams, high_fparams);

  if (!best_md0(params, low_fparams, high_fparams, model)) {
    return false;
  }

  vector<int> types;
  vector<double> fparams;
  if (!split(params, types, fparams)) {
    return false;
  }

  if (!rescale_params(types, fparams, org_conversion_factor)) {
    return false;
  }

  reset_grid_size();
  /* approx_max_dimension runs solely on master
     #if defined( USE_MPI )
     {
     int errorno = -29000;
     pm_msg msg;
     msg.type = PM_NEW_GRID_SIZE;
     msg.grid_conversion_factor = org_conversion_factor;

     for ( int i = 1; i < npes; ++i )
     {
     if ( MPI_SUCCESS != MPI_Send( &msg,
     sizeof( pm_msg ),
     MPI_CHAR,
     i,
     PM_MSG,
     MPI_COMM_WORLD ) )
     {
     if ( us_log )
     {
     us_log->log( QString( "%1: MPI send failed in best_ga() PM_NEW_GRID_SIZE\n"
     ).arg( myrank ) );
     }
     MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
     exit( errorno - myrank );
     }
     }
     }
     #endif
  */

  double d_approx_max_d = fparams[0] > fparams[1] ? fparams[0] : fparams[1];
  d_approx_max_d *= 1.314e0;

  d_approx_max_d *= org_conversion_factor;
  approx_max_d = (unsigned int)(d_approx_max_d) + 1;

  return true;
}

bool US_PM::best_ga(vector<double>& params, set<pm_point>& model,
                    unsigned int points_max, double finest_conversion,
                    double coarse_conversion, double refinement_range_pct,
                    double conversion_divisor) {
  if (us_log) {
    us_log->log(US_Vector::qs_vector("best_ga: params", params));
    us_log->log(QString("best_ga:     points_max             %1\n"
                        "             finest_conversion      %2\n"
                        "             coarse_conversion      %3\n"
                        "             refinement_range_pct   %4\n"
                        "             conversion_divisor     %5\n")
                    .arg(points_max)
                    .arg(finest_conversion)
                    .arg(coarse_conversion)
                    .arg(refinement_range_pct)
                    .arg(conversion_divisor));
  }

  vector<int> types;
  vector<double> fparams;
  split(params, types, fparams);

  if (!zero_params(params, types)) {
    error_msg = "best_ga: " + error_msg;
    return false;
  }
  split(params, types, fparams);

  if (!set_grid_size(coarse_conversion)) {
    return false;
  }
#if defined(USE_MPI)
  {
    int errorno = -29000;
    pm_msg msg;
    msg.type = PM_NEW_GRID_SIZE;
    msg.grid_conversion_factor = coarse_conversion;

    for (int i = 1; i < npes; ++i) {
      if (MPI_SUCCESS !=
          MPI_Send(&msg, sizeof(pm_msg), MPI_CHAR, i, PM_MSG, MPI_COMM_WORLD)) {
        if (us_log) {
          us_log->log(
              QString("%1: MPI send failed in best_ga() PM_NEW_GRID_SIZE\n")
                  .arg(myrank));
        }
        MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
        exit(errorno - myrank);
      }
    }
  }
#endif
  vector<double> max_low_fparams;
  vector<double> max_high_fparams;
  set_limits(params, max_low_fparams, max_high_fparams);

  vector<double> org_low_fparams;
  vector<double> org_high_fparams;
  set_limits(params, org_low_fparams, org_high_fparams,
             (double)org_max_dimension / coarse_conversion);
  if (us_log) {
    us_log->log(QString("org_max_dimension %1 org_conversion_factor %2 "
                        "coarse_conversion %3 use_max_d %4\n")
                    .arg(org_max_dimension)
                    .arg(org_conversion_factor)
                    .arg(coarse_conversion)
                    .arg((double)org_max_dimension / coarse_conversion));
  }

  vector<double> low_fparams = org_low_fparams;
  vector<double> high_fparams = org_high_fparams;

  vector<double> next_low_fparams = low_fparams;
  vector<double> next_high_fparams = high_fparams;

  double new_grid_conversion_factor = grid_conversion_factor;

  unsigned int steps = 0;

  if (us_log) {
    us_log->log(US_Vector::qs_vector("best_ga fparams", fparams));
    us_log->log(US_Vector::qs_vector("best_ga types", types));
  }

  unsigned int ga_sequential_pegged = 0;

  for (; grid_conversion_factor >= finest_conversion; ++steps) {
    // need params[] from best model and ability to set limits for best model
    // have to add to best_*, maybe add general best_model( params )

    if (us_log) {
      us_log->log(
          "--------------------------------------------------------------------"
          "-----\n");
      us_log->log(
          QString("best_ga: grid cube side: %1\n").arg(grid_conversion_factor));
      us_log->log(
          "--------------------------------------------------------------------"
          "-----\n");
      us_log->log(US_Vector::qs_vector2("param limits before clipping:",
                                        next_low_fparams, next_high_fparams));
    }

    clip_limits(next_low_fparams, max_low_fparams, max_high_fparams);
    clip_limits(next_high_fparams, max_low_fparams, max_high_fparams);
    low_fparams = next_low_fparams;
    high_fparams = next_high_fparams;

    if (us_log) {
      us_log->log(
          US_Vector::qs_vector2("param limits:", low_fparams, high_fparams));
    }

    pm_ga_individual best_individual;
    if (!ga_run(types, best_individual, points_max, low_fparams,
                high_fparams)) {
      return false;
    }
    model = best_individual.model;
    ga_delta_to_fparams(best_individual.v, ga_fparams);
    join(params, types, ga_fparams);
    {
      QString oname = QString("/root/ultrascan/somo/saxs/tmp/last_best_gc_%1")
                          .arg(grid_conversion_factor)
                          .replace(".", "_");
      write_model(oname, model, params);
      write_I(oname, model);
    }

    // convert limits to new factor
    if (pm_ga_pegged) {
      ga_sequential_pegged++;
    } else {
      ga_sequential_pegged = 0;
    }

    if (grid_conversion_factor == finest_conversion &&
        pm_ga_last_max_best_delta_min <= best_delta_min &&
        (!pm_ga_pegged || ga_sequential_pegged > USPM_SEQ_PEGGED_LIMIT)) {
      if (ga_sequential_pegged > USPM_SEQ_PEGGED_LIMIT) {
        if (us_log) {
          us_log->log(QString("Warning: sequential pegged limit %1 reached\n")
                          .arg(USPM_SEQ_PEGGED_LIMIT));
        }
      }
      break;
    }

    new_grid_conversion_factor = grid_conversion_factor / conversion_divisor;
    if (new_grid_conversion_factor < finest_conversion) {
      new_grid_conversion_factor = finest_conversion;
    }

    if (us_log) {
      us_log->log(US_Vector::qs_vector("result params:", params));
      us_log->log(US_Vector::qs_vector2(
          "previous limits before rescaling:", low_fparams, high_fparams));
      us_log->log(QString("grid_conversion_factor: %1")
                      .arg(new_grid_conversion_factor));
    }

    if (!rescale_params(params, next_low_fparams, next_high_fparams,
                        new_grid_conversion_factor, refinement_range_pct)) {
      return false;
    }

    if (!rescale_params(types, fparams, new_grid_conversion_factor)) {
      return false;
    }
    join(params, types, fparams);
    ga_seed_params.push_back(params);

    if (grid_conversion_factor == finest_conversion &&
        pm_ga_last_max_best_delta_min <= best_delta_min && pm_ga_pegged) {
      // leave non-pegged alone
      for (int i = 0; i < (int)pm_ga_peggedv.size(); ++i) {
        if (!pm_ga_peggedv[i]) {
          next_low_fparams[i] = low_fparams[i];
          next_high_fparams[i] = high_fparams[i];
        }
      }
    }

    if (us_log) {
      us_log->log(US_Vector::qs_vector2(
          "new limits after rescaling:", next_low_fparams, next_high_fparams));
    }

    if (new_grid_conversion_factor != grid_conversion_factor) {
      if (!set_grid_size(new_grid_conversion_factor)) {
        return false;
      }
#if defined(USE_MPI)
      {
        int errorno = -29000;
        pm_msg msg;
        msg.type = PM_NEW_GRID_SIZE;
        msg.grid_conversion_factor = new_grid_conversion_factor;

        for (int i = 1; i < npes; ++i) {
          if (MPI_SUCCESS != MPI_Send(&msg, sizeof(pm_msg), MPI_CHAR, i, PM_MSG,
                                      MPI_COMM_WORLD)) {
            if (us_log) {
              us_log->log(
                  QString("%1: MPI send failed in best_ga() PM_NEW_GRID_SIZE\n")
                      .arg(myrank));
            }
            MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
            exit(errorno - myrank);
          }
        }
      }
#endif
    }
  }
  return true;
}
