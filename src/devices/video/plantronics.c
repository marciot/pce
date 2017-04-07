/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/plantronics.c                              *
 * Created:     2008-10-13 by John Elliott <jce@seasip.demon.co.uk>          *
 * Copyright:   (C) 2008-2016 Hampa Hug <hampa@hampa.ch>                     *
 *              (C) 2008-2016 John Elliott <jce@seasip.demon.co.uk>          *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include <stdio.h>
#include <stdlib.h>

#include <lib/log.h>

#include <devices/video/cga.h>
#include <devices/video/plantronics.h>


#define CGA_MODE          8
#define CGA_CSEL          9
#define CGA_STATUS        10
#define PLA_SPECIAL       13

#define CGA_MODE_G320     0x02

#define PLA_SPECIAL_EXT2  0x10
#define PLA_SPECIAL_EXT1  0x20
#define PLA_SPECIAL_PLANE 0x40

#define PLA_UPDATE_DIRTY 1


static inline
int pla_alternate_plane (const cga_t *pla)
{
	if ((pla->reg[CGA_MODE] & CGA_MODE_G320) == 0) {
		return (0);
	}

	if ((pla->reg[PLA_SPECIAL] & PLA_SPECIAL_PLANE) == 0) {
		return (0);
	}

	if ((pla->reg[PLA_SPECIAL] & (PLA_SPECIAL_EXT1 | PLA_SPECIAL_EXT2)) == 0) {
		return (0);
	}

	return (1);
}

static
void pla_mem_set_uint8 (cga_t *pla, unsigned long addr, unsigned char val)
{
	if (pla_alternate_plane (pla)) {
		addr ^= 0x4000;
	}

	if (addr < pla->memblk->size) {
		pla->mem[addr] = val;
		pla->update_state |= PLA_UPDATE_DIRTY;
	}
}

static
unsigned char pla_mem_get_uint8 (cga_t *pla, unsigned long addr)
{
	if (pla_alternate_plane (pla)) {
		addr ^= 0x4000;
	}

	if (addr < pla->memblk->size) {
		return (pla->mem[addr]);
	}

	return (0);
}

static
unsigned short pla_mem_get_uint16 (cga_t *pla, unsigned long addr)
{
	unsigned short val;

	val = pla_mem_get_uint8 (pla, addr);
	val |= (unsigned) pla_mem_get_uint8 (pla, addr + 1) << 8;

	return (val);
}

static
void pla_mem_set_uint16 (cga_t *pla, unsigned long addr, unsigned short val)
{
	pla_mem_set_uint8 (pla, addr, val & 0xff);
	pla_mem_set_uint8 (pla, addr + 1, (val >> 8) & 0xff);
}


/*
 * Update mode 3 (graphics 320 * 200 * 16)
 */
static
void pla_mode3_update (cga_t *pla)
{
	unsigned            i;
	unsigned            x, y, w, h;
	unsigned            val0, val1, idx;
	unsigned char       *dst;
	const unsigned char *col;
	const unsigned char *mem[4];

	if (cga_set_buf_size (pla, 8 * pla->w, 2 * pla->h)) {
		return;
	}

	mem[0] = pla->mem;
	mem[2] = pla->mem + 0x4000;
	mem[1] = mem[0] + 8192;
	mem[3] = mem[2] + 8192;

	dst = pla->buf;

	w = 2 * pla->w;
	h = 2 * pla->h;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			val0 = mem[(y & 1) + 0][x];
			val1 = mem[(y & 1) + 2][x];

			for (i = 0; i < 4; i++) {
				idx = ((val1 >> 7) & 1) | ((val0 >> 5) & 2);
				idx |= ((val0 >> 5) & 4) | ((val1 >> 3) & 8);

				col = cga_rgb[idx];

				dst[0] = col[0];
				dst[1] = col[1];
				dst[2] = col[2];

				val0 <<= 2;
				val1 <<= 2;

				dst += 3;
			}
		}

		mem[(y & 1) + 0] += w;
		mem[(y & 1) + 2] += w;
	}
}

/*
 * Update mode 4 (graphics 620 * 200 * 4)
 */
static
void pla_mode4_update (cga_t *pla)
{
	unsigned            i, x, y, w, h;
	unsigned            val0, val1, idx;
	unsigned char       *dst;
	const unsigned char *col;
	const unsigned char *mem[4];

	if (cga_set_buf_size (pla, 16 * pla->w, 2 * pla->h)) {
		return;
	}

	mem[0] = pla->mem;
	mem[2] = pla->mem + 0x4000;
	mem[1] = mem[0] + 8192;
	mem[3] = mem[2] + 8192;

	dst = pla->buf;

	w = 2 * pla->w;
	h = 2 * pla->h;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			val0 = mem[(y & 1) + 0][x];
			val1 = mem[(y & 1) + 2][x];

			for (i = 0; i < 8; i++) {
				idx = ((val0 >> 7) & 1) | ((val1 >> 6) & 2);
				idx = pla->pal[idx];
				col = cga_rgb[idx];

				dst[0] = col[0];
				dst[1] = col[1];
				dst[2] = col[2];

				val0 <<= 1;
				val1 <<= 1;

				dst += 3;
			}
		}

		mem[(y & 1) + 0] += w;
		mem[(y & 1) + 2] += w;
	}
}

