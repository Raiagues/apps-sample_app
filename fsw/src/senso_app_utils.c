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

#include "senso_app.h"
#include "senso_app_tbl.h"
#include "senso_app_utils.h"

CFE_Status_t SENSO_APP_TblValidationFunc(void *TblData)
{
    CFE_Status_t             ReturnCode = CFE_SUCCESS;
    SENSO_APP_ExampleTable_t *TblDataPtr = (SENSO_APP_ExampleTable_t *)TblData;

    if (TblDataPtr->Int1 > SENSO_APP_PLATFORM_TBL_ELEMENT_1_MAX)
        ReturnCode = SENSO_APP_PLATFORM_TABLE_OUT_OF_RANGE_ERR_CODE;

    return ReturnCode;
}

void SENSO_APP_GetCrc(const char *TableName)
{
    CFE_Status_t   status;
    uint32         Crc;
    CFE_TBL_Info_t TblInfoPtr;

    status = CFE_TBL_GetInfo(&TblInfoPtr, TableName);
    if (status != CFE_SUCCESS)
        CFE_ES_WriteToSysLog("Sample App: Error Getting Example Table Info");
    else
    {
        Crc = TblInfoPtr.Crc;
        CFE_ES_WriteToSysLog("Sample App: CRC: 0x%08lX\n\n", (unsigned long)Crc);
    }
}

/* [SENSOR_INFRA] */
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

typedef enum {
    PARSE_FLOAT = 0,
    PARSE_DOUBLE,
    PARSE_TRIPLE_X,
    PARSE_TRIPLE_Y,
    PARSE_TRIPLE_Z,
    PARSE_UINT8,
    PARSE_UINT16,
    PARSE_GPS_0, PARSE_GPS_1, PARSE_GPS_2, PARSE_GPS_3,
    PARSE_GPS_4, PARSE_GPS_5, PARSE_GPS_6, PARSE_GPS_7,
    PARSE_GPS_8, PARSE_GPS_9, PARSE_GPS_10, PARSE_GPS_11,
} SensorParseType_t;

typedef struct {
    const char       *serial_tag;
    SensorParseType_t parse_type;
    size_t            payload_offset;
} SensorField_t;

static void SENSO_APP_ParseSensorField(
    const char *buf, const SensorField_t *f, void *payload_base)
{
    char search[12];
    snprintf(search, sizeof(search), "%s:", f->serial_tag);
    const char *p = strstr(buf, search);
    if (!p) return;
    p += strlen(search);
    if (strncmp(p, "ERR", 3) == 0) return;

    void *dst = (char *)payload_base + f->payload_offset;

    switch (f->parse_type) {
        case PARSE_FLOAT:    *(float  *)dst = (float)atof(p); break;
        case PARSE_DOUBLE:   *(double *)dst = atof(p);         break;
        case PARSE_TRIPLE_X: *(float  *)dst = (float)atof(p); break;
        case PARSE_TRIPLE_Y: {
            const char *q = strchr(p, ',');
            if (q) *(float *)dst = (float)atof(q + 1);
            break;
        }
        case PARSE_TRIPLE_Z: {
            const char *q = strchr(p, ',');
            if (q) { q = strchr(q + 1, ','); if (q) *(float *)dst = (float)atof(q + 1); }
            break;
        }
        case PARSE_UINT8:  *(uint8  *)dst = (uint8)atoi(p);  break;
        case PARSE_UINT16: *(uint16 *)dst = (uint16)atoi(p); break;
        default: {
            int n = (int)f->parse_type - (int)PARSE_GPS_0;
            const char *q = p;
            int _gi;
            for (_gi = 0; _gi < n; _gi++) {
                q = strchr(q, ',');
                if (!q) return;
                q++;
            }
            if (strncmp(q, "ERR", 3) == 0) return;
            if      (n == 0 || n == 1) *(double *)dst = atof(q);
            else if (n <= 3)           *(float  *)dst = (float)atof(q);
            else if (n == 11)          *(uint16 *)dst = (uint16)atoi(q);
            else                       *(uint8  *)dst = (uint8)atoi(q);
            break;
        }
    }
}
/* [/SENSOR_INFRA] */


/* [SENSOR:bmp280] */
static const SensorField_t SENSO_APP_BMP280_Fields[] = {
    { "PRES", PARSE_FLOAT, offsetof(SENSO_APP_HkTlm_Payload_t, BMP280.Pressure) },
    { "ALT", PARSE_FLOAT, offsetof(SENSO_APP_HkTlm_Payload_t, BMP280.EstimatedAltitude) },
};
#define SENSO_APP_BMP280_NFIELDS (sizeof(SENSO_APP_BMP280_Fields)/sizeof(SENSO_APP_BMP280_Fields[0]))
/* [/SENSOR:bmp280] */

/* [SERIAL_INFRA] */
static int senso_app_serial_fd = -1;

static void SENSO_APP_InitSerial(void)
{
    struct termios tty;
    senso_app_serial_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_SYNC);
    if (senso_app_serial_fd < 0) return;
    memset(&tty, 0, sizeof(tty));
    tcgetattr(senso_app_serial_fd, &tty);
    cfsetospeed(&tty, B115200); cfsetispeed(&tty, B115200);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag = 0; tty.c_oflag = 0; tty.c_lflag = 0;
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cc[VMIN] = 0; tty.c_cc[VTIME] = 20;
    tcsetattr(senso_app_serial_fd, TCSANOW, &tty);
    tcflush(senso_app_serial_fd, TCIOFLUSH);
}

static void SENSO_APP_ReadSerial(SENSO_APP_HkTlm_Payload_t *Payload)
{
    char buf[256];
    int  total = 0;
    char c;
    int  n;
    struct timeval tv;
    fd_set readfds;

    if (senso_app_serial_fd < 0) SENSO_APP_InitSerial();
    if (senso_app_serial_fd < 0) return;

    FD_ZERO(&readfds);
    FD_SET(senso_app_serial_fd, &readfds);
    tv.tv_sec = 2; tv.tv_usec = 0;
    if (select(senso_app_serial_fd + 1, &readfds, NULL, NULL, &tv) <= 0) return;

    memset(buf, 0, sizeof(buf));
    while (total < (int)sizeof(buf) - 1) {
        n = read(senso_app_serial_fd, &c, 1);
        if (n <= 0) { senso_app_serial_fd = -1; return; }
        buf[total++] = c;
        if (c == '\n') break;
    }

    /* [SENSOR_CALLS:start] */
    /* [SENSOR:bmp280:calls] */
    { size_t _si; for (_si=0;_si<SENSO_APP_BMP280_NFIELDS;_si++) SENSO_APP_ParseSensorField(buf,&SENSO_APP_BMP280_Fields[_si],Payload); }
    /* [/SENSOR:bmp280:calls] */
    /* [SENSOR_CALLS:end] */
}
/* [/SERIAL_INFRA] */

/* [SENSOR_UPDATE] */
void SENSO_APP_UpdateSensors(SENSO_APP_HkTlm_Payload_t *Payload)
{
    /* [SENSOR:bmp280:defaults] */
    Payload->BMP280.Pressure = -1.0f;
    Payload->BMP280.EstimatedAltitude = -1.0f;
    /* [/SENSOR:bmp280:defaults] */
    SENSO_APP_ReadSerial(Payload);
}
/* [/SENSOR_UPDATE] */
