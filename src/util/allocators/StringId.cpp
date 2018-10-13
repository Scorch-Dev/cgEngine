#include "StringId.h"

namespace sentinel
{

/**
 * interns a string into the global table.
 * does nothing but if the string is already interned
 * 
 * @param str: the string to intern
 * @return StrId of the string
 */
StrId StringId::internStr(const std::string& str)
{
	static std::hash<std::string> s_hash;

	//insert if not already interned
	StrId str_id = s_hash(str);
	if (getSidMap().count(str_id) == 0)
		getSidMap().emplace(str_id, std::string(str));

	return str_id;
}

/**
 * returns the string value that was interned
 * by a string id.
 *
 * @param sid: the sid to use as the key
 * @returns a const std::string& to the original std::string
 */
const std::string& StringId::getStr(StrId sid)
{
	S_ASSERT(getSidMap().count(sid) > 0,
		"you need to intern strings before retrieving them");

	return getSidMap()[sid];
}

/**
 * avoid static initialization issues, returns
 * a function static map& to use for static inits
 * or otherwise.
 */
std::map<StrId, std::string>& StringId::getSidMap() {
	static std::map<StrId, std::string> s_sid_map;
	return s_sid_map;
}

} //namespace sentinel
