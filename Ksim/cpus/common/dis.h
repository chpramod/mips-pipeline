/*-*-mode:c++-*-**********************************************************
 *
 *  Copyright (c) 1999 Cornell University
 *  Computer Systems Laboratory
 *  Cornell University, Ithaca, NY 14853
 *  All Rights Reserved
 *
 *  Permission to use, copy, modify, and distribute this software
 *  and its documentation for any purpose and without fee is hereby
 *  granted, provided that the above copyright notice appear in all
 *  copies. Cornell University makes no representations
 *  about the suitability of this software for any purpose. It is
 *  provided "as is" without express or implied warranty. Export of this
 *  software outside of the United States of America may require an
 *  export license.
 *
 *  $Id: dis.h,v 1.1.1.1 2006/05/23 13:53:58 mainakc Exp $
 *
 **************************************************************************
 */
#ifndef __DIS_H__
#define __DIS_H__

#define DIS_TYPE_NUMBER 0x1
#define DIS_TYPE_NAMES  0x2

char *mips_dis_insn (int type, unsigned int ins);

#endif /* __DIS_H__ */
