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
                              p_title       VARCHAR(255),
                              p_html        LONGTEXT,
                              p_ownerID     INT )
  MODIFIES SQL DATA

BEGIN
  DECLARE duplicate_key TINYINT DEFAULT 0;
  DECLARE null_field    TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1062
    SET duplicate_key = 1;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SET @LAST_INSERT_ID = 0;
 
  IF ( ( verify_user( p_personGUID, p_password ) = @OK ) &&
       ( check_GUID ( p_personGUID, p_password, p_reportGUID ) = @OK ) ) THEN
    INSERT INTO report SET
      reportGUID  = p_reportGUID,
      title       = p_title,
      html        = p_html;

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
                                 p_reportGUID   CHAR(36),
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
      reportGUID  = p_reportGUID,
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
        SELECT    report.reportID, reportGUID, title, html
        FROM      reportPerson, report
        WHERE     reportPerson.personID = p_ID
        AND       reportPerson.reportID = report.reportID
        ORDER BY  reportID DESC;
   
      ELSE
        SELECT    report.reportID, reportGUID, title, html
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

      SELECT    report.reportID, reportGUID, title, html
      FROM      reportPerson, report
      WHERE     reportPerson.personID = @US3_ID
      AND       reportPerson.reportID = report.reportID
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

      SELECT    report.reportID, reportGUID, title, html
      FROM      reportPerson, report
      WHERE     report.reportID = p_reportID
      AND       report.reportID = reportPerson.reportID;

    END IF;

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
    LEFT JOIN   reportDetail   ON ( report.reportID = reportDetail.reportID )
    LEFT JOIN   documentLink   ON ( reportDetail.reportDetailID = documentLink.reportDetailID )
    LEFT JOIN   reportDocument ON ( documentLink.reportDocumentID = reportDocument.reportDocumentID )
    WHERE       report.reportID = p_reportID;

    DELETE      reportDetail
    FROM        report
    LEFT JOIN   reportDetail ON ( report.reportID = reportDetail.reportID )
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

DROP FUNCTION IF EXISTS count_reportDetail$$
CREATE FUNCTION count_reportDetail( p_personGUID CHAR(36),
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
    FROM   reportDetail
    WHERE  reportID = p_reportID;

  END IF;

  RETURN( count_reports );

END$$

-- INSERTs a new report detail record with the specified information
DROP PROCEDURE IF EXISTS new_reportDetail$$
CREATE PROCEDURE new_reportDetail ( p_personGUID        CHAR(36),
                                    p_password          VARCHAR(80),
                                    p_reportDetailGUID  CHAR(36),
                                    p_reportID          INT,
                                    p_resultID          INT,
                                    p_experimentID      INT,
                                    p_runID             VARCHAR(80),
                                    p_triple            VARCHAR(20) )
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
 
  IF ( verify_report_permission( p_personGUID, p_password, p_reportID ) != @OK ) THEN
    -- Can't do that
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to edit this report';
    
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) != @OK ) THEN
    -- Doesn't have permission to use this experiment
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view that experiment';
    
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( check_GUID ( p_personGUID, p_password, p_reportDetailGUID ) != @OK ) THEN
    SET @US3_LAST_ERRNO = @BADGUID;
    SET @US3_LAST_ERROR = 'MySQL: the reportDetailGUID is not in the correct format';

    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    INSERT INTO reportDetail SET
      reportDetailGUID  = p_reportDetailGUID,
      reportID          = p_reportID,
      resultID          = p_resultID,
      experimentID      = p_experimentID,
      runID             = p_runID,
      triple            = p_triple;

    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for reportDetailGUID field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for reportDetailGUID field";

    ELSEIF ( constraint_failed = 1 ) THEN
      SET @US3_LAST_ERRNO = @CONSTRAINT_FAILED;
      SET @US3_LAST_ERROR = "MySQL: FK Constraint failed inserting into reportDetail";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing report detail record with the specified information
