#include<sys/stat.h>
#include<stdio.h>
#include<sys/types.h>

struct s {
    __mode_t a;
    __u_int x;
    __time_t f;
    __time_t g;
    __time_t h;
    __u_short e;
    __u_short y;
    __u_short k[15];
}aaa;

struct inode {
	mode_t mode;
    off_t size;
    __time_t atime;
    __time_t mtime;
    __time_t ctime;
    unsigned char pointer_bmap;
    unsigned short dir_pointer[12];
    unsigned short ind_pointer[2];
    unsigned short doub_ind_pointer;    
}bbb;

struct inode ccc;

int main() {
    printf("%ld\n",sizeof(mode_t)+sizeof(off_t)+sizeof(__time_t)*3
    +sizeof(unsigned short)*15+sizeof(unsigned char));
    printf("%ld\n",sizeof(bbb));
    return 0;
}
