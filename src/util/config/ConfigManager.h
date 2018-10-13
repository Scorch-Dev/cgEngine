#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <map>

#include "DbFrameAllocator.h"
#include "IoManager.h"
#include "SentinelAssert.h"
#include "StringId.h"
#include "SVar.h"

namespace sentinel
{

// builds up a map of SVar to configure
// per user settings and engine settings
class ConfigManager
{
public:
	void startUp(int argc, const char** argv, IoManager& io_manager, DbFrameAllocator& frame_allocator); //in case cli args
	void shutDown(IoManager& io_manager, DbFrameAllocator& frame_allocator);

	bool containsSVar(StrId name_id);
    SVar& getSVar(StrId name_sid);
    void addSVar(SVar sv);
	void removeSVar(StrId name_id);
private:

	void processConfigBuffer(char* buffer, std::size_t bytes_read);

	std::map<StrId, SVar> m_svars;

    //interned strings we'll need for faster parse
	static StrId m_engine_cfg_path_sid;
	static StrId m_user_cfg_path_sid;

    static std::size_t CFG_BUF_SIZE;
};

}//namespace sentinel

#endif //CONFIG_MANAGER_H
