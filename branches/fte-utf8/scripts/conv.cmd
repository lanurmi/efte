cd ..
cd doc
perl ../scripts/mkcmds.pl ../src/c_commands.h >command.html
perl ../scripts/mkcontents.pl >contents.html
perl ../scripts/html2ipf.pl  about.html install.html cfgfiles.html cmdopt.html global.html colors.html modes.html events.html command.html regexp.html status.html perftips.html colorize.html >fte.ipf
ipfc /inf fte.ipf
