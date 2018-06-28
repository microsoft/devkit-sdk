// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef __OTA_UPDATE_CLIENT_H__
#define __OTA_UPDATE_CLIENT_H__

#include "http_client.h"
#include "CheckSumUtils.h"
#include "mico.h"

class OTAUpdateClient
{
    public:
        // Singleton class:
        // This is creation point for static instance variable
        static OTAUpdateClient& getInstance()
        {
            // Single client instance
            static OTAUpdateClient instance;
            return instance;
        }

        /**
        * @brief    Download new firmware from given url.
        *
        * @param    url                 The url to download firmware from.
        *           ssl_ca_pem          Certificate of given url.
        *
        * @return   Return 0 on success, otherwise return -1.
        */
        int updateFromUrl(const char *url, const char* ssl_ca_pem = NULL);

    private:
        OTAUpdateClient();
};


#endif
