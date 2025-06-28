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

#ifndef PL_RUNLENGTH_HH
#define PL_RUNLENGTH_HH

#include <qpdf/Pipeline.hh>

class QPDF_DLL_CLASS Pl_RunLength: public Pipeline
{
  public:
    enum action_e { a_encode, a_decode };

    QPDF_DLL
    Pl_RunLength(char const* identifier, Pipeline* next, action_e action);
    QPDF_DLL
    ~Pl_RunLength() override;

    // Limit the memory used.
    // NB This is a static option affecting all Pl_RunLength instances.
    QPDF_DLL
    static void setMemoryLimit(unsigned long long limit);

    QPDF_DLL
    void write(unsigned char const* data, size_t len) override;
    QPDF_DLL
    void finish() override;

  private:
    QPDF_DLL_PRIVATE
    void encode(unsigned char const* data, size_t len);
    QPDF_DLL_PRIVATE
    void decode(unsigned char const* data, size_t len);
    QPDF_DLL_PRIVATE
    void flush_encode();

    enum state_e { st_top, st_copying, st_run };

    class Members;

    std::unique_ptr<Members> m;
};

#endif // PL_RUNLENGTH_HH
