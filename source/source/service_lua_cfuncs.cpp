#include "service_lua.h"
#include "dassert.h"
#include "utils_docker.h"

// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

// lua C functions - defined in service_lua_cfuncs
extern "C" int l_addconfig(lua_State *L);
extern "C" int l_addvolume(lua_State *L);
extern "C" int l_addcontainer(lua_State *L);
extern "C" int l_drun(lua_State *L);
extern "C" int l_dstop(lua_State *L);

#define REGISTERLUAC(cfunc,luaname) lua_pushcfunction(L, cfunc); lua_setglobal(L, luaname);

namespace servicelua
{

   // -----------------------------------------------------------------------------------------------------------------------

   void _register_lua_cfuncs(lua_State *L)
   { // define all our C functions that we want available from service.lua
      REGISTERLUAC(l_addconfig, "addconfig")
      REGISTERLUAC(l_addvolume, "addvolume")
      REGISTERLUAC(l_addcontainer, "addcontainer")
      REGISTERLUAC(l_drun, "drun")
      REGISTERLUAC(l_dstop, "dstop")
   }

   // -----------------------------------------------------------------------------------------------------------------------


   luafile * get_luafile(lua_State *L)
   {
      int stacksize = lua_gettop(L);
      lua_getglobal(L, "luafile");
      drunner_assert(lua_islightuserdata(L, -1), "Is not lightuserdata.");
      void * luafilevoid = lua_touserdata(L, -1); 
      drunner_assert(luafilevoid != NULL, "`luafile' expected");
      lua_pop(L, 1); // pop top element
      drunner_assert(stacksize == lua_gettop(L), "Stack unbalanced : "+std::to_string(stacksize)+" != "+std::to_string(lua_gettop(L)));
      return (luafile *)luafilevoid;
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_addconfig(lua_State *L)
   {
      if (lua_gettop(L) != 5)
         return luaL_error(L, "Expected exactly one argument (the docker container to stop) for addconfig.");

      Configuration c;
      c.name = lua_tostring(L, 1);
      c.description = lua_tostring(L, 2);
      c.defaultval = lua_tostring(L, 3);
      c.required = (lua_toboolean(L, 5) == 1);

      {
         using namespace utils;
         switch (s2i(lua_tostring(L, 4)))
         {
         case s2i("port"):
            c.type = kCF_port;
            break;
         case s2i("path"):
            c.type = kCF_path;
            break;
         case s2i("existingpath"):
            c.type = kCF_existingpath;
            break;
         case s2i("string"):
            c.type = kCF_string;
            break;
         default:
            fatal("Unknown configuration type: " + std::string(lua_tostring(L, 4)));
         }
      }

      get_luafile(L)->addConfiguration(c);

      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return.
   }

   // -----------------------------------------------------------------------------------------------------------------------

   int _luasuccess(lua_State *L)
   {
      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return.
   }

   // -----------------------------------------------------------------------------------------------------------------------


   extern "C" int l_addvolume(lua_State *L)
   {
      if (lua_gettop(L) < 1 || lua_gettop(L)>3)
         return luaL_error(L, "Expected one to three arguments: (name, backup, external) for addvolume.");

      luafile * lf = get_luafile(L);

      Volume v;
      v.name = lf->getVariables().substitute(lua_tostring(L, 1)); // first argument. http://stackoverflow.com/questions/29449296/extending-lua-check-number-of-parameters-passed-to-a-function
      v.backup = true;
      v.external = false;

      if (lua_gettop(L)>1)
         v.backup = (1 == lua_toboolean(L, 2));

      if (lua_gettop(L)>2)
         v.external = (1 == lua_toboolean(L, 3));

      lf->addVolume(v);
      logmsg(kLDEBUG, "Added volume " + v.name);

      return _luasuccess(L);
   }


   // -----------------------------------------------------------------------------------------------------------------------


   extern "C" int l_addcontainer(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the name of the container) for addcontainer.");
      std::string cname = lua_tostring(L, 1); // first argument. http://stackoverflow.com/questions/29449296/extending-lua-check-number-of-parameters-passed-to-a-function

      get_luafile(L)->addContainer(cname);

      return _luasuccess(L);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_drun(lua_State *L)
   {
      if (lua_gettop(L) < 1)
         return luaL_error(L, "Expected at least one argument: drun( command,  arg1, arg2, ... )");

      luafile * lf = get_luafile(L);

      CommandLine operation;
      operation.command = lua_tostring(L, 1);
      for (int i = 2; i <= lua_gettop(L); ++i)
         operation.args.push_back(lf->getVariables().substitute(lua_tostring(L, i)));

      Poco::Path runin = lf->getPathdService();

      utils::runcommand_stream(operation, kORaw, runin, lf->getVariables().getAll());

      return _luasuccess(L);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dstop(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the container name to stop) for dstop.");
      std::string containerraw = lua_tostring(L, 1);

      luafile *lf = get_luafile(L);
      std::string subcontainer = lf->getVariables().substitute(containerraw);

      if (utils_docker::dockerContainerExists(subcontainer))
      {
         logmsg(kLINFO, "Stopping " + lf->getServiceName());
         utils_docker::stopContainer(subcontainer);
         utils_docker::removeContainer(subcontainer);
      }
      else
         logmsg(kLDEBUG, subcontainer + " is not running.");
      return _luasuccess(L);
   }

}
