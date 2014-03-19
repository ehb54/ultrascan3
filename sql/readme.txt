readme.txt

US3 Database and Stored Procedures Reference

For notes and a US3 Stored Procedures API reference see the US3 wiki.

the new instance creation relies on the database definition
(us3.sql), all the stored procedures, and the lims code to create a new
instance. The way I do it, I become the us3 user and run the
update-local.sh script first thing. This will update the us3 user's copy of
all of these files from subversion. Then I run the create new instance
stuff.

If any time you forget to update the code first it's not a terribly big
deal. You can do a subversion update of the lims code, and you can update
the stored procedures like this:

mysql -u user -p database <us3_procedures.sql

That file sources all the other sql files. If the us3.sql file was out of
date it's a little more difficult, because there are some records that get
populated in there as part of the setup process too. However, I believe I
have scripts that can be used to refresh that too.

