#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <limits.h>
long long getItemSize(char *dirname) {
	struct stat st;

	DIR *dir = opendir(dirname);
    if (dir == 0){ // Cant open as a directory
		if (lstat(dirname, &st) != 0){ return 0; } // Cant get file info
		return st.st_size;
	}
	long long total_size = 0;
	
	struct dirent *dit;
	while ((dit = readdir(dir)) != NULL) {
		// If it's one of the dot directories
		if ( (strcmp(dit->d_name, ".") == 0) || (strcmp(dit->d_name, "..") == 0) ){ continue; }
		
		char filePath[1024];
        sprintf(filePath, "%s/%s", dirname, dit->d_name);
		if (lstat(filePath, &st) != 0){ continue; } // Can't open file
		if(S_ISLNK(st.st_mode)){ continue; } // Is a alias, not a file
		
        if (S_ISDIR(st.st_mode)) { // Is another Directory
            total_size += getItemSize(filePath) + st.st_size;
        } else { // Is File
            total_size += st.st_size;
        }
	}
	closedir(dir);
    return total_size;
}

char* readableSize(double size/*in bytes*/, char *buf) {
    int i = 0;
    const char* sizeUnits[] = {"\e[38;5;226mB", "\e[38;5;214mkB", "\e[38;5;202mMB", "\e[38;5;196mGB", "TB", "PB", "EB", "ZB", "YB"};
    while (size > 1000) {
        size /= 1000; i++;
	}
	if(i){
		sprintf(buf, "%5.1f%s%s", size, sizeUnits[i], "\e[0m");
	} else {
		sprintf(buf, "%5d%s%s ", (int) size, sizeUnits[i], "\e[0m");
	}
    return buf;
}
char* getTime(time_t inTime, char *buf){
	time_t now = time(NULL);
	long int timeAgo = (now - inTime);
	
	int i = 0;
	const int times[] = {60, 60, 24, 30, 12, INT_MAX};
	const char* timeUnits[] = {"\e[38;5;124ms", "\e[38;5;142mm", "\e[38;5;130mh", "\e[38;5;148md", "\e[38;5;136mM", "\e[38;5;154my"};
	while (timeAgo > times[i]) {
		timeAgo /= times[i]; i++;
	}
	sprintf(buf, "%2ld%s%s", timeAgo, timeUnits[i], "\e[0m");
	return buf;
}

int main(int argc, char **argv){
	DIR *dir = opendir(".");
    if (dir == 0){ return 0; } // Cant open directory
	
	struct dirent *dit;
	while ((dit = readdir(dir)) != NULL) {
		char* fileName = dit->d_name;
		char printName[strlen(fileName)+10];
		// If it's one of the dot directories
		if ( (strcmp(fileName, ".") == 0) || (strcmp(fileName, "..") == 0) ){ continue; }
		
		struct stat fileStat;
		char filePath[260];
        sprintf(filePath, "./%s", fileName);
		if (lstat(filePath, &fileStat) != 0){ continue; } // Can't open file
		if(S_ISLNK(fileStat.st_mode)){ continue; } // Is a alias, not a file
		
		int statchmod = fileStat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
		char buf[10];
		char *size = readableSize(getItemSize(filePath), buf);
		char buf2[20];
		char *date = getTime(fileStat.st_mtime, buf2);
		
		if (S_ISDIR(fileStat.st_mode)) { // Is Directory
			strcpy(printName, "\e[1;38;5;54m");
			strcat(printName, fileName);
			strcat(printName, "\e[0m");
        } else { // Is File
			strcpy(printName, fileName);
		}
		printf("%3o %s %s %s\n", statchmod, date, size, printName);
	}
	closedir(dir);
	return 0;
}
