; LPub3D Install Files Script Include
; Last Update: April 15, 2025
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

; Perform appripriate installation
${If} ${RunningX64}
Goto RunX64Architecture
${Else}
Goto RunX86Architecture
${EndIf}

RunX64Architecture:
; Delete x86_64 files with old names if exist
IfFileExists "${ProductName}_${ArchExt}.exe" 0 ContinueX64Install
Delete "${ProductName}_${ArchExt}.exe"
ContinueX64Install:

; MSVC Redistributable
File "${Win64BuildDir}\vcredist\vcredist_x86_64.exe"

; Deposit new 64bit files...

; Executable
File "${Win64BuildDir}\${LPub3DBuildFile}"
; Program database (pdb)
File "${Win64BuildDir}\${ProductName}.pdb"

; OpenSSL
File "${Win64BuildDir}\${OpenSSL64LibCrypto}"
File "${Win64BuildDir}\${OpenSSL64LibSSL}"

; Qt Libs
File "${Win64BuildDir}\Qt5Concurrent.dll"
File "${Win64BuildDir}\Qt5Core.dll"
File "${Win64BuildDir}\Qt5Gui.dll"
File "${Win64BuildDir}\Qt5Network.dll"
File "${Win64BuildDir}\Qt5OpenGL.dll"
File "${Win64BuildDir}\Qt5PrintSupport.dll"
File "${Win64BuildDir}\Qt5Svg.dll"
File "${Win64BuildDir}\Qt5Widgets.dll"
File "${Win64BuildDir}\Qt5Xml.dll"

; New Stuff - Qt Libraries
File "${Win64BuildDir}\D3Dcompiler_47.dll"
File "${Win64BuildDir}\libEGL.dll"
File "${Win64BuildDir}\libGLESV2.dll"
File "${Win64BuildDir}\opengl32sw.dll"

; New Stuff - Qt Plugins
CreateDirectory "$INSTDIR\bearer"
SetOutPath "$INSTDIR\bearer"
File "${Win64BuildDir}\bearer\qgenericbearer.dll"
;File "${Win64BuildDir}\bearer\qgenericbearer.dll"
CreateDirectory "$INSTDIR\iconengines"
SetOutPath "$INSTDIR\iconengines"
File "${Win64BuildDir}\iconengines\qsvgicon.dll"
CreateDirectory "$INSTDIR\imageformats"
SetOutPath "$INSTDIR\imageformats"
File "${Win64BuildDir}\imageformats\qgif.dll"
File "${Win64BuildDir}\imageformats\qicns.dll"
File "${Win64BuildDir}\imageformats\qico.dll"
File "${Win64BuildDir}\imageformats\qjpeg.dll"
File "${Win64BuildDir}\imageformats\qsvg.dll"
File "${Win64BuildDir}\imageformats\qtga.dll"
File "${Win64BuildDir}\imageformats\qtiff.dll"
File "${Win64BuildDir}\imageformats\qwbmp.dll"
File "${Win64BuildDir}\imageformats\qwebp.dll"
CreateDirectory "$INSTDIR\printsupport"
SetOutPath "$INSTDIR\printsupport"
File "${Win64BuildDir}\printsupport\windowsprintersupport.dll"
CreateDirectory "$INSTDIR\platforms"
SetOutPath "$INSTDIR\platforms"
File "${Win64BuildDir}\platforms\qwindows.dll"
SetOutPath "$INSTDIR\styles"
File "${Win64BuildDir}\styles\qwindowsvistastyle.dll"

; 3rd party renderer executable -LDGLite
${If} $LDGLiteInstalled == 1
CreateDirectory "$INSTDIR\3rdParty\${LDGliteDir}\bin"
SetOutPath "$INSTDIR\3rdParty\${LDGliteDir}\bin"
File "${Win64BuildDir}\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"
${EndIf}

; 3rd party renderer executable - LDView
${If} $LDViewInstalled == 1
CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\bin"
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\bin"
File "${Win64BuildDir}\3rdParty\${LDViewDir}\bin\${LDViewExe}64.exe"
CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
${If} $OverwriteUserDataParamFiles == 0
 SetOverwrite off
