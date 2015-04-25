#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <time.h>
#include "ifs.h"

static void process_directory(char *path, const char *rel_path);

int main(int argc, char *argv[])
{
	void *img = (void *)malloc(0x1000000);

	ifs_create_image(img, 0x1000000);
	char *outp = "initrd.img";
	char *tdir = NULL;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-d") == 0)
			tdir = argv[++i];
		else if (strcmp(argv[i], "-o") == 0)
			outp = argv[++i];
	}
	FILE *dmp = fopen(outp, "w");

	process_directory(tdir, "");
	fwrite(img, 1, 0x1000000, dmp);

	fclose(dmp);
	return 0;
}

static void process_directory(char *path, const char *rel_path)
{
	struct dirent *ent;
	DIR *dir = opendir(path);
	char fpath[1000];
	char rpath[1000];

	memset(fpath, 0, 1000);
	memset(rpath, 0, 1000);
	while ((ent = readdir(dir)) != NULL) {
		if (strcmp(&ent->d_name, ".") && strcmp(&ent->d_name, "..")) {
			sprintf(fpath, "%s/%s", path, ent->d_name);
			sprintf(rpath, "%s/%s", rel_path, ent->d_name);

			if (ent->d_type == DT_DIR) {
				ifs_mkdir(remove_leading_slash(rpath));
				process_directory(fpath, rpath);
			} else if (ent->d_type == DT_REG) {
				FILE *f = fopen(fpath, "r");
				int size = 0;
				fseek(f, 0L, SEEK_END);
				size = ftell(f);
				fseek(f, 0L, SEEK_SET);
				char *tmp = (char *)malloc(size);
				fread(tmp, 1, size, f);
				fclose(f);
				ifs_add_file(remove_leading_slash(rpath), tmp, size);
				free(tmp);
			}
		}
	}
	closedir(dir);
}
