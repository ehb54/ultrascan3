-- 
-- update_CustomGrid.sql
-- 
-- Adds a couple of items needed to represent the new
--  analysis method known as 2DSA custom grid
--

-- Add the new analysis method
ALTER TABLE HPCAnalysisRequest
  MODIFY COLUMN method enum('2DSA','2DSA_CG','2DSA_MW','GA','GA_MW','GA_SC') NOT NULL default '2DSA';

-- Add table structure for table 2DSA_CG_Settings
DROP TABLE IF EXISTS 2DSA_CG_Settings;

CREATE TABLE IF NOT EXISTS 2DSA_CG_Settings (
  2DSA_CG_SettingsID int(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestID int(11) NOT NULL,
  CG_modelID int(11) NOT NULL,    -- maps to model with 'CustomGrid...' description
  uniform_grid int(11) NOT NULL default '6',
  mc_iterations int(11) NOT NULL default 1,
  tinoise_option tinyint(1) NOT NULL default '0',
  regularization int(11) NOT NULL default '0',
  meniscus_range double NOT NULL default '0.01',
  meniscus_points double NOT NULL default '3',
  max_iterations int(11) NOT NULL default 1,
  rinoise_option tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (2DSA_CG_SettingsID),
  INDEX ndx_2DSA_CG_Settings_HPCAnalysisRequestID (HPCAnalysisRequestID ASC),
  CONSTRAINT fk_2DSA_CG_Settings_HPCAnalysisRequestID
    FOREIGN KEY (HPCAnalysisRequestID)
    REFERENCES HPCAnalysisRequest (HPCAnalysisRequestID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION )
ENGINE=InnoDB;

