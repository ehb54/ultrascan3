-- 
-- update_dmga.sql
-- 
-- Adds a couple of items needed to represent the new
--  analysis method known as DMGA
--

-- Add the new analysis method
ALTER TABLE HPCAnalysisRequest
  MODIFY COLUMN method enum('2DSA','2DSA_CG','2DSA_MW','GA','GA_MW','GA_SC','DMGA') NOT NULL default '2DSA';

-- Add table structure for table DMGA_Settings
DROP TABLE IF EXISTS DMGA_Settings;

CREATE TABLE IF NOT EXISTS DMGA_Settings (
  DMGA_SettingsID int(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestID int(11) NOT NULL,
  DC_modelID int(11) NOT NULL,    -- maps to model with 'DMGA_Constr..' description
  mc_iterations int(11) NOT NULL default '0',
  demes int(11) NOT NULL default '31',
  population int(11) NOT NULL default '100',
  generations int(11) NOT NULL default '100',
  mutation int(11) NOT NULL default '50',
  crossover int(11) NOT NULL default '50',
  plague int(11) NOT NULL default '4',
  elitism int(11) NOT NULL default '2',
  migration int(11) NOT NULL default '3',
  p_grid int(11) NOT NULL default '500',
  seed int(11) NOT NULL default '0',
  PRIMARY KEY  (DMGA_SettingsID),
  INDEX ndx_DMGA_Settings_HPCAnalysisRequestID (HPCAnalysisRequestID ASC),
  CONSTRAINT fk_DMGA_Settings_HPCAnalysisRequestID
    FOREIGN KEY (HPCAnalysisRequestID)
    REFERENCES HPCAnalysisRequest (HPCAnalysisRequestID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION )
ENGINE=InnoDB;

