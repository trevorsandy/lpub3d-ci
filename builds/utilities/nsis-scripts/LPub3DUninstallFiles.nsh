; LPub3D Uninstall Files Script Include
; Last Update: October 29, 2025
; Copyright (C) 2016 - 2025 by Trevor SANDY

; Executable
Delete "$INSTDIR\${LPub3DBuildFile}"
; Program database (pdb)
Delete "$INSTDIR\${ProductName}.pdb"

; Qt Direct Dependencies Libs
Delete "$INSTDIR\Qt6Concurrent.dll"
Delete "$INSTDIR\Qt6Core.dll"
Delete "$INSTDIR\Qt6Core5Compat.dll"
Delete "$INSTDIR\Qt6Gui.dll"
Delete "$INSTDIR\Qt6Network.dll"
Delete "$INSTDIR\Qt6OpenGL.dll"
Delete "$INSTDIR\Qt6OpenGLWidgets.dll"
Delete "$INSTDIR\Qt6PrintSupport.dll"
Delete "$INSTDIR\Qt6Svg.dll"
Delete "$INSTDIR\Qt6Widgets.dll"
Delete "$INSTDIR\Qt6Xml.dll"

; Qt Plugins Generic
Delete "$INSTDIR\generic\qtuiotouchplugin.dll"

; Qt Plugins Icon Engines
Delete "$INSTDIR\iconengines\qsvgicon.dll"

; Qt Plugins Image Formats
Delete "$INSTDIR\imageformats\qgif.dll"
Delete "$INSTDIR\imageformats\qico.dll"
Delete "$INSTDIR\imageformats\qjpeg.dll"
Delete "$INSTDIR\imageformats\qsvg.dll"

; Qt Plugins Network Information
Delete "$INSTDIR\networkinformation\qnetworklistmanager.dll"

; Qt Plugins Platforms
Delete "$INSTDIR\platforms\qwindows.dll"

; Qt Plugins Styles
Delete "$INSTDIR\styles\qmodernwindowsstyle.dll"

; Qt Plugins Tls
Delete "$INSTDIR\tls\qcertonlybackend.dll"
Delete "$INSTDIR\tls\qopensslbackend.dll"
Delete "$INSTDIR\tls\qschannelbackend.dll"

!ifndef ARM64_INSTALL
; OpenGL Library
Delete "$INSTDIR\opengl32sw.dll"

; DirectX Compiler Libraries
Delete "$INSTDIR\d3dcompiler_47.dll"
!endif
Delete "$INSTDIR\dxcompiler.dll"
Delete "$INSTDIR\dxil.dll"

; Translation library
Delete "$INSTDIR\icuuc.dll"

; 3rd party renderer executable -LDGLite
${If} ${FileExists} "$INSTDIR\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"
Delete "$INSTDIR\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"
${EndIf}

; Perform appripriate uninstall
${If} ${RunningX64}
Goto UninstallX64Assets
${Else}
Goto UninstallX86Assets
${EndIf}

UninstallX64Assets:
; MSVC C++ Runtime Redistributable
!ifdef ARM64_INSTALL
Delete "$INSTDIR\vc_redist.arm64.exe"
!else
Delete "$INSTDIR\vc_redist.x64.exe"
!endif

; OpenSSL
!ifdef WIN64_SSL
Delete "$INSTDIR\${OpenSSL64LibCrypto}"
Delete "$INSTDIR\${OpenSSL64LibSSL}"
!endif

; 3rd party renderer executable - LDView
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\bin\${LDViewExe}64.exe"
Delete "$INSTDIR\3rdParty\${LDViewDir}\bin\${LDViewExe}64.exe"
${EndIf}

; 3rd party renderer executable - LPub3D_Trace
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin\${LPub3DTraceExe}64.exe"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin\${LPub3DTraceExe}64.exe"
${EndIf}

Goto ConfigurationDocumentsAndResources

