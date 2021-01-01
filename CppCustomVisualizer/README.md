# Dave's Visual Studio Debugger Visualizer
This is an enhanced version of the CppCustomVisualizer in the [Microsoft Concord Extensibility Sample](https://github.com/Microsoft/ConcordExtensibilitySamples/wiki/Cpp-Custom-Visualizer-Sample).

Variables of the following types are normally displayed in raw forms by the debugger, but as you can see in the following examples, with this extension they're displayed in more useful formats.


#### FILETIME, SYSTEMTIME
Displays the time value interpreted as both UTC and local time, shown in your current locale format.  

![FILETIME & SYSTEMTIME demonstration animation](ft-st-demo.gif)
***
#### COleDateTime, CTime, CTimeSpan
COleDateTime and CTime show the same format as FILETIME and SYSTEMTIME above.  
CTimeSpan shows the time value as Days, Hours, Minutes, and Seconds.

![COleDateTime, CTime, & CTimeSpan demonstration animation](timeclasses-demo.gif)
***
#### PROPERTYKEY  
Displays the key's canonical and display names.    

![PROPERTYKEY demonstration animation](propkey-demo.gif)
***
#### LOGFONT(A/W)
Displays the commonly used LOGFONT structure members in an easy to interpret format.

![LOGFONT demonstration animation](logfont-demo.gif)
***
### Original project on which this is based
The original CppCustomVisualizer only handles the FILETIME type, and only shows a local format of it.

More information can be found in the [Wiki for that project](https://github.com/Microsoft/ConcordExtensibilitySamples/wiki/Cpp-Custom-Visualizer-Sample).

Natvis documentation can be found on [docs.microsoft.com](https://docs.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects).

## Notes on Changes to the Original example
### dll folder

#### _EntryPoint.cpp  
Defined new GUIDs for the type visualizers & calls the appropriate converter when the matching GUID is passed (*from the debugger*).

#### FileAndSystemTimeViz.cpp/.h  
Converter for FILETIME & SYSTEMTIME -> String

#### PropertyKeyViz.cpp/.h  
Converter for PROPERTYKEY -> String

#### FontInfoViz.cpp/.h
Converter for LOGFONT(A/W) -> String

#### TargetApp folder  
Revised application to illustrate the types handled by this extension.

