/*
 ****************************************************************************
 *
 *                   "DHRYSTONE" Benchmark Program
 *                   -----------------------------
 *
 *  Version:    C, Version 2.1
 *
 *  File:       dhry_1.c (part 2 of 3)
 *
 *  Date:       May 25, 1988
 *
 *  Author:     Reinhold P. Weicker
 *
 ****************************************************************************
 */

#include "dhry.h"
#if defined(CONFIG_RISCV64) || defined(CONFIG_LEON3)
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
char _Next_Glob [sizeof (Rec_Type)];
char _Glob[sizeof (Rec_Type)];

extern int printf_uart(const char *_format, ... );

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#pragma GCC diagnostic ignored "-Wimplicit-int"
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#endif

/* Global Variables: */

Rec_Pointer     Ptr_Glob,
                Next_Ptr_Glob;
int             Int_Glob;
Boolean         Bool_Glob;
char            Ch_1_Glob,
                Ch_2_Glob;
int             Arr_1_Glob [50];
int             Arr_2_Glob [50] [50];

//extern char     *malloc ();
Enumeration     Func_1 ();
  /* forward declaration necessary since Enumeration may not simply be int */

#ifndef REG
        Boolean Reg = false;
#define REG
        /* REG becomes defined as empty */
        /* i.e. no register variables   */
#else
        Boolean Reg = true;
#endif

/* variables for time measurement: */

#ifdef CONFIG_LEON3
uint64_t k_cycle_get_64() {
    // 0x8000030C was added to into grlib gptimer in 2017 Jan 28.
    uint32 *p_gpTimer = (uint32 *)0x80000300;
    return (uint64_t)p_gpTimer[3];
}
#endif

#ifdef CONFIG_RISCV64
uint64_t k_cycle_get_64() {
    uint64_t *mtime = (uint64_t *)(0x0200bff8);
    return *mtime;
}
#endif

#ifdef TIMES
#if defined(CONFIG_RISCV64) || defined(CONFIG_LEON3)
#define HZ 60000000  // 60 MHz
int clock() {
    return (int)k_cycle_get_64();
}
#define Too_Small_Time (2)

#else
struct tms      time_info;
extern clock_t times_ (struct tms *t);
/* see library function "times" */
#define Too_Small_Time (2*HZ)
#endif
                /* Measurements should last at least about 2 seconds */
#endif
#ifdef TIME
extern long     time();
                /* see library function "time"  */
#define Too_Small_Time 2
                /* Measurements should last at least 2 seconds */
#endif
#ifdef MSC_CLOCK
extern clock_t	clock();
#define Too_Small_Time (2*HZ)
#endif

long            Begin_Time,
                End_Time,
                User_Time;
#if defined(CONFIG_RISCV64) || defined (CONFIG_LEON3)
uint64_t        Microseconds,
                Dhrystones_Per_Second;
#else
float           Microseconds,
                Dhrystones_Per_Second;
#endif

/* end of variables for time measurement */