DROP PROCEDURE IF EXISTS update_reportDetail$$
CREATE PROCEDURE update_reportDetail ( p_personGUID        CHAR(36),
                                       p_password          VARCHAR(80),
                                       p_reportDetailID    INT,
                                       p_reportDetailGUID  CHAR(36),
                                       p_resultID          INT,
                                       p_experimentID      INT,
                                       p_runID             VARCHAR(80),
                                       p_triple            VARCHAR(20) )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_reportID    INT;
  DECLARE not_found     TINYINT DEFAULT 0;
  DECLARE duplicate_key TINYINT DEFAULT 0;
  DECLARE null_field    TINYINT DEFAULT 0;
  DECLARE constraint_failed TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR NOT FOUND
    SET not_found = 1;

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

  -- Figure out the reportID
  SELECT reportID
  INTO   l_reportID
  FROM   reportDetail
  WHERE  reportDetailID = p_reportDetailID;

  IF ( verify_report_permission( p_personGUID, p_password, l_reportID ) != @OK ) THEN
    -- Can't do that
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to edit this report';
    
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) != @OK ) THEN
    -- Doesn't have permission to use this experiment
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view that experiment';
    
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( check_GUID ( p_personGUID, p_password, p_reportDetailGUID ) != @OK ) THEN
    SET @US3_LAST_ERRNO = @BADGUID;
    SET @US3_LAST_ERROR = 'MySQL: the reportDetailGUID is not in the correct format';

    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    UPDATE reportDetail SET
      reportDetailGUID  = p_reportDetailGUID,
      resultID          = p_resultID,
      experimentID      = p_experimentID,
      runID             = p_runID,
      triple            = p_triple
    WHERE reportDetailID = p_reportDetailID;

    IF ( not_found = 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_REPORT_DETAIL;
      SET @US3_LAST_ERROR = "MySQL: No report detail record with that ID exists";

    ELSEIF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for reportDetailGUID field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for reportDetailGUID field";

    ELSEIF ( constraint_failed = 1 ) THEN
      SET @US3_LAST_ERRNO = @CONSTRAINT_FAILED;
      SET @US3_LAST_ERROR = "MySQL: FK Constraint failed while updating reportDetail";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns the reportDetailID associated with the given reportDetailGUID
DROP PROCEDURE IF EXISTS get_reportDetailID$$
CREATE PROCEDURE get_reportDetailID ( p_personGUID        CHAR(36),
                                      p_password          VARCHAR(80),
                                      p_reportDetailGUID  CHAR(36) )
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
    FROM      reportDetail
    WHERE     reportDetailGUID = p_reportDetailGUID;

    IF ( TRIM( p_reportDetailGUID ) = '' ) THEN
      SET @US3_LAST_ERRNO = @EMPTY;
      SET @US3_LAST_ERROR = CONCAT( 'MySQL: The reportDetailGUID parameter to the ',
                                    'get_reportDetailID function cannot be empty' );

    ELSEIF ( count_reports < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   reportDetailID
      FROM     reportDetail
      WHERE    reportDetailGUID = p_reportDetailGUID;

    END IF;

  END IF;

END$$

-- Returns all report details associated with p_reportID
DROP PROCEDURE IF EXISTS get_reportDetail_desc$$
CREATE PROCEDURE get_reportDetail_desc ( p_personGUID CHAR(36),
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

  ELSEIF ( count_reportDetail( p_personGUID, p_password, p_reportID ) < 1 ) THEN
     SET @US3_LAST_ERRNO = @NOROWS;
     SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
     SELECT @US3_LAST_ERRNO AS status;

  ELSE
     SELECT @OK AS status;

     SELECT    reportDetailID, reportDetailGUID, resultID, experimentID, runID, triple
     FROM      reportDetail
     WHERE     reportID = p_reportID
     ORDER BY  runID, triple;
   
  END IF;

END$$

-- Returns all report details associated with a single p_reportDetailID
DROP PROCEDURE IF EXISTS get_reportDetail_info$$
CREATE PROCEDURE get_reportDetail_info ( p_personGUID       CHAR(36),
                                         p_password         VARCHAR(80),
                                         p_reportDetailID   INT )
  READS SQL DATA

BEGIN
  DECLARE l_reportID    INT;
  DECLARE count_reports INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT reportID
  INTO   l_reportID
  FROM   reportDetail
  WHERE  reportDetailID = p_reportDetailID;

  SELECT COUNT(*)
  INTO   count_reports
  FROM   reportDetail
  WHERE  reportDetailID = p_reportDetailID;

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

     SELECT    reportDetailGUID, resultID, experimentID, runID, triple
     FROM      reportDetail
     WHERE     reportDetailID = p_reportDetailID;
   
  END IF;

END$$

-- DELETEs a report detail record, plus information in related tables
DROP PROCEDURE IF EXISTS delete_reportDetail$$
CREATE PROCEDURE delete_reportDetail ( p_personGUID        CHAR(36),
                                       p_password          VARCHAR(80),
                                       p_reportDetailID    INT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_reportID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT reportID
  INTO   l_reportID
  FROM   reportDetail
  WHERE  reportDetailID = p_reportDetailID;

  IF ( verify_report_permission( p_personGUID, p_password, l_reportID ) = @OK ) THEN

    -- Make sure records match if they have related tables or not
    -- Have to do it in a couple of stages because of the constraints
    DELETE      documentLink, reportDocument
    FROM        reportDetail
    LEFT JOIN   documentLink   ON ( reportDetail.reportDetailID = documentLink.reportDetailID )
    LEFT JOIN   reportDocument ON ( documentLink.reportDocumentID = reportDocument.reportDocumentID )
    WHERE       reportDetail.reportDetailID = p_reportDetailID;

    DELETE FROM reportDetail
    WHERE       reportDetailID = p_reportDetailID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- This set of routines deals with one or more documents

DROP FUNCTION IF EXISTS count_reportDocument$$
CREATE FUNCTION count_reportDocument( p_personGUID       CHAR(36),
                                      p_password         VARCHAR(80),
                                      p_reportDetailID   INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE l_reportID    INT;
  DECLARE count_reports INT;

  CALL config();
  SET count_reports = 0;

  SELECT reportID
  INTO   l_reportID
  FROM   reportDetail
  WHERE  reportDetailID = p_reportDetailID;

  IF ( verify_report_permission( p_personGUID, p_password, l_reportID ) = @OK ) THEN
    -- This is either an admin, or a user getting info about his own reports
    SELECT COUNT(*)
    INTO   count_reports
    FROM   documentLink
    WHERE  reportDetailID = p_reportDetailID;

  END IF;

  RETURN( count_reports );

END$$

-- INSERTs a new report document with the specified information
DROP PROCEDURE IF EXISTS new_reportDocument$$
CREATE PROCEDURE new_reportDocument ( p_personGUID          CHAR(36),
                                      p_password            VARCHAR(80),
                                      p_reportDetailID      INT,
                                      p_reportDocumentGUID  CHAR(36),
                                      p_label               VARCHAR(80),
                                      p_filename            VARCHAR(255),
                                      p_analysis            enum('vHW', 'dcdt', 'secmo',
                                                              '2DSA', '2DSA-MC', '2DSA-MW', '2DSA-MW-MC',
                                                              'GA', 'GA-MC', 'GA-MW-MC'),
                                      p_subAnalysis         enum('report', 'velocity', 'residuals', 'rbitmap',
                                                              'rinoise', 'tinoise',
                                                              'sdistrib', 'mw-distrib', 'D-distrib', '3dplot',
                                                              'D_vs_s', 'D_vs_mw', 'ff0_vs_s', 'ff0_vs_mw',
                                                              'x-to-radius', 'x-to-S', 'average-S', 'combined'),
                                      p_documentType        enum('csv', 'png', 'svg', 'html', 'text'),
                                      p_contents longblob )

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
  FROM   reportDetail
  WHERE  reportDetailID = p_reportDetailID;

  IF ( verify_report_permission( p_personGUID, p_password, l_reportID ) != @OK ) THEN
    -- Can't do that
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to edit this report';
    
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( check_GUID ( p_personGUID, p_password, p_reportDocumentGUID ) != @OK ) THEN
    SET @US3_LAST_ERRNO = @BADGUID;
    SET @US3_LAST_ERROR = 'MySQL: the reportDocumentGUID is not in the correct format';

    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    INSERT INTO reportDocument SET
      reportDocumentGUID  = p_reportDocumentGUID,
      label               = p_label,
      filename            = p_filename,
      analysis            = p_analysis,
      subAnalysis         = p_subAnalysis,
      documentType        = p_documentType,
      contents            = p_contents;

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
        reportDetailID = p_reportDetailID,
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
                                         p_reportDocumentGUID  CHAR(36),
                                         p_label               VARCHAR(80),
                                         p_filename            VARCHAR(255),
                                         p_analysis            enum('vHW', 'dcdt', 'secmo',
                                                                 '2DSA', '2DSA-MC', '2DSA-MW', '2DSA-MW-MC',
                                                                 'GA', 'GA-MC', 'GA-MW-MC'),
                                         p_subAnalysis         enum('report', 'velocity', 'residuals', 'rbitmap',
                                                                 'rinoise', 'tinoise',
                                                                 'sdistrib', 'mw-distrib', 'D-distrib', '3dplot',
                                                                 'D_vs_s', 'D_vs_mw', 'ff0_vs_s', 'ff0_vs_mw',
                                                                 'x-to-radius', 'x-to-S', 'average-S', 'combined'),
                                         p_documentType        enum('csv', 'png', 'svg', 'html', 'text'),
                                         p_contents longblob )

  MODIFIES SQL DATA

BEGIN
  DECLARE l_reportID    INT;

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
  FROM   documentLink, reportDetail
  WHERE  documentLink.reportDocumentID = p_reportDocumentID
  AND    documentLink.reportDetailID = reportDetail.reportDetailID;

  IF ( verify_report_permission( p_personGUID, p_password, l_reportID ) != @OK ) THEN
    -- Can't do that
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to edit this report';
    
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( check_GUID ( p_personGUID, p_password, p_reportDocumentGUID ) != @OK ) THEN
    SET @US3_LAST_ERRNO = @BADGUID;
    SET @US3_LAST_ERROR = 'MySQL: the reportDocumentGUID is not in the correct format';

    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    UPDATE reportDocument SET
      reportDocumentGUID   = p_reportDocumentGUID,
      label                = p_label,
      filename             = p_filename,
      analysis             = p_analysis,
      subAnalysis          = p_subAnalysis,
      documentType         = p_documentType,
      contents             = p_contents
    WHERE reportDocumentID = p_reportDocumentID;

    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for reportDocumentGUID field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for reportDocumentGUID field";

    ELSEIF ( constraint_failed = 1 ) THEN
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

-- Returns most info about all reports associated with p_reportDetailID
DROP PROCEDURE IF EXISTS get_reportDocument_desc$$
CREATE PROCEDURE get_reportDocument_desc ( p_personGUID      CHAR(36),
                                           p_password        VARCHAR(80),
                                           p_reportDetailID  INT )
  READS SQL DATA

BEGIN
  DECLARE l_reportID    INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT reportID
  INTO   l_reportID
  FROM   reportDetail
  WHERE  reportDetailID = p_reportDetailID;

  IF ( verify_report_permission( p_personGUID, p_password, l_reportID ) != @OK ) THEN
     SET @US3_LAST_ERRNO = @NOTPERMITTED;
     SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this report';
     
     SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_reportDocument( p_personGUID, p_password, p_reportDetailID ) < 1 ) THEN
     SET @US3_LAST_ERRNO = @NOROWS;
     SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
     SELECT @US3_LAST_ERRNO AS status;

  ELSE
     SELECT @OK AS status;

     SELECT    reportDocument.reportDocumentID, reportDocumentGUID, label, filename, 
               analysis, subAnalysis, documentType
     FROM      documentLink, reportDocument
     WHERE     reportDetailID = p_reportDetailID
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
  DECLARE l_reportDetailID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT reportID, documentLink.reportDetailID
  INTO   l_reportID, l_reportDetailID
  FROM   documentLink, reportDetail
  WHERE  documentLink.reportDocumentID = p_reportDocumentID
  AND    documentLink.reportDetailID = reportDetail.reportDetailID;

  IF ( verify_report_permission( p_personGUID, p_password, l_reportID ) != @OK ) THEN
     SET @US3_LAST_ERRNO = @NOTPERMITTED;
     SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this report';
     
     SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_reportDocument( p_personGUID, p_password, l_reportDetailID ) < 1 ) THEN
     SET @US3_LAST_ERRNO = @NOROWS;
     SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
     SELECT @US3_LAST_ERRNO AS status;

  ELSE
     SELECT @OK AS status;

     SELECT    reportDocumentGUID, label, filename, 
               analysis, subAnalysis, documentType, contents
     FROM      reportDocument
     WHERE     reportDocumentID = p_reportDocumentID
     ORDER BY  label;
   
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
  FROM   documentLink, reportDetail
  WHERE  reportDocumentID = p_reportDocumentID
  AND    documentLink.reportDetailID = reportDetail.reportDetailID;

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
