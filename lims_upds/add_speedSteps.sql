--
-- add_speedSteps.sql
--
-- creates the table
--

CREATE  TABLE IF NOT EXISTS speedstep (
  speedstepID int(11) NOT NULL AUTO_INCREMENT ,
  experimentID int(11) NOT NULL ,
  scans int(11) NOT NULL ,
  durationhrs int(11) NOT NULL ,
  durationmins double NOT NULL ,
  delayhrs int(11) NOT NULL ,
  delaymins double NOT NULL ,
  rotorspeed int(11) NOT NULL ,
  acceleration int(11) NOT NULL ,
  accelerflag tinyint(1) DEFAULT 1 ,
  w2tfirst float NOT NULL ,
  w2tlast float NOT NULL ,
  timefirst int(11) NOT NULL ,
  timelast int(11) NOT NULL ,
  setspeed int(11) NOT NULL ,
  avgspeed float NOT NULL ,
  speedsdev float NOT NULL ,
  PRIMARY KEY (speedstepID) ,
  CONSTRAINT fk_speedstep_experimentID
    FOREIGN KEY (experimentID)
    REFERENCES experiment (experimentID)
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;

