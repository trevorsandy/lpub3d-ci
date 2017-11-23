# If there is no version tag in git this one will be used
VERSION = 1.0.0

# Need to discard STDERR so get path to NULL device
win32 {
    NULL_DEVICE = NUL # Windows doesn't have /dev/null but has NUL
} else {
    NULL_DEVICE = /dev/null
}

GIT_DIR = undefined
# Default location of Git directory
exists($$PWD/.git) {
    GIT_DIR = $$PWD/.git
    message("~~~ GIT_DIR [DEFAULT] $$GIT_DIR ~~~")
}
appveyor_qt_mingw64: exists($$(LP3D_WIN_GIT)) {
    WIN_GIT = $$(LP3D_WIN_GIT)\\git
    message("~~~ LP3D_WIN_GIT PATH $$WIN_GIT IS VALID ~~~")
} else {
	WIN_GIT = git
}

# AppVeyor 64bit Qt MinGW build has git.exe/cygwin conflict returning no .git directory found so use version.info file
# appveyor_qt_mingw64: GIT_DIR = undefined
equals(GIT_DIR, undefined) {
    appveyor_qt_mingw64 {
        BUILD_TYPE = release
        CONFIG(debug, debug|release): BUILD_TYPE = debug
        message("~~~ GIT_DIR [APPVEYOR, USING VERSION_INFO FILE] $$GIT_VER_FILE ~~~")
        # Trying to get version from git tag / revision
        RET = $$system($$PWD/builds/utilities/update-config-files.bat $$_PRO_FILE_PWD_ $$basename(PARENT_FOLDER))
    } else {
        message("~~~ GIT_DIR [UNDEFINED, USING VERSION_INFO FILE] $$GIT_VER_FILE ~~~")
    }
    GIT_VER_FILE = $$PWD/builds/utilities/version.info
    exists($$GIT_VER_FILE) {
        GIT_VERSION = $$cat($$GIT_VER_FILE, lines)
    } else {
        message("~~~ ERROR! GIT_DIR $$GIT_VER_FILE NOT FOUND ~~~")
        GIT_VERSION = $${VERSION}-00-00000000-000
    }
    # Token position       0 1 2  3  4   5
    # Version string       2 0 20 17 663 410fdd7
    GIT_VERSION ~= s/\\\"/""
    #message(~~~ DEBUG ~~ GIT_VERSION [FROM FILE RAW]: $$GIT_VERSION)

    # Separate the build number into major, minor and service pack etc.
    VER_MAJOR = $$section(GIT_VERSION, " ", 0, 0)
    VER_MINOR = $$section(GIT_VERSION, " ", 1, 1)
    VER_PATCH = $$section(GIT_VERSION, " ", 2, 2)
    VER_REVISION_STR = $$section(GIT_VERSION, " ", 3, 3)
    VER_BUILD_STR = $$section(GIT_VERSION, " ", 4, 4)
    VER_SHA_HASH_STR = $$section(GIT_VERSION, " ", 5, 5)

} else {
    # Need to call git with manually specified paths to repository
	BASE_GIT_COMMAND = $${WIN_GIT} --git-dir $$shell_quote$$GIT_DIR --work-tree $$shell_quote$$PWD

    # Trying to get version from git tag / revision
    GIT_VERSION = $$system($$BASE_GIT_COMMAND describe --long 2> $$NULL_DEVICE)

    # Check if we only have hash without version number (i.e. not version tag found)
    !contains(GIT_VERSION,\d+\.\d+\.\d+) {
        # If there is nothing we simply use version defined manually
        isEmpty(GIT_VERSION) {
            GIT_VERSION = $${VERSION}-00-00000000-000
            message("~~~ ERROR! GIT_VERSION NOT DEFINED, USING $$GIT_VERSION ~~~")
        } else { # otherwise construct proper git describe string
            GIT_COMMIT_COUNT = $$system($$BASE_GIT_COMMAND rev-list HEAD --count 2> $$NULL_DEVICE)
            isEmpty(GIT_COMMIT_COUNT) {
                GIT_COMMIT_COUNT = 0
                message("~~~ ERROR! GIT_COMMIT_COUNT NOT DEFINED, USING $$GIT_COMMIT_COUNT ~~~")
            }
            GIT_VERSION = g$$GIT_VERSION-$$GIT_COMMIT_COUNT
        }
    }
    #message(~~~ DEBUG ~~ GIT_VERSION [RAW]: $$GIT_VERSION)

    # Convert output from gv2.0.20-37-ge99beed-600 into "gv2.0.20.37.ge99beed.600"
    GIT_VERSION ~= s/-/"."
    GIT_VERSION ~= s/g/""
    GIT_VERSION ~= s/v/""
    #message(~~~ DEBUG ~~ GIT_VERSION [FORMATTED]: $$GIT_VERSION)

    # Separate the build number into major, minor and service pack etc.
    VER_MAJOR = $$section(GIT_VERSION, ., 0, 0)
    VER_MINOR = $$section(GIT_VERSION, ., 1, 1)
    VER_PATCH = $$section(GIT_VERSION, ., 2, 2)
    VER_REVISION_STR = $$section(GIT_VERSION, ., 3, 3)
    VER_SHA_HASH_STR = $$section(GIT_VERSION, ., 4, 4)
    VER_BUILD_STR = $$section(GIT_VERSION, ., 5, 5)
}

