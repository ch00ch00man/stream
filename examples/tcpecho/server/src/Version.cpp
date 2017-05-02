// Copyright 2011 Boris Kogan (boris@thekogans.net)
//
// This file is part of libthekogans_stream.
//
// libthekogans_stream is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// libthekogans_stream is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libthekogans_stream. If not, see <http://www.gnu.org/licenses/>.

#include "thekogans/tcpecho/server/Version.h"

namespace thekogans {
    namespace stream {
        namespace tcpecho {
            namespace server {

                const util::Version &GetVersion () {
                    static const thekogans::util::Version version (
                        THEKOGANS_TCPECHO_SERVER_MAJOR_VERSION,
                        THEKOGANS_TCPECHO_SERVER_MINOR_VERSION,
                        THEKOGANS_TCPECHO_SERVER_PATCH_VERSION);
                    return version;
                }

            } // namespace server
        } // namespace tcpecho
    } // namespace stream
} // namespace thekogans
