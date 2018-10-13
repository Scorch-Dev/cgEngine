#ifndef STRING_ID_H
#define STRING_ID_H

#include <functional>
#include <string>
#include <cstddef>
#include <map>

#include "SentinelAssert.h"

namespace sentinel
{
	typedef std::size_t StrId;

class StringId
{
public:
	static StrId internStr(const std::string& str);
	static const std::string& getStr(StrId sid);
private:
	/*function to avoid static initialization issues*/
	static std::map<StrId, std::string>& getSidMap();
};

}

#define INTERN_STR(str) sentinel::StringId::internStr(str)
#define STR(sid) sentinel::StringId::getStr(sid)

#endif // STRING_ID_H
