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

#include "fd-path.h"
#include "ogs-diameter-nextranet-aaa.h"
#include "nextranet-aaa-path.h"

static struct session_handler *nextranet_aaa_hdl = NULL;
static struct disp_hdl *hdl_nextranet_aaa_fb = NULL;

/* Session state for Nextranet AAA sessions */
struct sess_state {
    os0_t       nextranet_aaa_sid;  /* Nextranet AAA Session-Id */
    os0_t       peer_host;          /* Peer Host */
    smf_sess_t  *sess;              /* Associated SMF session */
    ogs_pool_id_t xact_id;          /* Transaction ID */
};

static OGS_POOL(sess_state_pool, struct sess_state);
static ogs_thread_mutex_t sess_state_mutex;

/* Session cleanup callback */
static void state_cleanup(struct sess_state *sess_data, os0_t sid, void *opaque)
{
    if (!sess_data) 
        return;
    
    ogs_thread_mutex_lock(&sess_state_mutex);
    ogs_pool_free(&sess_state_pool, sess_data);
    ogs_thread_mutex_unlock(&sess_state_mutex);
}

/* Create a new state */
static struct sess_state *new_state(os0_t sid)
{
    struct sess_state *new = NULL;
    
    ogs_thread_mutex_lock(&sess_state_mutex);
    ogs_pool_alloc(&sess_state_pool, &new);
    ogs_thread_mutex_unlock(&sess_state_mutex);
    
    if (!new) {
        ogs_error("New state allocation failed");
        return NULL;
    }
    
    memset(new, 0, sizeof(*new));
    
    /* Set the Session ID */
    new->nextranet_aaa_sid = sid;
    
    return new;
}

/* Callback for incoming Nextranet-AAA-Auth-Answer messages */
static int smf_nextranet_aaa_auth_cb(struct msg **msg, struct avp *avp, 
                                 struct session *sess, void *opaque,
                                 enum disp_action *act)
{
    int ret = 0;
    uint32_t result_code = 0;
    struct sess_state *sess_data = NULL;
    struct msg *req = NULL;
    struct avp_hdr *avp_hdr = NULL;
    smf_sess_t *smf_sess = NULL;
    smf_ue_t *smf_ue = NULL;
    smf_event_t *e = NULL;
    
    ogs_debug("[NEXTRANET-AAA] Auth Answer received");
    
    /* Get session state from the session */
    ret = fd_sess_state_retrieve(nextranet_aaa_hdl, sess, &sess_data);
    if (!ret && !sess_data) {
        ogs_error("[NEXTRANET-AAA] No Session State retrieved");
        goto out;
    }
    
    if (sess_data->sess) {
        smf_sess = sess_data->sess;
        smf_ue = smf_ue_find_by_id(smf_sess->smf_ue_id);
        if (smf_ue) {
            ogs_info("[NEXTRANET-AAA] Processing Auth-Answer for IMSI:%s", smf_ue->imsi_bcd);
        }
    }
    
    /* Get the request message from the session */
    ret = fd_msg_answ_getq(*msg, &req);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] Failed to get request from answer");
        goto out;
    }
    
    /* Find Result-Code AVP */
    ret = fd_msg_search_avp(*msg, ogs_diam_nextranet_aaa_result, &avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] Failed to find Result-Code AVP");
        goto out;
    }
    
    if (avp) {
        ret = fd_msg_avp_hdr(avp, &avp_hdr);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] Failed to get Result-Code AVP header");
            goto out;
        }
        
        result_code = avp_hdr->avp_value->u32;
        ogs_info("[NEXTRANET-AAA] Result code: %u", result_code);
        
        /* If authentication was successful */
        if (result_code == 0) {
            ogs_info("[NEXTRANET-AAA] Authentication successful for IMSI:%s", 
                    smf_ue ? smf_ue->imsi_bcd : "unknown");
            
            /* Continue with session establishment */
            if (smf_sess)
                smf_sess->nextranet_auth_success = true;
        } else {
            ogs_error("[NEXTRANET-AAA] Authentication failed: %u for IMSI:%s", 
                    result_code, smf_ue ? smf_ue->imsi_bcd : "unknown");
            
            /* Mark as authentication failed and terminate session */
            if (smf_sess) {
                smf_sess->nextranet_auth_success = false;
            }
        }
        
        /* Emit event to notify state machine of authentication result */
        if (smf_sess) {
            e = smf_event_new(SMF_EVT_NEXTRANET_AAA_MESSAGE);
            ogs_assert(e);
            e->sess_id = smf_sess->id;
            
            /* Pass OGS_INVALID_POOL_ID for transaction ID - the state machine already has it */
            e->gtp_xact_id = OGS_INVALID_POOL_ID;
            
            ogs_info("[NEXTRANET-AAA-DEBUG] Sending AAA authentication event for session ID:%d, auth_success:%d", 
                     e->sess_id, smf_sess->nextranet_auth_success);
            smf_event_send(e);
        }
    }
    
