-- -----------------------------------------------------
-- Table notice
-- -----------------------------------------------------
DROP TABLE IF EXISTS notice ;

CREATE  TABLE IF NOT EXISTS notice (
  id int(11) NOT NULL AUTO_INCREMENT ,
  type enum('info','warn','crit'),
  revision VARCHAR(12) NULL DEFAULT NULL,
  message LONGTEXT NULL DEFAULT NULL ,
  lastUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  PRIMARY KEY (id) )
ENGINE = InnoDB;

## Grants for us3_notice@% ##
GRANT USAGE ON *.* TO 'us3_notice'@'%' IDENTIFIED BY PASSWORD '*95BD446FAE62A858AB140023019D162269B6B9F6';
GRANT ALL PRIVILEGES ON `us3_notice`.* TO 'us3_notice'@'%';

