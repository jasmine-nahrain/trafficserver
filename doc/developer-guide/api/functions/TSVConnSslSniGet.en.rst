.. Licensed to the Apache Software Foundation (ASF) under one or more
   contributor license agreements.  See the NOTICE file distributed
   with this work for additional information regarding copyright
   ownership.  The ASF licenses this file to you under the Apache
   License, Version 2.0 (the "License"); you may not use this file
   except in compliance with the License.  You may obtain a copy of
   the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
   implied.  See the License for the specific language governing
   permissions and limitations under the License.

.. default-domain:: cpp

TSVConnSslSniGet
================

Synopsis
--------

.. code-block:: cpp

    #include <ts/ts.h>

.. function:: const char TSVConnSslSniGet(TSVConn sslp, int *length)


Description
-----------
Get the SNI (Server Name Indication) that corresponds to SSL connection :arg:`sslp`.

If :arg:`length` is not null, the length of the returned string (or 0 if this function returns null) will be stored.
