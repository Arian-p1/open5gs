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

#include "ogs-diameter-common.h"
#include "ogs-diameter-nextranet-aaa.h"

#define CHECK_dict_search( _type, _criteria, _what, _result )    \
    CHECK_FCT(  fd_dict_search( fd_g_config->cnf_dict, (_type), (_criteria), (_what), (_result), ENOENT) );

#define CHECK_dict_new( _type, _data, _parent, _ref )  \
    CHECK_FCT(  fd_dict_new( fd_g_config->cnf_dict, (_type), (_data), (_parent), (_ref))  );

/* Nextranet-AAA Application */
struct dict_object *ogs_diam_nextranet_aaa_application = NULL;

/* Nextranet-AAA commands */
struct dict_object *ogs_diam_nextranet_aaa_cmd_auth = NULL;
struct dict_object *ogs_diam_nextranet_aaa_cmd_auth_ans = NULL;
struct dict_object *ogs_diam_nextranet_aaa_cmd_term = NULL;
struct dict_object *ogs_diam_nextranet_aaa_cmd_term_ans = NULL;

/* Nextranet-AAA AVPs */
struct dict_object *ogs_diam_nextranet_aaa_imsi = NULL;
struct dict_object *ogs_diam_nextranet_aaa_imei = NULL;
struct dict_object *ogs_diam_nextranet_aaa_apn = NULL;
struct dict_object *ogs_diam_nextranet_aaa_result = NULL;
struct dict_object *ogs_diam_nextranet_aaa_uli = NULL;

