// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

/*
 *  FinderPatternInfo.cpp
 *  zxing
 *
 *  Created by Christian Brunschen on 13/05/2008.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <zxing/qrcode/detector/FinderPatternInfo.h>

namespace zxing {
namespace qrcode {

FinderPatternInfo::FinderPatternInfo(std::vector<Ref<FinderPattern> > patternCenters) :
    bottomLeft_(patternCenters[0]), topLeft_(patternCenters[1]), topRight_(patternCenters[2]) {
}

Ref<FinderPattern> FinderPatternInfo::getBottomLeft() {
  return bottomLeft_;
}
Ref<FinderPattern> FinderPatternInfo::getTopLeft() {
  return topLeft_;
}
Ref<FinderPattern> FinderPatternInfo::getTopRight() {
  return topRight_;
}

}
}
