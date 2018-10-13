#define CATCH_CONFIG_MAIN

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <cstring>

#include "catch.hpp"

#include "DbFrameAllocator.h"
#include "IoManager.h"
#include "ConfigManager.h"
#include "StringId.h"
#include "SVar.h"

namespace s_test
{

using namespace sentinel;

TEST_CASE("Try to read/write config files.")
{
	static DbFrameAllocator allocator;
 	static IoManager io;
    static ConfigManager conf;

	//startup with some faked args
	//(using cli for catch2 is surprisingly cumbersome/overkill
    io.startUp();
	char** fake_args = (char**) malloc(sizeof(char*) * 3);
	for (int i = 0; i < 2; i++)
	{
		fake_args[i] = (char*) malloc(sizeof(char) * 128);
		strcpy(fake_args[i], "hi");
	}
	fake_args[2] = nullptr;

    conf.startUp(2, const_cast<const char**>(fake_args), io, allocator);
	allocator.clearCurrentBuffer();

    //add several persistent svars
    SECTION("trying adding several (persistent) SVars")
    {
        SECTION("add engine SVars called test_svar1/2, w/ value 32 &\
                'hello world' respectively")
        {
            SVar test_svar1(INTERN_STR("test_svar1"), 32.0f, SVAR_PERSIST);
            conf.addSVar(test_svar1);

            SVar test_svar2(INTERN_STR("test_svar2"), 
                    INTERN_STR("hello world"), SVAR_PERSIST);
            conf.addSVar(test_svar2);
        }

        SECTION("add user SVars called test_svar3/4, w/ value 32 &\
                'hello world' respectively")
        {
            SVar test_svar3(INTERN_STR("test_svar3"), 32.0f, USER_CONF|SVAR_PERSIST);
            conf.addSVar(test_svar3);

            SVar test_svar4(INTERN_STR("test_svar4"), 
                    INTERN_STR("hello world"), USER_CONF|SVAR_PERSIST);
            conf.addSVar(test_svar4);
        }
    }

    //test that loaded up svars match ones persisted from last run
    SECTION("ensure SVars loaded up properly on new run")
    {
        SVar& loaded_sv1 = conf.getSVar(INTERN_STR("test_svar1"));
        REQUIRE(loaded_sv1.getFloatVal() == 32.0f);

        SVar& loaded_sv2 = conf.getSVar(INTERN_STR("test_svar2"));
        REQUIRE(loaded_sv2.getStrIdVal() == INTERN_STR("hello world"));

        SVar& loaded_sv3 = conf.getSVar(INTERN_STR("test_svar1"));
        REQUIRE(loaded_sv3.getFloatVal() == 32.0f);

        SVar& loaded_sv4 = conf.getSVar(INTERN_STR("test_svar2"));
        REQUIRE(loaded_sv4.getStrIdVal() == INTERN_STR("hello world"));

        SECTION("create a second test_svar1 with value 42.0, to test overwrite protection")
        {
            SVar test_svar5(INTERN_STR("test_svar1"), 42.0f, SVAR_PERSIST);
            conf.addSVar(test_svar5);
            SVar& loaded_sv5 = conf.getSVar(INTERN_STR("test_svar1"));
            REQUIRE(loaded_sv5.getFloatVal() == 32.0f);
        }
    }

    //clear the flags so we don't pollute our config
    SECTION("clear SVAR_PERSIST flags for cleanup")
    {
        SVar& loaded_sv1 = conf.getSVar(INTERN_STR("test_svar1"));
        SVar& loaded_sv2 = conf.getSVar(INTERN_STR("test_svar2"));
        SVar& loaded_sv3 = conf.getSVar(INTERN_STR("test_svar3"));
        SVar& loaded_sv4 = conf.getSVar(INTERN_STR("test_svar4"));

        loaded_sv1.clearFlag(SVAR_PERSIST);
        loaded_sv2.clearFlag(SVAR_PERSIST);
        loaded_sv3.clearFlag(SVAR_PERSIST);
        loaded_sv4.clearFlag(SVAR_PERSIST);
    }

	//shutdown sequence in reverse
	for (int i = 0; i < 3; i++)
	{
		free(fake_args[i]);
	}
	free(fake_args);


	conf.shutDown(io, allocator);
	io.shutDown();
}

} //namespace s_test
