# MO5 slideshow

Edit [SLIDE.INI](assets/SLIDE.INI) to choose the images and their order.
Each entry is an 8.3 `.M5Z` filename followed by `=` and a display time in
seconds, from 0 to 255:

```ini
LENA.M5Z=4
BIRDS.M5Z=8
```

Images in the mo5z format can be generated using tools available at https://github.com/fstark/png2mo5
and are very high quality given the Thomson MO5 graphic constrains.

Blank lines and lines beginning with `#` are ignored. Names may contain
letters, digits, underscores or hyphens; the index may contain up to 1024
bytes. Filenames are case-insensitive, and image assets in `assets/` must use
uppercase names.

Run `bash build.sh` to copy the index and its referenced images into the disk
image. On the MO5, the index is reloaded from disk after each slideshow cycle.
An unreadable, empty or invalid index is reported on screen and retried.

<img width="372" height="288" alt="image" src="https://github.com/user-attachments/assets/831fa9ed-b2f1-47ea-95df-68e0f07fc180" />

<img width="372" height="288" alt="image" src="https://github.com/user-attachments/assets/64010d84-f5a7-4808-9a47-d70e3cbddc03" />

≈<img width="372" height="288" alt="image" src="https://github.com/user-attachments/assets/5ff1fb26-fbe8-44f3-9efa-ca028baf4fdf" />
<img width="372" height="288" alt="image" src="https://github.com/user-attachments/assets/66f37368-f7ea-4382-9dfd-26c79536d20a" />

