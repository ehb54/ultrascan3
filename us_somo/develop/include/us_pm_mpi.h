#ifndef US_PM_MPI_H
#define US_PM_MPI_H

// pm_types for msg.type and also for some MPI_TAGs
enum pm_types {
  PM_SHUTDOWN  // sent to workers to shutdown
  ,
  PM_MSG  // sent to workers as base message
  ,
  PM_NEW_PM  // sent to workers as additional message info for NEW_PM
  ,
  PM_NEW_GRID_SIZE  // sent to workers as additional message info for
                    // NEW_GRID_SIZE
  ,
  PM_CALC_FITNESS  // sent to workers to process fitness
  ,
  PM_REGISTER  // sent form workers to register to master
  ,
  PM_FITNESS_RESULT  // sent form workers to report results
  ,
  PM_FITNESS_RESULT_MODEL  // sent form workers to report results
};

// flags -----------

#define PM_USE_ERRORS 1

class pm_msg {
 public:
  int16_t type;
  uint16_t flags;  // optional flags: flags && PM_USE_ERRORS etc
  uint32_t vsize;  // size of included vectors
  union {
    double grid_conversion_factor;  // only used for PM_NEW_PM
    double model_fitness;
  };
  uint32_t max_dimension;  // only used for PM_NEW_PM
  uint16_t max_harmonics;  // only used for PM_NEW_PM
  uint16_t max_mem_in_MB;  // only used for PM_NEW_PM

  friend ostream& operator<<(ostream&, const pm_msg&);
};

#endif
