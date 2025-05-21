# Copyright (C) 2024 by Open5GS

# This file is part of Open5GS.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

libdiameter_nextranet_aaa_sources = files('''
    dict.c
    message.c
'''.split())

libdiameter_nextranet_aaa_inc = include_directories('.')

libdiameter_nextranet_aaa = static_library('diameter_nextranet_aaa',
    sources : libdiameter_nextranet_aaa_sources,
    c_args : libdiameter_common_cc_flags,
    include_directories : [libdiameter_nextranet_aaa_inc,
                          libdiameter_common_inc],
    dependencies : [libdiameter_common_dep,
                    libfdcore_dep],
    install : false)

libdiameter_nextranet_aaa_dep = declare_dependency(
    link_with : libdiameter_nextranet_aaa,
    include_directories : [libdiameter_nextranet_aaa_inc,
                          libdiameter_common_inc],
    dependencies : [libdiameter_common_dep,
                   libfdcore_dep]) 