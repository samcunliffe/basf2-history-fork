#!/bin/bash

# this is an expample script demonstrating on how to submit a series of jobs for different random seeds to the queue at KEK
# NOTE: you HAVE to adjust the numbers below otherwise you will generate 2000k events!!

# list of random seeds for which events should be generated (adjust to your needs)
# for each of these seeds nevt events will be generated,
# the old method to generate seeds was with following shell command (the output file rand.txt contains them): 
#    rm -f rand.txt ; for i in `seq 1 500`; do echo "$RANDOM " >> rand.txt;  done; cat  rand.txt | sort | uniq | wc -l ;
# the command was repeated till it outputs 500 (the number of requested seeds) to ensure that all seeds are unique

# the 2500 seeds have been generated with a python script:
# out = open('rand.txt', 'w')
# import random
# r_list = []
# while len(r_list)<2500:
#     r=int(random.uniform(1,2**15-1)) # 2**15-1 limit, same as the old method with do echo "$RANDOM "
#     if r not in r_list:
#         r_list.append(r)
#         out.write(str(r)+" ")

# 2500 seeds (with nevts=4000 : 10Mio {Y(4S)+uniform muon particlegun} evts)
rndseeds=(21014 26190 14559 26563 1796 21258 28692 32043 23894 3530 3532 3009 32625 1010 13276 29286 23824 28897 30705 8868 28898 16895 10917 24935 22455 22411 17621 16566 14320 6680 20073 6933 18959 18244 32422 25671 5677 12412 28820 18306 24840 31053 30087 14593 12975 21356 5969 32522 8682 11489 6828 21059 19756 10105 16525 6301 11527 30902 19460 25610 31773 6243 26018 11876 25362 31332 24216 71 26717 3146 4972 23210 29310 28329 15446 30059 2484 24754 1834 6355 1111 14434 18136 14209 8837 1908 5047 8799 18947 18808 7951 27883 21837 25947 23564 25305 13562 19765 9202 3841 24396 16468 28770 25565 8164 29997 13334 12537 23686 10816 31852 29079 15655 4105 5215 4603 5181 21043 21495 23159 3425 12769 31659 18557 14534 28481 23588 2926 13141 15034 23940 13126 24292 29365 1953 11470 16926 1097 7161 5427 32743 25236 12092 24502 26671 7396 28540 26532 28569 25622 7781 17811 28585 29723 24371 19993 29175 19638 4421 8148 20497 16516 26265 9337 24786 20125 25282 30305 10290 15345 27899 7697 12703 26371 15913 7833 5126 26499 26973 2406 25656 25329 25261 7284 8428 14240 11055 16547 15774 8030 10041 23749 22497 10951 13596 4314 12689 30592 7433 25179 32108 4874 16735 571 3620 3178 15383 21490 1844 8327 19007 19503 17962 9423 21147 1216 6878 14270 28778 4692 476 15769 15463 24010 18262 14202 23670 9591 17385 5863 24980 28995 5776 1974 6943 12038 20824 8610 23227 23755 22538 14588 30929 9314 31701 13184 29368 26543 18533 5914 29078 26469 22983 26540 30712 2817 13831 11619 26667 8402 2282 29075 13381 27004 9473 2724 18827 32337 14676 1651 9961 14319 1027 4100 13221 18144 25184 15469 25524 31402 21238 14549 733 8617 32065 27694 32457 7032 16576 16090 11839 28230 23176 22200 27214 32481 8245 13541 334 528 2269 8179 18988 27468 30776 17489 29801 16612 23076 5986 18516 21464 20246 26076 3443 22014 14437 8404 25993 15617 13423 7843 11454 12865 5853 25624 5101 31178 24758 3818 19475 18309 10022 1458 7443 7317 7856 32475 4878 12736 1174 20642 1007 3249 25835 19664 18298 15395 29305 13293 10607 3409 24685 17796 3791 28100 11592 4377 17809 29538 19404 28137 12847 27932 31476 28637 21286 15466 12214 9858 17323 28099 21865 14026 12395 28303 14637 2379 30166 29905 5100 7250 4862 12594 2568 27614 6384 24076 15408 6204 15003 28302 2685 16087 13872 19855 15499 6241 17693 17579 3631 20019 10272 9696 21899 29064 25279 4155 26955 22989 18681 9495 23075 25384 3459 22373 26559 1356 3468 3107 6104 16284 14696 11887 22148 17137 56 9083 10435 24613 27071 11251 2267 10492 2377 14399 28183 27066 27826 26792 15810 22826 32100 20736 11974 5506 18456 30876 31245 8312 20006 23521 22721 17283 971 21278 1756 30318 25643 27682 30049 30962 3528 7509 67 31593 22078 9544 14441 8625 4405 23791 31872 694 7753 281 17136 26700 29221 31200 8804 9141 5245 18011 22054 3793 3397 2080 4069 24078 5456 23322 12995 19333 25714 20737 27581 5348 14298 437 17764 27068 16165 22733 18103 3432 4009 14846 1791 2672 4046 32341 14936 2132 6408 28242 21924 28057 6945 13699 6172 8541 14837 6076 14511 12348 24956 15221 20602 21005 13648 8779 22073 21138 6199 17725 3745 27750 18006 8750 15241 29827 25313 4048 120 16290 10535 9232 30401 3782 31100 22282 7539 32200 25917 3387 26304 27203 2583 6717 32414 14709 23414 19573 3150 20155 32319 29062 26229 24675 19962 17837 21166 22521 18428 30053 25109 3269 32162 14433 32489 15508 11554 1556 30493 17168 6998 30017 25225 6407 5502 5366 22564 32507 27081 16767 31129 14644 13695 31057 18307 31326 23185 16107 5318 14462 15579 17987 16216 10237 23071 8034 15302 2432 10820 18172 4101 12648 12623 8883 29429 4185 1649 4631 32545 10464 25732 23260 1279 19355 22725 16793 25692 10838 16932 15577 32285 21949 6312 16036 20312 21084 548 30533 2823 31359 22613 31337 26675 8228 28767 19778 5414 16973 17093 30794 27514 19669 21527 21787 12624 11994 4941 17348 2115 22844 28999 865 15339 25973 12223 13606 28098 8991 22630 12579 32384 25339 30369 29418 13526 11838 26269 2762 28512 26905 23146 5333 2457 9755 31130 18376 2044 30686 2193 20480 22939 17450 18912 6552 27798 23690 24125 12586 22565 4936 16027 6578 32663 2098 12694 26680 15965 22953 12438 16861 16765 14175 10415 17554 28262 29076 8602 19423 10426 29158 7235 27208 12619 31112 8620 15758 9268 22988 23331 22212 3264 31157 24273 19284 30723 5398 6218 7952 30467 28671 23022 31828 22661 18032 24615 31448 16552 19136 3486 31192 25914 24361 5562 30989 6127 10728 25050 2118 19321 28396 28592 24932 27142 18078 29301 23804 8956 28530 14562 32670 30700 15093 29682 31900 24430 561 10247 23570 32458 11845 10365 18938 284 17501 6499 8828 18106 27544 5885 14867 9929 5674 31826 2426 30015 14371 28391 2035 10805 995 19578 14825 27358 14955 16121 5056 21007 5549 13363 21226 5838 11789 28040 25138 19013 15546 8806 19825 15322 15371 5438 30150 23793 18059 8615 1653 5689 19349 871 8268 2381 14976 12460 4005 3863 24484 21546 29436 27291 26079 27275 10477 2936 1928 32361 13477 7966 24916 23798 31699 13118 20273 15139 8650 16144 3465 16986 776 8644 25602 22334 27592 479 22879 1563 19115 10 3373 30669 20968 7090 8702 15629 7085 17477 28105 26838 28167 21624 27326 8522 13453 3328 8548 25147 26503 30655 16201 10349 13565 8140 28103 11203 14295 3883 29168 8360 29476 13486 32380 3876 32078 27454 4833 5822 31628 2416 6664 23160 18188 29312 21977 1039 2176 27011 12179 3307 32075 5020 31677 2726 23276 6897 24712 21717 10642 22927 4666 301 16019 16013 8213 30103 9327 13758 22597 30933 18333 27359 13839 18683 16022 23342 24720 18128 5519 225 31062 1914 17910 21938 1380 20682 17913 8311 6905 32680 22310 31540 29002 16517 752 31959 14597 27756 18469 11167 31766 32603 19361 17792 10381 11926 30258 25700 29506 4130 27908 15277 32249 13280 4171 6374 10445 29680 1921 24110 12493 29198 2741 13922 30892 25477 2252 3476 21584 18241 20468 9063 4034 7114 8565 11023 24918 12825 19096 21968 9254 6272 12538 22085 2042 20407 4809 22429 887 9307 6273 23203 15624 28789 18213 5199 6356 31451 27685 3813 25733 6795 24260 22971 22484 1594 18000 20797 811 18943 19694 18926 23750 9157 28666 16503 12040 13037 26579 28801 5191 1147 27435 8256 21898 27013 17410 32194 6125 18706 7913 7336 14156 2547 30711 23012 18402 28 1168 6340 27003 21357 8270 10186 2443 20435 7111 27742 13760 5012 30411 18218 2807 12441 29169 29329 4416 22710 6684 25666 30300 11775 22767 11938 7489 28089 18408 6909 28632 10147 2225 31549 32587 19036 7865 577 23772 12262 29098 23560 26970 23855 8928 25347 10063 20284 15178 23219 3743 24898 9233 9400 7715 19689 21872 28787 29235 10969 30272 6222 29400 4512 22134 32622 32027 26652 26077 2088 30331 15853 31255 13494 17321 15419 2839 30134 19700 350 23376 16212 22678 9460 7557 12669 30434 4070 21124 8471 26711 20564 4040 8277 10182 6813 13567 17730 5629 10648 16005 30281 8106 14258 24949 23835 14091 10831 13511 3770 16499 30133 29553 30750 4361 20233 7269 20128 2680 24119 28290 16160 3136 30507 10075 28859 23433 17179 15993 1965 2276 18749 27538 14491 30676 29804 29249 19156 24653 21843 22921 14330 12795 22563 8299 24884 28009 19818 6842 27971 5738 16117 32465 15452 24014 4051 19076 14654 6254 454 18018 9698 12034 26398 24704 28208 21455 14131 27825 3487 19890 30029 26031 14492 18356 9808 10081 30855 25629 22374 8503 27469 1716 16295 9425 22530 22752 21692 17533 31373 3129 9685 11987 13954 6833 30111 2787 16219 25088 4430 13204 21292 7588 26762 8388 31293 29765 31252 22093 26933 14754 10675 22244 25739 10907 16303 8573 14307 26754 30528 3575 7600 12007 30682 31658 24841 17447 2298 27764 11981 30596 27944 557 3727 30448 8342 32052 31394 7550 31251 13560 25365 23864 3284 3561 4177 28721 20049 2195 28131 639 12416 7483 14495 15148 19640 20663 30514 29049 25357 3200 8848 17889 98 15498 32242 23471 16967 26983 30086 27219 15638 7019 19389 12555 14170 16461 31655 26484 7976 4437 8059 14920 4911 14969 19899 11750 457 6720 29866 3516 5667 29066 8972 1067 12169 11878 7429 13751 10386 12675 29326 27271 8317 5589 32665 22860 30003 5416 2938 14313 30608 2830 32416 1368 8977 3772 1452 18859 30444 16161 15631 7928 2086 10002 2985 26582 14715 14407 9687 20071 16387 8612 23945 9420 31576 15344 7647 31632 1019 10714 1157 6318 9760 27643 16666 5931 14604 15013 14873 20404 8320 12120 4744 21764 25185 2850 16146 22413 31403 10357 31514 26786 2629 20777 15108 22079 16964 1440 14584 27530 12701 19754 515 3050 8044 14692 28199 13377 4018 4507 32092 20448 15181 7662 6162 24101 5133 17646 125 32270 23198 13284 893 6298 4810 29694 1531 12030 16198 4364 10582 28378 32372 18313 3061 6386 28757 32558 6721 17720 27704 892 16014 15841 23088 10618 19248 13586 2731 7102 700 18435 31374 14119 16225 19192 677 2590 10181 23680 6140 12295 24269 21008 26830 13079 8907 16871 21470 25703 29906 6992 14536 20211 10417 28792 13398 20609 21738 18205 16865 19261 2608 547 1323 10929 15762 12561 23082 18574 5744 16759 17469 19153 32371 27171 27443 218 13444 21996 13818 5933 26705 21313 4370 23234 15920 10310 11103 10224 20475 12356 27936 29750 24532 9797 26836 24316 5795 25379 7066 14446 9583 28625 6777 28780 9217 27075 23747 19394 384 3347 19002 911 10251 24347 7610 22548 12752 27755 21883 5951 4096 16906 13704 24420 27953 29954 19107 26641 16617 10401 25241 15464 1866 12443 30738 14586 10011 18508 29034 29027 13290 29300 19974 19256 6441 18033 30632 13099 17972 14305 19596 27424 27539 8559 15421 5783 25336 2060 4797 19351 27535 2160 1938 31581 12717 14553 19025 2705 19029 29296 5675 28536 27363 12447 2373 74 17711 21626 25677 30703 26912 11289 11420 28737 5143 1963 24178 23489 28593 17024 12613 819 13914 10231 16589 9620 4360 13353 12213 8715 18463 8086 24807 22653 30500 12436 10787 30982 26468 23206 30324 27502 4545 8699 19604 12830 21349 25638 21742 2651 21160 26689 8098 5844 3241 5661 13487 12001 3294 20248 13795 25411 19659 16609 27292 1401 4774 22614 28457 12536 29044 27789 934 27699 6707 2551 11796 10633 8468 14970 853 29878 20603 1962 13405 29812 15216 16285 32459 5527 27024 13433 10218 5747 28822 13085 23603 25042 23990 1439 21792 30421 3568 14311 20151 5546 15425 23130 29510 10850 6852 32436 15010 16071 32498 725 18556 11528 17267 336 8258 6266 25556 2891 5202 8131 12517 11990 19830 9421 20192 29297 23090 9481 3303 22251 1902 13936 20362 31679 1520 30745 14487 7438 7142 13901 25891 12267 9743 4709 15267 20667 11969 21326 29453 13701 2425 5879 19738 11490 10279 6357 29988 14327 31812 28769 6965 24964 24429 31280 8875 7405 30607 23954 31467 23151 19788 24324 8603 30698 12814 28590 16332 7897 18944 5433 31115 7293 30266 30052 17860 6028 8627 11848 9424 8537 7261 8575 20141 19519 16420 18987 21 32758 25777 14714 19945 14011 7731 19194 22434 22705 7412 23684 17073 405 27281 22291 805 5063 31179 26834 1998 32046 21984 32573 32588 24308 31183 3127 23457 15060 1277 26205 6201 23087 18348 31502 20562 21430 30714 22086 30694 3808 12719 10555 22296 6078 27647 13022 10736 24519 27450 5170 20323 4884 19205 2753 4990 15081 15691 20109 12343 8363 7862 1824 5394 12154 21015 169 23721 32495 3171 26808 26751 30885 26606 3600 10730 26167 30008 5226 30232 25597 27324 29678 760 31397 250 12011 25526 30994 8221 26073 26193 19915 21023 19639 3959 11277 27988 26508 436 31034 8073 17390 22926 20311 18521 737 12180 28868 27867 11833 29459 31244 7299 28328 15447 12987 29353 9513 2280 23658 985 10125 23681 29166 17849 17520 1179 8807 266 28713 12283 25124 3950 28162 19665 10549 28614 32641 20097 23691 3804 7017 6900 32104 7981 18719 16637 30515 6802 3535 22234 6781 1453 31884 2486 3405 29332 3502 24469 23723 14705 6520 18446 31324 13700 11582 16502 21167 20676 1586 7021 13391 19769 5016 10961 7591 20508 20498 18100 31660 11345 13222 18250 28295 15572 16007 18139 12422 13680 11859 31859 802 1001 20258 3537 785 31235 1939 28292 29551 9128 29805 10815 2446 2678 2876 20093 16510 417 8593 11943 30953 31485 12496 8549 14662 28959 26118 15755 6233 30998 27722 30295 28568 15886 17631 18112 16098 15784 5662 27884 32444 2513 27720 30806 19146 1981 18968 19832 1957 20814 7996 22599 24183 25815 24143 20566 13062 7740 9498 4542 9997 27282 14557 1749 18788 31173 15051 32764 32091 25462 29261 26517 16789 1181 204 3585 25294 31823 23845 19203 16873 28818 22332 29861 9723 19183 20755 23053 30188 1141 21424 13499 27002 5321 27637 4359 21655 2393 8372 25046 24199 20655 16289 12204 9064 24291 15889 8773 27112 9422 32455 24558 5999 16693 17527 20238 10042 29292 16450 132 4780 14360 31210 29707 26087 16815 1615 24770 5664 11785 12887 13515 4527 25791 26541 8301 14996 18730 31700 12275 23443 32607 2101 11273 30346 24642 15038 12949 24229 5050 14961 10724 1475 19764 31009 16843 6048 4927 12273 25795 2366 22349 14861 31821 17917 4436 24731 18330 1310 10800 20619 12549 4720 14567 16785 24004 18453 29398 5565 12110 17010 29039 25854 10922 27489 18036 4792 11561 4966 11160 9050 21355 24698 18973 16465 12551 13207 10330 676 22372 11877 9666 172 7859 1891 31387 7933 2748 2835 27972 3356 17068 1485 1717 13173 11526 11278 16282 28357 7869 1999 8095 20465 10864 25878 19288 1228 12075 15922 15204 31320 7296 26053 3132 17522 5287 22851 4916 19790 15683 22205 25320 16618 15024 29785 18526 28039 32116 28011 32089 25847 22982 8178 7415 12772 9359 10671 10926 22803 24567 27916 27687 13923 26001 29130 17065 513 26476 19599 30623 10786 31368 30172 18050 255 13713 16088 30903 18173 17716 6786 29411 11766 29303 5959 21732 24165 6589 24209 15363 5582 22466 9771 4112 10229 5983 25056 16762 4935 5825 23928 25813 15620 10185 17570 3306 2544 10921 9028 9456 11479 21518 6613 13613 14672 27338 21526 967 28095 2002 7525 4681 30945 2896 18046 23638 20673 3156 24431 10880 9593 5149 32053 13124 2822 28146 19140 11625 94 23287 22751 12042 14721 2740 18825 27599 10718 24403 3097 25619 23915 7650 4153 5261 1905 29515 20389 10541 16114 8860 3138 3828 5718 30400 8403 6130 10976 3921 2126 12239 32397 9371 13497 11088 26803 12222 6126 715 19231 21475 3194 32500 12981 24654 6803 14689 16571 9044 21944 12277 20169 9081 11195 31955 15142 25038 2129 27985 948 20458 13189 19347 28869 1619 24222 21381 11302 16491 17515 3478 26790 9230 23478 14445 15111 28965 15349 3000 32486 24279 19910 31779 17592 31713 32580 4417 3489 19291 5544 15825 26657 18965 16770 6543 28301 16857 18671 16918 13242 7346 5395 13549 11798 29945 7031 21864 21442 23158 8915 7997 27547 4093 5222 23404 10052 13633 28622 5585 15007 19824 12994 17510 7240 19093 3774 28741 25702 11575 8237 761 3361)

