// <!--
// The MIT License (MIT)
//
// Copyright (c) 2024 Kris Jusiak <kris@jusiak.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
module;
#include "reflect"
export module reflect;

export namespace reflect {
  using reflect::visit;
  using reflect::size;
  using reflect::type_name;
  using reflect::type_id;
  using reflect::type_id_v;
  using reflect::to_underlying;
  using reflect::enum_min;
  using reflect::enum_max;
  using reflect::enum_name;
  using reflect::member_name;
  using reflect::get;
  using reflect::has_member_name;
  using reflect::copy;
  using reflect::to;
  using reflect::size_of;
  using reflect::align_of;
  using reflect::offset_of;
  using reflect::for_each;
} // namespace reflect

// -*- mode: c++; -*-
// vim: set filetype=cpp:
