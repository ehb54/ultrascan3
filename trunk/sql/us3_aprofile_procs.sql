--
-- us3_aprofile_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are procedures related to the analysisprofile data
-- Run as us3admin
--

DELIMITER $$

-- Routines that deal with an analysis profile

-- Returns the count of all analysis profiles
DROP FUNCTION IF EXISTS count_aprofiles$$
CREATE FUNCTION count_aprofiles( p_personGUID CHAR(36),
                                 p_password VARCHAR(80) )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_aprofiles INT;

  CALL config();
  SET count_aprofiles = 0;

  SELECT COUNT(*)
  INTO   count_aprofiles
  FROM   analysisprofile;

  RETURN( count_aprofiles );

END$$

-- INSERTs new analysis profile information
DROP PROCEDURE IF EXISTS new_aprofile$$
CREATE PROCEDURE new_aprofile ( p_personGUID   CHAR(36),
                                p_password     VARCHAR(80),
                                p_aprofileGUID CHAR(36),
                                p_name         VARCHAR(160),
                                p_xml          LONGTEXT )
  MODIFIES SQL DATA

BEGIN

  DECLARE l_aprofileID INT;

  DECLARE duplicate_key TINYINT DEFAULT 0;
  DECLARE null_field    TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1062
    SET duplicate_key = 1;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = -1;
 
  IF ( ( verify_user( p_personGUID, p_password ) = @OK ) &&
       ( check_GUID ( p_personGUID, p_password, p_aprofileGUID ) = @OK ) ) THEN
 
    INSERT INTO analysisprofile SET
      aprofileGUID   = p_aprofileGUID,
      name           = p_name,
      xml            = p_xml,
      dateUpdated    = NOW();
   
    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for aprofileGUID/name field(s)";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: Attempt to insert NULL value in the analysisprofile table";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;
 
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns the aprofileID and base information for all analysis profiles
DROP PROCEDURE IF EXISTS get_aprofile_desc$$
CREATE PROCEDURE get_aprofile_desc ( p_personGUID CHAR(36),
                                     p_password VARCHAR(80) )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( count_aprofiles( p_personGUID, p_password ) < 1 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    SELECT @OK AS status;
  
    SELECT   aprofileID, aprofileGUID, name, xml,
             timestamp2UTC( dateUpdated ) AS UTC_lastUpdated
    FROM     analysisprofile
    ORDER BY aprofileID DESC;

  END IF;

END$$

-- Returns the aprofileID and base information for one analysis profile
--  as identified by GUID
DROP PROCEDURE IF EXISTS get_aprofile_info$$
CREATE PROCEDURE get_aprofile_info ( p_personGUID    CHAR(36),
                                     p_password      VARCHAR(80),
                                     p_aprofileGUID  CHAR(36) )
  READS SQL DATA

BEGIN
  DECLARE count_aprofiles INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_aprofiles
  FROM       analysisprofile
  WHERE      aprofileGUID = p_aprofileGUID;

  IF ( count_aprofiles = 0 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_aprofiles > 1 ) THEN
    SET @US3_LAST_ERRNO = @MORE_THAN_SINGLE_ROW;
    SET @US3_LAST_ERROR = 'MySQL: more than a single row for an analysis profile';

    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    SELECT @OK AS status;

    SELECT   aprofileID, name, xml,
             timestamp2UTC( dateUpdated ) AS UTC_lastUpdated
    FROM     analysisprofile
    WHERE    aprofileGUID = p_aprofileGUID;
  END IF;

END$$

-- Returns the aprofileID and base information for one analysis profile
--  as identified by database ID
DROP PROCEDURE IF EXISTS get_aprofile_info_byID$$
CREATE PROCEDURE get_aprofile_info_byID ( p_personGUID  CHAR(36),
                                          p_password    VARCHAR(80),
                                          p_aprofileID  INT(11) )
  READS SQL DATA

BEGIN
  DECLARE count_aprofiles INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_aprofiles
  FROM       analysisprofile
  WHERE      aprofileID = p_aprofileID;

  IF ( count_aprofiles = 0 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_aprofiles > 1 ) THEN
    SET @US3_LAST_ERRNO = @MORE_THAN_SINGLE_ROW;
    SET @US3_LAST_ERROR = 'MySQL: more than a single row for an analysis profile';

    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    SELECT @OK AS status;

    SELECT   aprofileGUID, name, xml,
             timestamp2UTC( dateUpdated ) AS UTC_lastUpdated
    FROM     analysisprofile
    WHERE    aprofileID = p_aprofileID;
  END IF;

END$$

-- DELETEs an aprofile, plus information in related tables
DROP PROCEDURE IF EXISTS delete_aprofile$$
CREATE PROCEDURE delete_aprofile ( p_personGUID  CHAR(36),
                                   p_password  VARCHAR(80),
                                   p_aprofileID   INT )
  MODIFIES SQL DATA

BEGIN
  DECLARE count_aprofiles INT;	

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_aprofile_permission( p_personGUID, p_password, p_aprofileID ) = @OK ) THEN

     -- Make sure records match if they have related tables or not
     -- Have to do it in a couple of stages because of the constraints
     DELETE FROM aprofile
     WHERE aprofileID = p_aprofileID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$
