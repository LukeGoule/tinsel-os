#include <ext2.h>
#include <stdio.h>
#include <ata.h>
#include <memory.h>

#define dev ata_get_main_dev()

inode_t *inode = 0;
uint8_t *root_buf = 0;
uint8_t *block_buf = 0;
ext2_priv_data* _priv = 0;

size_t str_backspace(char* str, char c)
{
	size_t i = std::strlen((const char*)str);
	i--;
	while(i)
	{
		i--;
		if(str[i] == c)
		{
			str[i+1] = 0;
			return 1;
		}
	}
	return 0;
}

void ext2_read_block(uint8_t *buf, uint32_t block, ext2_priv_data *priv)
{
	uint32_t sectors_per_block = priv->sectors_per_block;
	if(!sectors_per_block) sectors_per_block = 1;
	/*mprint("we want to read block %d which is sectors [%d; %d]\n",
		block, block*sectors_per_block , block*sectors_per_block + sectors_per_block);*/
	//kprintf("  %d", block);
	ata_read(buf, block*sectors_per_block, sectors_per_block, dev);
}

void ext2_write_block(uint8_t *buf, uint32_t block, ext2_priv_data *priv)
{
	uint32_t sectors_per_block = priv->sectors_per_block;
	if(!sectors_per_block) sectors_per_block = 1;
	ata_write(buf, block*sectors_per_block, sectors_per_block, dev);
}

void ext2_alloc_block(uint32_t *out, ext2_priv_data *priv)
{
	/* Algorithm: Loop through block group descriptors,
	 * find which bg has a free block
	 * and set that.
	 */
	 ext2_read_block(root_buf, priv->first_bgd, priv);
	 block_group_desc_t *bg = (block_group_desc_t *)root_buf;
	 for(int i = 0; i < priv->number_of_bgs; i++)
	 {
	 	if(bg->num_of_unalloc_block)
	 	{
	 		*out = priv->sb.blocks - bg->num_of_unalloc_block + 1;
	 		bg->num_of_unalloc_block --;
	 		ext2_write_block(root_buf, priv->first_bgd + i, priv);

	 		std::printf("[ext2] Allocated block %d\n",*out);

	 		ext2_read_block(root_buf, priv->sb.superblock_id, priv);
			superblock_t *sb = (superblock_t *)root_buf;
			sb->unallocatedblocks --;
			ext2_write_block(root_buf, priv->sb.superblock_id, priv);
			return;
	 	}
	 	bg++;
	 }
}

void ext2_find_new_inode_id(uint32_t *id, ext2_priv_data *priv)
{
	/* Algorithm: Loop through the block group descriptors,
	 * and find the number of unalloc inodes
	 */

	/* Loop through the block groups */
	ext2_read_block(root_buf, priv->first_bgd, priv);
	block_group_desc_t *bg = (block_group_desc_t *)root_buf;
	for(int i = 0; i < priv->number_of_bgs; i++)
	{
		if(bg->num_of_unalloc_inode)
		{
			/* If the bg has some unallocated inodes,
			 * find which inode is unallocated
			 * This is easy:
			 * For each bg we have sb->inodes_in_blockgroup inodes,
			 * this one has num_of_unalloc_inode inodes unallocated,
			 * therefore the latest id is:
			 */
			 *id = ((i + 1) * priv->sb.inodes_in_blockgroup) - bg->num_of_unalloc_inode + 1;
			 bg->num_of_unalloc_inode --;
			 ext2_write_block(root_buf, priv->first_bgd + i, priv);
			 /* Now, update the superblock as well */
			 ext2_read_block(root_buf, priv->sb.superblock_id, priv);
			 superblock_t *sb = (superblock_t *)root_buf;
			 sb->unallocatedinodes --;
			 ext2_write_block(root_buf, priv->sb.superblock_id, priv);
			 return;
		}
		bg++;
	}
}

