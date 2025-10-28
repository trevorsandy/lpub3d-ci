; LPub3D Install Files Script Include
; Last Update: October 29, 2025
; Copyright (C) 2016 - 2025 by Trevor SANDY

; Universal architecture (x86, x86_64)
; First, delete files with old names if exist
IfFileExists "quazip.dll" 0 DeleteLdrawini
Delete "quazip.dll"
DeleteLdrawini:
IfFileExists "ldrawini.dll" 0 ContinueInstall
Delete "ldrawini.dll"
ContinueInstall:

; Check if Universal or SingleArchitecture Build
StrCmp ${UniversalBuild} "1" 0 SingleArchitectureBuild

; Perform appropriate installation
${If} ${RunningX64}
Goto RunX64Architecture
${Else}
Goto RunX86Architecture
${EndIf}

RunX64Architecture:
; Delete x86_64 files with old names if exist
IfFileExists "${ProductName}_x64.exe" 0 ContinueX64Install
Delete "${ProductName}_x64.exe"
ContinueX64Install:

!ifdef X64_INSTALL
; MSVC Redistributable
!ifdef ARM64_INSTALL
File "${Win64BuildDir}\vc_redist.arm64.exe"
!else
File "${Win64BuildDir}\vc_redist.x64.exe"
!endif

; Deposit new 64bit files...

; Executable
File "${Win64BuildDir}\${LPub3DBuildFile}"
; Program database (pdb)
File "${Win64BuildDir}\${ProductName}.pdb"

; OpenSSL - legacy manually placed ssl library files
!ifdef WIN64_SSL
File "${Win64BuildDir}\${OpenSSL64LibCrypto}"
File "${Win64BuildDir}\${OpenSSL64LibSSL}"
!endif

; Qt Direct Dependencies Libs
File "${Win64BuildDir}\Qt6Concurrent.dll"
File "${Win64BuildDir}\Qt6Core.dll"
File "${Win64BuildDir}\Qt6Core5Compat.dll"
File "${Win64BuildDir}\Qt6Gui.dll"
File "${Win64BuildDir}\Qt6Network.dll"
File "${Win64BuildDir}\Qt6OpenGL.dll"
File "${Win64BuildDir}\Qt6OpenGLWidgets.dll"
File "${Win64BuildDir}\Qt6PrintSupport.dll"
File "${Win64BuildDir}\Qt6Svg.dll"
File "${Win64BuildDir}\Qt6Widgets.dll"
File "${Win64BuildDir}\Qt6Xml.dll"
File "${Win64BuildDir}\Qt6Core5Compat.dll"

; Qt Plugins Generic
CreateDirectory "$INSTDIR\generic"
SetOutPath "$INSTDIR\generic"
File "${Win64BuildDir}\generic\qtuiotouchplugin.dll"

; Qt Plugins Icon Engines
CreateDirectory "$INSTDIR\iconengines"
SetOutPath "$INSTDIR\iconengines"
File "${Win64BuildDir}\iconengines\qsvgicon.dll"

; Qt Plugins Image Formats
CreateDirectory "$INSTDIR\imageformats"
SetOutPath "$INSTDIR\imageformats"
File "${Win64BuildDir}\imageformats\qgif.dll"
File "${Win64BuildDir}\imageformats\qico.dll"
File "${Win64BuildDir}\imageformats\qjpeg.dll"
File "${Win64BuildDir}\imageformats\qsvg.dll"

; Qt Plugins Network Information
CreateDirectory "$INSTDIR\networkinformation"
SetOutPath "$INSTDIR\networkinformation"
File "${Win64BuildDir}\networkinformation\qnetworklistmanager.dll"

; Qt Plugins Platforms
CreateDirectory "$INSTDIR\platforms"
SetOutPath "$INSTDIR\platforms"
File "${Win64BuildDir}\platforms\qwindows.dll"

; Qt Plugins Styles
CreateDirectory "$INSTDIR\styles"
SetOutPath "$INSTDIR\styles"
File "${Win64BuildDir}\styles\qmodernwindowsstyle.dll"

