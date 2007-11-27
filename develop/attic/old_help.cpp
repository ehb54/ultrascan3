
int Context_help(QString sys_str1, QString sys_str2)
{
	int	fork_result;
	int	return_value;

	fork_result = fork();
	if (fork_result == -1)	/* error */
	{
		/* check errno for type of error */
		return_value = -1; // errno;
	}
	else if (fork_result == 0)	/* I am the child */
	{
		/* do exec or system here to start the new process */
		Launch_help(sys_str1, sys_str2);
		/* The child has finished it's usefullness, time to go away. */
		exit(0);
	}
	else	/* I am the parent */
	{
		/* child forked successfully */
		return_value = 0;
	}
	return (return_value);
}	/* function Context_help */

/* Launch_help
 *
 * Launches Netscape via a system call. Since system blocks until
 * the called process returns, Launch_help should only be called by 
 * a child process.  Preferrably, Launch_help should only be called 
 * by the function Context_help!
 */
int Launch_help(QString sys_str1, QString sys_str2)
{
	if (system(sys_str1) != 0)
	{	/* netscape not open */
		if (system(sys_str2) != 0)
		{	/* problem with netscape */
			/* How do we tell the user that there was a problem?
			 * Theoretically, we are currently a child fork of the main
			 * program.  That means we still have access to most
			 * everything the main program has.
			 */
				return (-1);
			/*
			 * Can we call some sort of error mesage routine?
			 * Should we just create a new window with an error message?
			 * Should we just proceed and worry about that later?
			 */
		}
	}	/* if system != 0 */
	return(0);
}	/* Launch_help */

