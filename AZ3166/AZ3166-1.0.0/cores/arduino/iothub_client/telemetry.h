// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TELEMERTY_H
#define TELEMERTY_H

#ifdef _cplusplus
extern "C"
{
#endif
    void telemetry_init();
    void telemetry_enqueue(const char *iothub, const char *event, const char *message);

#ifdef _cplusplus
}
#endif

#endif