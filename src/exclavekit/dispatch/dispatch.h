/*
 * Copyright (c) 2022 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __DISPATCH_PUBLIC__
#define __DISPATCH_PUBLIC__

#include <Availability.h>
#include <TargetConditionals.h>

#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#define DISPATCH_API_VERSION 20181008

#ifndef __DISPATCH_BUILDING_DISPATCH__
#ifndef __DISPATCH_INDIRECT__
#define __DISPATCH_INDIRECT__
#endif

#include <dispatch/base.h>
#include <dispatch/object.h>
#include <dispatch/once.h>

#undef __DISPATCH_INDIRECT__
#endif /* !__DISPATCH_BUILDING_DISPATCH__ */

#endif
