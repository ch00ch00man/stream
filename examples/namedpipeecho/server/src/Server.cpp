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

#if defined (TOOLCHAIN_OS_Windows)

#include <list>
#include "thekogans/util/Exception.h"
#include "thekogans/util/LoggerMgr.h"
#include "thekogans/stream/ServerNamedPipe.h"
#include "thekogans/stream/namedpipeecho/server/Server.h"

namespace thekogans {
    namespace stream {
        namespace namedpipeecho {
            namespace server {

                void Server::Start (
                        const std::list<stream::Address> &addresses,
                        util::i32 priority) {
                    if (done) {
                        if (!addresses.empty ()) {
                            eventQueue.Reset (new stream::AsyncIoEventQueue);
                            for (std::list<stream::Address>::const_iterator
                                    it = addresses.begin (),
                                    end = addresses.end (); it != end; ++it) {
                                eventQueue->AddStream (
                                    *stream::ServerNamedPipe::SharedPtr (
                                        new stream::ServerNamedPipe (*it)),
                                    *this);
                                THEKOGANS_UTIL_LOG_INFO ("Listening on: %s\n",
                                    (*it).AddrToString ().c_str ());
                            }
                            done = false;
                            Create (priority);
                        }
                        else {
                            THEKOGANS_UTIL_THROW_STRING_EXCEPTION (
                                "%s", "Must supply at least one address to listen on.");
                        }
                    }
                    else {
                        THEKOGANS_UTIL_LOG_WARNING (
                            "%s\n", "Server is already running.");
                    }
                }

                void Server::Stop () {
                    if (!done) {
                        done = true;
                        jobQueue.Stop ();
                        eventQueue->Break ();
                        Wait ();
                        eventQueue.Reset ();
                    }
                    else {
                        THEKOGANS_UTIL_LOG_WARNING (
                            "%s\n", "Server is not running.");
                    }
                }

                void Server::Run () throw () {
                    while (!done) {
                        THEKOGANS_UTIL_TRY {
                            eventQueue->WaitForEvents ();
                        }
                        THEKOGANS_UTIL_CATCH_AND_LOG
                    }
                    THEKOGANS_UTIL_LOG_INFO (
                        "%s\n", "Server thread is exiting.");
                }

                void Server::HandleStreamError (
                        stream::Stream &stream,
                        const util::Exception &exception) throw () {
                    THEKOGANS_UTIL_LOG_ERROR ("%s\n", exception.Report ().c_str ());
                    THEKOGANS_UTIL_TRY {
                        eventQueue->DeleteStream (stream);
                    }
                    THEKOGANS_UTIL_CATCH_AND_LOG
                }

                void Server::HandleServerNamedPipeConnection (
                        stream::ServerNamedPipe &serverNamedPipe) throw () {
                    THEKOGANS_UTIL_LOG_INFO ("%s\n", "Received connection request.");
                    THEKOGANS_UTIL_TRY {
                        stream::ServerNamedPipe::SharedPtr newServerNamedPipe = serverNamedPipe.Clone ();
                        eventQueue->AddStream (*newServerNamedPipe, *this);
                    }
                    THEKOGANS_UTIL_CATCH_AND_LOG
                }

                void Server::HandleStreamDisconnect (stream::Stream &stream) throw () {
                    THEKOGANS_UTIL_LOG_INFO ("%s\n", "Connection closed.");
                    THEKOGANS_UTIL_TRY {
                        eventQueue->DeleteStream (stream);
                    }
                    THEKOGANS_UTIL_CATCH_AND_LOG
                }

                void Server::HandleStreamRead (
                        stream::Stream &stream,
                        util::Buffer buffer) throw () {
                    THEKOGANS_UTIL_TRY {
                        if (!buffer.IsEmpty ()) {
                            struct WriteJob : public util::RunLoop::Job {
                                stream::Stream::SharedPtr stream;
                                util::Buffer buffer;
                                WriteJob (
                                    stream::Stream &stream_,
                                    util::Buffer buffer_) :
                                    stream (&stream_),
                                    buffer (std::move (buffer_)) {}
                                // util::RunLoop::Job
                                virtual void Execute (const std::atomic<bool> &done) throw () {
                                    if (!ShouldStop (done)) {
                                        THEKOGANS_UTIL_TRY {
                                            stream->WriteBuffer (std::move (buffer));
                                        }
                                        THEKOGANS_UTIL_CATCH_AND_LOG
                                    }
                                }
                            };
                            jobQueue.EnqJob (
                                util::RunLoop::Job::SharedPtr (
                                    new WriteJob (stream, std::move (buffer))));
                        }
                    }
                    THEKOGANS_UTIL_CATCH_AND_LOG
                }

            } // namespace server
        } // namespace namedpipeecho
    } // namespace stream
} // namespace thekogans

#endif // defined (TOOLCHAIN_OS_Windows)
