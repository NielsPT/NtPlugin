//
//  Academic License - for use in teaching, academic research, and meeting
//  course requirements at degree granting institutions only.  Not for
//  government, commercial, or other organizational use.
//
//  myrupDelay_types.h
//
//  Code generation for function 'onParamChangeCImpl'
//


#ifndef MYRUPDELAY_TYPES_H
#define MYRUPDELAY_TYPES_H

// Include files
#include "myrupDelay.h"
#include "rtwtypes.h"

// Type Definitions
struct myrupDelayPersistentData
{
  derivedAudioPlugin plugin;
  boolean_T plugin_not_empty;
  unsigned long thisPtr;
  boolean_T thisPtr_not_empty;
};

struct myrupDelayStackData
{
  myrupDelayPersistentData *pd;
};

#endif

// End of code generation (myrupDelay_types.h)
