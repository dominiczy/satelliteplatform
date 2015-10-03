/*
 * File: fuel_rate_control_private.h
 *
 * Code generated for Simulink model 'fuel_rate_control'.
 *
 * Model version                  : 1.534
 * Simulink Coder version         : 8.6 (R2014a) 27-Dec-2013
 * C/C++ source code generated on : Thu Aug 20 17:04:08 2015
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Texas Instruments->MSP430
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef RTW_HEADER_fuel_rate_control_private_h_
#define RTW_HEADER_fuel_rate_control_private_h_
#include "rtwtypes.h"
#ifndef __RTWTYPES_H__
#error This file requires rtwtypes.h to be included
#endif                                 /* __RTWTYPES_H__ */

extern real32_T look2_iflf_linlca(real32_T u0, real32_T u1, const real32_T bp0[],
  const real32_T bp1[], const real32_T table[], const uint32_T maxIndex[],
  uint32_T stride);

#endif                                 /* RTW_HEADER_fuel_rate_control_private_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
