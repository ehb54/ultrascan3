ALTER TABLE speedstep
  ADD COLUMN setspeed int(11) NOT NULL DEFAULT 0
  AFTER timelast,
  ADD COLUMN avgspeed float NOT NULL DEFAULT 0.0,
  ADD COLUMN speedsdev float NOT NULL DEFAULT 0.0;
