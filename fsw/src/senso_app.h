/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *
 * Main header file for the Sample application
 */

#ifndef SENSO_APP_H
#define SENSO_APP_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"

#include "senso_app_mission_cfg.h"
#include "senso_app_platform_cfg.h"

#include "senso_app_perfids.h"
#include "senso_app_msgids.h"
#include "senso_app_msg.h"

/************************************************************************
** Sensor Metadata Types
*************************************************************************/

#define SENSO_MAX_FAIL_COUNT 3  /**< Consecutive failures before EVS ERROR */
#define SENSO_DISABLE_COUNT  6  /**< Consecutive failures before auto-disable */

typedef enum
{
    SENSOR_TEMP = 0,
    SENSOR_HUM,
    SENSOR_PRES,
    SENSOR_ALT,
    SENSOR_ACC,
    SENSOR_GYR,
    SENSOR_GPS,
    SENSO_NUM_SENSORS
} SENSO_Sensor_e;

typedef struct
{
    uint8       enabled;               /**< 1 = active, 0 = disabled */
    uint8       fail_count;            /**< Consecutive read failures */
    const char *name;                  /**< Human-readable name       */
    const char *tag;                   /**< Serial wire tag (e.g. "TEMP") */
    const char *unit;                  /**< Unit string for logging   */
    uint8       num_values;            /**< 1, 3, or 12 values        */
    const char *value_names[3];        /**< Field names (max 3)       */
} SENSO_Sensor_Metadata_t;

/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Global Data
*/
typedef struct
{
    /*
    ** Command interface counters...
    */
    uint8 CommandCounter;
    uint8 CommandErrorCounter;

    /*
    ** Housekeeping telemetry packet...
    */
    SENSO_APP_HkTlm_t HkTlm;

    /*
    ** Run Status variable used in the main processing loop
    */
    uint32 RunStatus;

    /*
    ** Operational data (not reported in housekeeping)...
    */
    CFE_SB_PipeId_t CommandPipe;

    CFE_TBL_Handle_t TblHandles[SENSO_APP_PLATFORM_NUMBER_OF_TABLES];
} SENSO_APP_Data_t;

/*
** Global data structure
*/
extern SENSO_APP_Data_t SENSO_APP_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (SENSO_APP_Main), these
**       functions are not called from any other source module.
*/
void         SENSO_APP_Main(void);
CFE_Status_t SENSO_APP_Init(void);

#endif /* SENSO_APP_H */
