// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#include "mbed.h"


/// This Watchdog class provides the APIs to the Independent Watchdog (IWDG) feature. 
///
/// By using the Watchdog class, you can easily configure various timeout intervals
/// that meets your system needs to control system reset because of a software or
/// hardware fault.
///
/// This Watchdog class is implemented according to the hardware specification by
/// http://www.st.com/content/ccc/resource/technical/document/reference_manual/group0/4f/7b/2b/bd/04/b3/49/25/DM00180369/files/DM00180369.pdf/jcr:content/translations/en.DM00180369.pdf

class Watchdog
{
  public:
    /**
     * @constructor             initialize a Watchdog object.
     */
    Watchdog();
    
    /**
    * @brief                    enable Watchdog timer with reset interval.
    * @param   timeoutInMs      timeout interval in milliseconds, as a float.
    * @returns none
    */
    bool configure(float timeoutInMs);

    /**
     * @brief                   reset Watchdog timer to prevent it from a system reset.
     * @returns none
     */
    void resetTimer();

    /**
     * @brief                   get the flag to indicate if the Watchdog trigger a system reset.
     * @returns                 return true if the Watchdog reset is triggered, else return false.
     */
    bool resetTriggered();

  private:
    bool _causedReset;
};

#endif // __WATCHDOG_H__