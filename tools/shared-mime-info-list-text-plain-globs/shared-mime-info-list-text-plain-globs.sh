#!/bin/sh
# SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
# SPDX-License-Identifier: LGPL-3.0-or-later

# To have a list of all globs for text/plain files, taken from the
# shared-mime-info database. The list is useful for Microsoft Windows packaging
# purposes, to package a general-purpose text editor.

shared_mime_info_xml_file='/usr/share/mime/packages/freedesktop.org.xml'
tmp_dir='/tmp/tepl-shared-mime-info-tool/'

# Have only the shared-mime-info database, not additional mime types from other
# packages.
rm -rf "$tmp_dir"
mkdir -p "$tmp_dir/packages/"
cp "$shared_mime_info_xml_file" "$tmp_dir/packages/"
update-mime-database "$tmp_dir" 2>/dev/null

sed '/^#/d' "${tmp_dir}/globs2" | cut -d':' -f'2,3'  > "${tmp_dir}/globs2-simplified"

for line in `cat "${tmp_dir}/globs2-simplified"`
do
	mime_type=`echo "$line" | cut -d':' -f1`
	glob=`echo "$line" | cut -d':' -f2`

	if `./mime-type-is-text-plain "$mime_type"`
	then
		echo "$glob"
	fi
done | sort | uniq > "${tmp_dir}/text-plain-globs"

comm -2 -3 "${tmp_dir}/text-plain-globs" globs-to-ignore > "${tmp_dir}/text-plain-globs-filtered"
cat "${tmp_dir}/text-plain-globs-filtered" globs-to-add | sort | uniq > "${tmp_dir}/text-plain-globs-final"
cat "${tmp_dir}/text-plain-globs-final"
