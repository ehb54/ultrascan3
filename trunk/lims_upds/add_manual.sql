ALTER TABLE buffer
  ADD COLUMN manual tinyint(1) NOT NULL DEFAULT 0
  AFTER density;
