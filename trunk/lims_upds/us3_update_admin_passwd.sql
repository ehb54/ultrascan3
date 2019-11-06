--
-- us3_update_admin_passwd.sql
--
-- Updates the password for all accounts with userlevel >= 4
--
-- mysql> select MD5( 'TSv3dberg' );
-- +----------------------------------+
-- | MD5( 'TSv3dberg' )               |
-- +----------------------------------+
-- | 76a6c9a17a598c00343cfc9a04ec5ea2 |
-- +----------------------------------+
-- 1 row in set (0.00 sec)
-- 
-- mysql>
--

UPDATE people
SET    password = '76a6c9a17a598c00343cfc9a04ec5ea2'
WHERE  userlevel >= 4;
