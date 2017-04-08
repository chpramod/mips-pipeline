/*-*-mode:c++-*-**********************************************************
 *
 *  Copyright (c) 1999 Cornell University
 *  Computer Systems Laboratory
 *  Ithaca, NY 14853
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
 *  $Id: config.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define DEFAULT_CONFIG_FILE     "sim.conf"

void ReadConfigFile (char *name = DEFAULT_CONFIG_FILE); 
// read variables from configuration file

char *ParamGetString (char *name); 
// read string parameter, returns NULL if non-existent

int ParamGetInt (char *name);
// read int parameter, return 0 if non-existent

float ParamGetFloat (char *name); 
// read floating-point parameter, return 0 if non-existent

#define PARAM_YES "Yes"
#define PARAM_NO "No"
int ParamGetBool (char *name);
// read boolean parameter, return 1 if TRUE, 0 if FALSE. return 0 if non-existent

typedef unsigned long long int LL;
LL ParamGetLL (char *name);
// read LL parameter, return 0 if non-existent

void RegisterDefault (char *name, int val);
void RegisterDefault (char *name, LL val);
void RegisterDefault (char *name, float val);
void RegisterDefault (char *name, char *s);
void OverrideConfig (char *name, char *value);

#endif /* __CONFIG_H__ */
