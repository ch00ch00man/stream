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
    #if !defined (_WINDOWS_)
        #if !defined (WIN32_LEAN_AND_MEAN)
            #define WIN32_LEAN_AND_MEAN
        #endif // !defined (WIN32_LEAN_AND_MEAN)
        #if !defined (NOMINMAX)
            #define NOMINMAX
        #endif // !defined (NOMINMAX)
        #include <windows.h>
    #endif // !defined (_WINDOWS_)
#else // defined (TOOLCHAIN_OS_Windows)
    #include <sys/types.h>
    #include <sys/ioctl.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif // defined (TOOLCHAIN_OS_Windows)
#include <cassert>
#include "thekogans/util/Exception.h"
#include "thekogans/util/LoggerMgr.h"
#include "thekogans/util/internal.h"
#include "thekogans/stream/AsyncIoEventQueue.h"
#include "thekogans/stream/AsyncIoEventSink.h"
#include "thekogans/stream/Pipe.h"

namespace thekogans {
    namespace stream {

        THEKOGANS_UTIL_IMPLEMENT_HEAP_WITH_LOCK (Pipe, util::SpinLock)

        void Pipe::Create (
                Pipe &readPipe,
                Pipe &writePipe) {
            THEKOGANS_UTIL_HANDLE handles[2] = {
                THEKOGANS_UTIL_INVALID_HANDLE_VALUE,
                THEKOGANS_UTIL_INVALID_HANDLE_VALUE
            };
            if (pipe (handles) < 0) {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE);
            }
            readPipe.handle = handles[0];
            writePipe.handle = handles[1];
        }

