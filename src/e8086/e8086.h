/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     e8086.h                                                    *
 * Created:       1996-04-28 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2003 by Hampa Hug <hampa@hampa.ch>                *
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

/* $Id: e8086.h,v 1.5 2003/04/16 17:19:17 hampa Exp $ */


#ifndef PCE_E8086_H
#define PCE_E8086_H 1


#include <stdio.h>


/* CPU flags */
#define E86_FLG_C 0x0001
#define E86_FLG_P 0x0004
#define E86_FLG_A 0x0010
#define E86_FLG_Z 0x0040
#define E86_FLG_S 0x0080
#define E86_FLG_T 0x0100
#define E86_FLG_I 0x0200
#define E86_FLG_D 0x0400
#define E86_FLG_O 0x0800

/* 16 bit register values */
#define E86_REG_AX 0
#define E86_REG_CX 1
#define E86_REG_DX 2
#define E86_REG_BX 3
#define E86_REG_SP 4
#define E86_REG_BP 5
#define E86_REG_SI 6
#define E86_REG_DI 7

/* 8 bit register values */
#define E86_REG_AL 0
#define E86_REG_CL 1
#define E86_REG_DL 2
#define E86_REG_BL 3
#define E86_REG_AH 4
#define E86_REG_CH 5
#define E86_REG_DH 6
#define E86_REG_BH 7

/* Segment register values */
#define E86_REG_ES 0
#define E86_REG_CS 1
#define E86_REG_SS 2
#define E86_REG_DS 3

#define E86_PREFIX_NEW  0x0001
#define E86_PREFIX_SEG  0x0002
#define E86_PREFIX_REP  0x0004
#define E86_PREFIX_REPN 0x0008
#define E86_PREFIX_LOCK 0x0010

#define E86_PQ_SIZE 6


typedef struct {
  unsigned char (*mem_get_uint8) (void *mem, unsigned long addr);
  unsigned short (*mem_get_uint16) (void *mem, unsigned long addr);

  void (*mem_set_uint8) (void *mem, unsigned long addr, unsigned char val);
  void (*mem_set_uint16) (void *mem, unsigned long addr, unsigned short val);

  unsigned char (*prt_get_uint8) (void *mem, unsigned long addr);
  unsigned short (*prt_get_uint16) (void *mem, unsigned long addr);

  void (*prt_set_uint8) (void *mem, unsigned long addr, unsigned char val);
  void (*prt_set_uint16) (void *mem, unsigned long addr, unsigned short val);

  void (*hook) (void *ext, unsigned char op1, unsigned char op2);

  void (*opstat) (void *ext, unsigned char op1, unsigned char op2);

  void *mem;
  void *prt;
  void *ext;

  unsigned short dreg[8];
  unsigned short sreg[4];
  unsigned short ip;
  unsigned short flg;

  unsigned       pq_cnt;
  unsigned char  pq[E86_PQ_SIZE];

  unsigned       prefix;

  unsigned short seg_override;

  struct {
    int            is_mem;
    unsigned char  *data;
    unsigned short seg;
    unsigned short ofs;
    unsigned short cnt;
    unsigned long  delay;
  } ea;

  unsigned long  delay;

  unsigned long  clocks;
  unsigned long  instructions;
} e8086_t;


#define e86_get_reg8(cpu, reg) \
  ((((reg) & 4) ? ((cpu)->dreg[(reg) & 3] >> 8) : (cpu)->dreg[(reg) & 3]) & 0xff)

#define e86_get_al(cpu) ((cpu)->dreg[E86_REG_AX] & 0xff)
#define e86_get_bl(cpu) ((cpu)->dreg[E86_REG_BX] & 0xff)
#define e86_get_cl(cpu) ((cpu)->dreg[E86_REG_CX] & 0xff)
#define e86_get_dl(cpu) ((cpu)->dreg[E86_REG_DX] & 0xff)
#define e86_get_ah(cpu) (((cpu)->dreg[E86_REG_AX] >> 8) & 0xff)
#define e86_get_bh(cpu) (((cpu)->dreg[E86_REG_BX] >> 8) & 0xff)
#define e86_get_ch(cpu) (((cpu)->dreg[E86_REG_CX] >> 8) & 0xff)
#define e86_get_dh(cpu) (((cpu)->dreg[E86_REG_DX] >> 8) & 0xff)


