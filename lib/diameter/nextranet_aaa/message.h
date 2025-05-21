/* Nextranet AAA Interface
 * Copyright (C) 2024 by Open5GS
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(OGS_DIAMETER_INSIDE) && !defined(OGS_DIAMETER_COMPILATION)
#error "This header cannot be included directly."
#endif

#ifndef OGS_DIAM_NEXTRANET_AAA_MESSAGE_H
#define OGS_DIAM_NEXTRANET_AAA_MESSAGE_H

#include "ogs-diameter-common.h"
#include "ogs-diameter-nextranet-aaa.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OGS_DIAM_NEXTRANET_AAA_APPLICATION_ID 111111112

#define OGS_DIAM_NEXTRANET_AAA_CMD_CODE_AUTH 111111113
#define OGS_DIAM_NEXTRANET_AAA_CMD_CODE_TERM 111111120

/* Vendor specific AVP definitions */
#define OGS_DIAM_NEXTRANET_VENDOR_ID 111111111
#define OGS_DIAM_NEXTRANET_AVP_RESULT_CODE 111111114
#define OGS_DIAM_NEXTRANET_AVP_IMSI 111111115
#define OGS_DIAM_NEXTRANET_AVP_IMEI 111111116
#define OGS_DIAM_NEXTRANET_AVP_APN 111111117
#define OGS_DIAM_NEXTRANET_AVP_TAC 111111118
#define OGS_DIAM_NEXTRANET_AVP_CGI 111111119
#define OGS_DIAM_NEXTRANET_AVP_EUCGI 1111111110

extern struct dict_object *ogs_diam_nextranet_aaa_application;

extern struct dict_object *ogs_diam_nextranet_aaa_cmd_auth;
extern struct dict_object *ogs_diam_nextranet_aaa_cmd_auth_answer;
extern struct dict_object *ogs_diam_nextranet_aaa_cmd_term;
extern struct dict_object *ogs_diam_nextranet_aaa_cmd_term_answer;

extern struct dict_object *ogs_diam_nextranet_aaa_result;
extern struct dict_object *ogs_diam_nextranet_aaa_imsi;
extern struct dict_object *ogs_diam_nextranet_aaa_imei;
extern struct dict_object *ogs_diam_nextranet_aaa_apn;
extern struct dict_object *ogs_diam_nextranet_aaa_tac;
extern struct dict_object *ogs_diam_nextranet_aaa_cgi;
extern struct dict_object *ogs_diam_nextranet_aaa_eucgi;

/* Vendor-Specific-Application-Id */
typedef struct ogs_diam_nextranet_aaa_message_s {
    struct {
        uint32_t vendor_id;
        uint32_t app_id;
    } avp;

    union {
        struct {
            /* Auth-Application-Id */
            uint32_t auth_application_id;
        } aar;
        struct {
            /* Auth-Application-Id */
            uint32_t auth_application_id;
            /* Result */
            uint32_t result_code;
        } aaa;
    };
} ogs_diam_nextranet_aaa_message_t;

int ogs_dict_nextranet_aaa_entry(char *conffile);
int ogs_diam_nextranet_aaa_init(void);

int ogs_diam_nextranet_aaa_init_message(
        ogs_diam_nextranet_aaa_message_t *msg, int type);

#ifdef __cplusplus
}
#endif

#endif /* OGS_DIAM_NEXTRANET_AAA_MESSAGE_H */ 