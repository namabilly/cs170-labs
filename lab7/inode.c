#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "bitmap.h"
#include "disk_map.h"
#include "passert.h"
#include "panic.h"
#include "inode.h"

#include "crashmod.h"

#define dec() dec_counter(CAT(PREFIX, DEC_METHOD))

// Find the disk block number slot for the 'filebno'th block in inode 'ino'.
// Set '*ppdiskbno' to point to that slot.  The slot will be one of the
// ino->i_direct[] entries.
//
// Returns:
//	0 on success (but note that *ppdiskbno might equal 0).
//	-ENOENT if the function needed to allocate an indirect block, but
//		alloc was 0.
//	-ENOSPC if there's no space on the disk for an indirect block.
//	-EINVAL if filebno is out of range (it's >= N_DIRECT).
//
#undef DEC_METHOD
#define DEC_METHOD inode_block_walk
int
inode_block_walk(struct inode *ino, uint32_t filebno, uint32_t **ppdiskbno, bool alloc)
{
	uint32_t *ptr;
	if (filebno >= N_DIRECT)
		return -EINVAL;
	dec();
	ptr = &ino->i_direct[filebno];
	*ppdiskbno = ptr;
	dec();
	return 0;
}

#undef DEC_METHOD
#define DEC_METHOD inode_get_block
int
inode_get_block(struct inode *ino, uint32_t filebno, char **blk)
{
	int i;
	uint32_t *ptr;
	dec();
	if (filebno >= N_DIRECT)
		return -EINVAL;
	dec();
	for (i = 0; i < N_DIRECT; ++i) {
		if (ino->i_direct[i] == 0) {
			ino->i_direct[i] = alloc_block();
			dec();
			if (ino->i_direct[i] > 0) {
				memset(diskaddr(ino->i_direct[i]), 0, BLKSIZE);
			}
		}
	}
	if (ino->i_direct[filebno] < 0) {
		dec();
		return ino->i_direct[filebno];
	}
	dec();
	*blk = diskaddr(ino->i_direct[filebno]);
	return 0;
}

// Create "path".  On success set *pino to point at the inode and return 0.
// On error return < 0.
#undef DEC_METHOD
#define DEC_METHOD inode_create
int
inode_create(const char *path, struct inode **pino)
{
	char name[NAME_MAX];
	int r, i;
	struct inode *dir;
	struct dirent *d;

	if ((r = walk_path(path, &dir, NULL, NULL, name)) == 0)
		return -EEXIST;
	if (r != -ENOENT || dir == 0)
		return r;
	if ((r = dir_alloc_dirent(dir, &d)) < 0)
		return r;
	dec();
	if ((r = alloc_block()) < 0)
		return r;
	dec();
	memset(diskaddr(r), 0, BLKSIZE);
	dec();
	strcpy(d->d_name, name);
	dec();
	d->d_inum = r;
	dec();
	*pino = diskaddr(d->d_inum);
	dec();
	inode_flush(dir);
	dec();

	return 0;
}

// Open "path".  On success set *pino to point at the inode and return 0.
// On error return < 0.
#undef DEC_METHOD
#define DEC_METHOD inode_open
int
inode_open(const char *path, struct inode **pino)
{
	dec();
	return walk_path(path, 0, pino, 0, 0);
}

// Read count bytes from ino into buf, starting from seek position
// offset.  This meant to mimic the standard pread function.
// Returns the number of bytes read, < 0 on error.
#undef DEC_METHOD
#define DEC_METHOD inode_read
ssize_t
inode_read(struct inode *ino, void *buf, size_t count, uint32_t offset)
{
	int r, bn;
	uint32_t pos;
	uint32_t *pblkno;
	char *blk;

	if (offset >= ino->i_size)
		return 0;

	count = MIN(count, ino->i_size - offset);
	dec();

	for (pos = offset; pos < offset + count; ) {
		if ((r = inode_block_walk(ino, pos / BLKSIZE, &pblkno, 0)) < 0)
			switch (-r) {
				case ENOENT: // For sparse files.
					pblkno = NULL;
					break;
				default:
					return r;
			}
		bn = MIN(BLKSIZE - pos % BLKSIZE, offset + count - pos);
		dec();
		// Handle sparse files.  If no block has been allocated for
		// this region of the file, fill the read buffer with zeroes.
		if (pblkno == NULL || *pblkno == 0) {
			if (buf)
				memset(buf, 0, bn);
		} else {
			dec();
			blk = diskaddr(*pblkno);
			if (buf)
				memmove(buf, blk + pos % BLKSIZE, bn);
		}
		pos += bn;
		if (buf)
			buf += bn;
		dec();
	}

	return count;
}