#define e86_get_reg16(cpu, reg) \
  ((cpu)->dreg[(reg) & 7])

#define e86_get_ax(cpu) ((cpu)->dreg[E86_REG_AX])
#define e86_get_bx(cpu) ((cpu)->dreg[E86_REG_BX])
#define e86_get_cx(cpu) ((cpu)->dreg[E86_REG_CX])
#define e86_get_dx(cpu) ((cpu)->dreg[E86_REG_DX])
#define e86_get_sp(cpu) ((cpu)->dreg[E86_REG_SP])
#define e86_get_bp(cpu) ((cpu)->dreg[E86_REG_BP])
#define e86_get_si(cpu) ((cpu)->dreg[E86_REG_SI])
#define e86_get_di(cpu) ((cpu)->dreg[E86_REG_DI])


#define e86_get_sreg(cpu, reg) \
  ((cpu)->sreg[(reg) & 3])

#define e86_get_cs(cpu) ((cpu)->sreg[E86_REG_CS])
#define e86_get_ds(cpu) ((cpu)->sreg[E86_REG_DS])
#define e86_get_es(cpu) ((cpu)->sreg[E86_REG_ES])
#define e86_get_ss(cpu) ((cpu)->sreg[E86_REG_SS])


#define e86_set_reg8(cpu, reg, val) \
  do { \
    if ((reg) & 4) { \
      (cpu)->dreg[(reg) & 3] &= 0x00ff; \
      (cpu)->dreg[(reg) & 3] |= ((val) & 0xff) << 8; \
    } \
    else { \
      (cpu)->dreg[(reg) & 3] &= 0xff00; \
      (cpu)->dreg[(reg) & 3] |= (val) & 0xff; \
    } \
  } while (0)

#define e86_set_al(cpu, val) e86_set_reg8 (cpu, E86_REG_AL, val)
#define e86_set_bl(cpu, val) e86_set_reg8 (cpu, E86_REG_BL, val)
#define e86_set_cl(cpu, val) e86_set_reg8 (cpu, E86_REG_CL, val)
#define e86_set_dl(cpu, val) e86_set_reg8 (cpu, E86_REG_DL, val)
#define e86_set_ah(cpu, val) e86_set_reg8 (cpu, E86_REG_AH, val)
#define e86_set_bh(cpu, val) e86_set_reg8 (cpu, E86_REG_BH, val)
#define e86_set_ch(cpu, val) e86_set_reg8 (cpu, E86_REG_CH, val)
#define e86_set_dh(cpu, val) e86_set_reg8 (cpu, E86_REG_DH, val)


#define e86_set_reg16(cpu, reg, val) \
  do { \
    (cpu)->dreg[(reg) & 7] = (val) & 0xffff; \
  } while (0)

#define e86_set_ax(cpu, val) do { (cpu)->dreg[E86_REG_AX] = (val) & 0xffff; } while (0)
#define e86_set_bx(cpu, val) do { (cpu)->dreg[E86_REG_BX] = (val) & 0xffff; } while (0)
#define e86_set_cx(cpu, val) do { (cpu)->dreg[E86_REG_CX] = (val) & 0xffff; } while (0)
#define e86_set_dx(cpu, val) do { (cpu)->dreg[E86_REG_DX] = (val) & 0xffff; } while (0)
#define e86_set_sp(cpu, val) do { (cpu)->dreg[E86_REG_SP] = (val) & 0xffff; } while (0)
#define e86_set_bp(cpu, val) do { (cpu)->dreg[E86_REG_BP] = (val) & 0xffff; } while (0)
#define e86_set_si(cpu, val) do { (cpu)->dreg[E86_REG_SI] = (val) & 0xffff; } while (0)
#define e86_set_di(cpu, val) do { (cpu)->dreg[E86_REG_DI] = (val) & 0xffff; } while (0)


#define e86_set_sreg(cpu, reg, val) \
  do { \
    (cpu)->sreg[(reg) & 3] = (val) & 0xffff; \
  } while (0)

#define e86_set_cs(cpu, val) do { (cpu)->sreg[E86_REG_CS] = (val) & 0xffff; } while (0)
#define e86_set_ds(cpu, val) do { (cpu)->sreg[E86_REG_DS] = (val) & 0xffff; } while (0)
#define e86_set_es(cpu, val) do { (cpu)->sreg[E86_REG_ES] = (val) & 0xffff; } while (0)
#define e86_set_ss(cpu, val) do { (cpu)->sreg[E86_REG_SS] = (val) & 0xffff; } while (0)

