
#ifndef _FIELD_H
#define _FIELD_H
#include "com.h"
#include <mysql.h>
class Field
{
    friend class ResultSet;
    friend class PreparedResultSet;

    public:

        bool GetBool() const // Wrapper, actually gets integer
        {
            return GetUInt8() == 1 ? true : false;
        }

        uint8 GetUInt8() const
        {
            if (!data.value)
                return 0;

            #ifdef _DEBUG
            if (!IsType(MYSQL_TYPE_TINY))
            {
                _LOG_WARN(LOG_FILTER_SQL, "Warning: GetUInt8() on non-tinyint field. Using type: %s.", FieldTypeToString(data.type));
                return 0;
            }
            #endif

            if (data.raw)
                return *reinterpret_cast<uint8*>(data.value);
            return static_cast<uint8>(atol((char*)data.value));
        }

        int8 GetInt8() const
        {
            if (!data.value)
                return 0;

            #ifdef _DEBUG
            if (!IsType(MYSQL_TYPE_TINY))
            {
                _LOG_WARN(LOG_FILTER_SQL, "Warning: GetInt8() on non-tinyint field. Using type: %s.", FieldTypeToString(data.type));
                return 0;
            }
            #endif

            if (data.raw)
                return *reinterpret_cast<int8*>(data.value);
            return static_cast<int8>(atol((char*)data.value));
        }

        uint16 GetUInt16() const
        {
            if (!data.value)
                return 0;

            #ifdef _DEBUG
            if (!IsType(MYSQL_TYPE_SHORT) && !IsType(MYSQL_TYPE_YEAR))
            {
                _LOG_WARN(LOG_FILTER_SQL, "Warning: GetUInt16() on non-smallint field. Using type: %s.", FieldTypeToString(data.type));
                return 0;
            }
            #endif

            if (data.raw)
                return *reinterpret_cast<uint16*>(data.value);
            return static_cast<uint16>(atol((char*)data.value));
        }

        int16 GetInt16() const
        {
            if (!data.value)
                return 0;

            #ifdef _DEBUG
            if (!IsType(MYSQL_TYPE_SHORT) && !IsType(MYSQL_TYPE_YEAR))
            {
                _LOG_WARN(LOG_FILTER_SQL, "Warning: GetInt16() on non-smallint field. Using type: %s.", FieldTypeToString(data.type));
                return 0;
            }
            #endif

            if (data.raw)
                return *reinterpret_cast<int16*>(data.value);
            return static_cast<int16>(atol((char*)data.value));
        }

        uint32 GetUInt32() const
        {
            if (!data.value)
                return 0;

            #ifdef _DEBUG
            if (!IsType(MYSQL_TYPE_INT24) && !IsType(MYSQL_TYPE_LONG))
            {
                _LOG_WARN(LOG_FILTER_SQL, "Warning: GetUInt32() on non-(medium)int field. Using type: %s.", FieldTypeToString(data.type));
                return 0;
            }
            #endif

            if (data.raw)
                return *reinterpret_cast<uint32*>(data.value);
            return static_cast<uint32>(atol((char*)data.value));
        }

        int32 GetInt32() const
        {
            if (!data.value)
                return 0;

            #ifdef _DEBUG
            if (!IsType(MYSQL_TYPE_INT24) && !IsType(MYSQL_TYPE_LONG))
            {
                _LOG_WARN(LOG_FILTER_SQL, "Warning: GetInt32() on non-(medium)int field. Using type: %s.", FieldTypeToString(data.type));
                return 0;
            }
            #endif

            if (data.raw)
                return *reinterpret_cast<int32*>(data.value);
            return static_cast<int32>(atol((char*)data.value));
        }

