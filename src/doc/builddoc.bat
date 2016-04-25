@echo off
:-  This batch file rebuilds the Libero documentation kit
:-  First time through to create all anchors
call htmlpp lrindex.txt
call htmlpp lrintr.txt
call htmlpp lrinst.txt
call htmlpp lruser.txt
call htmlpp lrexam.txt
call htmlpp lrlang.txt
call htmlpp lrtech.txt

:-  Second time through to generate the output
call htmlpp lrindex.txt
call htmlpp lrintr.txt
call htmlpp lrinst.txt
call htmlpp lruser.txt
call htmlpp lrexam.txt
call htmlpp lrlang.txt
call htmlpp lrtech.txt

call htmlpp lrdoc.txt
call htmlpp lrdoc.txt

zip -f i:\site\pub\tools\lrdoc
for %%f in (*.htm) do call install %%f i:\site\html\libero

i:
cd \site\html\libero
call makedoc
c:
