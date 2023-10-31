@ECHO o
@ECHO == MHVN Visual Novel Engine === MHVNビジュアルノベルエンジン ==
@ECHO ==    for PC-9800 series    ===      PC-9800シリーズ用       ==
@ECHO ==     Maxim Hoxha 2023     ===    マキシマム・ホジャ 2023     ==
@ECHO o
@ECHO This is a test, not a full story   : これは完全な物語じゃなくて、テストです。
@ECHO Please report any bugs immediately : バグをすぐに報告してください。
@ECHO o
@ECHO Starting Engine... : エンジン開始...
@ONGCHK
@IF ERRORLEVEL==5 GOTO s86
@IF ERRORLEVEL==4 GOTO sb2
@IF ERRORLEVEL==3 GOTO s86
@IF ERRORLEVEL==2 GOTO sb2
@IF ERRORLEVEL==1 GOTO s26k
@ECHO No suitable FM board found!!
@ECHO 相応しいＦＭ音源を見つけなかった！！
@MHVN98
@GOTO exit
:s26k
@ECHO 26K FM board found! Not as good, but better than nothing.
@ECHO ２６ＫＦＭ音源を見つけた！あまり良くないが、音源がない場合よりいい。
@PMD /V0 /K /F- /I-
@MHVN98
@PMD /R
@GOTO exit
:sb2
@ECHO YM2608-compatible FM board found! PCM playback may not sound very nice.
@ECHO ＹＭ２６０８互換ＦＭ音源を見つけた！ＰＣＭ再生はあまり良くないかも。
@PMDB2 /V0 /N- /K /F- /I-
@MHVN98
@PMDB2 /R
@GOTO exit
:s86
@ECHO 86 FM board found! This is perfect!
@ECHO ８６ＦＭ音源を見つけた！カンペキ！
@PMD86 /V0 /N- /K /F- /I-
@MHVN98
@PMD86 /R
:exit
@ECHO Goodbye for now!
@ECHO さようなら！