        uint64 GetUInt64() const
        {
            if (!data.value)
                return 0;

            #ifdef _DEBUG
            if (!IsType(MYSQL_TYPE_LONGLONG) && !IsType(MYSQL_TYPE_BIT))
            {
                _LOG_WARN(LOG_FILTER_SQL, "Warning: GetUInt64() on non-bigint field. Using type: %s.", FieldTypeToString(data.type));
                return 0;
            }
            #endif

            if (data.raw)
                return *reinterpret_cast<uint64*>(data.value);
            return static_cast<uint64>(atol((char*)data.value));
        }

        int64 GetInt64() const
        {
            if (!data.value)
                return 0;

            #ifdef _DEBUG
            if (!IsType(MYSQL_TYPE_LONGLONG) && !IsType(MYSQL_TYPE_BIT))
            {
                _LOG_WARN(LOG_FILTER_SQL, "Warning: GetInt64() on non-bigint field. Using type: %s.", FieldTypeToString(data.type));
                return 0;
            }
            #endif

            if (data.raw)
                return *reinterpret_cast<int64*>(data.value);
            return static_cast<int64>(strtol((char*)data.value, NULL, 10));
        }

        float GetFloat() const
        {
            if (!data.value)
                return 0.0f;

            #ifdef _DEBUG
            if (!IsType(MYSQL_TYPE_FLOAT))
            {
                _LOG_WARN(LOG_FILTER_SQL, "Warning: GetFloat() on non-float field. Using type: %s.", FieldTypeToString(data.type));
                return 0.0f;
            }
            #endif

            if (data.raw)
                return *reinterpret_cast<float*>(data.value);
            return static_cast<float>(atof((char*)data.value));
        }

        double GetDouble() const
        {
            if (!data.value)
                return 0.0f;

            #ifdef _DEBUG
            if (!IsType(MYSQL_TYPE_DOUBLE))
            {
                _LOG_WARN(LOG_FILTER_SQL, "Warning: GetDouble() on non-double field. Using type: %s.", FieldTypeToString(data.type));
                return 0.0f;
            }
            #endif

            if (data.raw)
                return *reinterpret_cast<double*>(data.value);
            return static_cast<double>(atof((char*)data.value));
        }

        char const* GetCString() const
        {
            if (!data.value)
                return NULL;

            #ifdef _DEBUG
            if (IsNumeric())
            {
                _LOG_WARN(LOG_FILTER_SQL, "Error: GetCString() on numeric field. Using type: %s.", FieldTypeToString(data.type));
                return NULL;
            }
            #endif
            return static_cast<char const*>(data.value);

        }

        std::string GetString() const
        {
            if (!data.value)
                return "";

            if (data.raw)
            {
                char const* string = GetCString();
                if (!string)
                    string = "";
                return std::string(string, data.length);
            }
            return std::string((char*)data.value);
        }

        bool IsNull() const
        {
            return data.value == NULL;
        }

    protected:
        Field();
        ~Field();

        #if defined(__GNUC__)
        #pragma pack(1)
        #else
        #pragma pack(push, 1)
        #endif
        struct
        {
            uint32 length;          // Length (prepared strings only)
            void* value;            // Actual data in memory
            enum_field_types type;  // Field type
            bool raw;               // Raw bytes? (Prepared statement or ad hoc)
         } data;
        #if defined(__GNUC__)
        #pragma pack()
        #else
        #pragma pack(pop)
        #endif

        void SetByteValue(void const* newValue, size_t const newSize, enum_field_types newType, uint32 length);
        void SetStructuredValue(char* newValue, enum_field_types newType);

        void CleanUp()
        {
            delete[] ((char*)data.value);
            data.value = NULL;
        }

        static size_t SizeForType(MYSQL_FIELD* field)
        {
            switch (field->type)
            {
                case MYSQL_TYPE_NULL:
                    return 0;
                case MYSQL_TYPE_TINY:
                    return 1;
                case MYSQL_TYPE_YEAR:
                case MYSQL_TYPE_SHORT:
                    return 2;
                case MYSQL_TYPE_INT24:
                case MYSQL_TYPE_LONG:
                case MYSQL_TYPE_FLOAT:
                    return 4;
                case MYSQL_TYPE_DOUBLE:
                case MYSQL_TYPE_LONGLONG:
                case MYSQL_TYPE_BIT:
                    return 8;

                case MYSQL_TYPE_TIMESTAMP:
                case MYSQL_TYPE_DATE:
                case MYSQL_TYPE_TIME:
                case MYSQL_TYPE_DATETIME:
                    return sizeof(MYSQL_TIME);

                case MYSQL_TYPE_TINY_BLOB:
                case MYSQL_TYPE_MEDIUM_BLOB:
                case MYSQL_TYPE_LONG_BLOB:
                case MYSQL_TYPE_BLOB:
                case MYSQL_TYPE_STRING:
                case MYSQL_TYPE_VAR_STRING:
                    return field->max_length + 1;

                case MYSQL_TYPE_DECIMAL:
                case MYSQL_TYPE_NEWDECIMAL:
                    return 64;

                case MYSQL_TYPE_GEOMETRY:
                /*
                Following types are not sent over the wire:
                MYSQL_TYPE_ENUM:
                MYSQL_TYPE_SET:
                */
                default:
                    _LOG_WARN(LOG_FILTER_SQL, "SQL::SizeForType(): invalid field type %u", uint32(field->type));
                    return 0;
            }
        }

        bool IsType(enum_field_types type) const
        {
            return data.type == type;
        }

        bool IsNumeric() const
        {
            return (data.type == MYSQL_TYPE_TINY ||
                    data.type == MYSQL_TYPE_SHORT ||
                    data.type == MYSQL_TYPE_INT24 ||
                    data.type == MYSQL_TYPE_LONG ||
                    data.type == MYSQL_TYPE_FLOAT ||
                    data.type == MYSQL_TYPE_DOUBLE ||
                    data.type == MYSQL_TYPE_LONGLONG );
        }

    private:
        #ifdef _DEBUG
        static char const* FieldTypeToString(enum_field_types type)
        {
            switch (type)
            {
                case MYSQL_TYPE_BIT:         return "BIT";
                case MYSQL_TYPE_BLOB:        return "BLOB";
                case MYSQL_TYPE_DATE:        return "DATE";
                case MYSQL_TYPE_DATETIME:    return "DATETIME";
                case MYSQL_TYPE_NEWDECIMAL:  return "NEWDECIMAL";
                case MYSQL_TYPE_DECIMAL:     return "DECIMAL";
                case MYSQL_TYPE_DOUBLE:      return "DOUBLE";
                case MYSQL_TYPE_ENUM:        return "ENUM";
                case MYSQL_TYPE_FLOAT:       return "FLOAT";
                case MYSQL_TYPE_GEOMETRY:    return "GEOMETRY";
                case MYSQL_TYPE_INT24:       return "INT24";
                case MYSQL_TYPE_LONG:        return "LONG";
                case MYSQL_TYPE_LONGLONG:    return "LONGLONG";
                case MYSQL_TYPE_LONG_BLOB:   return "LONG_BLOB";
                case MYSQL_TYPE_MEDIUM_BLOB: return "MEDIUM_BLOB";
                case MYSQL_TYPE_NEWDATE:     return "NEWDATE";
                case MYSQL_TYPE_NULL:        return "NULL";
                case MYSQL_TYPE_SET:         return "SET";
                case MYSQL_TYPE_SHORT:       return "SHORT";
                case MYSQL_TYPE_STRING:      return "STRING";
                case MYSQL_TYPE_TIME:        return "TIME";
                case MYSQL_TYPE_TIMESTAMP:   return "TIMESTAMP";
                case MYSQL_TYPE_TINY:        return "TINY";
                case MYSQL_TYPE_TINY_BLOB:   return "TINY_BLOB";
                case MYSQL_TYPE_VAR_STRING:  return "VAR_STRING";
                case MYSQL_TYPE_YEAR:        return "YEAR";
                default:                     return "-Unknown-";
            }
        }
        #endif
};

#endif

