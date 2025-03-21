/** @file

  A brief file description

  @section license License

  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

/****************************************************************************

  I_UDPConnection.h
  UDPConnection interface


 ****************************************************************************/

#pragma once

#include "iocore/eventsystem/Action.h"
#include "iocore/eventsystem/Continuation.h"
#include "iocore/eventsystem/UnixSocket.h"
#define INK_ETHERNET_MTU_SIZE 1472
class UDPPacket;
/**
   UDP Connection endpoint

   You can schedule packet to be sent immediately or for the future,
   and set up a persistent receive() operation.
 */

class UDPConnection : public Continuation
{
public:
  ~UDPConnection() override{};

  SOCKET getFd();
  void   setBinding(struct sockaddr const *);
  void   setBinding(const IpAddr &, in_port_t);
  bool   getBinding(struct sockaddr *);

  void destroy();
  int  shouldDestroy();
  /**
     <p>
     <b>Callbacks:</b><br>
     cont->handleEvent(NET_EVENT_DATAGRAM_WRITE_ERROR, UDPPacket *) on error
     <br>
     no callback on success.

     @return Action* send can't be cancelled via this Action
     @param c continuation to be called back
     @param p packet to be sent.
   */
  Action *send(Continuation *c, UDPPacket *p);

  /**
     <p>
     <b>Callbacks:</b><br>
     cont->handleEvent(NET_EVENT_DATAGRAM_ERROR, UDPConnection *) on error
     <br>
     cont->handleEvent(NET_EVENT_DATAGRAM_READ_READY, Queue&lt;UDPPacket&gt; *) on incoming packets.

     @return Action* Always returns nullptr.  Can't be
     cancelled via this Action.
     @param c continuation to be called back
   */
  Action *recv(Continuation *c);

  void Release();
  void AddRef();
  int  GetRefCount();

  int getPortNum();

  int  GetSendGenerationNumber(); // const
  void setContinuation(Continuation *c);

  /**
     Put socket on net queue for read/write polling.

     Not required for UDPConnections created with UDPNetProcessor::UDPBind

     Required for  and UDPNetProcessor::CreateUDPSocket.  They  don't do
     bindToThread() automatically so that the sockets can be passed to
     other Continuations.
  */
  void bindToThread(Continuation *c, EThread *t);

  virtual void UDPConnection_is_abstract() = 0;

  // this is for doing packet scheduling: we keep two values so that we can
  // implement cancel.  The first value tracks the startTime of the last
  // packet that was sent on this connection; the second value tracks the
  // startTime of the last packet when we are doing scheduling;  whenever the
  // associated continuation cancels a packet, we rest lastPktStartTime to be
  // the same as the lastSentPktStartTime.
  uint64_t lastSentPktStartTime = 0;
  uint64_t lastPktStartTime     = 0;
};

extern UDPConnection *new_UDPConnection(int fd);
