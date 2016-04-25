# Libero

Libero is a state machine code generation tool from iMatix Corporation
which can generate code in multiple languages (from a template).  It
was originally released in 1991, and updated throughout the 1990s.  The
functionality was later incorporated into other iMatix products, including
iMatix Studio.

This repository contains the last Libero source, binaries,and documentation
released in a stand alone version, retrieved from:

*  http://legacy.imatix.com/pub/libero

*  http://legacy.imatix.com/html/libero

on 2016-04-25.

The *source code* is licensed under the GPL v2, a copy of which can
be found in `src/copying`.  The license for the *documentation* (in
`website`) is unknown (and as the source to the documentation is
currently unavailable probably cannot be assumed to be the GPL);
however the documentation has been downloadable by all, without charge,
on the Internet for over 20 years at this point.

## `pub`

The `pub` directory contains the released artefacts (binaries, source
archives, etc) which are still retrievable; if older releases artefacts
are found they may be added later.  Due to their small size (well under
1MB each), and the fact that development ceased many years ago (so there
will not be growth in the archive), these are added directly into the
git repository.

## `src`

The `src` directory was created from the unpacked contents of
`pub/src/lrsrx232.tgz`, for ease of reference.

The code is licensed under the GPL v2, a copy of which can be found in
`src/copying`.

## `website`

The `website` directory contains the *rendered* documentation.  This
is approximately equivalent to the contents of `pub/doc/lrfull.zip`, but
was taken directly from http://legacy.imatix.com/html/libero.

The documentation was built (with `htmlpp`) from a marked up text file,
but at present no known copy of the documentation source file still
exists (if found, it will be added to the archive).
