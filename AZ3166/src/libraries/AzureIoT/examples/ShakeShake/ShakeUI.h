// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __SHAKE_UI_H__
#define __SHAKE_UI_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

void DrawAppTitle(char* text);

void DrawShakeAnimation(void);

void DrawCheckBox(int line, int col, int status);

void DrawTweetImage(int line, int col, int status);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SHAKE_UI_H__