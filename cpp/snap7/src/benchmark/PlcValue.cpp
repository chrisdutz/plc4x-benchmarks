#include "BaseTest.h"
#include "snap_platform.h"

// Default constructor
PlcValue::PlcValue() : type(PlcValueType::BOOL), value(nullptr) {
    value = new bool(false);
}

// Constructors for different types
PlcValue::PlcValue(bool val) : type(PlcValueType::BOOL) {
    value = new bool(val);
}

/*PlcValue::PlcValue(plcbyte val) : type(PlcValueType::BYTE) {
    value = new plcbyte(val);
}

PlcValue::PlcValue(plcshort val) : type(PlcValueType::WORD) {
    value = new plcshort(val);
}

PlcValue::PlcValue(plcint val) : type(PlcValueType::DWORD) {
    value = new plcint(val);
}

PlcValue::PlcValue(plclong val) : type(PlcValueType::LWORD) {
    value = new plclong(val);
}*/

PlcValue::PlcValue(int8_t val) : type(PlcValueType::SINT) {
    value = new int8_t(val);
}

PlcValue::PlcValue(int16_t val) : type(PlcValueType::INT) {
    value = new int16_t(val);
}

PlcValue::PlcValue(int32_t val) : type(PlcValueType::DINT) {
    value = new int32_t(val);
}

PlcValue::PlcValue(int64_t val) : type(PlcValueType::LINT) {
    value = new int64_t(val);
}

PlcValue::PlcValue(uint8_t val) : type(PlcValueType::USINT) {
    value = new uint8_t(val);
}

PlcValue::PlcValue(uint16_t val) : type(PlcValueType::UINT) {
    value = new uint16_t(val);
}

PlcValue::PlcValue(uint32_t val) : type(PlcValueType::UDINT) {
    value = new uint32_t(val);
}

PlcValue::PlcValue(uint64_t val) : type(PlcValueType::ULINT) {
    value = new uint64_t(val);
}

PlcValue::PlcValue(float val) : type(PlcValueType::REAL) {
    value = new float(val);
}

PlcValue::PlcValue(double val) : type(PlcValueType::LREAL) {
    value = new double(val);
}

PlcValue::PlcValue(char val) : type(PlcValueType::CHAR) {
    value = new char(val);
}

PlcValue::PlcValue(char16_t val) : type(PlcValueType::WCHAR) {
    value = new char16_t(val);
}

PlcValue::PlcValue(const std::string& val) : type(PlcValueType::STRING) {
    value = new std::string(val);
}

PlcValue::PlcValue(const std::u16string& val) : type(PlcValueType::WSTRING) {
    value = new std::u16string(val);
}

PlcValue::PlcValue(std::chrono::duration<double> val) : type(PlcValueType::TIME) {
    value = new std::chrono::duration<double>(val);
}

// Copy constructor
PlcValue::PlcValue(const PlcValue& other) : type(other.type) {
    copyValue(other);
}

// Assignment operator
PlcValue& PlcValue::operator=(const PlcValue& other) {
    if (this != &other) {
        freeValue();
        type = other.type;
        copyValue(other);
    }
    return *this;
}

// Destructor
PlcValue::~PlcValue() {
    freeValue();
}

// Get the type
PlcValueType PlcValue::getType() const {
    return type;
}

// Get the value (throws std::runtime_error if the type doesn't match)
bool PlcValue::getBool() const {
    if (type != PlcValueType::BOOL) {
        throw std::runtime_error("PlcValue is not a boolean");
    }
    return *static_cast<bool*>(value);
}

int8_t PlcValue::getInt8() const {
    if (type != PlcValueType::SINT) {
        throw std::runtime_error("PlcValue is not an int8");
    }
    return *static_cast<int8_t*>(value);
}

int16_t PlcValue::getInt16() const {
    if (type != PlcValueType::INT) {
        throw std::runtime_error("PlcValue is not an int16");
    }
    return *static_cast<int16_t*>(value);
}