; Qt Plugins Tls
CreateDirectory "$INSTDIR\tls"
SetOutPath "$INSTDIR\tls"
File "${Win64BuildDir}\tls\qcertonlybackend.dll"
File "${Win64BuildDir}\tls\qopensslbackend.dll"
File "${Win64BuildDir}\tls\qschannelbackend.dll"
!endif

!ifndef ARM64_INSTALL
; OpenGL Library
File "${Win64BuildDir}\opengl32sw.dll"

; DirectX Compiler Libraries
File "${Win64BuildDir}\d3dcompiler_47.dll"
!endif
File "${Win64BuildDir}\dxcompiler.dll"
File "${Win64BuildDir}\dxil.dll"

; Translation library
File "${Win64BuildDir}\icuuc.dll"

; 3rd party renderer executable -LDGLite
${!defineifexist} INS_FILE_EXISTS "${Win64BuildDir}\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LDGliteDir}\bin"
SetOutPath "$INSTDIR\3rdParty\${LDGliteDir}\bin"
File /NONFATAL "${Win64BuildDir}\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"
!undef INS_FILE_EXISTS
!endif

; 3rd party renderer executable - LDView
${!defineifexist} INS_FILE_EXISTS "${Win64BuildDir}\3rdParty\${LDViewDir}\bin\${LDViewExe}64.exe"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\bin"
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\bin"
File /NONFATAL "${Win64BuildDir}\3rdParty\${LDViewDir}\bin\${LDViewExe}64.exe"
!undef INS_FILE_EXISTS
!endif

; 3rd party renderer executable - LPub3DTrace
${!defineifexist} INS_FILE_EXISTS "${Win64BuildDir}\3rdParty\${LPub3DTraceDir}\bin\${LPub3DTraceExe}64.exe"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin"
File /NONFATAL "${Win64BuildDir}\3rdParty\${LPub3DTraceDir}\bin\${LPub3DTraceExe}64.exe"
!undef INS_FILE_EXISTS
!endif

${If} $OverwriteUserDataParamFiles == 0
 SetOverwrite off
${EndIf}

; 3rd party renderer configuration - LDView
${!defineifexist} INS_FILE_EXISTS "${Win64BuildDir}\3rdParty\${LDViewDir}\resources\config\ldview.ini"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
File /NONFATAL "${Win64BuildDir}\3rdParty\${LDViewDir}\resources\config\ldview.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${Win64BuildDir}\3rdParty\${LDViewDir}\resources\config\ldviewPOV.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${Win64BuildDir}\3rdParty\${LDViewDir}\resources\config\ldviewPOV.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${Win64BuildDir}\3rdParty\${LDViewDir}\resources\config\LDViewCustomini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${Win64BuildDir}\3rdParty\${LDViewDir}\resources\config\LDViewCustomini"
!undef INS_FILE_EXISTS
!endif

; 3rd party renderer configuration - LPub3DTrace
${!defineifexist} INS_FILE_EXISTS "${Win64BuildDir}\3rdParty\${LPub3DTraceDir}\resources\config\povray.conf"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config"
File /NONFATAL "${Win64BuildDir}\3rdParty\${LPub3DTraceDir}\resources\config\povray.conf"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${Win64BuildDir}\3rdParty\${LPub3DTraceDir}\resources\config\povray.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${Win64BuildDir}\3rdParty\${LPub3DTraceDir}\resources\config\povray.ini"
!undef INS_FILE_EXISTS
!endif
${If} $OverwriteUserDataParamFiles == 0
 SetOverwrite on
${EndIf}

Goto TranslationsDocumentsAndExtras

RunX86Architecture:
; Delete files with old names if exist
IfFileExists "${ProductName}_x32.exe" 0 ContinueX32Install
Delete "${ProductName}_x32.exe"
ContinueX32Install:

!ifdef X86_INSTALL
; MSVC Redistributable
File "${Win32BuildDir}\vc_redist.x86.exe"

; Deposit new 32bit files...

; Executable
File "${Win32BuildDir}\${LPub3DBuildFile}"
; Program database (pdb)
File "${Win32BuildDir}\${ProductName}.pdb"

; OpenSSL
File "${Win32BuildDir}\${OpenSSL32LibCrypto}"
File "${Win32BuildDir}\${OpenSSL32LibSSL}"

; Qt Direct Dependencies Libs
File "${Win32BuildDir}\Qt5Concurrent.dll"
File "${Win32BuildDir}\Qt5Core.dll"
File "${Win32BuildDir}\Qt5Gui.dll"
File "${Win32BuildDir}\Qt5Network.dll"
File "${Win32BuildDir}\Qt5OpenGL.dll"
File "${Win32BuildDir}\Qt5PrintSupport.dll"
File "${Win32BuildDir}\Qt5Svg.dll"
File "${Win32BuildDir}\Qt5Widgets.dll"
File "${Win32BuildDir}\Qt5Xml.dll"

; Qt Libraries
File "${Win32BuildDir}\d3dcompiler_47.dll"
File "${Win32BuildDir}\libEGL.dll"
File "${Win32BuildDir}\libGLESV2.dll"
File "${Win32BuildDir}\opengl32sw.dll"

; Qt Libraries
CreateDirectory "$INSTDIR\bearer"
SetOutPath "$INSTDIR\bearer"
File "${Win32BuildDir}\bearer\qgenericbearer.dll"
File "${Win32BuildDir}\bearer\qgenericbearer.dll"
CreateDirectory "$INSTDIR\iconengines"
SetOutPath "$INSTDIR\iconengines"
File "${Win32BuildDir}\iconengines\qsvgicon.dll"
CreateDirectory "$INSTDIR\imageformats"
SetOutPath "$INSTDIR\imageformats"
; File "${Win32BuildDir}\imageformats\qdds.dll"
File "${Win32BuildDir}\imageformats\qgif.dll"
File "${Win32BuildDir}\imageformats\qicns.dll"
File "${Win32BuildDir}\imageformats\qico.dll"
File "${Win32BuildDir}\imageformats\qjpeg.dll"
File "${Win32BuildDir}\imageformats\qsvg.dll"
File "${Win32BuildDir}\imageformats\qtga.dll"
File "${Win32BuildDir}\imageformats\qtiff.dll"
File "${Win32BuildDir}\imageformats\qwbmp.dll"
File "${Win32BuildDir}\imageformats\qwebp.dll"
CreateDirectory "$INSTDIR\printsupport"
SetOutPath "$INSTDIR\printsupport"
File "${Win32BuildDir}\printsupport\windowsprintersupport.dll"
CreateDirectory "$INSTDIR\platforms"
SetOutPath "$INSTDIR\platforms"
File "${Win32BuildDir}\platforms\qwindows.dll"
SetOutPath "$INSTDIR\styles"
File "${Win32BuildDir}\styles\qwindowsvistastyle.dll"
!endif

; 3rd party 32bit renderer executable - LDGLite
${!defineifexist} INS_FILE_EXISTS "${Win32BuildDir}\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LDGliteDir}\bin"
SetOutPath "$INSTDIR\3rdParty\${LDGliteDir}\bin"
File /NONFATAL "${Win32BuildDir}\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"
!undef INS_FILE_EXISTS
!endif

; 3rd party 32bit renderer executable - LDView
${!defineifexist} INS_FILE_EXISTS "${Win32BuildDir}\3rdParty\${LDViewDir}\bin\${LDViewExe}.exe"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\bin"
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\bin"
File /NONFATAL "${Win32BuildDir}\3rdParty\${LDViewDir}\bin\${LDViewExe}.exe"
!undef INS_FILE_EXISTS
!endif

; 3rd party 32bit renderer executable - LPub3DTrace
${!defineifexist} INS_FILE_EXISTS "${Win32BuildDir}\3rdParty\${LPub3DTraceDir}\bin\${LPub3DTraceExe}32.exe"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin"
File /NONFATAL "${Win32BuildDir}\3rdParty\${LPub3DTraceDir}\bin\${LPub3DTraceExe}32.exe"
!undef INS_FILE_EXISTS
!endif

${If} $OverwriteUserDataParamFiles == 0
 SetOverwrite off
${EndIf}

