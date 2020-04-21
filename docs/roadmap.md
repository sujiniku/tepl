Tepl roadmap
============

This page contains the plans for major code changes we hope to get done in the
future.

See the [NEWS file](../NEWS) for a detailed history.

Side/bottom panel container
---------------------------

- Status: **done**
- Release: Tepl 5 (GNOME 3.38)

See the TeplPanel class.

Rework file metadata
--------------------

- Status: **mostly done**
- Target release: Tepl 5 (GNOME 3.38)

Tasks:
- Make the metadata API independent of TeplFile, to better isolate toolkit
  features (separation of concerns).
- No longer use GVfs metadata, it complicates everything. With GVfs metadata
  there is the need to have async APIs, and the need to anyway have another
  backend for platforms that don't support GVfs metadata.
  See commit 2f21d526271a433466e4e546af9a358f80ee1f94 , the implementation and
  API was too complicated.
- Keep only the TeplMetadataManager for storing metadata on disk, and
  re-implement it to no longer depend on the libxml2.

Rework file loading and saving toolkit
--------------------------------------

- Status: **todo**

Tasks:
- Use the [libicu](http://site.icu-project.org/) for character encoding
  _auto-detection_, not
  [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/).
- Use the libicu for character encoding _conversion_, not iconv.
- Feature parity with the GtkSourceView file loading and saving API.
- Replace the few functions that do sync I/O with async I/O.

High-level file loading and saving API: separate it from the core framework
---------------------------------------------------------------------------

- Status: **todo**

The purpose is to have a more minimal core framework, and have the file loading
and saving high-level API in a separate sub-namespace: TeplFls for instance.

Continue high-level file loading and saving implementation
----------------------------------------------------------

- Status: **todo**

All the errors would be handled by Tepl, showing TeplInfoBars etc.

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