# Bhabha 500 seeds (with nevts=4000 & eventSimulationBhabha.py: 2Mio BhaBha evts)
# rndseeds=(16565 18025 20660 26317 9748 5259 7690 13822 28050 14260 25690 7415 11562 13645 10739 1448 25753 27373 10419 30888 26664 2526 9035 28431 28636 10481 9524 24387 3750 32548 28539 30523 494 398 9194 29171 618 13273 6425 23450 20642 32430 21780 11470 18189 26385 7999 19987 8536 13785 1915 27455 15860 7300 6095 25350 9285 2891 858 11953 9530 6805 25614 6452 15620 23777 30026 11068 12937 1114 10813 16342 5222 7491 14229 30671 24127 17776 32317 7757 14596 25121 16728 21266 30892 12862 19945 28228 24969 566 19076 32144 19360 9094 13372 31848 16319 19764 8105 16113 24578 7456 32100 24759 28030 23572 18801 22541 31643 26630 22561 15149 31861 31885 30443 9108 12457 24319 27516 31268 11503 20782 2097 29305 10821 18162 14256 14746 11397 14431 4891 18541 1322 8607 27476 23105 20039 16755 13640 16707 1609 138 29292 29478 2003 5722 20072 5129 8796 7882 13598 11315 32539 24815 12462 8013 15801 28583 24411 19093 13295 15797 7203 22640 28628 23671 17478 31904 17197 14013 14017 942 7025 21305 28273 3405 32685 17954 16891 7969 13017 14616 30427 28393 539 25950 20900 9118 24088 7635 11680 23101 12263 8053 2453 20659 32395 13349 12174 1443 18713 26593 13047 19377 25320 25764 1572 30203 12781 239 1732 26745 30787 19034 30885 23818 21412 15406 14891 6227 30366 32169 16548 25405 4273 29754 1984 17178 9423 8460 31665 13150 3346 30809 16435 28658 6643 13008 22986 8778 12173 29689 4737 8502 27921 21095 26816 21163 26982 11831 18625 343 15549 3011 2379 6026 13619 13257 31708 21091 18406 2030 3115 21844 29914 16676 10851 14434 31591 17742 8947 6113 10060 11066 26403 26519 21309 26546 9844 2489 20885 6336 19851 21320 16070 23863 8882 6189 27379 26129 10075 4683 28803 12638 12180 5362 19067 13358 22587 20306 6881 20285 1154 7534 17120 10350 25795 13164 15267 7856 32537 14346 5302 9523 9477 9976 27284 5431 10512 6782 28012 26473 1739 25884 13920 4611 22125 16147 31033 11646 10465 13601 32162 4313 32061 15124 24666 22382 21105 1482 18471 6079 11530 13196 12249 8946 19109 23409 6619 22091 27034 16667 7618 7392 28489 24909 29470 23562 3839 21819 754 22149 5963 2331 25937 12686 26291 13567 16445 20413 13187 22597 26527 30157 23580 2555 18313 13632 12712 25762 6355 16095 26707 23317 5852 10669 11820 16529 23241 28748 14081 20611 6659 15786 24146 17447 2334 10224 11903 1789 8693 30113 10061 15342 22014 30168 28340 14033 23916 21238 21210 6717 7959 26952 31231 13825 11004 24440 11899 28066 30631 28908 10087 9840 30351 16075 7338 21453 30324 15165 8024 13206 8330 6161 12096 8395 6982 32675 6479 27783 16903 31747 26737 18598 4872 19295 15131 26349 275 29881 23514 27527 21189 2705 31444 27063 16619 19328 2274 20689 19867 21201 29290 18557 18036 14216 5484 31162 11429 9138 28554 27023 5403 17242 11986 26207 21089 24880 6482 19498 6066 6222 3424 10158 11251 11051 13608 28176 13226 14321 16049 17274 18288 11385 31227 31396 12814 25319 31093 18292)

# the name of the output directory where the root files will be stored
outdir=./datadir/
# number of events generated per random seed (at around 6000 events the cpu limit is reached for the s queue)
nevt=4000
# the name of the queue at KEK
queue=l

# loop over all random seeds
for seed in ${rndseeds[@]} 
do
  echo $seed
  # use loglevel WARNING so that information on non-default DB payloads is displayed
  bsub -q $queue -o ${outdir}log${seed}.log  basf2 -l WARNING eventSimulation.py $seed $outdir -n $nevt
done
