# LisiblePNG - a PNG decoder

This PNG decoder is meant to be used in my other C projects.

## Supported features

This is a simple implementation of a PNG decoder, it currently implements the following features:
- Greyscale wihout alpha
- Truecolour without alpha, without palette
- 1 to 16 bit depth samples
- All PNG filtering methods

## Building

```bash 
meson setup <builddir>
meson compile -C <builddir>
```
