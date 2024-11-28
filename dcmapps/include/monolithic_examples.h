
#pragma once

#if defined(BUILD_MONOLITHIC)

#ifdef __cplusplus
extern "C" {
#endif

int dcmtk_cda2dcm_main(int argc, const char **argv);
int dcmtk_dcm2cda_main(int argc, const char **argv);
int dcmtk_dcm2img_main(int argc, const char **argv);
int dcmtk_dcm2json_main(int argc, const char **argv);
int dcmtk_dcm2pdf_main(int argc, const char **argv);
int dcmtk_dcm2pnm_main(int argc, const char **argv);
int dcmtk_dcm2xml_main(int argc, const char **argv);
int dcmtk_dcmcjpeg_main(int argc, const char **argv);
int dcmtk_dcmcjpls_main(int argc, const char **argv);
int dcmtk_dcmconv_main(int argc, const char **argv);
int dcmtk_dcmcrle_main(int argc, const char **argv);
int dcmtk_dcmdata_tests_main(int argc, const char **argv);
int dcmtk_dcmdjpeg_main(int argc, const char **argv);
int dcmtk_dcmdjpls_main(int argc, const char **argv);
int dcmtk_dcmdrle_main(int argc, const char **argv);
int dcmtk_dcmdspfn_main(int argc, const char **argv);
int dcmtk_dcmdump_main(int argc, const char **argv);
int dcmtk_dcmect_tests_main(int argc, const char **argv);
int dcmtk_dcmfg_tests_main(int argc, const char **argv);
int dcmtk_dcmftest_main(int argc, const char **argv);
int dcmtk_dcmgpdir_main(int argc, const char **argv);
int dcmtk_dcmicmp_main(int argc, const char **argv);
int dcmtk_dcmiod_tests_main(int argc, const char **argv);
int dcmtk_dcmj2pnm_main(int argc, const char **argv);
int dcmtk_dcml2pnm_main(int argc, const char **argv);
int dcmtk_dcmmkcrv_main(int argc, const char **argv);
int dcmtk_dcmmkdir_main(int argc, const char **argv);
int dcmtk_dcmmklut_main(int argc, const char **argv);
int dcmtk_dcmnet_tests_main(int argc, const char **argv);
int dcmtk_dcmodify_main(int argc, const char **argv);
int dcmtk_dcmp2pgm_main(int argc, const char **argv);
int dcmtk_dcmprscp_main(int argc, const char **argv);
int dcmtk_dcmprscu_main(int argc, const char **argv);
int dcmtk_dcmpschk_main(int argc, const char **argv);
int dcmtk_dcmpsmk_main(int argc, const char **argv);
int dcmtk_dcmpsprt_main(int argc, const char **argv);
int dcmtk_dcmpsrcv_main(int argc, const char **argv);
int dcmtk_dcmpssnd_main(int argc, const char **argv);
int dcmtk_dcmqridx_main(int argc, const char **argv);
int dcmtk_dcmqrscp_main(int argc, const char **argv);
int dcmtk_dcmqrti_main(int argc, const char **argv);
int dcmtk_dcmquant_main(int argc, const char **argv);
int dcmtk_dcmrecv_main(int argc, const char **argv);
int dcmtk_dcmrt_tests_main(int argc, const char **argv);
int dcmtk_dcmscale_main(int argc, const char **argv);
int dcmtk_dcmseg_tests_main(int argc, const char **argv);
int dcmtk_dcmsend_main(int argc, const char **argv);
int dcmtk_dcmsign_main(int argc, const char **argv);
int dcmtk_dcmsign_main(int argc, const char **argv);
int dcmtk_dcmsr_tests_main(int argc, const char **argv);
int dcmtk_dcmtls_tests_main(int argc, const char **argv);
int dcmtk_dcod2lum_main(int argc, const char **argv);
int dcmtk_dconvlum_main(int argc, const char **argv);
int dcmtk_drtdump_main(int argc, const char **argv);
int dcmtk_drttest_main(void);
int dcmtk_dsr2html_main(int argc, const char **argv);
int dcmtk_dsr2xml_main(int argc, const char **argv);
int dcmtk_dsrdump_main(int argc, const char **argv);
int dcmtk_dump2dcm_main(int argc, const char **argv);
int dcmtk_echoscu_main(int argc, const char **argv);
int dcmtk_findscu_main(int argc, const char **argv);
int dcmtk_getscu_main(int argc, const char **argv);
int dcmtk_img2dcm_main(int argc, const char **argv);
int dcmtk_mkdeftag_main(int argc, const char **argv);
int dcmtk_mkdictbi_main(int argc, const char **argv);
int dcmtk_mkreport_main(int argc, const char **argv);
int dcmtk_movescu_main(int argc, const char **argv);
int dcmtk_msgserv_main(int argc, const char **argv);
int dcmtk_oficonv_tests_main(int argc, const char **argv);
int dcmtk_ofstd_tests_main(int argc, const char **argv);
int dcmtk_pdf2dcm_main(int argc, const char **argv);
int dcmtk_stl2dcm_main(int argc, const char **argv);
int dcmtk_storescp_main(int argc, const char **argv);
int dcmtk_storescu_main(int argc, const char **argv);
int dcmtk_termscu_main(int argc, const char **argv);
int dcmtk_wlmscpfs_main(int argc, const char **argv);
int dcmtk_wltest_main(int argc, const char **argv);
int dcmtk_xml2dcm_main(int argc, const char **argv);
int dcmtk_xml2dsr_main(int argc, const char **argv);

#ifdef __cplusplus
}
#endif

#endif

