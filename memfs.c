#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>

struct inode {
	char *name;
	char content[512];
	mode_t st_mode;
	nlink_t st_nlink;
	size_t st_size;
	bool root_child;
	bool is_folder;
	struct inode* parent;
	struct inode* child;
	struct inode* next;
	struct inode* prev;
} inode;

struct inode root;

char* DELIMITER = "/";

struct inode* path_level_evaluation(char* path, struct inode* node){
	struct inode* elm = node;
	do{
		if((strcmp(path, elm->name) == 0))
			return elm;
		elm = elm->next;
	}while(elm!=NULL);
	return NULL;
}

char* name_evaluation(const char* path){
	char path_eval[strlen(path)+1];
	char *ptr, *res;
	ptr = (char*)malloc(sizeof(char) * 255);
	res = (char*)malloc(sizeof(char) * 255);
	strcpy(path_eval, path);
	ptr = strtok(path_eval, DELIMITER);
	
	while(ptr!=NULL){
		strcpy(res, ptr);
		ptr = strtok(NULL, DELIMITER);
	}
	free(ptr);
	return res;
}


struct inode* path_evaluation(const char* path){
	char path_eval[strlen(path)+1];
	char* ptr;
	strcpy(path_eval, path);
	if (strcmp(path, "/") == 0){
			return &root;
	}
		
	else{
		struct inode* res;
		ptr = strtok(path_eval, DELIMITER);
		if(root.child==NULL){
			return NULL;

		}
		res = &root;
		do{
			if(res->child == NULL)
				return NULL;
			res = path_level_evaluation(ptr,res->child);
			if(res==NULL)
				return NULL;
			ptr = strtok(NULL, DELIMITER);
		}while(ptr != NULL);
		return res;
	}
}

static int _mkdir(const char *path, mode_t mode)
{
	struct inode* elm, *last_sibling = NULL;

	char* name, * mount_path;
	int first_child = 0;

	struct inode* newdir = (struct inode*)malloc(sizeof(struct inode));

	name = name_evaluation(path);

	if(strlen(name)> 255)
		return -1;

	mount_path = (char*)malloc(sizeof(char) * (strlen(path)+1));
	strcpy(mount_path,path);
	mount_path[strlen(path) - strlen(name)] = '\0';	
	elm = path_evaluation(mount_path);


	if(elm==NULL)
		return -ENOENT;

	if(elm->child!=NULL){
		last_sibling = elm->child;
		while(last_sibling->next!=NULL)
			last_sibling = last_sibling->next;
	}
	else 
		first_child = 1;
	
	

	newdir->name = name;
	newdir->st_mode = mode | S_IFDIR;
	newdir->st_nlink = 2;
	newdir->prev = last_sibling;
	newdir->next = NULL;
	newdir->root_child = first_child;
	newdir->parent = elm;
	newdir->child = NULL;
	newdir->is_folder = true;

	if(!first_child)
		last_sibling->next = newdir;
	else
		elm->child = newdir;
	elm->st_nlink++;
	
	free(mount_path);
	return 0;
}

static int _getattr( const char *path, struct stat *st )
{
	struct inode* file;
	file = path_evaluation(path);

	if(file != NULL)
	{
		st->st_uid = getuid();
		st->st_gid = getgid(); 
		st->st_atime = time( NULL );
		st->st_mtime = time( NULL ); 
		st->st_mode =  file->st_mode;
		st->st_nlink = file->st_nlink;
		if(file->is_folder != true)
			st->st_size = file->st_size;
	}
	else 
		return -ENOENT;
	
	return 0;
}


static int _readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	struct inode* file;

	file = path_evaluation(path);
	if(file != NULL)
	{
		filler( buffer, ".", NULL, 0 ); // Current Directory
		filler( buffer, "..", NULL, 0 ); // Parent Directory
		struct inode aux;
		if(file->child != NULL){
			aux = *(file->child);
			while(1){
				filler( buffer, aux.name, NULL, 0 );
				if(aux.next == NULL)
					break;
				aux = *(aux.next);
			}
		}
	}
	else 
		return -ENOENT;
	
	return 0;
}

static int _read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	struct inode* elm;
	elm = path_evaluation(path);
	if(elm==NULL)
		return -ENOENT;

    memcpy(buffer, elm->content + offset, size );
		
	return strlen( elm->content ) - offset;
}

static int _create(const char *path, mode_t mode,
		      struct fuse_file_info *fi)
{
	struct inode* elm, *last_sibling = NULL;

	int first_child=0;

	char* name, * mount_path;

	struct inode* newdir = (struct inode*)malloc(sizeof(struct inode));

	name = name_evaluation(path);

	if(strlen(name)> 255)
		return -1;

	mount_path = (char*)malloc(sizeof(char) * (strlen(path)+1));
	strcpy(mount_path,path);
	mount_path[strlen(path) - strlen(name)] = '\0';

	
	elm = path_evaluation(mount_path);
	if(elm==NULL)
		return -ENOENT;

	if(elm->child!=NULL){
		last_sibling = elm->child;
		while(last_sibling->next!=NULL)
			last_sibling = last_sibling->next;
	}
	else 
		first_child=1;


	newdir->name = name;
	newdir->st_mode = mode | S_IFREG;
	newdir->st_nlink = 1;
	newdir->st_size = 0;
	newdir->prev = last_sibling;
	newdir->next = NULL;
	newdir->root_child = first_child;
	newdir->parent = elm;
	newdir->child = NULL;
	newdir->is_folder = false;


	if(!first_child)
		last_sibling->next = newdir;
	else
		elm->child = newdir;
	elm->st_nlink++;

	free(mount_path);
	return 0;
}

static int _write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
	struct inode* elm;
	elm = path_evaluation(path);
	if(elm==NULL)
		return -ENOENT;

	memcpy(elm->content+offset, buf, size);
	elm->st_size = strlen(elm->content);
	return size;
}


static int _open(const char *path, struct fuse_file_info *fi)
{
	int fd, ret=0;

	struct inode* file;
	file = path_evaluation(path);
	if(file == NULL || file->is_folder == true)
		return -ENOENT;

	fd = memfd_create( file->name, 0 );
	if (fd == -1)
		return -errno;

	int content_size = strlen(file->content) == 0 ? 0 : strlen(file->content)-1;
	write( fd, file->content, sizeof(char) * content_size);
	lseek( fd, 0, SEEK_SET );

	fi->fh = fd;
	return 0;
}


static struct fuse_operations operations = {
    .getattr	= _getattr,
    .readdir	= _readdir,
    .read	= _read,
	.mkdir = _mkdir,
	.write = _write,
	.open = _open,
	.create = _create,

};

int main( int argc, char *argv[] )
{

	root.name = strdup("/");
	root.st_mode =  S_IFDIR | 0755; 
	root.st_nlink = 2;
	root.prev = NULL;
	root.next = NULL;
	root.root_child = true;
	root.parent = NULL;
	root.child = NULL;
	root.is_folder = true;
	
	return fuse_main( argc, argv, &operations, NULL );
}