uint8_t ext2_writefile(char *fn, char *buf, uint32_t len, ext2_priv_data *priv)
{
	/* Steps to write to a file:
	 * - Locate and load the inode
	 * - Check if it is of type INODE_TYPE_FILE
	 * -- If no, bail out.
	 * - If inode->size == 0
	 * -- Allocate len / priv->blocksize amount of blocks.
	 * --- Write the buf to the blocks.
	 * - Else, check which block has the last byte, by
	 *   dividing inode->size by priv->blocksize.
	 * -- Load that block.
	 * -- Inside, the last byte is (inode->size)%priv->blocksize
	 * -- If len < priv->blocksize - (inode->size)%priv->blocksize
	 *    which means that the buf can fill the block.
	 * --- Write and return noerror.
	 * -- Else,
	 * --- Write the maximum possible bytes to the block.
	 * --- The next block doesn't exist. Allocate a new one.
	 * --- Write the rest to that block and repeat.
	 * ALSO, on write: adjust inode->size !!!
	 *
	 */

	/* Locate and load the inode */
	uint32_t inode_id = ext2_find_file_inode(fn, inode, priv);
	inode_id ++;
	if(inode_id == 1) return 0;
	std::printf("[ext2] %s's inode is %d\n", fn, inode_id);
	if(!inode) inode = (inode_t *)kmalloc(sizeof(inode_t));
	ext2_read_inode(inode, inode_id, priv);
	/* Check if it is of type INODE_TYPE_FILE */
	/*if(! (inode->type & INODE_TYPE_FILE))
	{
		/* -- If no, bail out.
		kprintf("Not a file.\n");
		return 0;
	}*/
	/* If inode->size == 0 */
	if(inode->size == 0)
	{
		/* Allocate len / priv->blocksize amount of blocks. */
		uint32_t blocks_to_alloc = len / priv->blocksize;
		blocks_to_alloc ++; /* Allocate atleast one! */
		if(blocks_to_alloc > 12)
		{
			/* @todo */
			std::printf("[ext2] Sorry, can't write to files bigger than 12Kb :(\n");
			return 0;
		}
		for(int i = 0; i < blocks_to_alloc; i++)
		{
			uint32_t bid = 0;
			ext2_alloc_block(&bid, priv);
			inode->dbp[i] = bid;
			//kprintf("Set dbp[%d] to %d\n", i, inode->dbp[i]);
		}
		std::printf("[ext2] Allocated %d blocks!\n", blocks_to_alloc);
		inode->size += len; // UPDATE the size
		/* Commit the inode to the disk */
		ext2_write_inode(inode, inode_id - 1, priv);
		/* Write the buf to the blocks. */
		for(int i = 0; i < blocks_to_alloc; i++)
		{
			/* We loop through the blocks and write. */
			ext2_read_block(root_buf, inode->dbp[i], priv);
			if(i + 1 < blocks_to_alloc) { // If not last block
				std::memcpy(root_buf, buf + i*priv->blocksize, priv->blocksize);
			} else {// If last block
				std::printf("[ext2] Last block write %d => %d!\n", i, inode->dbp[i]);
				std::memcpy(root_buf, buf + i*priv->blocksize, len);
			}
			ext2_write_block(root_buf, inode->dbp[i], priv);
		}
		std::printf("[ext2] Wrote the data to fresh blocks!\n");
		return 1;
	}
	/* Else, check which block has the last byte, by
	 *   dividing inode->size by priv->blocksize.
	 */
	uint32_t last_data_block = inode->size / priv->blocksize;
	uint32_t last_data_offset = (inode->size) % priv->blocksize;
	/* Load that block. */
	ext2_read_block(root_buf, last_data_block, priv);
	/* If len < priv->blocksize - (inode->size)%priv->blocksize
	 */
	if(len < priv->blocksize - last_data_offset)
	{
		/*    which means that the buf can fill the block. */
		/* Write and return noerror.*/
		std::memcpy(root_buf + last_data_offset, buf, len);
		ext2_write_block(root_buf, last_data_block, priv);
		return 1;
	}
	/*Else,
	 * --- Write the maximum possible bytes to the block.
	 * --- The next block doesn't exist. Allocate a new one.
	 * --- Write the rest to that block and repeat.
	 */
	/*uint32_t data_wrote = 0;
	memcpy(root_buf + last_data_offset, buf, priv->blocksize - last_data_offset);
	data_wrote += priv->blocksize - last_data_offset;*/

 	return 0;
}

