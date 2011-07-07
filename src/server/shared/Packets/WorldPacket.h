/*
 * Copyright (C) 2011      TrilliumEMU <http://www.trilliumemu.com/>
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS      <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _WORLDPACKET_H
#define _WORLDPACKET_H

#include "Common.h"
#include "Opcodes.h"
#include "ByteBuffer.h"
#include "World.h"
#include <zlib.h>

class WorldPacket : public ByteBuffer
{
    public:
                                                            // just container for later use
        WorldPacket()                                       : ByteBuffer(0), m_opcode(UNKNOWN_OPCODE)
        {
        }
        explicit WorldPacket(Opcodes opcode, size_t res = 200) : ByteBuffer(res), m_opcode(opcode) { }
                                                            // copy constructor
        WorldPacket(const WorldPacket& packet)              : ByteBuffer(packet), m_opcode(packet.m_opcode)
        {
        }

        void Initialize(Opcodes opcode, size_t newres = 200)
        {
            clear();
            _storage.reserve(newres);
            m_opcode = opcode;
        }

        Opcodes GetOpcode() const { return m_opcode; }
        void SetOpcode(Opcodes opcode) { m_opcode = opcode; }

        void compress(Opcodes opcode)
        {
            if (opcode == UNKNOWN_OPCODE)
                return;

            Opcodes uncompressedOpcode = GetOpcode();
            uint32 size = wpos();
            uint32 destsize = compressBound(size);

            std::vector<uint8> storage(destsize);

            _compress(static_cast<void*>(&storage[0]), &destsize, static_cast<const void*>(contents()), size);
            if (destsize == 0)
                return;

            clear();
            reserve(destsize + sizeof(uint32));
            *this << uint32(size);
            append(&storage[0], destsize);
            SetOpcode(opcode);

            sLog->outStaticDebug("Successfully compressed opcode %u (len %u) to %u (len %u)",
                uncompressedOpcode, size, opcode, destsize);
        }

    protected:
        Opcodes m_opcode;
        void _compress(void* dst, uint32 *dst_size, const void* src, int src_size)
        {
            z_stream c_stream;

            c_stream.zalloc = (alloc_func)0;
            c_stream.zfree = (free_func)0;
            c_stream.opaque = (voidpf)0;

            // default Z_BEST_SPEED (1)
            int z_res = deflateInit(&c_stream, sWorld->getIntConfig(CONFIG_COMPRESSION));
            if (z_res != Z_OK)
            {
                sLog->outError("Can't compress update packet (zlib: deflateInit) Error code: %i (%s)",z_res,zError(z_res));
                *dst_size = 0;
                return;
            }

            c_stream.next_out = (Bytef*)dst;
            c_stream.avail_out = *dst_size;
            c_stream.next_in = (Bytef*)src;
            c_stream.avail_in = (uInt)src_size;
            z_res = deflate(&c_stream, Z_NO_FLUSH);
            if (z_res != Z_OK)
            {
                sLog->outError("Can't compress update packet (zlib: deflate) Error code: %i (%s)",z_res,zError(z_res));
                *dst_size = 0;
                return;
            }

            if (c_stream.avail_in != 0)
            {
                sLog->outError("Can't compress update packet (zlib: deflate not greedy)");
                *dst_size = 0;
                return;
            }

            z_res = deflate(&c_stream, Z_FINISH);
            if (z_res != Z_STREAM_END)
            {
                sLog->outError("Can't compress update packet (zlib: deflate should report Z_STREAM_END instead %i (%s)",z_res,zError(z_res));
                *dst_size = 0;
                return;
            }

            z_res = deflateEnd(&c_stream);
            if (z_res != Z_OK)
            {
                sLog->outError("Can't compress update packet (zlib: deflateEnd) Error code: %i (%s)",z_res,zError(z_res));
                *dst_size = 0;
                return;
            }

            *dst_size = c_stream.total_out;
        }
};
#endif

