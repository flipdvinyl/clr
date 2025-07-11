#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  /usr/local/bin/JUCE-8.0.8/juceaide header /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/JuceLibraryCode/Debug/Defs.txt /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/JuceLibraryCode/JuceHeader.h
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  /usr/local/bin/JUCE-8.0.8/juceaide header /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/JuceLibraryCode/Release/Defs.txt /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/JuceLibraryCode/JuceHeader.h
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  /usr/local/bin/JUCE-8.0.8/juceaide header /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/JuceLibraryCode/MinSizeRel/Defs.txt /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/JuceLibraryCode/JuceHeader.h
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/d/JUCEClearHost/ClearHost_artefacts
  /usr/local/bin/JUCE-8.0.8/juceaide header /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/JuceLibraryCode/RelWithDebInfo/Defs.txt /Users/d/JUCEClearHost/ClearHost_artefacts/ClearHost_artefacts/JuceLibraryCode/JuceHeader.h
fi

