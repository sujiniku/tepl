Tepl roadmap
============

This page contains the plans for major code changes we hope to get done in the
future.

See the [NEWS file](../NEWS) for a detailed history.

Rework file metadata
--------------------

- Status: **in progress**
- Target release: Tepl 5 (GNOME 3.38)

Tasks:
- Make TeplFileMetadata independent of TeplFile, to better isolate toolkit
  features.
- Replace the metadata manager with TeplMetadataStore, to get rid of the libxml2
  dependency (at least in Tepl, the libxml2 is still used by GtkSourceView).
- If possible, limit synchronous I/O to application shutdown only, do
  asynchronous I/O the rest of the time (during application startup and normal
  execution).
- Write more unit tests.

Rework file loading and saving toolkit
--------------------------------------

- Status: **todo**

Tasks:
- Use the [libicu](http://site.icu-project.org/) for character encoding
  _auto-detection_, not
  [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/).
- Use the libicu for character encoding _conversion_, not iconv.
- Feature parity with the GtkSourceView file loading and saving API.

High-level file loading and saving API: separate it from the core framework
---------------------------------------------------------------------------

- Status: **todo**

The purpose is to have a more minimal core framework, and have the file loading
and saving high-level API in a separate sub-namespace: TeplFls for instance.

Continue high-level file loading and saving implementation
----------------------------------------------------------

- Status: **todo**

All the errors would be handled by Tepl, showing TeplInfoBars etc.

Side/bottom panel container
---------------------------

- Status: **todo**

File browser widget
-------------------

- Status: **todo**

To be integrated in a side panel.

Goto line UI
------------

- Status: **todo**

Search and replace UI
---------------------

- Status: **todo**

The goto line is similar and much easier to do than the search and replace, so
goto line can be done as a first step.

File printing UI
----------------

- Status: **todo**

High-level spell-checking API
-----------------------------

- Status: **todo**

Integrating [gspell](https://wiki.gnome.org/Projects/gspell) with the Tepl
framework.
