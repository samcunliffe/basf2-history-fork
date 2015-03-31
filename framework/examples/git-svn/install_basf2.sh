#!/usr/bin/env bash
# A script to do a fully automatic installation of basf2 and externals (using git-svn for basf2)

# Get it via svn export https://belle2.cc.kek.jp/svn/trunk/software/framework/examples/git-svn/install_basf2.sh
# and run bash install_basf2.sh inside your install directory.

#TODO: could start cloning at beginning, while tools are being installed...

#abort on errors
set -e

BASF2_SVN="https://belle2.cc.kek.jp/svn/trunk/software"

echo "This script will install the basf2 software, including tools and externals, into `pwd`,"
echo "creating the directories tools/, externals/ and basf2/."
echo "The basf2/ folder will be a git repository containing the entire history imported from SVN,"
echo "see https://belle2.cc.kek.jp/~twiki/bin/view/Software/GitSvn for details."
echo ""
echo "Press return to continue, Ctrl-c to abort."
read

if ! svn help >& /dev/null
then
  echo "subversion is not installed, please install it and try again."
  exit 1
fi

if ! git help svn >& /dev/null
then
  echo "git-svn is not installed, please install it and try again."
  exit 1
fi

if [ ! -d basf2 ]; then
  echo -n "Looking for current basf2 version..."
  CURRENT_REV=`svn log https://belle2.cc.kek.jp/svn/trunk/software | head -2 | tail -1 | awk '{print $1}'`
  echo " $CURRENT_REV"
else
  echo "basf2/ already exists."
fi

echo -n "Looking for current externals version..."
EXTERNALS_VERSION=`svn cat https://belle2.cc.kek.jp/svn/trunk/software/.externals`
echo " $EXTERNALS_VERSION."

if [ ! -d basf2 ]; then
  echo "================================================================================"
  echo "How much of the SVN history do you want to import? Select $CURRENT_REV for a quick checkout with history starting from the current revision, or 'all' for the entire history (.git requires about 900MB for storing the entire history, 220MB for the latest alone). Press Ctrl-c to abort."
  select SVN_CLONE_FROM in "all" $CURRENT_REV; do
    echo "Selected: $SVN_CLONE_FROM"
    break
  done
fi

#echo "Looking for available externals..."
#AVAILABLE_EXTERNALS=`get_externals.sh | head -n -1` #skip development, added manually
#
#echo "================================================================================"
#echo "Please select your preferred externals version. Ctrl-c to abort."
#select EXTERNALS_VERSION in $AVAILABLE_EXTERNALS development; do
#  echo "Selected: $EXTERNALS_VERSION"
#  break
#done

if [ ! -d externals/$EXTERNALS_VERSION ]
then
  if [ "$EXTERNALS_VERSION" == "development" ]; then
    BINARY_VARIANT=""
  else
    echo "================================================================================"
    echo "Select a binary variant of the externals for your system, or 'source' to compile them from source. All variants are for 64 bit unless mentioned otherwise. Press Ctrl-c to abort."
    select BINARY_VARIANT in "source" sl5 sl6 ubuntu1404 ubuntu1404_32bit; do
      if [ "$BINARY_VARIANT" == "source" ]; then
        BINARY_VARIANT=""
      fi
      break
    done
  fi
fi

if [ ! -d tools/ ]
then
  svn co https://belle2.cc.kek.jp/svn/trunk/tools
  ./tools/prepare_belle2.sh || true # optional packages still cause bad return value...
  ./tools/install.sh
fi

echo "Tools installed."
source tools/setup_belle2


echo "================================================================================"
if [ ! -d externals/ ]
then
  echo "Creating externals/ directory..."
  mkdir externals
fi

if [ ! -d externals/$EXTERNALS_VERSION ]
then
  echo "Now installing the externals... (logging to get_externals.log)"
  get_externals.sh $EXTERNALS_VERSION $BINARY_VARIANT >& get_externals.log &
  EXTERNALS_PID=$!
else
  echo "Externals $EXTERNALS_VERSION already installed, nothing to do."
  EXTERNALS_PID=$!
fi

if [ ! -d basf2 ]; then
  echo "Cloning SVN repository into basf2/... (this will take a while)"
  if [ "$SVN_CLONE_FROM" == "all" ]; then
    GIT_SVN_CLONE_ARGS=""
  else
    GIT_SVN_CLONE_ARGS="-$SVN_CLONE_FROM:HEAD"
  fi

  git svn clone $GIT_SVN_CLONE_ARGS $BASF2_SVN basf2
  # remaining setup
  pushd basf2
  ln -s site_scons/SConstruct .
  echo "head" > .release
  echo "$EXTERNALS_VERSION" > .externals

  echo "Installing pre-commit hook"
  cp framework/examples/git-svn/pre-commit .git/hooks/
  chmod 755 .git/hooks/pre-commit


  echo "Installing hooks to deal with svn:externals (e.g. genfit2 package)"
  cp framework/examples/git-svn/svnexternals.py .git/hooks/
  pushd .git/hooks > /dev/null
  chmod 755 svnexternals.py
  ln -s svnexternals.py post-rewrite
  ln -s svnexternals.py post-checkout
  popd > /dev/null
  popd > /dev/null


  echo "git svn clone finished."
else
  echo "basf2/ already exists, skipping git svn clone."
fi

echo "Waiting for externals install..."

if wait $EXTERNALS_PID; then
  echo "Externals install successful!"

  pushd basf2
  setuprel
  echo "Fetching svn:externals..."
  ./.git/hooks/svnexternals.py

  popd > /dev/null
else
  echo "Something went wrong during installation of externals, please check get_externals.log!"
  exit 1
fi

echo "================================================================================"
echo "Installation finished. You can now set up your basf2 environment using"
echo "  source tools/setup_belle2"
echo "  cd basf2"
echo "  setuprel"
echo "and compile basf2 using 'scons' (use -j4 to limit scons to only use e.g. 4 cores)"
