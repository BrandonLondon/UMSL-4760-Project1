#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>

//Function Prototype
int sizepathfun(char *path, char *options, int scale);
int depthfirstapply(char *dir, int pathfun(char *path, char *options, int scale), char *options, int scale, int depth, int ino, int math_depth);
void humanReadable(int size, char *pathname, char *options, int scale);

/*========================================================================================================================================================================
 ========================================================================== ShowTreeSize ====================================================================================
========================================================================================================================================================================*/

//This function simply prints the final calculated size and passes it into humanreadaable if the option is enabled
int showtreesize(char *path, int pathfun(char *path, char *options, int scale), char *options, int scale, int ino, int depth, int max_depth)
{
	//variable to hold the final size
	int size = depthfirstapply(path, sizepathfun, options, scale, depth, ino, max_depth);
	//if H was called print value as humanReadable
	if((strstr(options, "H") != NULL) || (strstr(options, "m") != NULL) || (strstr(options, "B") != NULL))
	{
		humanReadable(size, path, options, scale);
	}
	else
	{
	//else print as normal	
		printf("%-7d %s\n", size, path);
	}

	return size;
}
/*===========================================================================================================================================================================
========================================================================= Human Readable function ===========================================================================
===========================================================================================================================================================================*/
void humanReadable(int size, char *pathname, char *options, int scale)
{
	const char *sizesuffix = " "; 	
	if(strstr(options, "H") != NULL)
	{
		if(size >= 1000000000)
		{
			size = (long long) (size/1000000000);
			sizesuffix = "G";
		}
		else if(size >= 1000000 )	
		{
			size = (long long)(size/1000000);
			sizesuffix = "M";
		}
		else if(size >= 1000)
		{	
			size = (long long)(size/1000);
			sizesuffix = "K";
		}
		printf("%d%-7s %s\n", size, sizesuffix, pathname);
	}
	if(strstr(options, "B") != NULL)
	{
		size = size/scale;
		if(size < 1)size=1;
		printf("%-7d %s\n", size, pathname);
		
	}
	if(strstr(options, "m") != NULL)
	{
		size = size / 1000000;
		if(size < 1) size=1;
		printf("%d%-7s %s\n", size, "M", pathname);
	}

}


/* ========================================================================================================================================================================
============================================================================== MAIN =======================================================================================
==========================================================================================================================================================================*/
int main(int argc, char *argv[])
{
	//This hold a string of options
	char option_string[10];
	//scale just holds the number that it needs to be scaled by
	int scale = 0;
	int max_depth = 0;
	//initalize variable for getopt
	int opt;
	// concatonate commands for later use
	while((opt = getopt(argc, argv, "hLHbacsB:md:")) != -1)
	{
		switch(opt)
		{
			case 'h':
				printf("NAME:\n");
				printf("	%s - traverse a specified directory in depth-first order.\n", argv[0]);
				printf("\nUSAGE:\n");
				printf("	%s mydu [-h] [-B M | -m] [-c] [-d N] [-H] [-L][-s] <dirname> <dirname>.\n", argv[0]);
				printf("\nDESCRIPTION:\n");
				printf("	-h	: Print a help message and exit.\n");
				printf("	-a	: Write Counts for all files, not just Directories\n");
				printf("	-B M	: Scale sizes by M before printing, for example, -BM prints size in units of 1,048,576\n");
				printf("	-b	: Print sizes in bytes\n");
				printf("	-c	: Prints a grand total\n");
				printf("	-d N	: Print the total for a directory only if it is N or fewer levels below the command line argument\n");
				printf("	-H	: Human Readable; print size in Human readable format, for example 1K, 234M, 2G\n");
				printf("	-L	: Derefrences all symbolic links, by default, you will not dereference symbolic links.\n");
				printf("	-m	: Same as -B 1,048,576\n");
				printf("	-s	: Display Total for each.\n");
				return EXIT_SUCCESS;

			case 'L':
				strcat(option_string, "L");
				break;		
			case 'H':
				strcat(option_string, "H");
				break;
			case 'b':
				strcat(option_string, "b");
				break;				
			case 'B':
				strcat(option_string, "B");
				scale = atoi(optarg);
				break;
			case 'm':
				strcat(option_string, "m");
				scale = 1048576;
				break;
			case 'a':
				strcat(option_string, "a");
				break;
			case 'c':
				strcat(option_string, "c");
				break;
			case 's':
				strcat(option_string, "s");
				break;
			case 'd':
				strcat(option_string, "d");
				max_depth = atoi(optarg);

			default:
	
				fprintf(stderr, "%s: Please use \"-h\" option for more info.\n", argv[0]);
				return EXIT_FAILURE;
		}
	}
	//Holds stats for initial directory, this is used to gather info on inode
	
	struct stat stats;
	stat(".", &stats);
	//Used for later for linked list
	int inode = stats.st_ino;
	//If the first directory is not listed then set to current directory else make the first argument the directory and keep going until nothing is left
	char *pwd = ".";
	int totalsize = 0;

	if (argv[optind] == NULL) 
	{
 		int size = sizepathfun(pwd, option_string, scale);
 		if (size >= 0) totalsize += size;
  		else totalsize += showtreesize(pwd, sizepathfun, option_string, scale, inode, 0, max_depth);
	}
	else 
	{
  		for (; optind < argc; optind++) 
		{
    			char *path = argv[optind];
			int size = sizepathfun(path, option_string, scale);
    			if (size >= 0) totalsize += size;
    			else totalsize += showtreesize(path, sizepathfun, option_string, scale, inode, 0, max_depth);
  		}
	}
	if(((strstr(option_string, "c") != NULL)) && (strstr(option_string, "H") != NULL))
	{
		humanReadable(totalsize, "TOTAL", option_string, scale);
		
	}
	else if (strstr(option_string, "c") != NULL)
  	{
		printf("%-7d%s\n", totalsize, "TOTAL");
	}
	
	//When program is finished
	return EXIT_SUCCESS;
}

