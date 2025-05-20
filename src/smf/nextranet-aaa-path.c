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

int smf_nextranet_aaa_init(void)
{
    int rv;

    rv = ogs_dict_nextranet_aaa_entry(NULL);
    ogs_assert(rv == 0);
    
    rv = ogs_diam_nextranet_aaa_init();
    ogs_assert(rv == 0);

    /* Register the Nextranet AAA application with FreeDiameter */
    rv = fd_disp_app_support(ogs_diam_nextranet_aaa_application, NULL, 1, 0);
    ogs_assert(rv == 0);

    return OGS_OK;
}

void smf_nextranet_aaa_final(void)
{
} 
