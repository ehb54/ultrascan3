--
-- us3_protocol_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are procedures related to the run protocol data
-- Run as us3admin
--

DELIMITER $$

-- Routines that deal with a protocol for an experiment run

-- Verifies that the user has permission to view or modify
--  the run protocol
DROP FUNCTION IF EXISTS verify_protocol_permission$$
CREATE FUNCTION verify_protocol_permission( p_personGUID  CHAR(36),
                                            p_password    VARCHAR(80),
                                            p_protocolID  INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE count_protocols   INT;
  DECLARE count_permissions INT;
  DECLARE status            INT;

  CALL config();
  SET status   = @ERROR;
  SET @US3_LAST_ERROR = 'MySQL: error verifying protocol permission';

  SELECT COUNT(*)
  INTO   count_protocols
  FROM   protocol
  WHERE  protocolID = p_protocolID;

  SELECT COUNT(*)
  INTO   count_permissions
  FROM   protocolPerson
  WHERE  protocolID = p_protocolID
  AND    personID = @US3_ID;

  IF ( count_protocols = 0 ) THEN
    SET @US3_LAST_ERRNO = @NO_PROTOCOL;
    SET @US3_LAST_ERROR = 'MySQL: the specified protocol does not exist';

    SET status = @NO_PROTOCOL;

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
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view or modify this run protocol';

    SET status = @NOTPERMITTED;

  END IF;

  RETURN( status );

END$$

-- Returns the count of protocols associated with p_ID
--  If p_ID = 0, retrieves count of all protocols in db
--  Regular user can only get count of his own protocols
DROP FUNCTION IF EXISTS count_protocols$$
CREATE FUNCTION count_protocols( p_personGUID CHAR(36),
                                 p_password VARCHAR(80),
                                 p_ID       INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_protocols INT;

  CALL config();
  SET count_protocols = 0;

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( p_ID > 0 ) THEN
      SELECT COUNT(*)
      INTO   count_protocols
      FROM   protocolPerson
      WHERE  personID = p_ID;

    ELSE
      SELECT COUNT(*)
      INTO   count_protocols
      FROM   protocolPerson;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view those protocols';
     
    ELSE
      -- This person is asking about his own protocols
      -- Ignore p_ID and return user's own
      SELECT COUNT(*)
      INTO   count_protocols
      FROM   protocolPerson
      WHERE  personID = @US3_ID;

    END IF;
    
  END IF;

  RETURN( count_protocols );

END$$

-- INSERTs new protocol information
DROP PROCEDURE IF EXISTS new_protocol$$
CREATE PROCEDURE new_protocol ( p_personGUID   CHAR(36),
                                p_password     VARCHAR(80),
                                p_ownerID      INT,
                                p_protocolGUID CHAR(36),
                                p_description  VARCHAR(160),
                                p_xml          LONGTEXT,
                                p_optimaHost   VARCHAR(24),
                                p_rotorID      INT,
                                p_speed1       INT,
                                p_duration     FLOAT,
                                p_usedcells    INT,
                                p_estscans     INT,
                                p_solution1    VARCHAR(80),
                                p_solution2    VARCHAR(80),
                                p_wavelengths  INT )
  MODIFIES SQL DATA

BEGIN

  DECLARE l_protocolID INT;

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
       ( check_GUID ( p_personGUID, p_password, p_protocolGUID ) = @OK ) ) THEN
 
    INSERT INTO protocol SET
      protocolGUID   = p_protocolGUID,
      description    = p_description,
      xml            = p_xml,
      optimaHost     = p_optimaHost,
      dateUpdated    = NOW(),
      rotorID        = p_rotorID,
      speed1         = p_speed1,
      duration       = p_duration,
      usedcells      = p_usedcells,
      estscans       = p_estscans,
      solution1      = p_solution1,
      solution2      = p_solution2,
      wavelengths    = p_wavelengths;
   
    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for protocolGUID/description field(s)";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: Attempt to insert NULL value in the protocol table";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

      INSERT INTO protocolPerson SET
        protocolID = @LAST_INSERT_ID,
        personID   = p_ownerID;

    END IF;
 
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns the protocolID and description of all protocols associated with p_ID
--  If p_ID = 0, retrieves information about all protocols in db
--  Regular user can only get info about his own protocols
DROP PROCEDURE IF EXISTS get_protocol_desc$$
CREATE PROCEDURE get_protocol_desc ( p_personGUID CHAR(36),
                                     p_password VARCHAR(80),
                                     p_ID       INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_protocols( p_personGUID, p_password, p_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      IF ( p_ID > 0 ) THEN
        SELECT   p.protocolID, protocolGUID, description, xml, optimaHost,
                 timestamp2UTC( dateUpdated ) AS UTC_lastUpdated
        FROM     protocol p, protocolPerson
        WHERE    p.protocolID = protocolPerson.protocolID
        AND      protocolPerson.personID = p_ID
        ORDER BY p.protocolID DESC;
   
      ELSE
        SELECT   p.protocolID, protocolGUID, description, xml, optimaHost,
                 timestamp2UTC( dateUpdated ) AS UTC_lastUpdated,
                 personID
        FROM     protocol p, protocolPerson
        WHERE    p.protocolID = protocolPerson.protocolID
        ORDER BY p.protocolID DESC;

      END IF;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this protocol';
     
      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_protocols( p_personGUID, p_password, @US3_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      -- Ok, user wants his own info
      SELECT @OK AS status;

      SELECT   p.protocolID, protocolGUID, description, xml, optimaHost,
               timestamp2UTC( dateUpdated ) AS UTC_lastUpdated
      FROM     protocol p, protocolPerson
      WHERE    p.protocolID = protocolPerson.protocolID
      AND      p.personID = @US3_ID
      ORDER BY p.protocolID DESC;

    END IF;

  END IF;

END$$

-- Returns a more complete list of information about one protocol
DROP PROCEDURE IF EXISTS get_protocol_info$$
CREATE PROCEDURE get_protocol_info ( p_personGUID  CHAR(36),
                                     p_password    VARCHAR(80),
                                     p_protocolID  INT )
  READS SQL DATA

BEGIN
  DECLARE count_protocols INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_protocols
  FROM       protocol
  WHERE      protocolID = p_protocolID;

  IF ( verify_protocol_permission( p_personGUID, p_password, p_protocolID ) = @OK ) THEN
    IF ( count_protocols = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_protocols > 1 ) THEN
      SET @US3_LAST_ERRNO = @MORE_THAN_SINGLE_ROW;
      SET @US3_LAST_ERROR = 'MySQL: more than a single row for a run protocol';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   pc.protocolID, protocolGUID, description, xml, optimaHost,
               timestamp2UTC( dateUpdated ) AS UTC_lastUpdated,
               rotorID, speed1, duration, usedcells, estscans,
               solution1, solution2, wavelengths, pp.personID
      FROM     protocol pc, protocolPerson pp
      WHERE    pc.protocolID = pp.protocolID
      AND      pc.protocolID = p_protocolID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- DELETEs a protocol, plus information in related tables
DROP PROCEDURE IF EXISTS delete_protocol$$
CREATE PROCEDURE delete_protocol ( p_personGUID  CHAR(36),
                                   p_password  VARCHAR(80),
                                   p_protocolID   INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_protocol_permission( p_personGUID, p_password, p_protocolID ) = @OK ) THEN

    -- Make sure records match if they have related tables or not
    -- Have to do it in a couple of stages because of the constraints
    DELETE FROM protocolPerson
    WHERE protocolID = p_protocolID;

    DELETE FROM protocol
    WHERE protocolID = p_protocolID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$
