# UltraScan GIMP logo scripts

The files in this directory support two GIMP generations:

- `ultrascan.scm` is the original GIMP 2 Script-Fu implementation.
- `ultrascan_gimpv3.scm` is the GIMP 3 port. It leaves the original unchanged.
- `ultrascan72.ggr` is the custom gradient required by both scripts.

## Install in GIMP 3

1. In **GIMP > Settings > Folders > Scripts**, note or add a writable scripts
   folder, then copy `ultrascan_gimpv3.scm` into it.
2. In **GIMP > Settings > Folders > Gradients**, note or add a writable
   gradients folder, then copy `ultrascan72.ggr` into it.
3. Restart GIMP. (Refreshing gradients alone does not reload Script-Fu.)
4. Create a logo with **File > Create > Logos > UltraScan 3 Logo**.

To apply the effect to an existing RGBA layer, use
**Filters > Decor > UltraScan 3**.

The default logo settings reproduce `gimp.inf`: Times New Roman Bold Italic at
60 pixels, the `ultrascan72` gradient on the text and two-pixel outline, and a
12-pixel drop-shadow offset.