/* This function creates the dictionary definitions directly */
int ogs_dict_nextranet_aaa_entry(char *conffile)
{
    /* Applications section */
    {
        struct dict_object * vendor;
        struct dict_vendor_data vendor_data = { OGS_DIAM_NEXTRANET_VENDOR_ID, "Nextranet" };
        CHECK_dict_new(DICT_VENDOR, &vendor_data, NULL, &vendor);
        
        /* Create our custom application */
        struct dict_application_data app_data = { OGS_DIAM_NEXTRANET_AAA_APPLICATION_ID, "Nextranet-AAA" };
        struct dict_object *app = NULL;
        CHECK_dict_new(DICT_APPLICATION, &app_data, NULL, &app);
        
        ogs_info("[NEXTRANET-AAA-DEBUG] Created vendor and application entries");
    }

    /* Commands section */
    {
        /* Nextranet-AAA-Auth-Request command */
        {
            struct dict_object * cmd;
            struct dict_cmd_data data = { 
                OGS_DIAM_NEXTRANET_AAA_CMD_CODE_AUTH, /* Code */
                "Nextranet-Authentication-Request", /* Name */
                CMD_FLAG_REQUEST, /* Fixed flags */
                CMD_FLAG_REQUEST, /* Fixed flag values */
            };
            CHECK_dict_new(DICT_COMMAND, &data, NULL, &cmd);
            ogs_info("[NEXTRANET-AAA-DEBUG] Created Auth-Request command");
        }
        
        /* Nextranet-AAA-Auth-Answer command */
        {
            struct dict_object * cmd;
            struct dict_cmd_data data = { 
                OGS_DIAM_NEXTRANET_AAA_CMD_CODE_AUTH, /* Code */
                "Nextranet-Authentication-Answer", /* Name */
                CMD_FLAG_REQUEST, /* Fixed flags */
                0, /* Fixed flag values */
            };
            CHECK_dict_new(DICT_COMMAND, &data, NULL, &cmd);
            ogs_info("[NEXTRANET-AAA-DEBUG] Created Auth-Answer command");
        }
        
        /* Nextranet-AAA-Term-Request command */
        {
            struct dict_object * cmd;
            struct dict_cmd_data data = { 
                OGS_DIAM_NEXTRANET_AAA_CMD_CODE_TERM, /* Code */
                "Nextranet-Session-Termination-Request", /* Name */
                CMD_FLAG_REQUEST, /* Fixed flags */
                CMD_FLAG_REQUEST, /* Fixed flag values */
            };
            CHECK_dict_new(DICT_COMMAND, &data, NULL, &cmd);
            ogs_info("[NEXTRANET-AAA-DEBUG] Created Term-Request command");
        }
        
        /* Nextranet-AAA-Term-Answer command */
        {
            struct dict_object * cmd;
            struct dict_cmd_data data = { 
                OGS_DIAM_NEXTRANET_AAA_CMD_CODE_TERM, /* Code */
                "Nextranet-Session-Termination-Answer", /* Name */
                CMD_FLAG_REQUEST, /* Fixed flags */
                0, /* Fixed flag values */
            };
            CHECK_dict_new(DICT_COMMAND, &data, NULL, &cmd);
            ogs_info("[NEXTRANET-AAA-DEBUG] Created Term-Answer command");
        }
    }

    /* Nextranet specific AVP section */
    {
        /* Nextranet-AVP-Result */
        {
            struct dict_avp_data data = {
                OGS_DIAM_NEXTRANET_AVP_RESULT_CODE, /* Code */
                OGS_DIAM_NEXTRANET_VENDOR_ID,       /* Vendor ID */
                "Nextranet-AVP-Result", /* Name */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
                AVP_TYPE_UNSIGNED32 /* base type of data */
            };
            CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
            ogs_info("[NEXTRANET-AAA-DEBUG] Created Result AVP");
        }

        /* Nextranet-AVP-IMSI */
        {
            struct dict_avp_data data = {
                OGS_DIAM_NEXTRANET_AVP_IMSI, /* Code */
                OGS_DIAM_NEXTRANET_VENDOR_ID, /* Vendor ID */
                "Nextranet-AVP-IMSI", /* Name */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
                AVP_TYPE_OCTETSTRING /* base type of data */
            };
            CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
            ogs_info("[NEXTRANET-AAA-DEBUG] Created IMSI AVP");
        }

        /* Nextranet-AVP-IMEI */
        {
            struct dict_avp_data data = {
                OGS_DIAM_NEXTRANET_AVP_IMEI, /* Code */
                OGS_DIAM_NEXTRANET_VENDOR_ID, /* Vendor ID */
                "Nextranet-AVP-IMEI", /* Name */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
                AVP_TYPE_OCTETSTRING /* base type of data */
            };
            CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
            ogs_info("[NEXTRANET-AAA-DEBUG] Created IMEI AVP");
        }

        /* Nextranet-AVP-APN */
        {
            struct dict_avp_data data = {
                OGS_DIAM_NEXTRANET_AVP_APN, /* Code */
                OGS_DIAM_NEXTRANET_VENDOR_ID, /* Vendor ID */
                "Nextranet-AVP-APN", /* Name */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
                AVP_TYPE_OCTETSTRING /* base type of data */
            };
            CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
            ogs_info("[NEXTRANET-AAA-DEBUG] Created APN AVP");
        }

        /* Nextranet-AVP-TAC */
        {
            struct dict_avp_data data = {
                OGS_DIAM_NEXTRANET_AVP_TAC, /* Code */
                OGS_DIAM_NEXTRANET_VENDOR_ID, /* Vendor ID */
                "Nextranet-AVP-TAC", /* Name */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
                AVP_TYPE_OCTETSTRING /* base type of data */
            };
            CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
            ogs_info("[NEXTRANET-AAA-DEBUG] Created TAC AVP");
        }

        /* Nextranet-AVP-CGI */
        {
            struct dict_avp_data data = {
                OGS_DIAM_NEXTRANET_AVP_CGI, /* Code */
                OGS_DIAM_NEXTRANET_VENDOR_ID, /* Vendor ID */
                "Nextranet-AVP-CGI", /* Name */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
                AVP_TYPE_OCTETSTRING /* base type of data */
            };
            CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
            ogs_info("[NEXTRANET-AAA-DEBUG] Created CGI AVP");
        }

        /* Nextranet-AVP-EUCGI */
        {
            struct dict_avp_data data = {
                OGS_DIAM_NEXTRANET_AVP_EUCGI, /* Code */
                OGS_DIAM_NEXTRANET_VENDOR_ID, /* Vendor ID */
                "Nextranet-AVP-EUCGI", /* Name */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
                AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
                AVP_TYPE_OCTETSTRING /* base type of data */
            };
            CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
            ogs_info("[NEXTRANET-AAA-DEBUG] Created EUCGI AVP");
        }
    }

    ogs_info("[NEXTRANET-AAA-DEBUG] All dictionary entries created successfully");
    return 0;
}

