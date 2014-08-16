#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <time.h>
#include "ifs.h"

static void process_directory(char* dir, char* rdir);

int main(int argc, char* argv[])
{

	void* img = (void*)malloc(5000000);
	create_image(img, 5000000);
	char* outp = "initrd.img";
	char* tdir = NULL;
	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-d") == 0)
		{
			tdir = argv[++i];
		}
		else if (strcmp(argv[i], "-o") == 0)
		{
			outp = argv[++i];
		}
	}
	FILE* dmp = fopen(outp,"w");

	process_directory(tdir, "");
	fwrite(img,1, 5000000,dmp);

	fclose(dmp);
	return 0;
}

static void process_directory(char* path, char* rel_path)
{
	struct dirent *ent;
	DIR* dir = opendir (path);
	char fpath[1000];
	char rpath[1000];
	while ((ent = readdir (dir)) != NULL)
	{
		if(strcmp(&ent->d_name, ".") && strcmp(&ent->d_name, "..") )
		{
			sprintf(fpath, "%s/%s", path, ent->d_name);
			sprintf(rpath, "%s/%s", rel_path, ent->d_name);
			
			if(ent->d_type == DT_DIR)
			{
				make_dir(rpath);
				process_directory(fpath, rpath);
			}
			else if (ent->d_type == DT_REG)
			{
				FILE* f = fopen(fpath, "r");
				int size = 0;
				fseek(f, 0L, SEEK_END);
				size = ftell(f);
				fseek(f, 0L, SEEK_SET);
				char* tmp = (char*)malloc(size);
				fread(tmp, 1, size, f);
				fclose(f);
				add_file(rpath, tmp, size);
				free(tmp);
			}
		}


	}
	closedir (dir);

}
