REM docker run -it --rm 9318bc341fe1 /bin/ash
REM docker run -it --rm -v "%cd%:/usr/src/project" -w /usr/src/project source-highlight source-highlight -s cpp -f html --doc source-highlight.cc
docker run -it --rm -v "%cd%:/usr/src/project" -w /usr/src/project source-highlight source-highlight -s cpp -f html --doc snippet.cc snippet.given.00.cc snippet.before.00.cc snippet.after.00.cc snippet.return-based.00.cc snippet.return-based.01.cc snippet.exception-based.00.cc snippet.exception-based.01.cc
pause
