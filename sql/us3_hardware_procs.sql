--
-- us3_hardware_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are related to various tables pertaining to hardware
-- Run as root
--

DELIMITER $$

-- Returns the count of all rotors in db
DROP FUNCTION IF EXISTS count_rotors$$
CREATE FUNCTION count_rotors ( p_guid     CHAR(36),
                               p_password VARCHAR(80) )
  RETURNS INT
  READS SQL DATA

BEGIN

  DECLARE count_rotors INT;

  CALL config();
  SET count_rotors = 0;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    SELECT    COUNT(*)
    INTO      count_rotors
    FROM      rotor;

  END IF;

  RETURN( count_rotors );

END$$

-- SELECTs names of all rotors
DROP PROCEDURE IF EXISTS get_rotor_names$$
CREATE PROCEDURE get_rotor_names ( p_guid     CHAR(36),
                                   p_password VARCHAR(80) )
  READS SQL DATA

BEGIN
  DECLARE count_rotors INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    SELECT    COUNT(*)
    INTO      count_rotors
    FROM      rotor;

    IF ( count_rotors = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT rotorID, name
      FROM rotor
      ORDER BY name;
 
    END IF;

  END IF;

END$$

-- Returns a more complete list of information about one rotor
DROP PROCEDURE IF EXISTS get_rotor_info$$
CREATE PROCEDURE get_rotor_info ( p_guid     CHAR(36),
                                  p_password VARCHAR(80),
                                  p_rotorID  INT )
  READS SQL DATA

BEGIN
  DECLARE count_rotors INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_rotors
  FROM       rotor
  WHERE      rotorID = p_rotorID;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    IF ( count_rotors = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   r.name, serialNumber, stretchFunction, omega2_t,
               a.name
      FROM     rotor r, abstractRotor a
      WHERE    r.abstractRotorID = a.abstractRotorID
      AND      rotorID = p_rotorID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- adds a new rotor using an abstractRotor
DROP PROCEDURE IF EXISTS add_rotor$$
CREATE PROCEDURE add_rotor ( p_guid            CHAR(36),
                             p_password        VARCHAR(80),
                             p_abstractRotorID INT,
                             p_name            TEXT,
                             p_serialNumber    TEXT )
  MODIFIES SQL DATA

BEGIN
  DECLARE count_abstract_rotors INT;
  DECLARE l_defaultStretch      TEXT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;

  SELECT     COUNT(*)
  INTO       count_abstract_rotors
  FROM       abstractRotor
  WHERE      abstractRotorID = p_abstractRotorID;

  IF ( verify_userlevel( p_guid, p_password, @US3_ADMIN ) = @OK ) THEN
    IF ( count_abstract_rotors < 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_ROTOR;
      SET @US3_LAST_ERROR = CONCAT('MySQL: No abstract rotor with ID ',
                                   p_abstractRotorID,
                                   ' exists' );

    ELSE
      SELECT    defaultStretch
      INTO      l_defaultStretch
      FROM      abstractRotor
      WHERE     abstractRotorID = p_abstractRotorID;

      INSERT INTO rotor SET
        abstractRotorID   = p_abstractRotorID,
        name              = p_name,
        serialNumber      = p_serialNumber,
        stretchFunction   = l_defaultStretch,
        omega2_t          = 0.0,
        dateUpdated       = NOW();
        
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Get a list of abstract rotor names
DROP PROCEDURE IF EXISTS get_abstractRotor_names$$
CREATE PROCEDURE get_abstractRotor_names ( p_guid     CHAR(36),
                                           p_password VARCHAR(80) )
  READS SQL DATA

BEGIN
  DECLARE count_abstract_rotors INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    SELECT    COUNT(*)
    INTO      count_abstract_rotors
    FROM      abstractRotor;

    IF ( count_abstract_rotors = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   abstractRotorID, name
      FROM     abstractRotor
      ORDER BY name;
 
    END IF;

  END IF;

END$$

DELIMITER ;

