// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

/*
 * DetectorException.h
 *
 *  Created on: Aug 26, 2011
 *      Author: luiz
 */

#ifndef DETECTOREXCEPTION_H_
#define DETECTOREXCEPTION_H_

#include <zxing/Exception.h>

namespace zxing {
namespace datamatrix {

class DetectorException : public Exception {
  public:
    DetectorException(const char *msg);
    virtual ~DetectorException() throw();
};
} /* namespace nexxera */
} /* namespace zxing */
#endif /* DETECTOREXCEPTION_H_ */
