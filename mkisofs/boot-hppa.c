/*
 * Program boot-hppa.c - Handle HPPA boot extensions to iso9660.
 *
 * Written by Steve McIntyre <steve@einval.com> June 2004.
 *
 * Heavily inspired by palo:
 *
 ****************************************************************************
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) Hewlett-Packard (Paul Bame) paul_bame@hp.com
 *
 ****************************************************************************
 * Copyright 2004 Steve McIntyre
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <mconfig.h>
#include "mkisofs.h"
#include <fctldefs.h>
#include <utypes.h>
#include <intcvt.h>
#include "match.h"
#include "diskmbr.h"
#include "bootinfo.h"
#include <schily.h>
#include "endian.h"

int     add_boot_hppa_cmdline     __PR((char *cmdline));
int     add_boot_hppa_kernel_32   __PR((char *filename));
int     add_boot_hppa_kernel_64   __PR((char *filename));
int     add_boot_hppa_bootloader  __PR((char *filename));
int     add_boot_hppa_ramdisk     __PR((char *filename));

static  int     boot_hppa_write   __PR((FILE *outfile));

static  char   *hppa_cmdline = NULL;
static  char   *hppa_kernel_32 = NULL;
static  char   *hppa_kernel_64 = NULL;
static  char   *hppa_bootloader = NULL;
static  char   *hppa_ramdisk = NULL;

/* Simple function: store the filename to be used later when we need
   to find the boot file */
extern int add_boot_hppa_cmdline(cmdline)
    char *cmdline;
{
    char *ptr = NULL;
    hppa_cmdline = strdup(cmdline);
    ptr = hppa_cmdline;
    while (*ptr)
    {
        if (',' == *ptr)
            *ptr = ' ';
        ptr++;
    }    
    return 0;
}

/* Simple function: store the filename to be used later when we need
   to find the boot file */
extern int add_boot_hppa_kernel_32(filename)
    char *filename;
{
    hppa_kernel_32 = filename;
    return 0;
}

/* Simple function: store the filename to be used later when we need
   to find the boot file */
extern int add_boot_hppa_kernel_64(filename)
    char *filename;
{
    hppa_kernel_64 = filename;
    return 0;
}

/* Simple function: store the filename to be used later when we need
   to find the boot file */
extern int add_boot_hppa_bootloader(filename)
    char *filename;
{
    hppa_bootloader = filename;
    return 0;
}

/* Simple function: store the filename to be used later when we need
   to find the boot file */
extern int add_boot_hppa_ramdisk(filename)
    char *filename;
{
    hppa_ramdisk = filename;
    return 0;
}

static void exit_fatal(type, filename)
    char *type;
    char *filename;
{
#ifdef	USE_LIBSCHILY
    comerrno(EX_BAD, "Uh oh, I can't find the %s '%s'!\n",
             type, filename);
#else
    fprintf(stderr, "Uh oh, I can't find the %s '%s'!\n",
             type, filename);
    exit(1);
#endif
}

static int boot_hppa_write(outfile)
    FILE *outfile;
{
    unsigned char boot_sector[2048]; /* One (ISO) sector */
	struct directory_entry	*boot_file;	/* Boot file we need to search for */
    unsigned long length = 0;
    unsigned long extent = 0;
    int i = 0;

    memset(boot_sector, 0, sizeof(boot_sector));    

    boot_sector[0] = 0x80;  /* magic */
    boot_sector[1] = 0x00;  /* magic */
    boot_sector[2] = 'P';
    boot_sector[3] = 'A';
    boot_sector[4] = 'L';
    boot_sector[5] = 'O';
    boot_sector[6] = 0x00;
    boot_sector[7] = 0x04;  /* version */

    /* Find the dir entry for the 32-bit kernel by walking our file list */
    boot_file = search_tree_file(root, hppa_kernel_32);
    if (!boot_file)
        exit_fatal("HPPA 32-bit kernel", hppa_kernel_32);
    extent = 2048 * get_733(boot_file->isorec.extent);
    length = get_733(boot_file->isorec.size);
    fprintf(stderr, "Found hppa 32-bit kernel %s: using extent %lu (0x%lX), size %lu (0x%lX)\n",
            hppa_kernel_32, extent, extent, length, length);
    write_be32(extent, &boot_sector[8]);
    write_be32(length, &boot_sector[12]);

    /* Find the dir entry for the ramdisk by walking our file list */
    boot_file = search_tree_file(root, hppa_ramdisk);
    if (!boot_file)
        exit_fatal("HPPA ramdisk", hppa_ramdisk);
    extent = 2048 * get_733(boot_file->isorec.extent);
    length = get_733(boot_file->isorec.size);
    fprintf(stderr, "Found hppa ramdisk %s: using extent %lu (0x%lX), size %lu (0x%lX)\n",
            hppa_ramdisk, extent, extent, length, length);
    write_be32(extent, &boot_sector[16]);
    write_be32(length, &boot_sector[20]);

    /* Now the commandline */
    snprintf(&boot_sector[24], 127, "%s", hppa_cmdline);

    /* Find the dir entry for the 64-bit kernel by walking our file list */
    boot_file = search_tree_file(root, hppa_kernel_64);
    if (!boot_file)
        exit_fatal("HPPA 64-bit kernel", hppa_kernel_64);
    extent = 2048 * get_733(boot_file->isorec.extent);
    length = get_733(boot_file->isorec.size);
    fprintf(stderr, "Found hppa 64-bit kernel %s: using extent %lu (0x%lX), size %lu (0x%lX)\n",
            hppa_kernel_64, extent, extent, length, length);
    write_be32(extent, &boot_sector[232]);
    write_be32(length, &boot_sector[236]);

    /* Find the dir entry for the IPL by walking our file list */
    boot_file = search_tree_file(root, hppa_bootloader);
    if (!boot_file)
        exit_fatal("HPPA bootloader", hppa_bootloader);
    extent = 2048 * get_733(boot_file->isorec.extent);
    length = get_733(boot_file->isorec.size);
    fprintf(stderr, "Found hppa bootloader %s: using extent %lu (0x%lX), size %lu (0x%lX)\n",
            hppa_bootloader, extent, extent, length, length);
    write_be32(extent, &boot_sector[240]);
    write_be32(length, &boot_sector[244]);

    jtwrite(boot_sector, sizeof(boot_sector), 1, 0, FALSE);
    xfwrite(boot_sector, sizeof(boot_sector), 1, outfile, 0, FALSE);
    last_extent_written++;

    return 0;
}

struct output_fragment hppaboot_desc = {NULL, oneblock_size, NULL, boot_hppa_write, "hppa boot block"};



