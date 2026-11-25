# Analysis profile loading tests

`test_us_analysis_profile_inherit_gui` loads a profile with
`US_AnalysisProfileGui::inherit_protocol()` for each case in
`profile_fixture.h` and compares every initialized field of the profile, the
messages shown and the progress calls with `fixtures/<case>.json`.
`check_snapshots.py` runs each case in its own process. The comparison does not
use the XML or `US_AnaProfile::operator==`, which leave fields out.

Both tests link `us_analysis_profile_lib`, the library of the program.

`aprofID`, `protoID` and `nchan` are not initialized by the profile's
constructor, so the cases set them to -101, -102 and -103 to show which ones a
load changes. Four report fields nothing initializes (`scan_number`,
`load_volume`, `av_rotor_speed`, `av_temperature`) are recorded as a label.

## Running

Build the TEST profile and run `ctest -R test_us_analysis_profile`. The
`database` cases are skipped unless `US3_PROFILE_DB_HOST` is set to the
`host:port` of a disposable MariaDB server with TLS, the root password
`profile1329` and a database named `profile1329` with `fixture_database.sql`
loaded. That file creates an account without a password, so do not load it on a
shared server. On Linux the client checks the server's certificate with
`etc/mysql/ca-cert.pem` in the build directory.

With qmake, build `utils`, `qwtplot3d`, `gui` and `programlibs/analysis`, then
run qmake and make in this directory and `check_snapshots.py` on the program.

To record again: `python3 check_snapshots.py <program> --record`. Review each
difference.

## Results that are recorded as they are

- A wavelength to edit that is not one of the channel's becomes the middle one.
  Wavelengths with a fraction become 0.
- Replicate group numbers are limited to the number of channels, and
  use-references to half of it or, with a report off, to the highest reference
  of such a channel. ABDE with a report off limits them to the highest reference.
- ABDE with every report on clears Run and Report of the first B channel.
- The reports of an "(unspecified)" channel replace the reports after it and keep
  their durations.
- Values go through text with six digits: 1.23456789 becomes 1.23457.
- Left-out optics and B:Interf channels leave lists of different lengths.
- A profile read from disk still reads its reports from the database, and shows a
  message when it cannot.
