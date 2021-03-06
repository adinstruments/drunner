#include "validateimage.h"
#include "utils.h"
#include "drunner_paths.h"
#include "globallogger.h"
#include "utils_docker.h"

namespace validateImage
{
   void validate(std::string imagename)
   {
      if (utils::imageisbranch(imagename))
         logmsg(kLDEBUG, imagename + " looks like a development branch (won't be pulled).");
      else
         logmsg(kLDEBUG, imagename + " looks like a production image.");

      std::string data = R"EOF(#!/bin/bash
set -o nounset
set -e
# Validate the image I'm running in as being Docker Runner compatible

function die { echo "Not dRunner compatible - $1"; exit 1 ; }

if [ "$UID" -eq 0 ]; then die "the container runs as root." ; fi

# Check mandatory files in image before touching host. Is it a valid dService?
[ -e "/drunner/service.lua" ] || die "does not have service.lua file."

[ ! -e "/drunner/servicecfg.sh" ] || \
   die "Outdated dService with servicecfg.sh (no longer supported)."

[ ! -e "/drunner/servicerunner" ] || \
   echo "Backward compatible dService with deprecated servicerunner."

exit 0
)EOF";

      std::string op;
      if (utils_docker::runBashScriptInContainer(data, imagename, op) != kRSuccess)
      {
         if (utils::findStringIC(op, "Unable to find image"))
            logmsg(kLERROR, "Couldn't find image " + imagename);
         else
            logmsg(kLERROR, "Bash script failed in container:\n"+op);
      }
      logdbg_trim(op);

#ifdef _WIN32
      logmsg(kLINFO, imagename + " is dRunner compatible.");
#else
      logmsg(kLINFO, "\u2714  " + imagename + " is dRunner compatible.");
#endif      
   }

}