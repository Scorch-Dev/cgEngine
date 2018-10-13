#include "SVar.h"

namespace sentinel
{

const std::size_t SVar::SVAR_MAX_STR_LEN = 32;

/**
 * ctor
 */
SVar::SVar(StrId name_sid, float val, std::uint32_t flag_mask) :
	m_name_sid(name_sid),
	m_value(val),
	m_flags(flag_mask)
{
	//protect against incorrectly setting SVAR_NUMERIC in flag_mask
	setFlag(SVAR_NUMERIC);
}

/**
* ctor
*/
SVar::SVar(StrId name_sid, const StrId val, std::uint32_t flag_mask) :
	m_name_sid(name_sid),
	m_value(val),
	m_flags(flag_mask)
{
	//protect against incorrectly setting SVAR_NUMERIC in flag_mask
	clearFlag(SVAR_NUMERIC);
}

/******Getters and Setters******/
float SVar::getFloatVal()
{
	return m_value.f;
}

void SVar::setFloatVal(float val)
{
	setFlag(SVAR_NUMERIC);
	m_value.f = val;
}

StrId SVar::getStrIdVal()
{
	return m_value.sid;
}

void SVar::setStrIdVal(StrId val)
{
	clearFlag(SVAR_NUMERIC);
	m_value.sid = val;
}

StrId SVar::getNameStrId()
{
	return m_name_sid;
}

std::uint32_t SVar::getAllFlags()
{
	return m_flags;
}

void SVar::setFlag(SVarFlag flag)
{
	m_flags = (m_flags | flag);
}

void SVar::clearFlag(SVarFlag flag)
{
	m_flags = ( m_flags & (~flag) );
}

bool SVar::hasFlag(SVarFlag flag) 
{
	return (m_flags & flag);
}

/**
 * serializes the object to a buffer of fixed size 68.
 * any string data is retrieved from internment and
 * serialized properly, but maximum string length is 32
 * per string.
 * 
 * @param buffer: the char[] buffer to write to
 */
void SVar::serialize(char* buffer)
{
    //serialize name
    const std::string& name = STR(m_name_sid);
    S_ASSERT( (name.length() < (SVAR_MAX_STR_LEN - 1)),
            "max str len exceed for SVar name value");
    STR(m_name_sid).copy(buffer, (SVAR_MAX_STR_LEN - 1), 0);
    buffer[STR(m_name_sid).size()] = '\0';

    //store value field
    if(hasFlag(SVAR_NUMERIC))
    {
        float f = getFloatVal();
        char* f_val_bytes = reinterpret_cast<char*>(&f);
        for(unsigned int i = 0; i < sizeof(float); i++)
            buffer[SVAR_MAX_STR_LEN + i] = *(f_val_bytes + i);
    }
    else
    {
        S_ASSERT(STR(m_name_sid).length() < (SVAR_MAX_STR_LEN - 1),
                "max str len exceed for SVar str value field");

        std::string str_val = STR(getStrIdVal());
        str_val.copy(buffer + SVAR_MAX_STR_LEN, str_val.length(), 0);
        buffer[SVAR_MAX_STR_LEN+STR(getStrIdVal()).length()] = '\0';
    }

    //store flag_mask
    std::uint32_t flag_mask = getAllFlags();
    char* flag_mask_bytes = reinterpret_cast<char*>(&flag_mask);
    for(unsigned int i = 0; i < 4; i++)
        buffer[(SVAR_MAX_STR_LEN*2) + i] = *(flag_mask_bytes + i);
}

/**
 * Constructs an SVar from buffer written to 
 * by serialize() at any point in the past.
 * 
 * @param buffer: char* containing a serialized SVar
 * @returns an SVar from this.
 */
SVar SVar::deserialize(const char* buffer)
{
    //split bytes
    char name[SVAR_MAX_STR_LEN];
    char value[SVAR_MAX_STR_LEN];
    char flag_mask[4];

    for(int i = 0; i < SVAR_MAX_STR_LEN; i++)
        name[i] = buffer[i];

    for(int i = 0; i < SVAR_MAX_STR_LEN; i++)
        value[i] = buffer[SVAR_MAX_STR_LEN + i];

    for(int i = 0; i < 4; i++)
        flag_mask[i] = buffer[(SVAR_MAX_STR_LEN*2) + i];

    //now create the SVar
    StrId name_sid = INTERN_STR(std::string(name));

    std::uint32_t flags = *(
            reinterpret_cast<std::uint32_t*>(flag_mask) );

    if( flags & SVAR_NUMERIC )
    {
        float float_val = *(
                reinterpret_cast<float*>(&value) );
        return SVar(name_sid, float_val, flags);
    }
    else
    {
        StrId sid_val = INTERN_STR(std::string(value));
        return SVar(name_sid, sid_val, flags);
    }
}

std::size_t SVar::binarySize()
{
    return 68; //clearer than a global var
}

}//namespace sentinel
