--
-- add_timeState.sql
--
-- creates the table
--

CREATE  TABLE IF NOT EXISTS timestate (
  timestateID int(11) NOT NULL AUTO_INCREMENT ,
  experimentID int(11) NOT NULL ,
  filename varchar(255) NOT NULL default '',
  definitions longtext,
  data longblob,
  lastUpdated TIMESTAMP NULL ,
  PRIMARY KEY (timestateID) ,
  CONSTRAINT fk_timestate_experimentID
    FOREIGN KEY (experimentID)
    REFERENCES experiment (experimentID)
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;

