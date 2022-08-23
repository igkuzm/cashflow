/**
 * File              : chworkdir.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.02.2022
 * Last Modified Date: 30.05.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef k_lib_chworkdir_h__
#define k_lib_chworkdir_h__

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#ifdef __APPLE__
#elif defined _WIN32 || defined _WIN64
#include <Windows.h>
#elif __ANDROID__
#else
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#endif
#include <sys/stat.h>


int k_lib_chWorkDir(char *argv[]);

#ifndef changeWorkDir 
#define changeWorkDir(argv)	\
({	\
	int ___c = k_lib_chWorkDir(argv);\
	___c;	\
})
#endif

#ifdef __APPLE__
#elif defined _WIN32 || defined _WIN64
#elif __ANDROID__
#else
int k_lib_chWorkDir_cp(const char *from, const char *to)
{
	//check time
	struct stat from_st;
	if (stat(from, &from_st) == -1)
		return -1;

	struct stat to_st;
	if (stat(to, &to_st) == -1)
		return -1;

	//don't overwrite if date grater
	if (to_st.st_mtimespec.tv_sec > from_st.st_mtimespec.tv_sec)
		return 1;

	//open streams
	FILE * fp_from = fopen(from, "r");
	if (!fp_from) 
		return -1;

	FILE * fp_to = fopen(to, "w");
	if (!fp_to){
		fclose(fp_from);
		return -1;
	} 

	int ch;
	while ((ch = fgetc(fp_from)) != EOF)
		fputc(ch, fp_to);
	
	fclose(fp_from);
	fclose(fp_to);

	return 0;
}

//list of files in directory
int k_lib_chWorkDir_copy_recursive(const char *source, const char *destination, int depth){
	char error[BUFSIZ];
	struct dirent *entry;
	DIR *dp;	

	dp = opendir(dir);
	if (dp == NULL){
		sprintf(error, "Can't open directory: %s", dir);
		perror(error);
		return ENOENT;
	}	
	while((entry = readdir(dp))){ //reading files
		char *filename = entry->d_name; 
		char source_file[BUFSIZ];
		sprintf("%s/%s", source, filename);
		char dest_file[BUFSIZ];
		sprintf("%s/%s", destination, filename);
		if (depth < 10) {
			switch (entry->d_type) {
				case  DT_REG: k_lib_chWorkDir_cp(source_file, dest_file); break;
				case  DT_LNK: k_lib_chWorkDir_cp(source_file, dest_file); break;
				case  DT_DIR: depth++; k_lib_chWorkDir_copy_recursive(source_file, dest_file, depth); break;
				default: break;
			}
		}
	}
	closedir(dp);
	return 0;
}

#endif

int k_lib_chWorkDir(char *argv[]){
#ifndef __ANDROID__
	char *executable=dirname((char *)argv[0]);
	char workDir[BUFSIZ];
#ifdef __APPLE__
		sprintf(workDir, "%s%s", executable, "/../Resources"); //workdir for Apple is BUNDLE/../Resources
#elif defined _WIN32 || defined _WIN64
		sprintf(workDir, "%s", executable); //workdir for Windows is executable dir
#elif __ANDROID__
#else
		//for Unix systems
		//find homedir and create new dir
		char *progname = basename(argv[0]);
		sprintf(workDir, "%s/%s", getenv("HOME"), progname); 
		
		//create directory if not exists
		struct stat st;
		if (stat(workDir, &st) == -1) {
			if (mkdir(workDir, 0700) == -1) {
				perror("Can't create directory at $(HOME)");
			}			
		}

		//find executable path
		char selfpath[128];
		memset (selfpath, 0, sizeof (selfpath));
		if (readlink ("/proc/self/exe", selfpath, sizeof (selfpath) - 1) < 0)
			{
				perror ("first readlink");
				exit (EXIT_FAILURE);
			};
		execDir=dirname(selfpath);

		char resourceDir[BUFSIZ];
		sprintf(resourceDir, "%s/../share/%s", execDir, progname); //resources dir

		//copy files from resources to workdir
		int k_lib_chWorkDir_copy_recursive(resourceDir, workDir, 0);
		
#endif
	chdir(workDir); //change workdir
	printf("Workdir changed to:%s\n", workDir);
#endif
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif //k_lib_chworkdir_h__
