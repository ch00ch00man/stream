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

#include <list>
#include <string>
#include "thekogans/util/CommandLineOptions.h"
#include "thekogans/util/Path.h"
#include "thekogans/util/Exception.h"
#include "thekogans/util/LoggerMgr.h"
#include "thekogans/util/RunLoop.h"
#include "thekogans/util/ConsoleLogger.h"
#include "thekogans/util/FileLogger.h"
#include "thekogans/util/MainRunLoop.h"
#include "thekogans/util/File.h"
#include "thekogans/util/Version.h"
#include "thekogans/stream/Version.h"
#include "thekogans/stream/loggermgr/Options.h"
#include "thekogans/stream/loggermgr/Version.h"
#include "thekogans/stream/loggermgr/Server.h"

using namespace thekogans;
using namespace thekogans::stream;

namespace {
    std::string GetLogLevelList (const std::string &separator) {
        std::string logLevelList;
        {
            std::list<util::ui32> levels;
            util::LoggerMgr::GetLogLevels (levels);
            if (!levels.empty ()) {
                std::list<util::ui32>::const_iterator it = levels.begin ();
                logLevelList = util::LoggerMgr::levelTostring (*it++);
                for (std::list<util::ui32>::const_iterator end = levels.end (); it != end; ++it) {
                    logLevelList += separator + util::LoggerMgr::levelTostring (*it);
                }
            }
            else {
                logLevelList = "No LoggerMgr levels defined";
            }
        }
        return logLevelList;
    }
}

int main (
        int argc,
        const char *argv[]) {
    loggermgr::Options::Instance ().Parse (argc, argv, "hvlcfrkpa");
    THEKOGANS_UTIL_LOG_INIT (
        loggermgr::Options::Instance ().loggerMgr.level,
        loggermgr::Options::Instance ().loggerMgr.decorations);
    if (loggermgr::Options::Instance ().help ||
            loggermgr::Options::Instance ().version ||
            loggermgr::Options::Instance ().loggerMgr.consoleLogger) {
        THEKOGANS_UTIL_LOG_ADD_LOGGER (
            util::Logger::Ptr (
                new util::ConsoleLogger));
    }
    if (!loggermgr::Options::Instance ().loggerMgr.fileLogger.path.empty ()) {
        THEKOGANS_UTIL_LOG_ADD_LOGGER (
            util::Logger::Ptr (
                new util::FileLogger (
                    loggermgr::Options::Instance ().loggerMgr.fileLogger.path,
                    loggermgr::Options::Instance ().loggerMgr.fileLogger.archive,
                    loggermgr::Options::Instance ().loggerMgr.fileLogger.maxLogFileSize)));
    }
    if (loggermgr::Options::Instance ().help) {
        THEKOGANS_UTIL_LOG_INFO (
            "%s [-h] [-v] [-l:'%s'] [-c] [-f:'path'] [-r[:max size]] "
            "[-k:'path'] [-p:'host port'] [-a:'host address'*]\n\n"
            "h - Display this help message.\n"
            "v - Display version information.\n"
            "l - Set logging level.\n"
            "c - Log to console.\n"
            "f - Log to file (path - Path of log file).\n"
            "r - Archive file log (max size - Max log file size before archiving).\n"
            "k - Use lock file to prevent multiple instances (path - Path to lock file).\n"
            "p - Port to listen for clients on.\n"
            "a - Address to listen for clients on (can be repeated).\n",
            argv[0],
            GetLogLevelList (" | ").c_str ());
    }
    else if (loggermgr::Options::Instance ().version) {
        THEKOGANS_UTIL_LOG_INFO (
            "libthekogans_util - %s\n"
            "libthekogans_stream - %s\n"
            "%s - %s\n",
            util::GetVersion ().ToString ().c_str (),
            stream::GetVersion ().ToString ().c_str (),
            argv[0], loggermgr::GetVersion ().ToString ().c_str ());
    }
    else {
        THEKOGANS_UTIL_TRY {
            struct LockFile {
                util::Path path;
                explicit LockFile (const std::string &path_) :
                        path (path_) {
                    if (!path.IsEmpty ()) {
                        if (!path.Exists ()) {
                            util::File::Touch (path_);
                        }
                        else {
                            THEKOGANS_UTIL_THROW_STRING_EXCEPTION (
                                "Lock file (%s) found.\n", path_.c_str ());
                        }
                    }
                }
                ~LockFile () {
                    if (path.Exists ()) {
                        path.Delete ();
                    }
                }
            } lockFile (loggermgr::Options::Instance ().lockFilePath);
            THEKOGANS_UTIL_LOG_INFO ("%s starting.\n", argv[0]);
            loggermgr::Server::Instance ().Start (
                loggermgr::Options::Instance ().addresses);
            util::MainRunLoop::Instance ().Start ();
            loggermgr::Server::Instance ().Stop ();
            THEKOGANS_UTIL_LOG_INFO ("%s exiting.\n", argv[0]);
        }
        THEKOGANS_UTIL_CATCH_AND_LOG
    }
    THEKOGANS_UTIL_LOG_FLUSH
    return 0;
}
