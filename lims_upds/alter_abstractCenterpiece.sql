--
-- alter_abstractCenterpiece.sql
--
-- change the fill method
--

ALTER TABLE abstractCenterpiece
  MODIFY COLUMN loadMethod enum('top', 'side') NULL;