int32_t PlcValue::getInt32() const {
    if (type != PlcValueType::DINT) {
        throw std::runtime_error("PlcValue is not an int32");
    }
    return *static_cast<int32_t*>(value);
}

int64_t PlcValue::getInt64() const {
    if (type != PlcValueType::LINT) {
        throw std::runtime_error("PlcValue is not an int64");
    }
    return *static_cast<int64_t*>(value);
}

uint8_t PlcValue::getUint8() const {
    if ((type != PlcValueType::USINT) && (type != PlcValueType::BYTE)) {
        throw std::runtime_error("PlcValue is not an uint8");
    }
    return *static_cast<uint8_t*>(value);
}

uint16_t PlcValue::getUint16() const {
    if ((type != PlcValueType::UINT) && (type != PlcValueType::WORD)) {
        throw std::runtime_error("PlcValue is not an uint16");
    }
    return *static_cast<uint16_t*>(value);
}

uint32_t PlcValue::getUint32() const {
    if ((type != PlcValueType::UDINT) && (type != PlcValueType::DWORD)) {
        throw std::runtime_error("PlcValue is not an uint32");
    }
    return *static_cast<uint32_t*>(value);
}

uint64_t PlcValue::getUint64() const {
    if ((type != PlcValueType::ULINT) && (type != PlcValueType::LWORD)) {
        throw std::runtime_error("PlcValue is not an uint64");
    }
    return *static_cast<uint64_t*>(value);
}

float PlcValue::getFloat() const {
    if (type != PlcValueType::REAL) {
        throw std::runtime_error("PlcValue is not a float");
    }
    return *static_cast<float*>(value);
}

double PlcValue::getDouble() const {
    if (type != PlcValueType::LREAL) {
        throw std::runtime_error("PlcValue is not a double");
    }
    return *static_cast<double*>(value);
}

char PlcValue::getChar() const {
    if (type != PlcValueType::CHAR) {
        throw std::runtime_error("PlcValue is not a char");
    }
    return *static_cast<char*>(value);
}

char16_t PlcValue::getChar16() const {
    if (type != PlcValueType::WCHAR) {
        throw std::runtime_error("PlcValue is not a wchar");
    }
    return *static_cast<char16_t*>(value);
}

std::string PlcValue::getString() const {
    if (type != PlcValueType::STRING) {
        throw std::runtime_error("PlcValue is not a string");
    }
    return *static_cast<std::string*>(value);
}

std::u16string PlcValue::getWstring() const {
    if (type != PlcValueType::WSTRING) {
        throw std::runtime_error("PlcValue is not a wstring");
    }
    return *static_cast<std::u16string*>(value);
}

std::chrono::duration<double> PlcValue::getDuration() const {
    if (type != PlcValueType::TIME) {
        throw std::runtime_error("PlcValue is not a wstring");
    }
    return *static_cast<std::chrono::duration<double>*>(value);
}

// Comparison operators
bool PlcValue::operator==(const PlcValue& other) const {
    if (type != other.type) {
        return false;
    }

    switch (type) {
        case PlcValueType::BOOL:
            return getBool() == other.getBool();
        case PlcValueType::SINT:
            return getInt8() == other.getInt8();
        case PlcValueType::INT:
            return getInt16() == other.getInt16();
        case PlcValueType::DINT:
            return getInt32() == other.getInt32();
        case PlcValueType::LINT:
            return getInt64() == other.getInt64();
        case PlcValueType::USINT:
            return getUint8() == other.getUint8();
        case PlcValueType::UINT:
            return getUint16() == other.getUint16();
        case PlcValueType::UDINT:
            return getUint32() == other.getUint32();
        case PlcValueType::ULINT:
            return getUint64() == other.getUint64();
        case PlcValueType::REAL:
            return getFloat() == other.getFloat();
        case PlcValueType::LREAL:
            return getDouble() == other.getDouble();
        case PlcValueType::CHAR:
            return getChar() == other.getChar();
        case PlcValueType::WCHAR:
            return getChar16() == other.getChar16();
        case PlcValueType::STRING:
            return getString() == other.getString();
        case PlcValueType::WSTRING:
            return getWstring() == other.getWstring();
        case PlcValueType::TIME:
            return getDuration() == other.getDuration();
        default:
            return false;
    }
}

