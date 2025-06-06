; LPub3D Uninstall Files Script Include
; Last Update: April 14, 2025
; Copyright (C) 2016 - 2025 by Trevor SANDY

; Executable
Delete "$INSTDIR\${LPub3DBuildFile}"
; Program database (pdb)
Delete "$INSTDIR\${ProductName}.pdb"

Delete "$INSTDIR\Qt5Concurrent.dll"
Delete "$INSTDIR\Qt5Core.dll"
Delete "$INSTDIR\Qt5Gui.dll"
Delete "$INSTDIR\Qt5Network.dll"
Delete "$INSTDIR\Qt5OpenGL.dll"
Delete "$INSTDIR\Qt5PrintSupport.dll"
Delete "$INSTDIR\Qt5Svg.dll"
Delete "$INSTDIR\Qt5Widgets.dll"
Delete "$INSTDIR\Qt5Xml.dll"

; Qt Libraries
Delete "$INSTDIR\D3Dcompiler_47.dll"
Delete "$INSTDIR\libEGL.dll"
Delete "$INSTDIR\libGLESV2.dll"
Delete "$INSTDIR\opengl32sw.dll"

; Qt Plugins
Delete "$INSTDIR\bearer\qgenericbearer.dll"
Delete "$INSTDIR\bearer\qgenericbearer.dll"
Delete "$INSTDIR\iconengines\qsvgicon.dll"
Delete "$INSTDIR\imageformats\qgif.dll"
Delete "$INSTDIR\imageformats\qicns.dll"
Delete "$INSTDIR\imageformats\qico.dll"
Delete "$INSTDIR\imageformats\qjpeg.dll"
Delete "$INSTDIR\imageformats\qsvg.dll"
Delete "$INSTDIR\imageformats\qtga.dll"
Delete "$INSTDIR\imageformats\qtiff.dll"
Delete "$INSTDIR\imageformats\qwbmp.dll"
Delete "$INSTDIR\imageformats\qwebp.dll"
Delete "$INSTDIR\printsupport\windowsprintersupport.dll"
Delete "$INSTDIR\platforms\qwindows.dll"
Delete "$INSTDIR\styles\qwindowsvistastyle.dll"

; 3rd party renderer executable - LDGLite
${If} $LDGliteInstalled == 1
Delete "$INSTDIR\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"
${EndIf}

; 3rd party renderer configuration - LDView
${If} $LDViewInstalled == 1
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\config\ldview.ini"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\config\ldviewPOV.ini"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\config\LDViewCustomini"
${EndIf}

; 3rd party renderer configuration - LPub3DTrace
${If} $LPub3DTraceInstalled == 1
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config\povray.conf"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config\povray.ini"
${EndIf}

; Perform appripriate installation
${If} ${RunningX64}
Goto UninstallX64Assets
${Else}
Goto UninstallX86Assets
${EndIf}

UninstallX64Assets:
; MSVC Redistributables
Delete "$INSTDIR\vcredist_x86_64.exe"

; Qt Libraries
Delete "$INSTDIR\libgcc_s_seh-1.dll"

; OpenSSL
Delete "$INSTDIR\${OpenSSL64LibCrypto}"
Delete "$INSTDIR\${OpenSSL64LibSSL}"

; 3rd party renderer executable - LDView
${If} $LDViewInstalled == 1
Delete "$INSTDIR\3rdParty\${LDViewDir}\bin\${LDViewExe}64.exe"
${EndIf}

; 3rd party renderer executable - LPub3D_Trace
${If} $LPub3DTraceInstalled == 1
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin\${LPub3DTraceExe}64.exe"
${EndIf}

Goto DocumentsAndResources

UninstallX86Assets:
; MSVC Redistributables
Delete "$INSTDIR\vcredist_x86.exe"

; Qt Libraries
Delete "$INSTDIR\libgcc_s_dw2-1.dll"

; OpenSSL
Delete "$INSTDIR\${OpenSSL32LibCrypto}"
Delete "$INSTDIR\${OpenSSL32LibSSL}"

; 3rd party renderer executable - LDView
${If} $LDViewInstalled == 1
Delete "$INSTDIR\3rdParty\${LDViewDir}\bin\${LDViewExe}.exe"
${EndIf}

; 3rd party renderer executable - LPub3D_Trace
${If} $LPub3DTraceInstalled == 1
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin\${LPub3DTraceExe}32.exe"
${EndIf}

DocumentsAndResources:
; Data repository folder
Delete "$INSTDIR\data\excludedParts.lst"
Delete "$INSTDIR\data\fadeStepColorParts.lst"
Delete "$INSTDIR\data\freeformAnnotations.lst"
Delete "$INSTDIR\data\LDConfig.ldr"
Delete "$INSTDIR\data\ldglite.ini"
Delete "$INSTDIR\data\ldvExport.ini"
Delete "$INSTDIR\data\PDFPrint.jpg"
Delete "$INSTDIR\data\pli.mpd"
Delete "$INSTDIR\data\pliSubstituteParts.lst"
Delete "$INSTDIR\data\titleAnnotations.lst"
Delete "$INSTDIR\data\ldvMessages.ini"

; LDraw libraries - data/portable location
Delete "$INSTDIR\data\complete.zip"
Delete "$INSTDIR\data\lpub3dldrawunf.zip"
Delete "$INSTDIR\data\tenteparts.zip"
Delete "$INSTDIR\data\vexiqparts.zip"

; Documents
Delete "$INSTDIR\docs\COPYING.txt"
Delete "$INSTDIR\docs\CREDITS.txt"
Delete "$INSTDIR\docs\README.txt"
Delete "$INSTDIR\docs\RELEASE_NOTES.html"

; 3rd party renderer documentation - LDGLite
${If} $LDGliteInstalled == 1
Delete "$INSTDIR\3rdParty\${LDGliteDir}\doc\LICENCE"
Delete "$INSTDIR\3rdParty\${LDGliteDir}\doc\README.TXT"
${EndIf}

; 3rd party renderer documentation and resources - LDView
${If} $LDViewInstalled == 1
Delete "$INSTDIR\3rdParty\${LDViewDir}\doc\ChangeHistory.html"
Delete "$INSTDIR\3rdParty\${LDViewDir}\doc\Help.html"
Delete "$INSTDIR\3rdParty\${LDViewDir}\doc\License.txt"
Delete "$INSTDIR\3rdParty\${LDViewDir}\doc\Readme.txt"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\8464.mpd"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\LGEO.xml"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\m6459.ldr"
${EndIf}

; 3rd party renderer documentation and resources - LPub3DTrace
${If} $LPub3DTraceInstalled == 1
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\AUTHORS.txt"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\ChangeLog.txt"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\CUI_README.txt"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\LICENSE.txt"

Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\arrays.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\ash.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\benediti.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\bubinga.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\bumpmap_.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\cedar.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\chars.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\colors.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\colors_ral.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\consts.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\crystal.ttf"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\cyrvetic.ttf"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\debug.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\finish.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\fract003.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\functions.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\glass.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\glass_old.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\golds.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\ior.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\logo.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\makegrass.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\marbteal.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\math.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\meshmaker.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\metals.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\Mount1.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\Mount2.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\mtmand.pot"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\mtmandj.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\orngwood.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\pinkmarb.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\plasma2.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\plasma3.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\povlogo.ttf"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\povmap.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\rad_def.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\rand.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\rdgranit.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\screen.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapes.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapes_old.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapes2.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapes3.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapesq.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\skies.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\spiral.df3"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stage1.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stars.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stdcam.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stdinc.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stoneold.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stones.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stones1.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stones2.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\strings.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\sunpos.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\teak.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\test.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\textures.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\timrom.ttf"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\transforms.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\ttffonts.cat"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\whiteash.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\woodmaps.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\woods.inc"

Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\allanim.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\allstill.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\low.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\pngflc.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\pngfli.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\povray.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\quickres.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res120.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res1k.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res320.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res640.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res800.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\slow.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\tgaflc.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\tgafli.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\zipflc.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\zipfli.ini"
${EndIf}

; Remove Qt plugin folders
RMDir "$INSTDIR\bearer"
RMDir "$INSTDIR\iconengines"
RMDir "$INSTDIR\imageformats"
RMDir "$INSTDIR\printsupport"
RMDir "$INSTDIR\platforms"

; Remove LPub3D data folders
RMDir "$INSTDIR\data"
RMDir "$INSTDIR\docs"

; Remove 3rdParty folders
${If} $LDGliteInstalled == 1
RMDir "$INSTDIR\3rdParty\${LDGliteDir}\bin"
RMDir "$INSTDIR\3rdParty\${LDGliteDir}\doc"
RMDir "$INSTDIR\3rdParty\${LDGliteDir}"
${EndIf}

${If} $LDViewInstalled == 1
RMDir "$INSTDIR\3rdParty\${LDViewDir}\bin"
RMDir "$INSTDIR\3rdParty\${LDViewDir}\doc"
RMDir "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
RMDir "$INSTDIR\3rdParty\${LDViewDir}\resources"
RMDir "$INSTDIR\3rdParty\${LDViewDir}"
${EndIf}

${If} $LPub3DTraceInstalled == 1
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin"
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc"
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini"
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include"
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config"
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources"
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}"
RMDir "$INSTDIR\3rdParty"
${EndIf}