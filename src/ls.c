#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>

int ls(char *path) {
	struct dirent *l;
	DIR *dirp;
	dirp = opendir(path);
	char name[256];
	if (dirp == NULL) {
		sprintf(name, "%s", path);
		perror(name);
		return errno;
	}
	while((l = readdir(dirp)) != NULL) {
		printf("%s\n", l->d_name);
	}
	closedir(dirp);
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: ./lsm dir");
		return EINVAL;
	}
	return ls(argv[1]);
}