out:
    if (sess_data) {
        /* Store session state or cleanup if the session is terminating */
        ret = fd_sess_state_store(nextranet_aaa_hdl, sess, &sess_data);
        ogs_assert(ret == 0);
    }
        
    /* Free the answer message */
    fd_msg_free(*msg);
    *msg = NULL;
    
    return 0;
}

/* Handler for Nextranet-AAA-Auth-Answer error messages */
static int smf_nextranet_aaa_auth_err_cb(struct msg **msg, struct avp *avp,
                                     struct session *sess, void *opaque,
                                     enum disp_action *act)
{
    ogs_error("[NEXTRANET-AAA] Error in Nextranet-AAA-Auth-Answer");
    
    struct sess_state *sess_data = NULL;
    int ret;
    smf_sess_t *smf_sess = NULL;
    smf_ue_t *smf_ue = NULL;
    smf_event_t *e = NULL;
    
    /* Get session state */
    ret = fd_sess_state_retrieve(nextranet_aaa_hdl, sess, &sess_data);
    if (ret == 0 && sess_data) {
        if (sess_data->sess) {
            smf_sess = sess_data->sess;
            smf_ue = smf_ue_find_by_id(smf_sess->smf_ue_id);
            if (smf_ue) {
                ogs_error("[NEXTRANET-AAA] Auth error for IMSI:%s", smf_ue->imsi_bcd);
            }
            /* Mark authentication as failed */
            smf_sess->nextranet_auth_success = false;
            
            /* Emit event to notify state machine of authentication failure */
            e = smf_event_new(SMF_EVT_NEXTRANET_AAA_MESSAGE);
            ogs_assert(e);
            e->sess_id = smf_sess->id;
            
            /* Pass OGS_INVALID_POOL_ID for transaction ID - the state machine already has it */
            e->gtp_xact_id = OGS_INVALID_POOL_ID;
            
            ogs_info("[NEXTRANET-AAA-DEBUG] Sending AAA authentication failure event for session ID:%d", e->sess_id);
            smf_event_send(e);
        }
        
        /* Store session state */
        ret = fd_sess_state_store(nextranet_aaa_hdl, sess, &sess_data);
        ogs_assert(ret == 0);
    }
    
    /* Free the message */
    fd_msg_free(*msg);
    *msg = NULL;
    
    return 0;
}

