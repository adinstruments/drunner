#ifndef __SERVICE_INSTALL
#define __SERVICE_INSTALL

#include <string>
#include "cresult.h"
#include "service_paths.h"
#include "service_lua.h"

class service_install : public servicePaths
{
public:
   service_install(std::string servicename);
   service_install(std::string servicename, std::string imagename);

   cResult obliterate();
   cResult uninstall();
   cResult recover();
   cResult update();
   cResult install();
   cResult service_restore(const std::string & backupfile);

private:
   void _createLaunchScript() const;
   void _createVolumes(std::vector<std::string> & volumes);
   void _ensureDirectoriesExist() const;
   cResult _removeLaunchScript() const;
   cResult _recreate();

   std::string mImageName;
};


#endif