/*============================================================================================================================================================================
 ============================================================================= SIZEPATHFUN ===================================================================================
============================================================================================================================================================================*/
//This function will return either bytes or blocks depending on the options selected
int sizepathfun(char *path, char *options, int scale){
	//hold stat value for current file to check if its a normal file, if not return -1
	struct stat statbuf;
	if(stat(path, &statbuf) == -1)
	{
		perror("failed to get file status");	
		return -1;
	}
	if(S_ISREG(statbuf.st_mode) == 0) 
	{
		return -1;
	}
	else
	{
		//if options are b, B, or M return bytes else return blocks
		if((strstr(options, "b") != NULL) || (strstr(options, "B") != NULL) || (strstr(options, "m") != NULL)) 
		{
			return statbuf.st_size;
		}
		else
		{
			return (statbuf.st_blocks/2);
		}
	}
}
/*=============================================================================================================================================================================
 ========================================================================== DepthFirstApply====================================================================================
=============================================================================================================================================================================*/

//This function uses DepthFirst to transverse directories
int depthfirstapply(char *path, int pathfun(char *path, char *options, int scale), char *options, int scale, int depth, int ino, int max_depth) {
	
        DIR *dir;
        struct dirent *entry;
        struct stat info;
	//tries to open directory of the path, if it cant it returns -1
        if (!(dir = opendir(path))) return -1;
	//initalize integer to hold the result
        int result = 0;
	//while directory is not at the end of stream, repeat
        while ((entry = readdir(dir)) != NULL) {
		// holds name of current directory
                char *name = entry->d_name;
		//Holds pathname for current directory
		char pathname[4096];
		//save the pathname
		sprintf(pathname, "%s/%s", path, name);
		//Looks to see if -L option is called so it can transverse links
		if(strstr(options, "L") != NULL)
		{
                stat(pathname, &info);
		}
		else
		{
                lstat(pathname, &info);
		}
                mode_t mode = info.st_mode;
		//holds inode information about the inode so it doesnt loop and tranverses Link correctly
		int inode = info.st_ino;
		//check and see if function is a directory
                if (S_ISDIR(mode)) {
			//checks to see if L options
			if(strstr(options, "L") != NULL)
			{
			if (inode == ino && depth != 0) continue;
			}
			//Skip . and .. directories as to not repeat
                        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
			//go into next depth for recursion
                        int size = depthfirstapply(pathname, pathfun, options, scale, depth + 1, ino, max_depth);
			if (strstr(options, "b") == NULL)
			{
				size += info.st_size;
			}
			// check for the size and based on options print out
                        if (size >= 0) {
                                result += size;
				//if s skip printing
				if(strstr(options, "s") != NULL) continue;
				//if we need to do some formating go into human readable
				if((strstr(options, "H") != NULL) || (strstr(options, "B") != NULL) || (strstr(options, "m") != NULL))
				{
					humanReadable(size, pathname, options, scale);
				}
				else if((strstr(options, "d") != NULL) && depth >= max_depth)
				{
					humanReadable(size, pathname, options, scale);
				}
				
				else if((strstr(options, "d") != NULL) && depth < max_depth);
				else
				{
                                	printf("%-7d %s\n", size, pathname);
				}
                        }
                } else {
                        int size = pathfun(pathname, options, scale);
                        if (size > 0) result += size;
			//check for s if it is then skip printing	
			if(strstr(options, "s") != NULL) continue;
			//check and see if B or M was  called if so Make the default size 1 before printing
			if(((strstr(options, "B") != NULL) || (strstr(options, "m") != NULL)) && size < 1) size = 1;
			//If -a was called print out the files with directorys
			if(strstr(options, "a") != NULL)
			{
				//again formating go to humanreadable.
				if((strstr(options, "H") != NULL) || (strstr(options, "B") != NULL) || (strstr(options, "m") != NULL))
				{
					humanReadable(size, pathname, options, scale);
				}

				else if((strstr(options, "d") != NULL) && depth >= max_depth)
				{
					humanReadable(size, pathname, options, scale);
				}
				
				else if((strstr(options, "d") != NULL) && depth < max_depth);
				else
				{
                                	printf("%-7d %s\n", size, pathname);
				}
			} 	
                }
        }
	//close current directoy
        closedir(dir);
	//return result total
        return result;
}
