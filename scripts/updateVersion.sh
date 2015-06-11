. ./loadVersion.sh
	
# write it to AndroidManifest.xml

echo "Updating AndroidManifest.xml"

sed -i.bak "s/android:versionName=\"[[:digit:]\.]*\"/android:versionName=\"$versionString\"/" ../platforms/android/AndroidManifest.xml
sed -i.bak "s/android:versionCode=\"[[:digit:]]*\"/android:versionCode=\"$versionRolling\"/" ../platforms/android/AndroidManifest.xml


# write it to Info.plist

echo "Updating Info.plist"

# ios
perl -i.bak -p -000 -e 's/(<key>CFBundleShortVersionString<\/key>\s*<string>)[0-9\.]+(<\/string>)/$1$ENV{versionString}$2/' ../platforms/ios/Info.plist
perl -i.bak -p -000 -e 's/(<key>CFBundleVersion<\/key>\s*<string>)[0-9\.]+(<\/string>)/$1$ENV{versionRolling}$2/' ../platforms/ios/Info.plist

# osx
perl -i.bak -p -000 -e 's/(<key>CFBundleShortVersionString<\/key>\s*<string>)[0-9\.]+(<\/string>)/$1$ENV{versionString}$2/' ../platforms/osx/Info.plist
perl -i.bak -p -000 -e 's/(<key>CFBundleVersion<\/key>\s*<string>)[0-9\.]+(<\/string>)/$1$ENV{versionRolling}$2/' ../platforms/osx/Info.plist

echo Write it to the installer
sed -i.bak "s/<Version>[[:digit:]\.]*<\/Version>/<Version>$versionString<\/Version>/" ../appstore/installer/packages/org.entropytuner.app/meta/package.xml
sed -i.bak "s/<Version>[[:digit:]\.]*<\/Version>/<Version>$versionString<\/Version>/" ../appstore/installer/config/config.xml
sed -i.bak "s/<Version>[[:digit:]\.]*<\/Version>/<Version>$depsVersionString<\/Version>/" ../appstore/installer/packages/org.entropytuner.deps/meta/package.xml

echo Update date in the installer
sed -i.bak "s/<ReleaseDate>[[:digit:]\-]*<\/ReleaseDate>/<ReleaseDate>$releaseDate<\/ReleaseDate>/" ../appstore/installer/packages/org.entropytuner.app/meta/package.xml
sed -i.bak "s/<ReleaseDate>[[:digit:]\-]*<\/ReleaseDate>/<ReleaseDate>$releaseDate<\/ReleaseDate>/" ../appstore/installer/packages/org.entropytuner.deps/meta/package.xml

echo Update the version.xml file
echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<version>
	<app rolling=\"$versionRolling\" string=\"$versionString\" />
	<dependencies rolling=\"$depsVersionRolling\" string=\"$depsVersionString\" />
</version>" > ../appstore/installer/version.xml
