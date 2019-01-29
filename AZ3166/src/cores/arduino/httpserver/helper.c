// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "mico.h"
#include "helper.h"

#define helper_log(M, ...) custom_log("helper", M, ##__VA_ARGS__)
#define check_int(L, S, M)   \
if(S > L)                    \
{                            \
    custom_log("helper", M); \
    return kParamErr;        \
}
#define safe_search_str(S, T, O)              \
O = strstr(S, T);                             \
if(O == NULL)                                 \
{                                             \
    custom_log("helper", "Not found: %s", T); \
    return kNotFoundErr;                      \
}
#define check_file_result(E, M, ...)        \
if(E != FR_OK)                              \
{                                           \
    custom_log("helper", M, ##__VA_ARGS__); \
    return E;                               \
}
#define ensure_fs_mounted()                              \
if(!is_fs_mounted)                                       \
{                                                        \
    FRESULT err = mount_fs();                            \
    if (err != FR_OK)                                    \
    {                                                    \
      custom_log("helper", "Fail to mount filesystem."); \
      return err;                                        \
    }                                                    \
}

int httpd_get_tag_from_multipart_form(char *inbuf, char *boundary, const char *tag, char *val, unsigned val_len)
{
  check_int(strlen(boundary), 1, "boundray is empty.");
  check_int(val_len, 1, "val_len should be positive integer.");
  check_int(24, strlen(tag), "tag name should be less than 24 characters.");

  char tag_start_label[64];
  sprintf(tag_start_label, "Content-Disposition: form-data; name=\"%s\"", tag);

  char *tag_start, *tag_content_start, *tag_end;
  safe_search_str(inbuf, tag_start_label, tag_start);
  safe_search_str(tag_start, "\r\n\r\n", tag_content_start);
  safe_search_str(tag_start, boundary, tag_end);

  // exclude "\r\n\r\n" from content start
  tag_content_start += 4;
  // exclude "\r\n--" from content end
  tag_end -= 4;

  int tag_content_len = tag_end - tag_content_start;
  if (tag_content_len < 0)
  {
    helper_log("No content for tag: %s", tag);
    return kNotFoundErr;
  }
  if (tag_content_len >= (int)val_len)
  {
    helper_log("Target content length is larger than destination. Target content length: %d, destination length: %d", tag_content_len, val_len);
    return kNoSpaceErr;
  }

  if (tag_content_len > 0)
  {
    strncpy(val, tag_content_start, tag_content_len);
  }
  val[tag_content_len] = '\0';
  return kNoErr;
}
