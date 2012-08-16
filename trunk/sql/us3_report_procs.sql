--
-- us3_report_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are related to the report tables
-- Run as us3admin
--

DELIMITER $$

-- Verifies that the user has permission to view or modify
--  the specified report
DROP FUNCTION IF EXISTS verify_report_permission$$
CREATE FUNCTION verify_report_permission( p_personGUID  CHAR(36),
                                          p_password    VARCHAR(80),
                                          p_reportID    INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE count_reports     INT;
  DECLARE count_permissions INT;
  DECLARE status            INT;

  CALL config();
  SET status   = @ERROR;
  SET @US3_LAST_ERROR = 'MySQL: error verifying report permission';

  SELECT COUNT(*)
  INTO   count_reports
  FROM   report
  WHERE  reportID = p_reportID;

  SELECT COUNT(*)
  INTO   count_permissions
  FROM   reportPerson
  WHERE  reportID = p_reportID
  AND    personID = @US3_ID;

  IF ( count_reports = 0 ) THEN
    SET @US3_LAST_ERRNO = @NO_REPORT;
    SET @US3_LAST_ERROR = 'MySQL: the specified report does not exist';

    SET status = @NO_REPORT;

  ELSEIF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    SET @US3_LAST_ERRNO = @OK;
    SET @US3_LAST_ERROR = '';

    SET status = @OK;

  ELSEIF ( ( verify_user( p_personGUID, p_password ) = @OK ) &&
           ( count_permissions > 0                         ) ) THEN
    SET @US3_LAST_ERRNO = @OK;
    SET @US3_LAST_ERROR = '';

    SET status = @OK;

  ELSE
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view or modify this report';

    SET status = @NOTPERMITTED;

  END IF;

  RETURN( status );

END$$

-- This set of routines deals with the global report structure

-- Returns the count of reports associated with p_ID
--  If p_ID = 0, retrieves count of all reports in db
--  Regular user can only get count of his own reports
DROP FUNCTION IF EXISTS count_reports$$
CREATE FUNCTION count_reports( p_personGUID CHAR(36),
                               p_password   VARCHAR(80),
                               p_ID         INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_reports INT;

  CALL config();
  SET count_reports = 0;

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( p_ID > 0 ) THEN
      SELECT COUNT(*)
      INTO   count_reports
      FROM   reportPerson
      WHERE  personID = p_ID;

    ELSE
      SELECT COUNT(*)
      INTO   count_reports
      FROM   reportPerson;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view those reports';
     
    ELSE
      -- This person is asking about his own reports
      -- Ignore p_ID and return user's own
      SELECT COUNT(*)
      INTO   count_reports
      FROM   reportPerson
      WHERE  personID = @US3_ID;

    END IF;
    
  END IF;

  RETURN( count_reports );

END$$

-- INSERTs a new global report structure with the specified information
DROP PROCEDURE IF EXISTS new_report$$
CREATE PROCEDURE new_report ( p_personGUID  CHAR(36),
                              p_password    VARCHAR(80),
                              p_reportGUID  CHAR(36),
                              p_runID       VARCHAR(80),
                              p_title       VARCHAR(255),
                              p_html        LONGTEXT,
                              p_ownerID     INT )
  MODIFIES SQL DATA

BEGIN
  DECLARE duplicate_key TINYINT DEFAULT 0;
  DECLARE null_field    TINYINT DEFAULT 0;
  DECLARE count_experiment  INT DEFAULT 0;
  DECLARE count_reports     INT DEFAULT 0;
  DECLARE l_experimentID    INT DEFAULT -1;

  DECLARE CONTINUE HANDLER FOR 1062
    SET duplicate_key = 1;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SET @LAST_INSERT_ID = 0;
 
  -- runID is unique for the investigator in the experiment table, too 
  SELECT COUNT(*)
  INTO   count_experiment
  FROM   experimentPerson p, experiment e
  WHERE  p.personID = p_ownerID
  AND    p.experimentID = e.experimentID
  AND    e.runID = p_runID;

  -- And now check for uniqueness of the runID for this investigator
  SELECT COUNT(*)
  INTO   count_reports
  FROM   reportPerson p, report r
  WHERE  personID = p_ownerID
  AND    p.reportID = r.reportID
  AND    runID = p_runID;

  SELECT e.experimentID
  INTO   l_experimentID
  FROM   experimentPerson p, experiment e
  WHERE  p.personID = p_ownerID
  AND    p.experimentID = e.experimentID
  AND    e.runID = p_runID;

  IF ( count_experiment = 0 ) THEN
    SET @US3_LAST_ERRNO = @NO_EXPERIMENT;
    SET @US3_LAST_ERROR = "MySQL: No experiment with that runID exists";

  ELSEIF ( count_reports > 0 ) THEN
    SET @US3_LAST_ERRNO = @INSERTDUP;
    SET @US3_LAST_ERROR = "MySQL: Duplicate entry for runID field";

  ELSEIF ( ( verify_user( p_personGUID, p_password ) = @OK ) &&
           ( check_GUID ( p_personGUID, p_password, p_reportGUID ) = @OK ) ) THEN
    INSERT INTO report SET
      reportGUID   = p_reportGUID,
      experimentID = l_experimentID,
      runID        = p_runID,
      title        = p_title,
      html         = p_html;

    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for reportGUID field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for reportGUID field";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

      INSERT INTO reportPerson SET
        reportID  = @LAST_INSERT_ID,
        personID  = p_ownerID;

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing global report structure with the specified information
DROP PROCEDURE IF EXISTS update_report$$
CREATE PROCEDURE update_report ( p_personGUID   CHAR(36),
                                 p_password     VARCHAR(80),
                                 p_reportID     INT,
                                 p_title        VARCHAR(255),
                                 p_html         LONGTEXT )
  MODIFIES SQL DATA

BEGIN
  DECLARE not_found     TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR NOT FOUND
    SET not_found = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_report_permission( p_personGUID, p_password, p_reportID ) = @OK ) THEN
    UPDATE report SET
      title       = p_title,
      html        = p_html
    WHERE reportID  = p_reportID;

    IF ( not_found = 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_REPORT;
      SET @US3_LAST_ERROR = "MySQL: No report with that ID exists";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;
      
  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns the reportID associated with the given reportGUID
DROP PROCEDURE IF EXISTS get_reportID$$
CREATE PROCEDURE get_reportID ( p_personGUID  CHAR(36),
                                p_password    VARCHAR(80),
                                p_reportGUID  CHAR(36) )
  READS SQL DATA

BEGIN

  DECLARE count_reports INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET count_reports    = 0;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN

    SELECT    COUNT(*)
    INTO      count_reports
    FROM      report
    WHERE     reportGUID = p_reportGUID;

    IF ( TRIM( p_reportGUID ) = '' ) THEN
      SET @US3_LAST_ERRNO = @EMPTY;
      SET @US3_LAST_ERROR = CONCAT( 'MySQL: The reportGUID parameter to the ',
                                    'get_reportID function cannot be empty' );

    ELSEIF ( count_reports < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   reportID
      FROM     report
      WHERE    reportGUID = p_reportGUID;

    END IF;

  END IF;

END$$

-- Returns the reportID associated with the given runID
DROP PROCEDURE IF EXISTS get_reportID_by_runID$$
CREATE PROCEDURE get_reportID_by_runID ( p_personGUID  CHAR(36),
                                         p_password    VARCHAR(80),
                                         p_ID          INT,
                                         p_runID       VARCHAR(80) )
  READS SQL DATA

BEGIN

  DECLARE count_reports INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET count_reports    = 0;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN

    SELECT COUNT(*)
    INTO   count_reports
    FROM   reportPerson p, report r
    WHERE  personID = p_ID
    AND    p.reportID = r.reportID
    AND    runID = p_runID;

    IF ( TRIM( p_runID ) = '' ) THEN
      SET @US3_LAST_ERRNO = @EMPTY;
      SET @US3_LAST_ERROR = CONCAT( 'MySQL: The runID parameter to the ',
                                    'get_reportID_by_runID function cannot be empty' );

    ELSEIF ( count_reports < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT p.reportID
      FROM   reportPerson p, report r
      WHERE  personID = p_ID
      AND    p.reportID = r.reportID
      AND    runID = p_runID;

    END IF;

  END IF;

END$$

-- Returns the reportID and title of all reports associated with p_ID
--  If p_ID = 0, retrieves information about all reports in db
--  Regular user can only get info about his own reports
DROP PROCEDURE IF EXISTS get_report_desc$$
CREATE PROCEDURE get_report_desc ( p_personGUID CHAR(36),
                                   p_password   VARCHAR(80),
                                   p_ID         INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_reports( p_personGUID, p_password, p_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      IF ( p_ID > 0 ) THEN
        SELECT    report.reportID, reportGUID, title, experimentID, runID
        FROM      reportPerson, report
        WHERE     reportPerson.personID = p_ID
        AND       reportPerson.reportID = report.reportID
        ORDER BY  reportID DESC;
   
      ELSE
        SELECT    report.reportID, reportGUID, title, experimentID, runID
        FROM      reportPerson, report
        WHERE     reportPerson.reportID = report.reportID
        ORDER BY  reportID DESC;

      END IF;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this report';
     
      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_reports( p_personGUID, p_password, @US3_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      -- Ok, user wants his own info
      SELECT @OK AS status;

      SELECT    report.reportID, reportGUID, title, experimentID, runID
      FROM      reportPerson, report
      WHERE     reportPerson.personID = @US3_ID
      AND       reportPerson.reportID = report.reportID
      ORDER BY  reportID DESC;

    END IF;

  END IF;

END$$

-- Returns the reportID and title of all reports that have a given runID
-- that belong to the person identified by p_ID
-- An admin can view anybody's reports; a regular user only his own
-- If p_ID = 0, retrieves information about all reports in db
DROP PROCEDURE IF EXISTS get_report_desc_by_runID$$
CREATE PROCEDURE get_report_desc_by_runID ( p_personGUID CHAR(36),
                                            p_password   VARCHAR(80),
                                            p_ID         INT,
                                            p_runID      VARCHAR(80) )
  READS SQL DATA

BEGIN

  CALL config();

  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_reports_by_runID( p_personGUID, p_password, p_ID, p_runID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      IF ( p_ID > 0 ) THEN
        SELECT    r.reportID, reportGUID, title, experimentID
        FROM      reportPerson p, report r
        WHERE     p.personID = p_ID
        AND       p.reportID = r.reportID
        AND       runID    = p_runID
        ORDER BY  reportID DESC;
   
      ELSE
        SELECT    reportID, reportGUID, title, experimentID
        FROM      report
        WHERE     runID = p_runID
        ORDER BY  reportID DESC;

      END IF;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this report';
     
      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_reports( p_personGUID, p_password, @US3_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      -- Ok, user wants his own info
      SELECT @OK AS status;

      SELECT    r.reportID, reportGUID, title, experimentID
      FROM      reportPerson p, report r
      WHERE     p.personID = @US3_ID
      AND       p.reportID = r.reportID
      AND       runID    = p_runID
      ORDER BY  reportID DESC;

    END IF;

  END IF;

END$$

-- Returns all global report information about one report
DROP PROCEDURE IF EXISTS get_report_info$$
CREATE PROCEDURE get_report_info ( p_personGUID  CHAR(36),
                                   p_password    VARCHAR(80),
                                   p_reportID    INT )
  READS SQL DATA

BEGIN
  DECLARE count_reports INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_reports
  FROM       report
  WHERE      reportID = p_reportID;

  IF ( verify_report_permission( p_personGUID, p_password, p_reportID ) = @OK ) THEN
    IF ( count_reports = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT    report.reportID, reportGUID, experimentID, runID, title, html
      FROM      reportPerson, report
      WHERE     report.reportID = p_reportID
      AND       report.reportID = reportPerson.reportID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- Returns all global report information about one report
DROP PROCEDURE IF EXISTS get_report_info_by_runID$$
CREATE PROCEDURE get_report_info_by_runID ( p_personGUID  CHAR(36),
                                            p_password    VARCHAR(80),
                                            p_ID          INT,
                                            p_runID       VARCHAR(80) )
  READS SQL DATA

BEGIN
  DECLARE count_reports INT;
  DECLARE l_reportID    INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT COUNT(*)
  INTO   count_reports
  FROM   reportPerson p, report r
  WHERE  personID = p_ID
  AND    p.reportID = r.reportID
  AND    runID = p_runID;

  SELECT p.reportID
  INTO   l_reportID
  FROM   reportPerson p, report r
  WHERE  personID = p_ID
  AND    p.reportID = r.reportID
  AND    runID = p_runID;

  IF ( count_reports = 0 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( verify_report_permission( p_personGUID, p_password, l_reportID ) = @OK ) THEN
    SELECT @OK AS status;

    SELECT    report.reportID, reportGUID, experimentID, runID, title, html
    FROM      reportPerson, report
    WHERE     report.reportID = l_reportID
    AND       report.reportID = reportPerson.reportID;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- DELETEs a report, plus information in related tables
DROP PROCEDURE IF EXISTS delete_report$$
CREATE PROCEDURE delete_report ( p_personGUID  CHAR(36),
                                 p_password    VARCHAR(80),
                                 p_reportID    INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_report_permission( p_personGUID, p_password, p_reportID ) = @OK ) THEN

    -- Make sure records match if they have related tables or not
    -- Have to do it in a couple of stages because of the constraints
    DELETE      documentLink, reportDocument
    FROM        report
    LEFT JOIN   reportTriple   ON ( report.reportID = reportTriple.reportID )
    LEFT JOIN   documentLink   ON ( reportTriple.reportTripleID = documentLink.reportTripleID )
    LEFT JOIN   reportDocument ON ( documentLink.reportDocumentID = reportDocument.reportDocumentID )
    WHERE       report.reportID = p_reportID;

    DELETE      reportTriple
    FROM        report
    LEFT JOIN   reportTriple ON ( report.reportID = reportTriple.reportID )
    WHERE       report.reportID = p_reportID;

    DELETE FROM report
    WHERE reportID = p_reportID;
    
    DELETE FROM reportPerson
    WHERE reportID = p_reportID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- This set of routines deals with all documents relating to a single 
-- runID/triple combination

DROP FUNCTION IF EXISTS count_reportTriple$$
CREATE FUNCTION count_reportTriple( p_personGUID CHAR(36),
                                    p_password   VARCHAR(80),
                                    p_reportID   INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_reports INT;

  CALL config();
  SET count_reports = 0;

  IF ( verify_report_permission( p_personGUID, p_password, p_reportID ) = @OK ) THEN
    -- This is either an admin, or a user getting info about his own reports
    SELECT COUNT(*)
    INTO   count_reports
    FROM   reportTriple
    WHERE  reportID = p_reportID;

  END IF;

  RETURN( count_reports );

END$$

-- INSERTs a new report triple record with the specified information
DROP PROCEDURE IF EXISTS new_reportTriple$$
CREATE PROCEDURE new_reportTriple ( p_personGUID        CHAR(36),
                                    p_password          VARCHAR(80),
                                    p_reportTripleGUID  CHAR(36),
                                    p_reportID          INT,
                                    p_resultID          INT,
                                    p_triple            VARCHAR(20),
                                    p_dataDescription   VARCHAR(255) )
  MODIFIES SQL DATA

BEGIN
  DECLARE duplicate_key TINYINT DEFAULT 0;
  DECLARE null_field    TINYINT DEFAULT 0;
  DECLARE constraint_failed TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1062
    SET duplicate_key = 1;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  DECLARE CONTINUE HANDLER FOR 1452
    SET constraint_failed = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SET @LAST_INSERT_ID = 0;
 
  IF ( ( verify_report_permission( p_personGUID, p_password, p_reportID ) = @OK ) &&
       ( check_GUID ( p_personGUID, p_password, p_reportTripleGUID ) = @OK )      ) THEN
    INSERT INTO reportTriple SET
      reportTripleGUID  = p_reportTripleGUID,
      reportID          = p_reportID,
      resultID          = p_resultID,
      triple            = p_triple,
      dataDescription   = p_dataDescription ;

    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for reportTripleGUID field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for reportTripleGUID field";

    ELSEIF ( constraint_failed = 1 ) THEN
      SET @US3_LAST_ERRNO = @CONSTRAINT_FAILED;
      SET @US3_LAST_ERROR = "MySQL: FK Constraint failed inserting into reportTriple";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing report triple record with the specified information
DROP PROCEDURE IF EXISTS update_reportTriple$$
CREATE PROCEDURE update_reportTriple ( p_personGUID        CHAR(36),
                                       p_password          VARCHAR(80),
                                       p_reportTripleID    INT,
                                       p_resultID          INT,
                                       p_triple            VARCHAR(20),
                                       p_dataDescription   VARCHAR(255) )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_reportID    INT;
  DECLARE not_found     TINYINT DEFAULT 0;
  DECLARE constraint_failed TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR NOT FOUND
    SET not_found = 1;

  DECLARE CONTINUE HANDLER FOR 1452
    SET constraint_failed = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SET @LAST_INSERT_ID = 0;

  -- Figure out the reportID
  SELECT reportID
  INTO   l_reportID
  FROM   reportTriple
  WHERE  reportTripleID = p_reportTripleID;

  IF ( verify_report_permission( p_personGUID, p_password, l_reportID ) != @OK ) THEN
    -- Can't do that
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to edit this report';
    
    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    -- Let's guard against updating dataDescription with an empty string
    SET p_dataDescription = TRIM( p_dataDescription );

    IF ( LENGTH( p_dataDescription ) < 1 ) THEN
      UPDATE reportTriple SET
        resultID          = p_resultID,
        triple            = p_triple
      WHERE reportTripleID = p_reportTripleID;

    ELSE
      UPDATE reportTriple SET
        resultID          = p_resultID,
        triple            = p_triple,
        dataDescription   = p_dataDescription 
      WHERE reportTripleID = p_reportTripleID;
    END IF;

    IF ( not_found = 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_REPORT_TRIPLE;
      SET @US3_LAST_ERROR = "MySQL: No report triple record with that ID exists";

    ELSEIF ( constraint_failed = 1 ) THEN
      SET @US3_LAST_ERRNO = @CONSTRAINT_FAILED;
      SET @US3_LAST_ERROR = "MySQL: FK Constraint failed while updating reportTriple";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns the reportTripleID associated with the given reportTripleGUID
DROP PROCEDURE IF EXISTS get_reportTripleID$$
CREATE PROCEDURE get_reportTripleID ( p_personGUID        CHAR(36),
                                      p_password          VARCHAR(80),
                                      p_reportTripleGUID  CHAR(36) )
  READS SQL DATA

BEGIN

  DECLARE count_reports INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET count_reports    = 0;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN

    SELECT    COUNT(*)
    INTO      count_reports
    FROM      reportTriple
    WHERE     reportTripleGUID = p_reportTripleGUID;

    IF ( TRIM( p_reportTripleGUID ) = '' ) THEN
      SET @US3_LAST_ERRNO = @EMPTY;
      SET @US3_LAST_ERROR = CONCAT( 'MySQL: The reportTripleGUID parameter to the ',
                                    'get_reportTripleID function cannot be empty' );

    ELSEIF ( count_reports < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   reportTripleID
      FROM     reportTriple
      WHERE    reportTripleGUID = p_reportTripleGUID;

    END IF;

  END IF;

END$$

-- Returns all report details associated with p_reportID
DROP PROCEDURE IF EXISTS get_reportTriple_desc$$
CREATE PROCEDURE get_reportTriple_desc ( p_personGUID CHAR(36),
                                         p_password   VARCHAR(80),
                                         p_reportID   INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_report_permission( p_personGUID, p_password, p_reportID ) != @OK ) THEN
     SET @US3_LAST_ERRNO = @NOTPERMITTED;
     SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this report';
     
     SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_reportTriple( p_personGUID, p_password, p_reportID ) < 1 ) THEN
     SET @US3_LAST_ERRNO = @NOROWS;
     SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
     SELECT @US3_LAST_ERRNO AS status;

  ELSE
     SELECT @OK AS status;

     SELECT    reportTripleID, reportTripleGUID, resultID, triple, dataDescription
     FROM      reportTriple
     WHERE     reportID = p_reportID
     ORDER BY  triple;
   
  END IF;

END$$

-- Returns all report details associated with a single p_reportTripleID
DROP PROCEDURE IF EXISTS get_reportTriple_info$$
CREATE PROCEDURE get_reportTriple_info ( p_personGUID       CHAR(36),
                                         p_password         VARCHAR(80),
                                         p_reportTripleID   INT )
  READS SQL DATA

BEGIN
  DECLARE l_reportID    INT;
  DECLARE count_reports INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT reportID
  INTO   l_reportID
  FROM   reportTriple
  WHERE  reportTripleID = p_reportTripleID;

  SELECT COUNT(*)
  INTO   count_reports
  FROM   reportTriple
  WHERE  reportTripleID = p_reportTripleID;

  IF ( verify_report_permission( p_personGUID, p_password, l_reportID ) != @OK ) THEN
     SET @US3_LAST_ERRNO = @NOTPERMITTED;
     SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this report';
     
     SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_reports < 1 ) THEN
     SET @US3_LAST_ERRNO = @NOROWS;
     SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
     SELECT @US3_LAST_ERRNO AS status;

  ELSE
     SELECT @OK AS status;

     SELECT    reportTripleGUID, resultID, triple, dataDescription
     FROM      reportTriple
     WHERE     reportTripleID = p_reportTripleID;
   
  END IF;

END$$

-- DELETEs a report detail record, plus information in related tables
DROP PROCEDURE IF EXISTS delete_reportTriple$$
CREATE PROCEDURE delete_reportTriple ( p_personGUID        CHAR(36),
                                       p_password          VARCHAR(80),
                                       p_reportTripleID    INT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_reportID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT reportID
  INTO   l_reportID
  FROM   reportTriple
  WHERE  reportTripleID = p_reportTripleID;

  IF ( verify_report_permission( p_personGUID, p_password, l_reportID ) = @OK ) THEN

    -- Make sure records match if they have related tables or not
    -- Have to do it in a couple of stages because of the constraints
    DELETE      documentLink, reportDocument
    FROM        reportTriple
    LEFT JOIN   documentLink   ON ( reportTriple.reportTripleID = documentLink.reportTripleID )
    LEFT JOIN   reportDocument ON ( documentLink.reportDocumentID = reportDocument.reportDocumentID )
    WHERE       reportTriple.reportTripleID = p_reportTripleID;

    DELETE FROM reportTriple
    WHERE       reportTripleID = p_reportTripleID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- This set of routines deals with one or more documents

DROP FUNCTION IF EXISTS count_reportDocument$$
CREATE FUNCTION count_reportDocument( p_personGUID       CHAR(36),
                                      p_password         VARCHAR(80),
                                      p_reportTripleID   INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE l_reportID    INT;
  DECLARE count_reports INT;

  CALL config();
  SET count_reports = 0;

  SELECT reportID
  INTO   l_reportID
  FROM   reportTriple
  WHERE  reportTripleID = p_reportTripleID;

  IF ( verify_report_permission( p_personGUID, p_password, l_reportID ) = @OK ) THEN
    -- This is either an admin, or a user getting info about his own reports
    SELECT COUNT(*)
    INTO   count_reports
    FROM   documentLink
    WHERE  reportTripleID = p_reportTripleID;

  END IF;

  RETURN( count_reports );

END$$

-- INSERTs a new report document with the specified information
DROP PROCEDURE IF EXISTS new_reportDocument$$
CREATE PROCEDURE new_reportDocument ( p_personGUID          CHAR(36),
                                      p_password            VARCHAR(80),
                                      p_reportTripleID      INT,
                                      p_reportDocumentGUID  CHAR(36),
                                      p_editedDataID        INT,
                                      p_label               VARCHAR(160),
                                      p_filename            VARCHAR(255),
                                      p_analysis            VARCHAR(20),
                                      p_subAnalysis         VARCHAR(20),
                                      p_documentType        VARCHAR(20) )

  MODIFIES SQL DATA

BEGIN
  DECLARE l_reportID    INT;
  DECLARE l_documentID  INT;

  DECLARE duplicate_key TINYINT DEFAULT 0;
  DECLARE null_field    TINYINT DEFAULT 0;
  DECLARE constraint_failed TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1062
    SET duplicate_key = 1;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  DECLARE CONTINUE HANDLER FOR 1452
    SET constraint_failed = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SET @LAST_INSERT_ID = 0;
 
  SELECT reportID
  INTO   l_reportID
  FROM   reportTriple
  WHERE  reportTripleID = p_reportTripleID;

  IF ( ( verify_report_permission( p_personGUID, p_password, l_reportID ) = @OK ) &&
       ( check_GUID ( p_personGUID, p_password, p_reportDocumentGUID ) = @OK )    &&
       ( verify_editData_permission( p_personGUID, p_password, p_editedDataID ) = @OK ) ) THEN
    INSERT INTO reportDocument SET
      reportDocumentGUID  = p_reportDocumentGUID,
      editedDataID        = p_editedDataID,
      label               = p_label,
      filename            = p_filename,
      analysis            = p_analysis,
      subAnalysis         = p_subAnalysis,
      documentType        = p_documentType;

    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for reportDocumentGUID field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for reportDocumentGUID field";

    ELSEIF ( constraint_failed = 1 ) THEN
      SET @US3_LAST_ERRNO = @CONSTRAINT_FAILED;
      SET @US3_LAST_ERROR = "MySQL: FK Constraint failed while inserting into reportDocument";

    ELSE
      SET @US3_LAST_ERRNO = @OK;
      SET @US3_LAST_ERROR = '';

      SET l_documentID = LAST_INSERT_ID();

      INSERT INTO documentLink SET
        reportTripleID   = p_reportTripleID,
        reportDocumentID = l_documentID;

      SET @LAST_INSERT_ID = l_documentID;

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing report document with the specified information
DROP PROCEDURE IF EXISTS update_reportDocument$$
CREATE PROCEDURE update_reportDocument ( p_personGUID          CHAR(36),
                                         p_password            VARCHAR(80),
                                         p_reportDocumentID    INT,
                                         p_editedDataID        INT,
                                         p_label               VARCHAR(160),
                                         p_filename            VARCHAR(255),
                                         p_analysis            VARCHAR(20),
                                         p_subAnalysis         VARCHAR(20),
                                         p_documentType        VARCHAR(20) )

  MODIFIES SQL DATA

BEGIN
  DECLARE l_reportID    INT;
  DECLARE l_countLinks  INT;

  DECLARE constraint_failed TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1452
    SET constraint_failed = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SET @LAST_INSERT_ID = 0;
 
  SELECT COUNT(*)
  INTO   l_countLinks
  FROM   documentLink
  WHERE  reportDocumentID = p_reportDocumentID;
    
  SELECT reportID
  INTO   l_reportID
  FROM   documentLink, reportTriple
  WHERE  documentLink.reportDocumentID = p_reportDocumentID
  AND    documentLink.reportTripleID = reportTriple.reportTripleID;

  IF ( l_countLinks < 1 ) THEN
    SET @US3_LAST_ERRNO = @NO_DOCUMENT_LINK;
    SET @US3_LAST_ERROR = CONCAT('MySQL: The link between the triple and the document is missing; ',
                                 'Report document ID = ', p_reportDocumentID, '; ',
                                 'Link count = ', l_countLinks );
    
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( verify_report_permission( p_personGUID, p_password, l_reportID ) != @OK ) THEN
    -- Can't do that
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to edit this report';
    
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( verify_editData_permission( p_personGUID, p_password, p_editedDataID ) != @OK ) THEN
    -- Don't have permission to reference this edit profile
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to use that edit profile';
    
    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    UPDATE reportDocument SET
      editedDataID         = p_editedDataID,
      label                = p_label,
      filename             = p_filename,
      analysis             = p_analysis,
      subAnalysis          = p_subAnalysis,
      documentType         = p_documentType
    WHERE reportDocumentID = p_reportDocumentID;

    IF ( constraint_failed = 1 ) THEN
      SET @US3_LAST_ERRNO = @CONSTRAINT_FAILED;
      SET @US3_LAST_ERROR = "MySQL: FK Constraint failed while updating reportDocument";

    ELSE
      SET @US3_LAST_ERRNO = @OK;
      SET @US3_LAST_ERROR = '';

      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns the reportDocumentID associated with the given reportDocumentGUID
DROP PROCEDURE IF EXISTS get_reportDocumentID$$
CREATE PROCEDURE get_reportDocumentID ( p_personGUID        CHAR(36),
                                        p_password          VARCHAR(80),
                                        p_reportDocumentGUID  CHAR(36) )
  READS SQL DATA

BEGIN

  DECLARE count_reports INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET count_reports    = 0;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN

    SELECT    COUNT(*)
    INTO      count_reports
    FROM      reportDocument
    WHERE     reportDocumentGUID = p_reportDocumentGUID;

    IF ( TRIM( p_reportDocumentGUID ) = '' ) THEN
      SET @US3_LAST_ERRNO = @EMPTY;
      SET @US3_LAST_ERROR = CONCAT( 'MySQL: The reportDocumentGUID parameter to the ',
                                    'get_reportDocumentID function cannot be empty' );

    ELSEIF ( count_reports < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   reportDocumentID
      FROM     reportDocument
      WHERE    reportDocumentGUID = p_reportDocumentGUID;

    END IF;

  END IF;

END$$

-- Returns most info about all reports associated with p_reportTripleID
DROP PROCEDURE IF EXISTS get_reportDocument_desc$$
CREATE PROCEDURE get_reportDocument_desc ( p_personGUID      CHAR(36),
                                           p_password        VARCHAR(80),
                                           p_reportTripleID  INT )
  READS SQL DATA

BEGIN
  DECLARE l_reportID    INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT reportID
  INTO   l_reportID
  FROM   reportTriple
  WHERE  reportTripleID = p_reportTripleID;

  IF ( verify_report_permission( p_personGUID, p_password, l_reportID ) != @OK ) THEN
     SET @US3_LAST_ERRNO = @NOTPERMITTED;
     SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this report';
     
     SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_reportDocument( p_personGUID, p_password, p_reportTripleID ) < 1 ) THEN
     SET @US3_LAST_ERRNO = @NOROWS;
     SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
     SELECT @US3_LAST_ERRNO AS status;

  ELSE
     SELECT @OK AS status;

     SELECT    reportDocument.reportDocumentID, reportDocumentGUID, editedDataID, 
               label, filename, analysis, subAnalysis, documentType
     FROM      documentLink, reportDocument
     WHERE     reportTripleID = p_reportTripleID
     AND       documentLink.reportDocumentID = reportDocument.reportDocumentID
     ORDER BY  label;
   
  END IF;

END$$

-- Returns all report details associated with a single p_reportDocumentID
DROP PROCEDURE IF EXISTS get_reportDocument_info$$
CREATE PROCEDURE get_reportDocument_info ( p_personGUID      CHAR(36),
                                           p_password        VARCHAR(80),
                                           p_reportDocumentID  INT )
  READS SQL DATA

BEGIN
  DECLARE l_reportID       INT;
  DECLARE l_reportTripleID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT reportID, documentLink.reportTripleID
  INTO   l_reportID, l_reportTripleID
  FROM   documentLink, reportTriple
  WHERE  documentLink.reportDocumentID = p_reportDocumentID
  AND    documentLink.reportTripleID = reportTriple.reportTripleID;

  IF ( verify_report_permission( p_personGUID, p_password, l_reportID ) != @OK ) THEN
     SET @US3_LAST_ERRNO = @NOTPERMITTED;
     SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this report';
     
     SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_reportDocument( p_personGUID, p_password, l_reportTripleID ) < 1 ) THEN
     SET @US3_LAST_ERRNO = @NOROWS;
     SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
     SELECT @US3_LAST_ERRNO AS status;

  ELSE
     SELECT @OK AS status;

     SELECT    reportDocumentGUID, editedDataID, label, filename, 
               analysis, subAnalysis, documentType
     FROM      reportDocument
     WHERE     reportDocumentID = p_reportDocumentID
     ORDER BY  label;
   
  END IF;

END$$

-- UPDATEs a reportDocument record with the document content data
DROP PROCEDURE IF EXISTS upload_reportContents$$
CREATE PROCEDURE upload_reportContents ( p_personGUID       CHAR(36),
                                         p_password         VARCHAR(80),
                                         p_reportDocumentID INT,
                                         p_contents         LONGBLOB,
                                         p_checksum         CHAR(33) )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_checksum     CHAR(33);
  DECLARE l_reportID     INT;
  DECLARE not_found      TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR NOT FOUND
    SET not_found = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  -- Compare checksum with calculated checksum
  SET l_checksum = MD5( p_contents );
  SET @DEBUG = CONCAT( l_checksum , ' ', p_checksum );

  -- Get information we need to verify ownership
  SELECT reportID
  INTO   l_reportID
  FROM   reportTriple, documentLink
  WHERE  reportDocumentID = p_reportDocumentID
  AND    reportTriple.reportTripleID = documentLink.reportTripleID;

  IF ( l_checksum != p_checksum ) THEN

    -- Checksums don't match; abort
    SET @US3_LAST_ERRNO = @BAD_CHECKSUM;
    SET @US3_LAST_ERROR = "MySQL: Transmission error, bad checksum";

  ELSEIF ( verify_report_permission( p_personGUID, p_password, l_reportID ) = @OK ) THEN
 
    -- This is either an admin, or a person inquiring about his own experiment
    UPDATE reportDocument SET
           contents  = p_contents
    WHERE  reportDocumentID = p_reportDocumentID;

    IF ( not_found = 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_REPORT_DOCUMENT;
      SET @US3_LAST_ERROR = "MySQL: No report document with that ID exists";

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- SELECTs a reportDocument record of binary data previously saved with upload_reportContents
DROP PROCEDURE IF EXISTS download_reportContents$$
CREATE PROCEDURE download_reportContents ( p_personGUID       CHAR(36),
                                           p_password         VARCHAR(80),
                                           p_reportDocumentID INT )
  READS SQL DATA

BEGIN
  DECLARE l_count_reportContents INT;
  DECLARE l_reportID             INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  -- Get information to verify that there are records
  SELECT COUNT(*)
  INTO   l_count_reportContents
  FROM   reportDocument
  WHERE  reportDocumentID = p_reportDocumentID;

SET @DEBUG = CONCAT('Report document ID = ', p_reportDocumentID,
                    'Count = ', l_count_reportContents );

  -- Get information we need to verify ownership
  SELECT reportID
  INTO   l_reportID
  FROM   reportTriple, documentLink
  WHERE  reportDocumentID = p_reportDocumentID
  AND    reportTriple.reportTripleID = documentLink.reportTripleID;

  IF ( l_count_reportContents != 1 ) THEN
    -- Probably no rows
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows exist with that ID (or too many rows)';

    SELECT @NOROWS AS status;
    
  ELSEIF ( verify_report_permission( p_personGUID, p_password, l_reportID ) != @OK ) THEN
 
    -- verify_experiment_permission must have thrown an error, so pass it on
    SELECT @US3_LAST_ERRNO AS status;

  ELSE

    -- This is either an admin, or a person inquiring about his own experiment
    SELECT @OK AS status;

    SELECT contents, MD5( contents )
    FROM   reportDocument
    WHERE  reportDocumentID = p_reportDocumentID;

  END IF;

END$$

-- DELETEs a report detail record, plus information in related tables
DROP PROCEDURE IF EXISTS delete_reportDocument$$
CREATE PROCEDURE delete_reportDocument ( p_personGUID        CHAR(36),
                                         p_password          VARCHAR(80),
                                         p_reportDocumentID  INT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_reportCount    INT;
  DECLARE l_reportID       INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT COUNT(*)
  INTO   l_reportCount
  FROM   reportDocument
  WHERE  reportDocumentID = p_reportDocumentID;

  SELECT reportID
  INTO   l_reportID
  FROM   documentLink, reportTriple
  WHERE  reportDocumentID = p_reportDocumentID
  AND    documentLink.reportTripleID = reportTriple.reportTripleID;

  IF ( l_reportCount != 1 ) THEN
    -- Can't find the report document
    SET @US3_LAST_ERRNO = @NO_REPORT_DOCUMENT;
    SET @US3_LAST_ERROR = 'MySQL: No report document with that ID exists';

  ELSEIF ( verify_report_permission( p_personGUID, p_password, l_reportID ) = @OK ) THEN

    -- Make sure records match if they have related tables or not
    -- Have to do it in a couple of stages because of the constraints
    DELETE FROM reportDocument 
    WHERE       reportDocumentID = p_reportDocumentID;

    DELETE FROM documentLink 
    WHERE       reportDocumentID = p_reportDocumentID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$
