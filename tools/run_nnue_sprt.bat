@echo off
REM ---------------------------------------------------------------------------
REM #39 NNUE A/B: the SAME binary against itself with UseNNUE flipped.
REM
REM Both arms are one exe from one commit, so the only difference in the match is
REM the leaf evaluation. Testing against a separately-built huginn_2.4.exe would
REM re-introduce every incidental difference between the two builds into a
REM verdict that is supposed to be about the eval alone. (The NNUE-off arm is
REM independently confirmed bit-identical to 2.4: startpos d14 = 3,481,582 /
REM cp 31 / e2e4.)
REM
REM House standard: 500 rounds (1000 games), SPRT elo0=0 elo1=10 alpha=beta=0.05,
REM 10+0.1, noob_3moves.epd, -repeat, concurrency 4, no books, no tablebases.
REM nodes/seldepth are written into the PGN so the record carries tree size --
REM a record PGN that is mute on node counts cannot be re-analysed later.
REM ---------------------------------------------------------------------------
setlocal
set FC=C:\Users\m_lad\Documents\fastchess-windows-x86-64
set NAME=%1
if "%NAME%"=="" set NAME=nnue1

"%FC%\fastchess.exe" ^
  -engine cmd="%FC%\huginn_%NAME%.exe" name=Huginn_%NAME% option.OwnBook=false ^
  -engine cmd="%FC%\huginn_%NAME%.exe" name=Huginn_HCE option.OwnBook=false option.UseNNUE=false ^
  -each tc=10+0.1 proto=uci ^
  -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 ^
  -rounds 500 ^
  -repeat ^
  -concurrency 4 ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%FC%\huginn_%NAME%.pgn" nodes=true seldepth=true append=false ^
  -log file="%FC%\fastchess_%NAME%.log" level=warn

endlocal
