// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef UTILITY_H
#define UTILITY_H

void parseTwinMessage(DEVICE_TWIN_UPDATE_STATE, const char *);

void enterIdleState();
void enterActiveState();
void enterRecordingState();
void enterServerProcessingState();
void enterReceivingState();
void enterPlayingState();


#endif /* UTILITY_H */