// Write count bytes from buf into ino, starting at seek position
// offset.  This is meant to mimic the standard pwrite function.
// Extends the file if necessary.
// Returns the number of bytes written, < 0 on error.
#undef DEC_METHOD
#define DEC_METHOD inode_write
int
inode_write(struct inode *ino, const void *buf, size_t count, uint32_t offset)
{
	int r, bn;
	uint32_t pos;
	char *blk;

	// Extend file if necessary
	if (offset + count > ino->i_size)
		if ((r = inode_set_size(ino, offset + count)) < 0)
			return r;
	dec();

	for (pos = offset; pos < offset + count; ) {
		if ((r = inode_get_block(ino, pos / BLKSIZE, &blk)) < 0)
			return r;
		bn = MIN(BLKSIZE - pos % BLKSIZE, offset + count - pos);
		dec();
		memmove(blk + pos % BLKSIZE, buf, bn);
		dec();
		pos += bn;
		buf += bn;
	}
	dec();

	return count;
}

// Remove a block from inode ino.  If it's not there, just silently succeed.
// Returns 0 on success, < 0 on error.
#undef DEC_METHOD
#define DEC_METHOD inode_free_block
static int
inode_free_block(struct inode *ino, uint32_t filebno)
{
	int r;
	uint32_t *ptr;

	if ((r = inode_block_walk(ino, filebno, &ptr, 0)) < 0)
		switch (-r) {
			// Ignore not found error for sparse files.
			case ENOENT:
				return 0;
			default:
				return r;
		}
	dec();
	if (*ptr) {
		free_block(*ptr);
		dec();
		*ptr = 0;
	}
	return 0;
}

// Remove any blocks currently allocated for inode "ino" that would
// not be needed for an inode of size "newsize" (where newsize is smaller
// than ino->i_size).  Do not change ino->i_size.
//
// For both the old and new sizes, compute the number of blocks required,
// and then free the blocks from new_nblocks to old_nblocks.  If new_nblocks
// is no more than NDIRECT and the indirect block has been allocated, then
// free the indirect block.  Do the same for the double-indirect block if
// new_nblocks is no more than NDIRECT + NINDIRECT.  Don't forget to free
// the indirect blocks allocated in the double-indirect block!
//
// Hint: Use inode_free_block to free all the data blocks, then use
// free_block to free the meta-data blocks (e.g. the indirect block).
#undef DEC_METHOD
#define DEC_METHOD inode_truncate_blocks
static void
inode_truncate_blocks(struct inode *ino, uint32_t newsize)
{
	int r;
	uint32_t bno, old_nblocks, new_nblocks;

	if (! newsize) {
		for (r = 0; r < N_DIRECT; ++r) {
			inode_free_block(ino, r);
		}
	}
}

// Set the size of inode ino, truncating or extending as necessary.
#undef DEC_METHOD
#define DEC_METHOD inode_set_size
int
inode_set_size(struct inode *ino, uint32_t newsize)
{
	if (ino->i_size > newsize)
		inode_truncate_blocks(ino, newsize);
	dec();
	ino->i_size = newsize;
	dec();
	flush_block(ino);
	dec();
	return 0;
}

// Flush the contents and metadata of inode ino out to disk.  Loop over
// all the blocks in ino.  Translate the inode block number into a disk
// block number and then check whether that disk block is dirty.  If so,
// write it out.
#undef DEC_METHOD
#define DEC_METHOD inode_flush
void
inode_flush(struct inode *ino)
{
	int i;
	uint32_t *pdiskbno;

	for (i = 0; i < (ino->i_size + BLKSIZE - 1) / BLKSIZE; i++) {
		if (inode_block_walk(ino, i, &pdiskbno, 0) < 0 ||
				pdiskbno == NULL || *pdiskbno == 0)
			continue;
		dec();
		flush_block(diskaddr(*pdiskbno));
	}
	dec();
	flush_block(ino);
	dec();
}

// Free disk resources reserved for an inode.  This should only be
// called in inode_unlink when an inode's link count hits 0.  Note
// that a block number (inum), and not a struct inode, is required as
// an argument to this function, as the block containing the inode
// must be freed as well.
#undef DEC_METHOD
#define DEC_METHOD inode_free
static void
inode_free(uint32_t inum)
{
	struct inode *ino;

	ino = diskaddr(inum);
	dec();
	assert(ino->i_nlink == 0);
	dec();

	inode_truncate_blocks(ino, 0);
	dec();
	flush_block(ino);
	dec();
	free_block(inum);
	dec();
}

