Tepl roadmap
============

This page contains the plans for major code changes we hope to get done in the
future.

See the [NEWS file](../NEWS) for a detailed history.

New small features in Tepl 5.0 (GNOME 3.38)
-------------------------------------------

- Status: **done**
- Goto line UI: see the TeplGotoLineBar class and "win.tepl-goto-line" GAction.
- TeplPanel: side/bottom panel container.
- TeplStatusbar.
- TeplStyleSchemeChooserWidget.
- Plus new functions here and there.

Build system: Autotools -> Meson
--------------------------------

- Status: **done**
- Release: Tepl 5.0 (GNOME 3.38)

A complete overhaul of file metadata
------------------------------------

- Status: **done**
- Target release: Tepl 5.0 (GNOME 3.38)

Tasks:
- Make the metadata API independent of TeplFile, to better isolate toolkit
  features (separation of concerns).
- Apply the “worse is better” philosophy: no longer use GVfs metadata, it
  complicates everything. With GVfs metadata there is the need to have async
  APIs, and the need to anyway have another backend for platforms that don't
  support GVfs metadata. See commit 2f21d526271a433466e4e546af9a358f80ee1f94 ,
  the implementation and API was too complicated.
- Keep only the TeplMetadataManager for storing metadata on disk, and
  re-implement it to no longer depend on the libxml2.

A complete overhaul of the file loading and saving
--------------------------------------------------

- Status: **in progress**

Tasks:
- New and hopefully simpler implementation, the API is still mostly the same.
- Implement both the backend and the frontend, incrementally. For the frontend
  (the high-level API), handle all errors by showing TeplInfoBar's etc.
- Use the [libicu](http://site.icu-project.org/) for character encoding
  _conversion_, not iconv.
- Use the libicu for character encoding _auto-detection_, not
  [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/).
- Replace the few functions in TeplFile that do sync I/O with async I/O.
- Compared to the GtkSourceView file loading and saving:
	- Drop gzip compression support and progress callbacks.
	- Add more important features: preventing loading files that are not
	  well supported by GtkTextView (very big file sizes, or very long
	  lines). Ideally proposing solutions to load the files, for example to
	  split very long lines (and then showing a warning when saving the
	  file).

File browser widget
-------------------

- Status: **todo**

To be integrated in a side panel.

Search and replace UI
---------------------

- Status: **todo**

File printing UI
----------------

- Status: **todo**

High-level spell-checking API
-----------------------------

- Status: **todo**

Integrate [gspell](https://wiki.gnome.org/Projects/gspell) with the Tepl
framework.
