#!/bin/sh

# EXTREME TUXRACER
#
# Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
# Copyright (C) 2010,2013 Extreme Tux Racer Team
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# Go to source directory
cd "$(dirname $0)"

if autoreconf -f -s -i -m -I m4 ; then
    echo "$(dirname $0)/configure script created, run it next."
else
    echo "Autoreconf failed" >&2
    exit 1
fi
