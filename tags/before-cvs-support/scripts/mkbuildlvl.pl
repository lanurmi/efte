#!/usr/bin/perl -w

use strict;
use FindBin;
use File::Spec;
use File::Basename;

# what build level do we have?

my @time = localtime;
my $bldlvl = sprintf "%04d%02d%02d", $time[5] + 1900, $time[4] + 1, $time[3];

my $dir = File::Spec->canonpath(File::Spec->catdir($FindBin::Bin,
                                                   File::Spec->updir,
                                                   File::Spec->updir,
                                                  )
                               );

chdir $dir;

my $sources = File::Spec->catfile($dir, "fte-$bldlvl-src.zip");
my $common  = File::Spec->catfile($dir, "fte-$bldlvl-common.zip");
print "sources => $sources\ncommon => $common\n";

my @src_files = find_sources();
my @cmn_files = find_common();

unlink $sources, $common;
system qw(zip -9), $sources, @src_files;
system qw(zip -9), $common, @cmn_files;

sub find_sources
{
    # am I really going for the obsfucated perl code contest?
    (
     (
      grep {
          ! /\.o$/ and
              ! -d $_ and
              ! -x _ and
              ! /defcfg\.(?:cnf|h)/ and
              ! /~$/
      } map {
          glob "fte/$_"
      } qw(
      Makefile
      fte.in
      src/*
      src/bmps/*
      src/icons/*
     )  # FTE bug in indentation?
     ),
    qw( fte/install )
    );
}

sub find_common
{
    grep { 
        ! -d $_ and
            $_ !~ /Makefile$/ and
            ! /~$/
    } map {
        glob "fte/$_"
    } qw(
    [A-Z]*
    file_id.diz
    doc/*
    config/*
    config/kbd/*
    config/menu/*
    config/slang/*
);
}
