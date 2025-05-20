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

#include "ogs-diameter-nextranet-aaa.h"

#define CHECK_dict_search( _type, _criteria, _what, _result )  \
    CHECK_FCT(fd_dict_search(fd_g_config->cnf_dict, (_type), (_criteria), (_what), (_result), ENOENT))

struct dict_object *ogs_diam_nextranet_aaa_application = NULL;

struct dict_object *ogs_diam_nextranet_aaa_cmd_auth = NULL;
struct dict_object *ogs_diam_nextranet_aaa_cmd_auth_answer = NULL;

struct dict_object *ogs_diam_nextranet_aaa_result = NULL;
struct dict_object *ogs_diam_nextranet_aaa_imsi = NULL;
struct dict_object *ogs_diam_nextranet_aaa_imei = NULL;
struct dict_object *ogs_diam_nextranet_aaa_apn = NULL;
struct dict_object *ogs_diam_nextranet_aaa_tac = NULL;
struct dict_object *ogs_diam_nextranet_aaa_cgi = NULL;
struct dict_object *ogs_diam_nextranet_aaa_eucgi = NULL;

int ogs_diam_nextranet_aaa_init(void)
{
    application_id_t id = OGS_DIAM_NEXTRANET_AAA_APPLICATION_ID;

    ogs_assert(ogs_dict_nextranet_aaa_entry(NULL) == 0);

    CHECK_dict_search(DICT_APPLICATION, APPLICATION_BY_ID, (void *)&id, &ogs_diam_nextranet_aaa_application);

    CHECK_dict_search(DICT_COMMAND, CMD_BY_NAME, "Nextranet-AAA-Auth-Request", &ogs_diam_nextranet_aaa_cmd_auth);
    CHECK_dict_search(DICT_COMMAND, CMD_BY_NAME, "Nextranet-AAA-Auth-Answer", &ogs_diam_nextranet_aaa_cmd_auth_answer);

    CHECK_dict_search(DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Nextranet-AVP-Result", &ogs_diam_nextranet_aaa_result);
    CHECK_dict_search(DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Nextranet-AVP-IMSI", &ogs_diam_nextranet_aaa_imsi);
    CHECK_dict_search(DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Nextranet-AVP-IMEI", &ogs_diam_nextranet_aaa_imei);
    CHECK_dict_search(DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Nextranet-AVP-APN", &ogs_diam_nextranet_aaa_apn);
    CHECK_dict_search(DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Nextranet-AVP-TAC", &ogs_diam_nextranet_aaa_tac);
    CHECK_dict_search(DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Nextranet-AVP-CGI", &ogs_diam_nextranet_aaa_cgi);
    CHECK_dict_search(DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Nextranet-AVP-EUCGI", &ogs_diam_nextranet_aaa_eucgi);

    return 0;
} 