        util::ui32 Pipe::Read (
                void *buffer,
                util::ui32 count) {
        #if defined (TOOLCHAIN_OS_Windows)
            DWORD countRead = 0;
            if (buffer != 0 && count > 0) {
                TimedOverlapped::UniquePtr overlapped;
                if (readTimeout != util::TimeSpec::Zero) {
                    overlapped.reset (new TimedOverlapped);
                }
                if (!ReadFile (handle, buffer, count,
                        overlapped.get () == 0 ? 0 : &countRead, overlapped.get ())) {
                    THEKOGANS_UTIL_ERROR_CODE errorCode = THEKOGANS_UTIL_OS_ERROR_CODE;
                    if (errorCode == ERROR_IO_PENDING) {
                        countRead = overlapped->Wait (handle, readTimeout);
                    }
                    else {
                        THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (errorCode);
                    }
                }
                else if (overlapped.get () != 0 &&
                       !GetOverlappedResult (handle, overlapped.get (), &countRead, FALSE)) {
                    THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                        THEKOGANS_UTIL_OS_ERROR_CODE);
                }
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
            return countRead;
        #else // defined (TOOLCHAIN_OS_Windows)
            ssize_t countRead = 0;
            if (buffer != 0 && count > 0) {
                if (readTimeout != util::TimeSpec::Zero && GetDataAvailable () == 0) {
                    TimedEvent timedEvent;
                    if (!timedEvent.Wait (handle, AsyncInfo::EventRead, readTimeout)) {
                        THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                            THEKOGANS_UTIL_OS_ERROR_CODE_TIMEOUT);
                    }
                }
                countRead = read (handle, buffer, count);
                if (countRead < 0) {
                    THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                        THEKOGANS_UTIL_OS_ERROR_CODE);
                }
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
            return (util::ui32)countRead;
        #endif // defined (TOOLCHAIN_OS_Windows)
        }

        util::ui32 Pipe::Write (
                const void *buffer,
                util::ui32 count) {
        #if defined (TOOLCHAIN_OS_Windows)
            DWORD countWritten = 0;
            if (buffer != 0 && count > 0) {
                if (IsAsync ()) {
                    AsyncInfo::ReadWriteOverlapped::UniquePtr overlapped (
                        new AsyncInfo::ReadWriteOverlapped (*this, buffer, count));
                    if (!WriteFile (handle,
                            overlapped->buffer->GetReadPtr (),
                            (DWORD)overlapped->buffer->GetDataAvailableForReading (),
                            0, overlapped.get ())) {
                        THEKOGANS_UTIL_ERROR_CODE errorCode = THEKOGANS_UTIL_OS_ERROR_CODE;
                        if (errorCode != ERROR_IO_PENDING) {
                            THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (errorCode);
                        }
                    }
                    overlapped.release ();
                }
                else {
                    TimedOverlapped::UniquePtr overlapped;
                    if (writeTimeout != util::TimeSpec::Zero) {
                        overlapped.reset (new TimedOverlapped);
                    }
                    if (!WriteFile (handle, buffer, count,
                            overlapped.get () == 0 ? 0 : &countWritten, overlapped.get ())) {
                        THEKOGANS_UTIL_ERROR_CODE errorCode = THEKOGANS_UTIL_OS_ERROR_CODE;
                        if (errorCode == ERROR_IO_PENDING) {
                            countWritten = overlapped->Wait (handle, writeTimeout);
                        }
                        else {
                            THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (errorCode);
                        }
                    }
                    else if (overlapped.get () != 0 &&
                            !GetOverlappedResult (handle, overlapped.get (), &countWritten, FALSE)) {
                        THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                            THEKOGANS_UTIL_OS_ERROR_CODE);
                    }
                }
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
            return countWritten;
        #else // defined (TOOLCHAIN_OS_Windows)
            ssize_t countWritten = 0;
            if (buffer != 0 && count > 0) {
                if (IsAsync ()) {
                    asyncInfo->EnqBufferBack (
                        AsyncInfo::BufferInfo::UniquePtr (
                            new AsyncInfo::WriteBufferInfo (*this, buffer, count)));
                }
                else {
                    if (writeTimeout != util::TimeSpec::Zero) {
                        TimedEvent timedEvent;
                        if (!timedEvent.Wait (handle, AsyncInfo::EventWrite, writeTimeout)) {
                            THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                                THEKOGANS_UTIL_OS_ERROR_CODE_TIMEOUT);
                        }
                    }
                    countWritten = write (handle, buffer, count);
                    if (countWritten < 0) {
                        THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                            THEKOGANS_UTIL_OS_ERROR_CODE);
                    }
                }
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
            return (util::ui32)countWritten;
        #endif // defined (TOOLCHAIN_OS_Windows)
        }

        void Pipe::WriteBuffer (util::Buffer::UniquePtr buffer) {
            if (buffer.get () != 0) {
                if (IsAsync ()) {
                #if defined (TOOLCHAIN_OS_Windows)
                    AsyncInfo::ReadWriteOverlapped::UniquePtr overlapped (
                        new AsyncInfo::ReadWriteOverlapped (*this, std::move (buffer)));
                    if (!WriteFile (handle,
                            overlapped->buffer->GetReadPtr (),
                            (ULONG)overlapped->buffer->GetDataAvailableForReading (),
                            0, overlapped.get ())) {
                        THEKOGANS_UTIL_ERROR_CODE errorCode = THEKOGANS_UTIL_OS_ERROR_CODE;
                        if (errorCode != ERROR_IO_PENDING) {
                            THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (errorCode);
                        }
                    }
                    overlapped.release ();
                #else // defined (TOOLCHAIN_OS_Windows)
                    asyncInfo->EnqBufferBack (
                        AsyncInfo::BufferInfo::UniquePtr (
                            new AsyncInfo::WriteBufferInfo (*this, std::move (buffer))));
                #endif // defined (TOOLCHAIN_OS_Windows)
                }
                else {
                    THEKOGANS_UTIL_THROW_STRING_EXCEPTION (
                        "%s", "WriteBuffer is called on a blocking pipe.");
                }
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
        }

        void Pipe::SetReadTimeout (const util::TimeSpec &timeSpec) {
            if (timeSpec != util::TimeSpec::Infinite) {
                readTimeout = timeSpec;
                if (IsAsync ()) {
                    asyncInfo->UpdateTimedStream (AsyncInfo::EventRead);
                }
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
        }

        void Pipe::SetWriteTimeout (const util::TimeSpec &timeSpec) {
            if (timeSpec != util::TimeSpec::Infinite) {
                writeTimeout = timeSpec;
                if (IsAsync ()) {
                    asyncInfo->UpdateTimedStream (AsyncInfo::EventWrite);
                }
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
        }

        util::ui32 Pipe::GetDataAvailable () {
        #if defined (TOOLCHAIN_OS_Windows)
            DWORD totalBytesAvailable = 0;
            if (!PeekNamedPipe (handle, 0, 0, 0, &totalBytesAvailable, 0)) {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE);
            }
            return totalBytesAvailable;
        #else // defined (TOOLCHAIN_OS_Windows)
            unsigned long value = 0;
            if (ioctl (handle, FIONREAD, &value) < 0) {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE);
            }
            return (util::ui32)value;
        #endif // defined (TOOLCHAIN_OS_Windows)
        }

        void Pipe::InitAsyncIo () {
        #if defined (TOOLCHAIN_OS_Windows)
            PostAsyncRead ();
        #else // defined (TOOLCHAIN_OS_Windows)
            SetBlocking (false);
            asyncInfo->AddStreamForEvents (AsyncInfo::EventRead);
        #endif // defined (TOOLCHAIN_OS_Windows)
        }

    #if defined (TOOLCHAIN_OS_Windows)
        void Pipe::PostAsyncRead () {
            if (asyncInfo->bufferLength != 0) {
                AsyncInfo::ReadWriteOverlapped::UniquePtr overlapped (
                    new AsyncInfo::ReadWriteOverlapped (*this, asyncInfo->bufferLength));
                if (!ReadFile (handle,
                        overlapped->buffer->GetWritePtr (),
                        (DWORD)overlapped->buffer->GetDataAvailableForWriting (),
                        0, overlapped.get ())) {
                    THEKOGANS_UTIL_ERROR_CODE errorCode = THEKOGANS_UTIL_OS_ERROR_CODE;
                    if (errorCode != ERROR_IO_PENDING) {
                        THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (errorCode);
                    }
                }
                overlapped.release ();
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
        }

        void Pipe::HandleOverlapped (AsyncInfo::Overlapped &overlapped) throw () {
            if (overlapped.event == AsyncInfo::EventRead) {
                THEKOGANS_UTIL_TRY {
                    AsyncInfo::ReadWriteOverlapped &readWriteOverlapped =
                        (AsyncInfo::ReadWriteOverlapped &)overlapped;
                    if (readWriteOverlapped.buffer->GetDataAvailableForReading () != 0) {
                        PostAsyncRead ();
                        asyncInfo->eventSink.HandleStreamRead (
                            *this, std::move (readWriteOverlapped.buffer));
                    }
                    else {
                        asyncInfo->eventSink.HandleStreamDisconnect (*this);
                    }
                }
                THEKOGANS_UTIL_CATCH (util::Exception) {
                    THEKOGANS_UTIL_EXCEPTION_NOTE_LOCATION (exception);
                    asyncInfo->eventSink.HandleStreamError (*this, exception);
                }
            }
            else if (overlapped.event == AsyncInfo::EventWrite) {
                AsyncInfo::ReadWriteOverlapped &readWriteOverlapped =
                    (AsyncInfo::ReadWriteOverlapped &)overlapped;
                assert (readWriteOverlapped.buffer->GetDataAvailableForReading () == 0);
                asyncInfo->eventSink.HandleStreamWrite (
                    *this, std::move (readWriteOverlapped.buffer));
            }
        }
    #else // defined (TOOLCHAIN_OS_Windows)
        void Pipe::SetBlocking (bool blocking) {
            int flags = fcntl (handle, F_GETFL, 0);
            if (blocking) {
                flags &= ~O_NONBLOCK;
            }
            else {
                flags |= O_NONBLOCK;
            }
            if (fcntl (handle, F_SETFL, flags) < 0) {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE);
            }
        }

        void Pipe::HandleAsyncEvent (util::ui32 event) throw () {
            if (event == AsyncInfo::EventDisconnect) {
                asyncInfo->eventSink.HandleStreamDisconnect (*this);
            }
            else if (event == AsyncInfo::EventRead) {
                THEKOGANS_UTIL_TRY {
                    util::ui32 bufferLength = GetDataAvailable ();
                    if (bufferLength != 0) {
                        util::Buffer::UniquePtr buffer =
                            asyncInfo->eventSink.GetBuffer (*this, util::HostEndian, bufferLength);
                        if (buffer->AdvanceWriteOffset (
                                Read (
                                    buffer->GetWritePtr (),
                                    bufferLength)) > 0) {
                            asyncInfo->eventSink.HandleStreamRead (*this, std::move (buffer));
                        }
                    }
                    else {
                        asyncInfo->eventSink.HandleStreamDisconnect (*this);
                    }
                }
                THEKOGANS_UTIL_CATCH (util::Exception) {
                    THEKOGANS_UTIL_EXCEPTION_NOTE_LOCATION (exception);
                    asyncInfo->eventSink.HandleStreamError (*this, exception);
                }
            }
            else if (event == AsyncInfo::EventWrite) {
                asyncInfo->WriteBuffers ();
            }
        }
    #endif // defined (TOOLCHAIN_OS_Windows)

    } // namespace stream
} // namespace thekogans
