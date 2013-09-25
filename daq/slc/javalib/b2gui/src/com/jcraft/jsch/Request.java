/* -*-mode:java; c-basic-offset:2; indent-tabs-mode:nil -*- */
/*
Copyright (c) 2002-2010 ymnk, JCraft,Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright 
     notice, this list of conditions and the following disclaimer in 
     the documentation and/or other materials provided with the distribution.

  3. The names of the authors may not be used to endorse or promote products
     derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL JCRAFT,
INC. OR ANY CONTRIBUTORS TO THIS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

package com.jcraft.jsch;

abstract class Request{
  private boolean reply=false;
  private Session session=null;
  private Channel channel=null;
  void request(Session session, Channel channel) throws Exception{
    this.session=session;
    this.channel=channel;
    if(channel.connectTimeout>0){
      setReply(true);
    }
  }
  boolean waitForReply(){ return reply; }
  void setReply(boolean reply){ this.reply=reply; }
  void write(Packet packet) throws Exception{
    if(reply){
      channel.reply=-1;
    }
    session.write(packet);
    if(reply){
      long start=System.currentTimeMillis();
      long timeout=channel.connectTimeout;
      while(channel.isConnected() && channel.reply==-1){
	try{Thread.sleep(10);}
	catch(Exception ee){
	}
        if(timeout>0L &&
           (System.currentTimeMillis()-start)>timeout){
          channel.reply=0;
          throw new JSchException("channel request: timeout");
        }
      }

      if(channel.reply==0){
	throw new JSchException("failed to send channel request");
      }
    }
  }
}
