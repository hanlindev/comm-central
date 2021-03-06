/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsImportFieldMap_h___
#define nsImportFieldMap_h___

#include "nscore.h"
#include "nsIImportFieldMap.h"
#include "nsIAddrDatabase.h"
#include "nsVoidArray.h"


////////////////////////////////////////////////////////////////////////

class nsIStringBundle;

class nsImportFieldMap : public nsIImportFieldMap
{
public:
  NS_DECL_ISUPPORTS

  NS_DECL_NSIIMPORTFIELDMAP

  nsImportFieldMap(nsIStringBundle *aBundle);
  virtual ~nsImportFieldMap();

   static NS_METHOD Create(nsIStringBundle *aBundle, nsISupports *aOuter, REFNSIID aIID, void **aResult);

private:
  nsresult  Allocate(PRInt32 newSize);
  PRInt32    FindFieldNum(const PRUnichar *pDesc);


private:
  PRInt32    m_numFields;
  PRInt32  *  m_pFields;
  bool *  m_pActive;
  PRInt32    m_allocated;
  nsVoidArray  m_descriptions;
  PRInt32    m_mozFieldCount;
  bool        m_skipFirstRecord;
};


#endif
