#include "ConfigManager.h"

namespace sentinel
{

StrId ConfigManager::m_engine_cfg_path_sid = INTERN_STR("engine.cfg");
StrId ConfigManager::m_user_cfg_path_sid = INTERN_STR("user.cfg");

std::size_t ConfigManager::CFG_BUF_SIZE = 10240; //10kB should be enough

/**
 * parses config files on startup.
 * user.cfg is a personal config file for any user,
 * engine.cfg is an engine config file for engine features,
 */
void ConfigManager::startUp(int argc, const char** argv, IoManager& io_manager, DbFrameAllocator& frame_allocator)
{
	//open files + read in data async
	char* engine_cfg_read_buf = (char*) frame_allocator.alloc(CFG_BUF_SIZE);
	char* user_cfg_read_buf = (char*) frame_allocator.alloc(CFG_BUF_SIZE);

	FileHandle engine_cfg_f = io_manager.openFile(STR(m_engine_cfg_path_sid));
	FileHandle user_cfg_f = io_manager.openFile(STR(m_user_cfg_path_sid));

	AsyncJobHandle engine_cfg_read = io_manager.asyncRead(
            engine_cfg_f, engine_cfg_read_buf, CFG_BUF_SIZE, 
		[this, engine_cfg_read_buf](FileOpStatus status, std::size_t bytes_read)
		{
			S_ASSERT(status == success, "Error while reading config file");
			processConfigBuffer(engine_cfg_read_buf, bytes_read);
		});


	AsyncJobHandle user_cfg_read = io_manager.asyncRead(
            user_cfg_f, user_cfg_read_buf, CFG_BUF_SIZE,
		[this, user_cfg_read_buf](FileOpStatus status, std::size_t bytes_read)
		{
			S_ASSERT(status == success, "Error while reading config file");
			processConfigBuffer(user_cfg_read_buf, bytes_read);
		});

	//sit back and wait, then cleanup
	io_manager.waitAsyncIo(engine_cfg_read);
	io_manager.waitAsyncIo(user_cfg_read);

	io_manager.closeFile(engine_cfg_f);
	io_manager.closeFile(user_cfg_f);
}

/**
 * looks a lot like startUp except our callbacks are different.
 */
void ConfigManager::shutDown(IoManager& io_manager, DbFrameAllocator& frame_allocator)
{
	char* engine_cfg_write_buf = (char*)frame_allocator.alloc(CFG_BUF_SIZE);
	char* user_cfg_write_buf = (char*)frame_allocator.alloc(CFG_BUF_SIZE);

	FileHandle engine_cfg_f = io_manager.openFile(STR(m_engine_cfg_path_sid), true);
	FileHandle user_cfg_f = io_manager.openFile(STR(m_user_cfg_path_sid), true);

	//serialize to buffers
    int user_svar_count = 0;
    int engine_svar_count = 0;
    std::size_t svar_size = SVar::binarySize();
	for (auto iter = m_svars.begin(); iter != m_svars.end(); iter++)
	{
		SVar& sv = iter->second;
		if (sv.hasFlag(SVAR_PERSIST))
		{
            if(sv.hasFlag(USER_CONF))
            {
                sv.serialize(user_cfg_write_buf + (user_svar_count * svar_size));
                user_svar_count++;
            }
            else
            {
                sv.serialize(engine_cfg_write_buf + (engine_svar_count * svar_size));
                engine_svar_count++;
            }
		}

	}

	//dump our buffers to file
	AsyncJobHandle engine_cfg_write = io_manager.asyncWrite(
            engine_cfg_f, engine_cfg_write_buf, (engine_svar_count * svar_size),
		[](FileOpStatus status, std::size_t bytes_read)
		{
			S_ASSERT(status == success, "Error while reading config file");
		});

	AsyncJobHandle user_cfg_write = io_manager.asyncWrite(
            user_cfg_f, user_cfg_write_buf, (user_svar_count * svar_size),
		[](FileOpStatus status, std::size_t bytes_read)
		{
			S_ASSERT(status == success, "Error while reading config file");
		});


	//sit back and wait + cleanup
	io_manager.waitAsyncIo(engine_cfg_write);
	io_manager.waitAsyncIo(user_cfg_write);

	io_manager.closeFile(engine_cfg_f);
	io_manager.closeFile(user_cfg_f);
}

/**
 * contains check to prevent erroneous get statements
 * 
 * @param name_id, the StrId of the SVar
 * @returns true if contained, false otherwise
 */
bool ConfigManager::containsSVar(StrId name_id)
{
	return (m_svars.count(name_id) > 0);
}

/**
 * returns specified SVar
 *
 * @param name_sid: the sid of the SVar's name
 * @return SVar& requested.
 */
SVar& ConfigManager::getSVar(StrId name_sid)
{
    return m_svars.at(name_sid);
}

/**
 * adds an SVar to the internal map
 * if it's not already present. If it
 * is present, does nothing.
 *
 * @param sv: the SVar to add
 */
void ConfigManager::addSVar(SVar sv)
{
    if(m_svars.count(sv.getNameStrId()) == 0)
        m_svars.emplace(sv.getNameStrId(), sv);
}

/**
 * removes an SVar from the map
 *
 * @param name_id: the StrId of the SVar to remove
 */
void ConfigManager::removeSVar(StrId name_id)
{
	m_svars.erase(name_id);
}

/**
 * helper that adds SVars to memory based on
 * a buffer from a binary cfg file.
 *
 * @param buffer: the buffer to parse
 * @param bytes_read: bytes in buffer (amount returned by the io read,
		not the total buf size)
 */
void ConfigManager::processConfigBuffer(char* buffer, 
        std::size_t bytes_read)
{
    S_ASSERT(bytes_read % 68 == 0, 
            "bytes read not a multiple of serialized SVar size");
    int num_items = bytes_read / 68;
    for(int i = 0; i < num_items; i++)
    {
        SVar sv = SVar::deserialize( (buffer + (i*68)) );
        addSVar(sv);
    }
}


}//namespace sentinel
