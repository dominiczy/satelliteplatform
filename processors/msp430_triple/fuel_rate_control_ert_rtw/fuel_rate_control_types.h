/*
 * File: fuel_rate_control_types.h
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

#ifndef RTW_HEADER_fuel_rate_control_types_h_
#define RTW_HEADER_fuel_rate_control_types_h_
#include "rtwtypes.h"
#ifndef _DEFINED_TYPEDEF_FOR_sld_FuelModes_
#define _DEFINED_TYPEDEF_FOR_sld_FuelModes_

typedef enum {
  LOW = 1,                             /* Default value */
  RICH,
  DISABLED
} sld_FuelModes;

#endif

#ifndef _DEFINED_TYPEDEF_FOR_EngSensors_
#define _DEFINED_TYPEDEF_FOR_EngSensors_

typedef struct {
  real32_T throttle;
  real32_T speed;
  real32_T ego;
  real32_T map;
} EngSensors;

#endif

/* Forward declaration for rtModel */
typedef struct tag_RTM_fuel_rate_control_T RT_MODEL_fuel_rate_control_T;

#endif                                 /* RTW_HEADER_fuel_rate_control_types_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
