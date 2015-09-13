findSymbol
==========

You can download the source code of findSymbol with:

``` sh
curl -o findSymbol.zip https://codeload.github.com/Piker-Alpha/findSymbol/zip/master
```


Unzip
-----

You can unzip the downloaded archive with:

```
unzip -qu findSymbol.zip [-d target directory]
```


Make instructions
-----------------

```
cc findSymbol.c -o findSymbol
``` 


Usage
-----

```
./findSymbol /S*/L*/Kernels/kernel _version
./findSymbol /S*/L*/Kernels/Prelinkedkernels/prelinkedkernel _version
./findSymbol /S*/L*/Kernels/kernel _version_major
./findSymbol /S*/L*/Kernels/Prelinkedkernels/prelinkedkernel _version_major
./findSymbol /S*/L*/Kernels/kernel _version_minor
./findSymbol /S*/L*/Kernels/Prelinkedkernels/prelinkedkernel _version_minor
./findSymbol /S*/L*/Kernels/kernel _version_revision
./findSymbol /S*/L*/Kernels/Prelinkedkernels/prelinkedkernel _version_revision
```

Output (example)
----------------
```
Symbol number..: 18861
Current symbol.: _version
symbol length..: 8
nl @...........: 0x9a56d0
nl->n_un.n_strx: 0x81360
nl->n_type.....: 0xf
nl->n_sect.....: 0x2
nl->n_desc.....: 0x0
nl->n_value....: 0xffffff80009537f0
Symbol _version found @ 0xa2d060offset.........: 0x7537f0
string value...: Darwin Kernel Version 15.0.0: Wed Aug 26 19:41:34 PDT 2015; root:xnu-3247.1.106~5/RELEASE_X86_64
```


Bugs
----

All possible bugs (so called 'issues') should be filed at:

https://github.com/Piker-Alpha/findSymbol/issues

Please do **not** use my blog for this. Thank you!