#if defined(CONFIG_RISCV64) || defined (CONFIG_LEON3)
int shell_cmd_soc_dhry(int argc, char *argv[])
#else
main ()
#endif
/*****/

  /* main program, corresponds to procedures        */
  /* Main and Proc_0 in the Ada version             */
{
        One_Fifty       Int_1_Loc;
  REG   One_Fifty       Int_2_Loc;
        One_Fifty       Int_3_Loc;
  REG   char            Ch_Index;
        Enumeration     Enum_Loc;
        Str_30          Str_1_Loc;
        Str_30          Str_2_Loc;
  REG   int             Run_Index;
  REG   int             Number_Of_Runs;

  /* Initializations */

#if defined(CONFIG_RISCV64) || defined (CONFIG_LEON3)
  Next_Ptr_Glob = (Rec_Pointer) &_Next_Glob;
  Ptr_Glob = (Rec_Pointer) &_Glob ;
#else
  Next_Ptr_Glob = (Rec_Pointer) malloc (sizeof (Rec_Type));
  Ptr_Glob = (Rec_Pointer) malloc (sizeof (Rec_Type));
#endif

  Ptr_Glob->Ptr_Comp                    = Next_Ptr_Glob;
  Ptr_Glob->Discr                       = Ident_1;
  Ptr_Glob->variant.var_1.Enum_Comp     = Ident_3;
  Ptr_Glob->variant.var_1.Int_Comp      = 40;
  strcpy (Ptr_Glob->variant.var_1.Str_Comp,
          "DHRYSTONE PROGRAM, SOME STRING");
  strcpy (Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING");

  Arr_2_Glob [8][7] = 10;
        /* Was missing in published program. Without this statement,    */
        /* Arr_2_Glob [8][7] would have an undefined value.             */
        /* Warning: With 16-Bit processors and Number_Of_Runs > 32000,  */
        /* overflow may occur for this array element.                   */

  printf_uart ("\n");
  printf_uart ("Dhrystone Benchmark, Version 2.1 (Language: C)\n");
  printf_uart ("\n");
  if (Reg)
  {
    printf_uart ("Program compiled with 'register' attribute\n");
    printf_uart ("\n");
  }
  else
  {
    printf_uart ("Program compiled without 'register' attribute\n");
    printf_uart ("\n");
  }
#if defined(CONFIG_RISCV64) || defined (CONFIG_LEON3)
  Number_Of_Runs = 16*1024;
#else
  printf_uart ("Please give the number of runs through the benchmark: ");
  {
    int n;
    scanf ("%d", &n);
    Number_Of_Runs = n;
  }
  printf_uart ("\n");
#endif

  printf_uart ("Execution starts, %d runs through Dhrystone\n", Number_Of_Runs);

  /***************/
  /* Start timer */
  /***************/

#ifdef TIMES
#if defined(CONFIG_RISCV64) || defined (CONFIG_LEON3)
  Begin_Time = (long) clock();
#else
  times (&time_info);
  Begin_Time = (long) time_info.tms_utime;
#endif
#endif
#ifdef TIME
  Begin_Time = time ( (long *) 0);
#endif
#ifdef MSC_CLOCK
  Begin_Time = clock();
#endif

  for (Run_Index = 1; Run_Index <= Number_Of_Runs; ++Run_Index)
  {

    Proc_5();
    Proc_4();
      /* Ch_1_Glob == 'A', Ch_2_Glob == 'B', Bool_Glob == true */
    Int_1_Loc = 2;
    Int_2_Loc = 3;
    strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
    Enum_Loc = Ident_2;
    Bool_Glob = ! Func_2 (Str_1_Loc, Str_2_Loc);
      /* Bool_Glob == 1 */
    while (Int_1_Loc < Int_2_Loc)  /* loop body executed once */
    {
      Int_3_Loc = 5 * Int_1_Loc - Int_2_Loc;
        /* Int_3_Loc == 7 */
      Proc_7 (Int_1_Loc, Int_2_Loc, &Int_3_Loc);
        /* Int_3_Loc == 7 */
      Int_1_Loc += 1;
    } /* while */
      /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
    Proc_8 (Arr_1_Glob, Arr_2_Glob, Int_1_Loc, Int_3_Loc);
      /* Int_Glob == 5 */
    Proc_1 (Ptr_Glob);
    for (Ch_Index = 'A'; Ch_Index <= Ch_2_Glob; ++Ch_Index)
                             /* loop body executed twice */
    {
      if (Enum_Loc == Func_1 (Ch_Index, 'C'))
          /* then, not executed */
        {
        Proc_6 (Ident_1, &Enum_Loc);
        strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 3'RD STRING");
        Int_2_Loc = Run_Index;
        Int_Glob = Run_Index;
        }
    }
      /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
    Int_2_Loc = Int_2_Loc * Int_1_Loc;
    Int_1_Loc = Int_2_Loc / Int_3_Loc;
    Int_2_Loc = 7 * (Int_2_Loc - Int_3_Loc) - Int_1_Loc;
      /* Int_1_Loc == 1, Int_2_Loc == 13, Int_3_Loc == 7 */
    Proc_2 (&Int_1_Loc);
      /* Int_1_Loc == 5 */

  } /* loop "for Run_Index" */

  /**************/
  /* Stop timer */
  /**************/

#ifdef TIMES
#if defined(CONFIG_RISCV64) || defined (CONFIG_LEON3)
  End_Time = (long) clock();
#else
  times_ (&time_info);
  End_Time = (long) time_info.tms_utime;
#endif
#endif
#ifdef TIME
  End_Time = time ( (long *) 0);
#endif
#ifdef MSC_CLOCK
  End_Time = clock();
#endif

  printf_uart ("Execution ends\n");
  printf_uart ("\n");
  printf_uart ("Final values of the variables used in the benchmark:\n");
  printf_uart ("\n");
  printf_uart ("Int_Glob:            %d\n", Int_Glob);
  printf_uart ("        should be:   %d\n", 5);
  printf_uart ("Bool_Glob:           %d\n", Bool_Glob);
  printf_uart ("        should be:   %d\n", 1);
  printf_uart ("Ch_1_Glob:           %c\n", Ch_1_Glob);
  printf_uart ("        should be:   %c\n", 'A');
  printf_uart ("Ch_2_Glob:           %c\n", Ch_2_Glob);
  printf_uart ("        should be:   %c\n", 'B');
  printf_uart ("Arr_1_Glob[8]:       %d\n", Arr_1_Glob[8]);
  printf_uart ("        should be:   %d\n", 7);
  printf_uart ("Arr_2_Glob[8][7]:    %d\n", Arr_2_Glob[8][7]);
  printf_uart ("        should be:   Number_Of_Runs + 10\n");
  printf_uart ("Ptr_Glob->\n");
  printf_uart ("  Ptr_Comp:          %d\n", (int) Ptr_Glob->Ptr_Comp);
  printf_uart ("        should be:   (implementation-dependent)\n");
  printf_uart ("  Discr:             %d\n", Ptr_Glob->Discr);
  printf_uart ("        should be:   %d\n", 0);
  printf_uart ("  Enum_Comp:         %d\n", Ptr_Glob->variant.var_1.Enum_Comp);
  printf_uart ("        should be:   %d\n", 2);
  printf_uart ("  Int_Comp:          %d\n", Ptr_Glob->variant.var_1.Int_Comp);
  printf_uart ("        should be:   %d\n", 17);
  printf_uart ("  Str_Comp:          %s\n", Ptr_Glob->variant.var_1.Str_Comp);
  printf_uart ("        should be:   DHRYSTONE PROGRAM, SOME STRING\n");
  printf_uart ("Next_Ptr_Glob->\n");
  printf_uart ("  Ptr_Comp:          %d\n", (int) Next_Ptr_Glob->Ptr_Comp);
  printf_uart ("        should be:   (implementation-dependent), same as above\n");
  printf_uart ("  Discr:             %d\n", Next_Ptr_Glob->Discr);
  printf_uart ("        should be:   %d\n", 0);
  printf_uart ("  Enum_Comp:         %d\n", Next_Ptr_Glob->variant.var_1.Enum_Comp);
  printf_uart ("        should be:   %d\n", 1);
  printf_uart ("  Int_Comp:          %d\n", Next_Ptr_Glob->variant.var_1.Int_Comp);
  printf_uart ("        should be:   %d\n", 18);
  printf_uart ("  Str_Comp:          %s\n",
                                Next_Ptr_Glob->variant.var_1.Str_Comp);
  printf_uart ("        should be:   DHRYSTONE PROGRAM, SOME STRING\n");
  printf_uart ("Int_1_Loc:           %d\n", Int_1_Loc);
  printf_uart ("        should be:   %d\n", 5);
  printf_uart ("Int_2_Loc:           %d\n", Int_2_Loc);
  printf_uart ("        should be:   %d\n", 13);
  printf_uart ("Int_3_Loc:           %d\n", Int_3_Loc);
  printf_uart ("        should be:   %d\n", 7);
  printf_uart ("Enum_Loc:            %d\n", Enum_Loc);
  printf_uart ("        should be:   %d\n", 1);
  printf_uart ("Str_1_Loc:           %s\n", Str_1_Loc);
  printf_uart ("        should be:   DHRYSTONE PROGRAM, 1'ST STRING\n");
  printf_uart ("Str_2_Loc:           %s\n", Str_2_Loc);
  printf_uart ("        should be:   DHRYSTONE PROGRAM, 2'ND STRING\n");
  printf_uart ("\n");

  User_Time = End_Time - Begin_Time;

  if (User_Time < Too_Small_Time)
  {
    printf_uart ("Measured time too small to obtain meaningful results\n");
    printf_uart ("Please increase number of runs\n");
    printf_uart ("\n");
  }
  else
  {

#ifdef TIME
    Microseconds = (float) User_Time * Mic_secs_Per_Second
                        / (float) Number_Of_Runs;
    Dhrystones_Per_Second = (float) Number_Of_Runs / (float) User_Time;
#elif defined(CONFIG_RISCV64) || defined (CONFIG_LEON3)
#else
    Microseconds = (float) User_Time * Mic_secs_Per_Second
                        / ((float) HZ * ((float) Number_Of_Runs));
    Dhrystones_Per_Second = ((float) HZ * (float) Number_Of_Runs)
                        / (float) User_Time;
#endif
    printf_uart ("Microseconds for one run through Dhrystone: ");
#if defined(CONFIG_RISCV64) || defined (CONFIG_LEON3)
    int64_t div1 = (int64_t)HZ*(int64_t)Number_Of_Runs;
    Microseconds = 10ll * (int64_t)User_Time * Mic_secs_Per_Second;
    printf_uart ("%6d.%01d \n", (int)(Microseconds/(10*div1)), (int)(Microseconds%10));
#else
    printf_uart ("%6.1f \n", Microseconds);
#endif
    printf_uart ("Dhrystones per Second:                      ");
#if defined(CONFIG_RISCV64) || defined (CONFIG_LEON3)
    Dhrystones_Per_Second = 10ll * ((int64_t)HZ * (int64_t) Number_Of_Runs);
    div1 = (int64_t)User_Time;
    printf_uart ("%6d.%01d \n", (int)(Dhrystones_Per_Second/(10*div1)), (int)(Dhrystones_Per_Second%10));
#else
    printf_uart ("%6.1f \n", Dhrystones_Per_Second);
#endif
    printf_uart ("\n");

  }

  return 0;
}


Proc_1 (Ptr_Val_Par)
/******************/

REG Rec_Pointer Ptr_Val_Par;
    /* executed once */
{
  REG Rec_Pointer Next_Record = Ptr_Val_Par->Ptr_Comp;
                                        /* == Ptr_Glob_Next */
  /* Local variable, initialized with Ptr_Val_Par->Ptr_Comp,    */
  /* corresponds to "rename" in Ada, "with" in Pascal           */

  structassign (*Ptr_Val_Par->Ptr_Comp, *Ptr_Glob);
  Ptr_Val_Par->variant.var_1.Int_Comp = 5;
  Next_Record->variant.var_1.Int_Comp
        = Ptr_Val_Par->variant.var_1.Int_Comp;
  Next_Record->Ptr_Comp = Ptr_Val_Par->Ptr_Comp;
  Proc_3 (&Next_Record->Ptr_Comp);
    /* Ptr_Val_Par->Ptr_Comp->Ptr_Comp
                        == Ptr_Glob->Ptr_Comp */
  if (Next_Record->Discr == Ident_1)
    /* then, executed */
  {
    Next_Record->variant.var_1.Int_Comp = 6;
    Proc_6 (Ptr_Val_Par->variant.var_1.Enum_Comp,
           &Next_Record->variant.var_1.Enum_Comp);
    Next_Record->Ptr_Comp = Ptr_Glob->Ptr_Comp;
    Proc_7 (Next_Record->variant.var_1.Int_Comp, 10,
           &Next_Record->variant.var_1.Int_Comp);
  }
  else /* not executed */
    structassign (*Ptr_Val_Par, *Ptr_Val_Par->Ptr_Comp);
} /* Proc_1 */


Proc_2 (Int_Par_Ref)
/******************/
    /* executed once */
    /* *Int_Par_Ref == 1, becomes 4 */

One_Fifty   *Int_Par_Ref;
{
  One_Fifty  Int_Loc;
  Enumeration   Enum_Loc;

  Int_Loc = *Int_Par_Ref + 10;
  do /* executed once */
    if (Ch_1_Glob == 'A')
      /* then, executed */
    {
      Int_Loc -= 1;
      *Int_Par_Ref = Int_Loc - Int_Glob;
      Enum_Loc = Ident_1;
    } /* if */
  while (Enum_Loc != Ident_1); /* true */
} /* Proc_2 */


Proc_3 (Ptr_Ref_Par)
/******************/
    /* executed once */
    /* Ptr_Ref_Par becomes Ptr_Glob */

Rec_Pointer *Ptr_Ref_Par;

{
  if (Ptr_Glob != Null)
    /* then, executed */
    *Ptr_Ref_Par = Ptr_Glob->Ptr_Comp;
  Proc_7 (10, Int_Glob, &Ptr_Glob->variant.var_1.Int_Comp);
} /* Proc_3 */


Proc_4 () /* without parameters */
/*******/
    /* executed once */
{
  Boolean Bool_Loc;

  Bool_Loc = Ch_1_Glob == 'A';
  Bool_Glob = Bool_Loc | Bool_Glob;
  Ch_2_Glob = 'B';
} /* Proc_4 */


Proc_5 () /* without parameters */
/*******/
    /* executed once */
{
  Ch_1_Glob = 'A';
  Bool_Glob = false;
} /* Proc_5 */


        /* Procedure for the assignment of structures,          */
        /* if the C compiler doesn't support this feature       */
#ifdef  NOSTRUCTASSIGN
memcpy (d, s, l)
register char   *d;
register char   *s;
register int    l;
{
        while (l--) *d++ = *s++;
}
#endif

#if defined(CONFIG_RISCV64) || defined (CONFIG_LEON3)
#pragma GCC diagnostic pop
#endif
