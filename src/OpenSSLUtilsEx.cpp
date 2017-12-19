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

#if defined (THEKOGANS_STREAM_HAVE_OPENSSL)

#if defined (TOOLCHAIN_OS_Windows)
    #if !defined (_WINDOWS_)
        #if !defined (WIN32_LEAN_AND_MEAN)
            #define WIN32_LEAN_AND_MEAN
        #endif // !defined (WIN32_LEAN_AND_MEAN)
        #if !defined (NOMINMAX)
            #define NOMINMAX
        #endif // !defined (NOMINMAX)
    #endif // !defined (_WINDOWS_)
    #include <winsock2.h>
    #include <windows.h>
#endif // defined (TOOLCHAIN_OS_Windows)
#include <openssl/hmac.h>
#include "thekogans/stream/OpenSSLUtilsEx.h"

namespace thekogans {
    namespace stream {

        void EVP_PKEY_CTXDeleter::operator () (EVP_PKEY_CTX *ctx) {
            if (ctx != 0) {
                EVP_PKEY_CTX_free (ctx);
            }
        }

        void EC_POINTDeleter::operator () (EC_POINT *point) {
            if (point != 0) {
                EC_POINT_free (point);
            }
        }

        void EVP_CIPHER_CTXDeleter::operator () (EVP_CIPHER_CTX *ctx) {
            if (ctx != 0) {
                EVP_CIPHER_CTX_free (ctx);
            }
        }

        void EVP_MD_CTXDeleter::operator () (EVP_MD_CTX *ctx) {
            if (ctx != 0) {
                EVP_MD_CTX_destroy (ctx);
            }
        }

        void BIGNUMDeleter::operator () (BIGNUM *bn) {
            if (bn != 0) {
                BN_free (bn);
            }
        }

        namespace {
            const util::ui8 PRIME_MODP_1536[192] = {
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
                0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
                0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
                0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
                0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45, 0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
                0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
                0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
                0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D, 0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
                0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
                0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
                0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D, 0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
                0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x23, 0x73, 0x27, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
            };

            const util::ui8 PRIME_MODP_2048[256] = {
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
                0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
                0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
                0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
                0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45, 0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
                0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
                0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
                0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D, 0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
                0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
                0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
                0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D, 0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
                0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
                0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03, 0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F,
                0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9, 0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18,
                0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
                0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAC, 0xAA, 0x68, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
            };

            const util::ui8 PRIME_MODP_3072[384] = {
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
                0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
                0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
                0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
                0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45, 0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
                0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
                0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
                0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D, 0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
                0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
                0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
                0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D, 0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
                0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
                0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03, 0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F,
                0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9, 0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18,
                0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
                0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAA, 0xC4, 0x2D, 0xAD, 0x33, 0x17, 0x0D, 0x04, 0x50, 0x7A, 0x33,
                0xA8, 0x55, 0x21, 0xAB, 0xDF, 0x1C, 0xBA, 0x64, 0xEC, 0xFB, 0x85, 0x04, 0x58, 0xDB, 0xEF, 0x0A,
                0x8A, 0xEA, 0x71, 0x57, 0x5D, 0x06, 0x0C, 0x7D, 0xB3, 0x97, 0x0F, 0x85, 0xA6, 0xE1, 0xE4, 0xC7,
                0xAB, 0xF5, 0xAE, 0x8C, 0xDB, 0x09, 0x33, 0xD7, 0x1E, 0x8C, 0x94, 0xE0, 0x4A, 0x25, 0x61, 0x9D,
                0xCE, 0xE3, 0xD2, 0x26, 0x1A, 0xD2, 0xEE, 0x6B, 0xF1, 0x2F, 0xFA, 0x06, 0xD9, 0x8A, 0x08, 0x64,
                0xD8, 0x76, 0x02, 0x73, 0x3E, 0xC8, 0x6A, 0x64, 0x52, 0x1F, 0x2B, 0x18, 0x17, 0x7B, 0x20, 0x0C,
                0xBB, 0xE1, 0x17, 0x57, 0x7A, 0x61, 0x5D, 0x6C, 0x77, 0x09, 0x88, 0xC0, 0xBA, 0xD9, 0x46, 0xE2,
                0x08, 0xE2, 0x4F, 0xA0, 0x74, 0xE5, 0xAB, 0x31, 0x43, 0xDB, 0x5B, 0xFC, 0xE0, 0xFD, 0x10, 0x8E,
                0x4B, 0x82, 0xD1, 0x20, 0xA9, 0x3A, 0xD2, 0xCA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
            };

            const util::ui8 PRIME_MODP_4096[512] = {
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
                0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
                0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
                0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
                0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45, 0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
                0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
                0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
                0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D, 0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
                0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
                0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
                0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D, 0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
                0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
                0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03, 0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F,
                0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9, 0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18,
                0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
                0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAA, 0xC4, 0x2D, 0xAD, 0x33, 0x17, 0x0D, 0x04, 0x50, 0x7A, 0x33,
                0xA8, 0x55, 0x21, 0xAB, 0xDF, 0x1C, 0xBA, 0x64, 0xEC, 0xFB, 0x85, 0x04, 0x58, 0xDB, 0xEF, 0x0A,
                0x8A, 0xEA, 0x71, 0x57, 0x5D, 0x06, 0x0C, 0x7D, 0xB3, 0x97, 0x0F, 0x85, 0xA6, 0xE1, 0xE4, 0xC7,
                0xAB, 0xF5, 0xAE, 0x8C, 0xDB, 0x09, 0x33, 0xD7, 0x1E, 0x8C, 0x94, 0xE0, 0x4A, 0x25, 0x61, 0x9D,
                0xCE, 0xE3, 0xD2, 0x26, 0x1A, 0xD2, 0xEE, 0x6B, 0xF1, 0x2F, 0xFA, 0x06, 0xD9, 0x8A, 0x08, 0x64,
                0xD8, 0x76, 0x02, 0x73, 0x3E, 0xC8, 0x6A, 0x64, 0x52, 0x1F, 0x2B, 0x18, 0x17, 0x7B, 0x20, 0x0C,
                0xBB, 0xE1, 0x17, 0x57, 0x7A, 0x61, 0x5D, 0x6C, 0x77, 0x09, 0x88, 0xC0, 0xBA, 0xD9, 0x46, 0xE2,
                0x08, 0xE2, 0x4F, 0xA0, 0x74, 0xE5, 0xAB, 0x31, 0x43, 0xDB, 0x5B, 0xFC, 0xE0, 0xFD, 0x10, 0x8E,
                0x4B, 0x82, 0xD1, 0x20, 0xA9, 0x21, 0x08, 0x01, 0x1A, 0x72, 0x3C, 0x12, 0xA7, 0x87, 0xE6, 0xD7,
                0x88, 0x71, 0x9A, 0x10, 0xBD, 0xBA, 0x5B, 0x26, 0x99, 0xC3, 0x27, 0x18, 0x6A, 0xF4, 0xE2, 0x3C,
                0x1A, 0x94, 0x68, 0x34, 0xB6, 0x15, 0x0B, 0xDA, 0x25, 0x83, 0xE9, 0xCA, 0x2A, 0xD4, 0x4C, 0xE8,
                0xDB, 0xBB, 0xC2, 0xDB, 0x04, 0xDE, 0x8E, 0xF9, 0x2E, 0x8E, 0xFC, 0x14, 0x1F, 0xBE, 0xCA, 0xA6,
                0x28, 0x7C, 0x59, 0x47, 0x4E, 0x6B, 0xC0, 0x5D, 0x99, 0xB2, 0x96, 0x4F, 0xA0, 0x90, 0xC3, 0xA2,
                0x23, 0x3B, 0xA1, 0x86, 0x51, 0x5B, 0xE7, 0xED, 0x1F, 0x61, 0x29, 0x70, 0xCE, 0xE2, 0xD7, 0xAF,
                0xB8, 0x1B, 0xDD, 0x76, 0x21, 0x70, 0x48, 0x1C, 0xD0, 0x06, 0x91, 0x27, 0xD5, 0xB0, 0x5A, 0xA9,
                0x93, 0xB4, 0xEA, 0x98, 0x8D, 0x8F, 0xDD, 0xC1, 0x86, 0xFF, 0xB7, 0xDC, 0x90, 0xA6, 0xC0, 0x8F,
                0x4D, 0xF4, 0x35, 0xC9, 0x34, 0x06, 0x31, 0x99, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xfF
            };

