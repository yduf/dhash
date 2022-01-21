Difference Hash computation

Based on idea from [article](http://www.hackerfactor.com/blog/?/archives/529-Kind-of-Like-That.html) / [2](https://benhoyt.com/writings/duplicate-image-detection/)

- try to keep aspect ratio when computing hash
- use of these 3 ratio that fit into 64bits: 8x8, 9x7, 10x6
- rotate image by 90 to always fit into these ratio.
- use libvips thumbnail for fast reduction
- image formats are those supported by [libvips](https://www.libvips.org/)
- output results a one line json for easier script integration


