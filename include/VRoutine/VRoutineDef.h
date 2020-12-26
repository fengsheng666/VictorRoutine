// Copyright (c) 2020 FengSheng(EN. Victor Fung)
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//    http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef VROUTINEDEF_H_VROUTINE
#define VROUTINEDEF_H_VROUTINE

#include "../VictorDef/VictorDef.h"
#include <stdio.h>
#include <stdlib.h>

///宏定义
#ifdef VROUTINE_LIB
#define VROUTINE_API VICTOR_EXPORT
#else
#define VROUTINE_API VICTOR_IMPORT
#endif

static inline void vroutine_check(bool stat, const char* err, const char* filename, int line)
{
	if (!stat)
	{
		printf("!(%s) : in %s(%d)\n", err, filename, line);
		abort();
	}
}

#define VROUTINE_CHECKER(_Expression)	vroutine_check((_Expression),#_Expression,__FILE__,__LINE__)

#define VROUTINE_TASK_POOL_CACHE_COUNT	(0)

#define VROUTINE_ACTIVE_TASK_MAX_COUNT	(0)

#define VROUTINE_TASK_QUEUE_MAX_LENGTH	(0)

#endif