# Atlantis navigation

Atlantis is a navigation-first terminal markdown editor based on Dawn's rendering and editing engine.

## Quick Open

Press `Ctrl+L` while editing to open Quick Open. It scans the document's headings and presents them as a hierarchy-aware list. Type any part of a heading to fuzzy-filter the list; `↑`/`↓` selects a result and `Enter` jumps to it. `Escape` closes the view without changing the document.

This is intentionally one surface instead of separate navigation modes: the same interaction works for short notes and long documents.

## Other movement

- Arrow keys move through visual lines while retaining the preferred column.
- `Alt`/`Ctrl` + `←`/`→` move by words.
- `Ctrl+S` searches document text and jumps to the selected match.
- `Ctrl+N` follows footnote references and definitions.
- `Escape` exits modal navigation and returns to writing.

The heading index is rebuilt when Quick Open is opened, so it never requires a separate project index or background service.
