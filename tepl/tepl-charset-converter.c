/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2019 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Tepl is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Tepl is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "tepl-charset-converter.h"
#include "tepl-iconv.h"

typedef struct _Config Config;
struct _Config
{
	/* Note that @buffer_size here is unsigned, while the parameter of
	 * _tepl_charset_converter_new() is signed to allow -1 for the default
	 * value.
	 */
	gsize buffer_size;

	guint discard_output : 1;
};

struct _TeplCharsetConverter
{
	/* Never NULL. */
	Config *config;

	/* NULL if TeplCharsetConverter is closed.
	 * Not NULL if TeplCharsetConverter is successfully opened.
	 */
	TeplIconv *iconv_converter;
};

/******************************************************************************/
/* Config mini-class */

/* 1 MiB */
#define CONFIG_BUFFER_SIZE_DEFAULT_VALUE (1024 * 1024)

/* 32 bytes are most probably enough for any character set with multi-byte
 * characters.
 */
#define CONFIG_BUFFER_SIZE_MIN_VALUE (32)

static gboolean
config_buffer_size_is_valid (gssize buffer_size)
{
	return (buffer_size == -1 ||
		buffer_size >= CONFIG_BUFFER_SIZE_MIN_VALUE);
}

static Config *
config_new (gssize   buffer_size,
	    gboolean discard_output)
{
	Config *config;

	g_assert (config_buffer_size_is_valid (buffer_size));

	config = g_new0 (Config, 1);

	if (buffer_size == -1)
	{
		config->buffer_size = CONFIG_BUFFER_SIZE_DEFAULT_VALUE;
	}
	else
	{
		config->buffer_size = buffer_size;
	}

	config->discard_output = discard_output != FALSE;

	return config;
}

static void
config_free (Config *config)
{
	g_free (config);
}

/******************************************************************************/
static gboolean
is_opened (TeplCharsetConverter *charset_converter)
{
	return charset_converter->iconv_converter != NULL;
}

/******************************************************************************/
/* Public functions */

/**
 * _tepl_charset_converter_new:
 * @buffer_size: the internal buffer size to store converted characters. Pass -1
 *   for the default value. Or pass an integer greater than
 *   CONFIG_BUFFER_SIZE_MIN_VALUE.
 * @discard_output: set to %TRUE if you are only interested to know if there are
 *   charset conversion errors or the number of invalid characters in the input.
 */
TeplCharsetConverter *
_tepl_charset_converter_new (gssize   buffer_size,
			     gboolean discard_output)
{
	TeplCharsetConverter *charset_converter;

	g_return_val_if_fail (config_buffer_size_is_valid (buffer_size), NULL);

	charset_converter = g_new0 (TeplCharsetConverter, 1);
	charset_converter->config = config_new (buffer_size, discard_output);

	return charset_converter;
}

gboolean
_tepl_charset_converter_open (TeplCharsetConverter  *charset_converter,
			      const gchar           *from_charset,
			      const gchar           *to_charset,
			      GError               **error)
{
	gboolean ok;

	g_return_val_if_fail (charset_converter != NULL, FALSE);
	g_return_val_if_fail (from_charset != NULL, FALSE);
	g_return_val_if_fail (to_charset != NULL, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (!is_opened (charset_converter), FALSE);

	charset_converter->iconv_converter = _tepl_iconv_new ();

	ok = _tepl_iconv_open (charset_converter->iconv_converter,
			       to_charset,
			       from_charset,
			       error);
	if (!ok)
	{
		_tepl_iconv_close_and_free (charset_converter->iconv_converter, NULL);
		charset_converter->iconv_converter = NULL;
		return FALSE;
	}

	return TRUE;
}

void
_tepl_charset_converter_free (TeplCharsetConverter *charset_converter)
{
	if (charset_converter == NULL)
	{
		return;
	}

	g_return_if_fail (!is_opened (charset_converter));

	config_free (charset_converter->config);
	g_free (charset_converter);
}