            const util::ui8 PRIME_MODP_6144[768] = {
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
                0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
                0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
                0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
                0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45, 0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
                0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
                0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
                0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D, 0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
                0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
                0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
                0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D, 0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
                0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
                0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03, 0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F,
                0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9, 0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18,
                0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
                0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAA, 0xC4, 0x2D, 0xAD, 0x33, 0x17, 0x0D, 0x04, 0x50, 0x7A, 0x33,
                0xA8, 0x55, 0x21, 0xAB, 0xDF, 0x1C, 0xBA, 0x64, 0xEC, 0xFB, 0x85, 0x04, 0x58, 0xDB, 0xEF, 0x0A,
                0x8A, 0xEA, 0x71, 0x57, 0x5D, 0x06, 0x0C, 0x7D, 0xB3, 0x97, 0x0F, 0x85, 0xA6, 0xE1, 0xE4, 0xC7,
                0xAB, 0xF5, 0xAE, 0x8C, 0xDB, 0x09, 0x33, 0xD7, 0x1E, 0x8C, 0x94, 0xE0, 0x4A, 0x25, 0x61, 0x9D,
                0xCE, 0xE3, 0xD2, 0x26, 0x1A, 0xD2, 0xEE, 0x6B, 0xF1, 0x2F, 0xFA, 0x06, 0xD9, 0x8A, 0x08, 0x64,
                0xD8, 0x76, 0x02, 0x73, 0x3E, 0xC8, 0x6A, 0x64, 0x52, 0x1F, 0x2B, 0x18, 0x17, 0x7B, 0x20, 0x0C,
                0xBB, 0xE1, 0x17, 0x57, 0x7A, 0x61, 0x5D, 0x6C, 0x77, 0x09, 0x88, 0xC0, 0xBA, 0xD9, 0x46, 0xE2,
                0x08, 0xE2, 0x4F, 0xA0, 0x74, 0xE5, 0xAB, 0x31, 0x43, 0xDB, 0x5B, 0xFC, 0xE0, 0xFD, 0x10, 0x8E,
                0x4B, 0x82, 0xD1, 0x20, 0xA9, 0x21, 0x08, 0x01, 0x1A, 0x72, 0x3C, 0x12, 0xA7, 0x87, 0xE6, 0xD7,
                0x88, 0x71, 0x9A, 0x10, 0xBD, 0xBA, 0x5B, 0x26, 0x99, 0xC3, 0x27, 0x18, 0x6A, 0xF4, 0xE2, 0x3C,
                0x1A, 0x94, 0x68, 0x34, 0xB6, 0x15, 0x0B, 0xDA, 0x25, 0x83, 0xE9, 0xCA, 0x2A, 0xD4, 0x4C, 0xE8,
                0xDB, 0xBB, 0xC2, 0xDB, 0x04, 0xDE, 0x8E, 0xF9, 0x2E, 0x8E, 0xFC, 0x14, 0x1F, 0xBE, 0xCA, 0xA6,
                0x28, 0x7C, 0x59, 0x47, 0x4E, 0x6B, 0xC0, 0x5D, 0x99, 0xB2, 0x96, 0x4F, 0xA0, 0x90, 0xC3, 0xA2,
                0x23, 0x3B, 0xA1, 0x86, 0x51, 0x5B, 0xE7, 0xED, 0x1F, 0x61, 0x29, 0x70, 0xCE, 0xE2, 0xD7, 0xAF,
                0xB8, 0x1B, 0xDD, 0x76, 0x21, 0x70, 0x48, 0x1C, 0xD0, 0x06, 0x91, 0x27, 0xD5, 0xB0, 0x5A, 0xA9,
                0x93, 0xB4, 0xEA, 0x98, 0x8D, 0x8F, 0xDD, 0xC1, 0x86, 0xFF, 0xB7, 0xDC, 0x90, 0xA6, 0xC0, 0x8F,
                0x4D, 0xF4, 0x35, 0xC9, 0x34, 0x02, 0x84, 0x92, 0x36, 0xC3, 0xFA, 0xB4, 0xD2, 0x7C, 0x70, 0x26,
                0xC1, 0xD4, 0xDC, 0xB2, 0x60, 0x26, 0x46, 0xDE, 0xC9, 0x75, 0x1E, 0x76, 0x3D, 0xBA, 0x37, 0xBD,
                0xF8, 0xFF, 0x94, 0x06, 0xAD, 0x9E, 0x53, 0x0E, 0xE5, 0xDB, 0x38, 0x2F, 0x41, 0x30, 0x01, 0xAE,
                0xB0, 0x6A, 0x53, 0xED, 0x90, 0x27, 0xD8, 0x31, 0x17, 0x97, 0x27, 0xB0, 0x86, 0x5A, 0x89, 0x18,
                0xDA, 0x3E, 0xDB, 0xEB, 0xCF, 0x9B, 0x14, 0xED, 0x44, 0xCE, 0x6C, 0xBA, 0xCE, 0xD4, 0xBB, 0x1B,
                0xDB, 0x7F, 0x14, 0x47, 0xE6, 0xCC, 0x25, 0x4B, 0x33, 0x20, 0x51, 0x51, 0x2B, 0xD7, 0xAF, 0x42,
                0x6F, 0xB8, 0xF4, 0x01, 0x37, 0x8C, 0xD2, 0xBF, 0x59, 0x83, 0xCA, 0x01, 0xC6, 0x4B, 0x92, 0xEC,
                0xF0, 0x32, 0xEA, 0x15, 0xD1, 0x72, 0x1D, 0x03, 0xF4, 0x82, 0xD7, 0xCE, 0x6E, 0x74, 0xFE, 0xF6,
                0xD5, 0x5E, 0x70, 0x2F, 0x46, 0x98, 0x0C, 0x82, 0xB5, 0xA8, 0x40, 0x31, 0x90, 0x0B, 0x1C, 0x9E,
                0x59, 0xE7, 0xC9, 0x7F, 0xBE, 0xC7, 0xE8, 0xF3, 0x23, 0xA9, 0x7A, 0x7E, 0x36, 0xCC, 0x88, 0xBE,
                0x0F, 0x1D, 0x45, 0xB7, 0xFF, 0x58, 0x5A, 0xC5, 0x4B, 0xD4, 0x07, 0xB2, 0x2B, 0x41, 0x54, 0xAA,
                0xCC, 0x8F, 0x6D, 0x7E, 0xBF, 0x48, 0xE1, 0xD8, 0x14, 0xCC, 0x5E, 0xD2, 0x0F, 0x80, 0x37, 0xE0,
                0xA7, 0x97, 0x15, 0xEE, 0xF2, 0x9B, 0xE3, 0x28, 0x06, 0xA1, 0xD5, 0x8B, 0xB7, 0xC5, 0xDA, 0x76,
                0xF5, 0x50, 0xAA, 0x3D, 0x8A, 0x1F, 0xBF, 0xF0, 0xEB, 0x19, 0xCC, 0xB1, 0xA3, 0x13, 0xD5, 0x5C,
                0xDA, 0x56, 0xC9, 0xEC, 0x2E, 0xF2, 0x96, 0x32, 0x38, 0x7F, 0xE8, 0xD7, 0x6E, 0x3C, 0x04, 0x68,
                0x04, 0x3E, 0x8F, 0x66, 0x3F, 0x48, 0x60, 0xEE, 0x12, 0xBF, 0x2D, 0x5B, 0x0B, 0x74, 0x74, 0xD6,
                0xE6, 0x94, 0xF9, 0x1E, 0x6D, 0xCC, 0x40, 0x24, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
            };

