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

#ifndef SMF_NEXTRANET_AAA_PATH_H
#define SMF_NEXTRANET_AAA_PATH_H

#include "context.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Nextranet AAA initialization and cleanup */
int smf_nextranet_aaa_init(void);
void smf_nextranet_aaa_final(void);

/* Function to send Nextranet AAA authentication request */
int smf_nextranet_aaa_send_auth_request(smf_sess_t *sess);
int smf_nextranet_aaa_send_term_request(smf_sess_t *sess);

#ifdef __cplusplus
}
#endif

#endif /* SMF_NEXTRANET_AAA_PATH_H */ 