/* Function to send Nextranet-AAA-Auth-Request */
int smf_nextranet_aaa_send_auth_request(smf_sess_t *sess)
{
    int ret = 0;
    struct msg *req = NULL;
    struct avp *avp;
    union avp_value val;
    struct sess_state *sess_data = NULL;
    struct session *session = NULL;
    smf_ue_t *smf_ue = NULL;
    
    /* Check for session */
    if (!sess) {
        ogs_error("[NEXTRANET-AAA-DEBUG] No session provided");
        return -1;
    }
    
    /* Find UE context - don't crash if not found */
    smf_ue = smf_ue_find_by_id(sess->smf_ue_id);
    if (!smf_ue) {
        ogs_error("[NEXTRANET-AAA-DEBUG] Cannot find UE for session (ID:%d)", sess->index);
        return -1;
    }
    
    ogs_info("[NEXTRANET-AAA-DEBUG] *** CALLED Auth-Request for IMSI:%s, host='%s' ***", 
             smf_ue->imsi_bcd, sess->nextranet_aaa_host ? sess->nextranet_aaa_host : "NULL");
    
    /* Check if host is configured - we need this to proceed */
    if (!sess->nextranet_aaa_host) {
        ogs_error("[NEXTRANET-AAA-DEBUG] No Nextranet AAA host configured for session");
        return -1;
    }
    
    ogs_info("[NEXTRANET-AAA-DEBUG] Creating new Diameter message");
    ogs_info("[NEXTRANET-AAA-DEBUG] Using command at address: %p", ogs_diam_nextranet_aaa_cmd_auth);
    ret = fd_msg_new(ogs_diam_nextranet_aaa_cmd_auth, MSGFL_ALLOC_ETEID, &req);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA-DEBUG] fd_msg_new failed: %d", ret);
        goto out;
    }
    ogs_info("[NEXTRANET-AAA-DEBUG] Created Diameter message successfully");
    
    /* Create a new session */
    ogs_info("[NEXTRANET-AAA-DEBUG] Creating new Diameter session");
    ret = fd_sess_new(&session, 
                    fd_g_config->cnf_diamid, 
                    fd_g_config->cnf_diamid_len,
                    (os0_t)"smf-nextranet-aaa", 17);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA-DEBUG] fd_sess_new failed: %d", ret);
        goto out;
    }
    ogs_info("[NEXTRANET-AAA-DEBUG] Created Diameter session successfully");
    
    /* Store session in the request */
    ret = fd_msg_sess_set(req, session);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_sess_set failed: %d", ret);
        goto out;
    }
    
    /* Add Session-Id AVP */
    {
        os0_t sid;
        size_t sidlen;
        
        ret = fd_sess_getsid(session, &sid, &sidlen);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_sess_getsid failed: %d", ret);
            goto out;
        }
        
        ogs_info("[NEXTRANET-AAA] Created session ID: %.*s", (int)sidlen, (char*)sid);
        
        ret = fd_msg_avp_new(ogs_diam_session_id, 0, &avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_new failed: %d", ret);
            goto out;
        }
        
        val.os.data = sid;
        val.os.len = sidlen;
        ret = fd_msg_avp_setvalue(avp, &val);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_setvalue failed: %d", ret);
            goto out;
        }
        
        ret = fd_msg_avp_add(req, MSG_BRW_FIRST_CHILD, avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_add failed: %d", ret);
            goto out;
        }
    }
    
    /* Add Origin-Host, Origin-Realm */
    ret = fd_msg_add_origin(req, 0);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_add_origin failed: %d", ret);
        goto out;
    }
    
    /* Add Destination-Host AVP */
    if (sess->nextranet_aaa_host) {
        ogs_info("[NEXTRANET-AAA-DEBUG] Setting Destination-Host to: %s", sess->nextranet_aaa_host);
        ret = fd_msg_avp_new(ogs_diam_destination_host, 0, &avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA-DEBUG] fd_msg_avp_new failed for Destination-Host: %d", ret);
            goto out;
        }
        
        val.os.data = (uint8_t *)sess->nextranet_aaa_host;
        val.os.len = strlen(sess->nextranet_aaa_host);
        ret = fd_msg_avp_setvalue(avp, &val);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA-DEBUG] fd_msg_avp_setvalue failed for Destination-Host: %d", ret);
            goto out;
        }
        
        ret = fd_msg_avp_add(req, MSG_BRW_LAST_CHILD, avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA-DEBUG] fd_msg_avp_add failed for Destination-Host: %d", ret);
            goto out;
        }
        
        ogs_info("[NEXTRANET-AAA-DEBUG] Destination-Host AVP added successfully");
    } else {
        ogs_error("[NEXTRANET-AAA-DEBUG] No Destination-Host configured");
    }
    
    /* Add Destination-Realm */
    ret = fd_msg_avp_new(ogs_diam_destination_realm, 0, &avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_new failed: %d", ret);
        goto out;
    }
    
    val.os.data = (unsigned char *)(fd_g_config->cnf_diamrlm);
    val.os.len = strlen((char *)fd_g_config->cnf_diamrlm);
    ret = fd_msg_avp_setvalue(avp, &val);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_setvalue failed: %d", ret);
        goto out;
    }
    
    ret = fd_msg_avp_add(req, MSG_BRW_LAST_CHILD, avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_add failed: %d", ret);
        goto out;
    }
    
    /* Add Auth-Application-Id AVP */
    ret = fd_msg_avp_new(ogs_diam_auth_application_id, 0, &avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_new failed: %d", ret);
        goto out;
    }
    
    val.i32 = OGS_DIAM_NEXTRANET_AAA_APPLICATION_ID;
    ret = fd_msg_avp_setvalue(avp, &val);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_setvalue failed: %d", ret);
        goto out;
    }
    
    ret = fd_msg_avp_add(req, MSG_BRW_LAST_CHILD, avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_add failed: %d", ret);
        goto out;
    }
    
    /* Add Nextranet-AVP-IMSI AVP */
    ret = fd_msg_avp_new(ogs_diam_nextranet_aaa_imsi, 0, &avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_new failed: %d", ret);
        goto out;
    }
    
    val.os.data = (uint8_t *)smf_ue->imsi_bcd;
    val.os.len = strlen(smf_ue->imsi_bcd);
    ret = fd_msg_avp_setvalue(avp, &val);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_setvalue failed: %d", ret);
        goto out;
    }
    
    ret = fd_msg_avp_add(req, MSG_BRW_LAST_CHILD, avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_add failed: %d", ret);
        goto out;
    }
    
    /* Add Nextranet-AVP-APN AVP */
    if (sess->session.name) {
        ret = fd_msg_avp_new(ogs_diam_nextranet_aaa_apn, 0, &avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_new failed: %d", ret);
            goto out;
        }
        
        val.os.data = (uint8_t *)sess->session.name;
        val.os.len = strlen(sess->session.name);
        ret = fd_msg_avp_setvalue(avp, &val);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_setvalue failed: %d", ret);
            goto out;
        }
        
        ret = fd_msg_avp_add(req, MSG_BRW_LAST_CHILD, avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_add failed: %d", ret);
            goto out;
        }
        
        ogs_info("[NEXTRANET-AAA] Adding APN: %s", sess->session.name);
    }
    
    /* Add Nextranet-AVP-IMEI AVP if available */
    if (smf_ue->imeisv_bcd[0] != '\0') {
        ret = fd_msg_avp_new(ogs_diam_nextranet_aaa_imei, 0, &avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_new failed: %d", ret);
            goto out;
        }
        
        val.os.data = (uint8_t *)smf_ue->imeisv_bcd;
        val.os.len = strlen(smf_ue->imeisv_bcd);
        ret = fd_msg_avp_setvalue(avp, &val);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_setvalue failed: %d", ret);
            goto out;
        }
        
        ret = fd_msg_avp_add(req, MSG_BRW_LAST_CHILD, avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_add failed: %d", ret);
            goto out;
        }
        
        ogs_info("[NEXTRANET-AAA] Adding IMEI: %s", smf_ue->imeisv_bcd);
    }
    
    /* Get/create session state */
    ret = fd_sess_state_retrieve(nextranet_aaa_hdl, session, &sess_data);
    if (!ret && !sess_data) {
        /* No existing state, create a new one */
        sess_data = new_state(NULL);
        ogs_assert(sess_data);
        
        /* We'll get the GTP transaction ID from our caller when the AAA response arrives */
        sess_data->xact_id = OGS_INVALID_POOL_ID;
        
        sess_data->sess = sess;
        
        ogs_info("[NEXTRANET-AAA] Allocate new Nextranet AAA session");
    }
    
    /* Save state */
    ret = fd_sess_state_store(nextranet_aaa_hdl, session, &sess_data);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_sess_state_store failed: %d", ret);
        /* Proceed anyway... */
    }
    
    /* Send the request */
    ogs_info("[NEXTRANET-AAA-DEBUG] Sending Auth-Request message to AAA server");
    ret = fd_msg_send(&req, NULL, NULL);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA-DEBUG] fd_msg_send failed: %d", ret);
        goto out;
    }
    
    ogs_info("[NEXTRANET-AAA-DEBUG] Auth-Request sent successfully for IMSI:%s to host:%s", 
             smf_ue->imsi_bcd, sess->nextranet_aaa_host ? sess->nextranet_aaa_host : "unknown");
    
    return OGS_OK;
    
