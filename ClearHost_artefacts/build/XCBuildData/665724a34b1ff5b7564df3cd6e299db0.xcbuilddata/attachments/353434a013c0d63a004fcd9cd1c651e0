#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  /opt/homebrew/bin/cmake -E copy_directory /Users/d/JUCEClearHost/Resources /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/Debug/Resources
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  /opt/homebrew/bin/cmake -E copy /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/Debug/ClearHost /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  /opt/homebrew/bin/cmake -E copy_directory /Users/d/JUCEClearHost/Resources /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/Release/Resources
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  /opt/homebrew/bin/cmake -E copy /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/Release/ClearHost /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  /opt/homebrew/bin/cmake -E copy_directory /Users/d/JUCEClearHost/Resources /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/MinSizeRel/Resources
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  /opt/homebrew/bin/cmake -E copy /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/MinSizeRel/ClearHost /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  /opt/homebrew/bin/cmake -E copy_directory /Users/d/JUCEClearHost/Resources /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/RelWithDebInfo/Resources
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  /opt/homebrew/bin/cmake -E copy /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/RelWithDebInfo/ClearHost /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost
fi

