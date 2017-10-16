/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "types.h"
#ifdef ARM9
	#include "arm9/hardware/interrupt.h"
#elif ARM11
	#include "arm11/fmt.h"
	#include "arm11/hardware/interrupt.h"
#endif



noreturn void __fb_assert(const char *const str, u32 line)
{
#ifdef ARM9
	// Get rid of the warnings.
	(void)str;
	(void)line;
#elif ARM11
	ee_printf("Assertion failed: %s:%" PRIu32, str, line);
#endif

	enterCriticalSection();
	while(1) __wfi();
}