${EndIf}
File "${Win64BuildDir}\3rdParty\${LDViewDir}\resources\config\ldview.ini"
File "${Win64BuildDir}\3rdParty\${LDViewDir}\resources\config\ldviewPOV.ini"
File "${Win64BuildDir}\3rdParty\${LDViewDir}\resources\config\LDViewCustomini"
${If} $OverwriteUserDataParamFiles == 0
 SetOverwrite on
${EndIf}
${EndIf}

; 3rd party renderer executable - LPub3DTrace - POVRay
${If} $LPub3DTraceInstalled == 1
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin"
File "${Win64BuildDir}\3rdParty\${LPub3DTraceDir}\bin\${LPub3DTraceExe}64.exe"
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config"
${If} $OverwriteUserDataParamFiles == 0
 SetOverwrite off
${EndIf}
File "${Win64BuildDir}\3rdParty\${LPub3DTraceDir}\resources\config\povray.conf"
File "${Win64BuildDir}\3rdParty\${LPub3DTraceDir}\resources\config\povray.ini"
${If} $OverwriteUserDataParamFiles == 0
 SetOverwrite on
${EndIf}
${EndIf}

Goto DocumentsAndExtras

RunX86Architecture:
; Delete files with old names if exist
IfFileExists "${ProductName}_${ArchExt}.exe" 0 ContinueX32Install
Delete "${ProductName}_${ArchExt}.exe"
ContinueX32Install:

; MSVC Redistributable
File "${Win32BuildDir}\vcredist\vcredist_x86.exe"

; Deposit new 32bit files...

; Executable
; File "${Win32BuildDir}\${LPub3D32bitBuildFile}"
File "${Win32BuildDir}\${LPub3DBuildFile}"
; Program database (pdb)
File "${Win32BuildDir}\${ProductName}.pdb"

; OpenSSL
File "${Win32BuildDir}\${OpenSSL32LibCrypto}"
File "${Win32BuildDir}\${OpenSSL32LibSSL}"

; Qt Libs
File "${Win32BuildDir}\Qt5Concurrent.dll"
File "${Win32BuildDir}\Qt5Core.dll"
File "${Win32BuildDir}\Qt5Gui.dll"
File "${Win32BuildDir}\Qt5Network.dll"
File "${Win32BuildDir}\Qt5OpenGL.dll"
File "${Win32BuildDir}\Qt5PrintSupport.dll"
File "${Win32BuildDir}\Qt5Svg.dll"
File "${Win32BuildDir}\Qt5Widgets.dll"
File "${Win32BuildDir}\Qt5Xml.dll"

; New Stuff - Qt Libraries
File "${Win32BuildDir}\D3Dcompiler_47.dll"
File "${Win32BuildDir}\libEGL.dll"
File "${Win32BuildDir}\libGLESV2.dll"
File "${Win32BuildDir}\opengl32sw.dll"

; New Stuff - Qt Plugins
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

; 3rd party 32bit renderer executable - LDGLite
${If} $LDGLiteInstalled == 1
CreateDirectory "$INSTDIR\3rdParty\${LDGliteDir}\bin"
SetOutPath "$INSTDIR\3rdParty\${LDGliteDir}\bin"
File "${Win32BuildDir}\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"
${EndIf}

; 3rd party 32bit renderer executable - LDView
${If} $LDViewInstalled == 1
CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\bin"
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\bin"
File "${Win32BuildDir}\3rdParty\${LDViewDir}\bin\${LDViewExe}.exe"
CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
${If} $OverwriteUserDataParamFiles == 0
 SetOverwrite off
${EndIf}
File "${Win32BuildDir}\3rdParty\${LDViewDir}\resources\config\ldview.ini"
File "${Win32BuildDir}\3rdParty\${LDViewDir}\resources\config\ldviewPOV.ini"
File "${Win32BuildDir}\3rdParty\${LDViewDir}\resources\config\LDViewCustomini"
${If} $OverwriteUserDataParamFiles == 0
 SetOverwrite on
${EndIf}
${EndIf}

