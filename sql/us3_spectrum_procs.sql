--
-- us3_spectrum_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are related to the spectrum table
-- Run as us3admin
--

DELIMITER $$

-- Verifies that the specified component ID (bufferID, analyteID) exists
DROP FUNCTION IF EXISTS verify_componentID$$
CREATE FUNCTION verify_componentID( p_personGUID    CHAR(36),
                                    p_password      VARCHAR(80),
                                    p_componentID   INT,
                                    p_componentType enum( 'Buffer', 'Analyte' ) )
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

    ELSE  -- p_componentType = 'Analyte'
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

-- Returns the row count of spectrum info associated with p_componentID
DROP FUNCTION IF EXISTS count_spectrum$$
CREATE FUNCTION count_spectrum( p_personGUID    CHAR(36),
                                p_password      VARCHAR(80),
                                p_componentID   INT,
                                p_componentType enum( 'Buffer', 'Analyte' ),
                                p_opticsType    enum( 'Extinction', 'Refraction', 'Fluorescence' ) )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_component INT;

  CALL config();
  SET count_component = 0;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    SELECT COUNT(*)
    INTO   count_component
    FROM   spectrum
    WHERE  componentID   = p_componentID
    AND    componentType = p_componentType
    AND    opticsType    = p_opticsType;

  END IF;

  RETURN( count_component );

END$$

-- INSERTs a new spectrum value with the specified information
DROP PROCEDURE IF EXISTS new_spectrum$$
CREATE PROCEDURE new_spectrum( p_personGUID       CHAR(36),
                               p_password         VARCHAR(80),
                               p_componentID      INT,
                               p_componentType    enum( 'Buffer', 'Analyte' ),
                               p_opticsType       enum( 'Extinction', 'Refraction', 'Fluorescence' ),
                               p_lambda           FLOAT,
                               p_molarCoefficient FLOAT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_componentID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  IF ( verify_componentID( p_personGUID, p_password, p_componentID, p_componentType ) = @OK ) THEN
    INSERT INTO spectrum SET
      componentID      = p_componentID,
      componentType    = p_componentType,
      opticsType       = p_opticsType,
      lambda           = p_lambda,
      molarCoefficient = p_molarCoefficient;
    SET @LAST_INSERT_ID = LAST_INSERT_ID();
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing spectrum value with the specified information
DROP PROCEDURE IF EXISTS update_spectrum$$
CREATE PROCEDURE update_spectrum( p_personGUID       CHAR(36),
                                  p_password         VARCHAR(80),
                                  p_spectrumID       INT,
                                  p_componentID      INT,
                                  p_componentType    enum( 'Buffer', 'Analyte' ),
                                  p_opticsType       enum( 'Extinction', 'Refraction', 'Fluorescence' ),
                                  p_lambda           FLOAT,
                                  p_molarCoefficient FLOAT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_componentID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  IF ( verify_componentID( p_personGUID, p_password, p_componentID, p_componentType ) = @OK ) THEN
    UPDATE spectrum SET
      componentID      = p_componentID,
      componentType    = p_componentType,
      opticsType       = p_opticsType,
      lambda           = p_lambda,
      molarCoefficient = p_molarCoefficient
    WHERE spectrumID   = p_spectrumID;
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns all spectrum info associated with p_componentID
DROP PROCEDURE IF EXISTS get_spectrum$$
CREATE PROCEDURE get_spectrum( p_personGUID    CHAR(36),
                               p_password      VARCHAR(80),
                               p_componentID   INT,
                               p_componentType enum( 'Buffer', 'Analyte' ),
                               p_opticsType    enum( 'Extinction', 'Refraction', 'Fluorescence' ) )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_componentID( p_personGUID, p_password, p_componentID, p_componentType ) != @OK ) THEN
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_spectrum( p_personGUID, p_password, p_componentID, p_componentType, p_opticsType ) < 1 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';
  
    SELECT @US3_LAST_ERRNO AS status;
      
  ELSE
    -- Ok, we found some spectrum info
    SELECT @OK AS status;
  
    SELECT   spectrumID, lambda, molarCoefficient
    FROM     spectrum
    WHERE    componentID   = p_componentID
    AND      componentType = p_componentType
    AND      opticsType    = p_opticsType
    ORDER BY lambda;
  
  END IF;

END$$

-- DELETEs all extinction information associated with a spectrumID
DROP PROCEDURE IF EXISTS delete_spectrum$$
CREATE PROCEDURE delete_spectrum( p_personGUID    CHAR(36),
                                  p_password      VARCHAR(80),
                                  p_componentID   INT,
                                  p_componentType enum( 'Buffer', 'Analyte' ),
                                  p_opticsType    enum( 'Extinction', 'Refraction', 'Fluorescence' ) )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN

    DELETE FROM spectrum
    WHERE  componentID   = p_componentID
    AND    componentType = p_componentType
    AND    opticsType    = p_opticsType;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

