/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsPrintDialogQt_h__
#define nsPrintDialogQt_h__

#include "nsIPrintDialogService.h"

class nsIPrintSettings;

class nsPrintDialogServiceQt final : public nsIPrintDialogService
{
public:
    nsPrintDialogServiceQt();

    NS_DECL_ISUPPORTS
    NS_DECL_NSIPRINTDIALOGSERVICE

protected:
    virtual ~nsPrintDialogServiceQt();
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsPrintDialogServiceQt,
                              NS_IPRINTDIALOGSERVICE_IID)

#endif
