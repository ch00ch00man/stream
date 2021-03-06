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

#include "thekogans/stream/AsyncIoEventSink.h"

namespace thekogans {
    namespace stream {

        void AsyncIoEventSink::HandleStreamError (
                Stream &stream,
                const util::Exception &exception) throw () {
            if (next.Get () != 0) {
                next->HandleStreamError (stream, exception);
            }
        }

        void AsyncIoEventSink::HandleStreamDisconnect (Stream &stream) throw () {
            if (next.Get () != 0) {
                next->HandleStreamDisconnect (stream);
            }
        }

        void AsyncIoEventSink::HandleStreamRead (
                Stream &stream,
                util::Buffer buffer) throw () {
            if (next.Get () != 0) {
                next->HandleStreamRead (stream, std::move (buffer));
            }
        }

        void AsyncIoEventSink::HandleStreamWrite (
                Stream &stream,
                util::Buffer buffer) throw () {
            if (next.Get () != 0) {
                next->HandleStreamWrite (stream, std::move (buffer));
            }
        }

    #if defined (TOOLCHAIN_OS_Windows)
        void AsyncIoEventSink::HandleClientNamedPipeConnected (
                ClientNamedPipe &clientNamedPipe) throw () {
            if (next.Get () != 0) {
                next->HandleClientNamedPipeConnected (clientNamedPipe);
            }
        }

        void AsyncIoEventSink::HandleServerNamedPipeConnection (
                ServerNamedPipe &serverNamedPipe) throw () {
            if (next.Get () != 0) {
                next->HandleServerNamedPipeConnection (serverNamedPipe);
            }
        }
    #endif // defined (TOOLCHAIN_OS_Windows)

        void AsyncIoEventSink::HandleTCPSocketConnected (TCPSocket &tcpSocket) throw () {
            if (next.Get () != 0) {
                next->HandleTCPSocketConnected (tcpSocket);
            }
        }

    #if defined (THEKOGANS_STREAM_HAVE_OPENSSL)
        void AsyncIoEventSink::HandleSecureTCPSocketConnected (SecureTCPSocket &tcpSocket) throw () {
            if (next.Get () != 0) {
                next->HandleSecureTCPSocketConnected (tcpSocket);
            }
        }

        void AsyncIoEventSink::HandleSecureUDPSocketConnected (SecureUDPSocket &udpSocket) throw () {
            if (next.Get () != 0) {
                next->HandleSecureUDPSocketConnected (udpSocket);
            }
        }
    #endif // defined (THEKOGANS_STREAM_HAVE_OPENSSL)

        void AsyncIoEventSink::HandleTCPSocketShutdown (
                TCPSocket &tcpSocket,
                TCPSocket::ShutdownType shutdownType) throw () {
            if (next.Get () != 0) {
                next->HandleTCPSocketShutdown (tcpSocket, shutdownType);
            }
        }

        void AsyncIoEventSink::HandleServerTCPSocketConnection (
                ServerTCPSocket &serverTCPSocket,
                TCPSocket::SharedPtr connection) throw () {
            if (next.Get () != 0) {
                next->HandleServerTCPSocketConnection (serverTCPSocket, connection);
            }
        }

        void AsyncIoEventSink::HandleServerUDPSocketConnection (
                ServerUDPSocket &serverUDPSocket,
                ServerUDPSocket::Connection::SharedPtr connection) throw () {
            if (next.Get () != 0) {
                next->HandleServerUDPSocketConnection (
                    serverUDPSocket,
                    connection);
            }
        }

    #if defined (THEKOGANS_STREAM_HAVE_OPENSSL)
        void AsyncIoEventSink::HandleServerSecureTCPSocketConnection (
                ServerSecureTCPSocket &serverSecureTCPSocket,
                SecureTCPSocket::SharedPtr connection) throw () {
            if (next.Get () != 0) {
                next->HandleServerSecureTCPSocketConnection (
                    serverSecureTCPSocket,
                    connection);
            }
        }

        void AsyncIoEventSink::HandleServerSecureUDPSocketConnection (
                ServerSecureUDPSocket &serverSecureUDPSocket,
                SecureUDPSocket::SharedPtr connection) throw () {
            if (next.Get () != 0) {
                next->HandleServerSecureUDPSocketConnection (
                    serverSecureUDPSocket,
                    connection);
            }
        }

        void AsyncIoEventSink::HandleSecureTCPSocketHandshakeStarting (
                SecureTCPSocket &secureTCPSocket) throw () {
            if (next.Get () != 0) {
                next->HandleSecureTCPSocketHandshakeStarting (
                    secureTCPSocket);
            }
        }

        void AsyncIoEventSink::HandleSecureTCPSocketHandshakeCompleted (
                SecureTCPSocket &secureTCPSocket) throw () {
            if (next.Get () != 0) {
                next->HandleSecureTCPSocketHandshakeCompleted (
                    secureTCPSocket);
            }
        }

        void AsyncIoEventSink::HandleSecureTCPSocketShutdownCompleted (
                SecureTCPSocket &secureTCPSocket) throw () {
            if (next.Get () != 0) {
                next->HandleSecureTCPSocketShutdownCompleted (
                    secureTCPSocket);
            }
        }

        void AsyncIoEventSink::HandleSecureUDPSocketHandshakeStarting (
                SecureUDPSocket &secureUDPSocket) throw () {
            if (next.Get () != 0) {
                next->HandleSecureUDPSocketHandshakeStarting (
                    secureUDPSocket);
            }
        }

        void AsyncIoEventSink::HandleSecureUDPSocketHandshakeCompleted (
                SecureUDPSocket &secureUDPSocket) throw () {
            if (next.Get () != 0) {
                next->HandleSecureUDPSocketHandshakeCompleted (
                    secureUDPSocket);
            }
        }

        void AsyncIoEventSink::HandleSecureUDPSocketShutdownCompleted (
                SecureUDPSocket &secureUDPSocket) throw () {
            if (next.Get () != 0) {
                next->HandleSecureUDPSocketShutdownCompleted (
                    secureUDPSocket);
            }
        }
    #endif // defined (THEKOGANS_STREAM_HAVE_OPENSSL)

        void AsyncIoEventSink::HandleUDPSocketReadFrom (
                UDPSocket &udpSocket,
                util::Buffer buffer,
                const Address &address) throw () {
            if (next.Get () != 0) {
                next->HandleUDPSocketReadFrom (
                    udpSocket,
                    std::move (buffer),
                    address);
            }
        }

        void AsyncIoEventSink::HandleUDPSocketWriteTo (
                UDPSocket &udpSocket,
                util::Buffer buffer,
                const Address &address) throw () {
            if (next.Get () != 0) {
                next->HandleUDPSocketWriteTo (
                    udpSocket,
                    std::move (buffer),
                    address);
            }
        }

        void AsyncIoEventSink::HandleUDPSocketReadMsg (
                UDPSocket &udpSocket,
                util::Buffer buffer,
                const Address &from,
                const Address &to) throw () {
            if (next.Get () != 0) {
                next->HandleUDPSocketReadMsg (
                    udpSocket,
                    std::move (buffer),
                    from,
                    to);
            }
        }

        void AsyncIoEventSink::HandleUDPSocketWriteMsg (
                UDPSocket &udpSocket,
                util::Buffer buffer,
                const Address &from,
                const Address &to) throw () {
            if (next.Get () != 0) {
                next->HandleUDPSocketWriteMsg (
                    udpSocket,
                    std::move (buffer),
                    from,
                    to);
            }
        }

    } // namespace stream
} // namespace thekogans
