/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 01 13:21:37 CST			*
 ********************************************************
 *		after studying C 45 days		*
 *		after studying APUE 10 days		*
 ********************************************************/

/*
 * This program demonstrates function
 * struct passwd *getpwent(void);
 * void setpwent(void);
 * void endpwent(void);
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

int main(int argc, char *argv[])
{
	/* struct passwd {
	 * 	char   *pw_name;       username
	 * 	char   *pw_passwd;     user password
	 * 	uid_t   pw_uid;        user ID
	 * 	gid_t   pw_gid;        group ID
	 * 	char   *pw_gecos;      real name
	 * 	char   *pw_dir;        home directory
	 * 	char   *pw_shell;      shell program
	 * }; */
	struct passwd *pd;

	errno = 0;
	//struct passwd *getpwent(void);
	while((pd = getpwent()) != NULL){
		fprintf(stdout, "name = %s\n", pd->pw_name);
		fprintf(stdout, "password = %s\n", pd->pw_passwd);
		fprintf(stdout, "uid = %u\n", pd->pw_uid);
		fprintf(stdout, "gid = %u\n", pd->pw_gid);
		fprintf(stdout, "real name = %s\n", pd->pw_gecos);
		fprintf(stdout, "home directory = %s\n", pd->pw_dir);
		fprintf(stdout, "shell program = %s\n", pd->pw_shell);
		fprintf(stdout, "--------------------\n");
	}
	if(errno != 0){
		perror("getpwent");
		exit(1);
	}

}
