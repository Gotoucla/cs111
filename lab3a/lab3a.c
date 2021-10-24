// NAME: Meiyi Zheng
// EMAIL: meiyizheng@g.ucla.edu
// ID: 605147145

#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include "ext2_fs.h"

static int img_fd=0;
struct ext2_super_block super;
struct ext2_group_desc group;
static unsigned int inodes_count = 0, blocks_count=0, blocks_size=0, inodes_size = 0;
static unsigned int blocks_per_group=0,inodes_per_group=0,first_ino=0;
static unsigned int free_blocks=0, free_inodes=0, block_num_free_block=0,  block_num_free_inode=0, block_num_first_ino=0;
unsigned char *block_bitmap;
unsigned char *inode_bitmap;
#define BASE_OFFSET 1024
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block-1)*blocks_size)

void superblock_sum() {
  ssize_t read_byte;
  read_byte = pread(img_fd, &super, sizeof(super), 1024);
  if (read_byte < 0) {
    fprintf(stderr, "Error occurs when read from a file\n");
    exit(2);
  }
  inodes_count = super.s_inodes_count;
  blocks_count = super.s_blocks_count;
  blocks_size = 1024 << super.s_log_block_size;
  inodes_size = super.s_inode_size;
  blocks_per_group=super.s_blocks_per_group;
  inodes_per_group=super.s_inodes_per_group;;
  first_ino=super.s_first_ino;
  fprintf(stdout,"%s,%d,%d,%d,%d,%d,%d,%d\n","SUPERBLOCK",blocks_count,inodes_count,blocks_size,inodes_size,blocks_per_group,inodes_per_group,first_ino);  
  
}

void group_sum() {
  ssize_t read_byte;
  read_byte = pread(img_fd, &group, sizeof(group), 1024+blocks_size);
  if (read_byte < 0) {
    fprintf(stderr, "Error occurs when read from a file\n");
    exit(2);
  }
  free_blocks=group.bg_free_blocks_count;
  free_inodes=group.bg_free_inodes_count;
  block_num_free_block=group.bg_block_bitmap;
  block_num_free_inode=group.bg_inode_bitmap;
  block_num_first_ino=group.bg_inode_table;
    
  fprintf(stdout,"%s,%d,%d,%d,%d,%d,%x,%x,%x\n","GROUP",0,blocks_count,inodes_count,free_blocks,free_inodes,block_num_free_block, block_num_free_inode,block_num_first_ino);
}

int is_block_used(int bno, unsigned char *bitmap){
  int index = 0;
  int offset = 0;
  if (bno==0)
    return 1;

  index = (bno-1)/8;
  offset = (bno-1)%8;
  return ((bitmap[index]&(1<<offset)));
}

void free_block() {
  block_bitmap = malloc(blocks_size);
  lseek(img_fd,BLOCK_OFFSET(group.bg_block_bitmap), SEEK_SET);
  read(img_fd, block_bitmap, blocks_size); 

  unsigned int i=0;
  for (;i<blocks_count;i++) {
    if (is_block_used((int)i,block_bitmap)==0) { // if 0, then free
     
      fprintf(stdout,"BFREE,%d\n",i);
    }
  }
  
}

void free_inode() {
  inode_bitmap = malloc(blocks_size);
  lseek(img_fd,BLOCK_OFFSET(group.bg_inode_bitmap), SEEK_SET);
  read(img_fd, inode_bitmap, blocks_size); 

  unsigned int i=0;
  for (;i<blocks_count;i++) {
    if (is_block_used(i,inode_bitmap)==0) { // if 0, then free
      fprintf(stdout,"IFREE,%d\n",i);
    }
  }
}

static void read_inode(int fd, int inode_no, const struct ext2_group_desc *group, struct ext2_inode *inode) {
  lseek(fd, BLOCK_OFFSET(group->bg_inode_table)+(inode_no-1)*sizeof(struct ext2_inode), SEEK_SET);
  read(fd, inode, sizeof(struct ext2_inode));
}

void print_gmt_time(time_t elapse_seconds)
{
  struct tm* tm = gmtime(&elapse_seconds);
  fprintf(stdout,"%02d/%02d/%02d %d:%d:%d,",(tm->tm_mon)+1,tm->tm_mday,(tm->tm_year)%100,tm->tm_hour,tm->tm_min,tm->tm_sec);
}

