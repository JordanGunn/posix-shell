#ifndef TEMPLATE_COMMON_H
#define TEMPLATE_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dc_posix/dc_string.h>

#define SM_BUFF 256
#define MD_BUFF 512
#define LG_BUFF 1024
#define XL_BUFF 2048

/*
 * This file is part of dc_dump.
 *
 *  dc_dump is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Foobar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with dc_dump.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * A function to be documented.
 *
 * @param str a parameter to be documented.
 * @return a return value to be documented.
 */
int display(const char *str);

#endif // TEMPLATE_COMMON_H
