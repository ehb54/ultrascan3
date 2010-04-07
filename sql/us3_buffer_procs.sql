--
-- us3_buffer_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are related to the buffer table
-- Run as us3admin
--

DELIMITER $$

-- Verifies that the user has permission to modify
--  the specified buffer
DROP FUNCTION IF EXISTS verify_buffer_permission$$
CREATE FUNCTION verify_buffer_permission( p_guid     CHAR(36),
                                          p_password VARCHAR(80),
                                          p_bufferID INT )
  RETURNS TINYINT
  READS SQL DATA

BEGIN
  DECLARE count_buffer   INT;
  DECLARE status         TINYINT;

  CALL config();
  SET status   = @ERROR;

  SELECT COUNT(*)
  INTO   count_buffer
  FROM   bufferPerson
  WHERE  bufferID = p_bufferID
  AND    personID = @US3_ID;
 
  IF ( verify_user( p_guid, p_password ) = @OK &&
       count_buffer > 0 ) THEN
    SET status = @OK;

  ELSEIF ( verify_userlevel( p_guid, p_password, @US3_ADMIN ) = @OK ) THEN
    SET status = @OK;

  ELSE
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to modify this buffer';

    SET status = @NOTPERMITTED;

  END IF;

  RETURN( status );

END$$

-- Returns the count of buffers associated with p_ID
--  If p_ID = 0, retrieves count of all buffers in db
DROP FUNCTION IF EXISTS count_buffers$$
CREATE FUNCTION count_buffers( p_guid     CHAR(36),
                               p_password VARCHAR(80),
                               p_ID       INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_buffers INT;

  CALL config();
  SET count_buffers = 0;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    IF ( p_ID > 0 ) THEN
      SELECT COUNT(*)
      INTO count_buffers
      FROM bufferPerson
      WHERE personID   = p_ID;

    ELSE
      SELECT COUNT(*)
      INTO count_buffers
      FROM bufferPerson;

    END IF;

  END IF;

  RETURN( count_buffers );

END$$

-- INSERTs a new buffer with the specified information
DROP PROCEDURE IF EXISTS new_buffer$$
CREATE PROCEDURE new_buffer ( p_guid     CHAR(36),
                              p_password VARCHAR(80),
                              p_description TEXT,
                              p_spectrum TEXT,
                              p_pH FLOAT,
                              p_density FLOAT,
                              p_viscosity FLOAT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_bufferID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    INSERT INTO buffer SET
      description = p_description,
      spectrum    = p_spectrum,
      pH          = p_pH,
      density     = p_density,
      viscosity   = p_viscosity;

    SET @LAST_INSERT_ID = LAST_INSERT_ID();

    INSERT INTO bufferPerson SET
      bufferID    = @LAST_INSERT_ID,
      personID    = @US3_ID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing buffer with the specified information
DROP PROCEDURE IF EXISTS update_buffer$$
CREATE PROCEDURE update_buffer ( p_guid     CHAR(36),
                                 p_password VARCHAR(80),
                                 p_bufferID INT,
                                 p_description TEXT,
                                 p_spectrum TEXT,
                                 p_pH FLOAT,
                                 p_density FLOAT,
                                 p_viscosity FLOAT )
  MODIFIES SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_buffer_permission( p_guid, p_password, p_bufferID ) = @OK ) THEN
    UPDATE buffer SET
      description = p_description,
      spectrum    = p_spectrum,
      pH          = p_pH,
      density     = p_density,
      viscosity   = p_viscosity
    WHERE bufferID = p_bufferID;

  END IF;
      
  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns the bufferID and description of all buffers associated with p_ID
--  If p_ID = 0, retrieves information about all buffers in db
DROP PROCEDURE IF EXISTS get_buffer_desc$$
CREATE PROCEDURE get_buffer_desc ( p_guid     CHAR(36),
                                   p_password VARCHAR(80),
                                   p_ID       INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    IF ( count_buffers( p_guid, p_password, p_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      IF ( p_ID > 0 ) THEN
        SELECT b.bufferID, description
        FROM buffer b, bufferPerson
        WHERE b.bufferID = bufferPerson.bufferID
        AND bufferPerson.personID = p_ID
        ORDER BY b.bufferID DESC;
 
      ELSE
        SELECT b.bufferID, description
        FROM buffer b, bufferPerson
        WHERE b.bufferID = bufferPerson.bufferID
        ORDER BY b.bufferID DESC;
      END IF;

    END IF;

  END IF;

END$$

-- Returns a more complete list of information about one buffer
DROP PROCEDURE IF EXISTS get_buffer_info$$
CREATE PROCEDURE get_buffer_info ( p_guid     CHAR(36),
                                   p_password VARCHAR(80),
                                   p_bufferID INT )
  READS SQL DATA

BEGIN
  DECLARE count_buffers INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_buffers
  FROM       buffer
  WHERE      bufferID = p_bufferID;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    IF ( count_buffers = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   description, spectrum, pH, viscosity, density, personID
      FROM     buffer b, bufferPerson bp
      WHERE    b.bufferID = bp.bufferID
      AND      b.bufferID = p_bufferID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- DELETEs a buffer, plus information in related tables
DROP PROCEDURE IF EXISTS delete_buffer$$
CREATE PROCEDURE delete_buffer ( p_guid     CHAR(36),
                                 p_password VARCHAR(80),
                                 p_bufferID INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_buffer_permission( p_guid, p_password, p_bufferID ) = @OK ) THEN

    DELETE FROM bufferLink
    WHERE bufferID = p_bufferID;

    DELETE FROM solutionBuffer
    WHERE bufferID = p_bufferID;

    DELETE FROM bufferPerson
    WHERE bufferID = p_bufferID;

    DELETE FROM bufferExtinction
    WHERE bufferID = p_bufferID;

    DELETE FROM bufferRefraction
    WHERE bufferID = p_bufferID;

    DELETE FROM buffer
    WHERE bufferID = p_bufferID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- SELECTs descriptions for all buffer components
DROP PROCEDURE IF EXISTS get_buffer_component_desc$$
CREATE PROCEDURE get_buffer_component_desc ( p_guid     CHAR(36),
                                             p_password VARCHAR(80) )
  READS SQL DATA

BEGIN
  DECLARE count_components INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    SELECT    COUNT(*)
    INTO      count_components
    FROM      bufferComponent;

    IF ( count_components = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT bufferComponentID, description
      FROM bufferComponent
      ORDER BY description;
 
    END IF;

  END IF;

END$$

-- Returns a more complete list of information about one buffer component
DROP PROCEDURE IF EXISTS get_buffer_component_info$$
CREATE PROCEDURE get_buffer_component_info ( p_guid        CHAR(36),
                                             p_password    VARCHAR(80),
                                             p_componentID INT )
  READS SQL DATA

BEGIN
  DECLARE count_components INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_components
  FROM       bufferComponent
  WHERE      bufferComponentID = p_componentID;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    IF ( count_components = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   units, description, viscosity, density
      FROM     bufferComponent
      WHERE    bufferComponentID = p_componentID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- adds a new buffer component from bufferComponent
DROP PROCEDURE IF EXISTS add_buffer_component$$
CREATE PROCEDURE add_buffer_component ( p_guid          CHAR(36),
                                        p_password      VARCHAR(80),
                                        p_bufferID      INT,
                                        p_componentID   INT,
                                        p_concentration FLOAT )
  MODIFIES SQL DATA

BEGIN
  DECLARE count_buffers    INT;
  DECLARE count_components INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;

  SELECT     COUNT(*)
  INTO       count_buffers
  FROM       buffer
  WHERE      bufferID = p_bufferID;

  SELECT     COUNT(*)
  INTO       count_components
  FROM       bufferComponent
  WHERE      bufferComponentID = p_componentID;

  IF ( verify_buffer_permission( p_guid, p_password, p_bufferID ) = @OK ) THEN
    IF ( count_buffers < 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_BUFFER;
      SET @US3_LAST_ERROR = CONCAT('MySQL: No buffer with ID ',
                                   p_bufferID,
                                   ' exists' );

    ELSEIF ( count_components < 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_COMPONENT;
      SET @US3_LAST_ERROR = CONCAT('MySQL: No buffer component with ID ',
                                   p_componentID,
                                   ' exists' );

    ELSE
      INSERT INTO bufferLink SET
        bufferID          = p_bufferID,
        bufferComponentID = p_componentID,
        concentration     = p_concentration;

      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns information about all buffer components of a single buffer
DROP PROCEDURE IF EXISTS get_buffer_components$$
CREATE PROCEDURE get_buffer_components ( p_guid     CHAR(36),
                                         p_password VARCHAR(80),
                                         p_bufferID INT )
  READS SQL DATA

BEGIN
  DECLARE count_components INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    SELECT    COUNT(*)
    INTO      count_components
    FROM      bufferLink
    WHERE     bufferID = p_bufferID;

    IF ( count_components = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   l.bufferComponentID, description, viscosity, density, concentration
      FROM     bufferLink l, bufferComponent c
      WHERE    l.bufferComponentID = c.bufferComponentID
      AND      l.bufferID = p_bufferID
      ORDER BY description;
 
    END IF;

  END IF;

END$$

-- DELETEs all components associated with a buffer
DROP PROCEDURE IF EXISTS delete_buffer_components$$
CREATE PROCEDURE delete_buffer_components ( p_guid     CHAR(36),
                                            p_password VARCHAR(80),
                                            p_bufferID INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_buffer_permission( p_guid, p_password, p_bufferID ) = @OK ) THEN
    DELETE FROM bufferLink
    WHERE bufferID = p_bufferID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Functions and procedures associated with the buffer extinction table

-- Verifies that the specified bufferID exists in the buffer table
DROP FUNCTION IF EXISTS verify_bufferID$$
CREATE FUNCTION verify_bufferID( p_guid      CHAR(36),
                                 p_password  VARCHAR(80),
                                 p_bufferID  INT )
  RETURNS INT
  READS SQL DATA

BEGIN

  DECLARE count_bufferID INT;

  CALL config();
  SET count_bufferID = 0;
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    SELECT COUNT(*)
    INTO   count_bufferID
    FROM   buffer
    WHERE  bufferID = p_bufferID;

    IF ( count_bufferID < 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_BUFFER;
      SET @US3_LAST_ERROR = CONCAT('MySQL: No buffer with ID ',
                                   p_bufferID,
                                   ' exists' );

    END IF;

  END IF;

  RETURN( @US3_LAST_ERRNO );

END$$

-- Returns the row count of buffer extinction info associated with p_bufferID
DROP FUNCTION IF EXISTS count_buffer_extinction$$
CREATE FUNCTION count_buffer_extinction( p_guid      CHAR(36),
                                         p_password  VARCHAR(80),
                                         p_bufferID  INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_extinction INT;

  CALL config();
  SET count_extinction = 0;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    SELECT COUNT(*)
    INTO   count_extinction
    FROM   bufferExtinction
    WHERE  bufferID = p_bufferID;

  END IF;

  RETURN( count_extinction );

END$$

-- INSERTs a new buffer extinction value with the specified information
DROP PROCEDURE IF EXISTS new_buffer_extinction$$
CREATE PROCEDURE new_buffer_extinction ( p_guid                CHAR(36),
                                         p_password            VARCHAR(80),
                                         p_bufferID            INT,
                                         p_lambda              FLOAT,
                                         p_molarExtinctionCoef FLOAT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_bufferID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  IF ( verify_bufferID( p_guid, p_password, p_bufferID ) = @OK ) THEN
    INSERT INTO bufferExtinction SET
      bufferID            = p_bufferID,
      lambda              = p_lambda,
      molarExtinctionCoef = p_molarExtinctionCoef;
    SET @LAST_INSERT_ID = LAST_INSERT_ID();
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing buffer extinction value with the specified information
DROP PROCEDURE IF EXISTS update_buffer_extinction$$
CREATE PROCEDURE update_buffer_extinction ( p_guid                CHAR(36),
                                            p_password            VARCHAR(80),
                                            p_extinctionID        INT,
                                            p_bufferID            INT,
                                            p_lambda              FLOAT,
                                            p_molarExtinctionCoef FLOAT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_bufferID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  IF ( verify_bufferID( p_guid, p_password, p_bufferID ) = @OK ) THEN
    UPDATE bufferExtinction SET
      bufferID            = p_bufferID,
      lambda              = p_lambda,
      molarExtinctionCoef = p_molarExtinctionCoef
    WHERE extinctionID = p_extinctionID;
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns extinction information about all buffers associated with p_bufferID
DROP PROCEDURE IF EXISTS get_buffer_extinction$$
CREATE PROCEDURE get_buffer_extinction ( p_guid      CHAR(36),
                                         p_password  VARCHAR(80),
                                         p_bufferID  INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_bufferID( p_guid, p_password, p_bufferID ) != @OK ) THEN
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_buffer_extinction( p_guid, p_password, p_bufferID ) < 1 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';
  
    SELECT @US3_LAST_ERRNO AS status;
      
  ELSE
    -- Ok, we found some buffer extinction info
    SELECT @OK AS status;
  
    SELECT   extinctionID, lambda, molarExtinctionCoef
    FROM     bufferExtinction
    WHERE    bufferID = p_bufferID
    ORDER BY lambda;
  
  END IF;

END$$

-- DELETEs all extinction information associated with a bufferID
DROP PROCEDURE IF EXISTS delete_buffer_extinction$$
CREATE PROCEDURE delete_buffer_extinction ( p_guid      CHAR(36),
                                            p_password  VARCHAR(80),
                                            p_bufferID  INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN

    DELETE FROM bufferExtinction
    WHERE bufferID = p_bufferID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Functions and procedures associated with the buffer refraction table

-- Returns the row count of buffer refraction info associated with p_bufferID
DROP FUNCTION IF EXISTS count_buffer_refraction$$
CREATE FUNCTION count_buffer_refraction( p_guid      CHAR(36),
                                         p_password  VARCHAR(80),
                                         p_bufferID  INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_refraction INT;

  CALL config();
  SET count_refraction = 0;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    SELECT COUNT(*)
    INTO   count_refraction
    FROM   bufferRefraction
    WHERE  bufferID = p_bufferID;

  END IF;

  RETURN( count_refraction );

END$$

-- INSERTs a new buffer refraction value with the specified information
DROP PROCEDURE IF EXISTS new_buffer_refraction$$
CREATE PROCEDURE new_buffer_refraction ( p_guid            CHAR(36),
                                         p_password        VARCHAR(80),
                                         p_bufferID        INT,
                                         p_lambda          FLOAT,
                                         p_refractiveIndex FLOAT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_bufferID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  IF ( verify_bufferID( p_guid, p_password, p_bufferID ) = @OK ) THEN
    INSERT INTO bufferRefraction SET
      bufferID            = p_bufferID,
      lambda              = p_lambda,
      refractiveIndex     = p_refractiveIndex;
    SET @LAST_INSERT_ID = LAST_INSERT_ID();
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing buffer refraction value with the specified information
DROP PROCEDURE IF EXISTS update_buffer_refraction$$
CREATE PROCEDURE update_buffer_refraction ( p_guid            CHAR(36),
                                            p_password        VARCHAR(80),
                                            p_refractionID    INT,
                                            p_bufferID        INT,
                                            p_lambda          FLOAT,
                                            p_refractiveIndex FLOAT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_bufferID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  IF ( verify_bufferID( p_guid, p_password, p_bufferID ) = @OK ) THEN
    UPDATE bufferRefraction SET
      bufferID            = p_bufferID,
      lambda              = p_lambda,
      refractiveIndex     = p_refractiveIndex
    WHERE refractionID = p_refractionID;
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns refraction information about all buffers associated with p_bufferID
DROP PROCEDURE IF EXISTS get_buffer_refraction$$
CREATE PROCEDURE get_buffer_refraction ( p_guid      CHAR(36),
                                         p_password  VARCHAR(80),
                                         p_bufferID  INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_bufferID( p_guid, p_password, p_bufferID ) != @OK ) THEN
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_buffer_refraction( p_guid, p_password, p_bufferID ) < 1 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';
  
    SELECT @US3_LAST_ERRNO AS status;
      
  ELSE
    -- Ok, we found some buffer refraction info
    SELECT @OK AS status;
  
    SELECT   refractionID, lambda, refractiveIndex
    FROM     bufferRefraction
    WHERE    bufferID = p_bufferID
    ORDER BY lambda;
  
  END IF;

END$$

-- DELETEs all refraction information associated with a bufferID
DROP PROCEDURE IF EXISTS delete_buffer_refraction$$
CREATE PROCEDURE delete_buffer_refraction ( p_guid      CHAR(36),
                                            p_password  VARCHAR(80),
                                            p_bufferID  INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN

    DELETE FROM bufferRefraction
    WHERE bufferID = p_bufferID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

