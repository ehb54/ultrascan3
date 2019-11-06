--
-- Alter/Update Project table to have lastUpdated field
--

ALTER TABLE project
  ADD COLUMN lastUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
  AFTER status;

UPDATE project SET lastUpdated = now();