int ogs_diam_nextranet_aaa_init(void)
{
    application_id_t id = OGS_DIAM_NEXTRANET_AAA_APPLICATION_ID;
    struct dict_object *vendor;
    
    ogs_assert(ogs_diam_nextranet_aaa_application == NULL);
    
    /* Initialize dictionary entries directly */
    ogs_info("[NEXTRANET-AAA-DEBUG] Initializing Nextranet AAA dictionary entries directly");
    int rv = ogs_dict_nextranet_aaa_entry(NULL);
    if (rv != 0) {
        ogs_error("[NEXTRANET-AAA-DEBUG] Failed to initialize dictionary entries: %d", rv);
        return rv;
    }
    
    /* Find Vendor */
    ogs_info("[NEXTRANET-AAA-DEBUG] Looking up Nextranet vendor");
    CHECK_dict_search(DICT_VENDOR, VENDOR_BY_NAME, 
                    "Nextranet", &vendor);
    
    /* Find Application */
    ogs_info("[NEXTRANET-AAA-DEBUG] Looking up Nextranet-AAA application");
    CHECK_dict_search(DICT_APPLICATION, APPLICATION_BY_ID, &id, 
                     &ogs_diam_nextranet_aaa_application);
    ogs_assert(ogs_diam_nextranet_aaa_application);
    
    /* Find Commands */
    ogs_info("[NEXTRANET-AAA-DEBUG] Looking up Auth-Request command");
    CHECK_dict_search(DICT_COMMAND, CMD_BY_NAME, 
                     "Nextranet-Authentication-Request", &ogs_diam_nextranet_aaa_cmd_auth);
    
    ogs_info("[NEXTRANET-AAA-DEBUG] Looking up Term-Request command");
    CHECK_dict_search(DICT_COMMAND, CMD_BY_NAME, 
                     "Nextranet-Session-Termination-Request", &ogs_diam_nextranet_aaa_cmd_term);
    
    /* Find AVPs */
    ogs_info("[NEXTRANET-AAA-DEBUG] Looking up AVPs");
    CHECK_dict_search(DICT_AVP, AVP_BY_NAME, 
                     "Nextranet-AVP-IMSI", &ogs_diam_nextranet_aaa_imsi);
    CHECK_dict_search(DICT_AVP, AVP_BY_NAME, 
                     "Nextranet-AVP-IMEI", &ogs_diam_nextranet_aaa_imei);
    CHECK_dict_search(DICT_AVP, AVP_BY_NAME, 
                     "Nextranet-AVP-APN", &ogs_diam_nextranet_aaa_apn);
    CHECK_dict_search(DICT_AVP, AVP_BY_NAME, 
                     "Nextranet-AVP-Result", &ogs_diam_nextranet_aaa_result);
    CHECK_dict_search(DICT_AVP, AVP_BY_NAME, 
                     "Nextranet-AVP-ULI", &ogs_diam_nextranet_aaa_uli);
    
    ogs_info("[NEXTRANET-AAA-DEBUG] Nextranet AAA initialization completed successfully");
    return 0;
} 