; 3rd party 32bit renderer executable - LPub3DTrace - POVRay
${If} $LPub3DTraceInstalled == 1
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin"
File "${Win32BuildDir}\3rdParty\${LPub3DTraceDir}\bin\${LPub3DTraceExe}32.exe"
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config"
${If} $OverwriteUserDataParamFiles == 0
 SetOverwrite off
${EndIf}
File "${Win32BuildDir}\3rdParty\${LPub3DTraceDir}\resources\config\povray.conf"
File "${Win32BuildDir}\3rdParty\${LPub3DTraceDir}\resources\config\povray.ini"
${If} $OverwriteUserDataParamFiles == 0
 SetOverwrite on
${EndIf}
${EndIf}

Goto DocumentsAndExtras

SingleArchitectureBuild:

; Perform appripriate installation
${If} ${RunningX64}
Goto RunX64Architecture
${Else}
Goto RunX86Architecture
${EndIf}

DocumentsAndExtras:
SetOutPath "$INSTDIR"
File "${WinBuildDir}\docs\README.txt"
File "${WinBuildDir}\docs\RELEASE_NOTES.html"

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
${If} $LDGLiteInstalled == 1
CreateDirectory "$INSTDIR\3rdParty\${LDGliteDir}\doc"
SetOutPath "$INSTDIR\3rdParty\${LDGliteDir}\doc"
File "${WinBuildDir}\3rdParty\${LDGliteDir}\doc\LICENCE"
File "${WinBuildDir}\3rdParty\${LDGliteDir}\doc\README.TXT"
${EndIf}

; 3rd party renderer documentation and resources - LDView
${If} $LDViewInstalled == 1
CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\doc"
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\doc"
File "${WinBuildDir}\3rdParty\${LDViewDir}\doc\ChangeHistory.html"
File "${WinBuildDir}\3rdParty\${LDViewDir}\doc\Help.html"
File "${WinBuildDir}\3rdParty\${LDViewDir}\doc\License.txt"
File "${WinBuildDir}\3rdParty\${LDViewDir}\doc\Readme.txt"
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\resources"
File "${WinBuildDir}\3rdParty\${LDViewDir}\resources\8464.mpd"
File "${WinBuildDir}\3rdParty\${LDViewDir}\resources\LGEO.xml"
File "${WinBuildDir}\3rdParty\${LDViewDir}\resources\m6459.ldr"
${EndIf}

; 3rd party renderer documentation and resources - LPub3DTrace
${If} $LPub3DTraceInstalled == 1
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\doc\AUTHORS.txt"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\doc\ChangeLog.txt"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\doc\CUI_README.txt"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\doc\LICENSE.txt"
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\arrays.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\ash.map"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\benediti.map"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\bubinga.map"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\bumpmap_.png"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\cedar.map"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\chars.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\colors.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\colors_ral.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\consts.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\crystal.ttf"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\cyrvetic.ttf"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\debug.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\finish.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\fract003.png"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\functions.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\glass.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\glass_old.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\golds.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\ior.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\logo.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\makegrass.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\marbteal.map"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\math.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\meshmaker.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\metals.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\Mount1.png"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\Mount2.png"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\mtmand.pot"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\mtmandj.png"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\orngwood.map"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\pinkmarb.map"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\plasma2.png"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\plasma3.png"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\povlogo.ttf"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\povmap.png"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\rad_def.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\rand.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\rdgranit.map"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\screen.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapes.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapes2.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapes3.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapesq.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\shapes_old.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\skies.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\spiral.df3"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stage1.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stars.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stdcam.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stdinc.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stoneold.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stones.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stones1.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\stones2.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\strings.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\sunpos.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\teak.map"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\test.png"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\textures.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\timrom.ttf"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\transforms.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\ttffonts.cat"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\whiteash.map"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\woodmaps.inc"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\include\woods.inc"
CreateDirectory "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini"
SetOutPath "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\allanim.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\allstill.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\low.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\pngflc.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\pngfli.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\povray.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\quickres.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res120.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res1k.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res320.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res640.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\res800.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\slow.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\tgaflc.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\tgafli.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\zipflc.ini"
File "${WinBuildDir}\3rdParty\${LPub3DTraceDir}\resources\ini\zipfli.ini"
SetOutPath "$INSTDIR"
${EndIf}
