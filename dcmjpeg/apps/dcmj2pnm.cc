/*
 *
 *  Copyright (C) 2024, OFFIS e.V.
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  This software and supporting documentation were developed by
 *
 *    OFFIS e.V.
 *    R&D Division Health
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *
 *  Module:  dcmjpeg
 *
 *  Authors: Marco Eichelberg
 *
 *  Purpose: Proxy stub that calls dcm2img
 *
 */

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/ofstd/ofstub.h"

#include "monolithic_examples.h"

#if defined(BUILD_MONOLITHIC)
#define main dcmtk_dcmj2pnm_main
#endif

extern "C"
int main(int argc, const char **argv)
{
    return OFstub_main(argc, argv, "dcmj2pnm", "dcm2img");
}
