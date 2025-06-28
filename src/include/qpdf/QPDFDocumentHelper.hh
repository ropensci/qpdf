// Copyright (c) 2005-2021 Jay Berkenbilt
// Copyright (c) 2022-2025 Jay Berkenbilt and Manfred Holger
//
// This file is part of qpdf.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied. See the License for the specific language governing permissions and limitations under
// the License.
//
// Versions of qpdf prior to version 7 were released under the terms of version 2.0 of the Artistic
// License. At your option, you may continue to consider qpdf to be licensed under those terms.
// Please see the manual for additional information.

#ifndef QPDFDOCUMENTHELPER_HH
#define QPDFDOCUMENTHELPER_HH

#include <qpdf/DLL.h>
#include <qpdf/QPDF.hh>

// This is a base class for QPDF Document Helper classes. Document helpers are classes that provide
// a convenient, higher-level API for accessing document-level structures within a PDF file.
// Document helpers are always initialized with a reference to a QPDF object, and the object can
// always be retrieved. The intention is that you may freely intermix use of document helpers with
// the underlying QPDF object unless there is a specific comment in a specific helper method that
// says otherwise. The pattern of using helper objects was introduced to allow creation of higher
// level helper functions without polluting the public interface of QPDF.
class QPDF_DLL_CLASS QPDFDocumentHelper
{
  public:
    QPDFDocumentHelper(QPDF& qpdf) :
        qpdf(qpdf)
    {
    }
    QPDF_DLL
    virtual ~QPDFDocumentHelper();
    QPDF&
    getQPDF()
    {
        return qpdf;
    }
    QPDF const&
    getQPDF() const
    {
        return qpdf;
    }

  protected:
    QPDF& qpdf;
};

#endif // QPDFDOCUMENTHELPER_HH