out:
    if (req)
        fd_msg_free(req);
    
    return OGS_ERROR;
}

/* Function to send Nextranet-AAA-Session-Termination-Request */
int smf_nextranet_aaa_send_term_request(smf_sess_t *sess)
{
    int ret = 0;
    struct msg *req = NULL;
    struct avp *avp;
    union avp_value val;
    struct session *session = NULL;
    smf_ue_t *smf_ue = NULL;
    
    ogs_assert(sess);
    smf_ue = smf_ue_find_by_id(sess->smf_ue_id);
    ogs_assert(smf_ue);
    
    ogs_info("[NEXTRANET-AAA] *** CALLED Term-Request for IMSI:%s ***", 
             smf_ue->imsi_bcd);
    
    /* Create the request using Auth command - safer approach to avoid segfaults */
    ret = fd_msg_new(ogs_diam_nextranet_aaa_cmd_auth, MSGFL_ALLOC_ETEID, &req);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_new with Auth command failed: %d", ret);
        goto out;
    }
    
    ogs_info("[NEXTRANET-AAA-DEBUG] Created termination request message successfully");
    
    /* Create a new session */
    ret = fd_sess_new(&session, 
                    fd_g_config->cnf_diamid, 
                    fd_g_config->cnf_diamid_len,
                    (os0_t)"smf-nextranet-aaa-term", 22);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_sess_new failed: %d", ret);
        goto out;
    }
    
    /* Store session in the request */
    ret = fd_msg_sess_set(req, session);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_sess_set failed: %d", ret);
        goto out;
    }
    
    /* Add Session-Id AVP */
    {
        os0_t sid;
        size_t sidlen;
        
        ret = fd_sess_getsid(session, &sid, &sidlen);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_sess_getsid failed: %d", ret);
            goto out;
        }
        
        ret = fd_msg_avp_new(ogs_diam_session_id, 0, &avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_new failed: %d", ret);
            goto out;
        }
        
        val.os.data = sid;
        val.os.len = sidlen;
        ret = fd_msg_avp_setvalue(avp, &val);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_setvalue failed: %d", ret);
            goto out;
        }
        
        ret = fd_msg_avp_add(req, MSG_BRW_FIRST_CHILD, avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_add failed: %d", ret);
            goto out;
        }
    }
    
    /* Add Origin-Host, Origin-Realm */
    ret = fd_msg_add_origin(req, 0);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_add_origin failed: %d", ret);
        goto out;
    }
    
    /* Add Destination-Host AVP */
    if (sess->nextranet_aaa_host) {
        ret = fd_msg_avp_new(ogs_diam_destination_host, 0, &avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_new failed: %d", ret);
            goto out;
        }
        
        val.os.data = (uint8_t *)sess->nextranet_aaa_host;
        val.os.len = strlen(sess->nextranet_aaa_host);
        ret = fd_msg_avp_setvalue(avp, &val);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_setvalue failed: %d", ret);
            goto out;
        }
        
        ret = fd_msg_avp_add(req, MSG_BRW_LAST_CHILD, avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_add failed: %d", ret);
            goto out;
        }
    }
    
    /* Add Destination-Realm */
    ret = fd_msg_avp_new(ogs_diam_destination_realm, 0, &avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_new failed: %d", ret);
        goto out;
    }
    
    val.os.data = (unsigned char *)(fd_g_config->cnf_diamrlm);
    val.os.len = strlen((char *)fd_g_config->cnf_diamrlm);
    ret = fd_msg_avp_setvalue(avp, &val);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_setvalue failed: %d", ret);
        goto out;
    }
    
    ret = fd_msg_avp_add(req, MSG_BRW_LAST_CHILD, avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_add failed: %d", ret);
        goto out;
    }
    
    /* Add Auth-Application-Id AVP */
    ret = fd_msg_avp_new(ogs_diam_auth_application_id, 0, &avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_new failed: %d", ret);
        goto out;
    }
    
    val.i32 = OGS_DIAM_NEXTRANET_AAA_APPLICATION_ID;
    ret = fd_msg_avp_setvalue(avp, &val);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_setvalue failed: %d", ret);
        goto out;
    }
    
    ret = fd_msg_avp_add(req, MSG_BRW_LAST_CHILD, avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_add failed: %d", ret);
        goto out;
    }
    
    /* Add Nextranet-AVP-IMSI AVP */
    ret = fd_msg_avp_new(ogs_diam_nextranet_aaa_imsi, 0, &avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_new failed: %d", ret);
        goto out;
    }
    
    val.os.data = (uint8_t *)smf_ue->imsi_bcd;
    val.os.len = strlen(smf_ue->imsi_bcd);
    ret = fd_msg_avp_setvalue(avp, &val);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_setvalue failed: %d", ret);
        goto out;
    }
    
    ret = fd_msg_avp_add(req, MSG_BRW_LAST_CHILD, avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_add failed: %d", ret);
        goto out;
    }
    
    /* Add Nextranet-AVP-APN AVP */
    if (sess->session.name) {
        ret = fd_msg_avp_new(ogs_diam_nextranet_aaa_apn, 0, &avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_new failed: %d", ret);
            goto out;
        }
        
        val.os.data = (uint8_t *)sess->session.name;
        val.os.len = strlen(sess->session.name);
        ret = fd_msg_avp_setvalue(avp, &val);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_setvalue failed: %d", ret);
            goto out;
        }
        
        ret = fd_msg_avp_add(req, MSG_BRW_LAST_CHILD, avp);
        if (ret != 0) {
            ogs_error("[NEXTRANET-AAA] fd_msg_avp_add failed: %d", ret);
            goto out;
        }
    }
    
    /* Add Termination-Cause AVP */
    ret = fd_msg_avp_new(ogs_diam_termination_cause, 0, &avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_new failed: %d", ret);
        goto out;
    }
    
    /* Use DIAMETER_LOGOUT as termination cause */
    val.i32 = OGS_DIAM_TERMINATION_CAUSE_DIAMETER_LOGOUT;
    ret = fd_msg_avp_setvalue(avp, &val);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_setvalue failed: %d", ret);
        goto out;
    }
    
    ret = fd_msg_avp_add(req, MSG_BRW_LAST_CHILD, avp);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_avp_add failed: %d", ret);
        goto out;
    }
    
    /* We don't need to create or store session state for termination requests,
     * as we don't expect or process the response for these messages */
    
    /* Send the request */
    ogs_info("[NEXTRANET-AAA] Sending Term-Request message to AAA server");
    ret = fd_msg_send(&req, NULL, NULL);
    if (ret != 0) {
        ogs_error("[NEXTRANET-AAA] fd_msg_send failed: %d", ret);
        goto out;
    }
    
    ogs_info("[NEXTRANET-AAA] Term-Request sent successfully for IMSI:%s", smf_ue->imsi_bcd);
    
    return OGS_OK;
    
