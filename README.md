# fs2di

## Introduction

fs2di, or (f)ile(s)ystem "2" (d)isk (i)mage, is a tool that allows you to take
filesystem images and turn them into disk images. None of the existing tools
that I could find for this purpose worked exactly how I wanted, so I made fs2di.
The generated disk images are partitioned appropriately (although currently only
MBR is supported, but GPT support will probably eventually be added).

## Dependencies

System / software dependencies are:

* mincbuild (for build)
* A shell environment (for program execution)

## Management

* To build fs2di, run `mincbuild`
* To install fs2di after building, run `./install.sh` as root
* To uninstall fs2di after installation, run `./uninstall.sh` as root

## Usage

To turn a filesystem image called `disk-fs.img` into a bootable disk image
called `disk.img`, run:

```
$ fs2di --out=disk.img disk-fs.img boot 6
```

This assumes a sector size of 512 bytes, a partitioning scheme of MBR, and a
partition volume type of 6 (FAT16B).

For more information, read [the documentation](https://tirimid.net/software/fs2di.html).

## Contributing

Feel free to fork this repository and create your own version. If you add
support for GPT or fix a bug, I request that you open a pull request here.
