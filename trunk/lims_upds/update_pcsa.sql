-- 
-- update_pcsa.sql
-- 
-- Adds a couple of items needed to represent the new
--  analysis method known as PCSA
--

-- Add the new analysis method
ALTER TABLE HPCAnalysisRequest
  MODIFY COLUMN method enum('2DSA','2DSA_CG','2DSA_MW','GA','GA_MW','GA_SC','DMGA','PCSA') NOT NULL default '2DSA';

-- Alter HPCAnalysisRequestData for additional result file types
ALTER TABLE HPCAnalysisResultData
  MODIFY COLUMN HPCAnalysisResultType enum('model','noise','job_stats','mrecs','unknown') NOT NULL default 'unknown';

-- Add table structure for table PCSA_Settings
DROP TABLE IF EXISTS PCSA_Settings;

CREATE TABLE IF NOT EXISTS PCSA_Settings (
  PCSA_SettingsID int(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestID int(11) NOT NULL,
  curve_type VARCHAR(8) NOT NULL default 'SL',
  s_min double NOT NULL default '1',
  s_max double NOT NULL default '10',
  ff0_min double NOT NULL default '1',
  ff0_max double NOT NULL default '4',
  vars_count int(11) NOT NULL default 10,
  gfit_iterations int(11) NOT NULL default 3,
  curves_points int(11) NOT NULL default 200,
  thr_deltr_ratio double NOT NULL default 0.0001,
  tikreg_option tinyint(1) NOT NULL default 0,
  tikreg_alpha double NOT NULL default 0.0,
  mc_iterations int(11) NOT NULL default 1,
  tinoise_option tinyint(1) NOT NULL default '0',
  rinoise_option tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (PCSA_SettingsID),
  INDEX ndx_PCSA_Settings_HPCAnalysisRequestID (HPCAnalysisRequestID ASC),
  CONSTRAINT fk_PCSA_Settings_HPCAnalysisRequestID
    FOREIGN KEY (HPCAnalysisRequestID)
    REFERENCES HPCAnalysisRequest (HPCAnalysisRequestID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION )
ENGINE=InnoDB;