static
void pla_update (cga_t *pla)
{
	unsigned char mode, spec;

	mode = pla->reg[CGA_MODE];
	spec = pla->reg[PLA_SPECIAL];

	if (mode & CGA_MODE_G320) {
		if (spec & (PLA_SPECIAL_EXT1 | PLA_SPECIAL_EXT2)) {
			if (spec & PLA_SPECIAL_EXT2) {
				pla_mode3_update (pla);
			}
			else {
				pla_mode4_update (pla);
			}

			return;
		}
	}

	cga_update (pla);
}


static
unsigned char pla_get_special (cga_t *pla)
{
	return (pla->reg[PLA_SPECIAL]);
}

static
void pla_set_special (cga_t *pla, unsigned char val)
{
	pla->reg[PLA_SPECIAL] = val;

	pla->update_state |= PLA_UPDATE_DIRTY;
}

/*
 * Get a Plantronics register
 */
static
unsigned char pla_reg_get_uint8 (cga_t *pla, unsigned long addr)
{
	if (addr == PLA_SPECIAL) {
		return (pla_get_special (pla));
	}

	return (cga_reg_get_uint8 (pla, addr));
}

unsigned short pla_reg_get_uint16 (cga_t *pla, unsigned long addr)
{
	unsigned short ret;

	ret = pla_reg_get_uint8 (pla, addr);

	if ((addr + 1) < pla->regblk->size) {
		ret |= pla_reg_get_uint8 (pla, addr + 1) << 8;
	}

	return (ret);
}

/*
 * Set a Plantronics register
 */
void pla_reg_set_uint8 (cga_t *pla, unsigned long addr, unsigned char val)
{
	if (addr == PLA_SPECIAL) {
		pla_set_special (pla, val);
		return;
	}

	cga_reg_set_uint8 (pla, addr, val);
}

void pla_reg_set_uint16 (cga_t *pla, unsigned long addr, unsigned short val)
{
	pla_reg_set_uint8 (pla, addr, val & 0xff);

	if ((addr + 1) < pla->regblk->size) {
		pla_reg_set_uint8 (pla, addr + 1, val >> 8);
	}
}


static
void pla_print_info (cga_t *pla, FILE *fp)
{
	unsigned i;

	fprintf (fp, "DEV: Plantronics Colorplus\n");

	fprintf (fp, "CGA: OFS=%04X  POS=%04X  BG=%02X  PAL=%u\n",
		cga_get_start (pla),
		cga_get_cursor (pla),
		pla->reg[CGA_CSEL] & 0x0f,
		(pla->reg[CGA_CSEL] >> 5) & 1
	);

	fprintf (fp,
		"REG: MODE=%02X  CSEL=%02X  STATUS=%02X  SPEC=%02X"
		"  PAL=[%02X %02X %02X %02X]\n",
		pla->reg[CGA_MODE],
		pla->reg[CGA_CSEL],
		pla->reg[CGA_STATUS],
		pla->reg[PLA_SPECIAL],
		pla->pal[0], pla->pal[1], pla->pal[2], pla->pal[3]
	);

	fprintf (fp, "CRTC=[%02X", pla->reg_crt[0]);
	for (i = 1; i < 18; i++) {
		fputs ((i & 7) ? " " : "-", fp);
		fprintf (fp, "%02X", pla->reg_crt[i]);
	}
	fputs ("]\n", fp);

	fflush (fp);
}


void pla_init (cga_t *pla, unsigned long io, unsigned long addr, unsigned long size)
{
	if (size < 32768) {
		size = 32768;
	}

	cga_init (pla, io, addr, size);

	pla->video.print_info = (void *) pla_print_info;

	pla->update = pla_update;

	pla->regblk->set_uint8 = (void *) pla_reg_set_uint8;
	pla->regblk->set_uint16 = (void *) pla_reg_set_uint16;
	pla->regblk->get_uint8 = (void *) pla_reg_get_uint8;
	pla->regblk->get_uint16 = (void *) pla_reg_get_uint16;
}

void pla_free (cga_t *pla)
{
	cga_free (pla);
}

cga_t *pla_new (unsigned long io, unsigned long addr, unsigned long size)
{
	cga_t *pla;

	pla = malloc (sizeof (cga_t));
	if (pla == NULL) {
		return (NULL);
	}

	pla_init (pla, io, addr, size);

	return (pla);
}

video_t *pla_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr, size;
	unsigned      blink;
	cga_t         *pla;

	ini_get_uint32 (sct, "io", &io, 0x3d0);
	ini_get_uint32 (sct, "address", &addr, 0xb8000);
	ini_get_uint32 (sct, "size", &size, 32768);
	ini_get_uint16 (sct, "blink", &blink, 0);

	if (size < 32768) {
		size = 32768;
	}

	pce_log_tag (MSG_INF,
		"VIDEO:", "Plantronics io=0x%04lx addr=0x%05lx size=0x%05lx\n",
		io, addr, size
	);

	pla = pla_new (io, addr, size);
	if (pla == NULL) {
		return (NULL);
	}

	pla->memblk->set_uint8 = (void *) pla_mem_set_uint8;
	pla->memblk->set_uint16 = (void *) pla_mem_set_uint16;
	pla->memblk->get_uint8 = (void *) pla_mem_get_uint8;
	pla->memblk->get_uint16 = (void *) pla_mem_get_uint16;

	cga_set_blink_rate (pla, blink);

	return (&pla->video);
}