; 3rd party 32bit renderer configuration - LDView
${!defineifexist} INS_FILE_EXISTS "${Win32BuildDir}\3rdParty\${LDViewDir}\resources\config\ldview.ini"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
File /NONFATAL "${Win32BuildDir}\3rdParty\${LDViewDir}\resources\config\ldview.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${Win32BuildDir}\3rdParty\${LDViewDir}\resources\config\ldviewPOV.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${Win32BuildDir}\3rdParty\${LDViewDir}\resources\config\ldviewPOV.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${Win32BuildDir}\3rdParty\${LDViewDir}\resources\config\LDViewCustomini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${Win32BuildDir}\3rdParty\${LDViewDir}\resources\config\LDViewCustomini"
!undef INS_FILE_EXISTS
!endif

; 3rd party 32bit renderer configuration - LPub3DTrace
${!defineifexist} INS_FILE_EXISTS "${Win32BuildDir}\3rdParty\${LPub3DTraceDir}\resources\config\povray.conf"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config"
File /NONFATAL "${Win32BuildDir}\3rdParty\${LPub3DTraceDir}\resources\config\povray.conf"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${Win32BuildDir}\3rdParty\${LPub3DTraceDir}\resources\config\povray.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${Win32BuildDir}\3rdParty\${LPub3DTraceDir}\resources\config\povray.ini"
!undef INS_FILE_EXISTS
!endif

${If} $OverwriteUserDataParamFiles == 0
 SetOverwrite on
${EndIf}


Goto TranslationsDocumentsAndExtras

SingleArchitectureBuild:
; Perform appropriate installation
!ifdef X64_INSTALL
  Goto RunX64Architecture
!else
  Goto RunX86Architecture
!endif

TranslationsDocumentsAndExtras:
SetOutPath "$INSTDIR"
File "${WinBuildDir}\docs\README.txt"
File "${WinBuildDir}\docs\RELEASE_NOTES.html"

; Qt Translation Files
CreateDirectory "$INSTDIR\translations"
SetOutPath "$INSTDIR\translations"
File "${WinBuildDir}\translations\*.*"

; Data depository folder
CreateDirectory "$INSTDIR\data"
SetOutPath "$INSTDIR\data"
File "${WinBuildDir}\extras\excludedParts.lst"
File "${WinBuildDir}\extras\fadeStepColorParts.lst"
File "${WinBuildDir}\extras\freeformAnnotations.lst"
File "${WinBuildDir}\extras\LDConfig.ldr"
File "${WinBuildDir}\extras\ldglite.ini"
File "${WinBuildDir}\extras\ldvExport.ini"
File "${WinBuildDir}\extras\PDFPrint.jpg"
File "${WinBuildDir}\extras\pli.mpd"
File "${WinBuildDir}\extras\pliSubstituteParts.lst"
File "${WinBuildDir}\extras\titleAnnotations.lst"
File "${WinBuildDir}\extras\ldvMessages.ini"

; LDraw libraries - data/portable location
File "${WinBuildDir}\extras\complete.zip"
File "${WinBuildDir}\extras\lpub3dldrawunf.zip"
File "${WinBuildDir}\extras\tenteparts.zip"
File "${WinBuildDir}\extras\vexiqparts.zip"

; Documents
CreateDirectory "$INSTDIR\docs"
SetOutPath "$INSTDIR\docs"
File "${WinBuildDir}\docs\COPYING.txt"
File "${WinBuildDir}\docs\CREDITS.txt"
File "${WinBuildDir}\docs\README.txt"
File "${WinBuildDir}\docs\RELEASE_NOTES.html"
SetOutPath "$INSTDIR"

; 3rd party renderer documentation and resources - LDGLite
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LDGliteDir}\doc\LICENCE"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LDGliteDir}\doc"
SetOutPath "$INSTDIR\3rdParty\${LDGliteDir}\doc"
File /NONFATAL "${WinBuildDir}\3rdParty\${LDGliteDir}\doc\LICENCE"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LDGliteDir}\doc\README.TXT"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LDGliteDir}\doc\README.TXT"
!undef INS_FILE_EXISTS
!endif

