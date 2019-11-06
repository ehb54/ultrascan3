--
-- update-report-tables.sql
--
-- update report table structure to go with release of phase 1 US_Report class
-- 
SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS;
SET @OLD_SQL_MODE=@@SQL_MODE;

SET UNIQUE_CHECKS = 0;
SET FOREIGN_KEY_CHECKS=0;
SET SQL_MODE='STRICT_TRANS_TABLES,STRICT_ALL_TABLES,NO_ZERO_IN_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER';

--
-- Table structure for table report
--

DROP TABLE IF EXISTS report;
CREATE TABLE report (
  reportID int(11) NOT NULL AUTO_INCREMENT,
  reportGUID char(36) NOT NULL UNIQUE,
  experimentID int(11) NOT NULL,
  runID varchar(80) NOT NULL,
  title varchar(255) NOT NULL DEFAULT '',
  html longtext,
  PRIMARY KEY (reportID),
  INDEX ndx_report_experimentID (experimentID),
  CONSTRAINT fk_report_experimentID 
    FOREIGN KEY (experimentID) 
    REFERENCES experiment (experimentID) 
    ON DELETE CASCADE 
    ON UPDATE CASCADE
) ENGINE=InnoDB;

--
-- Table structure for table reportPerson
--

DROP TABLE IF EXISTS reportPerson;
CREATE TABLE reportPerson (
  reportID int(11) NOT NULL,
  personID int(11) NOT NULL,
  PRIMARY KEY (reportID),
  INDEX ndx_reportPerson_personID (personID),
  INDEX ndx_reportPerson_reportID (reportID),
  CONSTRAINT fk_reportPerson_personID 
    FOREIGN KEY (personID) 
    REFERENCES people (personID) 
    ON DELETE CASCADE 
    ON UPDATE CASCADE,
  CONSTRAINT fk_reportPerson_reportID 
    FOREIGN KEY (reportID) 
    REFERENCES report (reportID) 
    ON DELETE CASCADE 
    ON UPDATE CASCADE
) ENGINE=InnoDB;

--
-- Table structure for table reportTriple
--

DROP TABLE IF EXISTS reportDetail;
DROP TABLE IF EXISTS reportTriple;
CREATE TABLE reportTriple (
  reportTripleID int(11) NOT NULL AUTO_INCREMENT,
  reportTripleGUID char(36) NOT NULL UNIQUE,
  reportID int(11) NOT NULL,
  resultID int(11) DEFAULT NULL,
  triple varchar(20) NOT NULL DEFAULT '',     -- cell/channel/wavelength format
  PRIMARY KEY (reportTripleID),
  INDEX ndx_reportTriple_reportID (reportID),
  CONSTRAINT fk_reportTriple_reportID 
    FOREIGN KEY (reportID) 
    REFERENCES report (reportID) 
    ON DELETE CASCADE 
    ON UPDATE CASCADE
) ENGINE=InnoDB;

--
-- Table structure for table documentLink
--

DROP TABLE IF EXISTS documentLink;
CREATE TABLE documentLink (
  documentLinkID int(11) NOT NULL AUTO_INCREMENT,
  reportTripleID int(11) NOT NULL,
  reportDocumentID int(11) NOT NULL,
  PRIMARY KEY (documentLinkID),
  INDEX ndx_documentLink_reportTripleID (reportTripleID),
  INDEX ndx_documentLink_reportDocumentID (reportDocumentID),
  CONSTRAINT fk_documentLink_reportDocumentID 
    FOREIGN KEY (reportDocumentID) 
    REFERENCES reportDocument (reportDocumentID) 
    ON DELETE CASCADE 
    ON UPDATE CASCADE,
  CONSTRAINT fk_documentLink_reportTripleID 
    FOREIGN KEY (reportTripleID) 
    REFERENCES reportTriple (reportTripleID) 
    ON DELETE CASCADE 
    ON UPDATE CASCADE
) ENGINE=InnoDB;

--
-- Table structure for table reportDocument
--

DROP TABLE IF EXISTS reportDocument;
CREATE TABLE reportDocument (
  reportDocumentID int(11) NOT NULL AUTO_INCREMENT,
  reportDocumentGUID char(36) NOT NULL UNIQUE,
  editedDataID int(11) NOT NULL DEFAULT '1',
  label varchar(80) NOT NULL DEFAULT '',
  filename varchar(255) NOT NULL DEFAULT '',
  analysis varchar(20) DEFAULT '2DSA',
  subAnalysis varchar(20) DEFAULT 'report',
  documentType varchar(20) DEFAULT 'png',
  contents longblob,
  PRIMARY KEY (reportDocumentID),
  INDEX ndx_reportDocument_editedDataID (editedDataID),
  CONSTRAINT fk_reportDocument_editedDataID 
    FOREIGN KEY (editedDataID) 
    REFERENCES editedData (editedDataID) 
    ON UPDATE NO ACTION
) ENGINE=InnoDB;

SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
