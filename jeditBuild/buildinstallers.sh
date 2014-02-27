#!/bin/sh
#run on Mac or Linux

#checkout v.5.1.1 of jEdit
svn co -r 23300 https://svn.code.sf.net/p/jedit/svn/jEdit/branches/5.1.x jedit_temp

cd jedit_temp
cp -r ../../AithonLibrary .
cp -r ../../ProjectTemplate .
#set +e #continue even with error
ant dist
cp ../../aithon_plugin/jars/Aithon.jar ./lib/default-plugins
#set -e

#copy over the most recent versions of 'ld' and 'windres' (needed for Mac installer)
cp ../ld-new ./build/launch4j/bin/ld
cp ../windres ./build/launch4j/bin/windres

#copy the modified build.xml
cp ../build.xml.modified_5.1.1 ./build.xml

#copy the splash image
cp ../splash_aithon.png ./org/gjt/sp/jedit/icons/splash.png

#need for InnoSetup (needed for Windows installer)
cp ../build.properties ./build.properties
cp ../win32installer.iss ./package-files/windows/win32installer.iss

#cp Aithon plugin


ant dist