; 3rd party renderer documentation and resources - LDView
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LDViewDir}\doc\ChangeHistory.html"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\doc"
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\doc"
File /NONFATAL "${WinBuildDir}\3rdParty\${LDViewDir}\doc\ChangeHistory.html"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LDViewDir}\doc\Help.html"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LDViewDir}\doc\Help.html"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LDViewDir}\doc\License.txt"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LDViewDir}\doc\License.txt"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LDViewDir}\doc\Readme.txt"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LDViewDir}\doc\Readme.txt"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LDViewDir}\resources\8464.mpd"
!ifdef INS_FILE_EXISTS
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\resources"
File /NONFATAL "${WinBuildDir}\3rdParty\${LDViewDir}\resources\8464.mpd"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LDViewDir}\resources\LGEO.xml"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LDViewDir}\resources\LGEO.xml"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LDViewDir}\resources\m6459.ldr"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LDViewDir}\resources\m6459.ldr"
!undef INS_FILE_EXISTS
!endif

; 3rd party renderer documentation and resources - LPub3DTrace
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\doc\AUTHORS.txt"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc"
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\doc\AUTHORS.txt"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\doc\ChangeLog.txt"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\doc\ChangeLog.txt"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\doc\CUI_README.txt"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\doc\CUI_README.txt"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\doc\LICENSE.txt"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\doc\LICENSE.txt"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\arrays.inc"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include"
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\arrays.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\ash.map"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\ash.map"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\benediti.map"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\benediti.map"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\bubinga.map"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\bubinga.map"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\bumpmap_.png"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\bumpmap_.png"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\cedar.map"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\cedar.map"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\chars.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\chars.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\colors.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\colors.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\colors_ral.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\colors_ral.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\consts.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\consts.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\crystal.ttf"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\crystal.ttf"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\cyrvetic.ttf"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\cyrvetic.ttf"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\debug.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\debug.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\finish.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\finish.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\fract003.png"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\fract003.png"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\functions.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\functions.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\glass.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\glass.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\glass_old.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\glass_old.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\golds.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\golds.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\ior.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\ior.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\logo.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\logo.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\makegrass.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\makegrass.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\marbteal.map"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\marbteal.map"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\math.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\math.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\meshmaker.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\meshmaker.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\metals.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\metals.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\Mount1.png"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\Mount1.png"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\Mount2.png"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\Mount2.png"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\mtmand.pot"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\mtmand.pot"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\mtmandj.png"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\mtmandj.png"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\orngwood.map"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\orngwood.map"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\pinkmarb.map"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\pinkmarb.map"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\plasma2.png"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\plasma2.png"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\plasma3.png"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\plasma3.png"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\povlogo.ttf"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\povlogo.ttf"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\povmap.png"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\povmap.png"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\rad_def.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\rad_def.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\rand.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\rand.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\rdgranit.map"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\rdgranit.map"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\screen.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\screen.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapes.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapes.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapes2.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapes2.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapes3.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapes3.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapesq.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapesq.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapes_old.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapes_old.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\skies.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\skies.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\spiral.df3"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\spiral.df3"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stage1.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stage1.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stars.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stars.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stdcam.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stdcam.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stdinc.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stdinc.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stoneold.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stoneold.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stones.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stones.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stones1.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stones1.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stones2.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stones2.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\strings.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\strings.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\sunpos.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\sunpos.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\teak.map"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\teak.map"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\test.png"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\test.png"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\textures.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\textures.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\timrom.ttf"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\timrom.ttf"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\transforms.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\transforms.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\ttffonts.cat"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\ttffonts.cat"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\whiteash.map"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\whiteash.map"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\woodmaps.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\woodmaps.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\woods.inc"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\woods.inc"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\allanim.ini"
!ifdef INS_FILE_EXISTS
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini"
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\allanim.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\allstill.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\allstill.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\low.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\low.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\pngflc.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\pngflc.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\pngfli.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\pngfli.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\povray.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\povray.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\quickres.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\quickres.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res120.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res120.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res1k.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res1k.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res320.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res320.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res640.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res640.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res800.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res800.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\slow.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\slow.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\tgaflc.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\tgaflc.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\tgafli.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\tgafli.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\zipflc.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\zipflc.ini"
!undef INS_FILE_EXISTS
!endif
${!defineifexist} INS_FILE_EXISTS "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\zipfli.ini"
!ifdef INS_FILE_EXISTS
File /NONFATAL "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\zipfli.ini"
!undef INS_FILE_EXISTS
!endif
SetOutPath "$INSTDIR"