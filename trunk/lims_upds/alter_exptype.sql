--
-- alter_abstractCenterpiece.sql
--
-- change the fill method
--

ALTER TABLE experiment
  MODIFY COLUMN type ENUM('velocity', 'equilibrium', 'diffusion', 'buoyancy','calibration', 'other') NULL
    DEFAULT 'velocity';

