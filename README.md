# MO5 slideshow

Edit [SLIDE.INI](assets/SLIDE.INI) to choose the images and their order.
Each entry is an 8.3 `.M5Z` filename followed by `=` and a display time in
seconds, from 0 to 255:

```ini
LENA.M5Z=4
BIRDS.M5Z=8
```

Blank lines and lines beginning with `#` are ignored. Names may contain
letters, digits, underscores or hyphens; the index may contain up to 1024
bytes. Filenames are case-insensitive, and image assets in `assets/` must use
uppercase names.

Run `bash build.sh` to copy the index and its referenced images into the disk
image. On the MO5, the index is reloaded from disk after each slideshow cycle.
An unreadable, empty or invalid index is reported on screen and retried.

<img width="788" height="621" alt="image" src="https://github.com/user-attachments/assets/831fa9ed-b2f1-47ea-95df-68e0f07fc180" />

<img width="744" height="577" alt="image" src="https://github.com/user-attachments/assets/64010d84-f5a7-4808-9a47-d70e3cbddc03" />


