/*
 * Copyright (C) 2011      TrilliumEMU <http://www.trilliumemu.org/>
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

#ifndef STORAGE_LOADER_H
#define STORAGE_LOADER_H

#include "Define.h"
#include "Utilities/ByteConverter.h"
#include <cassert>

enum
{
    FT_NA='x',                                              //not used or unknown, 4 byte size
    FT_NA_BYTE='X',                                         //not used or unknown, byte
    FT_STRING='s',                                          //char*
    FT_FLOAT='f',                                           //float
    FT_INT='i',                                             //uint32
    FT_BYTE='b',                                            //uint8
    FT_SORT='d',                                            //sorted by this field, field is not included
    FT_IND='n',                                             //the same, but parsed to data
    FT_LOGIC='l',                                            //Logical (boolean)
    FT_SQL_PRESENT='p',                                      //Used in sql format to mark column present in sql dbc
    FT_SQL_ABSENT='a'                                       //Used in sql format to mark column absent in sql dbc
};

class StorageLoader
{
    public:
        StorageLoader();
        ~StorageLoader();

        bool LoadDBCStorage(const char* filename, const char* fmt);
        bool LoadDB2Storage(const char* filename, const char* fmt);

        class Record
        {
            public:
                float getFloat(size_t field) const
                {
                    assert(field < file.fieldCount);
                    float val = *reinterpret_cast<float*>(offset+file.GetOffset(field));
                    EndianConvert(val);
                    return val;
                }
                uint32 getUInt(size_t field) const
                {
                    assert(field < file.fieldCount);
                    uint32 val = *reinterpret_cast<uint32*>(offset+file.GetOffset(field));
                    EndianConvert(val);
                    return val;
                }
                uint8 getUInt8(size_t field) const
                {
                    assert(field < file.fieldCount);
                    return *reinterpret_cast<uint8*>(offset+file.GetOffset(field));
                }

                const char *getString(size_t field) const
                {
                    assert(field < file.fieldCount);
                    size_t stringOffset = getUInt(field);
                    assert(stringOffset < file.stringSize);
                    return reinterpret_cast<char*>(file.stringTable + stringOffset);
                }

            private:
                Record(StorageLoader &file_, unsigned char *offset_): offset(offset_), file(file_) {}
                unsigned char *offset;
                StorageLoader &file;

                friend class StorageLoader;

        };

        // Get record by id
        Record getRecord(size_t id);
        /// Get begin iterator over records

        uint32 GetNumRows() const { return recordCount; }
        uint32 GetRowSize() const { return recordSize; }
        uint32 GetCols() const { return fieldCount; }
        uint32 GetOffset(size_t id) const { return (fieldsOffset != NULL && id < fieldCount) ? fieldsOffset[id] : 0; }
        bool IsLoaded() const { return data != NULL; }
        char* AutoProduceData(const char* fmt, uint32& count, char**& indexTable, uint32 sqlRecordCount, uint32 sqlHighestIndex, char *& sqlDataTable);
        char* AutoProduceStringsArrayHolders(const char* fmt, char* dataTable);
        char* AutoProduceStrings(const char* fmt, char* dataTable);
        static uint32 GetFormatRecordSize(const char * format, int32 * index_pos = NULL);
        static uint32 GetFormatStringsFields(const char * format);
    private:

        uint32 recordSize;
        uint32 recordCount;
        uint32 fieldCount;
        uint32 stringSize;
        uint32 *fieldsOffset;
        unsigned char *data;
        unsigned char *stringTable;

        // WDB2 / WCH2 fields
        uint32 tableHash;    // WDB2
        uint32 build;        // WDB2

        int unk1;            // WDB2 (Unix time in WCH2)
        int unk2;            // WDB2
        int unk3;            // WDB2 (index table)
        int locale;          // WDB2
        int unk5;            // WDB2
};
#endif
