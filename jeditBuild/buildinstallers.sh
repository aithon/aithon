#!/bin/sh
#run on Mac or Linux

#checkout v.5.1.1 of jEdit
svn co -r 23300 https://jedit.svn.sourceforge.net/svnroot/jedit/jEdit/branches/5.1.x jedit_temp

cd jedit_temp
cp -r ../../AithonLibrary .
#set +e #continue even with error
ant dist
#set -e

#copy over the most recent versions of 'ld' and 'windres'
cp ../ld-new ./build/launch4j/bin/ld
cp ../windres ./build/launch4j/bin/windres

cp ../build.xml.modified_5.1.1 ./build.xml

#need for InnoSetup
cp ../build.properties ./build.properties
cp ../win32installer.iss ./package-files/windows/win32installer.iss

#cp Aithon plugin


ant dist


