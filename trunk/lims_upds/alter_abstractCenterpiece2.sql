--
-- alter_abstractCenterpiece.sql
--
-- change the fill method
--

ALTER TABLE abstractCenterpiece
  MODIFY COLUMN 
  shape enum( 'standard', 'rectangular', 'circular', 'synthetic',
              'band forming', 'meniscus matching', 'sector' )
              NOT NULL DEFAULT 'standard';

