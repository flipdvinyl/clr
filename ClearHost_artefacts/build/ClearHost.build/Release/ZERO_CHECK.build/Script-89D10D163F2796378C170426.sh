#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  make -f /Users/d/JUCEClearHost/ClearHost_artefacts/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  make -f /Users/d/JUCEClearHost/ClearHost_artefacts/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  make -f /Users/d/JUCEClearHost/ClearHost_artefacts/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  make -f /Users/d/JUCEClearHost/ClearHost_artefacts/CMakeScripts/ReRunCMake.make
fi

