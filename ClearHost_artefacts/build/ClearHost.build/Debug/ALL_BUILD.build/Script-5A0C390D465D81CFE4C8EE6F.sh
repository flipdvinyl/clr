#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  echo Build\ all\ projects
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  echo Build\ all\ projects
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  echo Build\ all\ projects
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  echo Build\ all\ projects
fi

