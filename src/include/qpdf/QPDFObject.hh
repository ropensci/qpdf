// Copyright (c) 2005-2021 Jay Berkenbilt
// Copyright (c) 2022-2025 Jay Berkenbilt and Manfred Holger
//
// This file is part of qpdf.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Versions of qpdf prior to version 7 were released under the terms
// of version 2.0 of the Artistic License. At your option, you may
// continue to consider qpdf to be licensed under those terms. Please
// see the manual for additional information.

#ifndef QPDFOBJECT_OLD_HH
#define QPDFOBJECT_OLD_HH

// Current code should not include <qpdf/QPDFObject.hh>. This file exists
// to ensure that code that includes it doesn't accidentally work because
// of an old qpdf installed on the system. Including this file became an
// error with qpdf version 12. The internal QPDFObject API is defined in
// QPDFObject_private.hh, which is not part of the public API.

// Instead of including this header, include <qpdf/Constants.h>, and
// replace `QPDFObject::ot_` with `::ot_` in your code.
#error "QPDFObject.hh is obsolete; see comments in QPDFObject.hh for details"

#endif // QPDFOBJECT_OLD_HH