void directory_sum(struct ext2_inode *inode, int inode_num) {
  struct ext2_dir_entry *entry;
  unsigned int size=0;
  unsigned char block[blocks_size];
  
  lseek(img_fd, BLOCK_OFFSET(inode->i_block[0]), SEEK_SET);
  read(img_fd, block, blocks_size);                         /* read block from disk*/
                                   
  entry = (struct ext2_dir_entry *) block;           /* first entry in the directory */
  while(size < inode->i_size) {
    char file_name[EXT2_NAME_LEN+1];
    memcpy(file_name, entry->name, entry->name_len);
    file_name[entry->name_len] = 0;              /* append null char to the file name */
    if ((unsigned int)!entry->file_type) break; 
    if (entry->inode != 0) {
      //printf("%10u %s\n", entry->inode, file_name);
      fprintf(stdout, "%s,%d,%d,%d,%d,%d,'%s'\n", "DIRENT",(int)inode_num, size, (int)entry->inode,(int) entry->rec_len, (int)entry->name_len,entry->name);
    
          /* move to the next entry */
    size += entry->rec_len;
    entry = (void*) entry + entry->rec_len; 
    }
  }
}

void directory_indirect(struct ext2_inode* inode, int inode_num, int block_num, int offset) {
  unsigned int size=0;
  unsigned char block[blocks_size];
  
  ssize_t read_byte=pread(img_fd, block, blocks_size, block_num * blocks_size);
  if (read_byte < 0) {
    fprintf(stderr, "Error occurs when read from a file\n");
    exit(2);
  }
  while(size < inode->i_size) {
    struct ext2_dir_entry entry;
    memcpy(&entry, block+size, sizeof(entry));
    if ((unsigned int)!entry.file_type) break;
    if (entry.inode != 0) {
      //printf("%10u %s\n", entry->inode, file_name);
      fprintf(stdout, "%s,%d,%d,%d,%d,%d,'%s'\n", "DIRENT",(int)inode_num,offset*blocks_size+size,(int) entry.inode,(int)entry.rec_len, (int)entry.name_len,entry.name);
    }
    size += entry.rec_len;
  }
  
}


void indirect_block(struct ext2_inode *inode, int inode_num, int block_num, int level, int offset, char file_type) {
  __u32 block[blocks_size/4];
  ssize_t ret = pread(img_fd, block, blocks_size, block_num * blocks_size);
  if (ret < 0) {
    fprintf(stderr, "Error occurs when read from a file\n");
    exit(2);
  }
  __u32 i = 0;
  for (; i < blocks_size/4; i++) {
    if (block[i] != 0) {
      if (level == 1 && file_type == 'd')
	directory_indirect(inode,inode_num, block[i],offset);

      fprintf(stdout, "%s,%d,%d,%d,%d,%d\n", "INDIRECT", inode_num,level,offset,block_num,block[i]);

      if (level != 1) {
	indirect_block(inode, inode_num, block[i],level-1,offset, file_type);
	if (level == 2)
	  offset += 256;
	if (level == 3)
	  offset += 65536;
	
      }
    }
    if (level == 1) offset++;
   
  }
}

void inode_sum() {
  unsigned int i = 1;
  struct ext2_inode inode;
  for (;i<inodes_count+1;i++) {
    read_inode(img_fd,i,&group,&inode);
    char file_type='?';
    if (S_ISDIR(inode.i_mode))
      file_type = 'd';
    else if (S_ISREG(inode.i_mode))
      file_type='f';
    else if (S_ISLNK(inode.i_mode))
      file_type='s';

    time_t ctime = (time_t)inode.i_ctime;
    time_t mtime = (time_t)inode.i_mtime;
    time_t atime = (time_t)inode.i_atime;



    if (file_type != '?') {
    fprintf(stdout,"%s,%d,%c,%o,%d,%d,%d,","INODE",i,file_type,inode.i_mode&0xFFF,inode.i_uid,inode.i_gid,inode.i_links_count);
    print_gmt_time(ctime);
    print_gmt_time(mtime);
    print_gmt_time(atime);

    fprintf(stdout,"%d,%d",inode.i_size,(int)inode.i_blocks);
    }

    if (file_type == 'f' || file_type == 'd' || (file_type == 's' && (inode.i_size > 60))) {
    __u32 j = 0;
    for (; j < EXT2_N_BLOCKS; j++)
    	fprintf(stdout,",%d", inode.i_block[j]);
    }
    fprintf(stdout,"\n");
    
    if (file_type == 'd') {
      directory_sum(&inode,i);
    }

    if (file_type == 'f' || file_type == 'd') {
      indirect_block(&inode, i,inode.i_block[12],1, 12, file_type);
      indirect_block(&inode, i,inode.i_block[13],2, 268,file_type);
      indirect_block(&inode, i,inode.i_block[14],3, 268+(256*256),file_type);
    }
    
  }
}



int main(int argc, const char * argv[]) {
  if (argc != 2 ) {
    fprintf(stderr, "Invalid arguments.\n");
    fprintf(stdout, "Usage: ./lab3a file_system.img\n");
    exit(1);
  }

  img_fd = open(argv[1], O_RDONLY);
  if (img_fd < 0) {
    fprintf(stderr, "The img can't be opened\n");
    exit(1);
  }

  superblock_sum();
  group_sum();
  free_block();
  free_inode();
  inode_sum();
  free(inode_bitmap);
  free(block_bitmap);
  exit(0);
}
  

  
    
