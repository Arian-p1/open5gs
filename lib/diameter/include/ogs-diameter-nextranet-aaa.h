/*
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

#ifndef OGS_DIAMETER_NEXTRANET_AAA_H
#define OGS_DIAMETER_NEXTRANET_AAA_H

#include "ogs-diameter-common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OGS_DIAM_NEXTRANET_AAA_APPLICATION_ID 45001

extern struct dict_object *ogs_diam_nextranet_aaa_application;

extern struct dict_object *ogs_diam_nextranet_aaa_cmd_auth;
extern struct dict_object *ogs_diam_nextranet_aaa_cmd_auth_ans;
extern struct dict_object *ogs_diam_nextranet_aaa_cmd_term;
extern struct dict_object *ogs_diam_nextranet_aaa_cmd_term_ans;

extern struct dict_object *ogs_diam_nextranet_aaa_imsi;
extern struct dict_object *ogs_diam_nextranet_aaa_imei;
extern struct dict_object *ogs_diam_nextranet_aaa_apn;
extern struct dict_object *ogs_diam_nextranet_aaa_result;
extern struct dict_object *ogs_diam_nextranet_aaa_uli;

/* Nextranet AVP Values */
#define OGS_DIAM_NEXTRANET_AAA_AUTH_SUCCESS         0
#define OGS_DIAM_NEXTRANET_AAA_AUTH_FAILURE         1

/* Command codes */
#define OGS_DIAM_NEXTRANET_AAA_CMD_CODE_AUTH        10001
#define OGS_DIAM_NEXTRANET_AAA_CMD_CODE_TERM        10002

/* Initialization function */
int ogs_diam_nextranet_aaa_init(void);

int ogs_dict_nextranet_aaa_entry(char *conffile);

#ifdef __cplusplus
}
#endif

#endif /* OGS_DIAMETER_NEXTRANET_AAA_H */ 