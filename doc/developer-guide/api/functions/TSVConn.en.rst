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

.. include:: ../../../common.defs

.. default-domain:: cpp

TSVConn
*******

Traffic Server APIs to get :type:`TSVConn` from :type:`TSHttpSsn` or :type:`TSHttpTxn` object.

Synopsis
========

.. code-block:: cpp

    #include <ts/ts.h>

.. function:: TSVConn TSHttpSsnClientVConnGet(TSHttpSsn ssnp)
.. function:: TSVConn TSHttpSsnServerVConnGet(TSHttpSsn ssnp)
.. function:: TSVConn TSHttpTxnServerVConnGet(TSHttpTxn txnp)

Description
===========

These APIs allow the developer to get the NetVconnection (represented by :type:`TSVConn`) from the Http session (:type:`TSHttpSsn`) or transaction (:type:`TSHttpTxn`) object.

:func:`TSHttpSsnClientVConnGet` returns the :type:`TSVConn` associated with the client side :type:`TSHttpSsn` object.
:func:`TSHttpSsnServerVConnGet` returns the same associated with the server side :type:`TSHttpSsn`.
:func:`TSHttpTxnServerVConnGet` returns the same associated with a :type:`TSHttpTxn`.