UninstallX86Assets:
; MSVC C++ Runtime Redistributable
Delete "$INSTDIR\vc_redist.x86.exe"

; Qt Libraries
Delete "$INSTDIR\libgcc_s_dw2-1.dll"

; OpenSSL
!ifdef X86_INSTALL
Delete "$INSTDIR\${OpenSSL32LibCrypto}"
Delete "$INSTDIR\${OpenSSL32LibSSL}"
!endif

; 3rd party renderer executable - LDView
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\bin\${LDViewExe}.exe"
Delete "$INSTDIR\3rdParty\${LDViewDir}\bin\${LDViewExe}.exe"
${EndIf}

; 3rd party renderer executable - LPub3D_Trace
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin\${LPub3DTraceExe}32.exe"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin\${LPub3DTraceExe}32.exe"
${EndIf}

ConfigurationDocumentsAndResources:
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

; 3rd party renderer configuration - LDView
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\resources\config\ldview.ini"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\config\ldview.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\resources\config\ldviewPOV.ini"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\config\ldviewPOV.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\resources\config\LDViewCustomini"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\config\LDViewCustomini"
${EndIf}

; 3rd party renderer configuration - LPub3D_Trace
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config\povray.conf"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config\povray.conf"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config\povray.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config\povray.ini"
${EndIf}

; 3rd party renderer documentation and resources - LDGLite
${If} ${FileExists} "$INSTDIR\3rdParty\${LDGliteDir}\doc\LICENCE"
Delete "$INSTDIR\3rdParty\${LDGliteDir}\doc\LICENCE"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDGliteDir}\doc\README.TXT"
Delete "$INSTDIR\3rdParty\${LDGliteDir}\doc\README.TXT"
${EndIf}

; 3rd party renderer documentation and resources - LDView
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\doc\ChangeHistory.html"
Delete "$INSTDIR\3rdParty\${LDViewDir}\doc\ChangeHistory.html"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\doc\Help.html"
Delete "$INSTDIR\3rdParty\${LDViewDir}\doc\Help.html"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\doc\License.txt"
Delete "$INSTDIR\3rdParty\${LDViewDir}\doc\License.txt"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\doc\Readme.txt"
Delete "$INSTDIR\3rdParty\${LDViewDir}\doc\Readme.txt"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\resources\8464.mpd"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\8464.mpd"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\resources\LGEO.xml"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\LGEO.xml"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\resources\m6459.ldr"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\m6459.ldr"
${EndIf}

