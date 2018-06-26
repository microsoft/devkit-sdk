// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef __OTA_CLASS_H__
#define __OTA_CLASS_H__

#include "http_client.h"
#include "CheckSumUtils.h"
#include "mico.h"

class OTAClass
{
    public:
        // Singleton class:
        // This is creation point for static instance variable
        static OTAClass& getInstance()
        {
            // Single audio instance
            static OTAClass otaInstance;
            return otaInstance;
        }

        /**
        * @brief    Download new firmware from given url.
        *
        * @param    url                 The url to download firmware from.
        *           ssl_ca_pem          Certificate of given url.
        *
        * @return   Return 0 on success, otherwise return -1.
        */
        int OTAFromUrl(char *url, const char* ssl_ca_pem = NULL);

    private:
        OTAClass();
};


#endif
