Tepl roadmap - done tasks
=========================

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
