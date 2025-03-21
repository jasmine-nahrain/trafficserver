/** @file

  Record utils definitions

  @section license License

  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

#include "tscore/ink_platform.h"
#include "tscore/ink_memory.h"
#include "tscore/ParseRules.h"
#include "tscore/Tokenizer.h"
#include "tsutil/Regex.h"
#include "records/RecordsConfig.h"
#include "P_RecUtils.h"
#include "P_RecCore.h"
//-------------------------------------------------------------------------
// RecRecord initializer / Free
//-------------------------------------------------------------------------
void
RecRecordInit(RecRecord *r)
{
  ink_zero(*r);
  rec_mutex_init(&(r->lock), nullptr);
}

void
RecRecordFree(RecRecord *r)
{
  rec_mutex_destroy(&(r->lock));
}

//-------------------------------------------------------------------------
// RecAlloc
//-------------------------------------------------------------------------
RecRecord *
RecAlloc(RecT rec_type, const char *name, RecDataT data_type)
{
  if (g_num_records >= max_records_entries) {
    Fatal("Too many config records already registered (%d). Hint: increase --maxRecords param.", max_records_entries);
    return nullptr;
  }

  int        i = g_num_records++;
  RecRecord *r = &(g_records[i]);

  RecRecordInit(r);
  r->rec_type  = rec_type;
  r->name      = ats_strdup(name);
  r->order     = i;
  r->data_type = data_type;

  return r;
}

//-------------------------------------------------------------------------
// RecDataZero
//-------------------------------------------------------------------------
void
RecDataZero(RecDataT data_type, RecData *data)
{
  if ((data_type == RECD_STRING) && (data->rec_string)) {
    ats_free(data->rec_string);
  }
  memset(data, 0, sizeof(RecData));
}

void
RecDataSetMax(RecDataT type, RecData *data)
{
  switch (type) {
  case RECD_INT:
  case RECD_COUNTER:
    data->rec_int = INT64_MAX; // Assumes rec_int is int64_t, which it currently is
    break;
  case RECD_FLOAT:
    data->rec_float = FLT_MAX;
    break;
  default:
    Fatal("unsupported type:%d\n", type);
  }
}

void
RecDataSetMin(RecDataT type, RecData *data)
{
  switch (type) {
  case RECD_INT:
  case RECD_COUNTER:
    data->rec_int = INT64_MIN; // Assumes rec_int is int64_t, which it currently is
    break;
  case RECD_FLOAT:
    data->rec_float = FLT_MIN;
    break;
  default:
    Fatal("unsupported type:%d\n", type);
  }
}

//-------------------------------------------------------------------------
// RecDataSet
//-------------------------------------------------------------------------
bool
RecDataSet(RecDataT data_type, RecData *data_dst, RecData *data_src)
{
  bool rec_set = false;

  switch (data_type) {
  case RECD_STRING:
    if (data_src->rec_string == nullptr) {
      if (data_dst->rec_string != nullptr) {
        ats_free(data_dst->rec_string);
        data_dst->rec_string = nullptr;
        rec_set              = true;
      }
    } else if (((data_dst->rec_string) && (strcmp(data_dst->rec_string, data_src->rec_string) != 0)) ||
               ((data_dst->rec_string == nullptr) && (data_src->rec_string != nullptr))) {
      ats_free(data_dst->rec_string);

      data_dst->rec_string = ats_strdup(data_src->rec_string);
      rec_set              = true;
      // Chop trailing spaces
      char *end = data_dst->rec_string + strlen(data_dst->rec_string) - 1;

      while (end >= data_dst->rec_string && isspace(*end)) {
        end--;
      }
      *(end + 1) = '\0';
    }
    break;
  case RECD_INT:
    if (data_dst->rec_int != data_src->rec_int) {
      data_dst->rec_int = data_src->rec_int;
      rec_set           = true;
    }
    break;
  case RECD_FLOAT:
    if (data_dst->rec_float != data_src->rec_float) {
      data_dst->rec_float = data_src->rec_float;
      rec_set             = true;
    }
    break;
  case RECD_COUNTER:
    if (data_dst->rec_counter != data_src->rec_counter) {
      data_dst->rec_counter = data_src->rec_counter;
      rec_set               = true;
    }
    break;
  default:
    ink_assert(!"Wrong RECD type!");
  }
  return rec_set;
}

int
RecDataCmp(RecDataT type, RecData left, RecData right)
{
  switch (type) {
  case RECD_INT:
  case RECD_COUNTER:
    if (left.rec_int > right.rec_int) {
      return 1;
    } else if (left.rec_int == right.rec_int) {
      return 0;
    } else {
      return -1;
    }
  case RECD_FLOAT:
    if (left.rec_float > right.rec_float) {
      return 1;
    } else if (left.rec_float == right.rec_float) {
      return 0;
    } else {
      return -1;
    }
  default:
    Fatal("unsupported type:%d\n", type);
    return 0;
  }
}

RecData
RecDataAdd(RecDataT type, RecData left, RecData right)
{
  RecData val;
  memset(&val, 0, sizeof(val));

  switch (type) {
  case RECD_INT:
  case RECD_COUNTER:
    val.rec_int = left.rec_int + right.rec_int;
    break;
  case RECD_FLOAT:
    val.rec_float = left.rec_float + right.rec_float;
    break;
  default:
    Fatal("unsupported type:%d\n", type);
    break;
  }
  return val;
}

RecData
RecDataSub(RecDataT type, RecData left, RecData right)
{
  RecData val;
  memset(&val, 0, sizeof(val));

  switch (type) {
  case RECD_INT:
  case RECD_COUNTER:
    val.rec_int = left.rec_int - right.rec_int;
    break;
  case RECD_FLOAT:
    val.rec_float = left.rec_float - right.rec_float;
    break;
  default:
    Fatal("unsupported type:%d\n", type);
    break;
  }
  return val;
}

RecData
RecDataMul(RecDataT type, RecData left, RecData right)
{
  RecData val;
  memset(&val, 0, sizeof(val));

  switch (type) {
  case RECD_INT:
  case RECD_COUNTER:
    val.rec_int = left.rec_int * right.rec_int;
    break;
  case RECD_FLOAT:
    val.rec_float = left.rec_float * right.rec_float;
    break;
  default:
    Fatal("unsupported type:%d\n", type);
    break;
  }
  return val;
}

RecData
RecDataDiv(RecDataT type, RecData left, RecData right)
{
  RecData val;
  memset(&val, 0, sizeof(val));

  switch (type) {
  case RECD_INT:
  case RECD_COUNTER:
    val.rec_int = left.rec_int / right.rec_int;
    break;
  case RECD_FLOAT:
    val.rec_float = left.rec_float / right.rec_float;
    break;
  default:
    Fatal("unsupported type:%d\n", type);
    break;
  }
  return val;
}

//-------------------------------------------------------------------------
// RecDataSetFromInt64
//-------------------------------------------------------------------------
bool
RecDataSetFromInt64(RecDataT data_type, RecData *data_dst, int64_t data_int64)
{
  RecData data_src;

  switch (data_type) {
  case RECD_INT:
    data_src.rec_int = data_int64;
    break;
  case RECD_FLOAT:
    data_src.rec_float = static_cast<float>(data_int64);
    break;
  case RECD_STRING: {
    char buf[32 + 1];
    snprintf(buf, 32, "%" PRId64 "", data_int64);
    data_src.rec_string = ats_strdup(buf);
    break;
  }
  case RECD_COUNTER:
    data_src.rec_counter = data_int64;
    break;
  default:
    ink_assert(!"Unexpected RecD type");
    return false;
  }

  return RecDataSet(data_type, data_dst, &data_src);
}

//-------------------------------------------------------------------------
// RecDataSetFromFloat
//-------------------------------------------------------------------------
bool
RecDataSetFromFloat(RecDataT data_type, RecData *data_dst, float data_float)
{
  RecData data_src;

  switch (data_type) {
  case RECD_INT:
    data_src.rec_int = static_cast<RecInt>(data_float);
    break;
  case RECD_FLOAT:
    data_src.rec_float = (data_float);
    break;
  case RECD_STRING: {
    char buf[32 + 1];
    snprintf(buf, 32, "%f", data_float);
    data_src.rec_string = ats_strdup(buf);
    break;
  }
  case RECD_COUNTER:
    data_src.rec_counter = static_cast<RecCounter>(data_float);
    break;
  default:
    ink_assert(!"Unexpected RecD type");
    return false;
  }

  return RecDataSet(data_type, data_dst, &data_src);
}

//-------------------------------------------------------------------------
// RecDataSetFromString
//-------------------------------------------------------------------------
bool
RecDataSetFromString(RecDataT data_type, RecData *data_dst, const char *data_string)
{
  RecData data_src;

  switch (data_type) {
  case RECD_INT:
    data_src.rec_int = ink_atoi64(data_string);
    break;
  case RECD_FLOAT:
    data_src.rec_float = atof(data_string);
    break;
  case RECD_STRING:
    if (data_string && (strlen(data_string) == 4) && strncasecmp((data_string), "NULL", 4) == 0) {
      data_src.rec_string = nullptr;
    } else {
      // It's OK to cast away the const here, because RecDataSet will copy the string.
      data_src.rec_string = const_cast<char *>(data_string);
    }
    break;
  case RECD_COUNTER:
    data_src.rec_counter = ink_atoi64(data_string);
    break;
  default:
    ink_assert(!"Unexpected RecD type");
    return false;
  }

  return RecDataSet(data_type, data_dst, &data_src);
}
//-------------------------------------------------------------------------
// Basic functions to help setting a record value properly. All this functionality is originally from WebMgmtUtils.
// TODO: we can work out something different.
namespace
{ // anonymous namespace
bool
recordRegexCheck(const char *pattern, const char *value)
{
  Regex regex;

  bool rval = regex.compile(pattern);
  if (rval == false) {
    return false;
  } else {
    return regex.exec(value);
  }

  return false; // no-op
}

bool
recordRangeCheck(const char *pattern, const char *value)
{
  char     *p = const_cast<char *>(pattern);
  Tokenizer dashTok("-");

  if (recordRegexCheck("^[0-9]+$", value)) {
    while (*p != '[') {
      p++;
    } // skip to '['
    if (dashTok.Initialize(++p, COPY_TOKS) == 2) {
      int l_limit = atoi(dashTok[0]);
      int u_limit = atoi(dashTok[1]);
      int val     = atoi(value);
      if (val >= l_limit && val <= u_limit) {
        return true;
      }
    }
  }
  return false;
}

bool
recordIPCheck(const char *pattern, const char *value)
{
  //  regex_t regex;
  //  int result;
  bool        check;
  const char *range_pattern = R"(\[[0-9]+\-[0-9]+\]\\\.\[[0-9]+\-[0-9]+\]\\\.\[[0-9]+\-[0-9]+\]\\\.\[[0-9]+\-[0-9]+\])";
  const char *ip_pattern    = "[0-9]*[0-9]*[0-9].[0-9]*[0-9]*[0-9].[0-9]*[0-9]*[0-9].[0-9]*[0-9]*[0-9]";

  Tokenizer dotTok1(".");
  Tokenizer dotTok2(".");

  check = true;
  if (recordRegexCheck(range_pattern, pattern) && recordRegexCheck(ip_pattern, value)) {
    if (dotTok1.Initialize(const_cast<char *>(pattern), COPY_TOKS) == 4 &&
        dotTok2.Initialize(const_cast<char *>(value), COPY_TOKS) == 4) {
      for (int i = 0; i < 4 && check; i++) {
        if (!recordRangeCheck(dotTok1[i], dotTok2[i])) {
          check = false;
        }
      }
      if (check) {
        return true;
      }
    }
  } else if (strcmp(value, "") == 0) {
    return true;
  }
  return false;
}
} // namespace

bool
RecordValidityCheck(const char *value, RecCheckT checkType, const char *pattern)
{
  switch (checkType) {
  case RECC_STR:
    if (recordRegexCheck(pattern, value)) {
      return true;
    }
    break;
  case RECC_INT:
    if (recordRangeCheck(pattern, value)) {
      return true;
    }
    break;
  case RECC_IP:
    if (recordIPCheck(pattern, value)) {
      return true;
    }
    break;
  case RECC_NULL:
    // skip checking
    return true;
  default:
    // unknown RecordCheckType...
    ;
  }

  return false;
}
