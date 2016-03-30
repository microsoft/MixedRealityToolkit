#!/bin/bash

xcodebuild -project Src/External/zint-2.4.3/src/Xcode/zint/zint.xcodeproj -scheme zint -configuration Release clean build

xcodebuild -project Src/External/zxing-cpp/src/Project/Xcode/zxing/zxing.xcodeproj -scheme zxing -configuration Release clean build

cd Src/External/apriltag-2015-03-18/
./BuildLib.sh
cd ../../../

chmod a+x Src/External/swigwin-3.0.2/swig

xcodebuild -project Src/Solutions/Xcode/XToolsClient/XToolsClient.xcodeproj -scheme FullBuild -configuration Release clean build
