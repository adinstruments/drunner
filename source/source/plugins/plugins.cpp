#include <Poco/String.h>



#include "plugins.h"
#include "generate_plugin_script.h"
#include "globalcontext.h"
#include "globallogger.h"
#include "enums.h"

#include "dbackup.h"
#include "ddev.h"
#include "dassert.h"

plugins::plugins()
{
   mPlugins.push_back(std::unique_ptr<plugin>(new dbackup()));
   mPlugins.push_back(std::unique_ptr<plugin>(new ddev()));
}

void plugins::generate_plugin_scripts() const
{
   for (auto p = mPlugins.begin() ; p != mPlugins.end() ; ++p)
      generate_plugin_script(p->get()->getName());
}

cResult plugins::runcommand() const
{
   std::string pluginname = GlobalContext::getParams()->getCommandStr();

   for (auto p = mPlugins.begin(); p != mPlugins.end(); ++p)
      if (0==Poco::icompare(p->get()->getName(), pluginname))
        return p->get()->runCommand();

   return cError("Unknown plugin '" + pluginname + "'");
}

// -----------------------------------


pluginhelper::pluginhelper(std::string name) : mName(name)
{
}

pluginhelper::~pluginhelper()
{
}

cResult pluginhelper::runCommand() const
{
   CommandLine cl;

   if (GlobalContext::getParams()->numArgs() == 0)
      cl.command = "help";
   else
   {
      cl.args = GlobalContext::getParams()->getArgs();
      cl.command = cl.args[0];
      cl.args.erase(cl.args.begin());
   }

   if (cl.command == "help")
      return showHelp();
   
   Poco::Path spath = configurationFilePath();
   persistvariables pv(mName, spath);
   pv.setConfiguration(mConfiguration);
   cResult r = pv.loadvariables();
   
   if (cl.command == "configure")
      return pv.handleConfigureCommand(cl);
   else
      return runCommand(cl, pv.getVariables());
}

cResult pluginhelper::addConfig(std::string name, std::string description, std::string defaultval, configtype type, bool required)
{
   Configuration c;
   c.name = name;
   c.description = description;
   c.defaultval = defaultval;
   c.type = type;
   c.required = required;

   mConfiguration.push_back(c);
   return kRSuccess;
}