uint8_t ext2_touch(char *file, ext2_priv_data *priv)
{
	/* file = "/levex.txt"; */
	/* First create the inode */
	char *fil = (char *)kmalloc(std::strlen((const char*)file) + 1);
	std::memcpy(fil, file, std::strlen((const char*)file) + 1);
	inode_t *fi = (inode_t *)kmalloc(sizeof(inode_t));
	fi->hardlinks = 1;
	fi->size = 0;
	fi->type = INODE_TYPE_FILE;
	fi->disk_sectors = 2;
	/* Create the directory entry */
	size_t n = std::strexplode(fil, '/') - 1;
	n--;
	while(n)
	{
		fil += std::strlen((const char*)fil) + 1;
		n--;
	}
	std::printf("[ext2] filename: %s\n", fil);
	ext2_dir *entry = (ext2_dir *)kmalloc(sizeof(ext2_dir) + std::strlen((const char*)fil) + 1);
	entry->size = sizeof(ext2_dir) + std::strlen((const char*)fil) + 1;
	entry->namelength = std::strlen((const char*)fil) + 1;
	entry->reserved = 0;
	std::memcpy(&entry->reserved + 1, fil, std::strlen((const char*)fil) + 1);
	std::printf("[ext2] Length of dir entry: %d + %d = %d\n", sizeof(ext2_dir), std::strlen((const char*)fil) + 1, entry->size);
	/* Now, calculate this inode's id,
	 * this is done from the superblock's inodes field
	 * don't forget to update the superblock as well.
	 */
	uint32_t id = 0;
	ext2_find_new_inode_id(&id, priv);
	std::printf("[ext2] Inode id = %d\n", id);
	entry->inode = id;
	ext2_read_directory(0, entry, priv);
	/* Find where the previous inode is
	 * and put this inode after this
	 */
	uint32_t block = 0; /* The block where this inode should be written */
	uint32_t ioff = 0; /* Offset into the block function to sizeof(inode_t) */
	ext2_get_inode_block(id, &block, &ioff, priv);
	std::printf("[ext2] This inode is located on block %d with ioff %d\n", block, ioff);
	/* First, read the block in */
	ext2_read_block(root_buf, block, priv);
	inode_t * winode = (inode_t *)root_buf;
	for(int i = 0;i < ioff; i++)
		winode++;
	std::memcpy(winode, fi, sizeof(inode_t));
	ext2_write_block(root_buf, block, priv);
	/* Now, we have added the inode, write the superblock as well. */
	ext2_write_block((uint8_t*)&priv->sb, priv->sb.superblock_id, priv);
	/* Now, add the directory entry,
	 * for this we have to locate the directory that holds us,
	 * and find his inode.
	 * We call ext2_find_file_inode() to place the inode to inode_buf
	 */
	char *f = (char *)kmalloc(std::strlen((const char*)file) + 1);
	std::memcpy(f, file, std::strlen((const char*)file) + 1);
	str_backspace(f, '/');

	std::printf("[ext2] LF: %s\n", f);
	if(!inode) inode = (inode_t *)kmalloc(sizeof(inode_t));
	if(!block_buf) block_buf = (uint8_t *)kmalloc(priv->blocksize);
	uint32_t t = ext2_find_file_inode(f, inode, priv);
	t++;
	std::printf("[ext2] Parent is inode %d\n", t);
	uint8_t found = 0;
	for(int i = 0; i < 12; i++)
	{
		/* Loop through the dpb to find an empty spot */
		if(inode->dbp[i] == 0)
		{
			/* This means we have not yet found a place for our entry,
			 * and the inode has no block allocated.
			 * Allocate a new block for this inode and place it there.
			 */
			uint32_t theblock = 0;
			ext2_alloc_block(&theblock, priv);
			inode->dbp[i] = theblock;
			ext2_write_inode(inode, t, priv);
 		}
		/* This DBP points to an array of directory entries */
		ext2_read_block(block_buf, inode->dbp[i], priv);
		/* Loop throught the directory entries */
		ext2_dir *d = (ext2_dir *)block_buf;
		uint32_t passed = 0;
		while(d->inode != 0) {
			if(d->size == 0) break;
			uint32_t truesize = d->namelength + 8;
			std::printf("[ext2] truesize has modulo 4 of %d, adding %d\n", truesize % 4, 4 - truesize%4);
			truesize += 4 - truesize % 4;
			uint32_t origsize = d->size;
			std::printf("[ext2] truesize: %d origsize: %d\n", truesize, origsize);
			if(truesize != d->size)
			{
				/* This is the last entry. Adjust the size to make space for our
				 * ext2_dir! Also, note that according to ext2-doc, entries must be on
				 * 4 byte boundaries!
				 */
				d->size = truesize;
				std::printf("[ext2] Adjusted entry len:%d, name %s!\n", d->size, &d->reserved + 1);
				/* Now, skip to the next */
				passed += d->size;
				d = (ext2_dir *)((uint32_t)d + d->size);
				/* Adjust size */
				entry->size = priv->blocksize - passed;
				std::printf("Entry size is now %d\n", entry->size);
				break;
			}
			std::printf("[ext2] skipped len: %d, name:%s!\n", d->size, &d->reserved + 1);
			passed += d->size;
			d = (ext2_dir *)((uint32_t)d + d->size);
		}
		/* There is a problem, however. The last entry will always span the whole
		 * block. We have to check if its size field is bigger than what it really is.
		 * If it is, adjust its size, and add the entry after it, adjust our size
		 * to span the block fully. If not, continue as we did before to the next DBP.
		 */

		if(passed >= priv->blocksize)
		{
			std::printf("[ext2] Couldn't find it in DBP %d (%d > %d)\n", i, passed, priv->blocksize);
			continue;
		}
		/* Well, found a free entry! */
		d = (ext2_dir *)((uint32_t)d + d->size);
	dir_write:
		std::memcpy(d, entry, entry->size);
		ext2_write_block(block_buf, inode->dbp[i], priv);
		std::printf("[ext2] Wrote to %d\n", inode->dbp[i]);
		return 1;
	}
	std::printf("[ext2] Couldn't write.\n");
	return 0;
}

