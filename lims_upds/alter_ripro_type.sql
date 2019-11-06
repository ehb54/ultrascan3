--
-- alter_ripro_type.sql
--
-- change the RIProfile type
--

ALTER TABLE experiment
  MODIFY COLUMN RIProfile LONGTEXT DEFAULT '';

