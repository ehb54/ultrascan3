--
-- Alter the permits table to handle collaborations too
--

ALTER TABLE permits
  DROP FOREIGN KEY fk_permits_instrumentID;

ALTER TABLE permits
  MODIFY COLUMN instrumentID INT(11) DEFAULT NULL;
