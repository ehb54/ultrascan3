--
-- alter_reportDocument.sql
--
-- change the label field
--

ALTER TABLE reportDocument
  MODIFY COLUMN label VARCHAR(160) NOT NULL DEFAULT '';