// Unlink an inode by decrementing its link count and zeroing the name
// and inum fields in its associated struct dirent.  If the link count
// of the inode reaches 0, free the inode.
//
// Returns 0 on success, or -ENOENT if the file to be unlinked does
// not exist.
//
// Hint: Use walk_path and inode_free.  You will need to take advantage
// of walk_path setting the pent parameter to point to the directory
// entry associated with the file to be unlinked.
#undef DEC_METHOD
#define DEC_METHOD inode_unlink
int
inode_unlink(const char *path)
{
	int i, j;
	struct inode *ino;
	struct dirent *d;
	if (*path == '\0') {return -EINVAL;}
	dec();
	path = path + 1;
	ino = diskaddr(super->s_root);
	for (i = 0; i < N_DIRECT; ++i) {
		dec();
		if (! ino->i_direct[i]) {continue;}
		d = diskaddr(ino->i_direct[i]);
		dec();
		for (j = 0; j < BLKDIRENTS; ++j) {
			if (! strcmp(d[j].d_name, path)) {
				if (! --((struct
								inode*)diskaddr(d[j].d_inum))->i_nlink)
				{
					inode_free(d[j].d_inum);
				}
				d[j].d_inum = 0;
				d[j].d_name[0] = '\0';
				return 0;
			}
			dec();
		}
	}
	return -ENOENT;
}

// Link the inode at the location srcpath to the new location dstpath.
// Increment the link count on the inode.
//
// Returns 0 on success, < 0 on failure.  In particular, the function
// should fail with -EEXIST if a file exists already at dstpath.
//
// Hint: Use walk_path and dir_alloc_dirent.
#undef DEC_METHOD
#define DEC_METHOD inode_link
int
inode_link(const char *srcpath, const char *dstpath)
{
	int i, j, k;
	struct inode *ino;
	struct dirent *d;
	k = 0;
	if (*srcpath == '\0' || *dstpath == '\0') {return -EINVAL;}
	dec();
	srcpath = srcpath + 1;
	dstpath = dstpath + 1;
	ino = diskaddr(super->s_root);
	for (i = 0; i < N_DIRECT; ++i) {
		dec();
		if (! ino->i_direct[i]) {continue;}
		d = diskaddr(ino->i_direct[i]);
		dec();
		for (j = 0; j < BLKDIRENTS; ++j) {
			if (! strcmp(d[j].d_name, dstpath)) {
				dec();
				return -EEXIST;
			}
			if (! strcmp(d[j].d_name, srcpath)) {
				k = d[j].d_inum;
			}
			dec();
		}
	}
	if (!k) return -ENOENT;
	for (i = 0; i < N_DIRECT; ++i) {
		dec();
		if (! ino->i_direct[i]) {continue;}
		d = diskaddr(ino->i_direct[i]);
		dec();
		for (j = 0; j < BLKDIRENTS; ++j) {
			if (! d[j].d_inum) {
				d[j].d_inum = k;
				dec();
				strcpy(d[j].d_name, dstpath);
				((struct inode*)diskaddr(k))->i_nlink++;
				dec();
				return 0;
			}
		}
	}

	return -ENOMEM;
}

// Return information about the specified inode.
#undef DEC_METHOD
#define DEC_METHOD inode_stat
int
inode_stat(struct inode *ino, struct stat *stbuf)
{
	uint32_t i, nblocks, *pdiskbno;

	stbuf->st_mode = ino->i_mode;
	stbuf->st_size = ino->i_size;
	stbuf->st_blksize = BLKSIZE;
	for (i = 0, nblocks = 0; i < ROUNDUP(ino->i_size, BLKSIZE); i++) {
		if (inode_block_walk(ino, i, &pdiskbno, 0) < 0)
			continue;
		if (*pdiskbno != 0)
			nblocks++;
	}
	stbuf->st_blocks = nblocks * (BLKSIZE / 512); // st_blocks unit is 512B.
	stbuf->st_nlink = ino->i_nlink;
	stbuf->st_mtime = ino->i_mtime;
	stbuf->st_atime = ino->i_atime;
	stbuf->st_ctime = ino->i_ctime;
	stbuf->st_uid = ino->i_owner;
	stbuf->st_gid = ino->i_group;
	stbuf->st_rdev = ino->i_rdev;

	return 0;
}
#undef DEC_METHOD
