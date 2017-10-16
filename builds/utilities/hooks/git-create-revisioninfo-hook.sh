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
##   cp builds/utilities/git-create-revisioninfo-hook.sh .git/hooks/post-commit
##   cp builds/utilities/git-create-revisioninfo-hook.sh .git/hooks/post-checkout
##   cp builds/utilities/git-create-revisioninfo-hook.sh .git/hooks/post-merge
##   chmod +x .git/hooks/post-*

exec 1>&2
branch=`git rev-parse --abbrev-ref HEAD`
shorthash=`git log --pretty=format:'%h' -n 1`
revcount=`git log --oneline | wc -l`
latesttag=`git describe --tags --abbrev=0`

VERSION="[$branch]$latesttag-$revcount($shorthash)"

