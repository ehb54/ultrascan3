--
-- us3_mrecs_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are related to the mrecs tables
-- Run as us3admin
--

DELIMITER $$

-- Verifies that the user has permission to view or modify
--  the specified mrecs
DROP FUNCTION IF EXISTS verify_mrecs_permission$$
CREATE FUNCTION verify_mrecs_permission( p_personGUID  CHAR(36),
                                         p_password    VARCHAR(80),
                                         p_mrecsID     INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE count_mrecs       INT;
  DECLARE count_permissions INT;
  DECLARE status            INT;

  CALL config();
  SET status   = @ERROR;
  SET @US3_LAST_ERROR = 'MySQL: error verifying mrecs permission';

  SELECT COUNT(*)
  INTO   count_mrecs
  FROM   pcsa_modelrecs
  WHERE  mrecsID = p_mrecsID;

  SELECT COUNT(*)
  INTO   count_permissions
  FROM   pcsa_modelrecs c, editedData e, rawData r, experimentPerson p
  WHERE  p.personID     = @US3_ID
  AND    r.experimentID = p.experimentID
  AND    e.rawDataID    = r.rawDataID
  AND    c.editedDataID = e.editedDataID;

  IF ( count_mrecs = 0 ) THEN
    SET @US3_LAST_ERRNO = @NO_MRECS;
    SET @US3_LAST_ERROR = 'MySQL: the specified mrecs file does not exist';

    SET status = @NO_MRECS;

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
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view or modify this mrecs file';

    SET status = @NOTPERMITTED;

  END IF;

  RETURN( status );

END$$

-- Returns the count of mrecs files associated with p_ID with a given editedDataID
--  p_ID cannot be 0
--  Admin user can get count of mrecs files that belong to others
DROP FUNCTION IF EXISTS count_mrecs_by_editID$$
CREATE FUNCTION count_mrecs_by_editID( p_personGUID CHAR(36),
                                       p_password VARCHAR(80),
                                       p_ID         INT,
                                       p_editID     INT )
  RETURNS INT
  READS SQL DATA

BEGIN

  DECLARE count_mrecs INT;

  CALL config();
  SET count_mrecs = 0;

  IF ( p_ID <= 0 ) THEN
    -- Gotta have a real ID
    RETURN ( 0 );

  ELSEIF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    SELECT COUNT(*)
    INTO   count_mrecs
    FROM   pcsa_modelrecs c, editedData e, rawData r, experimentPerson p
    WHERE  p.personID     = p_ID
    AND    r.experimentID = p.experimentID
    AND    e.rawDataID    = r.rawDataID
    AND    c.editedDataID = e.editedDataID
    AND    c.editedDataID = p_editID;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view those mrecs files';

    ELSE
      -- This person is asking about his own mrecs files
      -- Ignore p_ID and return user's own
      SELECT COUNT(*)
      INTO   count_mrecs
      FROM   pcsa_modelrecs c, editedData e, rawData r, experimentPerson p
      WHERE  p.personID     = @US3_ID
      AND    r.experimentID = p.experimentID
      AND    e.rawDataID    = r.rawDataID
      AND    c.editedDataID = e.editedDataID
      AND    c.editedDataID = p_editID;

    END IF;

  END IF;

  RETURN( count_mrecs );

END$$

-- Returns the count of mrecs files associated with p_ID
--  If p_ID = 0, retrieves count of all mrecs files in db
--  Regular user can only get count of his own mrecs files
DROP FUNCTION IF EXISTS count_mrecs$$
CREATE FUNCTION count_mrecs( p_personGUID CHAR(36),
                             p_password VARCHAR(80),
                             p_ID       INT )
  RETURNS INT
  READS SQL DATA

BEGIN

  DECLARE count_mrecs INT;

  CALL config();
  SET count_mrecs = 0;

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( p_ID > 0 ) THEN
      SELECT COUNT(*)
      INTO   count_mrecs
      FROM   pcsa_modelrecs c, editedData e, rawData r, experimentPerson p
      WHERE  p.personID     = p_ID
      AND    r.experimentID = p.experimentID
      AND    e.rawDataID    = r.rawDataID
      AND    c.editedDataID = e.editedDataID;

    ELSE
      SELECT COUNT(*)
      INTO   count_mrecs
      FROM   pcsa_modelrecs;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view those mrecs files';

    ELSE
      -- This person is asking about his own mrecs files
      -- Ignore p_ID and return user's own
      SELECT COUNT(*)
      INTO   count_mrecs
      FROM   pcsa_modelrecs c, editedData e, rawData r, experimentPerson p
      WHERE  p.personID     = @US3_ID
      AND    r.experimentID = p.experimentID
      AND    e.rawDataID    = r.rawDataID
      AND    c.editedDataID = e.editedDataID;

    END IF;

  END IF;

  RETURN( count_mrecs );

END$$

-- INSERTs a new mrecs file with the specified information
DROP PROCEDURE IF EXISTS new_mrecs$$
CREATE PROCEDURE new_mrecs ( p_personGUID    CHAR(36),
                             p_password      VARCHAR(80),
                             p_mrecsGUID     CHAR(36),
                             p_editedDataID  INT(11),
                             p_modelID       INT(11),
                             p_modelGUID     CHAR(36),
                             p_description   TEXT,
                             p_xml           LONGTEXT )     -- an xml file
  MODIFIES SQL DATA

BEGIN
  DECLARE l_editID          INT;
  DECLARE l_editID_count    INT;
  DECLARE l_modelID         INT;
  DECLARE l_modelID_count   INT;
  DECLARE l_modelGUID_count INT;

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
 
  -- Verify that the editedDataID exists, and we have permissions
  SET l_editID = 1;         -- default to special "unassigned" record
  IF ( verify_editData_permission( p_personGUID, p_password, p_editedDataID ) = @OK ) THEN
    SET l_editID = p_editedDataID;

  END IF;

  SELECT COUNT(*)
  INTO   l_editID_count
  FROM   editedData
  WHERE  editedDataID = p_editedDataID
  LIMIT  1;                         -- should be exactly 1, if p_editedDataID is supplied

  -- User can specify either modelID or modelGUID
  SELECT COUNT(*)
  INTO   l_modelGUID_count
  FROM   model
  WHERE  modelGUID = p_modelGUID
  LIMIT  1;                         -- should be exactly 1, if p_modelGUID is supplied

  SELECT COUNT(*)
  INTO   l_modelID_count
  FROM   model
  WHERE  modelID = p_modelID
  LIMIT  1;                         -- should be exactly 1, if p_modelID is supplied

  IF ( l_modelGUID_count = 1 ) THEN -- prefer the GUID
    SELECT modelID
    INTO   l_modelID
    FROM   model
    WHERE  modelGUID = p_modelGUID
    LIMIT  1;

  ELSEIF ( l_modelID_count = 1 ) THEN
    SET l_modelID = p_modelID;

  ELSE
    -- mrecs file doesn't belong to any model
    SET l_modelID   = 0;

  END IF;

  -- Verify that the editedDataID exists, and we have permissions
  SET l_editID = 1;         -- default to special "unassigned" record
  IF ( verify_editData_permission( p_personGUID, p_password, p_editedDataID ) = @OK ) THEN
    SET l_editID = p_editedDataID;
  END IF;

  IF ( check_GUID ( p_personGUID, p_password, p_mrecsGUID ) = @OK ) THEN
    INSERT INTO pcsa_modelrecs SET
      mrecsGUID    = p_mrecsGUID,
      editedDataID = l_editID,
      modelID      = l_modelID,
      description  = p_description,
      xml          = p_xml,
      lastUpdated  = NOW();

    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for mrecsGUID field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for mrecsGUID field";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing mrecs with the specified information
DROP PROCEDURE IF EXISTS update_mrecs$$
CREATE PROCEDURE update_mrecs ( p_personGUID    CHAR(36),
                                p_password      VARCHAR(80),
                                p_mrecsID       INT(11),
                                p_mrecsGUID     CHAR(36),
                                p_editedDataID  INT(11),
                                p_modelID       INT(11),
                                p_description   TEXT,
                                p_xml           LONGTEXT )     -- an xml file
  MODIFIES SQL DATA

BEGIN
  DECLARE l_editID          INT;
  DECLARE l_editID_count    INT;

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
 
  -- Verify that the editedDataID exists, and we have permissions
  SET l_editID = 1;         -- default to special "unassigned" record
  IF ( verify_editData_permission( p_personGUID, p_password, p_editedDataID ) = @OK ) THEN
    SET l_editID = p_editedDataID;

  END IF;

  SELECT COUNT(*)
  INTO   l_editID_count 
  FROM   editedData
  WHERE  editedDataID = p_editedDataID
  LIMIT  1;

  IF ( verify_editData_permission( p_personGUID, p_password, l_editID ) != @OK ) THEN
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( l_editID_count = 0 ) THEN
    SET @US3_LAST_ERROR = "MySQL: the editedData was not found in the database";
    SET @US3_LAST_ERRNO = @NO_EDITDATA;

    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( check_GUID ( p_personGUID, p_password, p_mrecsGUID ) = @OK ) THEN
    UPDATE pcsa_modelrecs SET
      mrecsGUID    = p_mrecsGUID,
      editedDataID = l_editID,
      modelID      = p_modelID,
      description  = p_description,
      xml          = p_xml,
      lastUpdated  = NOW()
    WHERE mrecsID  = p_mrecsID;

    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for mrecsGUID field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for mrecsGUID field";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns the mrecsID associated with the given mrecsGUID
DROP PROCEDURE IF EXISTS get_mrecsID$$
CREATE PROCEDURE get_mrecsID ( p_personGUID    CHAR(36),
                               p_password      VARCHAR(80),
                               p_mrecsGUID     CHAR(36) )
  READS SQL DATA

BEGIN

  DECLARE count_mrecs INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET count_mrecs    = 0;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN

    SELECT COUNT(*)
    INTO   count_mrecs
    FROM   pcsa_modelrecs
    WHERE  mrecsGUID = p_mrecsGUID;

    IF ( TRIM( p_mrecsGUID ) = '' ) THEN
      SET @US3_LAST_ERRNO = @EMPTY;
      SET @US3_LAST_ERROR = CONCAT( 'MySQL: The mrecsGUID parameter to the ',
                                    'get_mrecsID function cannot be empty' );

    ELSEIF ( count_mrecs < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT mrecsID
      FROM   pcsa_modelrecs
      WHERE  mrecsGUID = p_mrecsGUID;

    END IF;

  END IF;

END$$

-- Returns the mrecsID and description of all mrecs files associated with p_ID
--  If p_ID = 0, retrieves information about all mrecs files in db
--  Regular user can only get info about his own mrecs files
DROP PROCEDURE IF EXISTS get_mrecs_desc$$
CREATE PROCEDURE get_mrecs_desc ( p_personGUID CHAR(36),
                                  p_password   VARCHAR(80),
                                  p_ID         INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_mrecs( p_personGUID, p_password, p_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      IF ( p_ID > 0 ) THEN
        SELECT   mrecsID, mrecsGUID, c.editedDataID, e.editGUID,
                 c.modelID, m.modelGUID, c.description,
                 MD5( c.xml ) AS checksum, LENGTH( c.xml ) AS size,
                 timestamp2UTC( c.lastUpdated ) AS UTC_lastUpdated 
        FROM     pcsa_modelrecs c, editedData e, rawData r,
                 experimentPerson p, model m
        WHERE    p.personID     = p_ID
        AND      r.experimentID = p.experimentID
        AND      e.rawDataID    = r.rawDataID
        AND      e.editedDataID = c.editedDataID
        AND      m.modelID      = c.modelID
        ORDER BY mrecsID DESC;

      ELSE
        SELECT   mrecsID, mrecsGUID, c.editedDataID, e.editGUID,
                 c.modelID, m.modelGUID, c.description,
                 MD5( c.xml ) AS checksum, LENGTH( c.xml ) AS size,
                 timestamp2UTC( c.lastUpdated ) AS UTC_lastUpdated
        FROM     pcsa_modelrecs c, editedData e, model m
        WHERE    e.editedDataID = c.editedDataID
        AND      m.modelID      = c.modelID
        ORDER BY mrecsID DESC;

      END IF;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this mrecs';
     
      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_mrecs( p_personGUID, p_password, @US3_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      -- Ok, user wants his own info
      SELECT @OK AS status;

      SELECT   mrecsID, mrecsGUID, c.editedDataID, e.editGUID,
               c.modelID, m.modelGUID, c.description,
               MD5( c.xml ) AS checksum, LENGTH( c.xml ) AS size,
               timestamp2UTC( c.lastUpdated ) AS UTC_lastUpdated 
      FROM     pcsa_modelrecs c, editedData e, rawData r,
               experimentPerson p, model m
      WHERE    p.personID     = @US3_ID
      AND      r.experimentID = p.experimentID
      AND      e.rawDataID    = r.rawDataID
      AND      e.editedDataID = c.editedDataID
      AND      m.modelID      = c.modelID
      ORDER BY mrecsID DESC;

    END IF;

  END IF;

END$$

-- Returns the mrecsID and description of all mrecs files associated with p_ID and editID
--  Unlike get_mrecs_desc, in this function p_ID cannot be 0
--  Admin can view p_ID/editID combinations that belong to anyone
DROP PROCEDURE IF EXISTS get_mrecs_desc_by_editID$$
CREATE PROCEDURE get_mrecs_desc_by_editID ( p_personGUID CHAR(36),
                                            p_password   VARCHAR(80),
                                            p_ID         INT,
                                            p_editID     INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( p_ID <= 0 ) THEN
    -- Gotta have a real ID
    SET @US3_LAST_ERRNO = @EMPTY;
    SET @US3_LAST_ERROR = 'MySQL: The ID cannot be 0';

    SELECT @US3_LAST_ERRNO AS status;
    
  ELSEIF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_mrecs_by_editID( p_personGUID, p_password, p_ID, p_editID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      SELECT   mrecsID, mrecsGUID, c.editedDataID, e.editGUID,
               c.modelID, m.modelGUID, c.description,
               MD5( c.xml ) AS checksum, LENGTH( c.xml ) AS size,
               timestamp2UTC( c.lastUpdated ) AS UTC_lastUpdated 
      FROM     pcsa_modelrecs c, editedData e, rawData r,
               experimentPerson p, model m
      WHERE    p.personID     = p_ID
      AND      r.experimentID = p.experimentID
      AND      e.rawDataID    = r.rawDataID
      AND      c.editedDataID = p_editID
      AND      e.editedDataID = p_editID
      AND      m.modelID      = c.modelID
      ORDER BY mrecsID DESC;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this mrecs';
     
      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_mrecs_by_editID( p_personGUID, p_password, @US3_ID, p_editID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      -- Ok, user wants his own info
      SELECT @OK AS status;

      SELECT   mrecsID, mrecsGUID, c.editedDataID, e.editGUID,
               c.modelID, m.modelGUID, c.description,
               MD5( c.xml ) AS checksum, LENGTH( c.xml ) AS size,
               timestamp2UTC( c.lastUpdated ) AS UTC_lastUpdated 
      FROM     pcsa_modelrecs c, editedData e, rawData r,
               experimentPerson p, model m
      WHERE    p.personID     = @US3_ID
      AND      r.experimentID = p.experimentID
      AND      e.rawDataID    = r.rawDataID
      AND      c.editedDataID = p_editID
      AND      e.editedDataID = p_editID
      AND      m.modelID      = c.modelID
      ORDER BY mrecsID DESC;

    END IF;

  END IF;

END$$

-- Returns a more complete list of information about one mrecs file
--  Actually, returns most of the same information for one record
--  as mrecs_desc, but includes the xml content
DROP PROCEDURE IF EXISTS get_mrecs_info$$
CREATE PROCEDURE get_mrecs_info ( p_personGUID  CHAR(36),
                                  p_password    VARCHAR(80),
                                  p_mrecsID     INT )
  READS SQL DATA

BEGIN
  DECLARE count_mrecs INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_mrecs
  FROM       pcsa_modelrecs
  WHERE      mrecsID = p_mrecsID;

  IF ( verify_mrecs_permission( p_personGUID, p_password, p_mrecsID ) = @OK ) THEN
    IF ( count_mrecs = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   mrecsGUID, c.xml, c.editedDataID, e.editGUID,
               c.modelID, m.modelGUID, c.description,
               MD5( c.xml ) AS checksum, LENGTH( c.xml ) AS size,
               timestamp2UTC( c.lastUpdated ) AS UTC_lastUpdated 
      FROM     pcsa_modelrecs c, editedData e, model m
      WHERE    mrecsID        = p_mrecsID
      AND      e.editedDataID = c.editedDataID
      AND      m.modelID      = c.modelID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- DELETEs a mrecs file
DROP PROCEDURE IF EXISTS delete_mrecs$$
CREATE PROCEDURE delete_mrecs ( p_personGUID  CHAR(36),
                                p_password    VARCHAR(80),
                                p_mrecsID     INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_mrecs_permission( p_personGUID, p_password, p_mrecsID ) = @OK ) THEN

    DELETE FROM pcsa_modelrecs
    WHERE mrecsID = p_mrecsID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$
