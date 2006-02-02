static PageInfo standard_names[] = {
  /* sorted by name, remember to sort when changing */
  {"iso_a3", "297x420mm", "A3"},
  {"iso_a4", "210x297mm", "A4"},
  {"na_index-3x5", "3x5in", "Index 3x5"},
  {"na_letter", "8.5x11in", "Letter"},
  {"na_personal", "3.625x6.5in", "Personal (envelope)"},
};

/* These names from the standard are not yet converted into the table: 

Legacy Name           Ref. Alias (common name)            Self-Describing Name (inches)
                           personal (envelope)            
monarch-envelope      2                                   na_monarch_3.875x7.5in
na-number-9-envelope  1, 2                                na_number-9_3.875x8.875in
                           index-4x6 (postcard)           na_index-4x6_4x6in
na-number-10-envelope 1, 2 comm-10 (envelope)             na_number-10_4.125x9.5in
                           a2 (envelope)                  na_a2_4.375x5.75in
                           number-11 (envelope)           na_number-11_4.5x10.375in
                           number-12 (envelope)           na_number-12_4.75x11in
                           5x7                            na_5x7_5x7in
                           index-5x8                      na_index-5x8_5x8in
                           number-14 (envelope)           na_number-14_5x11.5in
invoice               2    statement, mini, half-letter   na_invoice_5.5x8.5in
                           index-4x6-ext                  na_index-4x6-ext_6x8in
na-6x9-envelope       1, 2 6x9 (envelope)                 na_6x9_6x9in
                           c5 (envelope)                  na_c5_6.5x9.5in
na-7x9-envelope       1, 2 7x9 (envelope)                 na_7x9_7x9in
executive             2                                   na_executive_7.25x10.5in
na-8x10               2    government-letter              na_govt-letter_8x10in
                           government-legal               na_govt-legal_8x13in
quarto                2                                   na_quarto_8.5x10.83in
                           fanfold-European               na_fanfold-eur_8.5x12in
                           letter-plus                    na_letter-plus_8.5x12.69in
                           foolscap, german-legal-fanfold na_foolscap_8.5x13in
na-legal              1, 2 legal                          na_legal_8.5x14in
                           super-a                        na_super-a_8.94x14in
na-9x11-envelope      1, 2 9x11 (envelope), letter-tab    na_9x11_9x11in
arch-a                2    architecture-a (envelope)      na_arch-a_9x12in
                           letter-extra                   na_letter-extra_9.5x12in
                           legal-extra                    na_legal-extra_9.5x15in
                           10x11                          na_10x11_10x11in
na-10x13-envelope     1, 2 10x13 (envelope)               na_10x13_10x13in
na-10x14-envelope     1, 2 10x14 (envelope)               na_10x14_10x14in
na-10x15-envelope     1, 2 10x15 (envelope)               na_10x15_10x15in
na-10x15-envelope     1, 2 10x15 (envelope)               na_10x15_10x15in
                           11x12                          na_11x12_11x12in
                           edp                            na_edp_11x14in


Legacy Name Ref. Alias (common name)           Self-Describing Name (inches)
                 fanfold-us                    na_fanfold-us_11x14.875in
                 11x15                         na_11x15_11x15in
tabloid     2    ledger, b, engineering-b      na_ledger_11x17in
                 european-edp                  na_eur-edp_12x14in
arch-b      2    architecture-b, tabloid-extra na_arch-b_12x18in
                 12x19                         na_12x19_12x19in
                 b-plus                        na_b-plus_12x19.17in
                 super-b                       na_super-b_13x19in
c           2    engineering-c                 na_c_17x22in
arch-c      2    architecture-c                na_arch-c_18x24in
d           2    engineering-d                 na_d_22x34in
arch-d      2    architecture-d                na_arch-d_24x36in
f           5    e1                            asme_f_28x40in
                 wide-format                   na_wide-format_30x42in
e           2    engineering-e                 na_e_34x44in
arch-e      2    architecture-e                na_arch-e_36x48in
                 f, engineering-f              na_f_44x68in

Legacy Name Ref. Alias (common name) Self-Describing Name (inches)
                 roc-16k             roc_16k_7.75x10.75in
                 roc-8k              roc_8k_10.75x15.5in

Legacy Name    Ref. Alias (common name) Self-Describing Name (mm)
iso-a10        1, 2 a10                 iso_a10_26x37mm
iso-a9         1, 2 a9                  iso_a9_37x52mm
iso-a8         1, 2 a8                  iso_a8_52x74mm
iso-a7         1, 2 a7                  iso_a7_74x105mm
iso-a6         1, 2 a6                  iso_a6_105x148mm
iso-a5         1, 2 a5                  iso_a5_148x210mm
                    a5-extra            iso_a5-extra_174x235mm
                    a4-tab              iso_a4-tab_225x297mm
                    a4-extra            iso_a4-extra_235.5x322.3mm
iso-a4x3, a4x3 2, 4                     iso_a4x3_297x630mm

Legacy Name    Ref. Alias (common name) Self-Describing Name (mm)
iso-a4x4, a4x4 2, 4                     iso_a4x4_297x841mm
iso-a4x5, a4x5 2, 4                     iso_a4x5_297x1051mm
iso-a4x6, a4x6 2, 4                     iso_a4x6_297x1261mm
iso-a4x7, a4x7 2, 4                     iso_a4x7_297x1471mm
iso-a4x8, a4x8 2, 4                     iso_a4x8_297x1682mm
iso-a4x9, a4x9 2, 4                     iso_a4x9_297x1892mm
iso-a3-extra                            iso_a3-extra_322x445mm
iso-a2         1, 2 a2                  iso_a2_420x594mm
iso-a3x3, a3x3 2, 4                     iso_a3x3_420x891mm
iso-a3x4, a3x4 2, 4                     iso_a3x4_420x1189mm
iso-a3x5, a3x5 2, 4                     iso_a3x5_420x1486mm
iso-a3x6, a3x6 2, 4                     iso_a3x6_420x1783mm
iso-a3x7, a3x7 2, 4                     iso_a3x7_420x2080mm
iso-a1         1, 2 a1                  iso_a1_594x841mm
iso-a2x3, a2x3 2, 4                     iso_a2x3_594x1261mm
iso-a2x4, a2x4 2, 4                     iso_a2x4_594x1682mm
iso-a2x5, a2x5 2, 4                     iso_a2x5_594x2102mm
iso-a0         1, 2 a0                  iso_a0_841x1189mm
iso-a1x3, a1x3 2, 4                     iso_a1x3_841x1783mm
iso-a1x4, a1x4 2, 4                     iso_a1x4_841x2378mm
a0x2           4    2a0                 iso_2a0_1189x1682mm
a0x3           4                        iso_a0x3_1189x2523mm
iso-b10        1, 2 b10                 iso_b10_31x44mm
iso-b9         1, 2 b9                  iso_b9_44x62mm
iso-b8         1, 2 b8                  iso_b8_62x88mm
iso-b7         1, 2 b7                  iso_b7_88x125mm
iso-b6         1, 2 b6 (envelope)       iso_b6_125x176mm
                    b6/c4 (envelope)    iso_b6c4_125x324mm
iso-b5         1, 2 b5 (envelope)       iso_b5_176x250mm
                    b5-extra            iso_b5-extra_201x276mm
iso-b4         1, 2 b4 (envelope)       iso_b4_250x353mm
iso-b3         1, 2 b3                  iso_b3_353x500mm
iso-b2         1, 2 b2                  iso_b2_500x707mm
iso-b1         1, 2 b1                  iso_b1_707x1000mm
iso-b0         1, 2 b0                  iso_b0_1000x1414mm
                    c10 (envelope)      iso_c10_28x40mm
                    c9 (envelope)       iso_c9_40x57mm
iso-c8         1    c8 (envelope)       iso_c8_57x81mm
iso-c7         1    c7 (envelope)       iso_c7_81x114mm
                    c7/c6 (envelope)    iso_c7c6_81x162mm
iso-c6         1, 2 c6 (envelope)       iso_c6_114x162mm

Legacy Name    Ref. Alias (common name)            Self-Describing Name (mm)
                    c6/c5 (envelope)               iso_c6c5_114x229mm
iso-c5         1, 2 c5 (envelope)                  iso_c5_162x229mm
iso-c4         1, 2 c4 (envelope)                  iso_c4_229x324mm
iso-c3         1, 2 c3 (envelope)                  iso_c3_324x458mm
iso-c2         1    c2 (envelope)                  iso_c2_458x648mm
iso-c1         1    c1 (envelope)                  iso_c1_648x917mm
iso-c0         1    c0 (envelope)                  iso_c0_917x1297mm
iso-designated 1, 2 designated-long, dl (envelope) iso_dl_110x220mm
iso-ra2                                            iso_ra2_430x610mm
iso-sra2                                           iso_sra2_450x640mm
iso-ra1                                            iso_ra1_610x860mm
iso-sra1                                           iso_sra1_640x900mm
iso-ra0                                            iso_ra0_860x1220mm
iso-sra0                                           iso_sra0_900x1280mm

Legacy Name Ref. Alias (common name)     Self-Describing Name (mm)
jis-b10     1, 2                         jis_b10_32x45mm
jis-b9      1, 2                         jis_b9_45x64mm
jis-b8      1, 2                         jis_b8_64x91mm
jis-b7      1, 2                         jis_b7_91x128mm
jis-b6      1, 2                         jis_b6_128x182mm
jis-b5      1, 2                         jis_b5_182x257mm
jis-b4      1, 2                         jis_b4_257x364mm
jis-b3      1, 2                         jis_b3_364x515mm
jis-b2      1, 2                         jis_b2_515x728mm
jis-b1      1, 2                         jis_b1_728x1030mm
jis-b0      1, 2                         jis_b0_1030x1456mm
                 exec                    jis_exec_216x330mm
                 chou4 (envelope)        jpn_chou4_90x205mm
                 hagaki (postcard)       jpn_hagaki_100x148mm
                 you4 (envelope)         jpn_you4_105x235mm
                 chou2 (envelope)        jpn_chou2_111.1x146mm
                 chou3 (envelope)        jpn_chou3_120x235mm
                 oufuku (reply postcard) jpn_oufuku_148x200mm
                 kahu (envelope)         jpn_kahu_240x322.1mm
                 kaku2 (envelope)        jpn_kaku2_240x332mm

Legacy Name Ref. Alias (common name) Self-Describing Name (mm)
                 prc-32k             prc_32k_97x151mm
                 prc1 (envelope)     prc_1_102x165mm
                 prc2 (envelope)     prc_2_102x176mm
                 prc4 (envelope)     prc_4_110x208mm
                 prc5 (envelope)     prc_5_110x220mm
                 prc8 (envelope)     prc_8_120x309mm
                 prc6 (envelope)     prc_6_120x320mm
                 prc3 (envelope)     prc_3_125x176mm
                 prc-16k             prc_16k_146x215mm
                 prc7 (envelope)     prc_7_160x230mm
                 juuro-ku-kai        om_juuro-ku-kai_198x275mm
                 pa-kai              om_pa-kai_267x389mm
                 dai-pa-kai          om_dai-pa-kai_275x395mm
                 prc10 (envelope)    prc_10_324x458mm

Legacy Name Ref. Alias (common name) Self-Describing Name (mm)
                 small-photo         om_small-photo_100x150mm
                 Italian (envelope)  om_italian_110x230mm
                 Postfix (envelope)  om_postfix_114x229mm
                 large-photo         om_large-photo_200x300
folio       2                        om_folio_210x330mm
                 folio-sp            om_folio-sp_215x315mm
                 Invite (envelope)   om_invite_220x220mm

*/