            const util::ui8 PRIME_MODP_8192[1024] = {
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
                0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
                0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
                0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
                0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45, 0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
                0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
                0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
                0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D, 0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
                0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
                0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
                0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D, 0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
                0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
                0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03, 0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F,
                0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9, 0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18,
                0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
                0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAA, 0xC4, 0x2D, 0xAD, 0x33, 0x17, 0x0D, 0x04, 0x50, 0x7A, 0x33,
                0xA8, 0x55, 0x21, 0xAB, 0xDF, 0x1C, 0xBA, 0x64, 0xEC, 0xFB, 0x85, 0x04, 0x58, 0xDB, 0xEF, 0x0A,
                0x8A, 0xEA, 0x71, 0x57, 0x5D, 0x06, 0x0C, 0x7D, 0xB3, 0x97, 0x0F, 0x85, 0xA6, 0xE1, 0xE4, 0xC7,
                0xAB, 0xF5, 0xAE, 0x8C, 0xDB, 0x09, 0x33, 0xD7, 0x1E, 0x8C, 0x94, 0xE0, 0x4A, 0x25, 0x61, 0x9D,
                0xCE, 0xE3, 0xD2, 0x26, 0x1A, 0xD2, 0xEE, 0x6B, 0xF1, 0x2F, 0xFA, 0x06, 0xD9, 0x8A, 0x08, 0x64,
                0xD8, 0x76, 0x02, 0x73, 0x3E, 0xC8, 0x6A, 0x64, 0x52, 0x1F, 0x2B, 0x18, 0x17, 0x7B, 0x20, 0x0C,
                0xBB, 0xE1, 0x17, 0x57, 0x7A, 0x61, 0x5D, 0x6C, 0x77, 0x09, 0x88, 0xC0, 0xBA, 0xD9, 0x46, 0xE2,
                0x08, 0xE2, 0x4F, 0xA0, 0x74, 0xE5, 0xAB, 0x31, 0x43, 0xDB, 0x5B, 0xFC, 0xE0, 0xFD, 0x10, 0x8E,
                0x4B, 0x82, 0xD1, 0x20, 0xA9, 0x21, 0x08, 0x01, 0x1A, 0x72, 0x3C, 0x12, 0xA7, 0x87, 0xE6, 0xD7,
                0x88, 0x71, 0x9A, 0x10, 0xBD, 0xBA, 0x5B, 0x26, 0x99, 0xC3, 0x27, 0x18, 0x6A, 0xF4, 0xE2, 0x3C,
                0x1A, 0x94, 0x68, 0x34, 0xB6, 0x15, 0x0B, 0xDA, 0x25, 0x83, 0xE9, 0xCA, 0x2A, 0xD4, 0x4C, 0xE8,
                0xDB, 0xBB, 0xC2, 0xDB, 0x04, 0xDE, 0x8E, 0xF9, 0x2E, 0x8E, 0xFC, 0x14, 0x1F, 0xBE, 0xCA, 0xA6,
                0x28, 0x7C, 0x59, 0x47, 0x4E, 0x6B, 0xC0, 0x5D, 0x99, 0xB2, 0x96, 0x4F, 0xA0, 0x90, 0xC3, 0xA2,
                0x23, 0x3B, 0xA1, 0x86, 0x51, 0x5B, 0xE7, 0xED, 0x1F, 0x61, 0x29, 0x70, 0xCE, 0xE2, 0xD7, 0xAF,
                0xB8, 0x1B, 0xDD, 0x76, 0x21, 0x70, 0x48, 0x1C, 0xD0, 0x06, 0x91, 0x27, 0xD5, 0xB0, 0x5A, 0xA9,
                0x93, 0xB4, 0xEA, 0x98, 0x8D, 0x8F, 0xDD, 0xC1, 0x86, 0xFF, 0xB7, 0xDC, 0x90, 0xA6, 0xC0, 0x8F,
                0x4D, 0xF4, 0x35, 0xC9, 0x34, 0x02, 0x84, 0x92, 0x36, 0xC3, 0xFA, 0xB4, 0xD2, 0x7C, 0x70, 0x26,
                0xC1, 0xD4, 0xDC, 0xB2, 0x60, 0x26, 0x46, 0xDE, 0xC9, 0x75, 0x1E, 0x76, 0x3D, 0xBA, 0x37, 0xBD,
                0xF8, 0xFF, 0x94, 0x06, 0xAD, 0x9E, 0x53, 0x0E, 0xE5, 0xDB, 0x38, 0x2F, 0x41, 0x30, 0x01, 0xAE,
                0xB0, 0x6A, 0x53, 0xED, 0x90, 0x27, 0xD8, 0x31, 0x17, 0x97, 0x27, 0xB0, 0x86, 0x5A, 0x89, 0x18,
                0xDA, 0x3E, 0xDB, 0xEB, 0xCF, 0x9B, 0x14, 0xED, 0x44, 0xCE, 0x6C, 0xBA, 0xCE, 0xD4, 0xBB, 0x1B,
                0xDB, 0x7F, 0x14, 0x47, 0xE6, 0xCC, 0x25, 0x4B, 0x33, 0x20, 0x51, 0x51, 0x2B, 0xD7, 0xAF, 0x42,
                0x6F, 0xB8, 0xF4, 0x01, 0x37, 0x8C, 0xD2, 0xBF, 0x59, 0x83, 0xCA, 0x01, 0xC6, 0x4B, 0x92, 0xEC,
                0xF0, 0x32, 0xEA, 0x15, 0xD1, 0x72, 0x1D, 0x03, 0xF4, 0x82, 0xD7, 0xCE, 0x6E, 0x74, 0xFE, 0xF6,
                0xD5, 0x5E, 0x70, 0x2F, 0x46, 0x98, 0x0C, 0x82, 0xB5, 0xA8, 0x40, 0x31, 0x90, 0x0B, 0x1C, 0x9E,
                0x59, 0xE7, 0xC9, 0x7F, 0xBE, 0xC7, 0xE8, 0xF3, 0x23, 0xA9, 0x7A, 0x7E, 0x36, 0xCC, 0x88, 0xBE,
                0x0F, 0x1D, 0x45, 0xB7, 0xFF, 0x58, 0x5A, 0xC5, 0x4B, 0xD4, 0x07, 0xB2, 0x2B, 0x41, 0x54, 0xAA,
                0xCC, 0x8F, 0x6D, 0x7E, 0xBF, 0x48, 0xE1, 0xD8, 0x14, 0xCC, 0x5E, 0xD2, 0x0F, 0x80, 0x37, 0xE0,
                0xA7, 0x97, 0x15, 0xEE, 0xF2, 0x9B, 0xE3, 0x28, 0x06, 0xA1, 0xD5, 0x8B, 0xB7, 0xC5, 0xDA, 0x76,
                0xF5, 0x50, 0xAA, 0x3D, 0x8A, 0x1F, 0xBF, 0xF0, 0xEB, 0x19, 0xCC, 0xB1, 0xA3, 0x13, 0xD5, 0x5C,
                0xDA, 0x56, 0xC9, 0xEC, 0x2E, 0xF2, 0x96, 0x32, 0x38, 0x7F, 0xE8, 0xD7, 0x6E, 0x3C, 0x04, 0x68,
                0x04, 0x3E, 0x8F, 0x66, 0x3F, 0x48, 0x60, 0xEE, 0x12, 0xBF, 0x2D, 0x5B, 0x0B, 0x74, 0x74, 0xD6,
                0xE6, 0x94, 0xF9, 0x1E, 0x6D, 0xBE, 0x11, 0x59, 0x74, 0xA3, 0x92, 0x6F, 0x12, 0xFE, 0xE5, 0xE4,
                0x38, 0x77, 0x7C, 0xB6, 0xA9, 0x32, 0xDF, 0x8C, 0xD8, 0xBE, 0xC4, 0xD0, 0x73, 0xB9, 0x31, 0xBA,
                0x3B, 0xC8, 0x32, 0xB6, 0x8D, 0x9D, 0xD3, 0x00, 0x74, 0x1F, 0xA7, 0xBF, 0x8A, 0xFC, 0x47, 0xED,
                0x25, 0x76, 0xF6, 0x93, 0x6B, 0xA4, 0x24, 0x66, 0x3A, 0xAB, 0x63, 0x9C, 0x5A, 0xE4, 0xF5, 0x68,
                0x34, 0x23, 0xB4, 0x74, 0x2B, 0xF1, 0xC9, 0x78, 0x23, 0x8F, 0x16, 0xCB, 0xE3, 0x9D, 0x65, 0x2D,
                0xE3, 0xFD, 0xB8, 0xBE, 0xFC, 0x84, 0x8A, 0xD9, 0x22, 0x22, 0x2E, 0x04, 0xA4, 0x03, 0x7C, 0x07,
                0x13, 0xEB, 0x57, 0xA8, 0x1A, 0x23, 0xF0, 0xC7, 0x34, 0x73, 0xFC, 0x64, 0x6C, 0xEA, 0x30, 0x6B,
                0x4B, 0xCB, 0xC8, 0x86, 0x2F, 0x83, 0x85, 0xDD, 0xFA, 0x9D, 0x4B, 0x7F, 0xA2, 0xC0, 0x87, 0xE8,
                0x79, 0x68, 0x33, 0x03, 0xED, 0x5B, 0xDD, 0x3A, 0x06, 0x2B, 0x3C, 0xF5, 0xB3, 0xA2, 0x78, 0xA6,
                0x6D, 0x2A, 0x13, 0xF8, 0x3F, 0x44, 0xF8, 0x2D, 0xDF, 0x31, 0x0E, 0xE0, 0x74, 0xAB, 0x6A, 0x36,
                0x45, 0x97, 0xE8, 0x99, 0xA0, 0x25, 0x5D, 0xC1, 0x64, 0xF3, 0x1C, 0xC5, 0x08, 0x46, 0x85, 0x1D,
                0xF9, 0xAB, 0x48, 0x19, 0x5D, 0xED, 0x7E, 0xA1, 0xB1, 0xD5, 0x10, 0xBD, 0x7E, 0xE7, 0x4D, 0x73,
                0xFA, 0xF3, 0x6B, 0xC3, 0x1E, 0xCF, 0xA2, 0x68, 0x35, 0x90, 0x46, 0xF4, 0xEB, 0x87, 0x9F, 0x92,
                0x40, 0x09, 0x43, 0x8B, 0x48, 0x1C, 0x6C, 0xD7, 0x88, 0x9A, 0x00, 0x2E, 0xD5, 0xEE, 0x38, 0x2B,
                0xC9, 0x19, 0x0D, 0xA6, 0xFC, 0x02, 0x6E, 0x47, 0x95, 0x58, 0xE4, 0x47, 0x56, 0x77, 0xE9, 0xAA,
                0x9E, 0x30, 0x50, 0xE2, 0x76, 0x56, 0x94, 0xDF, 0xC8, 0x1F, 0x56, 0xE8, 0x80, 0xB9, 0x6E, 0x71,
                0x60, 0xC9, 0x80, 0xDD, 0x98, 0xED, 0xD3, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
            };

