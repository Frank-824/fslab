# fs lab
May 2019. ICS2's lab 

在此讨论，写下想法，记录进度。
最新的事项写在issues中。

## Issues
---
1
块的规格数值大致算好了
计算中有一处失误，就是$\alpha$的值，只计算了该填的stat的属性（见ppt和根目录下的test.c），未计算inode中应有的pointer

2
Inode的结构基本给出，但是大小需要控制
在Inode Structure部分给出
改进的建议：
inode_block 和 block_pointer可以用偏移地址来计算，在superblock中存放inode blocks和data blocks的起始地址（data block可以存分段地址），则Inode中的指针即相当于偏移量，这样可以减小Inode大小
经过分析，如下inode structure，short可以用于表示偏移，则大大减小了inode的字节大小


## Block Analysis
---
> 希望ljt来做可视化的说明
> 
> ljt:好的


### 分析

```
#define BLOCK_SIZE 4096
#define BLOCK_NUM 65536
#define DISK_SIZE (BLOCK_SIZE*BLOCK_NUM)
```
相应需求：
* 至少250MB的真实可用空间
* 至少支持32768个文件及目录
* 只需支持单个文件最大8MB，如果有能力可以支持更大的单文件大小（存在额外测试点）。
* 只需支持文件名最大长度为24字符，且同一目录下不存在名称相同的文件或目录。


$$have: \\block\ id\ 65536 = 64 * 1024 = 64K\ storage\ size\\block\ size\ 4096 B = 4 KB\ storage\ size \\total\ size = 256 MB \\need:\\250M \ or \ 32768 * 8 \ MB = 256 \ MB \ blocks\ total\ size\\manage\ block: 0 - 6MB \\manage\ block: super\ block+map\ block+inode\ block \\super\ block = 1\ blk \\free\ block= \frac{252M}{4K} = 63K \\free\ block\ bitmap =  \frac{63K}{8*4K} = 2\ blk\\inode\ bitmap = \frac{32768}{8*4K} = 1\ blk\ inode\ block = 32768 *\alpha\ B = 32*\alpha\ KB = 8*\alpha\ blk\ ,\  \alpha = 72 (wrong,didn't\ consider\ pointers) \\manage\ blk = 72*8+1+2+1 = 580\\manage\ blk\ size = 580*4K \approx 2.3M$$

```
File System Map(similar to VSFS):
Super block \ inode map \ free data block map \
 inode block \ data block
 (i = sizeof(inode)) = 72
+-------+--------+--------+-----------+------------------+
| S:1db | is:1db | ds:2db | inode:8*i |   data blocks    |
+-------+--------+--------+-----------+------------------+
Total Size: 256MB
Manage Size: 2.3MB
Data Size: 250MB
Other Size: 3.7MB
```



### 实现相关
函数原型：int fs_statfs (const char *path, struct statvfs *stat);
函数功能：查询文件系统整体的统计信息。

```
struct statvfs {
   unsigned long  f_bsize; //块大小
   fsblkcnt_t     f_blocks;//块数量
   fsblkcnt_t     f_bfree; //空闲块数量
   fsblkcnt_t     f_bavail;//可用块数量
   fsfilcnt_t     f_files; //文件节点数
   fsfilcnt_t     f_ffree; //空闲节点数
   fsfilcnt_t     f_favail;//可用节点数
   unsigned long  f_namemax;//文件名长度上限
};
```


## Inode Structure
---
### 问题分析
需要支持文件最大为8MB，即为
$$\frac{8MB}{4KB}=2K\ blk$$

方案1
如果利用8B的指针来表示data block的地址，则一个data block中能够存放的data block指针数量为
$$\frac{4KB}{8B}=512个$$
则用indirect pointer来存放所有指针，至少需要4个indirect pointer

方案2
如果用偏移地址表示block pinter的地址：
$$addr\ bit = \log_{2}{total\ block\ number} = log_2{64k}=16$$
则用一个short即可表示所有block的偏移地址，只需要在super block中记录data block的地址即可（可以做分段以提高性能）
（inode也可以类似）

### 结构设计
假设：大部分文件都是小文件，大多为几十到几百KB，大文件为少数，故我们只用在inode设置少量indirect pointer或 double indirect pointer即可支持大文件。

不妨暂定为:
(基于实现的具体功能来设计)
```
Struct Inode{
    mode (read/write/executed)
    size
    atime,ctime,mtime
    inode_block
    blocks_cnt
    blocks_pointer{
        direct pointer: 12x
        indirect pointer: 2x
        double indirect pointer: 1x
    }
}



//--------- size of inode ---------
// #1
// pointer size = 8B
// pointers size:  (d_ptr + id_ptr + dib_ptr)x8 
// total size = mode + size + atime + ctime + mtime +  
//      (links_cnt) + inode_block + blocks_cnt + blocks_pointer
//       = char + size_t + time_t*3 + u_int*2 +  
         (int) + inode_block + int + blocks_pointer
         =?
// #2
// ptr = short
// total size = mode_t + u_int + time_t*3 + u_short*2 + u_short*15
// total size = 72

//deleted: links_cnt可以略去？ppt中说等于1即可
//deleted: uid=getuid(), gid=getgid()

//usage in functions:
//atime(read/readdir/mkdir) mtime(write/mkdir) ctime(write/mkdir)
//mode(open):判断用户是否有权限读/

```

