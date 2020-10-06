Tepl roadmap
============

This page contains the plans for major code changes we hope to get done in the
future.

See the [roadmap-done.md](roadmap-done.md) file for done tasks.

See the [NEWS file](../NEWS) for a detailed history.

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
