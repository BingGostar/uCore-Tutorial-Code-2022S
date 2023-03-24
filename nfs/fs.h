#ifndef __FS_H__
#define __FS_H__

#include "types.h"
// On-disk file system format.
// Both the kernel and user programs use this header file.

#define NFILE 100 // open files per system
#define NINODE 50 // maximum number of active i-nodes
#define NDEV 10 // maximum major device number
#define ROOTDEV 1 // device number of file system root disk
#define MAXOPBLOCKS 10 // max # of blocks any FS op writes
#define NBUF (MAXOPBLOCKS * 3) // size of disk block cache
#define FSSIZE 1000 // size of file system in blocks，文件系统的block数量（1000个够用了）
#define MAXPATH 128 // maximum file path name

#define ROOTINO 1 // root i-number
#define BSIZE 1024 // block size

// Disk layout:
// [ boot block | super block | inode blocks | free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock {
	uint magic; // Must be FSMAGIC
	uint size; // Size of file system image (blocks)
	uint nblocks; // Number of data blocks
	uint ninodes; // Number of inodes.
	uint inodestart; // Block number of first inode block
	uint bmapstart; // Block number of first free map block
};

// 基本信息：块大小 BSIZE = 1024B，总容量 FSSIZE = 1000 个 block = 1000 * 1024 B
// Layout: 
// 0号块留待后续拓展，可以忽略。superblock 固定为 1 号块，size 固定为一个块
// 其后是储存 inode 的若干个块，占用块数 = inode 上限 / 每个块上可以容纳的 inode 数量
// 其中 inode 上限固定为 200，每个块的容量 = BSIZE / sizeof(struct disk_inode)
// 再之后是数据块相关内容，包含一个 储存空闲块位置的 bitmap 和 实际的数据块，bitmap 块 
// 数量固定为 NBITMAP = FSSIZE / (BSIZE * 8) + 1 = 1000 / 8 + 1 = 126 块。 
// [ boot block | sb block | inode blocks | free bit map | data blocks ]

#define FSMAGIC 0x10203040

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// File type
#define T_DIR 1 // Directory
#define T_FILE 2 // File

// LAB4: Keep it the same as dinode in os/fs.h after you change it
// On-disk inode structure
struct dinode {
	short type; // File type
	short pad[3];
	uint size; // Size of file (bytes)
	uint addrs[NDIRECT + 1]; // Data block addresses
};

// Inodes per block.
#define IPB (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb) ((i) / IPB + sb.inodestart)

// Bitmap bits per block
#define BPB (BSIZE * 8)

// Block of free map containing bit for block b
#define BBLOCK(b, sb) ((b) / BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
	ushort inum;
	char name[DIRSIZ];
};

#endif //!__FS_H__
