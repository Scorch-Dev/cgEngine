#ifndef SVAR_H
#define SVAR_H

#include <string>

#include "StringId.h"

namespace sentinel
{

enum SVarFlag : std::uint32_t
{
	SVAR_PERSIST = 1,             //saves var to config file
	USER_CONF = (1 << 1),         //if is a per user config
	SVAR_NUMERIC = (1 << 2),      //indicates numeric/str val
};

/**
 * A union wrpaper for data & some options
 * to be passed from the command line or configuration files.
 */
class SVar
{
public:
	SVar(StrId name_sid, float val, std::uint32_t flag_mask);
	SVar(StrId name_sid, StrId val, std::uint32_t flag_mask);

	StrId getNameStrId();

	//data
	float getFloatVal();
	void setFloatVal(float val);
	
	StrId getStrIdVal();
	void setStrIdVal(StrId val);

	//flags
	std::uint32_t getAllFlags();
	void setFlag(SVarFlag flag);
	void clearFlag(SVarFlag flag);
	bool hasFlag(SVarFlag flag);

	//save to config
    void serialize(char* buffer);
    static SVar deserialize(const char* buffer);

    static std::size_t binarySize();

private:

	union ValueType
	{
		float f;
		StrId sid;

		ValueType(float val) : f(val) {}
		ValueType(StrId val) : sid(val) {}
		~ValueType() {}
	};

	StrId m_name_sid;
	ValueType m_value;
	std::uint32_t m_flags;

    static const std::size_t SVAR_MAX_STR_LEN;
};

} //namespace sentinel

#endif //S_VAR_H