bool PlcValue::operator!=(const PlcValue& other) const {
    return !(*this == other);
}

// Helper method to free the memory
void PlcValue::freeValue() {
    if (value) {
        switch (type) {
            case PlcValueType::BOOL:
                delete static_cast<bool*>(value);
                break;
            case PlcValueType::SINT:
                delete static_cast<int8_t*>(value);
                break;
            case PlcValueType::INT:
                delete static_cast<int16_t*>(value);
                break;
            case PlcValueType::DINT:
                delete static_cast<int32_t*>(value);
                break;
            case PlcValueType::LINT:
                delete static_cast<int64_t*>(value);
                break;
            case PlcValueType::BYTE:
            case PlcValueType::USINT:
                delete static_cast<uint8_t*>(value);
                break;
            case PlcValueType::WORD:
            case PlcValueType::UINT:
                delete static_cast<uint16_t*>(value);
                break;
            case PlcValueType::DWORD:
            case PlcValueType::UDINT:
                delete static_cast<uint32_t*>(value);
                break;
            case PlcValueType::LWORD:
            case PlcValueType::ULINT:
                delete static_cast<uint64_t*>(value);
                break;
            case PlcValueType::REAL:
                delete static_cast<float*>(value);
                break;
            case PlcValueType::LREAL:
                delete static_cast<double*>(value);
                break;
            case PlcValueType::CHAR:
                delete static_cast<char*>(value);
                break;
            case PlcValueType::WCHAR:
                delete static_cast<char16_t*>(value);
                break;
            case PlcValueType::STRING:
                delete static_cast<std::string*>(value);
                break;
            case PlcValueType::WSTRING:
                delete static_cast<std::u16string*>(value);
                break;
            case PlcValueType::TIME:
                delete static_cast<std::chrono::duration<double>(*)>(value);
                break;
        }
        value = nullptr;
    }
}

// Helper method to copy the value
void PlcValue::copyValue(const PlcValue& other) {
    switch (other.type) {
        case PlcValueType::BOOL:
            value = new bool(other.getBool());
            break;
        case PlcValueType::SINT:
            value = new int8_t(other.getInt8());
            break;
        case PlcValueType::INT:
            value = new int16_t(other.getInt16());
            break;
        case PlcValueType::DINT:
            value = new int32_t(other.getInt32());
            break;
        case PlcValueType::LINT:
            value = new int64_t(other.getInt64());
            break;
        case PlcValueType::BYTE:
        case PlcValueType::USINT:
            value = new uint8_t(other.getUint8());
            break;
        case PlcValueType::WORD:
        case PlcValueType::UINT:
            value = new uint16_t(other.getUint16());
            break;
        case PlcValueType::DWORD:
        case PlcValueType::UDINT:
            value = new uint32_t(other.getUint32());
            break;
        case PlcValueType::LWORD:
        case PlcValueType::ULINT:
            value = new uint64_t(other.getUint64());
            break;
        case PlcValueType::REAL:
            value = new float(other.getFloat());
            break;
        case PlcValueType::LREAL:
            value = new double(other.getDouble());
            break;
        case PlcValueType::CHAR:
            value = new char(other.getChar());
            break;
        case PlcValueType::WCHAR:
            value = new char16_t(other.getChar16());
            break;
        case PlcValueType::STRING:
            value = new std::string(other.getString());
            break;
        case PlcValueType::WSTRING:
            value = new std::u16string(other.getWstring());
            break;
        case PlcValueType::TIME:
            value = new std::chrono::duration<double>(other.getDuration());
            break;
    }
}
