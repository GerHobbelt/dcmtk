/*
 *
 *  Copyright (C) 2015-2024, OFFIS e.V.
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
 *  Module:  dcmseg
 *
 *  Author:  Michael Onken
 *
 *  Purpose: main test program
 *
 */

#include "dcmtk/config/osconfig.h"

#include "dcmtk/ofstd/oftest.h"

#include "monolithic_examples.h"

OFTEST_REGISTER(dcmseg_bigdim);
OFTEST_REGISTER(dcmseg_debugByte2Bin);
OFTEST_REGISTER(dcmseg_packBinaryFrame);
OFTEST_REGISTER(dcmseg_packAndUnpackBinaryFrame);
OFTEST_REGISTER(dcmseg_concatBinaryFrames);
OFTEST_REGISTER(dcmseg_roundtrip);
OFTEST_REGISTER(dcmseg_concat_binary);

#if defined(BUILD_MONOLITHIC)
#define main dcmtk_dcmseg_tests_main
#endif

OFTEST_MAIN("dcmseg")
