--
-- Update people with userlevel=4.5 for new address (uthscsa-to-montana)
--

-- UPDATE TABLE people

UPDATE people
  SET address='32 Campus Drive',
      city='Missoula',
      state='MT',
      zip='59812',
      organization='University of Montana'
  WHERE userlevel>3 AND organization LIKE 'UTH%';

UPDATE people
  SET phone='406-285-1935', email='borries.demeler@umontana.edu'
  WHERE lname='Demeler' AND fname='Borries';

UPDATE people
  SET phone='832-466-9211', email='gegorbet@gmail.com'
  WHERE lname='Gorbet';

UPDATE people
  SET phone='555-555-5555', email='alexsav.science@gmail.com',
      fname='Alexey', lname='Savelyev', username='us3savelyev'
  WHERE lname='Zollars';