out:
    if (req)
        fd_msg_free(req);
    
    return OGS_ERROR;
}

int smf_nextranet_aaa_init(void)
{
    int rv;
    struct disp_when data;

    ogs_info("[NEXTRANET-AAA] Initializing Nextranet AAA interface");
    
    ogs_thread_mutex_init(&sess_state_mutex);
    ogs_pool_init(&sess_state_pool, ogs_app()->pool.sess);

    /* Initialize dictionary */
    ogs_info("[NEXTRANET-AAA] Loading Nextranet AAA dictionary");
    rv = ogs_dict_nextranet_aaa_entry(NULL);
    if (rv != 0) {
        ogs_error("Failed to initialize Nextranet AAA dictionary");
        return OGS_ERROR;
    }
    ogs_info("[NEXTRANET-AAA] Dictionary loaded successfully");
    
    /* Initialize diameter objects */
    ogs_info("[NEXTRANET-AAA] Initializing Nextranet AAA diameter objects");
    rv = ogs_diam_nextranet_aaa_init();
    if (rv != 0) {
        ogs_error("Failed to initialize Nextranet AAA diameter objects");
        return OGS_ERROR;
    }
    ogs_info("[NEXTRANET-AAA] Diameter objects initialized successfully");

    /* Debug command objects addresses */
    ogs_info("[NEXTRANET-AAA] Auth command address: %p", ogs_diam_nextranet_aaa_cmd_auth);
    ogs_info("[NEXTRANET-AAA] Auth answer address: %p", ogs_diam_nextranet_aaa_cmd_auth_answer);
    ogs_info("[NEXTRANET-AAA] Term command address: %p", ogs_diam_nextranet_aaa_cmd_term);
    ogs_info("[NEXTRANET-AAA] Term answer address: %p", ogs_diam_nextranet_aaa_cmd_term_answer);
    ogs_info("[NEXTRANET-AAA] Dictionary sanity check: All command objects should be non-null");
    
    /* Register the Nextranet AAA application with FreeDiameter */
    ogs_info("[NEXTRANET-AAA] Registering Nextranet AAA application with FreeDiameter");
    rv = fd_disp_app_support(ogs_diam_nextranet_aaa_application, NULL, 1, 0);
    if (rv != 0) {
        ogs_error("Failed to register Nextranet AAA application");
        return OGS_ERROR;
    }
    ogs_info("[NEXTRANET-AAA] Application registered successfully");
    
    /* Initialize session handler */
    ogs_info("[NEXTRANET-AAA] Setting up session handler");
    memset(&data, 0, sizeof(data));
    data.app = ogs_diam_nextranet_aaa_application;
    data.command = ogs_diam_nextranet_aaa_cmd_auth;
    
    /* Register callback for Nextranet-AAA-Auth-Answer */
    ogs_info("[NEXTRANET-AAA] Registering Auth-Answer callback");
    rv = fd_disp_register(smf_nextranet_aaa_auth_cb, DISP_HOW_CC, &data, NULL,
                           &hdl_nextranet_aaa_fb);
    ogs_assert(rv == 0);
    
    /* Register callback for Nextranet-AAA-Auth-Answer errors */
    ogs_info("[NEXTRANET-AAA] Registering Auth-Answer error callback");
    rv = fd_disp_register(smf_nextranet_aaa_auth_err_cb, 
                           DISP_HOW_CC, &data, NULL, NULL);
    ogs_assert(rv == 0);
    
    /* Initialize session state management */
    ogs_info("[NEXTRANET-AAA] Creating session handler");
    rv = fd_sess_handler_create(&nextranet_aaa_hdl, 
                               (void (*)(struct sess_state *, os0_t, void *))state_cleanup, NULL, NULL);
    ogs_assert(rv == 0);

    ogs_info("[NEXTRANET-AAA] Interface initialized successfully");
    return OGS_OK;
}

void smf_nextranet_aaa_final(void)
{
    int rv;
    
    if (hdl_nextranet_aaa_fb)
        fd_disp_unregister(&hdl_nextranet_aaa_fb, NULL);
    
    if (nextranet_aaa_hdl) {
        rv = fd_sess_handler_destroy(&nextranet_aaa_hdl, NULL);
        ogs_assert(rv == 0);
    }
    
    ogs_pool_final(&sess_state_pool);
    ogs_thread_mutex_destroy(&sess_state_mutex);
} 