; 3rd party renderer documentation and resources - LPub3DTrace
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\AUTHORS.txt"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\AUTHORS.txt"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\ChangeLog.txt"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\ChangeLog.txt"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\CUI_README.txt"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\CUI_README.txt"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\LICENSE.txt"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\LICENSE.txt"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\arrays.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\arrays.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\ash.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\ash.map"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\benediti.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\benediti.map"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\bubinga.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\bubinga.map"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\bumpmap_.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\bumpmap_.png"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\cedar.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\cedar.map"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\chars.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\chars.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\colors.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\colors.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\colors_ral.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\colors_ral.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\consts.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\consts.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\crystal.ttf"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\crystal.ttf"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\cyrvetic.ttf"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\cyrvetic.ttf"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\debug.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\debug.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\finish.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\finish.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\fract003.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\fract003.png"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\functions.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\functions.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\glass.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\glass.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\glass_old.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\glass_old.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\golds.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\golds.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\ior.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\ior.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\logo.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\logo.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\makegrass.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\makegrass.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\marbteal.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\marbteal.map"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\math.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\math.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\meshmaker.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\meshmaker.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\metals.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\metals.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\Mount1.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\Mount1.png"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\Mount2.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\Mount2.png"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\mtmand.pot"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\mtmand.pot"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\mtmandj.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\mtmandj.png"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\orngwood.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\orngwood.map"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\pinkmarb.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\pinkmarb.map"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\plasma2.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\plasma2.png"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\plasma3.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\plasma3.png"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\povlogo.ttf"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\povlogo.ttf"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\povmap.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\povmap.png"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\rad_def.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\rad_def.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\rand.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\rand.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\rdgranit.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\rdgranit.map"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\screen.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\screen.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapes.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapes.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapes2.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapes2.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapes3.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapes3.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapesq.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapesq.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapes_old.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\shapes_old.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\skies.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\skies.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\spiral.df3"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\spiral.df3"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stage1.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stage1.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stars.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stars.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stdcam.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stdcam.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stdinc.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stdinc.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stoneold.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stoneold.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stones.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stones.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stones1.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stones1.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stones2.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\stones2.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\strings.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\strings.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\sunpos.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\sunpos.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\teak.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\teak.map"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\test.png"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\test.png"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\textures.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\textures.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\timrom.ttf"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\timrom.ttf"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\transforms.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\transforms.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\ttffonts.cat"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\ttffonts.cat"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\whiteash.map"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\whiteash.map"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\woodmaps.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\woodmaps.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\woods.inc"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\woods.inc"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\allanim.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\allanim.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\allstill.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\allstill.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\low.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\low.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\pngflc.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\pngflc.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\pngfli.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\pngfli.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\povray.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\povray.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\quickres.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\quickres.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res120.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res120.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res1k.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res1k.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res320.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res320.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res640.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res640.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res800.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\res800.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\slow.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\slow.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\tgaflc.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\tgaflc.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\tgafli.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\tgafli.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\zipflc.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\zipflc.ini"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\zipfli.ini"
Delete "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\zipfli.ini"
${EndIf}

; Remove Qt plugin folders
RMDir "$INSTDIR\generic"
RMDir "$INSTDIR\iconengines"
RMDir "$INSTDIR\imageformats"
RMDir "$INSTDIR\networkinformation"
RMDir "$INSTDIR\platforms"
RMDir "$INSTDIR\tls"

; Remove Qt Translation folder
RMDir "$INSTDIR\translations\*.*"

; Remove LPub3D data folders
RMDir "$INSTDIR\data"
RMDir "$INSTDIR\docs"

; Remove 3rdParty LDGLite folders
${If} ${FileExists} "$INSTDIR\3rdParty\${LDGliteDir}\bin\*.*"
RMDir "$INSTDIR\3rdParty\${LDGliteDir}\bin\*.*"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDGliteDir}\doc\*.*"
RMDir "$INSTDIR\3rdParty\${LDGliteDir}\doc\*.*"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDGliteDir}\*.*"
RMDir "$INSTDIR\3rdParty\${LDGliteDir}\*.*"
${EndIf}

; Remove 3rdParty LDView folders
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\bin\*.*"
RMDir "$INSTDIR\3rdParty\${LDViewDir}\bin\*.*"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\doc\*.*"
RMDir "$INSTDIR\3rdParty\${LDViewDir}\doc\*.*"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\resources\config\*.*"
RMDir "$INSTDIR\3rdParty\${LDViewDir}\resources\config\*.*"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\resources\*.*"
RMDir "$INSTDIR\3rdParty\${LDViewDir}\resources\*.*"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LDViewDir}\*.*"
RMDir "$INSTDIR\3rdParty\${LDViewDir}\*.*"
${EndIf}

; Remove 3rdParty LPub3DTrace folders
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin\*.*"
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}\bin\*.*"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\*.*"
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}\doc\*.*"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\*.*"
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\ini\*.*"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\*.*"
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\include\*.*"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config\*.*"
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\config\*.*"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\*.*"
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}\resources\*.*"
${EndIf}
${If} ${FileExists} "$INSTDIR\3rdParty\${LPub3DTraceDir}\*.*"
RMDir "$INSTDIR\3rdParty\${LPub3DTraceDir}\*.*"
${EndIf}

; Remove 3rdParty folder
${If} ${FileExists} "$INSTDIR\3rdParty\*.*"
RMDir "$INSTDIR\3rdParty\*.*"
${EndIf}