# Here we process the build date and time
win32 {
    BUILD_DATE = $$system( date /t )
    BUILD_TIME = $$system( echo %time% )
} else {
    BUILD_DATE = $$system( date "+%d/%m/%Y/%H:%M:%S" )
    BUILD_TIME = $$section(BUILD_DATE, /, 3, 3)
}
#message(~~~ DEBUG ~~ BUILD_DATE: $$BUILD_DATE) # output the current date
#message(~~~ DEBUG ~~ BUILD_TIME: $$BUILD_TIME) # output the current time

# Separate the date into day month, year
_APPVEYOR = $$(APPVEYOR)
contains(_APPVEYOR),True) {
    # AppVeyor CI uses date format 'Day MM/DD/YY'
    message("~~~ USING APPVEYOR WINDOWS DATE FORMAT")
    BUILD_DATE ~= s/[\sA-Za-z\s]/""
    DATE_MM = $$section(BUILD_DATE, /, 0, 0)
    DATE_DD = $$section(BUILD_DATE, /, 1, 1)
    DATE_YY = $$section(BUILD_DATE, /, 2, 2)
} else {
    DATE_DD = $$section(BUILD_DATE, /, 0, 0)
    DATE_MM = $$section(BUILD_DATE, /, 1, 1)
    DATE_YY = $$section(BUILD_DATE, /, 2, 2)
}

# C preprocessor #DEFINE to use in C++ code
DEFINES += VER_MAJOR=\\\"$$VER_MAJOR\\\"
DEFINES += VER_MINOR=\\\"$$VER_MINOR\\\"
DEFINES += VER_PATCH=\\\"$$VER_PATCH\\\"

DEFINES += BUILD_TIME=\\\"$$BUILD_TIME\\\"
DEFINES += DATE_YY=\\\"$$DATE_YY\\\"
DEFINES += DATE_MM=\\\"$$DATE_MM\\\"
DEFINES += DATE_DD=\\\"$$DATE_DD\\\"

DEFINES += VER_BUILD_STR=\\\"$$VER_BUILD_STR\\\"
DEFINES += VER_SHA_HASH_STR=\\\"$$VER_SHA_HASH_STR\\\"
DEFINES += VER_REVISION_STR=\\\"$$VER_REVISION_STR\\\"

# Now we are ready to pass parsed version to Qt ===
VERSION = $$VER_MAJOR"."$$VER_MINOR"."$$VER_PATCH

# Update the version number file for win/unix during build
# Generate git version data to the input files indicated. Input files are consumed during the
# build process to set the version informatio for LPub3D executable, its libraries (ldrawini and quazip)
# Update the application version in lpub3d.desktop (desktop configuration file), lpub3d.1 (man page)
# This flag will also add the version number to packaging configuration files PKGBUILD, changelog and
# lpub3d.spec depending on which build is being performed.
message(~~~ VERSION_INFO: $$VER_MAJOR $$VER_MINOR $$VER_PATCH $$VER_REVISION_STR $$VER_BUILD_STR $$VER_SHA_HASH_STR ~~~)
