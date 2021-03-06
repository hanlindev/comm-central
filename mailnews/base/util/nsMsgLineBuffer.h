/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef _nsMsgLineBuffer_H
#define _nsMsgLineBuffer_H

#include "msgCore.h"    // precompiled header...

// I can't believe I have to have this stupid class, but I can't find
// anything suitable (nsStrImpl might be, when it's done). nsIByteBuffer
// would do, if I had a stream for input, which I don't.

class NS_MSG_BASE nsByteArray 
{
public:
  nsByteArray();
  virtual ~nsByteArray();
  PRUint32  GetSize() {return m_bufferSize;}
  PRUint32  GetBufferPos() {return m_bufferPos;}
  nsresult  GrowBuffer(PRUint32 desired_size, PRUint32 quantum = 1024);
  nsresult  AppendString(const char *string);
  nsresult  AppendBuffer(const char *buffer, PRUint32 length);
  void    ResetWritePos() {m_bufferPos = 0;}
  char    *GetBuffer() {return m_buffer;}
protected:
  char    *m_buffer;
  PRUint32  m_bufferSize;
  PRUint32  m_bufferPos;  // write Pos in m_buffer - where the next byte should go.
};


class NS_MSG_BASE nsMsgLineBufferHandler : public nsByteArray
{
public:
  virtual PRInt32 HandleLine(char *line, PRUint32 line_length) = 0;
};

class NS_MSG_BASE nsMsgLineBuffer : public nsMsgLineBufferHandler
{
public:
  nsMsgLineBuffer(nsMsgLineBufferHandler *handler, bool convertNewlinesP);
  
  virtual    ~nsMsgLineBuffer();
  PRInt32     BufferInput(const char *net_buffer, PRInt32 net_buffer_size);
  // Not sure why anyone cares, by NNTPHost seems to want to know the buf pos.
  PRUint32    GetBufferPos() {return m_bufferPos;}
  
  virtual PRInt32 HandleLine(char *line, PRUint32 line_length);
  // flush last line, though it won't be CRLF terminated.
  virtual PRInt32 FlushLastLine();
protected:
  nsMsgLineBuffer(bool convertNewlinesP);
  
  PRInt32 ConvertAndSendBuffer();
  void SetLookingForCRLF(bool b);
  
  nsMsgLineBufferHandler *m_handler;
  bool        m_convertNewlinesP;
  bool        m_lookingForCRLF; 
};

// I'm adding this utility class here for lack of a better place. This utility class is similar to nsMsgLineBuffer
// except it works from an input stream. It is geared towards efficiently parsing new lines out of a stream by storing
// read but unprocessed bytes in a buffer. I envision the primary use of this to be our mail protocols such as imap, news and
// pop which need to process line by line data being returned in the form of a proxied stream from the server.

class nsIInputStream;

class NS_MSG_BASE nsMsgLineStreamBuffer
{
public:
  // aBufferSize -- size of the buffer you want us to use for buffering stream data
  // aEndOfLinetoken -- The delimiter string to be used for determining the end of line. This 
  //              allows us to parse platform specific end of line endings by making it
  //            a parameter.
  // aAllocateNewLines -- true if you want calls to ReadNextLine to allocate new memory for the line. 
  //            if false, the char * returned is just a ptr into the buffer. Subsequent calls to
  //            ReadNextLine will alter the data so your ptr only has a life time of a per call.
  // aEatCRLFs  -- true if you don't want to see the CRLFs on the lines returned by ReadNextLine. 
  //         false if you do want to see them.
  // aLineToken -- Specify the line token to look for, by default is LF ('\n') which cover as well CRLF. If
  //            lines are terminated with a CR only, you need to set aLineToken to CR ('\r')
  nsMsgLineStreamBuffer(PRUint32 aBufferSize, bool aAllocateNewLines, 
                        bool aEatCRLFs = true, char aLineToken = '\n'); // specify the size of the buffer you want the class to use....
  virtual ~nsMsgLineStreamBuffer();
  
  // Caller must free the line returned using PR_Free
  // aEndOfLinetoken -- delimiter used to denote the end of a line.
  // aNumBytesInLine -- The number of bytes in the line returned
  // aPauseForMoreData -- There is not enough data in the stream to make a line at this time...
  char * ReadNextLine(nsIInputStream * aInputStream, PRUint32 &anumBytesInLine, bool &aPauseForMoreData, nsresult *rv = nsnull, bool addLineTerminator = false);
  nsresult GrowBuffer(PRInt32 desiredSize);
  void ClearBuffer();
  bool NextLineAvailable();
protected:
  bool m_eatCRLFs;
  bool m_allocateNewLines;
  char * m_dataBuffer;
  PRUint32 m_dataBufferSize;
  PRUint32 m_startPos;
  PRUint32 m_numBytesInBuffer;
  char m_lineToken;
};


#endif