#define e86_get_ip(cpu) ((cpu)->ip)
#define e86_set_ip(cpu, val) do { (cpu)->ip = (val) & 0xffff; } while (0)


#define e86_get_flg(cpu, f) (((cpu)->flg & (f)) != 0)
#define e86_get_cf(cpu) (((cpu)->flg & E86_FLG_C) != 0)
#define e86_get_pf(cpu) (((cpu)->flg & E86_FLG_P) != 0)
#define e86_get_af(cpu) (((cpu)->flg & E86_FLG_A) != 0)
#define e86_get_zf(cpu) (((cpu)->flg & E86_FLG_Z) != 0)
#define e86_get_of(cpu) (((cpu)->flg & E86_FLG_O) != 0)
#define e86_get_sf(cpu) (((cpu)->flg & E86_FLG_S) != 0)
#define e86_get_df(cpu) (((cpu)->flg & E86_FLG_D) != 0)
#define e86_get_if(cpu) (((cpu)->flg & E86_FLG_I) != 0)
#define e86_get_tf(cpu) (((cpu)->flg & E86_FLG_T) != 0)


#define e86_set_flg(c, f, v) \
  do { if (v) (c)->flg |= (f); else (c)->flg &= ~(f); } while (0)

#define e86_set_cf(c, v) \
  do { if (v) (c)->flg |= E86_FLG_C; else (c)->flg &= ~E86_FLG_C; } while (0)

#define e86_set_pf(c, v) \
  do { if (v) (c)->flg |= E86_FLG_P; else (c)->flg &= ~E86_FLG_P; } while (0)

#define e86_set_af(c, v) \
  do { if (v) (c)->flg |= E86_FLG_A; else (c)->flg &= ~E86_FLG_A; } while (0)

#define e86_set_zf(c, v) \
  do { if (v) (c)->flg |= E86_FLG_Z; else (c)->flg &= ~E86_FLG_Z; } while (0)

#define e86_set_of(c, v) \
  do { if (v) (c)->flg |= E86_FLG_O; else (c)->flg &= ~E86_FLG_O; } while (0)

#define e86_set_sf(c, v) \
  do { if (v) (c)->flg |= E86_FLG_S; else (c)->flg &= ~E86_FLG_S; } while (0)

#define e86_set_flg0(cpu, f) do { (cpu)->flg &= ~(f); } while (0)
#define e86_set_flg1(cpu, f) do { (cpu)->flg |= (f); } while (0)


#define e86_get_linear(seg, ofs) \
  ((((seg) & 0xffff) << 4) + ((ofs) & 0xffff))

#define e86_get_mem8(cpu, seg, ofs) \
  ((cpu)->mem_get_uint8 ((cpu)->mem, e86_get_linear (seg, ofs) & 0xfffff))

#define e86_get_mem16(cpu, seg, ofs) \
  ((cpu)->mem_get_uint16 ((cpu)->mem, e86_get_linear (seg, ofs) & 0xfffff))

#define e86_set_mem8(cpu, seg, ofs, val) \
  do { \
    (cpu)->mem_set_uint8 ((cpu)->mem, e86_get_linear (seg, ofs) & 0xfffff, val); \
  } while (0)

#define e86_set_mem16(cpu, seg, ofs, val) \
  do { \
    (cpu)->mem_set_uint16 ((cpu)->mem, e86_get_linear (seg, ofs) & 0xfffff, val); \
  } while (0)


void e86_prt_state (e8086_t *c, FILE *fp);

void e86_execute (e8086_t *c);
void e86_clock (e8086_t *c);

void e86_reset (e8086_t *c);

e8086_t *e86_new (void);

void e86_del (e8086_t *c);


typedef struct {
  unsigned short ip;

  unsigned       dat_n;
  unsigned char  dat[16];

  char           op[64];

  unsigned       arg_n;
  char           arg1[64];
  char           arg2[64];
} e86_disasm_t;


void e86_disasm (e86_disasm_t *op, unsigned char *src, unsigned short ip);
void e86_disasm_mem (e8086_t *c, e86_disasm_t *op, unsigned short, unsigned short ip);
void e86_disasm_cur (e8086_t *c, e86_disasm_t *op);


#endif