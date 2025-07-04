# If no version info can be determined, VERSION (in LP3D_VERSION_INFO format) will be used
VERSION = 2.4.9

# Need to discard STDERR so get path to NULL device
if (mingw:ide_qtcreator)|win32-msvc* {
    NULL_DEVICE = NUL # Windows doesn't have /dev/null but has NUL
} else {
    NULL_DEVICE = /dev/null
}

# Default location of Git directory
GIT_DIR = $$PWD/.git
exists($$GIT_DIR/*) {
    message("~~~ $${LPUB3D} GIT_DIR FOUND AT $$GIT_DIR ~~~")
} else {
    GIT_DIR = undefined
    message("~~~ $${LPUB3D} GIT_DIR UNDEFINED AT $$PWD ~~~")
}

# enable to Test
# GIT_DIR = undefined
equals(GIT_DIR, undefined) {

    OPEN_BUILD_SERVICE = $$(OBS)
    contains(OPEN_BUILD_SERVICE, true) {
        GIT_DIR_ENV = OBS
    } else:contains(CONFIG, snp) {
        GIT_DIR_ENV = SNAP
        UPDATE_CONFIG = 1
    } else:contains(CONFIG, flp) {
        GIT_DIR_ENV = FLATPAK
        UPDATE_CONFIG = 1
    } else:contains(CONFIG, con) {
        GIT_DIR_ENV = CONDA
    } else:contains(CONFIG, msys) {
        GIT_DIR_ENV = MSYS2
    } else {
        GIT_DIR_ENV = UNDEFINED
    }

    !isEmpty(UPDATE_CONFIG) {
        GIT_BASE_COMMAND = git -C $$PWD/.git
        GIT_FOUND = $$system($$GIT_BASE_COMMAND rev-list --count HEAD 2> $$NULL_DEVICE)
        !isEmpty(GIT_FOUND) {
            $$system(bash builds/utilities/update-config-files.sh $$shell_quote($$PWD/mainApp))
        }
    }

    VER_BASE_NAME    = $$lower($$(LP3D_APP))
    isEmpty(VER_BASE_NAME): \
    VER_BASE_NAME    = lpub3d-ci
    USE_GIT_VER_FILE = true

} else {
    # Need to call git with manually specified paths to repository
    GIT_BASE_COMMAND = git --git-dir $$shell_quote($$GIT_DIR) --work-tree $$shell_quote($$PWD)

    # Trying to get version from git tag / revision
    GIT_VERSION = $$system($$GIT_BASE_COMMAND describe --tags --match v* --long 2> $$NULL_DEVICE)
    #message("~~~ $${LPUB3D} DEBUG ~~ GIT_VERSION [RAW]: $$GIT_VERSION")

    # Check if we do not have a valid version number (i.e. no version tag found)
    isEmpty(GIT_VERSION) {
        GIT_REVISION = 282
        GIT_SHA      = $$system($$GIT_BASE_COMMAND rev-parse --short HEAD 2> $$NULL_DEVICE)
        GIT_COMMIT   = $$system($$GIT_BASE_COMMAND rev-list --count HEAD 2> $$NULL_DEVICE)
        GIT_VERSION  = v$${VERSION}-$${GIT_REVISION}-$${GIT_SHA}
        contains(CONFIG, snp): GIT_DIR_ENV = SNAP
        else:contains(CONFIG, flp): GIT_DIR_ENV = FLATPAK
        if(isEmpty(GIT_SHA)|!isEmpty(GIT_DIR_ENV)) {
            USE_GIT_VER_FILE = true
        } else {
            VERSION_INFO_VAR = $$(LP3D_VERSION_INFO)
            USING_GITHUB = $$(GITHUB)
            # equals(USING_GITHUB, true): message("~~~ $${LPUB3D} DEBUG ~~ NOTICE: USING GITHUB ~~~")
            !isEmpty(VERSION_INFO_VAR) {
                USE_VERSION_INFO_VAR = true
                message("~~~ ALERT LPUB3D! GIT TAG NOT FOUND, USING LP3D_VERSION_INFO $$VERSION_INFO_VAR ~~~")
            } else: equals(USING_GITHUB, true) {
                USE_GIT_VER_FILE = true
                message("~~~ ALERT LPUB3D! GIT TAG AND LP3D_VERSION_INFO NOT FOUND ~~~")
            } else {
                message("~~~ ALERT LPUB3D! GIT TAG AND LP3D_VERSION_INFO NOT FOUND, USING HEAD $$GIT_VERSION ~~~")
            }
        }
        isEmpty(GIT_DIR_ENV): GIT_DIR_ENV = UNDEFINED
    }

    !equals(USE_GIT_VER_FILE, true):!equals(USE_VERSION_INFO_VAR, true) {
        # Get commit count
        GIT_COMMIT = $$system($$GIT_BASE_COMMAND rev-list --count HEAD 2> $$NULL_DEVICE)
        isEmpty(GIT_COMMIT) {
            GIT_COMMIT = 4563
            message("~~~ ERROR LPUB3D! GIT_COMMIT NOT DEFINED, USING $$GIT_COMMIT ~~~")
        }

        # Append commit count
        GIT_VERSION = g$$GIT_VERSION-$$GIT_COMMIT

        # Token position       0 1 2 3 4       5    [6]
        # Version string       2 4 3 1 410fdd7 2220 beta1
        #message("~~~ DEBUG ~~ GIT_VERSION [RAW]: $$GIT_VERSION")

        # Convert output from gv2.4.3-1-g410fdd7-2220 into "2.4.3.1.g410fdd7.2220"
        GIT_VERSION ~= s/-/"."
        GIT_VERSION ~= s/gv/""
        #message(~~~ DEBUG ~~ GIT_VERSION [FORMATTED]: $$GIT_VERSION)

        # Separate the build number into major, minor and service pack etc.
        VER_MAJOR      = $$section(GIT_VERSION, ., 0, 0)
        VER_MINOR      = $$section(GIT_VERSION, ., 1, 1)
        VER_PATCH      = $$section(GIT_VERSION, ., 2, 2)
        VER_REVISION   = $$section(GIT_VERSION, ., 3, 3)
        VER_GIT_SHA    = $$section(GIT_VERSION, ., 4, 4)
        VER_COMMIT     = $$section(GIT_VERSION, ., 5, 5)
        # Capture and convert version suffix - everything after "_" if it exist
        VER_PATCH_TEMP = $$section(GIT_VERSION, ., 2, 2)
        VER_SUFFIX     = $$section(VER_PATCH_TEMP, _, 1)
        !isEmpty(VER_SUFFIX): \
            VER_PATCH  = $$section(VER_PATCH_TEMP, _, 0, 0)

        # Strip leading 'g' from sha hash
        VER_GIT_SHA ~= s/g/""
    }

    # Get the git repository name
    GIT_BASE_NAME = $$system($$GIT_BASE_COMMAND rev-parse --show-toplevel 2> $$NULL_DEVICE)
    VER_BASE_NAME = $$lower($$basename(GIT_BASE_NAME))
    message("~~~ $${LPUB3D} USING GIT_BASE_NAME $$GIT_BASE_NAME ~~~")
}

if (equals(USE_GIT_VER_FILE, true)|equals(USE_VERSION_INFO_VAR, true)) {
    equals(USE_GIT_VER_FILE, true) {
        GIT_VER_FILE = $$PWD/builds/utilities/version.info

        exists($$GIT_VER_FILE) {
            message("~~~ $${LPUB3D} GIT_DIR [$$GIT_DIR_ENV, USING VERSION_INFO FILE] $$GIT_VER_FILE ~~~")
            GIT_VERSION = $$cat($$GIT_VER_FILE, lines)
        } else {
            message("~~~ ERROR LPUB3D! $$GIT_DIR_ENV VERSION_INFO FILE $$GIT_VER_FILE NOT FOUND ~~~")
            GIT_VERSION = $${VERSION}.282.4563.e5418aca0
            message("~~~ $${LPUB3D} GIT_DIR [$$GIT_DIR_ENV, USING VERSION] $$GIT_VERSION ~~~")
            GIT_VERSION ~= s/\./" "
        }
    } else: equals(USE_VERSION_INFO_VAR, true) {
        GIT_VERSION = $$VERSION_INFO_VAR
    }

    # Token position   0 1 2 3 4    5       [6]
    # Version string   2 4 3 1 2220 f32acd5 beta1
    #message("~~~ DEBUG ~~ GIT_VERSION [FILE]: $$GIT_VERSION")

    # Separate version info into major, minor, patch, revision etc...
    VER_MAJOR    = $$section(GIT_VERSION, " ", 0, 0)
    VER_MINOR    = $$section(GIT_VERSION, " ", 1, 1)
    VER_PATCH    = $$section(GIT_VERSION, " ", 2, 2)
    VER_REVISION = $$section(GIT_VERSION, " ", 3, 3)
    VER_COMMIT   = $$section(GIT_VERSION, " ", 4, 4)
    !isEmpty(GIT_SHA): \
    VER_GIT_SHA  = $$GIT_SHA
    else: \
    VER_GIT_SHA  = $$section(GIT_VERSION, " ", 5, 5)
    VER_SUFFIX   = $$section(GIT_VERSION, " ", 6, 6)
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

# Separate the date into 'DD/MM/YYYY' format
if (github_ci_win | azure_ci_win | appveyor_ci_win) {
    message("~~~ $${LPUB3D} BUILD DATE 'Day MM/DD/YYYY': $$BUILD_DATE ~~~")
    BUILD_DATE ~= s/[\sA-Za-z\s]/""
    DATE_MM = $$section(BUILD_DATE, /, 0, 0)
    DATE_DD = $$section(BUILD_DATE, /, 1, 1)
    DATE_YY = $$section(BUILD_DATE, /, 2, 2)
} else {
    message("~~~ $${LPUB3D} BUILD DATE 'DD/MM/YYYY': $$BUILD_DATE ~~~")
    DATE_DD = $$section(BUILD_DATE, /, 0, 0)
    DATE_DD = $$last(DATE_DD)
    DATE_MM = $$section(BUILD_DATE, /, 1, 1)
    DATE_YY = $$section(BUILD_DATE, /, 2, 2)
}

# Resolve Continuous Build version number
TRAVIS_BUILD   = $$(TRAVIS_TAG)
APPVEYOR_BUILD = $$(APPVEYOR_REPO_TAG_NAME)
LOCAL_BUILD    = $$system($$GIT_BASE_COMMAND describe --tags --abbrev=0 2> $$NULL_DEVICE)
contains(TRAVIS_BUILD,continuous) {
    BUILD_TYPE = $$TRAVIS_BUILD
} else:contains(APPVEYOR_BUILD,continuous) {
    BUILD_TYPE = $$APPVEYOR_BUILD
} else:contains(LOCAL_BUILD,continuous) {
    BUILD_TYPE = $$LOCAL_BUILD
}

# C preprocessor DEFINES passed to application
contains(BUILD_TYPE,continuous) {
    DEFINES += LP3D_CONTINUOUS_BUILD
}

if (contains(VER_BASE_NAME, $$lower($${TARGET}))) {
    message("~~~ $${LPUB3D} USING RELEASE BUILD VER_BASE_NAME: $$VER_BASE_NAME ~~~")
} else:contains(VER_BASE_NAME, $$lower("$${TARGET}next")) {
    message("~~~ $${LPUB3D} USING NEXT DEVEL BUILD VER_BASE_NAME: $$VER_BASE_NAME ~~~")
    DEFINES += LP3D_NEXT_BUILD
} else {
    message("~~~ $${LPUB3D} USING CI DEVEL BUILD VER_BASE_NAME: $$VER_BASE_NAME ~~~")
    DEFINES += LP3D_DEVOPS_BUILD
}

DEFINES += VER_MAJOR=\\\"$$VER_MAJOR\\\"
DEFINES += VER_MINOR=\\\"$$VER_MINOR\\\"
DEFINES += VER_PATCH=\\\"$$VER_PATCH\\\"

DEFINES += BUILD_TIME=\\\"$$BUILD_TIME\\\"
DEFINES += DATE_YY=\\\"$$DATE_YY\\\"
DEFINES += DATE_MM=\\\"$$DATE_MM\\\"
DEFINES += DATE_DD=\\\"$$DATE_DD\\\"

DEFINES += VER_COMMIT_STR=\\\"$$VER_COMMIT\\\"
DEFINES += VER_GIT_SHA_STR=\\\"$$VER_GIT_SHA\\\"
DEFINES += VER_REVISION_STR=\\\"$$VER_REVISION\\\"

LP3D_VERSION_INFO = $$VER_MAJOR $$VER_MINOR $$VER_PATCH $$VER_REVISION $$VER_COMMIT $$VER_GIT_SHA
!isEmpty(VER_SUFFIX) {
    DEFINES += VER_SUFFIX=\\\"$$VER_SUFFIX\\\"
    LP3D_VERSION_INFO += $$VER_SUFFIX
}

# Now we are ready to pass parsed version to Qt
VERSION = $$VER_MAJOR"."$$VER_MINOR"."$$VER_PATCH"."$$VER_REVISION

# Display the complete version string
message("~~~ $${LPUB3D} LP3D_VERSION_INFO: $$LP3D_VERSION_INFO $$upper($$BUILD_TYPE) ~~~")
