// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

/// Apply \a patch to \a old.  Will return the new file in \a _new which is allocated for you.
bool ApplyPatch( char *old, unsigned int oldsize, char **_new, unsigned int *newsize, char *patch, unsigned int patchsize );

