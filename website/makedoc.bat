@echo off
zip -r lrhtml *.* -x lrdoc.htm makedoc.bat
zip lrfull lrdoc.htm *.gif
copy *.zip \site\pub\libero\doc
del *.zip
