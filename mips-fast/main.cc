#include "mips.h"
#include "decode.h"
#include "executor.h"
#include "memory.h"
#include "wb.h"
#include "tasking.h"
#include <stdlib.h>
#include <string.h>

void cleanup (void)
{
  Log::CloseLog ();
}

#define SIZE 256

int main (int argc, char **argv)
{
  Mipc *mh;
  Decode *dec;
  Exe *exec;
  Memory *mem;
  Mem *m;
  Writeback *wb;
  char buf[SIZE];
  char *fname,*cname;
  Bool l,c;

  l = FALSE;
  c = FALSE;

  RegisterDefault ("Mipc.BootROM", "mipc.image");
  RegisterDefault ("Mipc.BootPC", (int)0xbfc00000);
  RegisterDefault ("Mipc.ArgvAddr", (int)0xbfc00100);
  RegisterDefault ("Mipc.CacheLineToWatch", 0x1ULL);
  RegisterDefault ("Log.FileName", "mipc.log");
  RegisterDefault ("Log.Level", "");
  RegisterDefault ("MemSystem.Type", "None");
  RegisterDefault ("Log.StartDumpTime", 0);
  RegisterDefault ("Mipc.PeriodicTimer", 100000);

  /* fixup arguments */
  if (argc > 1) {
     if (argv[1][0]=='-' && argv[1][1]=='l') {
        l = TRUE;
        argv++;
        argc--;
        MALLOC(fname,char,strlen(argv[1])+1);
        sprintf(fname,"%s",argv[1]);
	argc--;
        argv++;
     }
  }

  if (argc > 1) {
     if (argv[1][0]=='-' && argv[1][1]=='c') {
        c = TRUE;
        argv++;
        argc--;
        MALLOC(cname,char,strlen(argv[1])+1);  
        sprintf(cname,"%s",argv[1]);
        argc--;
        argv++;
        printf("conf file name is %s\n", cname);
     }
  }

  if (!c) {
     ReadConfigFile();
  }
  else {
     ReadConfigFile(cname);
  }

  logTimer = ParamGetLL("Log.StartDumpTime");

  if (argc > 1) {
    if (strlen (argv[1]) > SIZE - sizeof(".image")) {
      fatal_error ("Pathname `%s' too long!\n", argv[1]);
    }
    sprintf (buf, "%s.image", argv[1]);
    OverrideConfig ("Mipc.BootROM", buf);
    argc--;
    argv++;
  }

  if (!l) {
     Log::OpenLog (ParamGetString ("Log.FileName"));
  }
  else {
     Log::OpenLog (fname);
  }

  m = new Mem();

  mh = new Mipc(m);
  dec = new Decode(mh);
  exec = new Exe(mh);
  mem = new Memory(mh);
  wb = new Writeback(mh);
  SimCreateTask (mh, "FETCH");
  SimCreateTask (dec, "DECODE");
  SimCreateTask (exec, "EXE");
  SimCreateTask (mem, "MEM");
  SimCreateTask (wb, "WB");

  /* there are arguments! */
  if (argc > 0) 
	mh->_sys->ArgumentSetup (argc, argv, ParamGetInt ("Mipc.ArgvAddr"));

  simulate (cleanup);
}
