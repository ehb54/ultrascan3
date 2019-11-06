--
-- Alter/Update DMGA_Settings to the current settings columns
--

ALTER TABLE DMGA_Settings
  CHANGE montecarlo_value  mc_iterations int(11) NOT NULL default '1';

ALTER TABLE DMGA_Settings
  CHANGE demes_value       demes         int(11) NOT NULL default '31';

ALTER TABLE DMGA_Settings
  CHANGE genes_value       genes         int(11) NOT NULL default '100';

ALTER TABLE DMGA_Settings
  CHANGE generations_value generations   int(11) NOT NULL default '100';

ALTER TABLE DMGA_Settings
  CHANGE crossover_value   mutation      int(11) NOT NULL default '50';

ALTER TABLE DMGA_Settings
  CHANGE mutation_value    crossover     int(11) NOT NULL default '50';

ALTER TABLE DMGA_Settings
  CHANGE plague_value      plague        int(11) NOT NULL default '4';

ALTER TABLE DMGA_Settings
  CHANGE elitism_value     elitism       int(11) NOT NULL default '2';

ALTER TABLE DMGA_Settings
  CHANGE migration_value   migration     int(11) NOT NULL default '3';

ALTER TABLE DMGA_Settings
  CHANGE seed_value        seed          int(11) NOT NULL default '0';

ALTER TABLE DMGA_Settings
  DROP COLUMN regularization_value

ALTER TABLE DMGA_Settings
  DROP COLUMN conc_threshold

ALTER TABLE DMGA_Settings
  ADD COLUMN p_grid int(11) NOT NULL DEFAULT '500'
  AFTER migration_ratio;

