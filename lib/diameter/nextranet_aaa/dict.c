/*********************************************************************************************************
 * Software License Agreement (BSD License)                                                               *
 *                                                                                                        *
 * Copyright (c) 2024, Open5GS                                                                            *
 * All rights reserved.                                                                                   *
 *                                                                                                        *
 * Redistribution and use of this software in source and binary forms, with or without modification, are  *
 * permitted provided that the following conditions are met:                                              *
 *                                                                                                        *
 * * Redistributions of source code must retain the above                                                 *
 *   copyright notice, this list of conditions and the                                                    *
 *   following disclaimer.                                                                                *
 *                                                                                                        *
 * * Redistributions in binary form must reproduce the above                                              *
 *   copyright notice, this list of conditions and the                                                    *
 *   following disclaimer in the documentation and/or other                                               *
 *   materials provided with the distribution.                                                            *
 *                                                                                                        *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED *
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR *
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT     *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    *
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR *
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF   *
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                             *
 *********************************************************************************************************/

/*
 * Dictionary definitions for objects specified for the Nextranet AAA interface.
 */

#include <freeDiameter/extension.h>
#include "ogs-diameter-nextranet-aaa.h"

/* The content of this file follows the same structure as dict_base_proto.c */

#define CHECK_dict_new( _type, _data, _parent, _ref )  \
  CHECK_FCT(  fd_dict_new( fd_g_config->cnf_dict, (_type), (_data), (_parent), (_ref))  );

#define CHECK_dict_search( _type, _criteria, _what, _result )  \
  CHECK_FCT(  fd_dict_search( fd_g_config->cnf_dict, (_type), (_criteria), (_what), (_result), ENOENT) );

struct local_rules_definition {
  struct dict_avp_request avp_vendor_plus_name;
  enum rule_position  position;
  int       min;
  int      max;
};

#define RULE_ORDER( _position ) ((((_position) == RULE_FIXED_HEAD) || ((_position) == RULE_FIXED_TAIL)) ? 1 : 0 )

/* Attention! This version of the macro uses AVP_BY_NAME_AND_VENDOR, in contrast to most other copies! */
#define PARSE_loc_rules( _rulearray, _parent) {                \
  int __ar;                      \
  for (__ar=0; __ar < sizeof(_rulearray) / sizeof((_rulearray)[0]); __ar++) {      \
    struct dict_rule_data __data = { NULL,               \
      (_rulearray)[__ar].position,              \
      0,                     \
      (_rulearray)[__ar].min,                \
      (_rulearray)[__ar].max};              \
    __data.rule_order = RULE_ORDER(__data.rule_position);          \
    CHECK_FCT(  fd_dict_search(                 \
      fd_g_config->cnf_dict,                \
      DICT_AVP,                   \
      AVP_BY_NAME_AND_VENDOR,               \
      &(_rulearray)[__ar].avp_vendor_plus_name,          \
      &__data.rule_avp, 0 ) );              \
    if ( !__data.rule_avp ) {                \
      TRACE_DEBUG(INFO, "AVP Not found: '%s'", (_rulearray)[__ar].avp_vendor_plus_name.avp_name);    \
      return ENOENT;                  \
    }                      \
    CHECK_FCT_DO( fd_dict_new( fd_g_config->cnf_dict, DICT_RULE, &__data, _parent, NULL),  \
      {                          \
        TRACE_DEBUG(INFO, "Error on rule with AVP '%s'",            \
              (_rulearray)[__ar].avp_vendor_plus_name.avp_name);    \
        return EINVAL;                      \
      } );                          \
  }                              \
}

#define enumval_def_u32( _val_, _str_ ) \
    { _str_,     { .u32 = _val_ }}

#define enumval_def_os( _len_, _val_, _str_ ) \
    { _str_,     { .os = { .data = (unsigned char *)_val_, .len = _len_ }}}


int ogs_dict_nextranet_aaa_entry(char *conffile)
{
  struct dict_object * nextranet_vendor;
  struct dict_vendor_data vendor_data = { OGS_DIAM_NEXTRANET_VENDOR_ID, "Nextranet" };
  CHECK_FCT(fd_dict_new(fd_g_config->cnf_dict, DICT_VENDOR, &vendor_data, NULL, &nextranet_vendor));

  /* Applications section */
  {
    struct dict_application_data app_data = { OGS_DIAM_NEXTRANET_AAA_APPLICATION_ID, "Nextranet-AAA" };
    CHECK_FCT(fd_dict_new(fd_g_config->cnf_dict, DICT_APPLICATION, &app_data, nextranet_vendor, NULL));
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
        AVP_TYPE_OCTETSTRING /* base type of data */
      };
      CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
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
    }
  }

  /* Create commands */
  {
    /* Nextranet-AAA-Auth-Request (NAAR) Command */
    {
      struct dict_object * cmd;
      struct dict_cmd_data data = {
        OGS_DIAM_NEXTRANET_AAA_CMD_CODE, /* Code */
        "Nextranet-AAA-Auth-Request", /* Name */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR, /* Fixed flags */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE /* Fixed flag values */
      };
      CHECK_dict_new(DICT_COMMAND, &data, NULL, &cmd);

      /* Add AVPs to command */
      struct local_rules_definition rules[] =
      {
        {  { .avp_vendor = OGS_DIAM_NEXTRANET_VENDOR_ID, .avp_name = "Nextranet-AVP-IMSI" }, RULE_REQUIRED, -1, 1 },
        {  { .avp_vendor = OGS_DIAM_NEXTRANET_VENDOR_ID, .avp_name = "Nextranet-AVP-IMEI" }, RULE_OPTIONAL, -1, 1 },
        {  { .avp_vendor = OGS_DIAM_NEXTRANET_VENDOR_ID, .avp_name = "Nextranet-AVP-APN" }, RULE_OPTIONAL, -1, 1 },
        {  { .avp_vendor = OGS_DIAM_NEXTRANET_VENDOR_ID, .avp_name = "Nextranet-AVP-TAC" }, RULE_OPTIONAL, -1, 1 },
        {  { .avp_vendor = OGS_DIAM_NEXTRANET_VENDOR_ID, .avp_name = "Nextranet-AVP-CGI" }, RULE_OPTIONAL, -1, 1 },
        {  { .avp_vendor = OGS_DIAM_NEXTRANET_VENDOR_ID, .avp_name = "Nextranet-AVP-EUCGI" }, RULE_OPTIONAL, -1, 1 }
      };
      PARSE_loc_rules(rules, cmd);
    }

    /* Nextranet-AAA-Auth-Answer Command */
    {
      struct dict_object * cmd;
      struct dict_cmd_data data = {
        OGS_DIAM_NEXTRANET_AAA_CMD_CODE, /* Code */
        "Nextranet-AAA-Auth-Answer", /* Name */
        CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR, /* Fixed flags */
        CMD_FLAG_PROXIABLE /* Fixed flag values */
      };
      CHECK_dict_new(DICT_COMMAND, &data, NULL, &cmd);

      /* Add AVPs to command */
      struct local_rules_definition rules[] =
      {
        {  { .avp_vendor = OGS_DIAM_NEXTRANET_VENDOR_ID, .avp_name = "Nextranet-AVP-Result" }, RULE_REQUIRED, -1, 1 }
      };
      PARSE_loc_rules(rules, cmd);
    }
  }

  return 0;
} 