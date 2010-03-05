--
-- us3_hardware_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are related to various tables pertaining to hardware
-- Run as root
--

DELIMITER $$

--
-- Rotor procedures
--

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

--
-- Lab procedures
--

-- Returns the count of all labs in db
DROP FUNCTION IF EXISTS count_labs$$
CREATE FUNCTION count_labs ( p_guid     CHAR(36),
                             p_password VARCHAR(80) )
  RETURNS INT
  READS SQL DATA

BEGIN

  DECLARE count_labs INT;

  CALL config();
  SET count_labs = 0;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    SELECT    COUNT(*)
    INTO      count_labs
    FROM      lab;

  END IF;

  RETURN( count_labs );

END$$

-- SELECTs names of all labs
DROP PROCEDURE IF EXISTS get_lab_names$$
CREATE PROCEDURE get_lab_names ( p_guid     CHAR(36),
                                 p_password VARCHAR(80) )
  READS SQL DATA

BEGIN
  DECLARE count_labs INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    SELECT    COUNT(*)
    INTO      count_labs
    FROM      lab;

    IF ( count_labs = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT labID, name
      FROM lab
      ORDER BY name;
 
    END IF;

  END IF;

END$$

-- Returns a more complete list of information about one lab
DROP PROCEDURE IF EXISTS get_lab_info$$
CREATE PROCEDURE get_lab_info ( p_guid     CHAR(36),
                                p_password VARCHAR(80),
                                p_labID  INT )
  READS SQL DATA

BEGIN
  DECLARE count_labs INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_labs
  FROM       lab
  WHERE      labID = p_labID;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    IF ( count_labs = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   name, building, room
      FROM     lab
      WHERE    labID = p_labID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- adds a new lab
DROP PROCEDURE IF EXISTS add_lab$$
CREATE PROCEDURE add_lab ( p_guid            CHAR(36),
                           p_password        VARCHAR(80),
                           p_name            TEXT,
                           p_building        TEXT,
                           p_room            TEXT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;

  IF ( verify_userlevel( p_guid, p_password, @US3_ADMIN ) = @OK ) THEN
    INSERT INTO lab SET
      name              = p_name,
      building          = p_building,
      room              = p_room,
      dateUpdated       = NOW();

    SET @LAST_INSERT_ID = LAST_INSERT_ID();

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

--
-- Instrument procedures
--

-- Returns the count of all instruments in db
DROP FUNCTION IF EXISTS count_instruments$$
CREATE FUNCTION count_instruments ( p_guid     CHAR(36),
                                    p_password VARCHAR(80) )
  RETURNS INT
  READS SQL DATA

BEGIN

  DECLARE count_instruments INT;

  CALL config();
  SET count_instruments = 0;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    SELECT    COUNT(*)
    INTO      count_instruments
    FROM      instrument;

  END IF;

  RETURN( count_instruments );

END$$

-- SELECTs names of all instruments
DROP PROCEDURE IF EXISTS get_instrument_names$$
CREATE PROCEDURE get_instrument_names ( p_guid     CHAR(36),
                                        p_password VARCHAR(80) )
  READS SQL DATA

BEGIN
  DECLARE count_instruments INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    SELECT    COUNT(*)
    INTO      count_instruments
    FROM      instrument;

    IF ( count_instruments = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT instrumentID, name
      FROM instrument
      ORDER BY name;
 
    END IF;

  END IF;

END$$

-- Returns a more complete list of information about one instrument
DROP PROCEDURE IF EXISTS get_instrument_info$$
CREATE PROCEDURE get_instrument_info ( p_guid     CHAR(36),
                                       p_password VARCHAR(80),
                                       p_instrumentID  INT )
  READS SQL DATA

BEGIN
  DECLARE count_instruments INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_instruments
  FROM       instrument
  WHERE      instrumentID = p_instrumentID;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    IF ( count_instruments = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   name, serialNumber
      FROM     instrument
      WHERE    instrumentID = p_instrumentID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- adds a new instrument
DROP PROCEDURE IF EXISTS add_instrument$$
CREATE PROCEDURE add_instrument ( p_guid            CHAR(36),
                                  p_password        VARCHAR(80),
                                  p_name            TEXT,
                                  p_serialNumber    TEXT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;

  IF ( verify_userlevel( p_guid, p_password, @US3_ADMIN ) = @OK ) THEN
    INSERT INTO instrument SET
      name              = p_name,
      serialNumber      = p_serialNumber,
      dateUpdated       = NOW();

    SET @LAST_INSERT_ID = LAST_INSERT_ID();

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

DELIMITER ;

