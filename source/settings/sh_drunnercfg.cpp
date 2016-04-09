#include "sh_drunnercfg.h"
#include "utils.h"

// can't put this in header because circular
// dependency then with utils::getTime.
sh_drunnercfg::sh_drunnercfg(const std::string & rootpath) :
   settingsbash_reader(rootpath+"/"+"drunnercfg.sh")
{
   setDefaults(rootpath);
   read();

   // migrate old settings.
   if (readOkay()) 
      if (utils::findStringIC(getRootUtilImage(), "install-rootutils"))
      {
         setString("ROOTUTILIMAGE", "drunner/rootutils");
         write();
      }
}

void sh_drunnercfg::setDefaults(const std::string & rootpath)
{
   setString("ROOTPATH", rootpath);
   setString("ROOTUTILIMAGE", "drunner/rootutils");
   setString("DRUNNERINSTALLURL", R"EOF(https://drunner.s3.amazonaws.com/drunner-install)EOF");
   setString("DRUNNERINSTALLTIME", utils::getTime());
   setBool("PULLIMAGES", true);
}

bool sh_drunnercfg::write()
{
   return writeSettings(getPath());
}