            struct Prime {
                const util::ui8 *data;
                int length;
            } primes[] = {
                {PRIME_MODP_1536, 192},
                {PRIME_MODP_2048, 256},
                {PRIME_MODP_3072, 384},
                {PRIME_MODP_4096, 512},
                {PRIME_MODP_6144, 768},
                {PRIME_MODP_8192, 1024}
            };
            const std::size_t PrimesSize = THEKOGANS_UTIL_ARRAY_SIZE (primes);

            util::Buffer::UniquePtr BIGNUMToBuffer (const BIGNUM &bn) {
                util::Buffer::UniquePtr buffer (
                    new util::Buffer (util::HostEndian, (util::ui32)BN_bn2mpi (&bn, 0)));
                buffer->AdvanceWriteOffset ((util::ui32)BN_bn2mpi (&bn, buffer->GetWritePtr ()));
                return buffer;
            }

            BIGNUMPtr BufferToBIGNUM (const util::Buffer &buffer) {
                BIGNUMPtr bn (BN_new ());
                BN_mpi2bn (buffer.GetReadPtr (), (int)buffer.GetDataAvailableForReading (), bn.get ());
                return bn;
            }
        }

        EVP_PKEYPtr DHParams::FromPrimeLength (
                std::size_t primeLength,
                std::size_t generator,
                ENGINE *engine) {
            EVP_PKEY *params = 0;
            EVP_PKEY_CTXPtr ctx (EVP_PKEY_CTX_new_id (EVP_PKEY_DH, engine));
            if (ctx.get () != 0 &&
                    EVP_PKEY_paramgen_init (ctx.get ()) == 1 &&
                    EVP_PKEY_CTX_set_dh_paramgen_prime_len (ctx.get (), (int)primeLength) == 1 &&
                    EVP_PKEY_CTX_set_dh_paramgen_generator (ctx.get (), (int)generator) == 1 &&
                    EVP_PKEY_paramgen (ctx.get (), &params) == 1) {
                return EVP_PKEYPtr (params);
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        EVP_PKEYPtr DHParams::FromPrime (
                BIGNUM &prime,
                std::size_t generator,
                ENGINE *engine) {
            DHPtr dh (DH_new ());
            if (dh.get () != 0) {
                dh->p = BN_dup (&prime);
                dh->g = BN_new ();
                if (dh->p != 0 && dh->g != 0) {
                    BN_set_word (dh->g, generator);
                    EVP_PKEYPtr params (EVP_PKEY_new ());
                    if (params.get () != 0) {
                        EVP_PKEY_assign_DH (params.get (), dh.release ());
                        return params;
                    }
                    else {
                        THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                    }
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        EVP_PKEYPtr DHParams::FromRFC3526Prime (
                RFC3526Prime prime,
                std::size_t generator,
                ENGINE *engine) {
            DHPtr dh (DH_new ());
            if (dh.get () != 0) {
                dh->p = BN_new ();
                dh->g = BN_new ();
                if (dh->p != 0 && dh->g != 0) {
                    BN_bin2bn (primes[prime].data, primes[prime].length, dh->p);
                    BN_set_word (dh->g, generator);
                    EVP_PKEYPtr params (EVP_PKEY_new ());
                    if (params.get () != 0) {
                        EVP_PKEY_assign_DH (params.get (), dh.release ());
                        return params;
                    }
                    else {
                        THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                    }
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        DHSharedSecret::DHSharedSecret (
                Prime prime_,
                util::ui32 generator_) :
                prime (prime_),
                generator (generator_) {
            if (prime < PrimesSize) {
                Reset ();
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
        }

        void DHSharedSecret::Reset () {
            dh.reset (DH_new ());
            if (dh.get () != 0) {
                dh->p = BN_new ();
                dh->g = BN_new ();
                if (dh->p != 0 && dh->g != 0) {
                    BN_bin2bn (primes[prime].data, primes[prime].length, dh->p);
                    BN_set_word (dh->g, generator);
                    if (DH_generate_key (dh.get ()) != 1) {
                        THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                    }
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        util::Buffer::UniquePtr DHSharedSecret::GetPublicKey () {
            return BIGNUMToBuffer (*dh->pub_key);
        }

        util::SecureBuffer::UniquePtr DHSharedSecret::ComputeSharedSecret (
                const util::Buffer &buffer) {
            util::SecureBuffer::UniquePtr sharedSecret (
                new util::SecureBuffer (util::HostEndian, DH_size (dh.get ())));
            BIGNUMPtr publicKey = BufferToBIGNUM (buffer);
            int advance = DH_compute_key (sharedSecret->GetWritePtr (), publicKey.get (), dh.get ());
            if (advance != -1) {
                sharedSecret->AdvanceWriteOffset ((std::size_t)advance);
                return sharedSecret;
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        namespace {
            util::Buffer::UniquePtr EC_POINTToBuffer (
                    const EC_GROUP &group,
                    const EC_POINT &point) {
                util::Buffer::UniquePtr buffer (
                    new util::Buffer (
                        util::HostEndian,
                        (util::ui32)EC_POINT_point2oct (&group, &point,
                            EC_GROUP_get_point_conversion_form (&group), 0, 0, 0)));
                buffer->AdvanceWriteOffset (
                    (util::ui32)EC_POINT_point2oct (&group, &point,
                        EC_GROUP_get_point_conversion_form (&group),
                        buffer->GetWritePtr (),
                        buffer->GetDataAvailableForWriting (), 0));
                return buffer;
            }

            EC_POINTPtr BufferToEC_POINT (
                    const EC_GROUP &group,
                    const util::Buffer &buffer) {
                EC_POINTPtr point (EC_POINT_new (&group));
                if (EC_POINT_oct2point (&group, point.get (), buffer.GetReadPtr (),
                        buffer.GetDataAvailableForReading (), 0) == 1) {
                    return point;
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
        }

        ECDHSharedSecret::ECDHSharedSecret (int nid_) :
                nid (nid_) {
            Reset ();
        }

        void ECDHSharedSecret::Reset () {
            key.reset (EC_KEY_new_by_curve_name (nid));
            if (key.get () == 0 || EC_KEY_generate_key (key.get ()) != 1) {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        util::Buffer::UniquePtr ECDHSharedSecret::GetPublicKey () {
            return EC_POINTToBuffer (
                *EC_KEY_get0_group (key.get ()),
                *EC_KEY_get0_public_key (key.get ()));
        }

        util::SecureBuffer::UniquePtr ECDHSharedSecret::ComputeSharedSecret (
                const util::Buffer &buffer) {
            util::SecureBuffer::UniquePtr sharedSecret (
                new util::Buffer (
                    util::HostEndian,
                    (EC_GROUP_get_degree (EC_KEY_get0_group (key.get ())) + 7) / 8));
            EC_POINTPtr publicKey = BufferToEC_POINT (*EC_KEY_get0_group (key.get ()), buffer);
            int advance = ECDH_compute_key (
                sharedSecret->GetWritePtr (),
                sharedSecret->GetDataAvailableForWriting (),
                publicKey.get (),
                key.get (), 0);
            if (advance != -1) {
                sharedSecret->AdvanceWriteOffset ((std::size_t)advance);
                return sharedSecret;
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        THEKOGANS_UTIL_IMPLEMENT_HEAP_WITH_LOCK (SymmetricKey, util::SpinLock)

        SymmetricKey::SymmetricKey (
                const void *secret,
                std::size_t secretLength,
                const void *salt,
                const EVP_CIPHER *cipher_,
                const EVP_MD *md_) :
                cipher (cipher_ == 0 ? EVP_aes_256_cbc () : cipher_),
                md (md_ == 0 ? EVP_sha256 () : md_),
                key (util::HostEndian, EVP_MAX_KEY_LENGTH),
                iv (util::HostEndian, EVP_MAX_IV_LENGTH) {
            if (secret != 0 && secretLength > 0 && cipher != 0 && md != 0) {
                if (EVP_BytesToKey (cipher, md, (const util::ui8 *)salt,
                        (const util::ui8 *)secret, (int)secretLength, 1,
                        key.GetWritePtr (), iv.GetWritePtr ()) == 0) {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
        }

        Encryptor::Encryptor () :
                ctx (EVP_CIPHER_CTX_new ()) {
            if (ctx.get () == 0) {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        void Encryptor::Init (
                const SymmetricKey &encryptionKey,
                util::Endianness endianness,
                ENGINE *engine) {
            if (EVP_EncryptInit_ex (ctx.get (), encryptionKey.cipher, engine,
                    encryptionKey.key.GetReadPtr (), encryptionKey.iv.GetReadPtr ()) == 1) {
                ciphertext.reset (new util::Buffer (endianness));
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        void Encryptor::Update (
                const void *buffer,
                std::size_t length) {
            if (buffer != 0 && length > 0) {
                ciphertext->Resize (
                    (util::ui32)(
                        ciphertext->GetDataAvailableForReading () +
                        length + EVP_MAX_BLOCK_LENGTH + EVP_MAX_BLOCK_LENGTH));
                int ciphertextLength = 0;
                if (EVP_EncryptUpdate (ctx.get (), ciphertext->GetWritePtr (),
                        &ciphertextLength, (util::ui8 *)buffer, (int)length) == 1) {
                    ciphertext->AdvanceWriteOffset (ciphertextLength);
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
        }

        util::Buffer::UniquePtr Encryptor::Final () {
            int ciphertextLength = 0;
            if (EVP_EncryptFinal_ex (ctx.get (),
                    ciphertext->GetWritePtr (), &ciphertextLength) == 1) {
                ciphertext->AdvanceWriteOffset (ciphertextLength);
                return std::move (ciphertext);
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        Decryptor::Decryptor () :
                ctx (EVP_CIPHER_CTX_new ()) {
            if (ctx.get () == 0) {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        void Decryptor::Init (
                const SymmetricKey &decryptionKey,
                util::Endianness endianness,
                ENGINE *engine) {
            if (EVP_DecryptInit_ex (ctx.get (), decryptionKey.cipher, engine,
                    decryptionKey.key.GetReadPtr (), decryptionKey.iv.GetReadPtr ()) == 1) {
                plaintext.reset (new util::Buffer (endianness));
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        void Decryptor::Update (
                const void *buffer,
                std::size_t length) {
            if (buffer != 0 && length > 0) {
                plaintext->Resize (
                    (util::ui32)(
                        plaintext->GetDataAvailableForReading () +
                        length + EVP_MAX_BLOCK_LENGTH + EVP_MAX_BLOCK_LENGTH));
                int plaintextLength = 0;
                if (EVP_DecryptUpdate (ctx.get (), plaintext->GetWritePtr (),
                        &plaintextLength, (util::ui8 *)buffer, (int)length) == 1) {
                    plaintext->AdvanceWriteOffset (plaintextLength);
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
        }

        util::Buffer::UniquePtr Decryptor::Final () {
            int plaintextLength = 0;
            if (EVP_DecryptFinal_ex (ctx.get (),
                    plaintext->GetWritePtr (), &plaintextLength) == 1) {
                plaintext->AdvanceWriteOffset (plaintextLength);
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
            return std::move (plaintext);
        }

        _LIB_THEKOGANS_STREAM_DECL BIGNUMPtr _LIB_THEKOGANS_STREAM_API
        BIGNUMFromui32 (util::ui32 value) {
            BIGNUMPtr bn (BN_new ());
            BN_set_word (bn.get (), value);
            return bn;
        }

        _LIB_THEKOGANS_STREAM_DECL EVP_PKEYPtr _LIB_THEKOGANS_STREAM_API
        LoadPrivateKey (
                const std::string &path,
                pem_password_cb *passwordCallback,
                void *userData) {
            BIOPtr bio (BIO_new_file (path.c_str (), "r"));
            if (bio.get () != 0) {
                EVP_PKEYPtr key (PEM_read_bio_PrivateKey (bio.get (), 0, passwordCallback, userData));
                if (key.get () != 0) {
                    return key;
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        _LIB_THEKOGANS_STREAM_DECL EVP_PKEYPtr _LIB_THEKOGANS_STREAM_API
        LoadPublicKey (
                const std::string &path,
                pem_password_cb *passwordCallback,
                void *userData) {
            BIOPtr bio (BIO_new_file (path.c_str (), "r"));
            if (bio.get () != 0) {
                EVP_PKEYPtr key (PEM_read_bio_PUBKEY (bio.get (), 0, passwordCallback, userData));
                if (key.get () != 0) {
                    return key;
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        _LIB_THEKOGANS_STREAM_DECL EVP_PKEYPtr _LIB_THEKOGANS_STREAM_API
        LoadPublicKeyFromCertificat (
                const std::string &path,
                pem_password_cb *passwordCallback,
                void *userData) {
            BIOPtr bio (BIO_new_file (path.c_str (), "r"));
            if (bio.get () != 0) {
                X509Ptr certificate (PEM_read_bio_X509 (bio.get (), 0, passwordCallback, userData));
                if (certificate.get () != 0) {
                    EVP_PKEYPtr key (X509_get_pubkey (certificate.get ()));
                    if (key.get () != 0) {
                        return key;
                    }
                    else {
                        THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                    }
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        _LIB_THEKOGANS_STREAM_DECL util::Buffer::UniquePtr _LIB_THEKOGANS_STREAM_API
        PublicKeyToDER (EVP_PKEY &key) {
            util::ui8 *publicKey = 0;
            int publicKeyLength = i2d_PUBKEY (&key, &publicKey);
            return util::Buffer::UniquePtr (
                new util::Buffer (
                    util::NetworkEndian,
                    publicKey,
                    publicKeyLength,
                    0,
                    publicKeyLength,
                    &OpenSSLAllocator::Global));
        }

        _LIB_THEKOGANS_STREAM_DECL util::Buffer::UniquePtr _LIB_THEKOGANS_STREAM_API
        PrivateKeyToDER (EVP_PKEY &key) {
            util::ui8 *privateKey = 0;
            int privateKeyLength = i2d_PrivateKey (&key, &privateKey);
            return util::Buffer::UniquePtr (
                new util::Buffer (
                    util::NetworkEndian,
                    privateKey,
                    privateKeyLength,
                    0,
                    privateKeyLength,
                    &OpenSSLAllocator::Global));
        }

        _LIB_THEKOGANS_STREAM_DECL EVP_PKEYPtr _LIB_THEKOGANS_STREAM_API
        DERToPublicKey (
                const void *publicKey,
                std::size_t publicKeyLength) {
            if (publicKey != 0 && publicKeyLength > 0) {
                const util::ui8 *pp = (const util::ui8 *)publicKey;
                return EVP_PKEYPtr (d2i_PUBKEY (0, &pp, (long)publicKeyLength));
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
        }

        _LIB_THEKOGANS_STREAM_DECL EVP_PKEYPtr _LIB_THEKOGANS_STREAM_API
        DERToPrivateKey (
                const void *privateKey,
                std::size_t privateKeyLength) {
            if (privateKey != 0 && privateKeyLength > 0) {
                const util::ui8 *pp = (const util::ui8 *)privateKey;
                return EVP_PKEYPtr (d2i_AutoPrivateKey (0, &pp, (long)privateKeyLength));
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
        }

        _LIB_THEKOGANS_STREAM_DECL EVP_PKEYPtr _LIB_THEKOGANS_STREAM_API
        CreateRSAKey (
                std::size_t bits,
                BIGNUMPtr publicExponent,
                ENGINE *engine) {
            EVP_PKEY *key = 0;
            EVP_PKEY_CTXPtr ctx (EVP_PKEY_CTX_new_id (EVP_PKEY_RSA, engine));
            if (ctx.get () != 0 &&
                    EVP_PKEY_keygen_init (ctx.get ()) == 1 &&
                    EVP_PKEY_CTX_set_rsa_keygen_bits (ctx.get (), (int)bits) == 1 &&
                    EVP_PKEY_CTX_set_rsa_keygen_pubexp (ctx.get (), publicExponent.release ()) == 1 &&
                    EVP_PKEY_keygen (ctx.get (), &key) == 1) {
                return EVP_PKEYPtr (key);
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        namespace {
            EVP_PKEYPtr GenerateDSAParams (
                    std::size_t bits,
                    ENGINE *engine) {
                EVP_PKEY *params = 0;
                EVP_PKEY_CTXPtr ctx (EVP_PKEY_CTX_new_id (EVP_PKEY_DSA, engine));
                if (ctx.get () != 0 &&
                        EVP_PKEY_paramgen_init (ctx.get ()) == 1 &&
                        EVP_PKEY_CTX_set_dsa_paramgen_bits (ctx.get (), (int)bits) == 1 &&
                        EVP_PKEY_paramgen (ctx.get (), &params) == 1) {
                    return EVP_PKEYPtr (params);
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
        }

        _LIB_THEKOGANS_STREAM_DECL EVP_PKEYPtr _LIB_THEKOGANS_STREAM_API
        CreateDSAKey (
                std::size_t bits,
                ENGINE *engine) {
            EVP_PKEYPtr params = GenerateDSAParams (bits, engine);
            EVP_PKEY *key = 0;
            EVP_PKEY_CTXPtr ctx (EVP_PKEY_CTX_new (params.get (), engine));
            if (ctx.get () != 0 &&
                    EVP_PKEY_keygen_init (ctx.get ()) == 1 &&
                    EVP_PKEY_keygen (ctx.get (), &key) == 1) {
                return EVP_PKEYPtr (key);
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        _LIB_THEKOGANS_STREAM_DECL EVP_PKEYPtr _LIB_THEKOGANS_STREAM_API
        CreateDHKey (
                EVP_PKEY &params,
                ENGINE *engine) {
            EVP_PKEY *key = 0;
            EVP_PKEY_CTXPtr ctx (EVP_PKEY_CTX_new (&params, engine));
            if (ctx.get () != 0 &&
                    EVP_PKEY_keygen_init (ctx.get ()) == 1 &&
                    EVP_PKEY_keygen (ctx.get (), &key) == 1) {
                return EVP_PKEYPtr (key);
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        namespace {
            EVP_PKEYPtr GenerateECParams (
                    int nid,
                    int parameterEncoding,
                    ENGINE *engine) {
                EVP_PKEY *params = 0;
                EVP_PKEY_CTXPtr ctx (EVP_PKEY_CTX_new_id (EVP_PKEY_EC, engine));
                if (ctx.get () != 0 &&
                        EVP_PKEY_paramgen_init (ctx.get ()) == 1 &&
                        EVP_PKEY_CTX_set_ec_paramgen_curve_nid (ctx.get (), nid) == 1 &&
                        EVP_PKEY_CTX_set_ec_param_enc (ctx.get (), parameterEncoding) == 1 &&
                        EVP_PKEY_paramgen (ctx.get (), &params) == 1) {
                    return EVP_PKEYPtr (params);
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
        }

        _LIB_THEKOGANS_STREAM_DECL EVP_PKEYPtr _LIB_THEKOGANS_STREAM_API
        CreateECKey (
                int nid,
                int parameterEncoding,
                ENGINE *engine) {
            EVP_PKEYPtr params = GenerateECParams (nid, parameterEncoding, engine);
            EVP_PKEY *key = 0;
            EVP_PKEY_CTXPtr ctx (EVP_PKEY_CTX_new (params.get (), engine));
            if (ctx.get () != 0 &&
                    EVP_PKEY_keygen_init (ctx.get ()) == 1 &&
                    EVP_PKEY_keygen (ctx.get (), &key) == 1) {
                return EVP_PKEYPtr (key);
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        _LIB_THEKOGANS_STREAM_DECL EVP_PKEYPtr _LIB_THEKOGANS_STREAM_API
        CreateHMACKey (
                const void *secret,
                std::size_t secretLength,
                ENGINE *engine) {
            EVP_PKEY *key = 0;
            EVP_PKEY_CTXPtr ctx (EVP_PKEY_CTX_new_id (EVP_PKEY_HMAC, engine));
            if (ctx.get () != 0 &&
                    EVP_PKEY_keygen_init (ctx.get ()) == 1 &&
                    EVP_PKEY_CTX_ctrl (ctx.get (), -1, EVP_PKEY_OP_KEYGEN,
                        EVP_PKEY_CTRL_SET_MAC_KEY, (int)secretLength, (void *)secret) == 1 &&
                    EVP_PKEY_keygen (ctx.get (), &key) == 1) {
                return EVP_PKEYPtr (key);
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        _LIB_THEKOGANS_STREAM_DECL EVP_PKEYPtr _LIB_THEKOGANS_STREAM_API
        CreateCMACKey (
                const void *secret,
                std::size_t secretLength,
                const EVP_CIPHER *cipher,
                ENGINE *engine) {
            EVP_PKEY *key = 0;
            EVP_PKEY_CTXPtr ctx (EVP_PKEY_CTX_new_id (EVP_PKEY_CMAC, engine));
            if (ctx.get () != 0 &&
                    EVP_PKEY_keygen_init (ctx.get ()) == 1 &&
                    EVP_PKEY_CTX_ctrl (ctx.get (), -1, EVP_PKEY_OP_KEYGEN,
                        EVP_PKEY_CTRL_CIPHER, 0, (void *)(cipher == 0 ? EVP_aes_256_cbc () : cipher)) == 1 &&
                    EVP_PKEY_CTX_ctrl (ctx.get (), -1, EVP_PKEY_OP_KEYGEN,
                        EVP_PKEY_CTRL_SET_MAC_KEY, (int)secretLength, (void *)secret) == 1 &&
                    EVP_PKEY_keygen (ctx.get (), &key) == 1) {
                return EVP_PKEYPtr (key);
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        namespace {
            util::Buffer::UniquePtr MACSignBuffer (
                    const void *buffer,
                    std::size_t length,
                    EVP_PKEY &privateKey,
                    const EVP_MD *md,
                    util::Endianness endianness,
                    ENGINE *engine) {
                EVP_MD_CTXPtr ctx (EVP_MD_CTX_create ());
                if (ctx.get () != 0 &&
                        EVP_DigestInit_ex (ctx.get (), md, engine) == 1 &&
                        EVP_DigestSignInit (ctx.get (), 0, md, engine, &privateKey) == 1 &&
                        EVP_DigestSignUpdate (ctx.get (), buffer, length) == 1) {
                    size_t signatureLength = 0;
                    if (EVP_DigestSignFinal (ctx.get (), 0,
                            &signatureLength) == 1 && signatureLength > 0) {
                        util::Buffer::UniquePtr signature (
                            new util::Buffer (endianness, (util::ui32)signatureLength));
                        if (EVP_DigestSignFinal (ctx.get (),
                                signature->GetWritePtr (), &signatureLength) == 1) {
                            signature->AdvanceWriteOffset ((util::ui32)signatureLength);
                            return signature;
                        }
                        else {
                            THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                        }
                    }
                    else {
                        THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                    }
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }

            util::Buffer::UniquePtr PKSignBuffer (
                    const void *buffer,
                    std::size_t length,
                    EVP_PKEY &privateKey,
                    const EVP_MD *md,
                    util::Endianness endianness,
                    ENGINE *engine) {
                EVP_MD_CTXPtr ctx (EVP_MD_CTX_create ());
                if (ctx.get () != 0 &&
                        EVP_DigestSignInit (ctx.get (), 0, md, engine, &privateKey) == 1 &&
                        EVP_DigestSignUpdate (ctx.get (), buffer, length) == 1) {
                    size_t signatureLength = 0;
                    if (EVP_DigestSignFinal (ctx.get (), 0,
                            &signatureLength) == 1 && signatureLength > 0) {
                        util::Buffer::UniquePtr signature (
                            new util::Buffer (endianness, (util::ui32)signatureLength));
                        if (EVP_DigestSignFinal (ctx.get (),
                                signature->GetWritePtr (), &signatureLength) == 1) {
                            signature->AdvanceWriteOffset ((util::ui32)signatureLength);
                            return signature;
                        }
                        else {
                            THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                        }
                    }
                    else {
                        THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                    }
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
        }

        _LIB_THEKOGANS_STREAM_DECL util::Buffer::UniquePtr _LIB_THEKOGANS_STREAM_API
        SignBuffer (
                const void *buffer,
                std::size_t length,
                EVP_PKEY &privateKey,
                const EVP_MD *md,
                util::Endianness endianness,
                ENGINE *engine) {
            if (buffer != 0 && length > 0) {
                if (md == 0) {
                    md = EVP_sha256 ();
                }
                return privateKey.type == EVP_PKEY_HMAC || privateKey.type == EVP_PKEY_CMAC ?
                    MACSignBuffer (buffer, length, privateKey, md, endianness, engine) :
                    PKSignBuffer (buffer, length, privateKey, md, endianness, engine);
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
        }

        _LIB_THEKOGANS_STREAM_DECL util::Buffer::UniquePtr _LIB_THEKOGANS_STREAM_API
        EncryptBuffer (
                const void *buffer,
                std::size_t length,
                const SymmetricKey &encryptionKey,
                util::Endianness endianness,
                ENGINE *engine) {
            Encryptor encryptor;
            encryptor.Init (encryptionKey, endianness, engine);
            encryptor.Update (buffer, length);
            return encryptor.Final ();
        }

        _LIB_THEKOGANS_STREAM_DECL util::Buffer::UniquePtr _LIB_THEKOGANS_STREAM_API
        SignAndEncryptBuffer (
                const void *buffer,
                std::size_t length,
                const SymmetricKey &encryptionKey,
                EVP_PKEY &signatureKey,
                const EVP_MD *md,
                util::Endianness endianness,
                ENGINE *engine) {
            util::Buffer::UniquePtr signature =
                SignBuffer (buffer, length, signatureKey, md, endianness, engine);
            util::TenantReadBuffer plaintext (
                util::NetworkEndian, (const util::ui8 *)buffer, (util::ui32)length);
            util::Buffer plaintextAndSignature (util::NetworkEndian,
                util::UI32_SIZE +
                util::Serializer::Size (plaintext) +
                util::Serializer::Size (*signature));
            plaintextAndSignature << util::MAGIC32 << plaintext << *signature;
            return EncryptBuffer (
                plaintextAndSignature.GetReadPtr (),
                plaintextAndSignature.GetDataAvailableForReading (),
                encryptionKey,
                endianness,
                engine);
        }

        _LIB_THEKOGANS_STREAM_DECL util::Buffer::UniquePtr _LIB_THEKOGANS_STREAM_API
        DecryptBuffer (
                const void *buffer,
                std::size_t length,
                const SymmetricKey &decryptionKey,
                util::Endianness endianness,
                ENGINE *engine) {
            Decryptor decryptor;
            decryptor.Init (decryptionKey, endianness, engine);
            decryptor.Update (buffer, length);
            return decryptor.Final ();
        }

        namespace {
            bool MACVerifyBufferSignature (
                    const void *buffer,
                    std::size_t bufferLength,
                    const void *signature,
                    std::size_t signatureLength,
                    EVP_PKEY &publicKey,
                    const EVP_MD *md,
                    ENGINE *engine) {
                EVP_MD_CTXPtr ctx (EVP_MD_CTX_create ());
                if (EVP_DigestInit_ex (ctx.get (), md, engine) == 1 &&
                        EVP_DigestSignInit (ctx.get (), 0, md, engine, &publicKey) == 1 &&
                        EVP_DigestSignUpdate (ctx.get (), buffer, bufferLength) == 1) {
                    util::ui8 computedSignature[EVP_MAX_MD_SIZE];
                    std::size_t computedSignatureLength = sizeof (computedSignature);
                    if (EVP_DigestSignFinal (ctx.get (),
                            computedSignature, &computedSignatureLength) == 1) {
                        return signatureLength == computedSignatureLength &&
                            memcmp (signature, computedSignature, signatureLength) == 0;
                    }
                    else {
                        THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                    }
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }

            bool PKVerifyBufferSignature (
                    const void *buffer,
                    std::size_t bufferLength,
                    const void *signature,
                    std::size_t signatureLength,
                    EVP_PKEY &publicKey,
                    const EVP_MD *md,
                    ENGINE *engine) {
                EVP_MD_CTXPtr ctx (EVP_MD_CTX_create ());
                if (ctx.get () != 0 &&
                        EVP_DigestVerifyInit (ctx.get (), 0, md, engine, &publicKey) == 1 &&
                        EVP_DigestVerifyUpdate (ctx.get (), buffer, bufferLength) == 1) {
                    return EVP_DigestVerifyFinal (ctx.get (),
                        (const util::ui8 *)signature, signatureLength) == 1;
                }
                else {
                    THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
                }
            }
        }

        _LIB_THEKOGANS_STREAM_DECL bool _LIB_THEKOGANS_STREAM_API
        VerifyBufferSignature (
                const void *buffer,
                std::size_t bufferLength,
                const void *signature,
                std::size_t signatureLength,
                EVP_PKEY &publicKey,
                const EVP_MD *md,
                ENGINE *engine) {
            if (buffer != 0 && bufferLength > 0 &&
                    signature != 0 && signatureLength > 0) {
                if (md == 0) {
                    md = EVP_sha256 ();
                }
                return publicKey.type == EVP_PKEY_HMAC || publicKey.type == EVP_PKEY_CMAC ?
                    MACVerifyBufferSignature (
                        buffer, bufferLength, signature, signatureLength, publicKey, md, engine) :
                    PKVerifyBufferSignature (
                        buffer, bufferLength, signature, signatureLength, publicKey, md, engine);
            }
            else {
                THEKOGANS_UTIL_THROW_ERROR_CODE_EXCEPTION (
                    THEKOGANS_UTIL_OS_ERROR_CODE_EINVAL);
            }
        }

        _LIB_THEKOGANS_STREAM_DECL util::Buffer::UniquePtr _LIB_THEKOGANS_STREAM_API
        DecryptAndVerifyBufferSignature (
                const void *buffer,
                std::size_t length,
                const SymmetricKey &decryptionKey,
                EVP_PKEY &signatureKey,
                const EVP_MD *md,
                util::Endianness endianness,
                ENGINE *engine) {
            util::Buffer::UniquePtr plaintextAndSignature = DecryptBuffer (
                buffer, length, decryptionKey, endianness, engine);
            util::ui32 magic;
            *plaintextAndSignature >> magic;
            if (magic == util::MAGIC32) {
                util::Buffer::UniquePtr plaintext (new util::Buffer);
                util::Buffer signature;
                *plaintextAndSignature >> *plaintext >> signature;
                if (VerifyBufferSignature (
                        plaintext->GetReadPtr (),
                        plaintext->GetDataAvailableForReading (),
                        signature.GetReadPtr (),
                        signature.GetDataAvailableForReading (),
                        signatureKey,
                        md,
                        engine)) {
                    return plaintext;
                }
                else {
                    THEKOGANS_UTIL_THROW_STRING_EXCEPTION (
                        "%s", "Buffer failed signature verifacion.");
                }
            }
            else {
                THEKOGANS_UTIL_THROW_STRING_EXCEPTION (
                    "%s", "Invalid buffer.");
            }
        }

        _LIB_THEKOGANS_STREAM_DECL std::string _LIB_THEKOGANS_STREAM_API OTP (
                const void *key,
                std::size_t keyLength,
                const void *buffer,
                std::size_t bufferLength,
                const EVP_MD *md) {
            util::ui8 hash[EVP_MAX_MD_SIZE];
            util::ui32 hashLength = 0;
            if (md == 0) {
                md = EVP_sha1 ();
            }
            if (HMAC (md, key, (int)keyLength, (const util::ui8 *)buffer,
                    bufferLength, hash, &hashLength) != 0) {
                util::ui32 offset = hash[hashLength - 1] & 0xf;
                return util::FormatString ("%06d",
                    (((hash[offset] & 0x7f) << 24) |
                    ((hash[offset + 1] & 0xff) << 16) |
                    ((hash[offset + 2] & 0xff) << 8) |
                    (hash[offset + 3] & 0xff)) % 1000000);
            }
            else {
                THEKOGANS_STREAM_THROW_OPENSSL_EXCEPTION;
            }
        }

        _LIB_THEKOGANS_STREAM_DECL std::string _LIB_THEKOGANS_STREAM_API OTP (
                const void *key,
                std::size_t keyLength,
                util::ui64 value,
                const EVP_MD *md) {
            util::ui8 buffer[8];
            for (util::ui32 i = 8; i-- > 0;) {
                buffer[i] = (util::ui8)(value & 0xff);
                value >>= 8;
            }
            return OTP (key, keyLength, buffer, 8, md);
        }

        void KeySet::ComputeKeys (const util::Buffer &publicKey) {
            util::SecureBuffer::UniquePtr secret =
                sharedSecret->ComputeSharedSecret (publicKey);
            encryptionKey.reset (
                new stream::SymmetricKey (
                    secret->GetReadPtr (),
                    secret->GetDataAvailableForReading ()));
            signatureKey =
                stream::CreateHMACKey (
                    secret->GetReadPtr (),
                    secret->GetDataAvailableForReading ());
        }

    } // namespace stream
} // namespace thekogans

#endif // defined (THEKOGANS_STREAM_HAVE_OPENSSL)
