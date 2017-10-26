#!/bin/bash
#
# This pre-commit hook updates the LPub3D version number in appveyor.yml when you go to commit.
#
# To use this script copy it to .git/hooks/pre-commit and make it executable.
#
## Automatically generate git branch and revision info
##
## Example:
##   [master]v2.0.0-beta-191(a830382)
## Install:
##   pre-commit:
##   cp builds/utilities/git-create-revisioninfo-hook.sh .git/hooks/pre-commit
##   cp builds/utilities/git-create-revisioninfo-hook.sh .git/hooks/pre-checkout
##   cp builds/utilities/git-create-revisioninfo-hook.sh .git/hooks/pre-merge
##   chmod +x .git/hooks/pre-*
##
##  post-commit:
##   cp builds/utilities/git-create-revisioninfo-hook.sh .git/hooks/post-commit
##   cp builds/utilities/git-create-revisioninfo-hook.sh .git/hooks/post-checkout
##   cp builds/utilities/git-create-revisioninfo-hook.sh .git/hooks/post-merge
##   chmod +x .git/hooks/post-*

##  LPUB3D VERSION: VER_MAJOR VER_MINOR VER_PATCH VER_REVISION VER_BUILD VER_SHA_HASH
exec 1>&2
branch=`git rev-parse --abbrev-ref HEAD`
tag_long=`git describe --tags --long`
tag_short=`git describe --tags --abbrev=0`
commit_count=`git rev-list HEAD --count`
sha_hash_short=`git rev-parse --short HEAD`
date=`date "+%Y%m%d"`
date_time=`date +%d\ %m\ %Y\ %H:%M:%S`

files_commit=`git diff HEAD --name-only`
valid_commit="yes"
if test x"files_commit" = x""; then valid_commit="no"; fi

tmp1=${tag_long#*-}       # remove prefix ending in "-"
tmp2=${tag_short//./" "}  # replace . with " "
g_version=${tmp2/v/}       # replace v with ""
revision=${tmp1%-*}
#         1 2 3  4  5   6
# format "2 0 20 17 663 410fdd7"
read major minor patch <<< ${g_version}
old_revision=$revision
old_commit_count=$commit_count
if test x"$valid_commit" = x"yes"; then revision=`expr $old_revision + 1`; commit_count=`expr $old_commit_count + 1`; fi
VERSIONINFO=$major" "$minor" "$patch" "$revision" "$commit_count" "$sha_hash_short
working_dir=$PWD/mainApp

echo " Git version info:"
# echo
# echo "valid_commit.......$valid_commit"
# echo
# echo "lpub3d version.....$tag_short_$date"
# echo "branch.............$branch"
# echo "sha_hash_short.....$sha_hash_short"
# echo "tag_short..........$tag_short"
# echo "tag_long...........$tag_long"
# echo "date...............$date"
# echo "date_time..........$date_time"
# echo
echo "old_revision..............$old_revision"
# echo "revision...........$revision"
echo "old_commit_count [build]..$old_commit_count"
# echo "commit_count.......$commit_count"
echo
# echo "VERSIONINFO........$VERSIONINFO"

chmod +x builds/utilities/update-config-files.sh
./builds/utilities/update-config-files.sh $working_dir $major $minor $patch $revision $commit_count $sha_hash_short



# echo " Create file version info"
# FILE="git.pre-commit.version.info"
# if [ -f ${FILE} -a -r ${FILE} ]
# then
	# rm ${FILE}
# fi
# cat <<EOF >${FILE}
# valid_commit   = $valid_commit

# lpub3d version   = $tag_short_$date
# branch           = $branch
# sha_hash_short   = $sha_hash_short
# tag_short        = $tag_short
# tag_long         = $tag_long
# date             = $date
# date_time        = $date_time

# old_revision     = $old_revision
# revision         = $revision
# old_commit_count = $old_commit_count
# commit_count     = $commit_count

# VERSIONINFO      = $VERSIONINFO
# EOF
# echo " And here's the file we created:"
# cat ${FILE}