void ext2_read_inode(inode_t *inode_buf, uint32_t inode, ext2_priv_data *priv)
{
	uint32_t bg = (inode - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;
	/* Now we have which BG the inode is in, load that desc */
	if(!block_buf) block_buf = (uint8_t *)kmalloc(priv->blocksize);
	ext2_read_block(block_buf, priv->first_bgd, priv);
	block_group_desc_t *bgd = (block_group_desc_t*)block_buf;
	//printf("We seek BG %d\n", bg);
	/* Seek to the BG's desc */
	for(i = 0; i < bg; i++)
		bgd++;
	/* Find the index and seek to the inode */
	uint32_t index = (inode - 1) % priv->sb.inodes_in_blockgroup;
	//printf("Index of our inode is %d\n", index);
	uint32_t block = (index * sizeof(inode_t))/ priv->blocksize;
	//printf("Relative: %d, Absolute: %d\n", block, bgd->block_of_inode_table + block);
	ext2_read_block(block_buf, bgd->block_of_inode_table + block, priv);
	inode_t* _inode = (inode_t *)block_buf;
	index = index % priv->inodes_per_block;
	for(i = 0; i < index; i++)
		_inode++;
	/* We have found the inode! */
	std::memcpy(inode_buf, _inode, sizeof(inode_t));
}

void ext2_write_inode(inode_t *inode_buf, uint32_t ii, ext2_priv_data *priv)
{
	uint32_t bg = (ii - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;
	/* Now we have which BG the inode is in, load that desc */
	if(!block_buf) block_buf = (uint8_t *)kmalloc(priv->blocksize);
	ext2_read_block(block_buf, priv->first_bgd, priv);
	block_group_desc_t *bgd = (block_group_desc_t*)block_buf;
	std::printf("[ext2] We seek BG %d\n", bg);
	/* Seek to the BG's desc */
	for(i = 0; i < bg; i++)
		bgd++;
	/* Find the index and seek to the inode */
	uint32_t index = (ii - 1) % priv->sb.inodes_in_blockgroup;
	std::printf("[ext2] Index of our inode is %d\n", index);
	uint32_t block = (index * sizeof(inode_t))/ priv->blocksize;
	std::printf("[ext2] Relative: %d, Absolute: %d\n", block, bgd->block_of_inode_table + block);
	uint32_t final = bgd->block_of_inode_table + block;
	ext2_read_block(block_buf, final, priv);
	inode_t* _inode = (inode_t *)block_buf;
	index = index % priv->inodes_per_block;
	for(i = 0; i < index; i++)
		_inode++;
	/* We have found the inode! */
	std::memcpy(_inode, inode_buf, sizeof(inode_t));
	ext2_write_block(block_buf, final, priv);
}

uint32_t ext2_get_inode_block(uint32_t inode, uint32_t *b, uint32_t *ioff, ext2_priv_data *priv)
{
	uint32_t bg = (inode - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;
	/* Now we have which BG the inode is in, load that desc */
	if(!block_buf) block_buf = (uint8_t *)kmalloc(priv->blocksize);
	ext2_read_block(block_buf, priv->first_bgd, priv);
	block_group_desc_t *bgd = (block_group_desc_t*)block_buf;
	//printf("We seek BG %d\n", bg);
	/* Seek to the BG's desc */
	for(i = 0; i < bg; i++)
		bgd++;
	/* Find the index and seek to the inode */
	uint32_t index = (inode - 1) % priv->sb.inodes_in_blockgroup;
	uint32_t block = (index * sizeof(inode_t))/ priv->blocksize;
	index = index % priv->inodes_per_block;
	*b = block + bgd->block_of_inode_table;
	*ioff = index;
	return 1;
}

uint32_t ext2_read_directory(char *filename, ext2_dir *dir, ext2_priv_data *priv)
{
	while(dir->inode != 0) {
		char *name = (char *)kmalloc(dir->namelength + 1);
		std::memcpy(name, &dir->reserved+1, dir->namelength);
		name[dir->namelength] = 0;
		//kprintf("DIR: %s (%d)\n", name, dir->size);
		if(filename && std::strcmp(filename, name) == 0)
		{
			/* If we are looking for a file, we had found it */
			ext2_read_inode(inode, dir->inode, priv);
			std::printf("[ext2] Found inode %s! %d\n", filename, dir->inode);
			kfree(name);
			return dir->inode;
		}
		if(!filename && (uint32_t)filename != 1) {
			inode_t* inodebuf = (inode_t*)kmalloc(sizeof(inode_t));
			ext2_read_inode(inodebuf, dir->inode, priv);
			if (inodebuf->type & INODE_TYPE_DIRECTORY) {
				std::printf("[ext2] Directory: %s\n", name);
			} else if (inodebuf->type & INODE_TYPE_FILE) {
				std::printf("[ext2] File:      %s\n", name);
			} else {
				std::printf("[ext2] Other:     %s\n", name);
			}
			kfree(inodebuf);
		}
        //printf("name: %s\n", name);
		dir = (ext2_dir *)((uint32_t)dir + dir->size);
		kfree(name);
	}
	return 0;
}

uint8_t ext2_read_root_directory(char *filename, ext2_priv_data *priv)
{
	/* The root directory is always inode#2, so find BG and read the block. */
	if(!inode) inode = (inode_t *)kmalloc(sizeof(inode_t));
	if(!root_buf) root_buf = (uint8_t *)kmalloc(priv->blocksize);
	ext2_read_inode(inode, 2, priv);
	if((inode->type & 0xF000) != INODE_TYPE_DIRECTORY)
	{
		std::printf("[ext2] FATAL: Root directory is not a directory!\n");
		return 0;
	}
	/* We have found the directory!
	 * Now, load the starting block
	 */
	for(int i = 0;i < 12; i++)
	{
		uint32_t b = inode->dbp[i];
		if(b == 0) break;
		ext2_read_block(root_buf, b, priv);
		/* Now loop through the entries of the directory */
		if(ext2_read_directory(filename, (ext2_dir*)root_buf, priv)) return 1;
	}
	if(filename && (uint32_t)filename != 1) return 0;
	return 1;
}

uint8_t ext2_find_file_inode(char *ff, inode_t *inode_buf, ext2_priv_data *priv)
{
	char *filename = (char*)kmalloc(std::strlen((const char*)ff) + 1);
	std::memcpy(filename, ff, std::strlen((const char*)ff) +1);
	size_t n = std::strexplode(filename, '/');
	filename ++; // skip the first crap
	uint32_t retnode = 0;
	if(n > 1)
	{
		/* Read inode#2 (Root dir) into inode */
		ext2_read_inode(inode, 2, priv);
		/* Now, loop through the DPB's and see if it contains this filename */
		n--;
		while(n--)
		{
			//printf("[ext2] Looking for: %s\n", filename);
			for(int i = 0; i < 12; i++)
			{
				uint32_t b = inode->dbp[i];
				if(!b) break;
				ext2_read_block(root_buf, b, priv);
				uint32_t rc = ext2_read_directory(filename, (ext2_dir *)root_buf, priv);
				if(!rc)
				{
					if(std::strcmp(filename, "") == 0)
					{
						kfree(filename);
						return std::strcmp(ff, "/")?retnode:1;
					}
					std::printf("[ext2] File (%s (0x%x)) not found!\n", filename, filename);
					kfree(filename);
					return 0;
				} else {
					/* inode now contains that inode
					 * get out of the for loop and continue traversing
					 */
					 retnode = rc;
					 goto fix;
				}
			}
			fix:;
			uint32_t s = std::strlen((const char*)filename);
			filename += s + 1;
		}
		std::memcpy(inode_buf, inode, sizeof(inode_t));
	} else {
		/* This means the file is in the root directory */
		ext2_read_root_directory(filename, priv);
		std::memcpy(inode_buf, inode, sizeof(inode_t));
	}
	kfree(filename);
	return retnode;
}

#define SIZE_OF_SINGLY (priv->blocksize * priv->blocksize / 4)
uint8_t ext2_read_singly_linked(uint32_t blockid, uint8_t *buf, ext2_priv_data *priv)
{
	uint32_t blockadded = 0;
	uint32_t maxblocks = ((priv->blocksize) / (sizeof(uint32_t)));
	/* A singly linked block is essentially an array of
	 * uint32_t's storing the block's id which points to data
	 */
	 /* Read the block into root_buf */
	 ext2_read_block(root_buf, blockid, priv);
	 /* Loop through the block id's reading them into the appropriate buffer */
	 uint32_t *block = (uint32_t *)root_buf;
	 for(int i =0;i < maxblocks; i++)
	 {
	 	/* If it is zero, we have finished loading. */
	 	if(block[i] == 0) break;
	 	/* Else, read the block into the buffer */
	 	ext2_read_block(buf + i * priv->blocksize, block[i], priv);
	 }
	 return 1;
}

uint8_t ext2_read_doubly_linked(uint32_t blockid, uint8_t *buf, ext2_priv_data *priv)
{
	uint32_t blockadded = 0;
	uint32_t maxblocks = ((priv->blocksize) / (sizeof(uint32_t)));
	/* A singly linked block is essentially an array of
	 * uint32_t's storing the block's id which points to data
	 */
	 /* Read the block into root_buf */
	 ext2_read_block(block_buf, blockid, priv);
	 /* Loop through the block id's reading them into the appropriate buffer */
	 uint32_t *block = (uint32_t *)block_buf;
	 uint32_t s = SIZE_OF_SINGLY;
	 for(int i =0;i < maxblocks; i++)
	 {
	 	/* If it is zero, we have finished loading. */
	 	if(block[i] == 0) break;
	 	/* Else, read the block into the buffer */
	 	ext2_read_singly_linked(block[i], buf + i * s , priv);
	 }
	 return 1;
}

static inode_t *minode = 0;
uint8_t ext2_read_file(char *fn, char *buffer, ext2_priv_data *priv)
{
	/* Put the file's inode to the buffer */
	if(!minode) minode = (inode_t *)kmalloc(sizeof(inode_t));
	char *filename = fn;
	if(!ext2_find_file_inode(filename, minode, priv))
	{
		std::printf("[ext2] File inode not found.\n");
		return 0;
	}
	for(int i = 0; i < 12; i++)
	{
		uint32_t b = minode->dbp[i];
		if(b == 0) { std::printf("EOF\n"); return 1; }
		if(b > priv->sb.blocks) std::printf("[ext2] %s: block %d outside range (max: %d)!\n", __func__, b, priv->sb.blocks);
		std::printf("[ext2] Reading block: %d\n", b);

		ext2_read_block(root_buf, b, priv);
		//kprintf("Copying to: 0x%x size: %d bytes\n", buffer + i*(priv->blocksize), priv->blocksize);
		std::memcpy(buffer + i*(priv->blocksize), root_buf, priv->blocksize);
		//kprintf("%c%c%c\n", *(uint8_t*)(buffer + 1),*(uint8_t*)(buffer + 2), *(uint8_t*)(buffer + 3));
	}
	if(minode->singly_block) {
		//kprintf("Block of singly: %d\n", minode->singly_block);
		ext2_read_singly_linked(minode->singly_block, buffer + 12*(priv->blocksize), priv);
	}
	if(minode->doubly_block) {
		uint32_t s = SIZE_OF_SINGLY + 12*priv->blocksize;
		//kprintf("s is 0x%x (%d)\n", s, s);
		ext2_read_doubly_linked(minode->doubly_block, buffer + s, priv);
	}
	//mprint("Read all 12 DBP(s)! *BUG*\n");
	return 1;
}

void ext2_list_directory(char *dd, char *buffer, ext2_priv_data *priv)
{
	char *dir = dd;
	int rc = ext2_find_file_inode(dir, (inode_t *)buffer, priv);
	if(!rc) return;
	for(int i = 0;i < 12; i++)
	{
		uint32_t b = inode->dbp[i];
		if(!b) break;
		ext2_read_block(root_buf, b, priv);
		ext2_read_directory(0, (ext2_dir *)root_buf, priv);
	}
}

char* current_dir;
char* file_buffer;
bool ext2_online = true;

char* ext2_current_dir() {
    return current_dir;
}

void ext2_init() {
    if (!ata_online) return;

    current_dir = (uint8_t*)kmalloc(128);
    file_buffer = (char*)kmalloc(1024 * 64); // some large amount idk.
    char* _temp = "/"; // initial dir
    std::strcpy(current_dir, _temp);

    std::printf("[ext2] Attempt load ext2 fs on main dev.\n");
    // read superblock
    uint8_t* buf = (uint8_t*)kmalloc(1024);
    ata_read(buf, 2, 2, dev);
    superblock_t* sb = (superblock_t*)buf;

    if(sb->ext2_sig != EXT2_SIGNATURE)
	{
		std::printf("[ext2] Invalid EXT2 signature (0x%x)!\n", sb->ext2_sig);
        std::printf("[ext2] Not using EXT2.\n");
        ext2_online = false;
        return;
	}

    uint32_t blocksize = (1024 << sb->blocksize_hint);
    uint32_t inodes_per_block = blocksize / sizeof(inode_t);
    uint32_t volume_size = blocksize*(sb->blocks);

    std::printf("[ext2] Size of volume: %d bytes\n", volume_size);
    uint32_t block_bgdt = sb->superblock_id + (sizeof(superblock_t) / blocksize);
    uint32_t first_bgd = block_bgdt;

    uint32_t number_of_bgs0 = sb->blocks / sb->blocks_in_blockgroup;
    if(!number_of_bgs0) number_of_bgs0 = 1;
    std::printf("[ext2] There are %d block group(s).\n", number_of_bgs0);

    ext2_priv_data* priv = (ext2_priv_data*)kmalloc(sizeof(ext2_priv_data));
    std::memcpy(&priv->sb, sb, sizeof(superblock_t));
    priv->blocksize = blocksize;
    priv->inodes_per_block = blocksize / sizeof(inode_t);
    priv->sectors_per_block = blocksize / 512;
    priv->number_of_bgs = number_of_bgs0;
    priv->first_bgd = block_bgdt;

    _priv = priv;

    if (ext2_read_root_directory((char*)1, priv)) {
		std::printf("[ext2] Root directory read successful, reading enabled.\n");
	}

    char* c = (char*)kmalloc(4);
    std::strcpy(c, "bruh");
    //ext2_writefile("/ABC123test.txt", c, 4, priv);
	std::printf("[ext2] WARNING: File writing is broken, so attempting to create a new file will not work. Sorry :/\n");
}

bool CMD_ls(int argc, char** argv) {
    if (!ata_online || !ext2_online) return;
    char* ibuf = (char*)kmalloc(100);
    ext2_list_directory(current_dir, ibuf, _priv);

    return true;
}

bool CMD_cd(int argc, char** argv) {
    if (!ata_online || !ext2_online) return;

    std::strcpy(current_dir, argv[1]);

    return true;
}

bool CMD_nfile(int argc, char** argv) {
    if (!ata_online || !ext2_online) return;

    ext2_touch(argv[1], _priv);

    return true;
}

bool CMD_cat(int argc, char** argv) {
    if (!ata_online || !ext2_online) return;
    if (argc == 1) return;
    char* to_open = (char*)kmalloc(256); for (size_t i = 0; i < 256; i++) {to_open[i]=0;}

    for (size_t i = 0; i < 1024 * 64; i++) {
        file_buffer[i] = '\0';
    }

    if (std::strcmp(current_dir, "/") != 0) {
        char* ndir;
        std::memcpy(to_open, current_dir, std::strlen((const char*)current_dir));
        size_t i = std::strlen((const char*)current_dir);
        to_open[i] = '/';
        i++;
        for (; i < std::strlen((const char*)current_dir) + std::strlen((const char*)argv[1]) + 1; i++) {
            to_open[i] = argv[1][i - (std::strlen((const char*)current_dir) + 1)];
        }
    } else {
        char* ndir;
        std::memcpy(to_open, current_dir, std::strlen((const char*)current_dir));
        size_t i = std::strlen((const char*)current_dir);
        for (; i < std::strlen((const char*)current_dir) + std::strlen((const char*)argv[1]); i++) {
            to_open[i] = argv[1][i - (std::strlen((const char*)current_dir))];
        }
    }

    ext2_read_file(to_open, file_buffer, _priv);
    std::printf("File: %s\n%s\n", to_open, file_buffer);

    kfree(to_open);

    return true;
}
