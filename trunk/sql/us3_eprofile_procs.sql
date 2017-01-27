--
-- us3_eprofile_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are related to the extinctionProfile table
-- Run as us3admin
--

DELIMITER $$

-- Verifies that the specified component ID (bufferID, analyteID) exists
DROP FUNCTION IF EXISTS verify_componentID$$
CREATE FUNCTION verify_componentID( p_personGUID    CHAR(36),
                                    p_password      VARCHAR(80),
                                    p_componentID   INT,
                                    p_componentType enum( 'Buffer', 'Analyte', 'Sample' ) )
  RETURNS INT
  READS SQL DATA

BEGIN

  DECLARE count_componentID INT;

  CALL config();
  SET count_componentID = 0;
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( p_componentType = 'Buffer' ) THEN
      SELECT COUNT(*)
      INTO   count_componentID
      FROM   buffer
      WHERE  bufferID = p_componentID;
  
      IF ( count_componentID < 1 ) THEN
        SET @US3_LAST_ERRNO = @NO_BUFFER;
        SET @US3_LAST_ERROR = CONCAT('MySQL: No buffer with ID ',
                                     p_componentID,
                                     ' exists' );
  
      END IF;

    ELSEIF ( p_componentType = 'Analyte' ) THEN
      SELECT COUNT(*)
      INTO   count_componentID
      FROM   analyte
      WHERE  analyteID = p_componentID;
  
      IF ( count_componentID < 1 ) THEN
        SET @US3_LAST_ERRNO = @NO_ANALYTE;
        SET @US3_LAST_ERROR = CONCAT('MySQL: No analyte with ID ',
                                     p_componentID,
                                     ' exists' );
  
      END IF;

    END IF;

  END IF;

  RETURN( @US3_LAST_ERRNO );

END$$

-- Returns the row count of extinctionProfile info associated with component ID and Type
DROP FUNCTION IF EXISTS count_eprofile$$
CREATE FUNCTION count_eprofile( p_personGUID    CHAR(36),
                                p_password      VARCHAR(80),
                                p_componentID   INT,
                                p_componentType enum( 'Buffer', 'Analyte', 'Sample' ) )
  RETURNS INT
  READS SQL DATA

BEGIN

  DECLARE count_component INT;

  CALL config();
  SET count_component = 0;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    SELECT COUNT(*)
    INTO   count_component
    FROM   extinctionProfile
    WHERE  componentID   = p_componentID
    AND    componentType = p_componentType;

  END IF;

  RETURN( count_component );

END$$

-- INSERTs a new extinctionProfile value with the specified information
DROP PROCEDURE IF EXISTS new_eprofile$$
CREATE PROCEDURE new_eprofile( p_personGUID       CHAR(36),
                               p_password         VARCHAR(80),
                               p_componentID      INT,
                               p_componentType    enum( 'Buffer', 'Analyte', 'Sample' ),
                               p_valueType        enum( 'absorbance', 'molarExtinction', 'massExtinction' ),
                               p_xml              LONGTEXT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_componentID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  IF ( verify_componentID( p_personGUID, p_password, p_componentID, p_componentType ) = @OK ) THEN
    INSERT INTO extinctionProfile SET
      componentID      = p_componentID,
      componentType    = p_componentType,
      valueType        = p_valueType,
      xml              = p_xml;
    SET @LAST_INSERT_ID = LAST_INSERT_ID();
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing extinctionProfile value with the specified information
DROP PROCEDURE IF EXISTS update_eprofile$$
CREATE PROCEDURE update_eprofile( p_personGUID       CHAR(36),
                                  p_password         VARCHAR(80),
                                  p_profileID        INT,
                                  p_componentID      INT,
                                  p_componentType    enum( 'Buffer', 'Analyte', 'Sample' ),
                                  p_valueType        enum( 'absorbance', 'molarExtinction', 'massExtinction' ),
                                  p_xml              LONGTEXT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_componentID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  IF ( verify_componentID( p_personGUID, p_password, p_componentID, p_componentType ) = @OK ) THEN
    UPDATE extinctionProfile SET
      componentID      = p_componentID,
      componentType    = p_componentType,
      valueType        = p_valueType,
      xml              = p_xml
      WHERE profileID  = p_profileID;
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns all extinctionProfile info associated with component ID and Type
DROP PROCEDURE IF EXISTS get_eprofile$$
CREATE PROCEDURE get_eprofile( p_personGUID    CHAR(36),
                               p_password      VARCHAR(80),
                               p_componentID   INT,
                               p_componentType enum( 'Buffer', 'Analyte', 'Sample' ) )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_componentID( p_personGUID, p_password, p_componentID, p_componentType ) != @OK ) THEN
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_eprofile( p_personGUID, p_password, p_componentID, p_componentType ) < 1 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';
  
    SELECT @US3_LAST_ERRNO AS status;
      
  ELSE
    -- Ok, we found some extinctionProfile info
    SELECT @OK AS status;
  
    SELECT   profileID, valueType, xml
    FROM     extinctionProfile
    WHERE    componentID   = p_componentID
    AND      componentType = p_componentType;
  
  END IF;

END$$

-- DELETEs all extinction profile information associated with a component ID and Type
DROP PROCEDURE IF EXISTS delete_eprofile$$
CREATE PROCEDURE delete_eprofile( p_personGUID    CHAR(36),
                                  p_password      VARCHAR(80),
                                  p_componentID   INT,
                                  p_componentType enum( 'Buffer', 'Analyte', 'Sample' ) )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN

    DELETE FROM extinctionProfile
    WHERE  componentID   = p_componentID
    AND    componentType = p_componentType;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

