/*
    libzint - the open source barcode library
    Copyright (C) 2020 Robin Stuart <rstuart114@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
 */
/* vim: set ts=4 sw=4 et : */

#include "testcommon.h"
#include <sys/stat.h>

//#define TEST_PRINT_GENERATE_EXPECTED    1
//#define TEST_PRINT_OVERWRITE_EXPECTED   "bmp,emf,eps,gif,pcx,png,svg,tif,txt"
//#define TEST_PRINT_OVERWRITE_EXPECTED   "gif"

static void test_print(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        int option_1;
        int option_2;
        unsigned char* data;
        char* expected_file;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, "AIM", "code128_aim" },
        /*  1*/ { BARCODE_QRCODE, 2, 1, "1234567890", "qr_v1_m" },
        /*  2*/ { BARCODE_DOTCODE, -1, -1, "2741", "dotcode_aim_fig7" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char* exts[] = { "bmp", "emf", "eps", "gif", "pcx", "png", "svg", "tif", "txt" };
    int exts_len = sizeof(exts) / sizeof(char*);

    char data_dir[1024];
    char expected_file[1024];

    char escaped[1024];
    int escaped_size = 1024;

    #ifdef TEST_PRINT_GENERATE_EXPECTED
    strcpy(data_dir, "../data");
    if (!testUtilExists(data_dir)) {
        ret = mkdir(data_dir, 0755);
        assert_zero(ret, "mkdir(%s) ret %d != 0\n", data_dir, ret);
    }
    strcat(data_dir, "/print");
    if (!testUtilExists(data_dir)) {
        ret = mkdir(data_dir, 0755);
        assert_zero(ret, "mkdir(%s) ret %d != 0\n", data_dir, ret);
    }
    #endif

    for (int j = 0; j < exts_len; j++) {
        strcpy(data_dir, "../data/print/");
        strcat(data_dir, exts[j]);

        #ifdef TEST_PRINT_GENERATE_EXPECTED
        if (!testUtilExists(data_dir)) {
            ret = mkdir(data_dir, 0755);
            assert_zero(ret, "mkdir(%s) ret %d != 0\n", data_dir, ret);
        }
        #endif

        for (int i = 0; i < data_size; i++) {

            struct zint_symbol* symbol = ZBarcode_Create();
            assert_nonnull(symbol, "Symbol not created\n");

            symbol->symbology = data[i].symbology;
            if (data[i].option_1 != -1) {
                symbol->option_1 = data[i].option_1;
            }
            if (data[i].option_2 != -1) {
                symbol->option_2 = data[i].option_2;
            }

            int length = strlen(data[i].data);

            ret = ZBarcode_Encode(symbol, data[i].data, length);
            assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

            strcpy(symbol->outfile, "out.");
            strcat(symbol->outfile, exts[j]);

            strcpy(expected_file, data_dir);
            strcat(expected_file, "/");
            strcat(expected_file, data[i].expected_file);
            strcat(expected_file, ".");
            strcat(expected_file, exts[j]);

            ret = ZBarcode_Print(symbol, 0);
            assert_zero(ret, "i:%d j:%d %s %s ZBarcode_Print %s ret %d != 0\n", i, j, exts[j], testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

            #ifdef TEST_PRINT_GENERATE_EXPECTED

            if (j == 0) {
                printf("        /*%3d*/ { %s, %d, %d, \"%s\", \"%s\" },\n",
                        i, testUtilBarcodeName(data[i].symbology), data[i].option_1, data[i].option_2, testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].expected_file);
            }
            if (strstr(TEST_PRINT_OVERWRITE_EXPECTED, exts[j])) {
                ret = rename(symbol->outfile, expected_file);
                assert_zero(ret, "i:%d rename(%s, %s) ret %d != 0\n", i, symbol->outfile, expected_file, ret);
            }

            #else

            assert_nonzero(testUtilExists(symbol->outfile), "i:%d j:%d %s testUtilExists(%s) == 0\n", i, j, exts[j], symbol->outfile);

            if (strcmp(exts[j], "eps") == 0) {
                ret = testUtilCmpEpss(symbol->outfile, expected_file);
                assert_zero(ret, "i:%d %s testUtilCmpEpss(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
            } else if (strcmp(exts[j], "png") == 0) {
                ret = testUtilCmpPngs(symbol->outfile, expected_file);
                assert_zero(ret, "i:%d %s testUtilCmpPngs(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
            } else if (strcmp(exts[j], "svg") == 0) {
                ret = testUtilCmpSvgs(symbol->outfile, expected_file);
                assert_zero(ret, "i:%d %s testUtilCmpSvgs(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
            } else if (strcmp(exts[j], "txt") == 0) {
                ret = testUtilCmpTxts(symbol->outfile, expected_file);
                assert_zero(ret, "i:%d %s testUtilCmpTxts(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
            } else {
                ret = testUtilCmpBins(symbol->outfile, expected_file);
                assert_zero(ret, "i:%d %s testUtilCmpBins(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
            }

            assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

            #endif

            ZBarcode_Delete(symbol);
        }
    }

    testFinish();
}

int main()
{
    test_print();

    testReport();

    return 0;
}
