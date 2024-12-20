/*
 *
 *  Copyright (C) 2012-2021, OFFIS e.V.
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
 *  Module:  dcmrt
 *
 *  Author:  Uli Schlachter
 *
 *  Purpose: main test program
 *
 */

#include "dcmtk/config/osconfig.h"

#include "dcmtk/ofstd/oftest.h"

#include "monolithic_examples.h"

OFTEST_REGISTER(dcmrt_search);
OFTEST_REGISTER(dcmrt_contour);

#if defined(BUILD_MONOLITHIC)
#define main dcmtk_dcmrt_tests_main
#endif

OFTEST_MAIN("dcmrt")
